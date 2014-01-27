/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_fb.c
 * $Revision: 1.11 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2011, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *-----------------------------------------------------------------------------
 * Description:
 *  Framebuffer / kernel mode setting functions.
 *-----------------------------------------------------------------------------
 */
#define MODULE_NAME hal.oal

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fb.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/export.h>
#else
#include <linux/module.h>
#endif
#include <drmP.h>
#include <uapi/drm/drm.h>
#include <drm_crtc.h>
#include <drm_crtc_helper.h>
#include <drm_fb_helper.h>
#include <linux/version.h>
#include <linux/vga_switcheroo.h>

#include "drm_emgd_private.h"
#include "emgd_drv.h"
#include "emgd_drm.h"
#include "memory.h"
#include "io.h"
#include "mode_dispatch.h"

/* Necessary to import page list for a PVR-allocated surface */
#include "pvr_bridge_km.h"



#if  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
#define RETURN_PROBE_TYPE int
#define RETURN_PROBE return 0
#define PROBE_FUNC fb_changed
#else
#define RETURN_PROBE_TYPE void
#define RETURN_PROBE
#define PROBE_FUNC output_poll_changed
#endif



/*------------------------------------------------------------------------------
 * Global Variables
 *------------------------------------------------------------------------------
 */
extern igd_framebuffer_info_t primary_fb_info;
extern mode_context_t         mode_context[1];
static struct drm_mode_set    panic_mode;
extern int                    drm_emgd_init;
extern emgd_drm_config_t      config_drm;

/* CRTC Dispatch Tables */
extern const struct drm_crtc_funcs emgd_crtc_funcs;
extern const struct drm_crtc_helper_funcs emgd_crtc_helper_funcs;

/* Encoder Dispatch Tables */
extern const struct drm_encoder_funcs emgd_encoder_funcs;
extern const struct drm_encoder_helper_funcs emgd_encoder_helper_funcs;

/* Connector Dispatch Tables */
extern const struct drm_connector_funcs emgd_connector_funcs;
extern const struct drm_connector_helper_funcs emgd_connector_helper_funcs;

/* FBCON Dispatch Table */
extern const struct fb_ops emgd_fb_ops;



/*------------------------------------------------------------------------------
 * Formal Declaration
 *------------------------------------------------------------------------------
 */
extern int emgd_fbcon_initial_config(emgd_fbdev_t *emgd_fbdev);

static int emgd_fbdev_init(drm_emgd_priv_t *priv);
static void emgd_fbdev_destroy(drm_emgd_priv_t *priv);



/*------------------------------------------------------------------------------
 * FB Functions
 *------------------------------------------------------------------------------
 */
int emgd_framebuffer_init(struct drm_device *dev,
			emgd_framebuffer_t *emgd_fb,
			struct DRM_MODE_FB_CMD_TYPE *mode_cmd,
			unsigned long offset);
static struct drm_framebuffer *emgd_user_framebuffer_create(
								struct drm_device *dev,
								struct drm_file *filp,
								struct DRM_MODE_FB_CMD_TYPE *r);
RETURN_PROBE_TYPE  emgd_fb_probe(struct drm_device *dev);
static int emgd_fb_panic(struct notifier_block *n,
			unsigned long res,
			void *panic_str);
static void emgd_user_framebuffer_destroy (struct drm_framebuffer *fb);
static int  emgd_user_framebuffer_create_handle(struct drm_framebuffer *fb,
				struct drm_file *file_priv, unsigned int *handle);
static void emgd_fb_restore(void);


static const struct drm_mode_config_funcs emgd_mode_funcs = {
	.fb_create  = emgd_user_framebuffer_create,
	.PROBE_FUNC = emgd_fb_probe,
	/*.output_poll_changed:  we don't support hotplug */
};


static struct notifier_block paniced = {
	.notifier_call = emgd_fb_panic,
};


static const struct drm_framebuffer_funcs emgd_fb_funcs = {
	.destroy       = emgd_user_framebuffer_destroy,
	.create_handle = emgd_user_framebuffer_create_handle,
};


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
int convert_bpp_depth_to_drm_pixel_formal(unsigned int bpp, unsigned int depth)
{
	switch(bpp){
		case 8:
			if(depth == 8)
				return DRM_FORMAT_RGB332;

		case 16:
			if(depth == 15)
				return DRM_FORMAT_XRGB1555;
			else
				return DRM_FORMAT_RGB565;

		case 24:
			return DRM_FORMAT_RGB888;

		case 32:
            if(depth == 24)
                    return DRM_FORMAT_XRGB8888;
            else if (depth == 30)
                    return DRM_FORMAT_XRGB2101010;
            else
                    return DRM_FORMAT_ARGB8888;

		default:
            EMGD_ERROR("bad bpp %d, assuming x8r8g8b8 pixel format", bpp);
            return DRM_FORMAT_XRGB8888;
	}
}
#endif

/*
 * Called if something fails while trying to set up framebuffer based
 * console.
 */
static int emgd_fb_panic(struct notifier_block *n,
		unsigned long res,
		void *panic_str)
{
	EMGD_DEBUG("Panic occurred, switch back to text console.");

	emgd_fb_restore();

	return 0;
}


/*
 * emgd_fb_restore
 *
 * Restore the kernel's fbcon mode.
 */
void emgd_fb_restore(void)
{
	int ret;

	EMGD_TRACE_ENTER;

	/*
	 * FIXME: Need to have the real crtc saved so it can be restored.
	 */
	if ((ret = drm_crtc_helper_set_config(&panic_mode)) != 0) {
		EMGD_ERROR("Failed to restore crtc configuration: %d", ret);
	}

	EMGD_TRACE_EXIT;
}


/* Workqueue task function; needed during CRTC init */
void emgd_flip_worker(struct work_struct *w);

/*
 * create_crtcs
 *
 * Creates crtcs. This function loops through the valid pipes, creating
 * one crtc for each pipe.  This function assumes that dsp_alloc() has
 * been run.
 *
 * TODO:  Move this to emgd_crtc.c?
 *
 * @param dev     (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return None
 */
void create_crtcs(struct drm_device *dev)
{
	emgd_crtc_t *emgd_crtc = NULL;
	drm_emgd_priv_t *dev_priv = NULL;
	igd_context_t *context = NULL;
	igd_display_pipe_t *pipe = NULL;
	int i, index = 0;
	unsigned short *r, *g, *b;

	EMGD_TRACE_ENTER;

	dev_priv = ((drm_emgd_priv_t *)dev->dev_private);
	context = dev_priv->context;

	while ((pipe = context->mod_dispatch.dsp_get_next_pipe(context, pipe, 0))) {

		/* FIXME: Why are we allocating the extra space for the
		 * connectors here? */
		emgd_crtc = kzalloc(sizeof(emgd_crtc_t) +
			(INTELFB_CONN_LIMIT * sizeof(struct drm_connector *)),
			GFP_KERNEL);

		if (emgd_crtc == NULL) {
			EMGD_ERROR("create_crtcs: Failed to allocate CRTC structure.");
			return;
		}

		spin_lock_init(&emgd_crtc->crtc_lock);

		emgd_crtc->crtc_id     = 1 << index;
		emgd_crtc->igd_pipe    = pipe;
		dev_priv->crtcs[index] = emgd_crtc;
		EMGD_DEBUG("Creating CRTC with ID: 0x%x, igd_pipe=0x%lx",
			emgd_crtc->crtc_id, (unsigned long)pipe);


		/* Hook up crtc functions */
		drm_crtc_init(dev, &emgd_crtc->base, &emgd_crtc_funcs);
		EMGD_DEBUG("  Created CRTC [%d]", emgd_crtc->base.base.id);

		/* gamma */
		drm_mode_crtc_set_gamma_size(&emgd_crtc->base, 256);

		/* Set initial gamma values */
		r = emgd_crtc->base.gamma_store;
		g = emgd_crtc->base.gamma_store + 256;
		b = emgd_crtc->base.gamma_store + 512;
		for (i = 0; i < 256; i++) {
			emgd_crtc->lut_r[i] = i;
			emgd_crtc->lut_g[i] = i;
			emgd_crtc->lut_b[i] = i;
			emgd_crtc->lut_a[i] = 0;
			r[i] = (i << 8);
			g[i] = (i << 8);
			b[i] = (i << 8);
		}

		/* Initialize workqueue task to wait for render completion on flips */
		INIT_WORK(&emgd_crtc->flip_work, emgd_flip_worker);
		emgd_crtc->flip_work_queued = 0;

		/* Target FB that we're in the process of changing to */
		emgd_crtc->newfb = NULL;

		/* Target "ops complete" watermark to allow flip to proceed */
		emgd_crtc->render_complete_at = 0;

		/*
		 * Are we expected to perform flip cleanup (sending userspace event
		 * and such) on next vblank event?
		 */
		emgd_crtc->vblank_expected = 0;

		/* Userspace 'flip done' event */
		emgd_crtc->flip_event = NULL;

		/* TODO: Create connector list */
		emgd_crtc->mode_set.crtc       = &emgd_crtc->base;
		emgd_crtc->mode_set.connectors =
								(struct drm_connector **)(emgd_crtc + 1);
		emgd_crtc->mode_set.num_connectors = 0;

		/* Hook up crtc helper functions */
		drm_crtc_helper_add(&emgd_crtc->base, &emgd_crtc_helper_funcs);
		index++;
	}

	EMGD_TRACE_EXIT;
}



/**
 * create_encoder
 *
 * Creates an encoder for the igd_port in the parameter.
 *
 * @param dev      (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param igd_port (IN) pointer to IGD display port.  (pd_driver must be valid)
 *
 * @return None
 */
static void create_encoder(struct drm_device *dev, igd_display_port_t *igd_port)
{
	emgd_encoder_t  *emgd_encoder;
	emgd_crtc_t     *emgd_crtc;
	struct drm_crtc *crtc;
	unsigned long    drm_encoder_type;


	EMGD_TRACE_ENTER;

	/* Find the corresponding DRM encoder type */
	switch(igd_port->port_type) {
		case IGD_PORT_LVDS:
			drm_encoder_type = DRM_MODE_ENCODER_LVDS;
			break;

		case IGD_PORT_DIGITAL:
			drm_encoder_type = DRM_MODE_ENCODER_TMDS;
			break;

		case IGD_PORT_ANALOG:
		case IGD_PORT_RGBA:
		case IGD_PORT_TV:
		case IGD_PORT_SDVO_ST:
		case IGD_PORT_SDVO_ST_GPIO:
		default:
			EMGD_ERROR("Invalid Port Type");
			return;
	}


	emgd_encoder = kzalloc(sizeof(emgd_encoder_t), GFP_KERNEL);
	if (!emgd_encoder) {
		EMGD_ERROR("Out of memory!");
		return;
	}


	/* What we call "TWIN" is what KMS calls "CLONE".  None of the
	 * platforms we currently support allow TWIN, so just set
	 * the bits equal to the port type since a port can always
	 * "TWIN" with itself */
    emgd_encoder->clone_mask = igd_port->port_type;
	/*
	 * crtc's are identified by a 1 that is shifted over:
	 * 11
	 * ||_ pipe 0
	 * |__ pipe 1
	 * The order of the pipes is based on how they are initialized
	 * in the drm.
	 */
	emgd_encoder->crtc_mask  = KMS_PIPE_ID(igd_port->port_features);
	emgd_encoder->igd_port   = igd_port;
	emgd_encoder->state.port = igd_port;
	emgd_encoder->flags |= ENCODER_FLAG_FIRST_DPMS | ENCODER_FLAG_FIRST_ALTER;


	/* Since we do not support TWIN and pipe assignment is fixed, the
	 * current configuration is the only possible configuration */
	emgd_encoder->base.possible_crtcs  = emgd_encoder->crtc_mask;
	emgd_encoder->base.possible_clones = emgd_encoder->clone_mask;

	/* Initialize the CRTC associated with this encoder.  We should
	 * probably use the crtc_mask here to do the matching, but we
	 * have the same info in our internal structures. */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		emgd_crtc = container_of(crtc, emgd_crtc_t, base);

		if ((emgd_crtc->igd_pipe->pipe_features & IGD_PORT_MASK) ==
			emgd_encoder->igd_port->port_type) {
			emgd_encoder->base.crtc = crtc;
			break;
		}
	}


	EMGD_DEBUG("Creating encoder=0x%lx on pipe=0x%lx, of port=0x%lx, "
		"port_type=0x%lx",
		(unsigned long)&(emgd_encoder->base), emgd_encoder->crtc_mask,
		igd_port->port_number, igd_port->port_type);

	drm_encoder_init(dev, &emgd_encoder->base, &emgd_encoder_funcs,
						drm_encoder_type);
	drm_encoder_helper_add(&emgd_encoder->base, &emgd_encoder_helper_funcs);
}



/**
 * create_connector_properties
 *
 * Creates properties associated with the input connector.  Connector properties
 * are what EMGD calls "port attributes."  The only difference is EMGD's port
 * attributes are per-encoder, not per-connector.  For this implementation, we
 * are assuming one connector per encoder.  With this assumption, we can draw
 * a direct connection between "port attributes" and "connector properties."
 *
 * @param dev            (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param emgd_connector (IN) Selected connector
 *
 * @return None
 */
static void create_connector_properties(struct drm_device *dev,
				emgd_connector_t *emgd_connector)
{
	igd_display_port_t *igd_port = emgd_connector->encoder->igd_port;
	drm_emgd_priv_t    *priv     = emgd_connector->priv;
	unsigned long       num_of_attributes = 0, num_of_properties = 0;
	unsigned long       i, current_value;
	pd_attr_t          *attributes;

	struct drm_connector *drm_connector = &emgd_connector->base;
	struct drm_property  *new_prop;


	EMGD_TRACE_ENTER;

	/* Get port attributes from the port driver */
	priv->context->dispatch.get_attrs(priv->context,
								igd_port->port_number,
								&num_of_attributes,
								&attributes);


	/* If the connector has no attributes, then return */
	if (0 >= num_of_attributes) {
		EMGD_TRACE_EXIT;
		return;
	}

	emgd_connector->properties =
		(struct drm_property **) kzalloc(sizeof(struct drm_property *) *
									num_of_attributes, GFP_KERNEL);

	if (NULL == emgd_connector->properties) {
		EMGD_ERROR_EXIT("Failed to allocate emgd_connector->properties");
	}


	/* Convert port attributes to connector properties and attach them */
	for(i = 0; i < num_of_attributes; i++) {

		/* Invisible attributes are not settable so don't even report
		 * it was a property */
		if (attributes[i].flags & PD_ATTR_FLAG_USER_INVISIBLE) {
			continue;
		}

		/* Instead of using DRM_MODE_PROP_xxxx types, it may be possible
         * to use the PD_ATTR_TYPE_xxxx types, since the 2nd parameter
		 * to drm_property_create is a flag */
		switch (attributes[i].type) {
			case PD_ATTR_TYPE_RANGE:
			{
				igd_range_attr_t *attr = (igd_range_attr_t *) &attributes[i];


				new_prop = drm_property_create(dev, DRM_MODE_PROP_RANGE,
									attr->name, 3);

				if (NULL == new_prop) {
					EMGD_ERROR("Failed to allocate new property");
					continue;
				}

				new_prop->values[0] = attr->id;
				new_prop->values[1] = attr->min;
				new_prop->values[2] = attr->max;
				current_value       = attr->current_value;

				break;
			}

			case PD_ATTR_TYPE_BOOL:
			{
				igd_bool_attr_t *attr = (igd_bool_attr_t *) &attributes[i];


				new_prop = drm_property_create(dev, DRM_MODE_PROP_RANGE,
									attr->name, 2);

				if (NULL == new_prop) {
					EMGD_ERROR("Failed to allocate new property");
					continue;
				}

				new_prop->values[0] = false;
				new_prop->values[1] = true;
				current_value       = attr->current_value;

				break;
			}

			case PD_ATTR_TYPE_LIST:
			case PD_ATTR_TYPE_LIST_ENTRY:
			case PD_ATTR_TYPE_BUFFER:
			default:
				EMGD_ERROR("Unsupported PD Attribute type");
				continue;
		}

		drm_object_attach_property(&drm_connector->base, new_prop, current_value);
		emgd_connector->properties[num_of_properties++] = new_prop;
	}

	EMGD_TRACE_EXIT;
}



/**
 * create_connectors
 *
 * Creates connectors associated with the encoder.
 *
 * This function currently supports one connector per encoder.  Further
 * development required in the future to support encoders that have more
 * than one connector.
 *
 * @param dev          (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param emgd_encoder (IN) Encoder to be examined.
 *
 * @return None
 */
static void create_connectors(struct drm_device *dev,
				emgd_encoder_t *emgd_encoder)
{
	igd_display_port_t    *port = emgd_encoder->igd_port;
	pd_driver_t           *pd   = port->pd_driver;
	unsigned long          connector_type = DRM_MODE_CONNECTOR_LVDS;
	emgd_connector_t      *emgd_connector;


	EMGD_TRACE_ENTER;

	switch (pd->type) {
		case PD_DISPLAY_LVDS_EXT:
		case PD_DISPLAY_LVDS_INT:
			connector_type = DRM_MODE_CONNECTOR_LVDS;
			break;

		case PD_DISPLAY_FP:
			connector_type = DRM_MODE_CONNECTOR_DVID;
			break;

		case PD_DISPLAY_CRT_EXT:
		case PD_DISPLAY_CRT:
			connector_type = DRM_MODE_CONNECTOR_VGA;
			break;

		case PD_DISPLAY_HDMI_INT:
			connector_type = DRM_MODE_CONNECTOR_HDMIA;
			break;

		case PD_DISPLAY_HDMI_EXT:
			connector_type = DRM_MODE_CONNECTOR_HDMIA;
			break;

		case PD_DISPLAY_DP_INT:
			connector_type = DRM_MODE_CONNECTOR_DisplayPort;
			break;

		case PD_DISPLAY_TVOUT:
		case PD_DISPLAY_TVFP:
		case PD_DISPLAY_RGBA:
		case PD_DISPLAY_TVOUT_INT:
		case PD_DISPLAY_DRGB:
			EMGD_ERROR("Unsupported connector type");
			return;

		default:
			EMGD_ERROR("Unknown connector type");
			return;
	}


	/* Allocate a new connector */
	emgd_connector = kzalloc(sizeof(emgd_connector_t), GFP_KERNEL);
	if (!emgd_connector) {
		EMGD_ERROR("Out of memory!");
		return;
	}

	drm_connector_init(dev, &emgd_connector->base, &emgd_connector_funcs,
						connector_type);

	drm_mode_connector_attach_encoder(&emgd_connector->base,
		&emgd_encoder->base);

	drm_connector_helper_add(&emgd_connector->base,
		&emgd_connector_helper_funcs);

	EMGD_DEBUG("Creating connector=0x%lx, encoder=0x%lx, type=0x%lx",
		(unsigned long)&(emgd_connector->base),
		(unsigned long)&(emgd_encoder->base), connector_type);

	emgd_connector->encoder                          = emgd_encoder;
	emgd_connector->priv                             = dev->dev_private;
	emgd_connector->base.display_info.subpixel_order = SubPixelHorizontalRGB;
	emgd_connector->base.interlace_allowed           = false;
	emgd_connector->base.doublescan_allowed          = false;
	emgd_connector->base.encoder                     = &emgd_encoder->base;

	/* Create and attach connector properties */
	create_connector_properties(dev, emgd_connector);


#if 0
        drm_object_attach_property(&connector->base,
                        dev->mode_config.scaling_mode_property,
                        DRM_MODE_SCALE_FULLSCREEN);
        drm_object_attach_property(&connector->base,
                       dev->mode_config.edid_property, 0);
        drm_object_attach_property(&connector->base,
                       dev->mode_config.dpms_property, 0);
#endif


	drm_sysfs_connector_add(&emgd_connector->base);

	EMGD_TRACE_EXIT;
}



/**
 * emgd_setup_outputs
 *
 * This function enumerates all the available outputs (physical connectors) by
 * first initializing all the encoders in the system, and then querying
 * the encoders for the connectors.
 *
 * Because we are adapting from EMGD, the real work behind detecting encoders
 * has already been done by the time we get to this function.  Therefore,
 * all we need to do is using existing EMGD HAL dispatch functions to complete
 * the task.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return
 */
void emgd_setup_outputs(struct drm_device *dev)
{
	drm_emgd_priv_t         *priv            = dev->dev_private;
	igd_context_t           *igd_context     = priv->context;
	inter_module_dispatch_t *module_dispatch = &igd_context->mod_dispatch;
	igd_display_port_t      *port            = NULL;
	struct drm_encoder      *encoder;

	EMGD_TRACE_ENTER;

	/* Loop through all available ports.  What KMS calls "encoder" is a
     * subset of what EMGD calls "port."
     */
	while ((port = module_dispatch->dsp_get_next_port(igd_context, port, 0))) {

		/* If there is a port driver, then there's an encoder */
		if (port->pd_driver) {
			create_encoder(dev, port);
		}
	}


	/* For each encoder, create the connectors on the encoder */
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		emgd_encoder_t *emgd_encoder;

		emgd_encoder = container_of(encoder, emgd_encoder_t, base);
		create_connectors(dev, emgd_encoder);
	}

	EMGD_TRACE_EXIT;
}



/**
 * emgd_modeset_init
 *
 * This is the main initialization entry point.  Called during driver load
 * and does basic setup.
 *
 * @param dev      (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param full_kms (IN) If we are running with KMS enabled, then set this
 *                      to true to do the full KMS initialization.  If not,
 *                      then set this to false.
 *
 * @return None
 */
void emgd_modeset_init(struct drm_device *dev)
{
	drm_emgd_priv_t *devpriv = (drm_emgd_priv_t *)dev->dev_private;
	int ret;
	struct drm_encoder *encoder;

	EMGD_TRACE_ENTER;
	drm_mode_config_init(dev);  /* drm helper function */

	dev->mode_config.min_width  = 0;
	dev->mode_config.max_width  = 4096;
	dev->mode_config.min_height = 0;
	dev->mode_config.max_height = 2048;
	dev->mode_config.funcs      = (void *)&emgd_mode_funcs;


	/* OTC uses dev->agp->base for fb_base */
	dev->mode_config.fb_base = pci_resource_start(dev->pdev, PSB_GATT_RESOURCE);

	/* Initialize VBLANK handling */
	dev->irq_enabled      = true;
	dev->max_vblank_count = 0xffffff; /* 24-bit frame counter */

	ret = drm_vblank_init(dev, devpriv->num_crtc);
	if (ret) {
		EMGD_ERROR("Call to drm_vblank_init() failed.  drmWaitVBlank() "
			"will not work.");
		dev->irq_enabled = false;
	}


	/* Create the crtc's */
	create_crtcs(dev);

	drm_mode_create_scaling_mode_property(dev);
	emgd_setup_outputs(dev);

	/* The encoders need to be turned off to prevent the locking of some
	   of the registers - before doing a modeset */
    list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {                  
        struct drm_encoder_helper_funcs *e_funcs = encoder->helper_private;               
        (*e_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);                                     
    }

	drm_helper_disable_unused_functions(dev);

	/* Initialize the framebuffer device */
	emgd_fbdev_init(devpriv);

	EMGD_TRACE_EXIT;
}



/**
 * emgd_modeset_destroy
 *
 * Clean up resources allocated in emgd_modeset_init.  Called during driver
 * unload
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return None
 */
void emgd_modeset_destroy(struct drm_device *dev)
{
	drm_emgd_priv_t *priv = NULL;

	EMGD_TRACE_ENTER;

	priv = dev->dev_private;
	emgd_fbdev_destroy(priv);
	kfree(priv->emgd_fbdev);
	priv->emgd_fbdev = NULL;

	EMGD_TRACE_EXIT;
}



/*
 * emgd_fb_probe
 *
 * Registers panic mode for the DRM to switch to.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return TBD
 */
RETURN_PROBE_TYPE emgd_fb_probe(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;

	/* Set Panic Mode to 1024x768 because it is an acceptably generic mode */
	//memset(&panic_mode, 0, sizeof(struct drm_mode_set));

	/* In later kernels this function changes to "output_poll_changed", as
	 * such, it is unclear if we need to implement it at all since we do not
	 * support hot-plug
	 */


	/* Register a notifier to switch back to kernel console on panic */
	atomic_notifier_chain_register(&panic_notifier_list, &paniced);

	EMGD_TRACE_EXIT;
	RETURN_PROBE;
}
EXPORT_SYMBOL(emgd_fb_probe);


/**
 * emgd_user_framebuffer_create
 *
 * Creates an instance for the framebuffer.
 *
 * @param dev      (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param filp     (IN)
 * @param mode_cmd (IN) Input from the DRM_IOCTL_MODE_ADDFB
 *
 * @return pointer to allocated FB instance
 */
static struct drm_framebuffer *emgd_user_framebuffer_create(
								struct drm_device *dev,
								struct drm_file *filp,
								struct DRM_MODE_FB_CMD_TYPE *mode_cmd)
{
	emgd_framebuffer_t *emgd_fb;
	int                 ret;

	EMGD_TRACE_ENTER;

	emgd_fb = kzalloc(sizeof(emgd_framebuffer_t), GFP_KERNEL);
	if (!emgd_fb) {
		EMGD_ERROR("Failed to allocate an emgd_framebuffer_t");
		return NULL;
	}

	/* Create a framebuffer instance */
	ret = emgd_framebuffer_init(dev, emgd_fb, mode_cmd,
			(unsigned long)mode_cmd->DRMMODE_HANDLE);
	if (ret) {
		EMGD_ERROR("Failed to create framebuffer instance.");
		/* TODO: Free the allocation at mode_cmd->handle */
		kfree(emgd_fb);
		emgd_fb = NULL;
		return NULL;
	}

	EMGD_TRACE_EXIT;
	return &emgd_fb->base;
}


/**
 * emgd_framebuffer_init
 *
 * Creates an instance for the framebuffer.
 *
 * Maps an existing surface, which has been allocated via the PVR services,
 * into the GTT to make it displayable and returns a 'drm_framebuffer'
 * struct that can be used to refer to this surface in future KMS
 * operations.  This function is called as a result of drmModeAddFB()
 * being called in userspace; the handle passed to drmModeAddFB() should
 * be the kernel meminfo handle (i.e., meminfo->hKernelMemInfo in userspace).
 *
 * @param dev      (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param emgd_fb  (IN)
 * @param mode_cmd (IN) Input from the DRM_IOCTL_MODE_ADDFB
 * @param handle   (IN) Handle of allocation.  The handle will be the
 *                      PVR meminfo handle.  The initial framebuffer,
 *                      which is allocated directly by the GMM before
 *                      the PVR services starts up is a special case;
 *                      a handle of 0 will refer to the initial framebuffer.
 *
 * @return int return value
 */
int emgd_framebuffer_init(struct drm_device *dev,
		emgd_framebuffer_t *emgd_fb,
		struct DRM_MODE_FB_CMD_TYPE *mode_cmd,
		unsigned long handle)
{
	drm_emgd_priv_t *dev_priv = NULL;
	igd_context_t *context = NULL;
	PVRSRV_PER_PROCESS_DATA *pvr_perproc;
	PVRSRV_KERNEL_MEM_INFO *pvr_meminfo;
	struct page **pagelist;
	unsigned long numpages, gtt_offset, page_offset;
	int ret, pid;

	EMGD_TRACE_ENTER;

	dev_priv = ((drm_emgd_priv_t *)dev->dev_private);
	context = dev_priv->context;

	if (handle == EMGD_INITIAL_FRAMEBUFFER) {
		/*
		 * Special case:  initial EMGD framebuffer is allocated via GTT instead
		 * of through the PVR services.  It has no PVR meminfo handle.
		 */
		ret = drm_framebuffer_init(dev, &emgd_fb->base, &emgd_fb_funcs);
		if (ret) {
		    EMGD_ERROR("Failed to create framebuffer instance.");
		    return -ENOMEM;
		}

		drm_helper_mode_fill_fb_struct(&emgd_fb->base, mode_cmd);

		/*
		 * Initial framebuffer offset is stored in the device
		 * private structure.
		 */
		emgd_fb->type       = GMM_FRAMEBUFFER;
		emgd_fb->gtt_offset = dev_priv->initfb_info.fb_base_offset;
		emgd_fb->handle     = handle;

	} else {
		/* Fetch PVR services 'per-process' data structure */
		pid = OSGetCurrentProcessIDKM();
		pvr_perproc = PVRSRVPerProcessData(pid);

		/* Look up the kernel-side meminfo for the handle passed in*/
		ret = PVRSRVLookupHandle(pvr_perproc->psHandleBase,
				(void**)&pvr_meminfo,
			(IMG_HANDLE)handle, PVRSRV_HANDLE_TYPE_MEM_INFO);
		if (ret != PVRSRV_OK) {
			EMGD_ERROR("Buffer handle is not a valid PVR surface.");
			return -EINVAL;
		}

		/*
		 * Now fetch the page list, number of pages, and offset into the first
		 * page for this buffer.
		 */
		ret = PVRSRVGetPageListKM(pvr_meminfo, &pagelist, &numpages,
				&page_offset);
		if (ret != PVRSRV_OK) {
			EMGD_ERROR("Failed to get pagelist for PVR surface.");
			return -EINVAL;
		}

		/* Map this page list into the GTT */
		ret = context->dispatch.gmm_import_pages((void**)pagelist, &gtt_offset,
			numpages);
		if (ret < 0) {
			EMGD_ERROR("Failed to map pagelist into GTT.");
			return -ENOMEM;
		}

		/* Create a framebuffer instance */
		ret = drm_framebuffer_init(dev, &emgd_fb->base, &emgd_fb_funcs);
		if (ret) {
			EMGD_ERROR("Failed to create framebuffer instance.");
			return -ENOMEM;
		}

		/* Fill the EMGD framebuffer structure */
		drm_helper_mode_fill_fb_struct(&emgd_fb->base, mode_cmd);

		emgd_fb->type = PVR_FRAMEBUFFER;
		emgd_fb->pvr_meminfo = pvr_meminfo;
		emgd_fb->pagelist = pagelist;
		emgd_fb->gtt_offset = gtt_offset + page_offset;
		emgd_fb->handle     = handle;
	}

	EMGD_TRACE_EXIT;
	return 0;
}



/*
 * emgd_user_framebuffer_destroy
 *
 * clean up and remove a framebuffer instance.
 *
 */
static void emgd_user_framebuffer_destroy (struct drm_framebuffer *fb)
{
	drm_emgd_priv_t *dev_priv = NULL;
	emgd_framebuffer_t *emgd_fb = NULL;
	igd_context_t *context = NULL;

	EMGD_TRACE_ENTER;

	dev_priv = ((drm_emgd_priv_t *)fb->dev->dev_private);
	context = dev_priv->context;
	emgd_fb = container_of(fb, emgd_framebuffer_t, base);

	/* Unmap pages from GTT */
	if (emgd_fb->type == PVR_FRAMEBUFFER) {
		/* PVR-allocated pages imported into GTT:  just unmap */
		EMGD_DEBUG("Unmapping imported PVR framebuffer pages at %lu.", emgd_fb->gtt_offset);
		context->dispatch.gmm_release_import(emgd_fb->gtt_offset);
	} else {
		/* GMM-allocated pages (initial framebuffer): unmap and free pages */
		EMGD_DEBUG("Unmapping and freeing GMM framebuffer pages at %lu.", emgd_fb->gtt_offset);
		context->dispatch.gmm_free(emgd_fb->gtt_offset);
	}

	drm_framebuffer_cleanup(fb);
	kfree(emgd_fb);

	EMGD_TRACE_EXIT;
}



/*
 * emgd_user_framebuffer_create_handle
 *
 * Returns 'a' handle associated with the drm_framebuffer given in the
 * parameter.  In our case, we return 'the' handle because our handles
 * are already cross-process compatible, and PVR is keeping track of
 * the reference count so buffers don't get destroyed unless it is not
 * used.
 *
 * @param fb (IN)        DRM framebuffer to look up
 * @param file_priv (IN) Not used
 * @param handle (OUT)   
 *
 * @return 0 on success
 * @return -EINVAL on failure
 */
static int emgd_user_framebuffer_create_handle(struct drm_framebuffer *fb,
		struct drm_file *file_priv,
		unsigned int *handle)
{
	emgd_framebuffer_t *emgd_fb;
	int ret = -EINVAL;

	EMGD_TRACE_ENTER;

	/* To avoid compiler warning. */
	(void)file_priv;

	emgd_fb = container_of(fb, emgd_framebuffer_t, base);

	if (NULL != emgd_fb) {
		*handle = emgd_fb->handle;
	}


	EMGD_TRACE_EXIT;

	return ret;
}



/**
 * emgd_fbdev_init
 *
 * Allocates and initializes a framebuffer device.  Through
 * drm_fb_helper_initial_config(), this function will set a sane mode and
 * allocate a framebuffer surface to go with the framebuffer device.
 *
 * @param priv (IN) EMGD private DRM data structure
 *
 * @return 0 on success
 */

static int emgd_fbdev_init(drm_emgd_priv_t *priv)
{
	emgd_fbdev_t *emgd_fbdev;

	EMGD_TRACE_ENTER;

	emgd_fbdev = kzalloc(sizeof(emgd_fbdev_t), GFP_KERNEL);
	if (!emgd_fbdev) {
		return -ENOMEM;
	}

	emgd_fbdev->priv         = priv;
	priv->emgd_fbdev         = emgd_fbdev;

	emgd_fbcon_initial_config(emgd_fbdev);

	EMGD_TRACE_EXIT;
	return 0;
}



/**
 * emgd_fbdev_destroy
 *
 * Cleans up resources allocated during emgd_fbdev_init.
 * Since emgd_fbdev_init() calls drm_fb_helper_initial_config() which leads
 * to a call to emgd_fb_find_or_create_single(), we are also cleaning up
 * resources allocated in that function as well.  It is unclear if this
 * clean up is necessary because KMS APIs maybe smart enough to call
 * the corresponding destory function.
 *
 * @param priv (IN) EMGD private DRM data structure
 *
 * @return None
 */
static void emgd_fbdev_destroy(drm_emgd_priv_t *priv)
{
	emgd_fbdev_t           *emgd_fbdev = NULL;
	struct drm_framebuffer *fb         = NULL;
	struct fb_info         *info       = NULL;

	EMGD_TRACE_ENTER;

	emgd_fbdev = priv->emgd_fbdev;
	fb         = &emgd_fbdev->emgd_fb->base;

	if (priv->fbdev) {
		info = priv->fbdev;

		unregister_framebuffer(info);

		if (info->cmap.len) {
			fb_dealloc_cmap(&info->cmap);
		}

		priv->context->dispatch.gmm_unmap(info->screen_base);
		framebuffer_release(info);	
	}

	drm_framebuffer_cleanup(fb); /* Is this the right place to call this? */
	priv->context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);

	kfree(emgd_fbdev->emgd_fb);
	emgd_fbdev->emgd_fb = NULL;

	EMGD_TRACE_EXIT;
}



#if 0
/*------------------------------------------------------------------------------
 * drm_fb_helper related functions.  These will be removed once we have our
 * own version of drm_fb_helper (in emgd_fbcon.c)
 *------------------------------------------------------------------------------
 */
static int emgd_fb_find_or_create_single(struct drm_fb_helper *helper,
			struct drm_fb_helper_surface_size *sizes);
static void emgd_fb_gamma_set(struct drm_crtc *crtc,
				u16 red, u16 green, u16 blue, int regno);
static void emgd_fb_gamma_get(struct drm_crtc *crtc,
				u16 *red, u16 *green, u16 *blue, int regno);



/* FIXME:  This should be removed. */
static struct drm_fb_helper_funcs emgd_fb_helper_funcs = {
	.gamma_set = emgd_fb_gamma_set,
	.gamma_get = emgd_fb_gamma_get,
	.fb_probe  = emgd_fb_find_or_create_single,
};



/**
 * emgd_fb_create
 *
 * This function creates a frame buffer using the surface information contained
 * in surface_info.  The working assumption is this function will only be
 * called once at initialization time.  So the buffer allocated here is
 * for the console.
 *
 * @param emgd_fbdev   (IN) Framebuffer devices to attach the new FB to
 * @param surface_info (IN) Information about the surface to be allocated
 */
static int emgd_fb_create(emgd_fbdev_t *emgd_fbdev,
		struct drm_fb_helper_surface_size *surface_info)
{
	drm_emgd_priv_t        *priv    = emgd_fbdev->priv;
	struct drm_device      *dev     = priv->ddev;
	struct fb_info         *info    = NULL;
	struct drm_mode_fb_cmd  mode_cmd;
	struct drm_framebuffer *fb       = NULL;
	igd_context_t          *context  = NULL;
	int                     ret;
	unsigned long           size;
	unsigned long           offset = 0;

	EMGD_TRACE_ENTER;


	context = priv->context;

	priv->initfb_info.width        = surface_info->surface_width;
	priv->initfb_info.height       = surface_info->surface_height;
	priv->initfb_info.screen_pitch = 0;
	priv->initfb_info.pixel_format = IGD_PF_ARGB32;

	ret = context->dispatch.gmm_alloc_surface(
								&priv->initfb_info.fb_base_offset,
								 priv->initfb_info.pixel_format,
								&priv->initfb_info.width,
								&priv->initfb_info.height,
								&priv->initfb_info.screen_pitch,
								&size,
								IGD_GMM_ALLOC_TYPE_RESERVATION,
								&priv->initfb_info.flags);
	if (ret) {
		EMGD_ERROR_EXIT("Allocation of buffer failed: %d", ret);
		return -ENOMEM;
	}

	priv->initfb_info.allocated = 1;
	priv->initfb_info.visible_offset = 0;

	mode_cmd.handle = EMGD_INITIAL_FRAMEBUFFER;
	mode_cmd.pitch  = priv->initfb_info.screen_pitch;
	mode_cmd.width  = surface_info->surface_width;
	mode_cmd.height = surface_info->surface_height;
	mode_cmd.bpp    = surface_info->surface_bpp;
	mode_cmd.depth  = surface_info->surface_depth;

	/* Allocate fb_info */
	info = kzalloc(sizeof(struct fb_info), GFP_KERNEL);
	if (info == NULL) {
		EMGD_ERROR_EXIT("Allocation of fb_info failed");
		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);
		return -ENOMEM;
	}
	info->par   = emgd_fbdev;

	/* Allocate emgd_framebuffer_t */
	emgd_fbdev->emgd_fb = kzalloc(sizeof(emgd_framebuffer_t), GFP_KERNEL);
	if (!emgd_fbdev->emgd_fb) {
		EMGD_ERROR_EXIT("Allocation of emgd_framebuffer_t failed");
		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);
		kfree(info);
		info = NULL;
		return -ENOMEM;
	}

	/* Initialize the framebuffer */
	ret = emgd_framebuffer_init(dev, emgd_fbdev->emgd_fb, &mode_cmd,
			EMGD_INITIAL_FRAMEBUFFER);
	if (ret) {
		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);
		kfree(info);
		info = NULL;
		kfree(emgd_fbdev->emgd_fb);
		emgd_fbdev->emgd_fb = NULL;
		return ret;
	}



	info->flags = FBINFO_DEFAULT /* | FBINFO_CAN_FORCE_OUTPUT */;
	info->fbops = (struct fb_ops*) &emgd_fb_ops;
	strcpy(info->fix.id, "emgdfb"); /* fix.id is 16 bytes long */
	fb = &emgd_fbdev->emgd_fb->base;
	emgd_fbdev->helper.fb    = fb;
	emgd_fbdev->helper.fbdev = info;
	priv->fbdev              = info;

	EMGD_DEBUG("EMGD: Call fb_alloc_cmap()");
	ret = fb_alloc_cmap(&info->cmap, 256, 0);
	if (ret) {
		EMGD_ERROR("%s: Can't allocate color map", info->fix.id);
		//mutex_unlock(&dev->struct_mutex);
		return -ENOMEM;
	}

	/*
	 * Does this need to be filled in and if so, with what?  Right now
	 * I'm trying to reuse the framebuffer that was already configured by
	 * the EMGD code.
	 *
	 * setup aperture base/size for vesafb takeover
	 */
	info->apertures = alloc_apertures(1);
	if (!info->apertures) {
		EMGD_ERROR("%s: Can't allocate apertures", info->fix.id);
		//mutex_unlock(&dev->struct_mutex);
		return -ENOMEM;
	}

	info->apertures->ranges[0].base =
		(unsigned long)priv->context->device_context.virt_gttadr;
	info->apertures->ranges[0].size =
		priv->context->device_context.gatt_pages << PAGE_SHIFT;

	//mutex_unlock(&dev->struct_mutex);
	/*
	 * FIXME: What is fix.smem_start vs screen_base?
	 *
	 * smem_start is the start of frame buffer mem (physical address), does
	 * that mean GTT or that it expects a physical contigous block in real
	 * memory?
	 *
	 * screen_base is a virtual address
	 *
	 */
	/* Set up framebuffer surface */
	EMGD_DEBUG("EMGD: Call pci_resource_start()");
	info->fix.smem_start =  pci_resource_start(dev->pdev, PSB_GATT_RESOURCE) +
		priv->initfb_info.fb_base_offset;
	info->fix.smem_len = size;

	/* Get kernel virtual memory address of framebuffer */
	EMGD_DEBUG("EMGD: Call gmm_map()");
	info->screen_base =
		priv->context->dispatch.gmm_map(priv->initfb_info.fb_base_offset);
	if (!info->screen_base) {
		EMGD_ERROR("%s: Can't map framebuffer surface", info->fix.id);
		return -ENOSPC;
	}
	info->screen_size = size;

	info->pixmap.size         = 64 * 1024;
	info->pixmap.buf_align    = 8;
	info->pixmap.access_align = 32;
	info->pixmap.flags        = FB_PIXMAP_SYSTEM;
	info->pixmap.scan_align   = 1;

	EMGD_DEBUG("Frame buffer %dx%d @ 0x%08lx",
			fb->width, fb->height, offset);

	EMGD_DEBUG("EMGD: Call vga_switcheroo_client_fb_set()");
	vga_switcheroo_client_fb_set(dev->pdev, info);

	EMGD_TRACE_EXIT;
	return 0;
}



/*
 * emgd_fb_gamma_set
 *
 * Set the gamma values for a framebuffer for a particular
 * index.
 *
 * @param crtc     (IN) the drm pipe
 * @param red      (IN) red gamma value
 * @param green    (IN) green gamma value
 * @param blue     (IN) blue gamma value
 * @param regno    (IN) index value which we are setting
 *
 * @return None
 */
static void emgd_fb_gamma_set(struct drm_crtc *crtc,
		u16 red, u16 green, u16 blue,
		int regno)
{
	emgd_crtc_t *emgd_crtc = container_of(crtc, emgd_crtc_t, base);

	EMGD_TRACE_ENTER;
	emgd_crtc->lut_r[regno] = red >> 8;
	emgd_crtc->lut_g[regno] = green >> 8;
	emgd_crtc->lut_b[regno] = blue >> 8;
	EMGD_TRACE_EXIT;
}


/*
 * emgd_fb_gamma_get
 *
 * Set the gamma values for a framebuffer for a particular
 * index.
 *
 * @param crtc     (IN) the drm pipe
 * @param red      (OUT) red gamma value
 * @param green    (OUT) green gamma value
 * @param blue     (OUT) blue gamma value
 * @param regno    (IN) index value which we are getting
 *
 * @return None
 */
static void emgd_fb_gamma_get(struct drm_crtc *crtc,
		u16 *red, u16 *green, u16 *blue,
		int regno)
{
	emgd_crtc_t *emgd_crtc = container_of(crtc, emgd_crtc_t, base);

	EMGD_TRACE_ENTER;
	*red = (emgd_crtc->lut_r[regno] << 8);
	*blue = (emgd_crtc->lut_b[regno] << 8);
	*green = (emgd_crtc->lut_g[regno] << 8);
	EMGD_TRACE_EXIT;
}



/**
 * emgd_fb_find_or_create_single
 *
 * If there is no frame buffer associated with "helper", then allocate a new
 * one.  Otherwise, reuse the existing one.
 *
 * @param helper       (IN) TBD
 * @param surface_info (IN) Surface info
 *
 * @return None
 */
static int emgd_fb_find_or_create_single(struct drm_fb_helper *helper,
			struct drm_fb_helper_surface_size *surface_info)
{
	emgd_fbdev_t *fbdev  = container_of(helper, emgd_fbdev_t, helper);
	int           new_fb = false;


	EMGD_TRACE_ENTER;

	if (!helper->fb) {
		int ret = emgd_fb_create(fbdev, surface_info);
		if (ret) {
			return ret;
		}

		new_fb = true;
	}

	EMGD_TRACE_EXIT;

	return new_fb;
}

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_connector.c
 * $Revision: 1.3 $
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
 *  Connector / kenrel mode setting functions.
 *-----------------------------------------------------------------------------
 */
#define MODULE_NAME hal.oal


#include <drmP.h>
#include <drm_crtc_helper.h>
#include <linux/version.h>


#include "drm_emgd_private.h"



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
static enum drm_connector_status emgd_connector_detect(
									struct drm_connector *connector,
									bool force);
#else
static enum drm_connector_status emgd_connector_detect(
									struct drm_connector *connector);
#endif

static int  emgd_connector_set_property(struct drm_connector *connector,
				struct drm_property *property,
	   			uint64_t value);
static void emgd_connector_destroy(struct drm_connector *connector);
static int  emgd_connector_get_modes(struct drm_connector *connector);
static int  emgd_connector_mode_valid(struct drm_connector *connector,
				struct drm_display_mode *mode);
static struct drm_encoder *emgd_connector_best_encoder(
								struct drm_connector *connector);
static void emgd_connector_save (struct drm_connector *connector);
static void emgd_connector_restore (struct drm_connector *connector);



const struct drm_connector_funcs emgd_connector_funcs = {
	.save         = emgd_connector_save,
	.restore      = emgd_connector_restore,
	.dpms         = drm_helper_connector_dpms,
	.detect       = emgd_connector_detect,
	.fill_modes   = drm_helper_probe_single_connector_modes,
	.set_property = emgd_connector_set_property,
	.destroy      = emgd_connector_destroy,
};

const struct drm_connector_helper_funcs emgd_connector_helper_funcs = {
	.get_modes    = emgd_connector_get_modes,
	.mode_valid   = emgd_connector_mode_valid,
	.best_encoder = emgd_connector_best_encoder,
};



/**
 * emgd_mode_to_kms
 *
 * Converts an EMGD mode to a DRM KMS mode
 *
 * @param emgd_mode (IN)  emgd_mode timing information
 * @param drm_mode  (OUT) DRM mode
 */
static void emgd_mode_to_kms(igd_display_info_t *emgd_mode,
				struct drm_display_mode *drm_mode)
{
	drm_mode->status      = MODE_OK;
	drm_mode->type        = DRM_MODE_TYPE_DRIVER;
	drm_mode->clock       = emgd_mode->dclk;
	drm_mode->hdisplay    = emgd_mode->width;
	drm_mode->hsync_start = emgd_mode->hsync_start;
	drm_mode->hsync_end   = emgd_mode->hsync_end;
	drm_mode->htotal      = emgd_mode->htotal;
	drm_mode->vdisplay    = emgd_mode->height;
	drm_mode->vsync_start = emgd_mode->vsync_start;
	drm_mode->vsync_end   = emgd_mode->vsync_end;
	drm_mode->vtotal      = emgd_mode->vtotal;
	drm_mode->flags       = emgd_mode->flags;
	drm_mode->vrefresh    = emgd_mode->refresh;

	drm_mode_set_name(drm_mode);
}



/**
 * emgd_connector_save
 *
 * Saves a connector state before a power event.
 *
 * @param encoder (IN) Encoder
 * @param mode    (IN) power mode
 *
 * @return None
 */
static void emgd_connector_save (struct drm_connector *connector)
{
#if 0
	emgd_connector_t   *emgd_connector;
	emgd_encoder_t     *emgd_encoder;
	igd_display_port_t *igd_port;


	/* Currently there is no need to do anything in this function because
	 * CRTC save calls igd_driver_save() which eventually calls mode_save().
	 * Inside of mode_save() is where encoder state is saved.  In order for
	 * this function to work properly, we need to take the pd_save() call
	 * out of mode_save().  Let's wait until Phase 2 to do this.
	 */

	EMGD_TRACE_ENTER;

	emgd_connector = container_of(connector, emgd_connector_t, base);
	emgd_encoder   = emgd_connector->encoder;
	igd_port       = emgd_encoder->igd_port;

	igd_port->pd_driver->pd_save(igd_port->pd_context,
							&emgd_encoder->state.state, 0);

	EMGD_TRACE_EXIT;
#else
	EMGD_TRACE_ENTER;
	EMGD_TRACE_EXIT;
#endif
}



/**
 * emgd_connector_restore
 *
 * Restores a connector state after a power event
 *
 * @param encoder (IN) Encoder
 * @param mode    (IN) power mode
 *
 * @return None
 */
static void emgd_connector_restore (struct drm_connector *connector)
{
#if 0
	emgd_connector_t   *emgd_connector;
	emgd_encoder_t     *emgd_encoder;
	igd_display_port_t *igd_port;


	/* Currently there is no need to do anything in this function because
	 * CRTC save calls igd_driver_save() which eventually calls mode_save().
	 * Inside of mode_save() is where encoder state is saved.  In order for
	 * this function to work properly, we need to take the pd_save() call
	 * out of mode_save().  Let's wait until Phase 2 to do this.
	 */

	EMGD_TRACE_ENTER;

	emgd_connector = container_of(connector, emgd_connector_t, base);
	emgd_encoder   = emgd_connector->encoder;
	igd_port       = emgd_encoder->igd_port;

	igd_port->pd_driver->pd_restore(igd_port->pd_context,
							emgd_encoder->state.state, 0);
	emgd_encoder->state.state = NULL;

	EMGD_TRACE_EXIT;
#else
	EMGD_TRACE_ENTER;
	EMGD_TRACE_EXIT;
#endif
}



/**
 * emgd_connector_detect
 *
 * Checks to see if a display device is attached to the connector.  EMGD
 * does not currently support hot-plug.
 *
 * FIXME: The prototype for this function seemed to change sometime
 * around the 2.6.35 timeframe however, different distributions
 * cherrypicked it earlier.
 *
 * Fedora 14's 2.6.35.11 kernel has the patch (needs bool force)
 * MeeGo's 2.6.35.10 kernel doesn't.
 *
 * @param encoder (IN) Encoder
 * @param mode    (IN) power mode
 *
 * @return None
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)

static enum drm_connector_status emgd_connector_detect(
		struct drm_connector *connector, bool force) {
#else
static enum drm_connector_status emgd_connector_detect(
		struct drm_connector *connector) {
#endif
	emgd_connector_t          *emgd_connector;
	enum drm_connector_status  connector_status;
	igd_display_port_t        *igd_port;
	pd_port_status_t           port_status;

	EMGD_TRACE_ENTER;

	emgd_connector = container_of(connector, emgd_connector_t, base);
	igd_port       = emgd_connector->encoder->igd_port;

	memset(&port_status, 0, sizeof(pd_port_status_t));

	/* Get current status from the port driver */
	igd_port->pd_driver->pd_get_port_status(igd_port->pd_context, &port_status);

	switch (port_status.connected) {
		case PD_DISP_STATUS_ATTACHED:
			connector_status = connector_status_connected;
			break;

		case PD_DISP_STATUS_DETACHED:
			connector_status = connector_status_disconnected;
			break;

		case PD_DISP_STATUS_UNKNOWN:
		default:
			/*
             * Technically "unknown" is correct here, but that isn't actually
			 * what we want to pass back to userspace via KMS.  LVDS panels
			 * always have unknown connection status, so they'll always be
			 * ignored by userspace apps that only operate on connected outputs.
			 * If the driver is configured to use a port, then we should just
			 * assume that its actually connected when we report back to
			 * userspace.
			 */

			connector_status = connector_status_connected;
			break;
	}


	EMGD_TRACE_EXIT;

	return connector_status;
}



/**
 * emgd_connector_set_property
 *
 * Sets a port attribute.
 *
 * @param connector (IN) connector
 * @param property  (IN)
 * @param value     (IN)
 *
 * @return TBD
 */
static int emgd_connector_set_property(struct drm_connector *connector,
			struct drm_property *property,
			uint64_t value)
{
	emgd_connector_t   *emgd_connector;
	drm_emgd_priv_t    *priv;
	int                 ret;
	igd_attr_t		   *attributes, selected_attr;
	unsigned short      port_number;
	unsigned long       num_of_attributes, i;


	EMGD_TRACE_ENTER;

	/* Set the property value to the new one.  This doesn't actually change
     * anything on the HW. */
	ret = drm_connector_property_set_value(connector, property, value);
	if (ret) {
		return ret;
	}

	/* Take care of the HW changes associated with the value change */
	emgd_connector = container_of(connector, emgd_connector_t, base);
	port_number    = emgd_connector->encoder->igd_port->port_number;
	priv           = emgd_connector->priv;

	/* Search port attributes to find the one associated with the property */
	ret = priv->context->dispatch.get_attrs(priv->context,
									port_number,
									&num_of_attributes,
									&attributes);

	if (ret) {
		return ret;
	}


	for (i = 0; i < num_of_attributes; i++) {

		if (attributes[i].id == property->values[0]) {
			/* Copy the content of the attribute */
			memcpy(&selected_attr, &attributes[i], sizeof(igd_attr_t));

			switch (selected_attr.type) {
				case PD_ATTR_TYPE_RANGE:
				case PD_ATTR_TYPE_BOOL:
					selected_attr.current_value = (unsigned long) value;
					ret = priv->context->dispatch.set_attrs(priv->context,
													port_number,
													1, /* Setting 1 attribute */
													&selected_attr);
					break;

				case PD_ATTR_TYPE_LIST:
				case PD_ATTR_TYPE_LIST_ENTRY:
				case PD_ATTR_TYPE_BUFFER:
				default:
					EMGD_DEBUG("Unsupported PD Attribute type");
					continue;
			}

			break;
		}
	}

	EMGD_TRACE_EXIT;

	return ret;
}



/**
 * emgd_connector_destroy
 *
 * Cleans up the emgd_connector object.
 *
 * @param connector (IN) connector to clean up
 *
 * @return None
 */
static void emgd_connector_destroy(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	emgd_connector_t  *emgd_connector;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("connector=0x%lx",(unsigned long)connector);

	emgd_connector = container_of(connector, emgd_connector_t, base);

	drm_sysfs_connector_remove(connector);
	drm_connector_cleanup(connector);

	/* Remove the properties */
	if (emgd_connector->properties) {
		int i;

		for(i = 0; i < emgd_connector->num_of_properties; i++) {
			drm_property_destroy(dev, emgd_connector->properties[i]);
		}

		kfree(emgd_connector->properties);
		emgd_connector->properties = NULL;
	}

	kfree(emgd_connector);

	EMGD_TRACE_EXIT;
}



/**
 * emgd_connector_get_modes
 *
 * Get the list of supported modes for the given connector
 *
 * @param connector (IN) connector to query.
 *
 * @return None
 */
static int emgd_connector_get_modes(struct drm_connector *connector)
{
        emgd_connector_t        *emgd_connector;
        igd_display_port_t      *igd_port;
        unsigned long            i;
        struct drm_display_mode *drm_mode;


        EMGD_TRACE_ENTER;

        emgd_connector = container_of(connector, emgd_connector_t, base);
        igd_port       = emgd_connector->encoder->igd_port;

        EMGD_DEBUG("[EMGD] emgd_connector_get_modes for port %ld\n",
                igd_port->port_number);

        for (i = 0; i < igd_port->num_timing; i++) {
                drm_mode = drm_mode_create(emgd_connector->priv->ddev);

                emgd_mode_to_kms((igd_display_info_t *)&igd_port->timing_table[i],
                        drm_mode);

                /* Add current mode to the connector */
                drm_mode_probed_add(connector, drm_mode);
        }


        EMGD_TRACE_EXIT;

        return igd_port->num_timing;
}



/**
 * emgd_connector_mode_valid
 *
 * Examines the mode given and see if the connector can support it.
 * Note:  the ModeStatus enum is defined in xorg/hw/xfree86/common/xf86str.h
 *
 * @param connector (IN) the connector to be analyzed.
 * @param mode      (IN) mode to check
 *
 * @return MODE_OK if supported, other ModeStatus enum if not
 */
static int emgd_connector_mode_valid(struct drm_connector *connector,
                struct drm_display_mode *mode)
{
        emgd_connector_t *emgd_connector;

        EMGD_TRACE_ENTER;

        emgd_connector = container_of(connector, emgd_connector_t, base);


        /* Basic checks */
        if ((0 >= mode->hdisplay) || (mode->hdisplay > mode->hsync_start) ||
                (mode->hsync_start >= mode->hsync_end) ||
                (mode->hsync_end >= mode->htotal)) {
                return MODE_H_ILLEGAL;
        }

        if ((0 >= mode->vdisplay) || (mode->vdisplay > mode->vsync_start) ||
                (mode->vsync_start >= mode->vsync_end) ||
                (mode->vsync_end >= mode->vtotal)) {
                return MODE_V_ILLEGAL;
        }

		/* We should reject modes that are already rejected by the Port driver */
		if (!(mode->flags & IGD_MODE_SUPPORTED)) {
			return MODE_BAD;
		}

        /* We can do some basic checks here, but it is better to call
     * match_mode() in the display/mode module because that function does
     * a much more thorough check */



        EMGD_TRACE_EXIT;

        return MODE_OK;
}



/**
 * emgd_connector_best_encoder
 *
 * Returns the best encoder for the given connector.  In EMGD the connector is
 * fixed to the encoder.
 *
 * @param connector (IN) the connector to be analyzed.
 *
 * @return Encoder onto which the connector is fixed on.
 */
static struct drm_encoder *emgd_connector_best_encoder(
                struct drm_connector *connector)
{
        emgd_connector_t *emgd_connector;

        EMGD_TRACE_ENTER;

        emgd_connector = container_of(connector, emgd_connector_t, base);

        EMGD_TRACE_EXIT;

        return &emgd_connector->encoder->base;
}

/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_fbcon.c
 * $Revision: 1.5 $
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
 *  Framebuffer Console related functions.  This is the equivalent of
 *  what is in drm_fb_helper.c, a set of functions for configuring the
 *  framebuffer console. 
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.oal

#include <drmP.h>
#include <drm_crtc_helper.h>
#include <linux/vga_switcheroo.h>


#include "user_config.h"
#include "drm_emgd_private.h"
#include "mode_dispatch.h"
#include "emgd_drm.h"
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/export.h>
#endif


/*------------------------------------------------------------------------------
 * Global Variables
 *------------------------------------------------------------------------------
 */
extern emgd_drm_config_t config_drm;



/*------------------------------------------------------------------------------
 * Formal Declaration
 *------------------------------------------------------------------------------
 */
extern int emgd_framebuffer_init(struct drm_device *dev,
			emgd_framebuffer_t *emgd_fb,
			struct DRM_MODE_FB_CMD_TYPE *mode_cmd,
			unsigned long handle);
extern int convert_bpp_depth_to_drm_pixel_formal(unsigned int bpp,
        unsigned int depth);


/* Sets up initial display configuration */
int  emgd_fbcon_initial_config(emgd_fbdev_t *emgd_fbdev);


/*------------------------------------------------------------------------------
 * These are called by the framebuffer console
 *------------------------------------------------------------------------------
 */
static int  alloc_initial_fb(emgd_fbdev_t *emgd_fbdev);
static void fill_fix(emgd_fbdev_t *emgd_fbdev, struct fb_info *info);
static void fill_var(emgd_fbdev_t *emgd_fbdev, struct fb_info *info);



/*------------------------------------------------------------------------------
 * FBCON Functions
 * These are called by the framebuffer console
 *------------------------------------------------------------------------------
 */
static int emgd_fbcon_setcolreg(unsigned int regno,
			unsigned int red, unsigned int green, unsigned int blue,
			unsigned int transp, struct fb_info *info);
static int emgd_fbcon_pan_display(struct fb_var_screeninfo *var,
			struct fb_info *info);
static int emgd_fbcon_set_par(struct fb_info *info);
static int emgd_fbcon_check_var(struct fb_var_screeninfo *var,
			struct fb_info *info);
static int emgd_fbcon_blank(int blank, struct fb_info *info);
static int emgd_fbcon_setcmap(struct fb_cmap *cmap, struct fb_info *info);


/* This is called from within FBCON, the framebuffer console */
const struct fb_ops emgd_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = emgd_fbcon_check_var,
	.fb_set_par     = emgd_fbcon_set_par,
	.fb_setcolreg   = emgd_fbcon_setcolreg,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_pan_display = emgd_fbcon_pan_display,
	.fb_blank       = emgd_fbcon_blank,
	.fb_setcmap     = emgd_fbcon_setcmap,
	/* .fb_debug_enter = drm_fb_helper_debug_enter, */
	/* .fb_debug_leave = drm_fb_helper_debug_leave, */
};



static void fill_fix(emgd_fbdev_t *emgd_fbdev, struct fb_info *info)
{
	struct drm_framebuffer *fb = &emgd_fbdev->emgd_fb->base;

	info->fix.type        = FB_TYPE_PACKED_PIXELS;
	info->fix.visual      = FB_VISUAL_TRUECOLOR;
	info->fix.mmio_start  = 0;
	info->fix.mmio_len    = 0;
	info->fix.type_aux    = 0;
	info->fix.xpanstep    = 1; /* doing it in hw */
	info->fix.ypanstep    = 1; /* doing it in hw */
	info->fix.ywrapstep   = 0;
	info->fix.accel       = FB_ACCEL_NONE;
	info->fix.type_aux    = 0;
	info->fix.line_length = fb->DRMFB_PITCH;
}



/**
 * fill_var
 *
 * Fills in the fb_info structure.  This function is called by alloc_init_fb,
 * and as such an actual mode would not have been set yet.  This means we
 * don't really know what "var.xres" and "var.yres" will be, so we have to
 * make the assumption that "config_drm->width" and "config_drm->height"
 * sepcify the resolution for the eventual mode.
 *
 * @param emgd_fbdev [IN]    FB device that contains the relevant information
 * @param info       [INOUT] fb_info structure to fill in
 *
 * @return 0 on success, an error code otherwise
 */
static void fill_var(emgd_fbdev_t *emgd_fbdev, struct fb_info *info)
{
	struct drm_framebuffer *fb = &emgd_fbdev->emgd_fb->base;


	/* Actual resolution for the mode.  We are assuming here that the
	 * mode requested through config_drm->width and config_drm->height can
	 * be set successfully */
	info->var.xres           = config_drm.width;
	info->var.yres           = config_drm.height;

	/* Size of the framebuffer */
	info->var.xres_virtual   = fb->width;
	info->var.yres_virtual   = fb->height;

	info->pseudo_palette     = emgd_fbdev->pseudo_palette;
	info->var.bits_per_pixel = fb->bits_per_pixel;
	info->var.accel_flags    = FB_ACCELF_TEXT;
	info->var.xoffset        = 0;
	info->var.yoffset        = 0;
	info->var.activate       = FB_ACTIVATE_NOW;
	info->var.height         = -1;
	info->var.width          = -1;

	switch (fb->depth) {
	case 8:
		info->var.red.offset = 0;
		info->var.green.offset = 0;
		info->var.blue.offset = 0;
		info->var.red.length = 8; /* 8bit DAC */
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 15:
		info->var.red.offset = 10;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 5;
		info->var.blue.length = 5;
		info->var.transp.offset = 15;
		info->var.transp.length = 1;
		break;
	case 16:
		info->var.red.offset = 11;
		info->var.green.offset = 5;
		info->var.blue.offset = 0;
		info->var.red.length = 5;
		info->var.green.length = 6;
		info->var.blue.length = 5;
		info->var.transp.offset = 0;
		break;
	case 24:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 0;
		info->var.transp.length = 0;
		break;
	case 32:
		info->var.red.offset = 16;
		info->var.green.offset = 8;
		info->var.blue.offset = 0;
		info->var.red.length = 8;
		info->var.green.length = 8;
		info->var.blue.length = 8;
		info->var.transp.offset = 24;
		info->var.transp.length = 8;
		break;
	default:
		break;
	}
}



/**
 * alloc_initial_fb
 *
 * This function creates a frame buffer using the config_drm.width and
 * config_drm.height.  The working assumption is this function will only be
 * called once at initialization time.  So the buffer allocated here is
 * for the console.
 * FIXME:  need to make sure resources are freed properly for all the
 *         failed cases.
 *
 * @param emgd_fbdev [IN] Framebuffer device to allocate a buffer for.
 *
 * @return 0 on success, an error code otherwise
 */
static int alloc_initial_fb(emgd_fbdev_t *emgd_fbdev)
{
	struct fb_info         *info     = NULL;
	struct drm_device      *dev      = emgd_fbdev->priv->ddev;
	drm_emgd_priv_t        *priv     = emgd_fbdev->priv;
	igd_context_t          *context;
	int                     ret;
	unsigned long           size;
	struct DRM_MODE_FB_CMD_TYPE mode_cmd;
	unsigned int bpp, depth;

	EMGD_TRACE_ENTER;


	context = priv->context;

	memset(&priv->initfb_info, 0, sizeof(priv->initfb_info));
   	priv->initfb_info.width        = config_drm.width;
	priv->initfb_info.height       = config_drm.height;
	priv->initfb_info.pixel_format = IGD_PF_ARGB32;
	bpp = IGD_PF_BPP(priv->initfb_info.pixel_format);
	depth = bpp;

	/* The initial framebuffer is a displayable surface. */
	priv->initfb_info.flags = (priv->initfb_info.flags & IGD_FB_FLAGS_MASK) |
                							IGD_SURFACE_DISPLAY;

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

	priv->initfb_info.allocated      = 1;
	priv->initfb_info.visible_offset = 0;


	/* Allocate emgd_framebuffer_t */
	emgd_fbdev->emgd_fb = kzalloc(sizeof(emgd_framebuffer_t), GFP_KERNEL);

	if (!emgd_fbdev->emgd_fb) {
		EMGD_ERROR_EXIT("Allocation of emgd_framebuffer_t failed");
		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);
		return -ENOMEM;
	}


	/* Initialize emgd_framebuffer_t */
	mode_cmd.DRMMODE_HANDLE = EMGD_INITIAL_FRAMEBUFFER;
	mode_cmd.DRMFB_PITCH  = priv->initfb_info.screen_pitch;
	mode_cmd.width  = priv->initfb_info.width;
	mode_cmd.height = priv->initfb_info.height;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
    mode_cmd.pixel_format = convert_bpp_depth_to_drm_pixel_formal(bpp, depth);
#else
	mode_cmd.bpp    = bpp;
    mode_cmd.depth  = depth;
#endif

	ret = emgd_framebuffer_init(dev, emgd_fbdev->emgd_fb, &mode_cmd,
			EMGD_INITIAL_FRAMEBUFFER);
	if (ret) {
		kfree(emgd_fbdev->emgd_fb);
		emgd_fbdev->emgd_fb = NULL;

		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);

		return ret;
	}


	/* Allocate fb_info */
	info = kzalloc(sizeof(struct fb_info), GFP_KERNEL);
	if (NULL == info) {
		EMGD_ERROR_EXIT("Allocation of fb_info failed");
		drm_framebuffer_cleanup(&emgd_fbdev->emgd_fb->base);

		kfree(emgd_fbdev->emgd_fb);
		emgd_fbdev->emgd_fb = NULL;

		context->dispatch.gmm_free(priv->initfb_info.fb_base_offset);

		return -ENOMEM;
	}


	info->par   = emgd_fbdev; /* Private data for all FBCON functions  */
	info->flags = FBINFO_DEFAULT; /* | FBINFO_CAN_FORCE_OUTPUT */
	info->fbops = (struct fb_ops*) &emgd_fb_ops;
	strcpy(info->fix.id, "emgdfb"); /* fix.id is 16 bytes long */

	priv->fbdev = info;

	EMGD_DEBUG("EMGD: Call fb_alloc_cmap()");
	ret = fb_alloc_cmap(&info->cmap, 256, 0);
	if (ret) {
		EMGD_ERROR("%s: Can't allocate color map", info->fix.id);

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
		return -ENOMEM;
	}

	info->apertures->ranges[0].base =
		(unsigned long)context->device_context.virt_gttadr;
	info->apertures->ranges[0].size =
		context->device_context.gatt_pages << PAGE_SHIFT;


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
	info->fix.smem_start = pci_resource_start(dev->pdev, PSB_GATT_RESOURCE) +
						   priv->initfb_info.fb_base_offset;
	info->fix.smem_len   = size;

	/* Get kernel virtual memory address of framebuffer */
	EMGD_DEBUG("EMGD: Call gmm_map()");
	info->screen_base =
			context->dispatch.gmm_map(priv->initfb_info.fb_base_offset);
	if (!info->screen_base) {
		EMGD_ERROR("%s: Can't map framebuffer surface", info->fix.id);
		return -ENOSPC;
	}

	info->screen_size         = size;
	info->pixmap.size         = 64 * 1024;
	info->pixmap.buf_align    = 8;
	info->pixmap.access_align = 32;
	info->pixmap.flags        = FB_PIXMAP_SYSTEM;
	info->pixmap.scan_align   = 1;

	/* Initialize info->fix and info->var */
	fill_fix(emgd_fbdev, info);
	fill_var(emgd_fbdev, info);

	EMGD_DEBUG("Frame buffer %dx%d @ 0x%08lx",
			priv->initfb_info.width,
			priv->initfb_info.height, priv->initfb_info.fb_base_offset);

	EMGD_DEBUG("EMGD: Call vga_switcheroo_client_fb_set()");
	vga_switcheroo_client_fb_set(dev->pdev, info);

	EMGD_TRACE_EXIT;
	return 0;
}



/**
 * emgd_fbcon_setcmap
 *
 * Sets color map for the framebuffer console device.  For now, we will set
 * both CRTC to the same color map, regardless of which display configuration
 * we are in.  There may be a case in the future where we will have to set
 * the color map for both CRTCs differently.
 *
 * We will also assume that we are dealing with FB_VISUAL_TRUECOLOR because
 * our alloc_initial_fb() function will only allocate framebuffer of this
 * type.
 *
 * @param cmap [IN] Input color map
 * @param info [IN] framebuffer to set
 *
 * @return 0 on success
 */
static int emgd_fbcon_setcmap(struct fb_cmap *cmap, struct fb_info *info)
{
	emgd_fbdev_t      *emgd_fbdev = info->par;
	struct drm_device *dev        = emgd_fbdev->priv->ddev;
	int                ret = 0;
	u16               *red, *green, *blue, *transp;
	u16                hred, hgreen, hblue, htransp;
	u32                new_value, mask;
	int                i, start_index;
	struct drm_crtc   *crtc;


	EMGD_TRACE_ENTER;


	/* Set all the CRTCs to the same color map */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		red         = cmap->red;
		green       = cmap->green;
		blue        = cmap->blue;
		transp      = cmap->transp;
		start_index = cmap->start;

		for(i = 0; i < cmap->len; i++) {
			htransp = 0xffff;
			hred    = *red++;
			hgreen  = *green++;
			hblue   = *blue++;

			if (transp) {
				htransp = *transp++;
			}

			/* The palette only has 17 entries */
			if (16 < start_index ) {
				ret = -IGD_ERROR_INVAL_PARAM;
				break;
			}

			hred   >>= (16 - info->var.red.length);
			hgreen >>= (16 - info->var.green.length);
			hblue  >>= (16 - info->var.blue.length);

			new_value = (hred   << info->var.red.offset)   |
						(hgreen << info->var.green.offset) |
						(hblue  << info->var.blue.offset);

			if (info->var.transp.length > 0) {
				mask = (1 << info->var.transp.length) - 1;
				mask <<= info->var.transp.offset;
				new_value |= mask;
			}

			((u32 *) info->pseudo_palette)[start_index] = new_value;

			start_index++;
		}

		if (ret) {
			EMGD_ERROR("Invalid parameter.");
			break;
		}

		((struct drm_crtc_helper_funcs *)crtc->helper_private)->load_lut(crtc);
	}	


	EMGD_TRACE_EXIT;

	return ret;
}



/*
 * Currently, we are using the device indpendent DRM functions
 * for these.  If we need to do special processing, then uncomment
 * these functions and hook them into the function table.
 */
static int emgd_fbcon_check_var(struct fb_var_screeninfo *var,
		struct fb_info *info)
{
	int ret = 0;

	EMGD_TRACE_ENTER;



	EMGD_TRACE_EXIT;

	return ret;
}



/*
 * FIXME: Should this fbcon function be implemented this way?
 * The logic in this function needs to be there in some fbcon function;
 * through debugging, it was noticed that this particular function and
 * fb_pan_display are called whenever we do a VT switch (away from Weston).
 * This function was chosen arbitrarily to flip to the initial framebuffer.
 */
static int emgd_fbcon_set_par(struct fb_info *info)
{
	int ret = 0;
	struct drm_device *dev;
	emgd_fbdev_t *emgd_fbdev;
	emgd_crtc_t *emgd_crtc;
	igd_context_t *context;
	drm_emgd_priv_t *priv;
	igd_display_pipe_t *pipe;
	igd_surface_t surface = {0};

	EMGD_TRACE_ENTER;

	emgd_fbdev = info->par;
	priv	   = emgd_fbdev->priv;
	dev		   = priv->ddev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	/* Determine the active crtc by looking for a non-NULL display context.
	 * We might have to modify this logic if DC is not 1 (that is for single
	 * and extended modes, which we do not fully support at the kms level at
	 * this moment)
	 */
	if (priv->crtcs[0]->igd_pipe->owner) {
		emgd_crtc = priv->crtcs[0];
	} else {
		emgd_crtc = priv->crtcs[1];
	}

	pipe = emgd_crtc->igd_pipe;

	/* During the initial modeset, when this function is called, it was observed
	 * the pipe timing and plane were NULL; therefore, this guard here is necesary.
     */
	if(!pipe->timing || !pipe->plane) {
		return ret;
	}

	/* This logic is very similar to what is done in the flip worker; all that
     * we are doing here is re-attaching the initial framebuffer to the crtc
     * and flipping it onto the screen using set_surface.
     * FIXME: Should we also do a modeset to be safe?
     */
	emgd_crtc->newfb = emgd_fbdev->emgd_fb;

	surface.flags        = IGD_SURFACE_DISPLAY;
    surface.offset       = emgd_crtc->newfb->gtt_offset;
    surface.pitch        = emgd_crtc->newfb->base.DRMFB_PITCH;
    surface.width        = emgd_crtc->newfb->base.width;
    surface.height       = emgd_crtc->newfb->base.height;
    surface.pixel_format = IGD_PF_ARGB32;

    ret = context->dispatch.set_surface(pipe->owner, IGD_PRIORITY_NORMAL,
						IGD_BUFFER_DISPLAY, &surface, NULL, 0);
	// set mode here:  crtc->set_config
    // look at drm_fb_helper_set_par for reference


	EMGD_TRACE_EXIT;

	return ret;
}



static int emgd_fbcon_setcolreg(unsigned int regno,
		unsigned int red, unsigned int green, unsigned int blue,
		unsigned int transp, struct fb_info *info)
{
	EMGD_TRACE_ENTER;

	EMGD_DEBUG("STUBED emgd_fbcon_setcolreg");

	EMGD_TRACE_EXIT;
	return 0;
}



static int emgd_fbcon_pan_display(struct fb_var_screeninfo *var,
		struct fb_info *info)
{
	int ret = 0;

	EMGD_TRACE_ENTER;



	EMGD_TRACE_EXIT;
	return ret;
}



/*
 * Currently, we are using the device indpendent DRM functions
 * for these.  If we need to do special processing, then uncomment
 * these functions and hook them into the function table.
 */
static int emgd_fbcon_blank(int blank, struct fb_info *info)
{
	int ret = 0;
	struct drm_device *dev;
	struct drm_crtc *crtc = NULL;
	struct drm_encoder *encoder = NULL;
	struct drm_encoder_helper_funcs *encoder_funcs;
	emgd_fbdev_t *emgd_fbdev;

	EMGD_TRACE_ENTER;

	emgd_fbdev = info->par;
	dev	       = emgd_fbdev->priv->ddev;

	switch(blank) {
	case FB_BLANK_UNBLANK:
		EMGD_DEBUG("Turn on Display");

		list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {

			((struct drm_crtc_helper_funcs *)crtc->helper_private)->dpms(crtc,
				DRM_MODE_DPMS_ON);

			list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
				if(encoder->crtc == crtc) {
					encoder_funcs = encoder->helper_private;
					encoder_funcs->dpms(encoder, DRM_MODE_DPMS_ON);
				}
			}
		}
		break;
	case FB_BLANK_NORMAL:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_POWERDOWN:
		EMGD_DEBUG("Turn off Display");

		list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
			list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
				if(encoder->crtc == crtc) {
					encoder_funcs = encoder->helper_private;
					encoder_funcs->dpms(encoder, DRM_MODE_DPMS_OFF);
				}
			}

			((struct drm_crtc_helper_funcs *)crtc->helper_private)->dpms(crtc,
				DRM_MODE_DPMS_OFF);
		}
		break;
	default:
		EMGD_DEBUG("ERROR: Incorrect FB_BLANK value passed");
		break;
	}

	EMGD_TRACE_EXIT;

	return ret;
}



/*
 * emgd_fbcon_initial_config
 *
 * Uses configurations in config_drm to set the start-up configuration.
 * We will not support configurations that require more than one framebuffer
 * at DRM boot time.  The fallback configuration is SINGLE.
 *
 * @param emgd_fbdev (IN) Framebuffer device to initialize
 *
 * @return 0 on success, an error code otherwise
 */
int emgd_fbcon_initial_config(emgd_fbdev_t *emgd_fbdev)
{
	int                    err;
	unsigned long         *dc_assigned;
	igd_context_t         *context;
	unsigned short         port_number;
	emgd_crtc_t           *emgd_crtc;
	struct drm_crtc       *crtc         = NULL;
	igd_display_context_t *primary      = NULL, *secondary = NULL;
	bool                   mode_set_ret = FALSE;
	struct drm_device     *dev          = emgd_fbdev->priv->ddev;

	struct drm_display_mode primary_mode, secondary_mode;

	EMGD_TRACE_ENTER;


	context = ((drm_emgd_priv_t *)dev->dev_private)->context;


	/* Basic error checking.  SINGLE is the fallback configuration */
	if( IGD_DISPLAY_CONFIG_CLONE  != config_drm.dc &&
		IGD_DISPLAY_CONFIG_SINGLE != config_drm.dc ) {
		config_drm.dc = IGD_DISPLAY_CONFIG_SINGLE;
	}


	/*******************************************************
	 * Allocate DSP for the desired DC.  This is to make sure the
	 * display context is valid for subsequent operations.  The
	 * dc_assigned coming out will contain port info in addition to
	 * display configuration.
	 ******************************************************/
	/* Query for the best DC that matches the request */
	err = context->dispatch.query_dc(context, config_drm.dc,
								&dc_assigned, IGD_QUERY_DC_INIT);
	if (err) {
		EMGD_ERROR_EXIT("Cannot initialize the display as requested.\n"
				"The query_dc() function returned %d.", err);
		return -IGD_ERROR_INVAL;
	}

	emgd_fbdev->priv->dc = *dc_assigned;


	/* Allocate and initialize igd_display_context */
	err = context->mod_dispatch.dsp_alloc(context, *dc_assigned, 0);
	if (err) {
		EMGD_ERROR_EXIT("Cannot initialize display context.\n"
			"The dsp_alloc() function returned %d.", err);
		return -IGD_ERROR_INVAL;
	}

	/* Initialize pipe->owner for PRIMARY and SECONDARY */
	port_number = IGD_DC_PRIMARY(*dc_assigned);

	primary = context->mod_dispatch.dsp_display_list[port_number];
	PIPE(primary)->owner = primary;

	if (IGD_DC_SECONDARY(*dc_assigned)) {
		port_number = IGD_DC_SECONDARY(*dc_assigned);
		secondary = context->mod_dispatch.dsp_display_list[port_number];
		PIPE(secondary)->owner = secondary;
	}



	/*******************************************************
	 * Allocate framebuffer
	 ******************************************************/
	if (0 == (err = alloc_initial_fb(emgd_fbdev))) {

		/* Attach the frame buffer to the CRTC(s)  */
		list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
			emgd_crtc = container_of(crtc, emgd_crtc_t, base);

			if (PIPE(primary) == emgd_crtc->igd_pipe) {
				crtc->fb = &emgd_fbdev->emgd_fb->base;
			}

			/* Attach frame buffer to the secondary CRTC, if necessary */
			if (IGD_DC_SECONDARY(*dc_assigned) &&
				PIPE(secondary) == emgd_crtc->igd_pipe) {
				crtc->fb = &emgd_fbdev->emgd_fb->base;
			}
		}


		err = register_framebuffer(emgd_fbdev->priv->fbdev);

		EMGD_DEBUG("fb%d: %s framebuffer device",
			emgd_fbdev->priv->fbdev->node,
			emgd_fbdev->priv->fbdev->fix.id);
	}


	/*******************************************************
	 * Set Mode for Primary
	 ******************************************************/
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		emgd_crtc = container_of(crtc, emgd_crtc_t, base);

		if (PIPE(primary) == emgd_crtc->igd_pipe) {

			memset(&primary_mode, 0, sizeof(primary_mode));
			primary_mode.crtc_hdisplay = config_drm.width;
			primary_mode.crtc_vdisplay = config_drm.height;
			primary_mode.vrefresh      = config_drm.refresh;

			mode_set_ret = drm_crtc_helper_set_mode(crtc,
								&primary_mode,
								0, 0,
								NULL);
			break;
		}
	}

	if (FALSE == mode_set_ret) {
		EMGD_ERROR_EXIT("Failed to set mode on primary CRTC.  Abort.");
		return -IGD_ERROR_INVAL;
	}


	/*******************************************************
	 * Set Mode for Secondary, if necessary
	 ******************************************************/
	if (IGD_DC_SECONDARY(*dc_assigned)) {

		list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
			emgd_crtc = container_of(crtc, emgd_crtc_t, base);

			if (PIPE(secondary) == emgd_crtc->igd_pipe) {

				memset(&secondary_mode, 0, sizeof(secondary_mode));
				secondary_mode.crtc_hdisplay = config_drm.width;
				secondary_mode.crtc_vdisplay = config_drm.height;
				secondary_mode.vrefresh      = config_drm.refresh;

				mode_set_ret = drm_crtc_helper_set_mode(crtc,
								&secondary_mode,
								0, 0,
								NULL);
								break;
			}
		}

	}

	if (FALSE == mode_set_ret) {
		EMGD_ERROR_EXIT("Failed to set mode on secondary CRTC.  Abort.");
		return -IGD_ERROR_INVAL;
	}


	EMGD_TRACE_EXIT;

	return 0;
}



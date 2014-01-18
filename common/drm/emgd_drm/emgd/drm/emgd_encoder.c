/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_encoder.c
 * $Revision: 1.4 $
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
 *  Encoder / kenrel mode setting functions.
 *-----------------------------------------------------------------------------
 */
#define MODULE_NAME hal.oal

#include <drmP.h>
#include <drm_crtc_helper.h>
#include <linux/version.h>

#include <mode_dispatch.h>
#include "drm_emgd_private.h"

#include <igd_pwr.h>



/*------------------------------------------------------------------------------
 * External Functions
 *------------------------------------------------------------------------------
 */
extern int calculate_eld(igd_display_port_t *port,
				igd_timing_info_t *timing_info);



static void emgd_encoder_destroy(struct drm_encoder *encoder);
static void emgd_encoder_dpms(struct drm_encoder *encoder, int mode);
static bool emgd_encoder_mode_fixup(struct drm_encoder *encoder,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode);
static void emgd_encoder_prepare(struct drm_encoder *encoder);
static void emgd_encoder_mode_set(struct drm_encoder *encoder,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode);
static void emgd_encoder_commit(struct drm_encoder *encoder);


const struct drm_encoder_funcs emgd_encoder_funcs = {
	.destroy = emgd_encoder_destroy,
};

const struct drm_encoder_helper_funcs emgd_encoder_helper_funcs = {
	.dpms       = emgd_encoder_dpms,
	.mode_fixup = emgd_encoder_mode_fixup,
	.prepare    = emgd_encoder_prepare,
	.mode_set   = emgd_encoder_mode_set,
	.commit     = emgd_encoder_commit,
};


/**
 * emgd_encoder_dpms
 *
 * This function will put the encoder to either an ON or OFF state.  Anything
 * that is not DRM_MODE_DPMS_ON is treated as an off-state.
 *
 * @param encoder (IN) Encoder
 * @param mode    (IN) power mode
 *
 * @return None
 */
static void emgd_encoder_dpms(struct drm_encoder *encoder, int mode)
{
	emgd_crtc_t *emgd_crtc = container_of(encoder->crtc, emgd_crtc_t, base);
	emgd_encoder_t *emgd_encoder = container_of(encoder, emgd_encoder_t, base);
	igd_display_port_t *igd_port = emgd_encoder->igd_port;

	EMGD_TRACE_ENTER;

	/* The following check is a work around for KMS tries to
	 * program both the crtcs and ports (LVDS and SDVO)
	 * even if it is in single mode. It results in a SIGSEGV.
	 * By putting this check we ensure that it moves forward
	 * only if there is a valid context associated  with the
	 * port.
	 */
	if(emgd_crtc->igd_pipe->owner) {

		EMGD_DEBUG("Setting port %lx power to %d",
					igd_port->port_number, mode);

		switch(mode) {

			case DRM_MODE_DPMS_ON:
				mode_context->kms_dispatch->kms_program_port(emgd_encoder,
						IGD_DISPLAY_ENABLE);
				mode_context->kms_dispatch->kms_post_program_port(emgd_encoder,
						TRUE);
				break;

			case DRM_MODE_DPMS_STANDBY:
			case DRM_MODE_DPMS_SUSPEND:
			case DRM_MODE_DPMS_OFF:
				mode_context->kms_dispatch->kms_program_port(emgd_encoder,
						IGD_DISPLAY_DISABLE);
				break;

			default:
			EMGD_ERROR_EXIT("Unsupported DPMS mode");
			return;
		}
	}else {
		EMGD_DEBUG("Owner is null for this pipe");
	}

	EMGD_TRACE_EXIT;
}



/**
 * emgd_encoder_mode_fixup
 *
 * Called before a mode set, takes the input "mode", matches it to the closest
 * supported mode, then put the supported mode into "adjusted_mode" to let the
 * caller know.
 *
 * Note: We cannot handle centered and scaled mode with this.  To handle this
 *       we need to program the pipe and the port to different sets of timings.
 *       The CRTC Helper does not allow this.  It wants to send adjusted_mode
 *       to both the CRTC and the Encoder.  We can maybe get around this by
 *       modifying the "mode" parameter, but that is not the right approach.
 *
 * @param encoder (IN) Encoder being prepared
 * @param mode    (IN) Requested mode
 * @param adjusted_mode (IN) Encoder supported mode
 *
 * @return true, false (details TBD)
 */
static bool emgd_encoder_mode_fixup(struct drm_encoder *encoder,
		struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	struct drm_device      *dev          = NULL;
	igd_context_t          *context      = NULL;
	igd_display_port_t     *port         = NULL;
	igd_framebuffer_info_t *fb_info      = NULL;
	emgd_encoder_t         *emgd_encoder = NULL;
	igd_timing_info_t      *timing       = NULL;
	igd_display_info_t     *pt_info      = NULL;
	emgd_crtc_t            *emgd_crtc    = NULL;
	igd_display_pipe_t     *pipe         = NULL;
	unsigned long           existing_height  = 0;
	unsigned long           existing_width   = 0;
	unsigned long           existing_refresh = 0;
	int                     ret;

	EMGD_TRACE_ENTER;

	/* Check ajusted mode to see if it's valid.  If not, populate it */
	if (adjusted_mode->crtc_htotal == 0) {
		EMGD_DEBUG("No valid mode in adjusted mode, setting valid mode");
		drm_mode_set_crtcinfo(adjusted_mode, 0);
	}

	dev = encoder->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	emgd_encoder = container_of(encoder, emgd_encoder_t, base);
	port = emgd_encoder->igd_port;
	if (!port->pt_info) {
		port->pt_info = kzalloc(sizeof(igd_display_info_t), GFP_KERNEL);
		if (!port->pt_info) {
			EMGD_DEBUG("Cannot allocate igd_display_into_t");
			return false;
		}
	}
	existing_height  = port->pt_info->height;
	existing_width   = port->pt_info->width;
	existing_refresh = port->pt_info->refresh;
	pt_info          = port->pt_info;

	fb_info = kzalloc(sizeof(igd_framebuffer_info_t), GFP_KERNEL);
	if (!fb_info) {
		EMGD_DEBUG("Cannot allocate framebuffer info");
		return false;
	}

	/* Get the dimension of the framebuffer linked to the CRTC.  If it is
	 * smaller than the resolution, kms_match_mode will either center it
	 * or let the encoder hardware scale it */
	fb_info->width  = encoder->crtc->fb->width;
	fb_info->height = encoder->crtc->fb->height;
	EMGD_DEBUG("Setting fb_info to: %dx%d", fb_info->width, fb_info->height);

	pt_info->width        = mode->crtc_hdisplay;
	pt_info->height       = mode->crtc_vdisplay;
	pt_info->refresh      = mode->vrefresh;
	pt_info->dclk         = mode->synth_clock;
	pt_info->htotal       = mode->crtc_htotal;
	pt_info->hblank_start = mode->crtc_hblank_start;
	pt_info->hblank_end   = mode->crtc_hblank_end;
	pt_info->hsync_start  = mode->crtc_hsync_start;
	pt_info->hsync_end    = mode->crtc_hsync_end;
	pt_info->vtotal       = mode->crtc_vtotal;
	pt_info->vblank_start = mode->crtc_vblank_start;
	pt_info->vblank_end   = mode->crtc_vblank_end;
	pt_info->vsync_start  = mode->crtc_vsync_start;
	pt_info->vsync_end    = mode->crtc_vsync_end;
	pt_info->mode_number  = mode->clock_index;
	pt_info->flags        = 0;
	EMGD_DEBUG("Setting pt_info to: %dx%d", pt_info->width, pt_info->height);

	ret = mode_context->kms_dispatch->kms_match_mode((void *)emgd_encoder,
		(void *)fb_info, &timing);

	if (!ret) {
		adjusted_mode->crtc_hdisplay     = timing->width;
		adjusted_mode->crtc_vdisplay     = timing->height;
		adjusted_mode->vrefresh          = timing->refresh;
		adjusted_mode->synth_clock       = timing->dclk;
		adjusted_mode->crtc_htotal       = timing->htotal;
		adjusted_mode->crtc_hblank_start = timing->hblank_start;
		adjusted_mode->crtc_hblank_end   = timing->hblank_end;
		adjusted_mode->crtc_hsync_start  = timing->hsync_start;
		adjusted_mode->crtc_hsync_end    = timing->hsync_end;
		adjusted_mode->crtc_vtotal       = timing->vtotal;
		adjusted_mode->crtc_vblank_start = timing->vblank_start;
		adjusted_mode->crtc_vblank_end   = timing->vblank_end;
		adjusted_mode->crtc_vsync_start  = timing->vsync_start;
		adjusted_mode->crtc_vsync_end    = timing->vsync_end;
		adjusted_mode->clock_index       = timing->mode_number;
		adjusted_mode->private_flags     = timing->mode_info_flags;

		EMGD_DEBUG("(%dx%d@%d)->(%dx%d@%d)",
			mode->crtc_hdisplay, mode->crtc_vdisplay, mode->vrefresh,
			adjusted_mode->crtc_hdisplay, adjusted_mode->crtc_vdisplay,
			adjusted_mode->vrefresh);

		/* Check our new mode against what currently there
		 * to see if we can do a seamless mode-set
		 */
		if (emgd_encoder->flags & ENCODER_FLAG_FIRST_ALTER) {
			if (mode_context->fw_info) {

				emgd_crtc = container_of(encoder->crtc, emgd_crtc_t, base);
				pipe = emgd_crtc->igd_pipe;

				existing_width = mode_context->fw_info->
					timing_arr[pipe->pipe_num].width;
				existing_height = mode_context->fw_info->
					timing_arr[pipe->pipe_num].height;
				existing_refresh = mode_context->fw_info->
					timing_arr[pipe->pipe_num].refresh;

			}
		}

		if (adjusted_mode->crtc_hdisplay == existing_width &&
			adjusted_mode->crtc_vdisplay == existing_height) {

			if (abs(adjusted_mode->vrefresh - existing_refresh) <= 1) {
				emgd_encoder->flags |= ENCODER_FLAG_SEAMLESS;
			}
		}
	}

	kfree(fb_info);

	EMGD_TRACE_EXIT;
	return (!ret);
}



/**
 * emgd_encoder_prepare
 *
 * Based on the available documentation at the moment, this function gets
 * called right before a mode change.  Its job is to turn off the display.
 *
 * @param encoder (IN) Encoder being prepared
 *
 * @return None
 */
static void emgd_encoder_prepare(struct drm_encoder *encoder)
{
	struct drm_encoder_helper_funcs *encoder_funcs;
	emgd_encoder_t *emgd_encoder;

	EMGD_TRACE_ENTER;

	emgd_encoder = container_of(encoder, emgd_encoder_t, base);

	if (!(emgd_encoder->flags & ENCODER_FLAG_SEAMLESS)) {
		encoder_funcs = encoder->helper_private;
		encoder_funcs->dpms(encoder, DRM_MODE_DPMS_OFF);
	}

	emgd_encoder->flags &= ~ENCODER_FLAG_FIRST_ALTER;

	EMGD_TRACE_EXIT;
}



/**
 * emgd_encoder_commit
 *
 * This function commits the mode change sequence by actually programming
 * the registers.
 *
 * @param encoder (IN) Encoder being prepared
 *
 * @return None
 */
static void emgd_encoder_commit(struct drm_encoder *encoder)
{
	struct drm_encoder_helper_funcs *encoder_funcs;
	emgd_encoder_t     *emgd_encoder = NULL;
	emgd_crtc_t        *emgd_crtc;
	igd_display_port_t *port;
	igd_display_pipe_t *pipe;

	EMGD_TRACE_ENTER;

	emgd_encoder = container_of(encoder, emgd_encoder_t, base);


	if (!(emgd_encoder->flags & ENCODER_FLAG_SEAMLESS)) {

		port      = emgd_encoder->igd_port;
		emgd_crtc = container_of(encoder->crtc, emgd_crtc_t, base);
		pipe      = emgd_crtc->igd_pipe;

/*		mode_context->kms_dispatch->kms_program_port(emgd_encoder,
	    		IGD_DISPLAY_ENABLE);*/
/*
		port->pd_driver->set_mode(port->pd_context, pipe->timing,
								1<<pipe->pipe_num);*/
		

		encoder_funcs = encoder->helper_private;
		encoder_funcs->dpms(encoder, DRM_MODE_DPMS_ON);

	}
	/* Reset our seamless variable */
	emgd_encoder->flags &= ~ENCODER_FLAG_SEAMLESS;

	/* TODO: Add call to check_display */

	EMGD_TRACE_EXIT;
}



/**
 * emgd_encoder_mode_set
 *
 * This function saves the requested timings into the Port Timing Info
 * structure.  At emgd_encoder_commit() time we should be using these
 * timings to program the port, but currently we are using timings from
 * the pipe.  This is fine for now, but at one point we should investigate
 * the centering case in which the port timings may not match the pipe timings.
 *
 * @param encoder (IN) Encoder being prepared
 * @param mode    (IN)
 * @param adjusted_mode (IN)
 *
 * @return None
 */
static void emgd_encoder_mode_set(struct drm_encoder *encoder,
		struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	emgd_crtc_t        *emgd_crtc    = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	igd_display_pipe_t *pipe         = NULL;
	igd_display_port_t *port         = NULL;
	pd_timing_t        *timing       = NULL;

	EMGD_TRACE_ENTER;


	emgd_encoder = container_of(encoder, emgd_encoder_t, base);

	if (!(emgd_encoder->flags & ENCODER_FLAG_SEAMLESS)) {
		port = emgd_encoder->igd_port;
		emgd_crtc = container_of(encoder->crtc, emgd_crtc_t, base);
		pipe = emgd_crtc->igd_pipe;

		if (pipe) {
			timing = (pd_timing_t *)pipe->timing;

			if (NULL == port->pt_info) {
				port->pt_info = kzalloc(sizeof(igd_display_info_t), GFP_KERNEL);

				if (!port->pt_info) {
					EMGD_ERROR_EXIT("Unable to allocate pt_info.");
					return;
				}
			}

			port->pt_info->width        = adjusted_mode->crtc_hdisplay;
			port->pt_info->height       = adjusted_mode->crtc_vdisplay;
			port->pt_info->refresh      = adjusted_mode->vrefresh;
			port->pt_info->dclk         = adjusted_mode->synth_clock;
			port->pt_info->htotal       = adjusted_mode->crtc_htotal;
			port->pt_info->hblank_start = adjusted_mode->crtc_hblank_start;
			port->pt_info->hblank_end   = adjusted_mode->crtc_hblank_end;
			port->pt_info->hsync_start  = adjusted_mode->crtc_hsync_start;
			port->pt_info->hsync_end    = adjusted_mode->crtc_hsync_end;
			port->pt_info->vtotal       = adjusted_mode->crtc_vtotal;
			port->pt_info->vblank_start = adjusted_mode->crtc_vblank_start;
			port->pt_info->vblank_end   = adjusted_mode->crtc_vblank_end;
			port->pt_info->vsync_start  = adjusted_mode->crtc_vsync_start;
			port->pt_info->vsync_end    = adjusted_mode->crtc_vsync_end;
			port->pt_info->mode_number  = adjusted_mode->clock_index;
			port->pt_info->flags        = adjusted_mode->private_flags;

			port->pt_info->x_offset     = timing->x_offset;
			port->pt_info->y_offset     = timing->y_offset;
			port->pt_info->flags       |= IGD_DISPLAY_ENABLE;


			EMGD_DEBUG("Calculate ELD");
			if (calculate_eld(port, timing)) {
				EMGD_DEBUG("Fail to calculate ELD");
			}

		} else {
			EMGD_ERROR("Trying to set the mode without a pipe attached.");
		}
	}

	EMGD_TRACE_EXIT;
}



/**
 * emgd_encoder_destroy
 *
 * Frees the resources allocated for this encoder during "create_encoder()"
 *
 * @param encoder (IN) Encoder to be freed
 *
 * @return None
 */
static void emgd_encoder_destroy(struct drm_encoder *encoder)
{
	emgd_encoder_t *emgd_encoder;

	EMGD_TRACE_ENTER;
	emgd_encoder = container_of(encoder, emgd_encoder_t, base);

	drm_encoder_cleanup(encoder);

	kfree(emgd_encoder);

	EMGD_TRACE_EXIT;
}

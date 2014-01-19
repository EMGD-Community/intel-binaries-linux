/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_crtc.c
 * $Revision: 1.7 $
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
 *  CRTC / kernel mode setting functions.
 *-----------------------------------------------------------------------------
 */
#define MODULE_NAME hal.oal


#include <drmP.h>
#include <drm_crtc_helper.h>
#include <linux/version.h>


#include "drm_emgd_private.h"
#include "emgd_drm.h"

#include <memory.h>
#include <mode_dispatch.h>
#include <igd_pwr.h>

/* Necessary to cursor memory from PVR buffer */
#include "pvr_bridge_km.h"

/* Maximum cursor size supported by our HAL: 64x64 in ARGB */
#define MAX_CURSOR_SIZE (64*64*4)

static void emgd_crtc_dpms(struct drm_crtc *crtc, int mode);
static bool emgd_crtc_mode_fixup(struct drm_crtc *crtc,
		struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode);
static int emgd_crtc_mode_set(struct drm_crtc *crtc,
		struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode,
		int x, int y, struct drm_framebuffer *old_fb);
static int emgd_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
		struct drm_framebuffer *old_fb);
static void emgd_crtc_prepare(struct drm_crtc *crtc);
static void emgd_crtc_commit(struct drm_crtc *crtc);

static void emgd_crtc_save(struct drm_crtc *crtc);
static void emgd_crtc_restore(struct drm_crtc *crtc);
static int emgd_crtc_cursor_set(struct drm_crtc *crtc,
		struct drm_file *file_priv, uint32_t handle,
		uint32_t width, uint32_t height);
static int emgd_crtc_cursor_move(struct drm_crtc *crtc, int x, int y);
static void emgd_crtc_gamma_set(struct drm_crtc *crtc,
		unsigned short *red, unsigned short *green, unsigned short *blue,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		uint32_t start,
#endif
		uint32_t size);
static void emgd_crtc_destroy(struct drm_crtc *crtc);
static void emgd_crtc_load_lut(struct drm_crtc *crtc);
static int emgd_crtc_page_flip(struct drm_crtc *crtc,
                                struct drm_framebuffer *fb,
                                struct drm_pending_vblank_event *event);
static int emgd_crtc_set_config(struct drm_mode_set *set);



const struct drm_crtc_helper_funcs emgd_crtc_helper_funcs = {
	.dpms          = emgd_crtc_dpms,
	.mode_fixup    = emgd_crtc_mode_fixup,
	.mode_set      = emgd_crtc_mode_set,
	.mode_set_base = emgd_crtc_mode_set_base,
	.prepare       = emgd_crtc_prepare,
	.commit        = emgd_crtc_commit,
	.load_lut      = emgd_crtc_load_lut,
};

const struct drm_crtc_funcs emgd_crtc_funcs = {
	.save        = emgd_crtc_save,
	.restore     = emgd_crtc_restore,
	.cursor_set  = emgd_crtc_cursor_set,
	.cursor_move = emgd_crtc_cursor_move,
	.gamma_set   = emgd_crtc_gamma_set,
	.set_config  = emgd_crtc_set_config,
	.destroy     = emgd_crtc_destroy,
	.page_flip   = emgd_crtc_page_flip,
};



static int emgd_crtc_set_config(struct drm_mode_set *set)
{
	int ret;
	struct drm_device  *dev;
	struct drm_encoder *encoder;
	struct drm_encoder_helper_funcs *encoder_funcs;

	EMGD_TRACE_ENTER;

	ret = drm_crtc_helper_set_config(set);

	if (ret) {
		EMGD_ERROR_EXIT("Failed to set config\n");
		return ret;
	}

	/*  FIXME: This is a temporary fix for the resume problem:  Screen goes
	 *  blank after an idling period, then doesn't come back.  At this point
	 *  it is unclear why KMS is not calling our DPMS functions to re-enable
	 *  the display.  KMS does call this function on resume, so we will
	 *  enable the displays here
	 */
	emgd_crtc_dpms(set->crtc, DRM_MODE_DPMS_ON);

	dev = set->crtc->dev;

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (encoder->crtc == set->crtc) {
			encoder_funcs = encoder->helper_private;
			encoder_funcs->dpms(encoder, DRM_MODE_DPMS_ON);
		}
	}


	EMGD_TRACE_EXIT;

	return ret;
}

/*
 * Sets the power management mode of the pipe.
 */
static void emgd_crtc_dpms(struct drm_crtc *crtc, int mode)
{
	emgd_crtc_t           *emgd_crtc = NULL;
	igd_display_pipe_t    *pipe = NULL;

	EMGD_TRACE_ENTER;


	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	EMGD_DEBUG("pipe=%d, mode=%d", emgd_crtc->crtc_id, mode);
	pipe = emgd_crtc->igd_pipe;

	/* The following check is a work around.KMS tries to program
	 * both the crtcs and ports (LVDS and SDVO) even if it is in
	 * single mode. It results in a SIGSEGV.
	 * By putting this check we ensure that it moves forward
	 * only if there is a valid context associated  with the
	 * crtc. We check it by checking the owner of the pipe which
	 * should not be null.
	 */
	if (pipe->owner){

		switch(mode) {

			case DRM_MODE_DPMS_ON:
				EMGD_DEBUG("Checking if we have pipe timings");
				if (!pipe->timing) {
					/* If there is no pipe timing, we cannot enable */
					EMGD_ERROR("No pipe timing, can't enable pipe=%d, mode=%d",
							emgd_crtc->crtc_id, DRM_MODE_DPMS_ON );
				} else {
					EMGD_DEBUG("Calling program pipe");
					mode_context->kms_dispatch->kms_program_pipe(emgd_crtc);
					EMGD_DEBUG("Calling program plane");
					mode_context->kms_dispatch->
						kms_set_plane_pwr(emgd_crtc, TRUE);

					crtc->enabled = true;
				}
				break;

			case DRM_MODE_DPMS_STANDBY:
			case DRM_MODE_DPMS_SUSPEND:
			case DRM_MODE_DPMS_OFF:
				if (emgd_crtc->igd_pipe->inuse && crtc->enabled) {
					EMGD_DEBUG("Calling program plane");
					mode_context->kms_dispatch->
						kms_set_plane_pwr(emgd_crtc, FALSE);

					EMGD_DEBUG("Calling program pipe");
					mode_context->kms_dispatch->
						kms_set_pipe_pwr(emgd_crtc, FALSE);
					crtc->enabled = false;
				}else {
					EMGD_ERROR("pipe is already off");
				}
				break;
			default:
				break;
		}
	}
	EMGD_TRACE_EXIT;
}



static bool emgd_crtc_mode_fixup(struct drm_crtc *crtc,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	EMGD_TRACE_ENTER;

	/* Check ajusted mode to see if it's valid.  If not, populate it */
	if (adjusted_mode->crtc_htotal == 0) {
		drm_mode_set_crtcinfo(adjusted_mode, 0);
	}

	EMGD_TRACE_EXIT;
	return 1;
}



/**
 * emgd_crtc_mode_set
 *
 * Sets mode for the selected CRTC.  This function only sets the timings
 * into the CRTC, but doesn't actually program the timing values into the
 * registers.  The actual programming is done in emgd_crtc_commit.
 *
 * @param crtc   (IN) CRTC to configure
 * @param x      (IN) starting X position in the frame buffer
 * @param y      (IN) starting Y position in the frame buffer
 * @param old_fb (IN) Not used
 *
 * @return 0
 */
static int emgd_crtc_mode_set(struct drm_crtc *crtc,
		struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode,
		int x, int y, struct drm_framebuffer *old_fb)
{
	emgd_crtc_t            *emgd_crtc = NULL;
	struct drm_device      *dev = NULL;
	igd_context_t          *context = NULL;
	igd_display_pipe_t     *pipe = NULL;
	igd_timing_info_t      *timing = NULL;

	EMGD_TRACE_ENTER;


	dev = crtc->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	/* Which pipe are we using */
	EMGD_DEBUG("Getting PIPE");
	emgd_crtc = container_of(crtc, emgd_crtc_t, base);

	EMGD_DEBUG("\t\tpipe=%d, ->(%dx%d@%d)", emgd_crtc->crtc_id,
		adjusted_mode->crtc_hdisplay, adjusted_mode->crtc_vdisplay,
		adjusted_mode->vrefresh);

	pipe  = emgd_crtc->igd_pipe;


	if (old_fb) {
		EMGD_DEBUG("Handling old framebuffer?");
		/* What do we do with the old framebuffer? */
	}

	timing = kzalloc(sizeof(igd_timing_info_t), GFP_KERNEL);
	if (!timing) {
		EMGD_ERROR_EXIT("unable to allocate a igd_timing_info struct.");
		return 1;
	}

	timing->width = adjusted_mode->crtc_hdisplay;
	timing->height = adjusted_mode->crtc_vdisplay;
	timing->refresh = adjusted_mode->vrefresh;
	timing->dclk = adjusted_mode->synth_clock; /* Is this the right variable? */
	timing->htotal = adjusted_mode->crtc_htotal;
	timing->hblank_start = adjusted_mode->crtc_hblank_start;
	timing->hblank_end = adjusted_mode->crtc_hblank_end;
	timing->hsync_start = adjusted_mode->crtc_hsync_start;
	timing->hsync_end = adjusted_mode->crtc_hsync_end;
	timing->vtotal = adjusted_mode->crtc_vtotal;
	timing->vblank_start = adjusted_mode->crtc_vblank_start;
	timing->vblank_end = adjusted_mode->crtc_vblank_end;
	timing->vsync_start = adjusted_mode->crtc_vsync_start;
	timing->vsync_end = adjusted_mode->crtc_vsync_end;
	timing->mode_number = adjusted_mode->clock_index;
	timing->mode_info_flags = adjusted_mode->private_flags;
	timing->x_offset = x;
	timing->y_offset = y;
	timing->mode_info_flags |= IGD_DISPLAY_ENABLE;

	if (pipe->timing) {
		OS_MEMCPY(pipe->timing, timing, sizeof(igd_timing_info_t));
		kfree(timing);
	} else {
		pipe->timing = timing;
	}

	/* The code above only sets the CRTC timing, not the plane */
	emgd_crtc_mode_set_base(crtc, x, y, old_fb);


	EMGD_TRACE_EXIT;
	return 0;
}



/**
 * emgd_crtc_mode_set_base
 *
 * Sets the starting position in the framebuffer for the given CRTC.
 *
 * @param crtc   (IN) CRTC to configure
 * @param x      (IN) starting X position in the frame buffer
 * @param y      (IN) starting Y position in the frame buffer
 * @param old_fb (IN) Not used
 *
 * @return 0
 */
static int emgd_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
		struct drm_framebuffer *old_fb)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_display_context_t *display = NULL;
	emgd_framebuffer_t *emgd_fb;
	igd_framebuffer_info_t *plane_fb_info;
	struct drm_framebuffer *fb = NULL;
	int ret = 0;

	EMGD_TRACE_ENTER;


	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	fb        = crtc->fb;
	emgd_fb   = container_of(fb, emgd_framebuffer_t, base);
	display   = emgd_crtc->igd_pipe->owner;


	if (old_fb) {
		/* We should not need to do anything here. */
	}

	plane_fb_info = PLANE(display)->fb_info;

	plane_fb_info->width          = fb->width;
	plane_fb_info->height         = fb->height;
	plane_fb_info->screen_pitch   = fb->DRMFB_PITCH;
	plane_fb_info->flags          = 0;
	plane_fb_info->allocated      = 1;
	plane_fb_info->fb_base_offset = emgd_fb->gtt_offset;
	plane_fb_info->visible_offset = (y * fb->DRMFB_PITCH) +
		(x * (fb->bits_per_pixel / 8));


	PLANE(display)->inuse = 1;
	PLANE(display)->ref_cnt++;

	mode_context->kms_dispatch->kms_program_plane(emgd_crtc, TRUE);


	EMGD_TRACE_EXIT;
	return ret;
}



static void emgd_crtc_prepare(struct drm_crtc *crtc)
{
	EMGD_TRACE_ENTER;
	emgd_crtc_dpms(crtc, DRM_MODE_DPMS_OFF);
	EMGD_TRACE_EXIT;
}

static void emgd_crtc_commit(struct drm_crtc *crtc)
{
	EMGD_TRACE_ENTER;
	emgd_crtc_dpms(crtc, DRM_MODE_DPMS_ON);

	EMGD_TRACE_EXIT;
}


static void emgd_crtc_save(struct drm_crtc *crtc)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;
	unsigned long save_flags;

	/* We should probably break out register save and restore
	 * so that its specific to a crtc
	 */
	EMGD_TRACE_ENTER;

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;
	EMGD_DEBUG("\t\tpipe=%d", emgd_crtc->crtc_id);

	save_flags = (IGD_REG_SAVE_ALL & ~IGD_REG_SAVE_GTT & ~IGD_REG_SAVE_RB) |
		IGD_REG_SAVE_TYPE_MISC;
	context->dispatch.driver_save((igd_driver_h)context, save_flags);

	EMGD_TRACE_EXIT;
}

static void emgd_crtc_restore(struct drm_crtc *crtc)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;
	unsigned long restore_flags;

	EMGD_TRACE_ENTER;

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;
	EMGD_DEBUG("\t\tpipe=%d", emgd_crtc->crtc_id);

	restore_flags = IGD_REG_SAVE_TYPE_MISC;
	context->dispatch.driver_restore((igd_driver_h)context, restore_flags);

	EMGD_TRACE_EXIT;
}

static int emgd_crtc_cursor_set(struct drm_crtc *crtc,
		struct drm_file *file_priv, uint32_t handle,
		uint32_t width, uint32_t height)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;
	igd_display_context_t *display = NULL;
	igd_cursor_info_t *cursor_info = NULL;
	PVRSRV_PER_PROCESS_DATA *pvr_perproc;
	PVRSRV_KERNEL_MEM_INFO *pvr_meminfo;
	PVRSRV_ERROR ret;
	unsigned char *tempcurs;
	unsigned long pid;
	struct page **pagelist;
	unsigned long numpages, page_offset, cursor_size = 0;
	void *pageaddr;

	EMGD_TRACE_ENTER;

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	if (!emgd_crtc->igd_pipe->inuse) {
		EMGD_ERROR("\t\tpipe %d is not available", emgd_crtc->crtc_id);
		return 1;
	}
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;
	display = emgd_crtc->igd_pipe->owner;

	if (!handle) {
		/* If handle is 0, turn off the cursor */
		EMGD_DEBUG("Turning off cursor");
		mode_context->dispatch->full->program_cursor(display, FALSE);
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

		/* Allocate a temporary buffer to hold the cursor image */
		tempcurs = OS_ALLOC(MAX_CURSOR_SIZE);
		if (!tempcurs) {
			return -ENOMEM;
		}

		/* Map the page list into kernel virtual address space */
		pageaddr = vmap(pagelist, numpages, VM_MAP, PAGE_KERNEL_UC_MINUS);

		/*
		 * Our HAL only accepts 64x64 x 4byte cursors.  Only copy the first
		 * 64x64x4 of the provided buffer if the user tries to pass us
		 * something too big.
		 */
		if (numpages * PAGE_SIZE > MAX_CURSOR_SIZE) {
			cursor_size = MAX_CURSOR_SIZE;
		} else {
			cursor_size = numpages * PAGE_SIZE;
		}

		OS_MEMCPY(tempcurs, pageaddr, cursor_size);

		vunmap(pageaddr);

		/* Pass the cursor image to the HAL to program the cursor plane */
		cursor_info = emgd_crtc->igd_pipe->cursor->cursor_info;
		cursor_info->flags = IGD_CURSOR_LOAD_ARGB_IMAGE | IGD_CURSOR_ON;
		cursor_info->width = width;
		cursor_info->height = height;
		context->dispatch.alter_cursor(display, cursor_info, tempcurs);

		/* Free temporary cursor image */
		OS_FREE(tempcurs);
	}

	EMGD_TRACE_EXIT;
	return 0;
}

static int emgd_crtc_cursor_move(struct drm_crtc *crtc, int x, int y)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;
	igd_display_context_t *display = NULL;
	igd_cursor_info_t *emgd_cursor;

	/* Too spammy; commenting out */
#if 0
	EMGD_TRACE_ENTER;
#endif

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	if (!emgd_crtc->igd_pipe->inuse) {
		EMGD_ERROR("\t\tpipe %d is not available", emgd_crtc->crtc_id);
		return 1;
	}
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;
	display = emgd_crtc->igd_pipe->owner;

    emgd_cursor = emgd_crtc->igd_pipe->cursor->cursor_info;
	emgd_cursor->x_offset = x;
	emgd_cursor->y_offset = y;
	context->dispatch.alter_cursor_pos(display, emgd_cursor);

	/* Too spammy; commenting out */
#if 0
	EMGD_TRACE_EXIT;
#endif
	return 0;
}

static void emgd_crtc_gamma_set(struct drm_crtc *crtc,
		unsigned short *red, unsigned short *green, unsigned short *blue,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		uint32_t start,
#endif
		uint32_t size)
{
	int end, i;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
	int start = 0;
#endif
	emgd_crtc_t *emgd_crtc = NULL;

	EMGD_TRACE_ENTER;

	end = (start + size > 256) ? 256 : start + size;
	emgd_crtc = container_of(crtc, emgd_crtc_t, base);

	for (i=start; i < end; i++) {
		emgd_crtc->lut_r[i] = red[i] >> 8;
		emgd_crtc->lut_g[i] = green[i] >> 8;
		emgd_crtc->lut_b[i] = blue[i] >> 8;
	}

	emgd_crtc_load_lut(crtc);

	EMGD_TRACE_EXIT;
}

static void emgd_crtc_destroy(struct drm_crtc *crtc)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;
	igd_display_pipe_t *igd_pipe = NULL;

	EMGD_TRACE_ENTER;

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	igd_pipe = emgd_crtc->igd_pipe;
	if (!igd_pipe) {
		EMGD_ERROR("\t\tpipe %d is not available", emgd_crtc->crtc_id);
		return;
	}
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;

	EMGD_DEBUG("\t\tpipe=%d", emgd_crtc->crtc_id);

	drm_crtc_cleanup(crtc);

	/* Free our private crtc structure */
	kfree(emgd_crtc);

	igd_pipe->inuse = 0;
	igd_pipe->plane = NULL;
	igd_pipe->timing = NULL;
	igd_pipe->owner = NULL;

	EMGD_TRACE_EXIT;
}

static void emgd_crtc_load_lut(struct drm_crtc *crtc)
{
	emgd_crtc_t *emgd_crtc = NULL;
	igd_context_t *context = NULL;

	EMGD_TRACE_ENTER;

	emgd_crtc = container_of(crtc, emgd_crtc_t, base);
	if (!emgd_crtc->igd_pipe) {
		EMGD_ERROR("\t\tpipe %d is not available", emgd_crtc->crtc_id);
		return;
	}
	context = ((drm_emgd_priv_t *)crtc->dev->dev_private)->context;
	EMGD_DEBUG("\t\tpipe=%d", emgd_crtc->crtc_id);
	/* Call into hal function to set color map. */
	context->mod_dispatch.reg_crtc_lut_set(context, emgd_crtc);

	EMGD_TRACE_EXIT;
}


/*
 * crtc_pageflip_handler()
 *
 * VBlank handler to be called when a pageflip is complete.  This will send
 * the vblank event to userspace.
 *
 * State upon entry (assuming vblank_expected is set):
 *  * newfb is non-NULL
 *  * flip_event is non-NULL
 *  * flip_work_queued is FALSE
 *  * vblank_expected is TRUE (based on assumption)
 *
 * State upon exit (assuming entered with vblank_expected):
 *  * newfb is NULL
 *  * flip_event is NULL
 *  * flip_work_queued is FALSE
 *  * vblank_expected is FALSE
 */
int crtc_pageflip_handler(struct drm_device *dev, int port_num)
{
	drm_emgd_priv_t *devpriv = dev->dev_private;
	emgd_crtc_t *emgd_crtc;
	struct drm_pending_vblank_event *e;
	struct timeval now;
	igd_context_t *context = NULL;
	int crtcnum;
	unsigned long flags;

	EMGD_TRACE_ENTER;

	/*
	 * This handler should only be executed if KMS modesetting is
	 * enabled.  However, the device dependent mode code always hooks
	 * this up. Abort early if KMS modesetting is disabled.
	 */
	if (!devpriv->kms_enabled) {
		return 0;
	}

	context = devpriv->context;

	/* Look up which CRTC that this flip is for. */
	/* If the DC=1, only one of the CRTCs would have a non-NULL owner (display context), 
 	 * so we determine the CRTC by looking at the owner field. If the DC is not 1  
 	 * (which means both the CRTCs have valid owner fields), we look at the port number to 
 	 * determine the right CRTC.
	 */
	if (devpriv->crtcs[0]->igd_pipe->owner && devpriv->crtcs[1]->igd_pipe->owner) {
		if (port_num == devpriv->crtcs[0]->igd_pipe->owner->port_number) {
			crtcnum = 0;
		} else {
			crtcnum = 1;
		}		
	} else {
		if (devpriv->crtcs[0]->igd_pipe->owner) {
			crtcnum = 0;
		} else {
			crtcnum = 1;
		}
	}


	emgd_crtc = devpriv->crtcs[crtcnum];

	/* Protect access to CRTC */
	spin_lock_irqsave(&emgd_crtc->crtc_lock, flags);

	/*
	 * Were we waiting for a vblank to do flip cleanup?  If not, we
	 * should just bail out.
	 */
	if (!emgd_crtc->vblank_expected) {
		spin_unlock_irqrestore(&emgd_crtc->crtc_lock, flags);
		return 1;
	}

	/* Sanity check: shouldn't be here if we still have flip work queued */
	if (emgd_crtc->flip_work_queued) {
		EMGD_ERROR("Flip vblank handler while work queued!");
		spin_unlock_irqrestore(&emgd_crtc->crtc_lock, flags);
		return 0;
	}

	/*
	 * Sanity check: shouldn't be possible to get to this point without a
	 * userspace event to send.
	 */
	if (!emgd_crtc->flip_event) {
		EMGD_ERROR("Pageflip vblank handler has no userspace event");
		spin_unlock_irqrestore(&emgd_crtc->crtc_lock, flags);
		return 1;
	}

	/* Release vblank refcount */
	drm_vblank_put(dev, crtcnum);
	emgd_crtc->vblank_expected = 0;

	/* Flip is now complete; send userspace event, if requested */
	e = emgd_crtc->flip_event;
	do_gettimeofday(&now);
	e->event.sequence = 0;
	e->event.tv_sec = now.tv_sec;
	e->event.tv_usec = now.tv_usec;
	list_add_tail(&e->base.link, &e->base.file_priv->event_list);
	wake_up_interruptible(&e->base.file_priv->event_wait);

	/*
	 * Cleanup; not in process of switching to new FB, no outstanding
	 * userspace event awaiting our attention.
	 */
	emgd_crtc->newfb = NULL;
	emgd_crtc->flip_event = NULL;

	spin_unlock_irqrestore(&emgd_crtc->crtc_lock, flags);

	EMGD_TRACE_EXIT;
	return 1;
}


/**
 * emgd_flip_worker
 *
 * Workqueue task to schedule a flip when rendering to the new framebuffer
 * is complete.  Should check the "cancel" flag and just give up,
 * regardless of whether rendering is complete or not if set.
 *
 * State upon entry:
 *  * newfb is non-NULL
 *  * no constraint on flip_event
 *  * flip_work_queued is TRUE
 *  * vblank_expected is FALSE
 *
 * State upon exit:
 *  * no constraint on newfb
 *  * no constraint on flip_event
 *  * no constraint on flip_work_queued
 *  * no constraint on vblank_expected
 */
void emgd_flip_worker(struct work_struct *w)
{
	drm_emgd_priv_t *dev_priv;
	igd_context_t *igd_context;
	PVRSRV_KERNEL_MEM_INFO *meminfo;
	PVRSRV_SYNC_DATA *syncdata = NULL;
	emgd_crtc_t *crtc;
	igd_surface_t igd_surface = { 0 };
	unsigned long flags;
	unsigned int crtcnum;
	int ret;

	/* Which CRTC does this work task belong to? */
	crtc = container_of(w, emgd_crtc_t, flip_work);

	/* Protect updates to the CRTC structure */
	spin_lock_irqsave(&crtc->crtc_lock, flags);

	/* Sanity check:  flip_work_queued must be TRUE */
	if (!crtc->flip_work_queued) {
		EMGD_ERROR("Flip worker running without being queued");
		spin_unlock_irqrestore(&crtc->crtc_lock, flags);
		return;
	}

	/* Sanity check:  newfb must not be NULL */
	if (!crtc->newfb) {
		EMGD_ERROR("No newfb in flip worker");
		spin_unlock_irqrestore(&crtc->crtc_lock, flags);
		return;
	}

	/*
	 * Sanity check: we shouldn't be expecting vblank/cleanup while we're
	 * still doing flip work (prior flips that may have scheduled a vblank
	 * were cancelled already.
	 */
	if (crtc->vblank_expected) {
		EMGD_ERROR("Expecting vblank cleanup while flip work in progress");
		spin_unlock_irqrestore(&crtc->crtc_lock, flags);
		return;
	}

	/*
	 * Grab the PVR meminfo and syncinfo for the surface we're waiting for
	 * rendering completion on.
	 */
	if (crtc->newfb->type == PVR_FRAMEBUFFER) {
		meminfo = (PVRSRV_KERNEL_MEM_INFO *)crtc->newfb->pvr_meminfo;
		syncdata = meminfo->psKernelSyncInfo->psSyncData;
	}

	/*
	 * Have we completed all the operations that were pending when the flip
	 * ioctl was called?  If so, proceed with issuing the actual flip.  If
	 * we're flipping to a GMM framebuffer (i.e., the initial system fb),
	 * then we don't need to wait for any kind of rendering).
	 *
	 * It seems like we should test for wraparound here, but I don't see
	 * anywhere in PVR's code where they handle wraparound.  It's probably safe
	 * to ignore for now since even if we perform 60 ops per second against the
	 * framebuffer, it would still take over two years to overflow the 32-bit
	 * unsigned int for the operation counter.
	 */
	if (crtc->newfb->type == GMM_FRAMEBUFFER ||
		syncdata->ui32WriteOpsComplete >= crtc->render_complete_at)
	{
		dev_priv = (drm_emgd_priv_t *) crtc->base.dev->dev_private;
		igd_context = dev_priv->context;
		crtcnum = (crtc == dev_priv->crtcs[0]) ? 0 : 1;

		/* Rendering complete; program the plane registers */
		igd_surface.flags        = IGD_SURFACE_DISPLAY;
		igd_surface.offset       = crtc->newfb->gtt_offset;
		igd_surface.pitch        = crtc->newfb->base.DRMFB_PITCH;
		igd_surface.width        = crtc->newfb->base.width;
		igd_surface.height       = crtc->newfb->base.height;
		igd_surface.pixel_format = IGD_PF_ARGB32;

		igd_context->dispatch.set_surface(
			crtc->igd_pipe->owner,
			IGD_PRIORITY_NORMAL,
			IGD_BUFFER_DISPLAY,
			&igd_surface,
			NULL, /* Not used */
			0);

		/* Flip issued.  No need to requeue the work. */
		crtc->flip_work_queued = 0;

		/*
		 * If a userspace event was requested for this flip, request vblank
		 * interrupts if they aren't already on.
		 */
		if (crtc->flip_event) {
			/* Request vblank events (or inc the refcount if they're already on) */
			ret = drm_vblank_get(crtc->base.dev, crtcnum);
			if (ret) {
				EMGD_ERROR("Failed enable vblanks");
				return;
			}
			crtc->vblank_expected = 1;
		} else {
			/*
			 * No userspace event; the flip is complete as far as we're
			 * concerned.  Clear the "in progress of switching to fb" field.
			 */
			crtc->newfb = NULL;
		}

	} else {
		/* Rendering not complete.  Requeue the work task. */
		schedule_work(w);
	}

	spin_unlock_irqrestore(&crtc->crtc_lock, flags);
}


/**
 * emgd_crtc_page_flip
 *
 * Page flip ioctl handler.  The ioctl simply dispatches a workqueue task
 * which will wait until current rendering against the new framebuffer
 * is complete, then issue the actual flip.  This ioctl should return
 * immediately, allowing pipelining of subsequent CPU execution with
 * the outstanding rendering happening against this framebuffer.
 *
 * @param crtc  (INOUT) The pipe to put the new framebuffer on
 * @param fb    (IN)    Framebuffer to flip to
 * @param event (IN)    Event to signal when flip has been completed
 *
 * @return
 *
 * State upon entry:
 *  * No constraint on newfb
 *  * No constraint on flip_event
 *  * No constraint on flip_work_queued
 *  * No constraint on vblank_expected
 *
 * State upon exit:
 *  * newfb is non-NULL
 *  * no constraint on flip_event
 *  * flip_work_queued is TRUE
 *  * vblank_expected is FALSE
 */
static int emgd_crtc_page_flip(struct drm_crtc *crtc,
                                struct drm_framebuffer *fb,
                                struct drm_pending_vblank_event *event)
{
	emgd_crtc_t        *emgd_crtc;
	emgd_framebuffer_t *emgd_fb;
	drm_emgd_priv_t    *dev_priv;
	igd_context_t      *igd_context;
	unsigned int crtcnum;
	unsigned long flags;
	struct drm_pending_vblank_event *e;
	struct timeval now;
	PVRSRV_KERNEL_MEM_INFO *meminfo;
	PVRSRV_SYNC_DATA *syncdata;

	EMGD_TRACE_ENTER;

	emgd_crtc   = container_of(crtc, emgd_crtc_t, base);
	dev_priv    = (drm_emgd_priv_t *) crtc->dev->dev_private;
	igd_context = dev_priv->context;
	emgd_fb     = container_of(fb, emgd_framebuffer_t, base);
	crtcnum = (emgd_crtc == dev_priv->crtcs[0]) ? 0 : 1;

	/*
	 * Protect updates to the CRTC structure. We don't want this code to
	 * overlap with either the workqueue task or the vblank handler.
	 */
	spin_lock_irqsave(&emgd_crtc->crtc_lock, flags);

	/*
	 * Was there a pending userspace event for a previous flip request?  If
	 * so, just send the completion back now.  We're essentially cancelling
	 * the previous flip (it will never show up on the display), but we don't
	 * want userspace to get confused by not receiving notification.  We'll
	 * also decrement our vblank request if we'd already scheduled on for
	 * cleanup.
	 */
	if ((e = emgd_crtc->flip_event) != NULL) {
		do_gettimeofday(&now);
		e->event.sequence = 0;
		e->event.tv_sec = now.tv_sec;
		e->event.tv_usec = now.tv_usec;
		list_add_tail(&e->base.link, &e->base.file_priv->event_list);
		wake_up_interruptible(&e->base.file_priv->event_wait);
		emgd_crtc->flip_event = NULL;

		/*
		 * If the work task had completed and actually programmed the
		 * registers, it had also requested a vblank callback.  Cancel
		 * that as well since we've already taken care of the necessary
		 * cleanup.  Note that vblanks are only requested when we actually
		 * have a userspace event to send back.
		 */
		if (emgd_crtc->vblank_expected) {
			drm_vblank_put(crtc->dev, crtcnum);
			emgd_crtc->vblank_expected = 0;
		}
	}

	/* Update the CRTC's "target framebuffer" field. */
	emgd_crtc->newfb = emgd_fb;
	emgd_crtc->flip_event = event;

	/*
	 * Set the number of rendering operations that need to complete before we
	 * can flip to this buffer.  I.e., we don't need render to completely
	 * quiesce, we can flip as soon as any operations that are outstanding
	 * right now complete, even if more rendering ops get added to the pipeline
	 * after we return.
	 */
	meminfo = (PVRSRV_KERNEL_MEM_INFO *)emgd_fb->pvr_meminfo;
	syncdata = meminfo->psKernelSyncInfo->psSyncData;
	emgd_crtc->render_complete_at = syncdata->ui32WriteOpsPending;

	/*
	 * If work is already scheduled, nothing more to do here; the
	 * already-scheduled work will see the new values we set next time it wakes
	 * up and will act upon them.  However if work is not scheduled, then
	 * we need to schedule it now.
	 */
	if (!emgd_crtc->flip_work_queued) {
		schedule_work(&emgd_crtc->flip_work);
		emgd_crtc->flip_work_queued = 1;
	}

	/* Move the FB currently associated with the CRTC to the new FB */
	crtc->fb = fb;

	/* Done updating CRTC structure */
	spin_unlock_irqrestore(&emgd_crtc->crtc_lock, flags);

	EMGD_TRACE_EXIT;

	return 0;
}

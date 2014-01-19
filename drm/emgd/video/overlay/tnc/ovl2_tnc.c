/*
 *-----------------------------------------------------------------------------
 * Filename: ovl2_tnc.c
 * $Revision: 1.23 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2010, Intel Corporation.
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
 *  This file contains function that actually programs the second
 *  overlay with the bits to properly configure the overlay
 *  Also includes functions to execute the second overlay flip
 *  instruction, and query the overlay flip status.
 *  Also contains some hardware capabilities querrying functions
 *  for upper overlay layer to get this chips second overlay
 *  capabilities
 *-----------------------------------------------------------------------------
 */
/* Referenced from Napa. Need change later */

#define MODULE_NAME hal.overlay

#include <io.h>
#include <memory.h>
#include <intelpci.h>
#include <math_fix.h>


#include <igd_mode.h>

#include <utils.h>
#include <general.h>
#include <context.h>
#include <rb.h>
#include <mode.h>

#include <tnc/instr.h>
#include <tnc/regs.h>
#include <tnc/cmd.h>
#include <tnc/context.h>

#include "ovl2_tnc.h"
#include "ovl2_regs_tnc.h"
#include "../cmn/ovl_dispatch.h"
#include "../cmn/ovl_virt.h"

extern unsigned long sprite_pixel_formats_tnc[];
extern int micro_prepare_ovl2_tnc(
	igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	ovl2_reg_tnc_t      *spritec_regs_tnc,
	unsigned int         flags);

/*----------------------------------------------------------------------
 * Function: ovl2_check_pf_tnc()
 * Parameters: unsigned int requested_pixel_format -
 *             according to definitions in igd_mode.h
 *
 * Description:
 *
 * Returns:
 *   TRUE on Success
 *   FALSE on The first pixel format that is supported
 *----------------------------------------------------------------------*/
static unsigned int ovl2_check_pf_tnc(
	igd_display_context_t *display,
	unsigned int requested_pixel_format)
{
	unsigned long *spr_pf = sprite_pixel_formats_tnc;
	int temp_loop = 0;

	while(spr_pf[temp_loop]) {
		if(spr_pf[temp_loop] == requested_pixel_format) {
			return TRUE;
		}
		++temp_loop;
	}

	return FALSE;
}

static unsigned int get_uv_shift_x (unsigned long pf)
{

	switch(pf) {
	case IGD_PF_YUV422_PACKED_YUY2:
	case IGD_PF_YUV422_PACKED_UYVY:
	case IGD_PF_YUV420_PLANAR_I420: /* same as IYUV */
	case IGD_PF_YUV420_PLANAR_YV12:
	case IGD_PF_YUV420_PLANAR_NV12:
		return 1;
		break;
	case IGD_PF_YUV410_PLANAR_YVU9:
		return 2;
		break;
	default:
		return 0;
	}

}

static unsigned int get_uv_shift_y (unsigned long pf)
{

	switch(pf) {
	case IGD_PF_YUV420_PLANAR_I420: /* same as IYUV */
	case IGD_PF_YUV420_PLANAR_YV12:
	case IGD_PF_YUV420_PLANAR_NV12:
		return 1;
		break;
	case IGD_PF_YUV410_PLANAR_YVU9:
		return 2;
		break;
	default:
		return 0;
	}

}

unsigned int ovl2_check_tnc(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	igd_timing_info_t *timing;
	unsigned int min_w, min_h;

	EMGD_TRACE_ENTER;

	if (!display){
	    EMGD_ERROR_EXIT("display is null");
	    return -IGD_ERROR_INVAL;
	}
	if (!PIPE(display)){
	    EMGD_ERROR_EXIT("PIPE(display) is null");
	    return -IGD_ERROR_INVAL;
	}

	timing = PIPE(display)->timing;

	/* The following parameters are only valid if the overlay is on, so
	 * return success if the overlay is being turned off. */
	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	timing = PIPE(display)->timing;

	if(!timing) {
	    EMGD_ERROR_EXIT("timing is null\n");
	    return -IGD_ERROR_INVAL;
	}

	/*************************************************************************
	 * Ensure the framebuffer dotclock does not exceed the board SKU
	 * max dotclock
	 *************************************************************************/
	/* Make it chipset-specific */
	/* DCT-PC99TA crashes with dotclock > 300MHz */
	if(timing->dclk >= 340000){
		EMGD_ERROR_EXIT("Cannot support dotclock > 340MHz for this SKU");
		return -IGD_ERROR_HWERROR;
	}

	/*************************************************************************
	 * Ensure the overlay surface is ok and can be properly displayed.
	 * This ensures the following is valid:
	 *    - Ensure x1, x2, y1, y2 are pixel aligned
	 *    - 2 pixels or greater in width and height
	 *    - Pixel format is supported by the overlay
	 *    - Pitch is <= 16KB
	 *    - Based on the pixel format, the width is supported
	 *************************************************************************/
	if (!src_surf){
	    EMGD_ERROR_EXIT("src_surf is null");
	    return -IGD_ERROR_INVAL;
	}
	if (!src_rect){
	    EMGD_ERROR_EXIT("src_rect is null");
	    return -IGD_ERROR_INVAL;
	}
	/* Get the minimum size of 1 pixel in width and height for y, u, and v.
	 */
	min_w = 1 << get_uv_shift_x(src_surf->pixel_format);
	min_h = 1 << get_uv_shift_y(src_surf->pixel_format);

	if (((src_rect->x2 - src_rect->x1) < min_w*2) ||
		((src_rect->y2 - src_rect->y1) < min_h*2)) {
		EMGD_ERROR_EXIT(
			"Overlay2 source width or height is < 2 pixels (%dx%d)\n",
			src_rect->x2 - src_rect->x1, src_rect->y2 - src_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	if (FALSE == ovl2_check_pf_tnc(display, src_surf->pixel_format)) {
		EMGD_ERROR_EXIT("Overlay2 source pixel format unsupported (pf:0x%lx)",
			src_surf->pixel_format);
		return -IGD_ERROR_HWERROR;
	}

	if (src_surf->pitch > 16384) {
		EMGD_ERROR_EXIT("Overlay2 source pitch (%d) > 16KB",
			src_surf->pitch);
		return -IGD_ERROR_HWERROR;
	}

	/*************************************************************************
	 * Ensure the location on the framebuffer is ok and can be properly
	 * displayed
	 * This ensures the following is valid:
	 *    - Greater than 1 pixel width and height
	 *    - Will be displayed on screen (not panned off)
	 *************************************************************************/
	if (!dest_rect){
	    EMGD_ERROR_EXIT("dest_rect is null");
	    return -IGD_ERROR_INVAL;
	}
	if (((dest_rect->x2 - dest_rect->x1) <= 1) ||
		((dest_rect->y2 - dest_rect->y1) <= 1)) {
		EMGD_ERROR_EXIT(
			"Overlay2 dest width or height is single pixel (%dx%d)\n",
			dest_rect->x2 - dest_rect->x1, dest_rect->y2 - dest_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	if ((dest_rect->x1 >= timing->width) ||
		(dest_rect->y1 >= timing->height)) {
		EMGD_ERROR_EXIT(
			"Overlay2 dest is panned off the screen (%d,%d)\n",
			dest_rect->x1, dest_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

unsigned int ovl2_send_instr_tnc(
	igd_display_context_t     *display,
	ovl2_reg_tnc_t    *spritec_regs_tnc,
	unsigned int      flags)
{
	unsigned char *mmio = MMIO(display);
	 unsigned long tmp, pipe_reg, pipe_num;
	inter_module_dispatch_t *md;
	platform_context_tnc_t * platform;


	EMGD_TRACE_ENTER;

	/* We dont need the CMD_WAIT_OVL2_TNC instruction coz
	 * our alter_ovl code already querried status
	 * for last flip completion before getting here. See
	 * micro_prepare_ovl2_tnc called by alter_ovl2_tnc.
	 * It calls query overlay before the next flip
	 */

	/*If Overlay+FB Blend is requested and the FB is xRGB
	 *turn on the ARGB format. */
	if(ovl_context->fb_blend_ovl) {
		if((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_ON) {
			tmp = EMGD_READ32(mmio + PLANE(display)->plane_reg);
			if((tmp & 0x3c000000) == 0x18000000) {
				tmp = tmp & 0xc3FFFFFF;
				EMGD_WRITE32(tmp | 0x1c000000, mmio + PLANE(display)->plane_reg);
				EMGD_READ32(mmio + PLANE(display)->plane_reg);
				tmp = EMGD_READ32(mmio + PLANE(display)->plane_reg + 0x1c);
				EMGD_WRITE32(tmp, mmio + PLANE(display)->plane_reg + 0x1c);
			}
		} else {
			tmp =  EMGD_READ32(mmio + PLANE(display)->plane_reg);
			if((tmp & 0x3c000000) == 0x1c000000) {
				tmp = tmp & 0xc3FFFFFF;
				EMGD_WRITE32(tmp | 0x18000000, mmio + PLANE(display)->plane_reg);
				EMGD_READ32(mmio + PLANE(display)->plane_reg);
				tmp = EMGD_READ32(mmio + PLANE(display)->plane_reg + 0x1c);
				EMGD_WRITE32(tmp, mmio + PLANE(display)->plane_reg + 0x1c);
				OS_SLEEP(100);
			}
		}
	}

	/* Send a load register instruction to write the Plane C sprite address
	 * which is the trigger register.
	 * This is an instruction, so it happens after blend, and since it
	 * is an instruction, we do not have to poll waiting for it. */
	EMGD_WRITE32(spritec_regs_tnc->start, mmio + 0x7219C);

	/* Since the ISR bit 0x100 actually doesnt work,
	 * we need to setup a trigger for a VBLANK event
	 * on Pipe-B to guarantee that the Sprite-C had
	 * actually completed its last flip.
	 * (ISR bit was tested on Poulsbo D2 by capturing
	 * timestamps of quick successive alter_overlays..
	 * checked ISR bit directly after the write to Sprite
	 * C Address register in process_vqueue handling..
	 * the ISR bit never changed
	 */

	md = &display->context->mod_dispatch;
	platform = (platform_context_tnc_t *)display->context->
					platform_context;

	pipe_num = PIPE(display)->pipe_num; 
    
	if(pipe_num){ 
		pipe_reg = PIPEB_STAT; 
	} else { 
		pipe_reg = PIPEA_STAT; 
	} 

	if(md && md->set_flip_pending){
		OS_PTHREAD_MUTEX_LOCK(&platform->flip_mutex);
		md->set_flip_pending(MMIO(display), pipe_reg);
		OS_PTHREAD_MUTEX_UNLOCK(&platform->flip_mutex);
	}

	ovl_context->sync2 = WAIT_FOR_FLIP;

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

int alter_ovl2_tnc(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	ovl2_reg_tnc_t spritec_regs_tnc;
	int ret=0;

	EMGD_TRACE_ENTER;

	/* Initialize structure so compilers don't complain */
	OS_MEMSET(&spritec_regs_tnc, 0, sizeof(ovl2_reg_tnc_t));

	if (micro_prepare_ovl2_tnc(display, src_surf, src_rect, dest_rect,
		ovl_info, &spritec_regs_tnc, flags)) {
		return -IGD_ERROR_HWERROR;
	}

	/* Send the instructions to the command queue */
	ret = ovl2_send_instr_tnc(display, &spritec_regs_tnc, flags);
	EMGD_DEBUG("Sprite C= %s",flags & IGD_OVL_ALTER_ON?"ON":"OFF");
	EMGD_TRACE_EXIT;
	return ret;
}

int query_ovl2_tnc(igd_display_h display_h,
	unsigned int flags)
{
	igd_display_context_t *display = (igd_display_context_t *)display_h;
	inter_module_dispatch_t *md;
	platform_context_tnc_t * platform;
	os_alarm_t timeout;
	int ret;
	unsigned long pipe_reg, pipe_num;

	EMGD_TRACE_ENTER;

	switch (flags) {
	case IGD_OVL_QUERY_IS_HW_SUPPORTED:
		/* This is the second overlay, so HW overlay is not supported */
		break;

	case IGD_OVL_QUERY_IS_LAST_FLIP_DONE:
		/* If there no sync to wait on, then the last flip is done, and the
		 * Register Update has occured, simply return TRUE (Flip done).
		 */
		if (!ovl_context->sync2) {
			EMGD_DEBUG("Overlay already synced");
			EMGD_TRACE_EXIT;
			return TRUE;
		}

		/* According to the PBL B-spec, there doesnt seem to exist any bit
		 * for Sprite C Flip-Pending status. Testing 0x20AC in code during
		 * virt queue's REG write shows nothing changed for Bit8. Thus, we
		 * are using state of the VBLANK ISR bit as ovl2 flip status.
		 * Assumption is that if were running 2nd overlay, its either clone
		 * display or VEXT in WinCE. In either case, were not doing full screen
		 * FB flipping, so this check should be 'statefully' accurate
		 */
		md = &display->context->mod_dispatch;
		platform = (platform_context_tnc_t *)display->context->
						platform_context;

		pipe_num = PIPE(display)->pipe_num; 
		if(pipe_num){ 
			pipe_reg = PIPEB_STAT; 
		} else { 
			pipe_reg = PIPEA_STAT; 
		} 

		if(md && md->check_flip_pending){
			ret = OS_PTHREAD_MUTEX_LOCK(&platform->flip_mutex);
			if(md->check_flip_pending(MMIO(display), pipe_reg)){
				OS_PTHREAD_MUTEX_UNLOCK(&platform->flip_mutex);
				EMGD_DEBUG("Overlay2 Sync done but Flip not done");
				return FALSE;
			}
			OS_PTHREAD_MUTEX_UNLOCK(&platform->flip_mutex);
		}

		/* Now that we know the last flip is done and the register update is
		 * complete, set the sync to 0 and return TRUE (Flip done). */
		ovl_context->sync2 = FLIP_DONE;
		break;
	case IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE:
		/* Wait for 200 milliseconds for the last flip to complete.  If not
		 * done in that time, there is likely a hardware problem so return
		 * FALSE. */
		timeout = OS_SET_ALARM(200);
		do {
			if (TRUE ==
				query_ovl2_tnc(display_h, IGD_OVL_QUERY_IS_LAST_FLIP_DONE)) {
				EMGD_TRACE_EXIT;
				return TRUE;
			}
		} while (!OS_TEST_ALARM(timeout));
		EMGD_ERROR_EXIT("Timeout waiting for last flip done");
		return FALSE;
		break;
	case IGD_OVL_QUERY_IS_GAMMA_SUPPORTED:
		return TRUE;
		break;
	case IGD_OVL_QUERY_IS_VIDEO_PARAM_SUPPORTED:
		return TRUE;
		break;
	}

	EMGD_TRACE_EXIT;
	return TRUE;
}



int query_max_size_ovl2_tnc(
	igd_display_h display_h,
	unsigned long pf,
	unsigned int *max_width,
	unsigned int *max_height)
{
	EMGD_TRACE_ENTER;

	/* FIXME: Should this be size of FB? */
	*max_width = 2048;
	*max_height = 2048;

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

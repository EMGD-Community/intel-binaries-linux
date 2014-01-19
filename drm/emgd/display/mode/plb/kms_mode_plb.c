/*
 *-----------------------------------------------------------------------------
 * Filename: kms_mode_plb.c
 * $Revision: 1.2 $
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
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode


#include <igd.h>
#include <igd_pwr.h>
#include <vga.h>
#include <pi.h>
#include <plb/regs.h>
#include "drm_emgd_private.h"
#include "../cmn/mode_dispatch.h"
#include "../cmn/match.h"
#include "mode_plb.h"
#include <math_fix.h>
#include <memory.h>


/*------------------------------------------------------------------------------
 * Function Prototypes
 *----------------------------------------------------------------------------*/
int kms_mode_get_stride_stereo_plb(igd_display_pipe_t *pipe,
		igd_display_plane_t *plane,
		unsigned long *stride,
		unsigned long *stereo,
		unsigned long flags);
void kms_program_pipe_vga_plb(emgd_crtc_t *emgd_crtc);
static void kms_program_pipe_plb(emgd_crtc_t *emgd_crtc);
static void kms_set_pipe_pwr_plb(emgd_crtc_t *emgd_crtc, unsigned long enable);
static void kms_program_plane_plb(emgd_crtc_t *emgd_crtc, unsigned long status);
static void kms_set_plane_pwr_plb(emgd_crtc_t *emgd_crtc, unsigned long enable);
static int set_color_correct_plb(emgd_crtc_t *emgd_crtc);


extern int kms_program_clock_plb(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock, unsigned long dclk);
extern int wait_for_vblank_plb(unsigned char *mmio,
	unsigned long pipe_reg);
extern void kms_program_pipe_vga(emgd_crtc_t *emgd_crtc,
	igd_timing_info_t *timings);
extern void disable_vga_plb(unsigned char *mmio);


/*------------------------------------------------------------------------------
 * Global Variables
 *----------------------------------------------------------------------------*/
extern mode_data_plb_t device_data_plb[1];


/*------------------------------------------------------------------------------
 * KMS Dispatch Table
 *----------------------------------------------------------------------------*/
mode_kms_dispatch_t mode_kms_dispatch_plb = {
	kms_program_pipe_plb,
	kms_set_pipe_pwr_plb,
	kms_program_plane_plb,
	kms_set_plane_pwr_plb,
	NULL,
	NULL,
	NULL,                       /* kms_get_vblank_counter */
	kms_match_mode,
};


/*!
 * Get the stride and stereo values based on the display.  This is also used
 * by the MI instructions.
 *
 * @param pipe Pointer to hardware device instance data
 * @param plane Pointer to hardware device instance data
 * @param stride
 * @param stereo
 * @param flags Should the stereo be for the frontbuffer or backbuffer?
 *
 * @return stride - Stride of the display
 * @return stereo - Stereo address of the display
 */
int kms_mode_get_stride_stereo_plb(igd_display_pipe_t *pipe,
	igd_display_plane_t *plane,
	unsigned long *stride,
	unsigned long *stereo,
	unsigned long flags)
{
	unsigned long pitch = plane->fb_info->screen_pitch;
	igd_timing_info_t *timing = pipe->timing;
	unsigned long base_offset;

	base_offset = plane->fb_info->visible_offset;

	*stride = pitch;
	*stereo = 0;

	/* For field replication, valid for interlaced modes only
	 *     set stereo = fb_base,
	 *         stride = pitch
	 */
	if (timing->mode_info_flags & IGD_SCAN_INTERLACE) {

		if(timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* Interlaced + Line double flags means field replication.
			 * same lines are sent for both fields. Program the
			 * second eye to be same as the first.
			 */
			*stereo = base_offset;
		} else {
			/* Regular interlaced. Second eye starts on line 2.
			 * Skip every other line.
			 */
			*stereo = base_offset + pitch;
			*stride = pitch * 2;
		}
	}

	return 0;
}



/*!
 *
 * @param emgd_crtc
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL if color attributes not found
 */
static int set_color_correct_plb(emgd_crtc_t *emgd_crtc)
{
	const int        MID_PIXEL_VAL    = 125;
	const int        MAX_PIXEL_VAL    = 255;
	const int        NUM_PALETTE_ENTRIES = 256;

	struct drm_device  *dev = NULL;
	igd_context_t      *context = NULL;
	igd_display_port_t *port = NULL;
	igd_display_pipe_t *pipe = NULL;
	struct drm_encoder *encoder = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;

	unsigned int     gamma_r_max_24i_8f, gamma_r_min_24i_8f;
	unsigned int     gamma_g_max_24i_8f, gamma_g_min_24i_8f;
	unsigned int     gamma_b_max_24i_8f, gamma_b_min_24i_8f;
	unsigned int     new_gamma_r_24i_8f, new_gamma_g_24i_8f;
	unsigned int     new_gamma_b_24i_8f;
	unsigned int     gamma_normal_r_24i_8f, gamma_normal_g_24i_8f;
	unsigned int     gamma_normal_b_24i_8f;
	int              brightness_factor_r, brightness_factor_g;
	int              brightness_factor_b;
	int              contrast_factor_r, contrast_factor_g;
	int              contrast_factor_b;

	unsigned int      *palette;
	unsigned int      i;

	igd_range_attr_t *gamma_attr      = NULL, *contrast_attr = NULL;
	igd_range_attr_t *brightness_attr = NULL;
	igd_attr_t       *hal_attr_list   = NULL;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			break;
		}
	}
	if (!port) {
		EMGD_ERROR_EXIT("No port being used.");
		return -IGD_ERROR_INVAL;
	}
	hal_attr_list  = port->attributes;

	/* Using OS_ALLOC to avoid using > 1024 on stack (frame size warning ) */
	palette = OS_ALLOC(sizeof (unsigned int) * NUM_PALETTE_ENTRIES);

	/* start with a fresh palette */
	for (i = 0; i < NUM_PALETTE_ENTRIES; i++) {
		palette[i] = (i << 16) | (i << 8) | i;
	}

	/* get a pointer to gamma, contrast, and brightness attr */
	i = 0;

	while (PD_ATTR_LIST_END != hal_attr_list[i].id) {
		switch (hal_attr_list[i].id) {
		case PD_ATTR_ID_FB_GAMMA:
			gamma_attr      = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		case PD_ATTR_ID_FB_BRIGHTNESS:
			brightness_attr = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		case PD_ATTR_ID_FB_CONTRAST:
			contrast_attr   = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		default:
			break;
		}

		i++;
	}

	if(!gamma_attr || !brightness_attr || !contrast_attr) {
		EMGD_ERROR("Color Correction Atrributes not found!");
		return -IGD_ERROR_INVAL;
	}

	/* Get the max and min */
	gamma_r_max_24i_8f = ((gamma_attr->max >> 16) & 0xFF) << 3;
	gamma_g_max_24i_8f = ((gamma_attr->max >>  8) & 0xFF) << 3;
	gamma_b_max_24i_8f =  (gamma_attr->max        & 0xFF) << 3;

	gamma_r_min_24i_8f = ((gamma_attr->min >> 16) & 0xFF) << 3;
	gamma_g_min_24i_8f = ((gamma_attr->min >>  8) & 0xFF) << 3;
	gamma_b_min_24i_8f =  (gamma_attr->min        & 0xFF) << 3;

	/* The new gamma values are in 3i.5f format, but we must convert it
	 * to 24i.8f format before passing it to OS_POW_FIX
	 */
	new_gamma_r_24i_8f = ((gamma_attr->current_value >> 16) & 0xFF) << 3;
	new_gamma_g_24i_8f = ((gamma_attr->current_value >> 8) & 0xFF) << 3;
	new_gamma_b_24i_8f = (gamma_attr->current_value & 0xFF) << 3;

	/* make sure the new gamma is within range */
	new_gamma_r_24i_8f = OS_MIN(gamma_r_max_24i_8f, new_gamma_r_24i_8f);
	new_gamma_r_24i_8f = OS_MAX(gamma_r_min_24i_8f, new_gamma_r_24i_8f);
	new_gamma_g_24i_8f = OS_MIN(gamma_g_max_24i_8f, new_gamma_g_24i_8f);
	new_gamma_g_24i_8f = OS_MAX(gamma_g_min_24i_8f, new_gamma_g_24i_8f);
	new_gamma_b_24i_8f = OS_MIN(gamma_b_max_24i_8f, new_gamma_b_24i_8f);
	new_gamma_b_24i_8f = OS_MAX(gamma_b_min_24i_8f, new_gamma_b_24i_8f);


	gamma_normal_r_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_r_24i_8f);

	gamma_normal_g_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_g_24i_8f);

	gamma_normal_b_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_b_24i_8f);

	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		unsigned int new_gamma;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* Note that we do not try to calculate the gamma if it
		 * is 1.0, e.g. 0x100.  This is to avoid round-off errors
		 */

		/* red: calculate and make sure the result is within range */
		if (0x100 != new_gamma_r_24i_8f) {
			cur_color  = (cur_palette >> 16) & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_r_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_r_24i_8f;
			palette[i] &= 0x00FFFF;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF) << 16;
		}

		/* green: calculate and make sure the result is within range */
		if (0x100 != new_gamma_g_24i_8f) {
			cur_color  = (cur_palette >> 8) & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_g_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_g_24i_8f;
			palette[i] &= 0xFF00FF;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF) << 8;
		}

		/* blue: calculate and make sure the result is within range */
		if (0x100 != new_gamma_b_24i_8f) {
			cur_color  = cur_palette & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_b_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_b_24i_8f;
			palette[i] &= 0xFFFF00;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF);
		}
	}


	/* Brightness correction */
	brightness_factor_r = (brightness_attr->current_value >> 16) & 0xFF;
	brightness_factor_g = (brightness_attr->current_value >> 8) & 0xFF;
	brightness_factor_b = brightness_attr->current_value & 0xFF;

	/* The factors are offset by 0x80 because 0x80 is 0 correction */
	brightness_factor_r -= 0x80;
	brightness_factor_g -= 0x80;
	brightness_factor_b -= 0x80;

	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		int          new_pixel_val;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* red: calculate and make sure the result is within range */
		cur_color     =  (cur_palette >> 16) & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_r;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0x00FFFF;
		palette[i]    |= (OS_MAX(new_pixel_val, 0) & 0xFF) << 16;

		/* green: calculate and make sure the result is within range */
		cur_color     =  (cur_palette >> 8) & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_g;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0xFF00FF;
		palette[i]    |= (OS_MAX(new_pixel_val, 0) & 0xFF) << 8;

		/* blue: calculate and make sure the result is within range */
		cur_color     =  cur_palette & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_b;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0xFFFF00;
		palette[i]    |= OS_MAX(new_pixel_val, 0) & 0xFF;
	}


	/* contrast correction */
	contrast_factor_r = (contrast_attr->current_value >> 16) & 0xFF;
	contrast_factor_g = (contrast_attr->current_value >> 8) & 0xFF;
	contrast_factor_b = contrast_attr->current_value & 0xFF;

	/* make sure values are within range */
	contrast_factor_r -= 0x80;
	contrast_factor_g -= 0x80;
	contrast_factor_b -= 0x80;


	/* We're doing integer division in this loop using 16i.16f
	 * integers.  The result will then be converted back into a
	 * regular, 32-bit integer
	 */
	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		int new_pixel_val;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* red: calculate and make sure the result is within range */
		if (0 != contrast_factor_r ) {
			cur_color     = (cur_palette >> 16) & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_r);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0x00FFFF;  /* clear out the R color */
			palette[i]    |=  (OS_MAX(new_pixel_val, 0) & 0xFF) << 16;
		}

		/* green: calculate and make sure the result is within range */
		if (0 != contrast_factor_g ) {
			cur_color     = (cur_palette >> 8) & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_g);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0xFF00FF;  /* clear out the G color */
			palette[i]    |=  (OS_MAX(new_pixel_val, 0) & 0xFF) << 8;
		}

		/* blue: calculate and make sure the result is within range */
		if (0 != contrast_factor_b) {
			cur_color     = cur_palette & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_b);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0xFFFF00;  /* clear out the B color */
			palette[i]    |=   OS_MAX(new_pixel_val, 0) & 0xFF;
		}
	}


	/* write the new values in the palette */
	for (i = 0; i < NUM_PALETTE_ENTRIES; i++) {
		EMGD_WRITE32(palette[i], context->device_context.virt_mmadr +
			pipe->palette_reg + i*4);
	}
	OS_FREE(palette);

	return 0;
}



/*!
 *
 * @param emgd_crtc Pointer to hardware device instance data
 *
 * @return void
 */
void kms_program_pipe_vga_plb(
	emgd_crtc_t *emgd_crtc)
{
	struct drm_device  *dev          = NULL;
	igd_timing_info_t  *timing       = NULL;
	igd_display_pipe_t *pipe         = NULL;
	igd_context_t      *context      = NULL;
	struct drm_encoder *encoder      = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	igd_display_port_t *port         = NULL;
	unsigned long vga_control;
	unsigned long upscale = 0;
	int centering = 1;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			break;
		}
	}

#ifdef CONFIG_MICRO
	/*
	 * We cannot set a VGA mode unless the display planes are turned off.
     * This becomes evident during a Windows BSOD.  Since neither Windows
     * nor IEGD got a chance to turn off these registers, and the VGA mode was
     * set by the VBIOS, the screen gets corrupted.  In order to fix this
     * problem, we will turn the cursor and display planes here.
     *
     * Note: Removing previous partial-fix in favor of this complete one.
	 */
    /* Cursor A */
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x70080);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x70084);
    /* Cursor B */
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x700C0);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x700C4);
    /* Display A */
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x70180);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x70184);
    /* Display B */
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x71180);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x71184);
    /* Display C */
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x72180);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + 0x72184);
#endif

	/*
	 * VGA Plane can attach to only one pipe at a time. LVDS can
	 * only attach to pipe B. We need to use the display passed to
	 * determine the pipe number to use. (Plba is same as Alm).
	 */

	/*
	 * We can come here with following cases:
	 *   1. magic->vga    CRT, DVI type displays
	 *   2. native->vga   int-lvds, and up-scaling lvds displays
	 *   3. pipe->vga     TV and other unscaled-lvds displays
	 */
	vga_control = EMGD_READ32(context->device_context.virt_mmadr + 0x71400);
	vga_control &= 0x18e3ff00;
	vga_control |= 0x8e;

	timing = pipe->timing;
	if(!timing->extn_ptr) {
		EMGD_ERROR_EXIT("No Extension pointer in program_pipe_vga_plb");
		return;
	}

	if (port) {
		/* Find UPSCALING attr value*/
		pi_pd_find_attr_and_value(port,
			PD_ATTR_ID_PANEL_FIT,
			0,/*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);
		/* this PI func will not modify value of upscale if attr does not exist */
	}

	/* magic->vga or native->vga cases */
	if ((timing->width == 720 && timing->height == 400) || upscale) {
		centering = 0;
	}

	/* Enable border */
	if((timing->width >= 800) && !upscale) {
		EMGD_DEBUG("Enable VGA Border");
		vga_control |= (1L<<26);
	}

	if(timing->width == 640) {
		EMGD_DEBUG("Enable Nine Dot Disable");
		vga_control |= (1L<<18);
	}

	if(centering) {
		EMGD_DEBUG("Enable VGA Center Centering");
		vga_control |= 1L<<24;

		if(timing->height >= 960) {
			if(timing->width >= 1280) {
				EMGD_DEBUG("Enable VGA 2x (Nine Dot Disable)");
				vga_control |= (1L<<30) | (1L<<18);
			}
		}
	} else {
		if (port) {
			if(port->port_type == IGD_PORT_LVDS) {
				EMGD_DEBUG("Enable VGA Upper-Left Centering & Nine Dot Disable");
				vga_control |= (1L<<25 | (1L<<18));
			} else if (upscale) {
				EMGD_DEBUG("Enable VGA Center Upper-left for upscale ports");
				vga_control |= 1L<<25;
			}
		}
	}

	if(pipe->pipe_num) {
		vga_control |= 1L<<29;
	}

	kms_program_pipe_vga(emgd_crtc, (igd_timing_info_t *)timing->extn_ptr);
	EMGD_WRITE32(vga_control, context->device_context.virt_mmadr + 0x71400);

	EMGD_TRACE_EXIT;
	return;
}



/**
 * kms_set_pipe_pwr_plb
 *
 * Turns the pipe ON or OFF depending on the input
 *
 * @param emgd_crtc (IN) specifies the pipe to change
 * @param enable    (IN) TRUE to enable pipe, FALSE to disable
 *
 * @return
 */
static void kms_set_pipe_pwr_plb(emgd_crtc_t *emgd_crtc, unsigned long enable)
{
	unsigned long       pipe_conf;
	struct drm_device  *dev;
	igd_display_pipe_t *pipe;
	igd_context_t      *context;


	EMGD_TRACE_ENTER;


	dev       = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context   = ((drm_emgd_priv_t *)dev->dev_private)->context;

	pipe      = emgd_crtc->igd_pipe;
	pipe_conf = device_data_plb->pipe_preserve &
		EMGD_READ32(context->device_context.virt_mmadr + pipe->pipe_reg);


	/* Do nothing if current power state is same as what we want to set */
	/* The PIPE_ENABLE bit is at bit-position 31 */
	if ( (enable << 31) == (pipe_conf & PIPE_ENABLE) ){

		EMGD_TRACE_EXIT;
		return;
	}


	if (!enable) {
		/* Disable pipe */
		EMGD_WRITE32(pipe_conf & ~PIPE_ENABLE,
			context->device_context.virt_mmadr + pipe->pipe_reg);

		EMGD_DEBUG("Set Pipe Power: OFF");

	} else {
		/* Enable pipe */
		EMGD_WRITE32(pipe_conf | PIPE_ENABLE,
			context->device_context.virt_mmadr + pipe->pipe_reg);

		EMGD_DEBUG("Set Pipe Power: ON");
	}


	EMGD_TRACE_EXIT;
	return;
}



/*!
 * This function programs the Timing registers and clock registers and
 * other control registers for PIPE.
 *
 * @param emgd_crtc
 * @param status
 *
 * @return void
 */
static void kms_program_pipe_plb(emgd_crtc_t *emgd_crtc)
{
	unsigned long       timing_reg;
	unsigned long       pipe_conf;
	unsigned long       hactive, vactive;
	igd_timing_info_t  *pTimings;
	unsigned long       temp;
	struct drm_device  *dev     = NULL;
	igd_context_t      *context = NULL;
	igd_display_pipe_t *pipe    = NULL;
	igd_display_port_t *port    = NULL;
	int i;

	EMGD_TRACE_ENTER;


	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	EMGD_DEBUG("Device power state: D%ld", context->device_context.power_state);

	pipe_conf = device_data_plb->pipe_preserve &
		EMGD_READ32(context->device_context.virt_mmadr + pipe->pipe_reg);

	/* Reset the plane of this pipe back to NULL, it will be set on the
	 * call to program_plane, which is ok, since program_pipe occurs
	 * before program_plane */
	pipe->plane = NULL;

	pTimings = pipe->timing;

	/*
	 * If the mode is VGA and the PD says it handles all VGA modes without
	 * reprogramming then just set the mode and leave centering off.
	 */
	if(pTimings->mode_info_flags & IGD_MODE_VESA) {
		if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
			/* Pipe timings and clocks are not used but it must be on anyway */
			EMGD_WRITE32(pipe_conf | 0x80000000,
				context->device_context.virt_mmadr + pipe->pipe_reg);

			kms_program_pipe_vga_plb(emgd_crtc);
			return;
		}
	}

	/* Program dot clock divisors. */
	kms_program_clock_plb(emgd_crtc, pipe->clock_reg, pTimings->dclk);

	/* Program timing registers for the pipe */
	timing_reg = pipe->timing_reg;
	if (pTimings->mode_info_flags & IGD_PIXEL_DOUBLE) {
		hactive = (unsigned long)pTimings->width*2 - 1;
	} else {
		hactive = (unsigned long)pTimings->width - 1;
	}

	if (pTimings->mode_info_flags & IGD_LINE_DOUBLE) {
		if (pTimings->mode_info_flags & IGD_SCAN_INTERLACE) {
			vactive = (unsigned long)pTimings->height - 1;
		} else {
			vactive = (unsigned long)pTimings->height*2 - 1;
		}
	} else {
		if (pTimings->mode_info_flags & IGD_SCAN_INTERLACE) {
			vactive = (unsigned long)pTimings->height/2 - 1;
		} else {
			vactive = (unsigned long)pTimings->height - 1;
		}
	}

	/*
	 * DPLL should be on at this point which is required for touching
	 * the palette.
	 */
	/* reset the palette */
	for (i = 0; i < 256; i++) {
		EMGD_WRITE32(((i<<16) | (i<<8) | i),
			context->device_context.virt_mmadr + pipe->palette_reg + i*4);
	}

	/* apply color correction */
	for( i = 0; PD_ATTR_LIST_END != port->attributes[i].id; i++ ) {

		if ((PD_ATTR_ID_FB_GAMMA      == (port->attributes[i].id)) ||
			(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)) ||
			(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)))  {

			set_color_correct_plb(emgd_crtc);
		}
	}


	/*
	 * NOTE: For size reasons the timng table contains unsigned short
	 * values. Don't shift them past 16. Use a temp instead.
	 * All register offsets and bit shift are verified for Napa
	 */
	temp = ((unsigned long)pTimings->htotal << 16) | hactive;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg);

	temp = ((unsigned long)pTimings->hblank_end << 16) |
		(unsigned long)pTimings->hblank_start;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x04);

	temp = ((unsigned long)pTimings->hsync_end << 16) |
		(unsigned long)pTimings->hsync_start;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x08);

	temp = ((unsigned long)pTimings->vtotal << 16) | vactive;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x0C);

	temp = ((unsigned long)pTimings->vblank_end << 16) |
		(unsigned long)pTimings->vblank_start;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x10);

	temp = ((unsigned long)pTimings->vsync_end << 16) |
		(unsigned long)pTimings->vsync_start;
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x14);

	/*
	 * If there is a linked mode it is either the VGA or a scaled
	 * mode. If it is scaled then we need to use it as the source size.
	 */
	if(pTimings->extn_ptr) {
		igd_timing_info_t *scaled_timings =
			(igd_timing_info_t *)pTimings->extn_ptr;
		if((scaled_timings->mode_info_flags & IGD_MODE_VESA) &&
			(scaled_timings->mode_number <= VGA_MODE_NUM_MAX)) {
			temp = (hactive << 16) | vactive;
		} else {
			temp = (unsigned long)scaled_timings->width  - 1;
			temp = (temp << 16) |
				(unsigned long)(scaled_timings->height - 1);
		}
	} else {
		temp = (hactive << 16) | vactive;
	}
	EMGD_WRITE32(temp, context->device_context.virt_mmadr + timing_reg + 0x1C);

	/* Set other registers */

	/*
	 * FIXME: max_dclk needs to be determined from core clock
	 * at init time. 915 etc has several skus with different
	 * clocks for the same device ID.
	 *
	 */

	/* These values are derived from the Poulsbo B-Spec as
	 * the suggested values */
	EMGD_WRITE32(device_data_plb->fw_blc1,
		context->device_context.virt_mmadr + FW_BLC1);
	EMGD_WRITE32(device_data_plb->fw_blc2,
		context->device_context.virt_mmadr + FW_BLC2);
	EMGD_WRITE32(device_data_plb->fw_blc3,
		context->device_context.virt_mmadr + FW_BLC3);
	EMGD_WRITE32(device_data_plb->fw_self,
		context->device_context.virt_mmadr + FW_BLC_SELF);
	EMGD_WRITE32(device_data_plb->dsp_arb,
		context->device_context.virt_mmadr + PIPEA_DISP_ARB_CTRL);

	/* The SGX 2D engine can saturate the memory bus and starve
	 * the display engine causing visible screen tearing.
	 * This reduces the priority of the SGX vs. display engine
	 */
	temp = EMGD_READ32(context->device_context.virt_mmadr + G_DEBUG);
	EMGD_WRITE32((temp | (1 << 11)),
		context->device_context.virt_mmadr + G_DEBUG);

	EMGD_WRITE32(pipe_conf,
		context->device_context.virt_mmadr + pipe->pipe_reg);

	/*
	 * Set the VGA address range to 0xa0000 so that a normal (not VGA)
	 * mode can be accessed through 0xa0000 in a 16bit world.
	 */
	WRITE_AR(context->device_context.virt_mmadr, 0x10, 0xb);
	WRITE_VGA(context->device_context.virt_mmadr, GR_PORT, 0x06, 0x5);
	WRITE_VGA(context->device_context.virt_mmadr, GR_PORT, 0x10, 0x1);

	if(pTimings->extn_ptr) {
		/* This means either internal scaling (LVDS) or centered VGA */
		pTimings = pTimings->extn_ptr;
		if(pTimings->extn_ptr) {
			/* This is both the scaled and centered VGA */
			pTimings = pTimings->extn_ptr;
		}
		if(pTimings->mode_info_flags & IGD_MODE_VESA) {
			if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
				kms_program_pipe_vga_plb(emgd_crtc);
			}
		}
	}

	EMGD_TRACE_EXIT;
	return;
}



/*!
 * kms_set_plane_pwr_plb
 *
 * Enables or disables the plane associated with the given CRTC
 *
 * @param enable [IN] turns the plane on or off
 *
 * @return void
 */
static void kms_set_plane_pwr_plb(emgd_crtc_t *emgd_crtc, unsigned long enable)
{
	unsigned long           plane_control;
	unsigned long           plane_reg;
	struct drm_device      *dev         = NULL;
	igd_display_plane_t    *plane       = NULL;
	igd_display_pipe_t     *pipe        = NULL;
	igd_context_t          *context     = NULL;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program Plane: %s", enable?"ENABLE":"DISABLE");

	pipe    = emgd_crtc->igd_pipe;
	dev     = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	plane   = PLANE(pipe->owner);

	if (!plane) {
		EMGD_ERROR_EXIT("Trying to set power to a plane that is not tied "
			" to a crtc.");
		return;
	}

	/* In case a plane update is already in progress */
	wait_for_vblank_plb(context->device_context.virt_mmadr, pipe->pipe_reg);

	/* Get the current value of the plane control register */
	plane_reg     = plane->plane_reg;
	plane_control = EMGD_READ32(context->device_context.virt_mmadr + plane_reg);

	if(plane->plane_reg == DSPACNTR) {
		plane_control &= device_data_plb->plane_a_preserve;
	} else { /* if it's plane b or plane c */
		plane_control &= device_data_plb->plane_b_c_preserve;
	}


	if((enable == FALSE) ||
		(context->device_context.power_state != IGD_POWERSTATE_D0)) {

		/*
		 * Note: The vga programming code does not have an "off". So
		 * when programming the plane to off we make sure VGA is off
		 * as well.
		 */
		disable_vga_plb(context->device_context.virt_mmadr);

		/*
		 * To turn off plane A or B, the program have to trigger the plane A
		 * or B start register.  Or else, it will not work.
		 */
		plane_control &= 0xEFFFFFFF;

		EMGD_WRITE32(plane_control,
						context->device_context.virt_mmadr + plane_reg);

		EMGD_WRITE32(EMGD_READ32(context->device_context.virt_mmadr +
			plane_reg + DSP_START_OFFSET),
			context->device_context.virt_mmadr + plane_reg + DSP_START_OFFSET);
	} else {
		/* Enable Pipe */
		plane_control |= 0x80000000;

		EMGD_WRITE32(plane_control,
			context->device_context.virt_mmadr + plane_reg);
	}


	wait_for_vblank_plb(context->device_context.virt_mmadr, pipe->pipe_reg);
	EMGD_TRACE_EXIT;

	return;
}



/*!
 * Program Display Plane Values.
 *
 * @param emgd_crtc Pointer to hardware device instance data
 *
 * @return void
 */
static void kms_program_plane_plb(emgd_crtc_t *emgd_crtc,
	unsigned long status)
{
	unsigned long stereo;
	unsigned long stride;
	unsigned long size;
	unsigned long plane_control;
	unsigned long plane_reg;
	unsigned long start_addr_reg;
	igd_timing_info_t      *timing  = NULL;
	struct drm_device      *dev     = NULL;
	igd_display_plane_t    *plane   = NULL;
	igd_display_pipe_t     *pipe    = NULL;
	igd_context_t          *context = NULL;
	igd_framebuffer_info_t *fb_info = NULL;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	plane   = PLANE(pipe->owner);
	fb_info = plane->fb_info;
	plane_reg = plane->plane_reg;
	start_addr_reg = DSPAADDR;

	EMGD_DEBUG("Device power state: D%ld", context->device_context.power_state);

	wait_for_vblank_plb(context->device_context.virt_mmadr, pipe->pipe_reg);

	plane_control = EMGD_READ32(context->device_context.virt_mmadr + plane_reg);
	if(plane->plane_reg == DSPACNTR) {
		plane_control &= device_data_plb->plane_a_preserve;
	}
	else { /* if it's plane b or plane c */
		plane_control &= device_data_plb->plane_b_c_preserve;
		start_addr_reg = 0x71184;
	}

	if((status == FALSE) ||
		(context->device_context.power_state != IGD_POWERSTATE_D0)) {

		/*
		 * Note: The vga programming code does not have an "off". So
		 * when programming the plane to off we make sure VGA is off
		 * as well.
		 */
		disable_vga_plb(context->device_context.virt_mmadr);

		/*
		 * To turn off plane A or B, the program have to triger the plane A or B
		 * start register.  Or else, it will not work.
		 */
		EMGD_WRITE32(plane_control, context->device_context.virt_mmadr + plane_reg);
		EMGD_WRITE32(EMGD_READ32(context->device_context.virt_mmadr + start_addr_reg),
			context->device_context.virt_mmadr + start_addr_reg);

		wait_for_vblank_plb(context->device_context.virt_mmadr, pipe->pipe_reg);
		return;
	}
	/*
	 * Note: The very first pass through this function will be with
	 * status false and timings == NULL. Don't use the timings before
	 * the check above.
	 */
	timing = pipe->timing;
	/* There is a special case code for legacy VGA modes */
	while (timing->extn_ptr) {
		timing = (igd_timing_info_t *)timing->extn_ptr;
	}
	if(MODE_IS_VGA(timing)) {
		kms_program_plane_vga(context->device_context.virt_mmadr, timing);
		return;
	}

	disable_vga_plb(context->device_context.virt_mmadr);

	size = (((unsigned long)timing->height - 1)<<16) |
		(unsigned long)(timing->width - 1);

	/* enable plane, select pipe, enable gamma correction logic */
	plane_control |= 0x80000000 | (pipe->pipe_num<<24);
	pipe->plane = plane;
#ifndef CONFIG_MICRO
	plane_control |= (1<<30);
#endif

	/* Here the settings:
	 *   If line + pixel dbling, set 21,20 to 01b, and set Horz Multiply
	 *   If line dbling only,    set 21,20 to 11b
	 *   If pixel dbling only,   set 21,20 to 00b, but set Horz Multiply
	 *   If no doubling,         set 21,20 to 00b (no Horz Multiply)
	 * For pixel doubling
	 *           --> both progressive/interlaced modes
	 * For Line doubling
	 *           --> progressive modes only
	 */

	if (!(timing->mode_info_flags & IGD_SCAN_INTERLACE)) {
		/* Line doubling in progressive mode requires special bits */
		if (timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* BIT 20 for line & pixel doubling*/
			plane_control |= BIT20;
			/* check later, if no pixel doubling, set bit 21 too*/
		}
	}
	if (timing->mode_info_flags & IGD_PIXEL_DOUBLE) {
		/* Horz pixel multiply must be set for double */
		plane_control |= BIT11;
		/* TODO -> Plba can more than double,
		It can 3X, 4X etc. These arent exposed now */
	}
	else if(plane_control & BIT20){
		/* For line ONLY doubling, set bit 21 also '1' */
		plane_control |= BIT21;
	}

	kms_mode_get_stride_stereo_plb(pipe, plane, &stride, &stereo, 0);

	/* set color depth */
	switch (IGD_PF_DEPTH(fb_info->pixel_format)) {
	case PF_DEPTH_8:
		plane_control |= BIT27 | BIT30;
		break;
	case PF_DEPTH_16:
		plane_control |= BIT28 | BIT26;
		break;
	default:
	case PF_DEPTH_32:
		plane_control |= BIT28 | BIT27;
		break;
	}

	if(fb_info->flags & IGD_ENABLE_DISPLAY_GAMMA) {
		plane_control |= (BIT30);
	}

	EMGD_DEBUG(" Plane Control = 0x%lx", plane_control);
	EMGD_DEBUG(" Plane Base = 0x%lx", fb_info->visible_offset);
	EMGD_DEBUG(" Plane Pitch = 0x%lx", stride);
	EMGD_DEBUG(" Plane Size = 0x%lx", size);

	EMGD_WRITE32(stride, context->device_context.virt_mmadr + plane_reg + DSP_STRIDE_OFFSET);
	/*
	 * In reality this only exists for plane B. It doesn't seem to hurt
	 * plane A so just do it anyway and save us another case.
	 */
	EMGD_WRITE32(size, context->device_context.virt_mmadr + plane_reg + DSP_SIZE_OFFSET);

	/*EMGD_WRITE32(stereo, MMIO(display) + plane_reg + DSP_STEREO_OFFSET);
		- This register is Reserved ON plba */
	EMGD_WRITE32(fb_info->visible_offset,
		context->device_context.virt_mmadr + plane_reg + DSP_START_OFFSET);

	/* It seems we need push or trigger plane A/B to start to work
	 * on Poulsbo, especially for sDVO port. Let's write plane control
	 * register and start address register at last.
	 */
	EMGD_WRITE32(plane_control, context->device_context.virt_mmadr + plane_reg);
	EMGD_WRITE32(EMGD_READ32(context->device_context.virt_mmadr + start_addr_reg),
		context->device_context.virt_mmadr + start_addr_reg);

	wait_for_vblank_plb(context->device_context.virt_mmadr, pipe->pipe_reg);

	EMGD_TRACE_EXIT;
	return;
}


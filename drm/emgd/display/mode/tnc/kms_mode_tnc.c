/*
 *-----------------------------------------------------------------------------
 * Filename: kms_mode_tnc.c
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
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <igd_pwr.h>
#include <utils.h>
#include <vga.h>
#include <pi.h>

#include <tnc/regs.h>
#include <tnc/context.h>
#include <tnc/mi.h>
#include <tnc/instr.h>
#include <tnc/igd_tnc_wa.h>
#include "drm_emgd_private.h"

#include "../cmn/mode_dispatch.h"
#include "../cmn/match.h"
#include "mode_tnc.h"
#include <math_fix.h>
#include <memory.h>


/*------------------------------------------------------------------------------
 * Function Prototypes
 *----------------------------------------------------------------------------*/
int kms_mode_get_stride_stereo_tnc(igd_display_pipe_t *pipe,
	igd_display_plane_t *plane,
	unsigned long *stride,
	unsigned long *stereo,
	unsigned long flags);
void kms_program_pipe_vga_tnc(emgd_crtc_t *emgd_crtc);
static void kms_program_pipe_tnc(emgd_crtc_t *emgd_crtc);
static void kms_set_pipe_pwr_tnc(emgd_crtc_t *emgd_crtc, unsigned long enable);
static void kms_program_plane_tnc(emgd_crtc_t *emgd_crtc, unsigned long status);
static void kms_set_plane_pwr_tnc(emgd_crtc_t *emgd_crtc, unsigned long enable);
static int  kms_program_port_tnc(emgd_encoder_t *emgd_encoder,
	unsigned long status);
static int  kms_program_port_lvds_tnc(emgd_encoder_t *emgd_encoder,
	unsigned long status);
static int  kms_program_port_sdvo_tnc(emgd_encoder_t *emgd_encoder,
	unsigned long status);
static int  kms_post_program_port_tnc(emgd_encoder_t *emgd_encoder,
	unsigned long status);
static u32  kms_get_vblank_counter_tnc(emgd_crtc_t *emgd_crtc);


static int set_color_correct_tnc(emgd_crtc_t *emgd_crtc);

extern unsigned long get_port_type(int crtc_id);
extern void wait_pipe(unsigned long pt, unsigned long pipe_reg, unsigned long check_on_off);
extern void disable_vga_tnc (unsigned char *mmio);
extern int write_bunit_tnc(unsigned long reg, unsigned long val);
extern int read_bunit_tnc(unsigned long reg, unsigned long *val);
extern int kms_program_clock_tnc(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock, unsigned long dclk);
extern int wait_for_vblank_tnc(unsigned long pipe_reg);
extern void initialize_ST_bridge(igd_context_t *context,
	igd_display_port_t *port);


/*------------------------------------------------------------------------------
 * Global Variables
 *----------------------------------------------------------------------------*/
extern int flag_basic_htotal_formula_wa;
extern int flag_turn_off_port_wa;
extern int flag_enable_tuning_wa;
extern unsigned long ports_tnc[2];
extern int vphase;
extern tnc_wa_timing_t tune;
extern mode_data_tnc_t device_data_tnc[1];


/*------------------------------------------------------------------------------
 * KMS Dispatch Table
 *----------------------------------------------------------------------------*/
mode_kms_dispatch_t mode_kms_dispatch_tnc = {
	kms_program_pipe_tnc,
	kms_set_pipe_pwr_tnc,
	kms_program_plane_tnc,
	kms_set_plane_pwr_tnc,
	kms_program_port_tnc,
	kms_post_program_port_tnc,
	kms_get_vblank_counter_tnc,
	kms_match_mode,
};



/*!
 * Get the stride and stereo values based on the display.
 *
 * @param plane Pointer to hardware device instance data
 * @param pipe  Pointer to hardware device instance data
 * @param flags Should the stereo be for the frontbuffer or backbuffer?
 *
 * @return stride - Stride of the display
 * @return stereo - Stereo address of the display
 */
int kms_mode_get_stride_stereo_tnc(igd_display_pipe_t *pipe,
		igd_display_plane_t *plane,
		unsigned long *stride,
		unsigned long *stereo,
		unsigned long flags)
{
	unsigned long pitch = plane->fb_info->screen_pitch;
	igd_timing_info_t *timing = pipe->timing;
	unsigned long base_offset;

	EMGD_TRACE_ENTER;

	base_offset = plane->fb_info->visible_offset;
	*stride = pitch;
	*stereo = 0;

	/* For field replication, valid for interlaced modes only
	 *     set stereo = fb_base
	 *         stride = pitch
	 */
	if (timing->mode_info_flags & IGD_SCAN_INTERLACE) {

		if(timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* Interlaced + Line double flags means field replication.
			 * same lines are sent for both fields. Program the
			 * second eye to be same as the first
			 */
			*stereo = base_offset;
		} else {
			/* Regular interlaced. Second eye starts on line 2.
			 * Skip every other line.
			 */
			*stereo = base_offset + pitch;
			*stride = pitch << 1;
		}
	}

	EMGD_TRACE_EXIT;
	return 0;
}



/*!
 *
 * @param emgd_crtc
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL if color attributes not found
 */
static int set_color_correct_tnc(emgd_crtc_t *emgd_crtc)
{
	const int        MID_PIXEL_VAL    = 125;
	const int        MAX_PIXEL_VAL    = 255;
	const int        NUM_PALETTE_ENTRIES = 256;

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
	unsigned int       i;
	struct drm_device *dev = NULL;

	igd_range_attr_t *gamma_attr      = NULL, *contrast_attr = NULL;
	igd_range_attr_t *brightness_attr = NULL;
	igd_attr_t       *hal_attr_list   = NULL;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
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
		EMGD_ERROR_EXIT("Color Correction Atrributes not found!");
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
		/* SDVO palette register is not accesible */
		EMGD_WRITE32(palette[i],
			MMIO_TNC(IGD_PORT_LVDS) +
			pipe->palette_reg + i*4);
	}

	OS_FREE(palette);

	EMGD_TRACE_EXIT;
	return 0;
}



/*!
 * kms_program_pipe_vga_tnc
 *
 * @param emgd_crtc Pointer to our private kms crtc structure
 *
 * @return void
 */
void kms_program_pipe_vga_tnc(
	emgd_crtc_t *emgd_crtc)
{
	struct drm_device  *dev = NULL;
	igd_timing_info_t  *timing = NULL;
	igd_display_pipe_t *pipe = NULL;
	igd_context_t      *context = NULL;
	struct drm_encoder *encoder = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	igd_display_port_t *port = NULL;
	unsigned long       vga_control;
	unsigned long       upscale = 0;
	int                 centering = 1;
	unsigned long       pt;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	/*
	 * VGA Plane can attach to only one pipe at a time. LVDS can
	 * only attach to pipe B. We need to use the display passed to
	 * determine the pipe number to use. (Napa is same as Alm).
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
		EMGD_ERROR_EXIT("No Extension pointer in program_pipe_vga_tnc");
		return;
	}

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;

		    /* Find UPSCALING attr value*/
		    pi_pd_find_attr_and_value(port,
	            PD_ATTR_ID_PANEL_FIT,
	            0,/*no PD_FLAG for UPSCALING */
	            NULL, /* dont need the attr ptr*/
	            &upscale);
		    /* this PI func will not modify value
			 * of upscale if attr does not exist
			 */
			break;
		}
	}

	/* magic->vga || native->vga cases, centering isn't required */
	if ((timing->width == 720 && timing->height == 400) || upscale) {
		EMGD_DEBUG("Centering = 0");
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
		pt = get_port_type(emgd_crtc->crtc_id);
		if(pt == IGD_PORT_LVDS) {
			EMGD_DEBUG("Enable VGA Upper-Left Centering & Nine Dot Disable");
			vga_control |= (1L<<25 | (1L<<18));
		} else if (upscale) {
			EMGD_DEBUG("Enable VGA Center Upper-left for upscale ports");
			vga_control |= 1L<<25;
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
 * kms_set_pipe_pwr_tnc
 *
 * Turns the pipe ON or OFF depending on the input
 *
 * @param emgd_crtc (IN) specifies the pipe to change
 * @param enable    (IN) TRUE to enable pipe, FALSE to disable
 *
 * @return
 */
static void kms_set_pipe_pwr_tnc(emgd_crtc_t *emgd_crtc, unsigned long enable)
{
	unsigned long       pipe_conf;
	unsigned long       port_type;
	unsigned long 		temp;
	struct drm_device  *dev;
	igd_display_pipe_t *pipe;
	igd_context_t      *context;


	EMGD_TRACE_ENTER;


	dev       = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context   = ((drm_emgd_priv_t *)dev->dev_private)->context;

	pipe      = emgd_crtc->igd_pipe;
	port_type = get_port_type(emgd_crtc->crtc_id);
	pipe_conf = READ_MMIO_REG_TNC(port_type, pipe->pipe_reg);


	/* Do nothing if current power state is same as what we want to set */
	/* The PIPE_ENABLE bit is at bit-position 31 */
	if ( (enable << 31) == (pipe_conf & PIPE_ENABLE) ){

		EMGD_TRACE_EXIT;
		return;
	}


	if (!enable) {

		/* For SDVO, disable both pipe Bs in 0:2:0 and 0:3:0 */
		if (IGD_PORT_SDVO == port_type) {
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, pipe->pipe_reg,
				pipe_conf & ~PIPE_ENABLE);
		}

		/* Disable pipe */
		WRITE_MMIO_REG_TNC(port_type, pipe->pipe_reg, pipe_conf & ~PIPE_ENABLE);

		/* check when the pipe is disabled. */
		wait_pipe(port_type, pipe->pipe_reg, 0);

		/* Make sure the associated DPLL is turned off. */
		temp = READ_MMIO_REG_TNC(port_type, pipe->clock_reg->dpll_control);
		if (temp & BIT31) {
			/* Double buffered */
			WRITE_MMIO_REG_TNC(port_type, pipe->clock_reg->dpll_control,
				temp & ~BIT31);
			WRITE_MMIO_REG_TNC(port_type, pipe->clock_reg->dpll_control,
				temp & ~BIT31);
		}


		EMGD_DEBUG("Set Pipe Power: OFF");

	} else {

		if (IGD_PORT_SDVO == port_type) {
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, pipe->pipe_reg,
				pipe_conf | PIPE_ENABLE);
		}

		/* Enable pipe */
		WRITE_MMIO_REG_TNC(port_type, pipe->pipe_reg, pipe_conf | PIPE_ENABLE);

		EMGD_DEBUG("Set Pipe Power: ON");
	}


	EMGD_TRACE_EXIT;
	return;
}



/**
 * kms_program_pipe_tnc
 *
 * Called before a mode set, takes the input "mode", matches it to the closest
 * supported mode, then put the supported mode into "adjusted_mode" to let the
 * caller know.
 *
 * @param encoder (IN) Encoder being prepared
 *
 * @return true, false (details TBD)
 */
static void kms_program_pipe_tnc(emgd_crtc_t *emgd_crtc)
{
	unsigned long       timing_reg;
	unsigned long       pipe_conf;
	unsigned long       hactive, vactive;
	short               hactive_tmp, vactive_tmp;
	igd_timing_info_t  *current_timings;
	igd_timing_info_t   pTimings_tmp;
	struct drm_device  *dev          = NULL;
	igd_display_pipe_t *pipe         = NULL;
	igd_display_port_t *port         = NULL;
	igd_context_t      *context      = NULL;
	struct drm_encoder *encoder      = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	pd_timing_t        *vga_timing   = NULL;

	unsigned long flag_clip_fix;
	unsigned long pipe_reg;
	unsigned long temp;
	unsigned long pt;
	unsigned long dc;
	unsigned long calc;
	int           i;
	tnc_wa_timing_t *wa;
	/* igd_framebuffer_info_t *fb_info = PLANE(display)->fb_info; */
	platform_context_tnc_t *platform_context;

	EMGD_TRACE_ENTER;

	pipe            = emgd_crtc->igd_pipe;
	current_timings = pipe->timing;
	vga_timing      = (pd_timing_t *) current_timings->extn_ptr;
	pipe_reg        = pipe->pipe_reg;

	pt       = get_port_type(emgd_crtc->crtc_id);
	dev      = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context  = ((drm_emgd_priv_t *)dev->dev_private)->context;

	platform_context = (platform_context_tnc_t *) mode_context->context->platform_context;
	flag_clip_fix    = mode_context->clip_hw_fix;

	EMGD_DEBUG("Device power state: D%ld", context->device_context.power_state);

	pipe_conf = READ_MMIO_REG_TNC(pt, pipe->pipe_reg);

	/* Preserving bits 0:17, bit 20, bit 24, bit 26, bit 29:30.  The spec on
	 * which bits to preserve has been updated since we've originally written
	 * the code for the non-KMS path.  However, to minimize risk of this fix,
	 * we are only updating them here.  This is why we are not using
	 * device_data_tnc->pipe_preserve.  */
	pipe_conf &= 0x6513FFFF;

	/* For TNC B1, enable hardware cliping fix*/
	if((platform_context->tnc_dev3_rid == TNC_B1_DEV3_RID)&&
		(flag_clip_fix & IGD_CLIP_FIX_GLOBAL_ENABLE)) {

		/* Disable SDVO Pipe in Device 2 and Device 3 */
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, pipe->pipe_reg,
			pipe_conf & (~0x80000000L));

		WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, pipe->pipe_reg,
			pipe_conf & (~0x80000000L));

		/* check when the pipe is disabled. */
		wait_pipe(IGD_PORT_LVDS, pipe->pipe_reg, 0);

		/* Enable clipping hardware fix */
		temp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS);
		if(flag_clip_fix & IGD_CLIP_FIX_REPLACE_STALL) {
			temp |= BIT18;
		} else {
			temp &= ~BIT18;
		}
		if(flag_clip_fix & IGD_CLIP_FIX_DISABLE_THROTTLE) {
			temp |= BIT15;
		} else {
			temp &= ~BIT15;
		}
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS, temp);
	}

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			break;
		}
	}

	if (NULL == emgd_encoder) {
		EMGD_ERROR_EXIT("Invalid CRTC selected.");
		return;
	}

	/* Debug messages */
	EMGD_DEBUG("Current timings %ux%u mode_number = %u "
			   "mode_info_flags = 0x%lx, dclk = %lu",
			   current_timings->width,
			   current_timings->height,
			   current_timings->mode_number,
			   current_timings->mode_info_flags,
			   current_timings->dclk);

	if (vga_timing) {
	  EMGD_DEBUG("ext_timing %ux%u mode_number = %u "
				 "mode_info_flags= 0x%lx, dclk = %lu",
				 vga_timing->width,
				 vga_timing->height,
				 vga_timing->mode_number,
				 vga_timing->mode_info_flags,
				 vga_timing->dclk);
	}


	/*
	 * If the mode is VGA and the PD says it handles all VGA modes without
	 * reprogramming then just set the mode and leave centering off.
	 */
	if(current_timings->mode_info_flags & IGD_MODE_VESA) {
		EMGD_DEBUG("IGD_MODE_VESA");

		if (current_timings->mode_number <= VGA_MODE_NUM_MAX) {
			EMGD_DEBUG("current_timings->mode_number <= VGA_MODE_NUM_MAX");
			kms_program_pipe_vga_tnc(emgd_crtc);

			EMGD_TRACE_EXIT;
			return;
		}
	}

	/* Program dot clock divisors. */
	kms_program_clock_tnc(emgd_crtc, pipe->clock_reg, current_timings->dclk);

	/* Program timing registers for the pipe */
	timing_reg = pipe->timing_reg;
	if (current_timings->mode_info_flags & IGD_PIXEL_DOUBLE) {
		hactive = (unsigned long)current_timings->width*2 - 1;
	} else {
		hactive = (unsigned long)current_timings->width - 1;
	}

	if (current_timings->mode_info_flags & IGD_LINE_DOUBLE) {
		vactive = (unsigned long)current_timings->height*2 - 1;
	} else {
		/* For Atom E6xx Hardware will automatically divide by 2 to
		   get the number of line in each field */
		vactive = (unsigned long)current_timings->height - 1;
	}


	/* Reset the Palette */
	for (i = 0; i < 256; i++) {
		/* Program each of the 256 4-byte palette entries */
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, pipe->palette_reg + (i << 2),
			((i<<16) | (i<<8) | i));
	}

	if (port) {
		/* apply color correction */
		for( i = 0; PD_ATTR_LIST_END != port->attributes[i].id; i++ ) {

			if ((PD_ATTR_ID_FB_GAMMA      == (port->attributes[i].id)) ||
				(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)) ||
				(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)))  {

				set_color_correct_tnc(emgd_crtc);
			}
		}
	}


	/*
	 * NOTE: For size reasons the timng table contains unsigned short
	 * values. Don't shift them past 16. Use a temp instead.
	 * All register offsets and bit shift are verified for Gen4
	 *
	 * For SDVO display:
	 * Write values into pipe B registers in both 0:2:0 and 0:3:0
	 */

	dc = *(context->mod_dispatch.dsp_current_dc);

	wa = WA_TUNE;

	for (i=0; i<2; i++) {
		/* Temp variable */
		pTimings_tmp = *current_timings;
		hactive_tmp  = (short) hactive;
		vactive_tmp  = (short) vactive;

		/* This work around is only for Atom E6xx B0 */
		if((IGD_DC_CLONE(dc) || IGD_DC_EXTENDED(dc))
			&& pt == IGD_PORT_SDVO &&
			(context->device_context.rid == TNC_B0_RID)
			&& FLAG(flag_basic_htotal_formula_wa)){

			if(!(current_timings->reserved_dd & TNC_HTOTAL_TUNED)){
				/* Modify blanks so it always begin after active pixel and ends
				 * at the end. Do not change it if we are already tuned to
				 * maintain original timing specification
				 */
				pTimings_tmp.hblank_start =
								(short) (current_timings->width - 1);
				pTimings_tmp.vblank_start =
								(short) (current_timings->height - 1);
				pTimings_tmp.hblank_end = (short) (current_timings->htotal);
				pTimings_tmp.vblank_end = (short) (current_timings->vtotal);
			}

			if(i==0){
				if(current_timings->reserved_dd == 0 ||
					(wa->counter > LIMIT_TOTAL_CHECK_DISPLAY)){
					/* First time tuning */
					if ((mode_context->ref_freq != 0) &&
							(mode_context->ref_freq >= 190000) &&
							(mode_context->ref_freq <= 210000)){
						calc = (current_timings->htotal *
									mode_context->ref_freq);
					} else {
						calc = (current_timings->htotal * LNC_CLOCK);
					}

					calc = (calc / (pipe->clock_reg->actual_dclk));
					calc *= (current_timings->vtotal -1);
					calc /= current_timings->vtotal;
					pTimings_tmp.htotal = (short)calc;
					current_timings->reserved_dd = wa->htotal =
						pTimings_tmp.htotal;
					EMGD_DEBUG("Delta = %d", wa->htotal);
				}else if (current_timings->reserved_dd & TNC_HTOTAL_TUNED){
					pTimings_tmp.htotal =
						(short)(current_timings->reserved_dd &
								(~TNC_HTOTAL_TUNED));
				}else{
					if(wa->htotal == 0)
						wa->htotal = (short)current_timings->reserved_dd;
					pTimings_tmp.htotal = wa->htotal;
				}
				/* Use vphase formula if available */
				if(vphase){
					pTimings_tmp.vtotal -= (short)vphase;
					pTimings_tmp.vsync_start -= (short)vphase;
					pTimings_tmp.vsync_end -= (short)vphase;
					pTimings_tmp.vblank_end -= (short)vphase;
				}else{
					pTimings_tmp.hblank_end +=
						(short) (pTimings_tmp.htotal - current_timings->htotal);
				}
			}
		}


		/* Program Timings */
		temp = (unsigned long)(pTimings_tmp.htotal) << 16 | hactive_tmp;
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg, temp);

		temp = ((unsigned long) pTimings_tmp.hblank_end << 16) |
			(unsigned long)(pTimings_tmp.hblank_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x04, temp);

		temp = ((unsigned long)(current_timings->hsync_end) << 16) |
			(unsigned long)(current_timings->hsync_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x08, temp);

		temp = ((unsigned long)(pTimings_tmp.vtotal) << 16) | vactive_tmp;
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x0C, temp);

		temp = ((unsigned long)(pTimings_tmp.vblank_end) << 16) |
			(unsigned long)(pTimings_tmp.vblank_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x10, temp);

		temp = ((unsigned long)(pTimings_tmp.vsync_end)<< 16) |
			(unsigned long)(pTimings_tmp.vsync_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x14, temp);

		/*
		 * If there is a linked mode it is either the VGA or a scaled
		 * mode. If it is scaled then we need to use it as the source size.
		 */
		if (current_timings->extn_ptr) {
			igd_timing_info_t *scaled_timings =
				(igd_timing_info_t *)current_timings->extn_ptr;

			if((scaled_timings->mode_info_flags & IGD_MODE_VESA) &&
				(scaled_timings->mode_number <= VGA_MODE_NUM_MAX)) {
				temp = (hactive << 16) | vactive;
			} else {
				EMGD_DEBUG("scaled_timings->width [%d], "
						   "scaled_timings->height [%d]\n",
					scaled_timings->width, scaled_timings->height);
				temp = (unsigned long)scaled_timings->width  - 1;
				temp = (temp << 16) |
					(unsigned long)(scaled_timings->height - 1);
			}
		} else {
			temp = (hactive_tmp << 16) | vactive_tmp;
		}
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x1C, temp);

		/* Put pipe in interlaced mode if requested:
		 *     should only happen for LVDS display if at all. */
		if (current_timings->mode_info_flags & IGD_SCAN_INTERLACE) {
			pipe_conf |= (INTERLACE_EN);
		} else {
			pipe_conf &= ~(INTERLACE_EN);
		}

		pipe_conf |= PIPE_ENABLE;
		WRITE_MMIO_REG_TNC(ports_tnc[i], pipe->pipe_reg, pipe_conf);
		WRITE_MMIO_REG_TNC(ports_tnc[i], pipe->pipe_reg, pipe_conf);

		/* For LVDS port, don't touch dev3 registers stop after 1st iteration */
		if (pt == IGD_PORT_LVDS) {
			break;
		}
	}


	/* Gen4 can check when the pipe is enabled. */
	wait_pipe(IGD_PORT_LVDS, pipe->pipe_reg, 0x40000000);

	/*
	 * Set the VGA address range to 0xa0000 so that a normal (not VGA)
	 * mode can be accessed through 0xa0000 in a 16bit world.
	 */
	WRITE_AR(context->device_context.virt_mmadr, 0x10, 0xb);
	WRITE_VGA(context->device_context.virt_mmadr, GR_PORT, 0x06, 0x5);
	WRITE_VGA(context->device_context.virt_mmadr, GR_PORT, 0x10, 0x1);

	if(current_timings->extn_ptr) {
		/* This means either internal scaling (LVDS) or centered VGA */
		current_timings = current_timings->extn_ptr;
		if(current_timings->extn_ptr) {
			/* This is both the scaled and centered VGA */
			current_timings = current_timings->extn_ptr;
		}
		if (current_timings->mode_info_flags & IGD_MODE_VESA) {
			if (current_timings->mode_number <= VGA_MODE_NUM_MAX) {
				kms_program_pipe_vga_tnc(emgd_crtc);
			}
		}
	}

	if (pt == IGD_PORT_SDVO) {
		/*  Enable Chicken Bit */
		/*  Setting BIT6 enable Pipe B Palette Write
         *  to prevent hang during palette write */
		temp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS);
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS, temp | BIT6);
	}

	EMGD_TRACE_EXIT;
	return;
}



static void kms_set_plane_pwr_tnc(emgd_crtc_t *emgd_crtc, unsigned long enable)
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
	wait_for_vblank_tnc(pipe->pipe_reg);

	/* Get the current value of the plane control register */
	plane_reg     = plane->plane_reg;
	plane_control = EMGD_READ32(context->device_context.virt_mmadr + plane_reg);

	if((enable == FALSE) ||
		(context->device_context.power_state != IGD_POWERSTATE_D0)) {
		/*
		 * Note: The vga programming code does not have an "off". So
		 * when programming the plane to off we make sure VGA is off
		 * as well.
		 */
		disable_vga_tnc(context->device_context.virt_mmadr);

		/* The Sprite and Cursor planes need to turned off for the modeset
		   to succeed. */
       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + DSPCCNTR);
       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + DSPCCNTR + DSP_START_OFFSET);

       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + CUR_B_CNTR);
       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + CUR_B_CNTR + CUR_BASE_OFFSET);

       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + CUR_A_CNTR);
       	EMGD_WRITE32(0x0, context->device_context.virt_mmadr + CUR_B_CNTR + CUR_BASE_OFFSET);

		/*
		 * To turn off plane A or B, the program have to trigger the plane A
		 * or B start register.  Or else, it will not work.
		 */
		plane_control &= ~BIT31;

		EMGD_WRITE32(plane_control,
						context->device_context.virt_mmadr + plane_reg);

		EMGD_WRITE32(EMGD_READ32(context->device_context.virt_mmadr +
			plane_reg + DSP_START_OFFSET),
			context->device_context.virt_mmadr + plane_reg + DSP_START_OFFSET);
	} else {
		/* Enable Plane */
		plane_control |= BIT31;

		EMGD_WRITE32(plane_control,
			context->device_context.virt_mmadr + plane_reg);

		EMGD_WRITE32(EMGD_READ32(context->device_context.virt_mmadr +
			plane_reg + DSP_START_OFFSET),
			context->device_context.virt_mmadr + plane_reg + DSP_START_OFFSET);
	}


	wait_for_vblank_tnc(pipe->pipe_reg);
	EMGD_TRACE_EXIT;
	return;
}



static void kms_program_plane_tnc(emgd_crtc_t *emgd_crtc, unsigned long status)
{
	unsigned long stride;
	unsigned long stereo;
	unsigned long plane_control;
	unsigned long other_plane_reg;
	igd_timing_info_t      *timing      = NULL;
	igd_framebuffer_info_t *fb_info     = NULL;
	unsigned long           plane_reg;
	struct drm_device      *dev         = NULL;
	igd_display_plane_t    *plane       = NULL;
	igd_display_pipe_t     *pipe        = NULL;
	igd_context_t          *context     = NULL;
	igd_timing_info_t      *pipe_timing = NULL;
	unsigned long           ulreg       = 0;
	unsigned long           b_reg       = 0;
	/* tnc_wa_timing_t *wa = WA_TUNE; */

	EMGD_TRACE_ENTER;


	pipe    = emgd_crtc->igd_pipe;
	dev     = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	plane     = PLANE(pipe->owner);
	if (!plane) {
		EMGD_ERROR("Trying to program a plane that is not tied to a crtc.");
		return;
	}
	fb_info   = plane->fb_info;
	plane_reg = plane->plane_reg;


	EMGD_DEBUG("Device power state: D%ld", context->device_context.power_state);

	wait_for_vblank_tnc(pipe->pipe_reg);

	plane_control = EMGD_READ32(context->device_context.virt_mmadr + plane_reg);
	if(plane->plane_reg == DSPACNTR) {
		plane_control &= device_data_tnc->plane_a_preserve;
	} else { /* if it's plane b or plane c */
		plane_control &= device_data_tnc->plane_b_c_preserve;
	}


	/*
	 * Note: The very first pass through this function will be with
	 * status false and timings == NULL. Don't use the timings before
	 * the check above.
	 */
	timing = pipe->timing;
	pipe_timing = timing;
	/* There is a special case code for legacy VGA modes */
	while (timing->extn_ptr) {
		timing = (igd_timing_info_t *)timing->extn_ptr;
	}
	if(MODE_IS_VGA(timing) && CHECK_VGA(pipe_timing)) {
		kms_program_plane_vga(context->device_context.virt_mmadr, timing);
		EMGD_TRACE_EXIT;
		return;
	}

	disable_vga_tnc(context->device_context.virt_mmadr);

	/* enable plane, select pipe, enable gamma correction logic */
	plane_control |= (pipe->pipe_num<<24);
	pipe->plane = plane;
	plane_control |= (1<<30);


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
		/* For line ONLY doubling, set bit 21 also '1' */
		plane_control |= BIT21;
	}

	kms_mode_get_stride_stereo_tnc(pipe, plane, &stride, &stereo, 0);

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

	if(fb_info->flags & IGD_SURFACE_TILED) {
		plane_control |= (BIT10);
	}

	/* Set watermark for Atom E6xx */
	if (!mode_context->en_reg_override) {
		if (plane_reg == DSPACNTR) {
			other_plane_reg = DSPBCNTR;
		} else {
			other_plane_reg = DSPACNTR;
		}

		if (EMGD_READ32(context->device_context.virt_mmadr + other_plane_reg) &
				0x80000000) {

			EMGD_WRITE32(device_data_tnc->dsp_arb,
				context->device_context.virt_mmadr + PIPEA_DISP_ARB_CTRL);
		} else if (plane_reg == DSPACNTR) {
			EMGD_WRITE32(0x00003fff,
				context->device_context.virt_mmadr + PIPEA_DISP_ARB_CTRL);
		} else {
			EMGD_WRITE32(0x00003f80,
				context->device_context.virt_mmadr + PIPEA_DISP_ARB_CTRL);
		}

		/*
		 * Setting WM priority level to 11
		 * to workaround display bouncing issues
		 * TODO: Mode set from Clone->Single(Primary)
		 * PIPEA_DISP_ARB_CTRL does not get updated
		 */
		ulreg = EMGD_READ32(context->device_context.virt_mmadr + 0x00020f8);
		EMGD_WRITE32(ulreg | 0x000003f0,
		context->device_context.virt_mmadr + 0x00020f8);

		EMGD_WRITE32(device_data_tnc->fifo_watermark1,
			context->device_context.virt_mmadr + FW_1);
		EMGD_WRITE32(device_data_tnc->fifo_watermark2,
			context->device_context.virt_mmadr + FW_2);
		EMGD_WRITE32(device_data_tnc->fifo_watermark3,
			context->device_context.virt_mmadr + FW_3);
		EMGD_WRITE32(device_data_tnc->fifo_watermark4,
			context->device_context.virt_mmadr + FW_4);
		EMGD_WRITE32(device_data_tnc->fifo_watermark5,
			context->device_context.virt_mmadr + FW_5);
		EMGD_WRITE32(device_data_tnc->fifo_watermark6,
			context->device_context.virt_mmadr + FW_6);
	} else {
		/* en_reg_override=1 */
		/* Override display registers */
		EMGD_WRITE32(mode_context->gvd_hp_control,
			context->device_context.virt_mmadr + 0x00020f8);
		EMGD_WRITE32(mode_context->disp_arb,
			context->device_context.virt_mmadr + PIPEA_DISP_ARB_CTRL);
		EMGD_WRITE32(mode_context->fifo_watermark1,
			context->device_context.virt_mmadr + FW_1);
		EMGD_WRITE32(mode_context->fifo_watermark2,
			context->device_context.virt_mmadr + FW_2);
		EMGD_WRITE32(mode_context->fifo_watermark3,
			context->device_context.virt_mmadr + FW_3);
		EMGD_WRITE32(mode_context->fifo_watermark4,
			context->device_context.virt_mmadr + FW_4);
		EMGD_WRITE32(mode_context->fifo_watermark5,
			context->device_context.virt_mmadr + FW_5);
		EMGD_WRITE32(mode_context->fifo_watermark6,
			context->device_context.virt_mmadr + FW_6);
		EMGD_WRITE32(mode_context->disp_chicken_bits,
			context->device_context.virt_mmadr + 0x0070400);

		/* Override Bunit Chickenbits and BWFLUSH register */
		write_bunit_tnc(0x30, mode_context->bunit_chicken_bits);
		write_bunit_tnc(0x02, mode_context->bunit_write_flush);
	}

	EMGD_DEBUG(" GVD HP_CONTROL: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr +
				PIPEA_DISP_ARB_CTRL);
	EMGD_DEBUG(" Display Arbitration register: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_1);
	EMGD_DEBUG(" FIFO Watermark Control Register 1: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_2);
	EMGD_DEBUG(" FIFO Watermark Control Register 2: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_3);
	EMGD_DEBUG(" FIFO Watermark Control Register 3: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_4);
	EMGD_DEBUG(" FIFO Watermark Control Register 4: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_5);
	EMGD_DEBUG(" FIFO Watermark Control Register 5: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + FW_6);
	EMGD_DEBUG(" FIFO Watermark Control Register 6: 0x%lx", ulreg);
	ulreg = EMGD_READ32(context->device_context.virt_mmadr + DSP_CHICKENBITS);
	EMGD_DEBUG(" Display Chicken bits: 0x%lx", ulreg);
	read_bunit_tnc(0x30, &b_reg);
	EMGD_DEBUG(" Bunit Chicken bits: 0x%lx", b_reg);
	read_bunit_tnc(0x02, &b_reg);
	EMGD_DEBUG(" Bunit Write Flush: 0x%lx", b_reg);


	/* FIXME: Not required for TNC.
	 * The B-Spec states that rendering will be slower if the fences are not
	 * a power of 2.  So for now, always use a power of 2. */
	/* EMGD_WRITE32(0x04000400, context->device_context.virt_mmadr + 0x209c); */

	EMGD_DEBUG(" Plane Control:  0x%lx", plane_control);
	EMGD_DEBUG(" Plane Base:     0x%lx", fb_info->fb_base_offset);
	EMGD_DEBUG(" Visible Offset: 0x%lx", fb_info->visible_offset);
	EMGD_DEBUG(" Plane Pitch:    0x%lx", stride);

	EMGD_WRITE32(plane_control, context->device_context.virt_mmadr + plane_reg);
	EMGD_WRITE32(stride,
		context->device_context.virt_mmadr + plane_reg + DSP_STRIDE_OFFSET);

	EMGD_WRITE32(fb_info->visible_offset,
		context->device_context.virt_mmadr + plane_reg + DSP_LINEAR_OFFSET);
	EMGD_WRITE32(0, context->device_context.virt_mmadr + plane_reg + 0x24);
	EMGD_WRITE32(fb_info->fb_base_offset,
		context->device_context.virt_mmadr + plane_reg + DSP_START_OFFSET);

	wait_for_vblank_tnc(pipe->pipe_reg);

	EMGD_TRACE_EXIT;
}


/*!
 *
 * @param emgd_encoder
 * @param status
 *
 * @return program_port_lvds_gen4()
 * @return program_port_sdvo_gen4()
 * @return -IGD_ERROR_INVAL on failure
 */
static int kms_program_port_tnc(emgd_encoder_t *emgd_encoder,
		unsigned long status)
{
	EMGD_TRACE_ENTER;

	if (emgd_encoder->igd_port->port_type == IGD_PORT_LVDS) {
		EMGD_TRACE_EXIT;
		return kms_program_port_lvds_tnc(emgd_encoder, status);
	} else {
		EMGD_TRACE_EXIT;
		return kms_program_port_sdvo_tnc(emgd_encoder, status);
	}
}


/*!
 *
 * @param emgd_encoder
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
int kms_program_port_lvds_tnc(emgd_encoder_t *emgd_encoder,
		unsigned long status)
{
	int ret = 0;
	unsigned long pd_powerstate = PD_POWER_MODE_D3;
	pd_timing_t        *timing    = NULL;
	igd_display_port_t *port      = NULL;
	struct drm_encoder *encoder   = NULL;
	struct drm_device  *dev       = NULL;
	igd_context_t      *context   = NULL;
	struct drm_crtc    *crtc      = NULL;
	emgd_crtc_t        *emgd_crtc = NULL;
	igd_display_pipe_t *pipe      = NULL;

	EMGD_TRACE_ENTER;

	encoder = &emgd_encoder->base;
	port = emgd_encoder->igd_port;
	dev = encoder->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	EMGD_DEBUG("Program LVDS: (%s)", status?"ENABLE":"DISABLE");

	if (status == TRUE) {

		if(!(port->pt_info->flags & IGD_DISPLAY_ENABLE)) {
			EMGD_TRACE_EXIT;
			return 0;
		}

		pd_powerstate = (context->device_context.power_state > port->power_state) ?
			context->device_context.power_state : port->power_state;

		if (pd_powerstate == IGD_POWERSTATE_D0) {
			EMGD_DEBUG("Power State: D0");

			list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
				if (crtc == encoder->crtc) {
					emgd_crtc = container_of(crtc, emgd_crtc_t, base);
					pipe = emgd_crtc->igd_pipe;
					break;
				}
			}

			timing = (pd_timing_t *)pipe->timing;
			/* Reach end timing to get user resolution and pass it to pd */
			if(timing->extn_ptr) {
				timing = (pd_timing_t *)timing->extn_ptr;
			}
			/* set mode will take care of port control */
			ret = port->pd_driver->set_mode(
					port->pd_context,
					timing,
					1<<pipe->pipe_num);
		}
	}

	/* either status == FALSE, or status == TRUE, but powerstate is D1/D2/D3 */
	if (pd_powerstate != IGD_POWERSTATE_D0) {
		ret = port->pd_driver->set_power(
			port->pd_context, PD_POWER_MODE_D3);
	}

	if (ret) {
		EMGD_ERROR_EXIT("PD set_%s returned: 0x%x",
			(pd_powerstate == IGD_POWERSTATE_D0)?"mode":"power", ret);
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return ret;
}


/*!
 *
 * @param emgd_encoder
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int kms_program_port_sdvo_tnc(emgd_encoder_t *emgd_encoder,
		unsigned long status)
{
	unsigned long port_control;
	unsigned long pd_powerstate = PD_POWER_MODE_D3;
	unsigned long upscale = 0;
	igd_timing_info_t  local_timing;
	igd_timing_info_t  *timing    = NULL;
	igd_display_port_t *port      = NULL;
	struct drm_encoder *encoder   = NULL;
	struct drm_device  *dev       = NULL;
	igd_context_t      *context   = NULL;
	struct drm_crtc    *crtc      = NULL;
	emgd_crtc_t        *emgd_crtc = NULL;
	igd_display_pipe_t *pipe      = NULL;
	unsigned long temp;
	int ret;

	EMGD_TRACE_ENTER;

	encoder = &emgd_encoder->base;
	port = emgd_encoder->igd_port;
	dev = encoder->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	EMGD_DEBUG("Program Port: (%s)", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("pd_flags: 0x%lx", port->pd_flags);

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (crtc == encoder->crtc) {
			emgd_crtc = container_of(crtc, emgd_crtc_t, base);
			pipe = emgd_crtc->igd_pipe;
			break;
		}
	}

	timing = pipe->timing;

	port_control = READ_MMIO_REG_TNC(IGD_PORT_SDVO, port->port_reg);

	if (status == TRUE) {
		if (!(port->pt_info->flags & IGD_DISPLAY_ENABLE)) {
			EMGD_TRACE_EXIT;
			return 0;
		}

		/* Enable VGA syncs for native vga modes */
		if (port->vga_sync == 1) {
			EMGD_DEBUG("VGA sync true, is width x height 720 x 400?");
			if((timing->width == 720) && (timing->height == 400)) {
				EMGD_DEBUG("Modify port control and multi_port_control");
				port_control |= (1L<<15);
			}
		}

		/* Fact that both IGD_ powerstates and PD_ powermodes have
		 * same definitions */
		pd_powerstate =
			(context->device_context.power_state > port->power_state) ?
			context->device_context.power_state : port->power_state;

		if (pd_powerstate == IGD_POWERSTATE_D0) {
			EMGD_DEBUG("Power State: D0");
			/* Upscale */
			pi_pd_find_attr_and_value(port,
				PD_ATTR_ID_PANEL_FIT,
				0, /*no PD_FLAG for UPSCALING */
				NULL, /* dont need the attr ptr*/
				&upscale);

			/* Reach the end timing if upscaling is enabled */
			if (timing->extn_ptr && upscale) {
				timing = (pd_timing_t *)timing->extn_ptr;
			}

			local_timing = *timing;
			if (upscale) {
				/* For timings smaller than width 360 and height 200,
				 * double the size. This is because the active area of the mode
				 * is double the size of the resolution for these modes
				 *  - Very tricky huh */
				if (local_timing.width <= 360) {
					local_timing.width <<= 1;
				}
				if (local_timing.height <= 200) {
					local_timing.height <<= 1;
				}
			}

			/* BIT31 - Enable
			 * BIT30 - PIPE B
			 * BIT29 - Stall
			 * BIT7  - Border
			 */
			port_control |= BIT31|BIT30|BIT29|BIT7;

			/* Program cDVO registers:
			 * Keep default values for
			 *     7000h - cDVO control register
			 *     7004h - cDVO slew rate register
			 *     7008h - cDVO strength register
			 *     700Ch - cDVO RCOMP update register
			 *     6102Ch - cDVO stall register = 0xA.
			 * Note: Though EAS says 6102Ch default value is 6, it is a typo
			 *     in the spec, based on Si DE hw default value is 10 (0xA),
			 *     so no need to program explicitly. This saves few bytes for
			 *     micro.
			 */

			/* Enable Current Source */
			temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG);
			temp |= 0x2000;
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG, temp);
		}
	}

	if (pd_powerstate == PD_POWER_MODE_D0) {
		ret = port->pd_driver->set_mode(port->pd_context, &local_timing, 0);
	} else {
		ret = port->pd_driver->set_power(port->pd_context, pd_powerstate);
	}

	if (ret) {
		EMGD_ERROR_EXIT("PD set_%s returned: 0x%x",
			(pd_powerstate == PD_POWER_MODE_D0)?"mode":"power", ret);
		return -IGD_ERROR_INVAL;
	}

	EMGD_DEBUG("Port_control: 0x%lx", port_control);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, port->port_reg, port_control);

	EMGD_TRACE_EXIT;
	return 0;
}



/*!
 * @param emgd_encoder
 * @param status
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int kms_post_program_port_tnc(emgd_encoder_t * emgd_encoder,
	unsigned long status)
{
	int ret;
	struct drm_device  *dev       = NULL;
	igd_context_t      *context   = NULL;
	igd_display_port_t *port      = NULL;
	igd_display_pipe_t *pipe      = NULL;
	struct drm_encoder *encoder   = NULL;
	struct drm_crtc    *crtc      = NULL;
	emgd_crtc_t        *emgd_crtc = NULL;
	igd_timing_info_t  *timings   = NULL;
	/* unsigned long portreg; */

	EMGD_TRACE_ENTER;

	encoder = &emgd_encoder->base;
	dev = encoder->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	port = emgd_encoder->igd_port;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (crtc == encoder->crtc) {
			emgd_crtc = container_of(crtc, emgd_crtc_t, base);
			pipe = emgd_crtc->igd_pipe;
			timings = pipe->timing;
			break;
		}
	}

	/*
	 * The programming found in the common code for all chipsets
	 * has the device programming sequence as follows:
	 *  Port
	 *  Pipe
	 *  Post Port
	 *  Plane
	 * On Gen4, if the port is enabled before the pipe, there is a 10%
	 * chance that the port will not turn on properly.
	 * Due to compatability requires with other chipsets, this workaround
	 * fixes this issue
	 *
	 * This is handled by set_power.  This also only seems to apply to a
	 * chipset that is no longer supported.  Not sure if this will get
	 * re-introduced with ValleyView and beyond, so I left it here.
	 *
	 * portreg = READ_MMIO_REG_TNC(port->port_type, port->port_reg);
	 * WRITE_MMIO_REG_TNC(port->port_type, port->port_reg, portreg & ~BIT31);
	 * WRITE_MMIO_REG_TNC(port->port_type, port->port_reg, portreg);
	 */

	if (port->port_type == IGD_PORT_SDVO) {
		initialize_ST_bridge(context, port);
	}

	ret = 0;
	/* call post_set_mode() if exists */
	if (port->pd_driver->post_set_mode) {
		if (port->port_type == IGD_PORT_SDVO){
			ret = port->pd_driver->post_set_mode(port->pd_context, timings,
				status); /*Needed for LAPIS*/
		} else {
			ret = port->pd_driver->post_set_mode(port->pd_context, timings,
				1<<pipe->pipe_num);
		}
		if (ret) {
			EMGD_ERROR_EXIT("PD post_set_mode returned: 0x%x", ret);
		}
	}

	EMGD_TRACE_EXIT;
	return ret;
}



/*!
 * kms_get_vblank_counter_tnc
 *
 * This function returns the vblank counter number back to the caller.
 *
 * @param emgd_crtc [IN] The pipe to get frame value from
 *
 * @return 0 frame number, which can also be used as a vblank counter number
 */
static u32 kms_get_vblank_counter_tnc(emgd_crtc_t *emgd_crtc)
{
	unsigned long      high1, high2, low;
	struct drm_device *dev;
	unsigned long      frame_high_reg, frame_low_reg;
	igd_context_t     *context;


	dev     = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;


	switch (emgd_crtc->igd_pipe->pipe_features & IGD_PORT_MASK) {
		case IGD_PORT_SHARE_DIGITAL:
			frame_high_reg = PIPEB_FRAME_HIGH;
			frame_low_reg  = PIPEB_FRAME_PIXEL;
			break;

		case IGD_PORT_SHARE_LVDS:
		default:
			frame_high_reg = PIPEA_FRAME_HIGH;
			frame_low_reg  = PIPEA_FRAME_PIXEL;
			break;
	}


	/*
	 * High and low register fields are not synchronized so it is possible
	 * that our low value is actually not from the same high value, e.g.
	 * going from "99" to "00" when transitioning 499 to 500.  To get
	 * around this, we will read until the two reads of high values stay
	 * the same.
	 */
	do {
		high1 = EMGD_READ32(context->device_context.virt_mmadr +
					frame_high_reg);
		high1 &= PIPE_FRAME_HIGH_MASK;

		low   = EMGD_READ32(context->device_context.virt_mmadr +
					frame_low_reg);
		low   &= PIPE_FRAME_LOW_MASK;

		high2 = EMGD_READ32(context->device_context.virt_mmadr +
					frame_high_reg);
		high2 &= PIPE_FRAME_HIGH_MASK;
	} while (high1 != high2);


	/* Frame count low is located at bits 24-31 */
	low >>= PIPE_FRAME_LOW_SHIFT;

	/* Low value is 8 bits long, so shift high by 8 bits */
	return (high1 << 8) | low;
}

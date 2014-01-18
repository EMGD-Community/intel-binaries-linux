/*
 *-----------------------------------------------------------------------------
 * Filename: micro_ovl_tnc.c
 * $Revision: 1.18 $
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
#include "ovl_tnc_cache.h"
#include "ovl2_tnc.h"
#include "ovl2_regs_tnc.h"
#include "../cmn/ovl_dispatch.h"
#include "../cmn/ovl_virt.h"

extern unsigned long sprite_pixel_formats_tnc[];

extern unsigned int ovl2_check_tnc(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags);

int micro_alter_ovl2_tnc(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags);
unsigned int micro_spritec_send_instr_tnc(
	igd_display_context_t     *display,
	unsigned long regs,
	unsigned long value);
extern ovl_context_t ovl_context[1];

#ifndef CONFIG_MICRO_OVERLAY
#define	OVL2_CHECK_TNC_RET(ret,a, b, c, d, e,f) ret = ovl2_check_tnc(a ,b ,c ,d ,e ,f)
#define QUERY_OVL2_TNC_RET(ret,a, b) ret = query_ovl2_tnc(a, b)
#else
#define	OVL2_CHECK_TNC_RET(ret,a, b, c, d, e,f) 0
#define QUERY_OVL2_TNC_RET(ret,a, b) 0
#endif

ovl_dispatch_t ovl_micro_dispatch_tnc[] = {
	{
		NULL,
		micro_alter_ovl2_tnc,
		NULL,
		NULL,
	},
};

#ifndef OVL_TNC_CACHE_QUICK_SWAP /* If caching is NOT enabled */

unsigned int micro_spritec_update_src_tnc(igd_display_context_t *display,
	ovl2_reg_tnc_t *spritec_regs_tnc,
	igd_surface_t *src_surf,
	igd_rect_t    *src_rect)
{
	unsigned int      src_Bpp;
	igd_surface_t     blt_surf;
	igd_rect_t        blt_rect_dest;
	igd_coord_t       blt_coord_src;
	igd_chroma_t      chroma;

	EMGD_TRACE_ENTER;

	/* This is in Bytes per pixel */
	src_Bpp = IGD_PF_BPP(src_surf->pixel_format)/8;

	/* src surface */
	spritec_regs_tnc->start = src_surf->offset;

	/* DSPCLINOFF - Display C/Sprite Linear Offset Register */
	EMGD_WRITE32(
		     (src_rect->y1 * src_surf->pitch) + (src_rect->x1 * src_Bpp),
		     MMIO(display) + 0x72184);

	/* DSPCTILEOFF - Display C Tiled Offset Reg */
	EMGD_WRITE32(0,
		     MMIO(display) + 0x721a4);

	/* DSPCSTRIDE - Display C/Sprite Stride Register */
	EMGD_WRITE32(src_surf->pitch,
		     MMIO(display) + 0x72188);

	/* src pixel format */
	switch(src_surf->pixel_format){
	case IGD_PF_YUV422_PACKED_YUY2:
		spritec_regs_tnc->control |= OVL2_CMD_YUV_422;
		break;
	case IGD_PF_YUV422_PACKED_UYVY:
		spritec_regs_tnc->control |= OVL2_CMD_YUV_422 | OVL2_CMD_UYVY;
		break;
	case IGD_PF_ARGB32_8888:
		spritec_regs_tnc->control |= OVL2_CMD_ARGB_8888;
		break;
	case IGD_PF_xRGB32_8888:
		spritec_regs_tnc->control |= OVL2_CMD_RGB_8888;
		break;
	case IGD_PF_RGB16_565:
		spritec_regs_tnc->control |= OVL2_CMD_RGB_565;
		break;
	case IGD_PF_ARGB8_INDEXED:
		spritec_regs_tnc->control |= OVL2_CMD_RGB_8;
		break;
	default:
		EMGD_ERROR_EXIT("Invalid pixel format: 0x%lx", src_surf->pixel_format);
		return -IGD_ERROR_HWERROR;
	}

	/* Turn off YUV to RGB conversion if the src is RGB */
	if (!(src_surf->pixel_format & PF_TYPE_YUV)) {
		spritec_regs_tnc->control |= (1<<19);
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


/*----------------------------------------------------------------------
 * Function: spritec_update_video_quality_tnc()
 *
 * Description:
 *   This function updates the contrast, brightness, and saturation of
 *   the overlay using the values specified in overlay_info.
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/

static int micro_spritec_update_video_quality_tnc(
	igd_display_context_t        *display,
	igd_surface_t                *src_surf,
	igd_ovl_video_quality_info_t *video_quality)
{
	int                          calc_brightness_tmp = 0;
	int                          calc_brightness     = 0;
	unsigned int                 calc_contrast_tmp   = 0;
	unsigned int                 calc_contrast       = 0;
	unsigned int                 calc_saturation_tmp = 0;
	unsigned int                 calc_saturation     = 0;
	unsigned int                 calc_hue_tmp = 0;
	unsigned int                 calc_hue     = 0;
	
	EMGD_TRACE_ENTER;

	/* If the src_surf pixel format is RGB, then brightness, contrast,
	 * and saturation should all be set to the exact default */
	if (src_surf->pixel_format & PF_TYPE_RGB) {
		if (video_quality->brightness != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set brightness to default");
		}
		if (video_quality->contrast != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set contrast to default");
		}
		if (video_quality->saturation != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set saturation to default");
		}
		if (video_quality->hue != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set saturation to default");
		}
		EMGD_WRITE32(OVL2_RGB_COLOR_DEF_CONT_BRGHT,
			MMIO(display) + 0x721D0);
		EMGD_WRITE32(OVL2_RGB_COLOR_DEF_SATN_HUE,
			MMIO(display) + 0x721D4);

		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/*************************************************************************
	 * Brightness
	 *************************************************************************/
	if (0x8000 == video_quality->brightness) {
		calc_brightness = MID_BRIGHTNESS_YUV;
	} else if (video_quality->brightness < 0x8000) {
		/*
		 * we have here a brightness that is less than the default
		 * mid point
		 */
		calc_brightness_tmp = 0x8000 - video_quality->brightness;
		calc_brightness_tmp <<= 14;
		calc_brightness_tmp /= 0x8000;
		calc_brightness     = -128 - MID_BRIGHTNESS_YUV;
		/*
		 * more range if the midpoint is positive but less range
		 * if midpoint is negative
		 */

		calc_brightness *= calc_brightness_tmp;
		calc_brightness += BIT13;
		calc_brightness >>= 14;

		if (calc_brightness < -128) {
			calc_brightness = -128;
		}
		if (calc_brightness > MID_BRIGHTNESS_YUV) {
			calc_brightness = MID_BRIGHTNESS_YUV;
		}
	} else {
		/*
		 * we have here a brightness that is more than the default
		 * mid point
		 */
		calc_brightness_tmp = video_quality->brightness - 0x8000;
		calc_brightness_tmp <<= 14;
		calc_brightness_tmp /= 0x8000;
		calc_brightness     = 127 - MID_BRIGHTNESS_YUV;
		/*
		 * less range if the midpoint is positive but more range
		 * if midpoint is negative
		 */
		calc_brightness *= calc_brightness_tmp;
		calc_brightness += BIT13;
		calc_brightness >>= 14;

		if (calc_brightness > 127) {
			calc_brightness = 127;
		}
		if (calc_brightness < MID_BRIGHTNESS_YUV) {
			calc_brightness = MID_BRIGHTNESS_YUV;
		}
	}

	EMGD_WRITE32((EMGD_READ32(MMIO(display) + 0x721D0) & 0xFFFFFF00) |
		(calc_brightness & 0xFF),
		MMIO(display) + 0x721D0);


	/*************************************************************************
	 * Contrast
	 *************************************************************************/
	if (0x8000 == video_quality->contrast ){
		calc_contrast = MID_CONTRAST_YUV;
	} else if (video_quality->contrast < 0x8000) {
		/* we have here a contrast that is less than the
		 * default mid point */
		calc_contrast_tmp = video_quality->contrast;
		calc_contrast_tmp <<= 12;
		calc_contrast_tmp /= 0x8000;
		calc_contrast     = MID_CONTRAST_YUV;
		calc_contrast     *= calc_contrast_tmp;
		calc_contrast     += BIT11;
		calc_contrast     >>= 12;
		if (calc_contrast > 0x3F) {
			calc_contrast = 0x3F;
		}
	} else {
		/* we have here a contrast that is more than the
		 * default mid point */
		calc_contrast_tmp = video_quality->contrast - 0x8000;
		calc_contrast_tmp <<= 12;
		calc_contrast_tmp /= 0x8000;
		calc_contrast     = (0x1FF - MID_CONTRAST_YUV);
		calc_contrast     *= calc_contrast_tmp;
		calc_contrast     += BIT11;
		calc_contrast     >>= 12;
		calc_contrast     += MID_CONTRAST_YUV;
		if (calc_contrast > 0x1FF) {
			calc_contrast = 0x1FF;
		}
	}

	EMGD_WRITE32((EMGD_READ32(MMIO(display) + 0x721D0) & 0xF803FFFF) |
		((calc_contrast & 0x1FF) << 18),
		MMIO(display) + 0x721D0);

	/*************************************************************************
	 * Saturation
	 *************************************************************************/
	if (video_quality->saturation == 0x8000) {
		calc_saturation = MID_SATURATION_YUV;
	} else if (video_quality->saturation < 0x8000) {
		/* we have here a saturation that is less than the default
		 * mid point */
		calc_saturation_tmp = video_quality->saturation;
		calc_saturation_tmp <<= 12;
		calc_saturation_tmp /= 0x8000;
		calc_saturation     = MID_SATURATION_YUV;
		calc_saturation     *= calc_saturation_tmp;
		calc_saturation     += BIT11;
		calc_saturation     >>= 12;
		if (calc_saturation > 0x7F) {
			calc_saturation = 0x7F;
		}
	} else {
		/* we have here a saturation that is more than the default
		 * mid point*/
		calc_saturation_tmp = video_quality->saturation - 0x8000;
		calc_saturation_tmp <<= 12;
		calc_saturation_tmp /= 0x8000;
		calc_saturation     = (0x3FF - MID_SATURATION_YUV);
		calc_saturation     *= calc_saturation_tmp;
		calc_saturation     += BIT11;
		calc_saturation     >>= 12;
		calc_saturation     += MID_SATURATION_YUV;

		if (calc_saturation > 0x3FF) {
			calc_saturation = 0x3FF;
		}
	}

	EMGD_WRITE32((EMGD_READ32(MMIO(display) + 0x721D4) & 0xFFFFFC00) |
		(calc_saturation & 0x3FF),
		MMIO(display) + 0x721D4);

	/*************************************************************************
	 * hue
	 *************************************************************************/
	if (video_quality->hue == 0x8000) {
		calc_hue = 0;
		} else if (video_quality->hue < 0x8000) {
				/* we have here a hue that is less than the middle value
				 * get 2's complement value decrease from 0x3ff to 0 */

				calc_hue_tmp = 0x8000 - video_quality->hue;
				calc_hue_tmp <<= 12;
				calc_hue_tmp /= 0x8000;
				calc_hue     = 0x3FF;
				calc_hue     *= calc_hue_tmp;
				calc_hue     += BIT11;
				calc_hue     >>= 12;
				
				if (calc_hue > 0x3FF) {
					calc_hue = 0x3FF;
				}
				/*  2's complement negative value equal the corresponding
				 *  positive value reverse every bits then plus 1 
				 * and add signed bit, then the calc_hue is from -0x3ff to 0 */

				calc_hue = (~calc_hue + 1) & 0x7ff;
		} else {
			/* we have here a saturation that is more than the middle value
			 * get 2's complement value increase from 0 to 0x3ff */

			calc_hue_tmp = video_quality->hue - 0x8000;
			calc_hue_tmp <<= 12;
			calc_hue_tmp /= 0x8000;
				calc_hue     = 0x3FF;
				calc_hue     *= calc_hue_tmp;
				calc_hue     += BIT11;
				calc_hue     >>= 12;

				if (calc_hue > 0x3FF) {
					calc_hue = 0x3FF;
				}
		}
	/* set 11bits hue value to register */
	EMGD_WRITE32((EMGD_READ32(MMIO(display) + 0x721D4) & 0xF800FFFF) |
		(calc_hue & 0x7FF) << 16,
		MMIO(display) + 0x721D4);


	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

/*-----------------------------------------------------------------------------
 * Function: spritec_update_gamma_tnc()
 *
 * Description:
 *    This function sets the gamma correction values for the overlays.
 *
 * Returns:
 *   != 0 on Error
 *   IGD_SUCCESS on Success
 *---------------------------------------------------------------------------*/
static int micro_spritec_update_gamma_tnc(
	igd_display_context_t *display,
	ovl2_reg_tnc_t *spritec_regs_tnc,
	igd_ovl_gamma_info_t * ovl_gamma)
{
	const int gamma_reg_input[OVL2_TOTAL_GAMMA_REG] = {8, 16, 32, 64, 128, 192};
	const int gamma_reg_offset[OVL2_TOTAL_GAMMA_REG] = {
		OVL2_REG_ADDR_GAMMA0,
		OVL2_REG_ADDR_GAMMA1,
		OVL2_REG_ADDR_GAMMA2,
		OVL2_REG_ADDR_GAMMA3,
		OVL2_REG_ADDR_GAMMA4,
		OVL2_REG_ADDR_GAMMA5
	};
	const unsigned int gamma_def[OVL2_TOTAL_GAMMA_REG] = {
		0x00080808,
		0x00101010,
		0x00202020,
		0x00404040,
		0x00808080,
		0x00c0c0c0
	};
	unsigned int          new_gamma_red_24i_8f, new_gamma_green_24i_8f;
	unsigned int          new_gamma_blue_24i_8f;
	unsigned int          gamma_normal_r_24i_8f;
	unsigned int          gamma_normal_g_24i_8f;
	unsigned int          gamma_normal_b_24i_8f;
	unsigned int          gamma_reg, gamma_reg_24i_8f;
	unsigned int          i;

	EMGD_TRACE_ENTER;

	/* FIXME: The gamma values are re-written for every alter_ovl call.
	 * This may cause issues or may be to slow?  If so, store the previous
	 * values and only re-write when they change. */

	/* If the overlay gamma is disabled or the display is an 8 bit mode
	 * (second overay can not support gamma in an 8 bit mode),
	 * set it to the default */
	if (((ovl_gamma->flags & IGD_OVL_GAMMA_ENABLE) == IGD_OVL_GAMMA_DISABLE) ||
		(PLANE(display)->fb_info->pixel_format == IGD_PF_ARGB8_INDEXED)) {
		for (i = 0; i < OVL2_TOTAL_GAMMA_REG; i++) {
			/* program register */
			EMGD_WRITE32(gamma_def[i], MMIO(display) + gamma_reg_offset[i]);
		}
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/* It is assumed that the input value is a 24-bit number */
	new_gamma_red_24i_8f   = ovl_gamma->red;
	new_gamma_green_24i_8f = ovl_gamma->green;
	new_gamma_blue_24i_8f  = ovl_gamma->blue;

	/*
	 * Program RGB for each of the 6 gamma registers
	 */

	/* Since the OS_POW_FIX function can only take an integer base,
	 * we need to normalize the result by gamma_normal_x
	 */
	gamma_normal_r_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_red_24i_8f);
	gamma_normal_g_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_green_24i_8f);
	gamma_normal_b_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_blue_24i_8f);

	for( i = 0; i < OVL2_TOTAL_GAMMA_REG; i++ )
	{
		/* red */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
								(1<<16)/new_gamma_red_24i_8f);
		gamma_reg        =
			((255 * gamma_reg_24i_8f) / gamma_normal_r_24i_8f) << 16;

		/* green */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
							(1<<16)/new_gamma_green_24i_8f);
		gamma_reg        |=
			((255 * gamma_reg_24i_8f) / gamma_normal_g_24i_8f) << 8;

		/* blue */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
							(1<<16)/new_gamma_blue_24i_8f);
		gamma_reg        |=
			((255 * gamma_reg_24i_8f) / gamma_normal_b_24i_8f);

		/* turn overlay off (TBD) */

		/* program register */
		EMGD_WRITE32(gamma_reg, MMIO(display) + gamma_reg_offset[i]);

		/* turn overlay on (TBD) */
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

#endif // End if caching is not enabled.

static unsigned int convert_color_key_to_hw (
	unsigned long pf,
	unsigned int input)
{
	unsigned int output;

	switch (pf) {
	case IGD_PF_ARGB32:
	case IGD_PF_xRGB32:
	case IGD_PF_ARGB8_INDEXED:
	default:
		output = input;
		break;
	case IGD_PF_RGB16_565:
		output =
			((((input & 0xf800)>>11)<<3)<<16) |
			((((input & 0x07e0)>>5 )<<2)<<8 ) |
			((((input & 0x001f)>>0 )<<3)<<0 );
		break;
	}

	return output;
}
static unsigned int convert_color_key_to_mask (
	unsigned long pf,
	unsigned int input)
{
	unsigned int output;

	switch (pf) {
	case IGD_PF_ARGB32:
	case IGD_PF_xRGB32:
	default:
		output = 0x00ffffff;
		break;
	case IGD_PF_RGB16_565:
		output = 0x00f8fcf8;
		break;
	case IGD_PF_ARGB8_INDEXED:
		output = 0x000000ff;
		break;
	}

	return output;
}

/* Convert YUV to UVY for YUV pixel formats.
 * Do not convert RGB pixel formats */
static unsigned int yuv_to_uvy(
	unsigned long pf,
	unsigned int input)
{
	unsigned int output;

	if (IGD_PF_TYPE(pf) & PF_TYPE_YUV) {
		output =
			((input & 0x00ff0000) >> 16) |
			((input & 0x0000ff00) << 8)  |
			((input & 0x000000ff) << 8);
	} else {
		output = input;
	}

	return output;
}

#ifndef OVL_TNC_CACHE_QUICK_SWAP /* If caching is NOT enabled */

static unsigned int micro_spritec_update_regs_tnc(
	igd_display_context_t *display,
	ovl2_reg_tnc_t      *spritec_regs_tnc,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	unsigned int plane_control, orig_plane_control, plane_start;
	unsigned int pipe_num;
	unsigned int ckey_low, ckey_high;
	int ret;


	EMGD_TRACE_ENTER;

	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {
		/* Turn off the plane control key enable and the second overlay
		 * control. */
		plane_control = EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg);
		orig_plane_control = plane_control;

		if ((plane_control & (3<<22)) != 0) {
			plane_control &= ~(3<<22);
		}
		if((plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_ARGB_8888) {
			plane_control &= ~0x04000000;
		}
		if(plane_control != orig_plane_control) {
			EMGD_WRITE32(plane_control,
				MMIO(display) + PLANE(display)->plane_reg);
			plane_start =
				EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg + 0x1c);
			EMGD_WRITE32(plane_start,
				MMIO(display) + PLANE(display)->plane_reg + 0x1c);
		}

		/* Turn the overlay Off.
		 * Ensure we are using the correct Pipe. */
		pipe_num = PIPE(display)->pipe_num ?
			(1<<24)/*Pipe B*/ :
			(0<<24)/*Pipe A*/;
		EMGD_WRITE32(pipe_num, MMIO(display) + MMIO_OFFSET_TNC2_CONTROL);
		EMGD_WRITE32(0, MMIO(display) + MMIO_OFFSET_TNC2_LINEAR_OFF);
		EMGD_WRITE32(0, MMIO(display) + MMIO_OFFSET_TNC2_SURF_ADDR);

		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/*************************************************************************
	 * Copy the information passed in to the HW overlay structure
	 *************************************************************************/
	/* Zero the control, since they will be OR'ed in with data
	 * below */
	spritec_regs_tnc->control = 0;

	/* Interleaved/progressive and Odd/Even if interleaved */
	if (flags & IGD_OVL_ALTER_INTERLEAVED) {
		EMGD_ERROR("Overlay2 does not support Interleaved");
	}

	/* Dest rect information */
	EMGD_WRITE32((dest_rect->y1<<16) | (dest_rect->x1),
		MMIO(display) + 0x7218C);
	EMGD_WRITE32(((dest_rect->y2 - dest_rect->y1 - 1)<<16) |
		(dest_rect->x2 - dest_rect->x1 - 1),
		MMIO(display) + 0x72190);

	/* Src rect and surface information */
	ret = micro_spritec_update_src_tnc(display, spritec_regs_tnc, src_surf, src_rect);
	if (ret) {
		EMGD_ERROR_EXIT("Overlay2 updating src failed");
		return ret;
	}

	/* Color control information */
	if (!(flags & IGD_OVL_OSD_ON_SPRITEC)) {
		ret = micro_spritec_update_video_quality_tnc(display, src_surf,
			&ovl_info->video_quality);
		if (ret) {
			EMGD_ERROR_EXIT("Overlay2 video quality failed");
			return ret;
		}
		ret = micro_spritec_update_gamma_tnc(display, spritec_regs_tnc, &ovl_info->gamma);
		if (ret) {
			EMGD_ERROR("Overlay2 gamma failed");
			return ret;
		}
	}

	/* Destination color key */
	plane_control = EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg);
	orig_plane_control = plane_control;

	EMGD_DEBUG("Color key.flags: 0x%lx", ovl_info->color_key.flags);
	if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {
		EMGD_DEBUG("Overlay Enable Dest Color Key");
		/* The mask and color key are different for the different
		 * pixel formats */
		EMGD_WRITE32(convert_color_key_to_hw(
			PLANE(display)->fb_info->pixel_format, ovl_info->color_key.dest),
			MMIO(display) + PLANE(display)->plane_reg +
			MMIO_OFFSET_TNC2_COLORKEY_HW);
		EMGD_WRITE32(convert_color_key_to_mask(
			PLANE(display)->fb_info->pixel_format, ovl_info->color_key.dest),
			MMIO(display) + PLANE(display)->plane_reg +
			MMIO_OFFSET_TNC2_COLORKEY_MASK);

		/* Both the plane control key enable and the second overlay
		 * control order must be enabled to turn on destination color
		 * key.  Also rewrite the plane_start which is the trigger for
		 * Plane A/B */
		plane_control |= (3<<22);

		if (flags & IGD_OVL_OSD_ON_SPRITEC) {
			/* Sprite C should be below plane and above first overlay */
			spritec_regs_tnc->control |= 1;
		} else {
			/* Sprite C should be on the bottom of the Z order. */
			spritec_regs_tnc->control |= 4;
		}
	} else {
		EMGD_DEBUG("Overlay Disable Dest Color Key");
		plane_control &= ~(3<<22);
	}

	if(ovl_info->color_key.flags & IGD_OVL_DST_BLEND_ENABLE) {
		/*
		 * If Overlay + FB Blend is requested and the FB is xRGB
		 * turn on the ARGB format.
		 */
		if((plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_RGB_8888) {
			plane_control |= DSPxCNTR_ARGB_8888;
		}

		/* Dest Blend requires Plane C on the bottom of the Z order */
		spritec_regs_tnc->control |= 4;
	} else {
		if((plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_ARGB_8888) {
			plane_control &= ~0x04000000;
		}
	}

	if(plane_control != orig_plane_control) {
		EMGD_WRITE32(plane_control, MMIO(display) + PLANE(display)->plane_reg);
		plane_start = EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg + 0x1c);

		/* trigger registers must be updated using a command packet */
		ret = micro_spritec_send_instr_tnc(display, PLANE(display)->plane_reg +
			0x1c, plane_start);

		/* Note: On a 915GM (maybe other platforms as well), the dest
		 * color key is not always enabled when the plane A/B and
		 * plane C are both modified in the same vblank.  So, ensure
		 * they occur on a different vblank.  This should be fine,
		 * since this will only occur the first time when enabling
		 * the dest color key. */
		display->context->dispatch.wait_vblank((igd_display_h)display);


	}

	/* Source Color key */
	if (ovl_info->color_key.flags & IGD_OVL_SRC_COLOR_KEY_ENABLE) {
		EMGD_DEBUG("Overlay Enable Src Color Key");

		ckey_high = convert_color_key_to_hw(src_surf->pixel_format,
			ovl_info->color_key.src_hi);
		ckey_high = yuv_to_uvy(src_surf->pixel_format,
			ckey_high);

		ckey_low = convert_color_key_to_hw(src_surf->pixel_format,
			ovl_info->color_key.src_lo);
		ckey_low = yuv_to_uvy(src_surf->pixel_format,
			ckey_low);

		EMGD_WRITE32(ckey_low,  MMIO(display) + 0x72194);
		EMGD_WRITE32(ckey_high, MMIO(display) + 0x721a0);
		EMGD_WRITE32(7,         MMIO(display) + 0x72198);
		spritec_regs_tnc->control |= (1<<22);
	} else {
		EMGD_DEBUG("Overlay Disable Src Color Key");
		EMGD_WRITE32(0,         MMIO(display) + 0x72198);
	}

	/* General overlay information.  Turn the second overlay on.
	 * The trigger register is the start register which causes the
	 * overlay to update.  The trigger register is written in send_instr */
	spritec_regs_tnc->control |= (1<<31);
	spritec_regs_tnc->control |= PIPE(display)->pipe_num ?
		(1<<24)/*Pipe B*/ :
		(0<<24)/*Pipe A*/;
	EMGD_WRITE32(spritec_regs_tnc->control,
		MMIO(display) + MMIO_OFFSET_TNC2_CONTROL);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

#else /* OVL_TNC_CACHE_QUICK_SWAP is enabled. */

/* Atom E6xx overlay cache structure */
static ovl_tnc_cache_t ovl_cache;

/* Flag to signal the cache is invalid and needs
 * to be re-initialized */
static int ovl_cache_needs_init = TRUE;



/*----------------------------------------------------------------------
 * Function: micro_spritec_update_src_ptr_tnc()
 *
 * Description:
 *   This function updates the source offset
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/
unsigned int micro_spritec_update_src_ptr_tnc(igd_surface_t *src_surf,
					  igd_rect_t    *src_rect)
{
	unsigned int      src_Bpp;

	EMGD_TRACE_ENTER;

	/* This is in Bytes per pixel */
	src_Bpp = IGD_PF_BPP(src_surf->pixel_format) / 8;

	/* src surface */
	ovl_cache.ovl2_regs.start = src_surf->offset;

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

/*----------------------------------------------------------------------
 * Function: micro_spritec_update_src_tnc()
 *
 * Description:
 *   This function updates the source pitch and pixel format
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/
unsigned int micro_spritec_update_src_tnc(
	igd_display_context_t *display,
	igd_surface_t *src_surf,
	igd_rect_t    *src_rect)
{
	EMGD_TRACE_ENTER;

	ovl_cache.ovl2_regs.linear_offset =
		(src_rect->y1 * src_surf->pitch) +
		(src_rect->x1 * (IGD_PF_BPP(src_surf->pixel_format)/8));
	ovl_cache.ovl2_regs.pitch = src_surf->pitch;
	ovl_cache.ovl2_regs.tiled_offset = 0;

	/* src pixel format */
	switch(src_surf->pixel_format){
	case IGD_PF_YUV422_PACKED_YUY2:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_YUV_422;
		break;
	case IGD_PF_YUV422_PACKED_UYVY:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_YUV_422 | OVL2_CMD_UYVY;
		break;
	case IGD_PF_ARGB32_8888:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_ARGB_8888;
		break;
	case IGD_PF_xRGB32_8888:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_RGB_8888;
		break;
	case IGD_PF_RGB16_565:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_RGB_565;
		break;
	case IGD_PF_ARGB8_INDEXED:
		ovl_cache.ovl2_regs.control |= OVL2_CMD_RGB_8;
		break;
	default:
		EMGD_ERROR_EXIT("Invalid pixel format: 0x%lx", src_surf->pixel_format);
		return -IGD_ERROR_HWERROR;
	}

	/* Turn off YUV to RGB conversion if the src is RGB */
	if (!(src_surf->pixel_format & PF_TYPE_YUV)) {
		ovl_cache.ovl2_regs.control |= (1<<19);
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

/*----------------------------------------------------------------------
 * Function: micro_spritec_update_video_quality_tnc()
 *
 * Description:
 *   This function updates the contrast, brightness, and saturation of
 *   the overlay using the values specified in overlay_info.
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/

static int micro_spritec_update_video_quality_tnc(
	igd_display_context_t        *display,
	igd_surface_t                *src_surf,
	igd_ovl_video_quality_info_t *video_quality)
{
	int                          calc_brightness_tmp = 0;
	int                          calc_brightness     = 0;
	unsigned int                 calc_contrast_tmp   = 0;
	unsigned int                 calc_contrast       = 0;
	unsigned int                 calc_saturation_tmp = 0;
	unsigned int                 calc_saturation     = 0;
	unsigned int                 calc_hue_tmp = 0;
	unsigned int                 calc_hue     = 0;
	EMGD_TRACE_ENTER;

	/* If the src_surf pixel format is RGB, then brightness, contrast,
	 * and saturation should all be set to the exact default */
	if (src_surf->pixel_format & PF_TYPE_RGB) {
		if (video_quality->brightness != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set brightness to default");
		}
		if (video_quality->contrast != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set contrast to default");
		}
		if (video_quality->saturation != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set saturation to default");
		}
		if (video_quality->hue != 0x8000) {
			EMGD_DEBUG("RGB surfaces must set saturation to default");
		}
		ovl_cache.ovl2_regs.cont_bright = OVL2_RGB_COLOR_DEF_CONT_BRGHT;
		ovl_cache.ovl2_regs.satn_hue =  OVL2_RGB_COLOR_DEF_SATN_HUE;

		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/*************************************************************************
	 * Brightness
	 *************************************************************************/
	if (0x8000 == video_quality->brightness) {
		calc_brightness = MID_BRIGHTNESS_YUV;
	} else if (video_quality->brightness < 0x8000) {
		/*
		 * we have here a brightness that is less than the default
		 * mid point
		 */
		calc_brightness_tmp = 0x8000 - video_quality->brightness;
		calc_brightness_tmp <<= 14;
		calc_brightness_tmp /= 0x8000;
		calc_brightness     = -128 - MID_BRIGHTNESS_YUV;
		/*
		 * more range if the midpoint is positive but less range
		 * if midpoint is negative
		 */

		calc_brightness *= calc_brightness_tmp;
		calc_brightness += BIT13;
		calc_brightness >>= 14;

		if (calc_brightness < -128) {
			calc_brightness = -128;
		}
		if (calc_brightness > MID_BRIGHTNESS_YUV) {
			calc_brightness = MID_BRIGHTNESS_YUV;
		}
	} else {
		/*
		 * we have here a brightness that is more than the default
		 * mid point
		 */
		calc_brightness_tmp = video_quality->brightness - 0x8000;
		calc_brightness_tmp <<= 14;
		calc_brightness_tmp /= 0x8000;
		calc_brightness     = 127 - MID_BRIGHTNESS_YUV;
		/*
		 * less range if the midpoint is positive but more range
		 * if midpoint is negative
		 */
		calc_brightness *= calc_brightness_tmp;
		calc_brightness += BIT13;
		calc_brightness >>= 14;

		if (calc_brightness > 127) {
			calc_brightness = 127;
		}
		if (calc_brightness < MID_BRIGHTNESS_YUV) {
			calc_brightness = MID_BRIGHTNESS_YUV;
		}
	}

	ovl_cache.ovl2_regs.cont_bright =
		(ovl_cache.ovl2_regs.cont_bright & 0xFFFFFF00) |
		(calc_brightness & 0xFF);


	/*************************************************************************
	 * Contrast
	 *************************************************************************/
	if (0x8000 == video_quality->contrast ){
		calc_contrast = MID_CONTRAST_YUV;
	} else if (video_quality->contrast < 0x8000) {
		/* we have here a contrast that is less than the
		 * default mid point */
		calc_contrast_tmp = video_quality->contrast;
		calc_contrast_tmp <<= 12;
		calc_contrast_tmp /= 0x8000;
		calc_contrast     = MID_CONTRAST_YUV;
		calc_contrast     *= calc_contrast_tmp;
		calc_contrast     += BIT11;
		calc_contrast     >>= 12;
		if (calc_contrast > 0x3F) {
			calc_contrast = 0x3F;
		}
	} else {
		/* we have here a contrast that is more than the
		 * default mid point */
		calc_contrast_tmp = video_quality->contrast - 0x8000;
		calc_contrast_tmp <<= 12;
		calc_contrast_tmp /= 0x8000;
		calc_contrast     = (0x1FF - MID_CONTRAST_YUV);
		calc_contrast     *= calc_contrast_tmp;
		calc_contrast     += BIT11;
		calc_contrast     >>= 12;
		calc_contrast     += MID_CONTRAST_YUV;
		if (calc_contrast > 0x1FF) {
			calc_contrast = 0x1FF;
		}
	}

	ovl_cache.ovl2_regs.cont_bright =
		(ovl_cache.ovl2_regs.cont_bright & 0xF803FFFF ) |
		((calc_contrast & 0x1FF) << 18);


	/*************************************************************************
	 * Saturation
	 *************************************************************************/
	if (video_quality->saturation == 0x8000) {
		calc_saturation = MID_SATURATION_YUV;
	} else if (video_quality->saturation < 0x8000) {
		/* we have here a saturation that is less than the default
		 * mid point */
		calc_saturation_tmp = video_quality->saturation;
		calc_saturation_tmp <<= 12;
		calc_saturation_tmp /= 0x8000;
		calc_saturation     = MID_SATURATION_YUV;
		calc_saturation     *= calc_saturation_tmp;
		calc_saturation     += BIT11;
		calc_saturation     >>= 12;
		if (calc_saturation > 0x7F) {
			calc_saturation = 0x7F;
		}
	} else {
		/* we have here a saturation that is more than the default
		 * mid point*/
		calc_saturation_tmp = video_quality->saturation - 0x8000;
		calc_saturation_tmp <<= 12;
		calc_saturation_tmp /= 0x8000;
		calc_saturation     = (0x3FF - MID_SATURATION_YUV);
		calc_saturation     *= calc_saturation_tmp;
		calc_saturation     += BIT11;
		calc_saturation     >>= 12;
		calc_saturation     += MID_SATURATION_YUV;

		if (calc_saturation > 0x3FF) {
			calc_saturation = 0x3FF;
		}
	}
	
	ovl_cache.ovl2_regs.satn_hue =
		(ovl_cache.ovl2_regs.satn_hue & 0xFFFFFC00 ) |
		(calc_saturation & 0x3FF);

	/*************************************************************************
	 * hue
	 *************************************************************************/
	if (video_quality->hue == 0x8000) {
		calc_hue = 0;
		} else if (video_quality->hue < 0x8000) {
				/* we have here a hue that is less than the middle value
				 * get 2's complement value decrease from 0x3ff to 0 */

				calc_hue_tmp = 0x8000 - video_quality->hue;
				calc_hue_tmp <<= 12;
				calc_hue_tmp /= 0x8000;
				calc_hue     = 0x3FF;
				calc_hue     *= calc_hue_tmp;
				calc_hue     += BIT11;
				calc_hue     >>= 12;
				
				if (calc_hue > 0x3FF) {
					calc_hue = 0x3FF;
				}
				/*  2's complement negative value equal the corresponding
				 *  positive value reverse every bits then plus 1 
				 * and add signed bit, then the calc_hue is from -0x3ff to 0 */

				calc_hue = (~calc_hue + 1) & 0x7ff;
		} else {
			/* we have here a saturation that is more than the middle value
			 * get 2's complement value increase from 0 to 0x3ff */

			calc_hue_tmp = video_quality->hue - 0x8000;
			calc_hue_tmp <<= 12;
			calc_hue_tmp /= 0x8000;
				calc_hue     = 0x3FF;
				calc_hue     *= calc_hue_tmp;
				calc_hue     += BIT11;
				calc_hue     >>= 12;

				if (calc_hue > 0x3FF) {
					calc_hue = 0x3FF;
				}
		}
		
	/* set 11bits hue value to register */	
	ovl_cache.ovl2_regs.satn_hue =
		(ovl_cache.ovl2_regs.satn_hue & 0xF800FFFF ) |
		((calc_hue & 0x7FF) << 16);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


static const int gamma_reg_input[OVL2_TOTAL_GAMMA_REG] = {8, 16, 32, 64, 128, 192};
static const int gamma_reg_offset[OVL2_TOTAL_GAMMA_REG] = {
	OVL2_REG_ADDR_GAMMA0,
	OVL2_REG_ADDR_GAMMA1,
	OVL2_REG_ADDR_GAMMA2,
	OVL2_REG_ADDR_GAMMA3,
	OVL2_REG_ADDR_GAMMA4,
	OVL2_REG_ADDR_GAMMA5
};
static const unsigned int gamma_def[OVL2_TOTAL_GAMMA_REG] = {
	0x00080808,
	0x00101010,
	0x00202020,
	0x00404040,
	0x00808080,
	0x00c0c0c0
};

/*-----------------------------------------------------------------------------
 * Function: spritec_update_gamma_tnc()
 *
 * Description:
 *    This function sets the gamma correction values for the overlays.
 *
 * Returns:
 *   != 0 on Error
 *   IGD_SUCCESS on Success
 *---------------------------------------------------------------------------*/
static int micro_spritec_update_gamma_tnc(
	igd_display_context_t *display,
	igd_ovl_gamma_info_t *ovl_gamma)
{
	unsigned int          new_gamma_red_24i_8f, new_gamma_green_24i_8f;
	unsigned int          new_gamma_blue_24i_8f;
	unsigned int          gamma_normal_r_24i_8f;
	unsigned int          gamma_normal_g_24i_8f;
	unsigned int          gamma_normal_b_24i_8f;
	unsigned int          gamma_reg, gamma_reg_24i_8f;
	unsigned int          i;

	EMGD_TRACE_ENTER;

	/* FIXME: The gamma values are re-written for every alter_ovl call.
	 * This may cause issues or may be to slow?  If so, store the previous
	 * values and only re-write when they change. */

	/* If the overlay gamma is disabled or the display is an 8 bit mode
	 * (second overay can not support gamma in an 8 bit mode),
	 * set it to the default */
	if (((ovl_gamma->flags&IGD_OVL_GAMMA_ENABLE)==IGD_OVL_GAMMA_DISABLE) ||
	     (PLANE(display)->fb_info->pixel_format == IGD_PF_ARGB8_INDEXED)) {
		for (i = 0; i < OVL2_TOTAL_GAMMA_REG; i++) {
			/* program register */
			ovl_cache.ovl2_regs.gamma_regs[i] = gamma_def[i];
		}
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/* It is assumed that the input value is a 24-bit number */
	new_gamma_red_24i_8f   = ovl_gamma->red;
	new_gamma_green_24i_8f = ovl_gamma->green;
	new_gamma_blue_24i_8f  = ovl_gamma->blue;

	/*
	 * Program RGB for each of the 6 gamma registers
	 */

	/* Since the OS_POW_FIX function can only take an integer base,
	 * we need to normalize the result by gamma_normal_x
	 */
	gamma_normal_r_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_red_24i_8f);
	gamma_normal_g_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_green_24i_8f);
	gamma_normal_b_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_blue_24i_8f);

	for( i = 0; i < OVL2_TOTAL_GAMMA_REG; i++ )
	{
		/* red */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
					      (1<<16)/new_gamma_red_24i_8f);
		gamma_reg =
			((255 * gamma_reg_24i_8f) / gamma_normal_r_24i_8f) << 16;

		/* green */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
					      (1<<16)/new_gamma_green_24i_8f);
		gamma_reg        |=
			((255 * gamma_reg_24i_8f) / gamma_normal_g_24i_8f) << 8;

		/* blue */
		gamma_reg_24i_8f = OS_POW_FIX(gamma_reg_input[i],
					      (1<<16)/new_gamma_blue_24i_8f);
		gamma_reg        |=
			((255 * gamma_reg_24i_8f) / gamma_normal_b_24i_8f);

		/* program register */
		ovl_cache.ovl2_regs.gamma_regs[i] = gamma_reg;

	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


/*----------------------------------------------------------------------
 * Function: micro_spritec_disable_ovl_tnc()
 *
 * Description:
 * Write the registers needed to turn off the overlay.
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/

static unsigned int micro_spritec_disable_ovl_tnc(
	igd_display_context_t *display	)
{
	unsigned int pipe_num;
	unsigned int plane_control, orig_plane_control, plane_start;

	EMGD_TRACE_ENTER;

	/* Turn off the plane control key enable and the second overlay
	 * control. */
	plane_control =
		EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg);
	orig_plane_control = plane_control;

	if ((plane_control & (3<<22)) != 0) {
		plane_control &= ~(3<<22);
	}
	if(((plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_ARGB_8888) && 
		(!display->context->mod_dispatch.fb_blend_ovl_override)) {
		/* Preserve the state of the register if FB blend +
		 * Ovl override is invoked */
		plane_control &= ~0x04000000;
	}
	if(plane_control != orig_plane_control) {
		EMGD_WRITE32(plane_control,
			MMIO(display) + PLANE(display)->plane_reg);
		plane_start = EMGD_READ32(
			MMIO(display) + PLANE(display)->plane_reg + 0x1c);
		EMGD_WRITE32(plane_start,
			MMIO(display) + PLANE(display)->plane_reg + 0x1c);
	}

	/* Turn the overlay Off.
	 * Ensure we are using the correct Pipe. */
	pipe_num = PIPE(display)->pipe_num ?
		(1<<24)/*Pipe B*/ :
		(0<<24)/*Pipe A*/;
	EMGD_WRITE32(pipe_num, MMIO(display) + MMIO_OFFSET_TNC2_CONTROL);
	EMGD_WRITE32(0, MMIO(display) + MMIO_OFFSET_TNC2_SURF_ADDR);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}



/*----------------------------------------------------------------------
 * Function: micro_spritec_clear_cache_tnc()
 *
 * Description:
 *
 *----------------------------------------------------------------------*/

static void micro_spritec_clear_cache_tnc(
        igd_display_context_t *display,
	unsigned int           flags)
{
	/* Force every cache check to miss */
	OS_MEMSET(&ovl_cache, 0, sizeof(ovl_tnc_cache_t));

	/* We just set our cached flags to 0, which might accidently
	 * match up with "OFF" for some important incoming flag
	 * bits, causing us to think we already handled them when
	 * we didn't.  So set our cached flags to the exact
	 * opposite of the incoming flags, which will force
	 * us to test and handle every single bit, regardless
	 * of whether it is on or off. */
	ovl_cache.flags = ~flags;

	/* init our cached registers */
	ovl_cache.ovl2_regs.plane_control =
		EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg);
	ovl_cache.ovl2_regs.plane_start =
		EMGD_READ32(MMIO(display) + PLANE(display)->plane_reg + 0x1c);

	/* initialization complete */
	ovl_cache_needs_init = FALSE;

}


/*-----------------------------------------------------------------------------
 * Function: spritec_update_colorkey_tnc()
 *
 * Description:
 *    This function sets the colorkey values for the overlays.
 *
 * Returns:
 *   != 0 on Error
 *   IGD_SUCCESS on Success
 *---------------------------------------------------------------------------*/
static void micro_spritec_update_colorkey_tnc(
	igd_display_context_t *display,
	igd_surface_t         *src_surf,
	igd_ovl_info_t        *ovl_info)
{
	unsigned int ckey_low, ckey_high;

	/* Destination color key */
	EMGD_DEBUG("Color key.flags: 0x%lx", ovl_info->color_key.flags);

	if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {

		EMGD_DEBUG("Overlay Enable Dest Color Key");
		/* The mask and color key are different for the
		 * different pixel formats */
		ovl_cache.ovl2_regs.colorkey_hw =
			convert_color_key_to_hw
			(PLANE(display)->fb_info->pixel_format,
			 ovl_info->color_key.dest);

		ovl_cache.ovl2_regs.colorkey_mask =
			convert_color_key_to_mask
			(PLANE(display)->fb_info->pixel_format,
			 ovl_info->color_key.dest);
		/*
		 * Both the plane control key enable and the second
		 * overlay control order must be enabled to turn on
		 * destination color key.  Also rewrite the
		 * plane_start which is the trigger for Plane A/B
		 */
		ovl_cache.ovl2_regs.plane_control |= (3<<22);

	} else {
		EMGD_DEBUG("Overlay Disable Dest Color Key");
		ovl_cache.ovl2_regs.plane_control &= ~(3<<22);
	}

	if(ovl_info->color_key.flags & IGD_OVL_DST_BLEND_ENABLE) {
		/*
		 * If Overlay + FB Blend is requested and the FB is xRGB
		 * turn on the ARGB format.
		 */
		if((ovl_cache.ovl2_regs.plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_RGB_8888) {
			ovl_cache.ovl2_regs.plane_control |= DSPxCNTR_ARGB_8888;
		}
	} else {
		if(((ovl_cache.ovl2_regs.plane_control & DSPxCNTR_SRC_FMT_MASK) == DSPxCNTR_ARGB_8888) && 
			(!display->context->mod_dispatch.fb_blend_ovl_override)){
			/* Preserve the state of the register if FB blend +
			 * Ovl override is invoked */
			ovl_cache.ovl2_regs.plane_control &= ~0x04000000;
		}
	}

	/* Source Color key */
	if (ovl_info->color_key.flags & IGD_OVL_SRC_COLOR_KEY_ENABLE) {
		EMGD_DEBUG("Overlay Enable Src Color Key");

		ckey_high = convert_color_key_to_hw
			(src_surf->pixel_format,
			 ovl_info->color_key.src_hi);
		ckey_high = yuv_to_uvy(src_surf->pixel_format,
				       ckey_high);

		ckey_low = convert_color_key_to_hw
			(src_surf->pixel_format,
			 ovl_info->color_key.src_lo);
		ckey_low = yuv_to_uvy(src_surf->pixel_format,
				      ckey_low);

		ovl_cache.ovl2_regs.ckey_low = ckey_low;
		ovl_cache.ovl2_regs.ckey_high = ckey_high;
		ovl_cache.ovl2_regs.ckey_enable = 7;
		ovl_cache.ovl2_regs.control |= (1<<22);
	} else {
		EMGD_DEBUG("Overlay Disable Src Color Key");
		ovl_cache.ovl2_regs.ckey_enable = 0;
	}

}




/*----------------------------------------------------------------------
 * Function: micro_spritec_write_cache_tnc()
 *
 * Description:
 *
 *----------------------------------------------------------------------*/

static void micro_spritec_write_cache_tnc(
        igd_display_context_t *display,
	ovl2_reg_tnc_t        *spritec_regs_tnc,
	igd_ovl_info_t        *ovl_info,
	int                    cache_changed)
{
	int i;

	/*
	 * Now write all the changed registers to the HW
	 * TODO: Or should we write all the registers, regardless of
	 * if they have changed?
	 * TODO: It may be beneficial to turn off overlay while
	 * updateing the regs?
	*/


	/* Write dest rect information */
    if (cache_changed & IGD_OVL_TNC_UPDATE_DEST) {
		EMGD_WRITE32( ovl_cache.ovl2_regs.dest_rect_x1y1,
			    MMIO(display) + MMIO_OFFSET_TNC2_DEST_RECT_X1Y1);
		EMGD_WRITE32( ovl_cache.ovl2_regs.dest_rect_w_h,
			    MMIO(display) + MMIO_OFFSET_TNC2_DEST_RECT_W_H);
	}


	/* Write source information */
    if (cache_changed & (IGD_OVL_TNC_UPDATE_SURF |
                             IGD_OVL_TNC_UPDATE_SRC  ) ) {
		EMGD_WRITE32(ovl_cache.ovl2_regs.linear_offset,
			   MMIO(display) + MMIO_OFFSET_TNC2_LINEAR_OFF);
		EMGD_WRITE32(ovl_cache.ovl2_regs.pitch,
			   MMIO(display) + MMIO_OFFSET_TNC2_PITCH);
		EMGD_WRITE32(ovl_cache.ovl2_regs.tiled_offset,
			   MMIO(display) + MMIO_OFFSET_TNC2_DSPC_TILE_OFF);
	}


	/* write the quality information */
    if (cache_changed & (IGD_OVL_TNC_UPDATE_VQ   |
                             IGD_OVL_TNC_UPDATE_SURF ) ) {
		EMGD_WRITE32(ovl_cache.ovl2_regs.cont_bright,
			MMIO(display) + MMIO_OFFSET_TNC2_CONT_BRIGHT);
		EMGD_WRITE32(ovl_cache.ovl2_regs.satn_hue,
			MMIO(display) + MMIO_OFFSET_TNC2_SATN_HUE);
	}

	/* Write the gamma */
    if (cache_changed & IGD_OVL_TNC_UPDATE_GAMMA) {
		for (i = 0; i < OVL2_TOTAL_GAMMA_REG; i++) {
			/* program register */
			EMGD_WRITE32(ovl_cache.ovl2_regs.gamma_regs[i],
			   MMIO(display) + gamma_reg_offset[i]);
		}
	}

	/* Write the colorkey data */
    if (cache_changed & IGD_OVL_TNC_UPDATE_COLORKEY) {

		/* Dest color key */
		if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {

			/* Write the regs needed to turn it on */
			EMGD_WRITE32(ovl_cache.ovl2_regs.colorkey_hw,
				   MMIO(display) +
				   PLANE(display)->plane_reg +
				   MMIO_OFFSET_TNC2_COLORKEY_HW);

			EMGD_WRITE32(ovl_cache.ovl2_regs.colorkey_mask,
				   MMIO(display) +
				   PLANE(display)->plane_reg +
				   MMIO_OFFSET_TNC2_COLORKEY_MASK);
		}

		EMGD_WRITE32(ovl_cache.ovl2_regs.plane_control,
			   MMIO(display) + PLANE(display)->plane_reg);

		EMGD_WRITE32(ovl_cache.ovl2_regs.plane_start,
			   MMIO(display) + PLANE(display)->plane_reg + 0x1c);

		/*
		 * Note: On a 915GM (maybe other platforms as well),
		 * the dest color key is not always enabled when the
		 * plane A/B and plane C are both modified in the
		 * same vblank.  So, ensure they occur on a different
		 * vblank.  This should be fine, since this will only
		 * occur the first time when enabling the dest color
		 * key.
		 */
		if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {
			display->context->dispatch.
				wait_vblank((igd_display_h)display);
		}

		/* Source Color key */
		if (ovl_info->color_key.flags & IGD_OVL_SRC_COLOR_KEY_ENABLE) {
			EMGD_WRITE32(ovl_cache.ovl2_regs.ckey_low,
				   MMIO(display) + MMIO_OFFSET_TNC2_CKEY_LOW);
			EMGD_WRITE32(ovl_cache.ovl2_regs.ckey_high,
				   MMIO(display) + MMIO_OFFSET_TNC2_CKEY_HIGH);
		}
		EMGD_WRITE32(ovl_cache.ovl2_regs.ckey_enable,
			   MMIO(display) + MMIO_OFFSET_TNC2_CKEY_ENABLE);
	}

	/* Write the control register, but not the start register.
	   The trigger register is the start register
	   which causes the overlay to update.  The trigger
	   register is written in send_instr */

	EMGD_WRITE32(ovl_cache.ovl2_regs.control,
		   MMIO(display) + MMIO_OFFSET_TNC2_CONTROL);

	spritec_regs_tnc->start = ovl_cache.ovl2_regs.start;
	spritec_regs_tnc->control = ovl_cache.ovl2_regs.control;

}





/*----------------------------------------------------------------------
 * Function: micro_spritec_update_regs_tnc()
 *
 * Description:
 * Examine the incoming overlay parameters, and update the overlay hardware
 * regs according to what changed.
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/

static unsigned int micro_spritec_update_regs_tnc(
	igd_display_context_t *display,
	ovl2_reg_tnc_t      *spritec_regs_tnc,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	// unsigned int ckey_low, ckey_high;
	// int i,
	int ret;
	int cache_changed;


	EMGD_TRACE_ENTER;

	/* Fast path for turning off overlay. No need for cache */
	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {
		ret = micro_spritec_disable_ovl_tnc(display);

		/* Reset the cache */
		ovl_cache_needs_init = TRUE;

		EMGD_TRACE_EXIT;
		return ret;
	}

	/* Init the cache if needed */
	if ((ovl_cache_needs_init) || (ovl_context->ovl_display_swapped)){
		micro_spritec_clear_cache_tnc(display, flags);
		ovl_context->ovl_display_swapped = 0;
	}

	/* See what has changed in the cache */
	cache_changed = get_cache_changes_tnc (
		src_surf,
		src_rect,
		dest_rect,
		ovl_info,
		flags,
		&ovl_cache);

	/*
	 * Perhaps the biggest challenge of caching the overlay
	 * state is what to do with the command and config regs.
	 * Normally we would clear command and config to 0 here,
	 * and let the update process set only the bits that are
	 * needed.  But doing this would invalidate our cache.
	 * Instead we are relying on the above call to
	 * get_cache_changes() to clear those bits in command
	 * and config that will be changing
	 */


	/* Normally we would set interleave parameters here,
	 * but the secondary overlay does not support interleave.*/
	if (flags & IGD_OVL_ALTER_INTERLEAVED) {
		EMGD_ERROR("Overlay2 does not support Interleaved");
	}

	/* ----------------------------------------------------------*/
	/* Has our destination rectangle changed? */
	if (cache_changed & IGD_OVL_TNC_UPDATE_DEST) {
		ovl_cache.ovl2_regs.dest_rect_x1y1 =
			( (dest_rect->y1 << 16) | dest_rect->x1 );
		ovl_cache.ovl2_regs.dest_rect_w_h =
			( (dest_rect->y2 - dest_rect->y1 - 1) << 16) |
			  (dest_rect->x2 - dest_rect->x1 - 1)        ;
	}

	/* ----------------------------------------------------------*/
	/* Always update the source pointers every frame. */
	ret = micro_spritec_update_src_ptr_tnc(src_surf,
					   src_rect);
	if (ret) {
		EMGD_ERROR_EXIT("Overlay2 updating src failed");
		EMGD_TRACE_EXIT;
		return ret;
	}


	/* ----------------------------------------------------------*/
	/* Did either the Src rect or surface change? */
    if (cache_changed & (IGD_OVL_TNC_UPDATE_SURF |
                             IGD_OVL_TNC_UPDATE_SRC  ) ) {
		ret = micro_spritec_update_src_tnc(display,
						   src_surf,
						   src_rect);
		if (ret) {
			EMGD_ERROR_EXIT("Overlay2 updating src failed");
			EMGD_TRACE_EXIT;
			return ret;
		}
	}

	/* ----------------------------------------------------------*/
	/* Did the quality information change? */
    if (cache_changed & (IGD_OVL_TNC_UPDATE_VQ   |
                             IGD_OVL_TNC_UPDATE_SURF ) ) {
		ret = micro_spritec_update_video_quality_tnc(display,
			src_surf, &ovl_info->video_quality);
		if (ret) {
			EMGD_ERROR_EXIT("Overlay2 video quality failed");
			EMGD_TRACE_EXIT;
			return ret;
		}
	}

	/* ----------------------------------------------------------*/
	/* Did the gamma change? */
    if (cache_changed & IGD_OVL_TNC_UPDATE_GAMMA) {
		ret = micro_spritec_update_gamma_tnc(display, &ovl_info->gamma);
		if (ret) {
			EMGD_ERROR("Overlay2 gamma failed");
			EMGD_TRACE_EXIT;
			return ret;
		}

	}

	/* ----------------------------------------------------------*/
	/* Did the color key change? */
    if (cache_changed & IGD_OVL_TNC_UPDATE_COLORKEY) {
		micro_spritec_update_colorkey_tnc(display,
						  src_surf,
						  ovl_info);
	}


	if ((ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) ||
		(ovl_info->color_key.flags & IGD_OVL_DST_BLEND_ENABLE)) {
		if (flags & IGD_OVL_OSD_ON_SPRITEC) {
			ovl_cache.ovl2_regs.control |= 1;
		} else {
			/* Sprite C should be on the bottom of the Z order.
			 * Plane B should be above Sprite C */
			ovl_cache.ovl2_regs.control |= 4;
		}
	}

	/* General overlay information.  Turn the second overlay on. */
	ovl_cache.ovl2_regs.control |= (1<<31);
	if (PIPE(display)->pipe_num == 1) {
		ovl_cache.ovl2_regs.control |= (1<<24);/*Pipe B*/
	} else {
		ovl_cache.ovl2_regs.control &= ~(1<<24);/*Pipe A*/
	}

	/*
	 * Now write all the changes to the part
	 */
	micro_spritec_write_cache_tnc(display,
				      spritec_regs_tnc,
				      ovl_info,
				      cache_changed);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

#endif

unsigned int micro_spritec_send_instr_tnc(
	igd_display_context_t     *display,
	unsigned long regs,
	unsigned long value)
{
	EMGD_TRACE_ENTER;

	/* Send a load register instruction to write the Plane C sprite address
	 * which is the trigger register.
	 * This is an instruction, so it happens after blend, and since it
	 * is an instruction, we do not have to poll waiting for it. */

	EMGD_WRITE32(value, MMIO(display) + regs);

	ovl_context->sync2 = 0;

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

int micro_prepare_ovl2_tnc(
	igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	ovl2_reg_tnc_t      *spritec_regs_tnc,
	unsigned int         flags)
{
	int ret = 0;

	EMGD_TRACE_ENTER;

	/* Check to ensure the overlay can be used given the current mode as
	 * well as what the IAL is asking for.  If not return an error. */

	if( (OVL2_CHECK_TNC_RET(ret,display, src_surf, src_rect,
										dest_rect, ovl_info,flags)) )
	{
		if (ret) {
			printk(KERN_ERR "Overlay2 Check failed\n");
			return ret;
		}
	}

	/* Check if last flip is still pending.
	 * This is necessary for the following reasons:
	 *    - If the previous instructions have not been processed, then the
	 *      spritec_regs_tnc is still in use and can not be overwritten.
	 */
	if( (QUERY_OVL2_TNC_RET(ret,(igd_display_h)display,
								IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE)) )
	{
		//printk(KERN_ERR "QEURY_OVL2_TNC_RET failedi\n");
		if ((FALSE == ret) &&
			(flags & IGD_OVL_ALTER_ON)) {
			/* Only return an error if the overlay is on.  If turning it off,
			* allow it to continue, since something may have failed and we
			* should try our best to turn the overlay off. */
			return -IGD_ERROR_HWERROR;
		}
	}
	/* Update all Overlay Update Registers */
	ret = micro_spritec_update_regs_tnc(display, spritec_regs_tnc,
		src_surf, src_rect, dest_rect, ovl_info,
		flags);
	if (ret) {
		printk(KERN_ERR "Sprite C update Registers failed");
		return ret;
	}

	EMGD_TRACE_EXIT;
	return ret;
}

int micro_alter_ovl2_tnc(igd_display_context_t *display,
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

	/* Directlt write the register to update 2nd overlay */
#ifdef OVL_TNC_CACHE_QUICK_SWAP
	ret = micro_spritec_send_instr_tnc(display,
		MMIO_OFFSET_TNC2_SURF_ADDR,
		ovl_cache.ovl2_regs.start);
#else
	ret = micro_spritec_send_instr_tnc(display,
		MMIO_OFFSET_TNC2_SURF_ADDR,
		spritec_regs_tnc.start);
#endif
	EMGD_TRACE_EXIT;
	return ret;
}


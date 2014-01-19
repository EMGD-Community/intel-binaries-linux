/*
 *-----------------------------------------------------------------------------
 * Filename: ovl_plb.c
 * $Revision: 1.30 $
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
 *  This file contains function that actually programs the overlay
 *  register back buffer with the bits to properly configure
  *  the overlay
 *  Also includes functions to execute the overlay flip instruction,
 *  and query the overlay flip status.
 *  Also contains some hardware capabilities querrying functions
 *  for upper overlay layer to get this chips overlay capabilities
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.overlay

#include <math_fix.h>
#include <plb/cmd.h>
#include "ovl_plb_cache.h"
#include "../cmn/ovl_dispatch.h"
#include "../cmn/ovl_virt.h"
#include "../cmn/ovl_coeff.h"
#include "ovl2_plb.h"

/*-----------------------------------------------------------------------------
 * Common dispatch functions
 *---------------------------------------------------------------------------*/
static int alter_ovl_plb(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags);
static int query_ovl_plb(igd_display_h display_h,
	unsigned int flags);
static int query_max_size_ovl_plb(igd_display_h display_h,
	unsigned long pf,
	unsigned int *max_width,
	unsigned int *max_height);


ovl_dispatch_t ovl_dispatch_plb[] = {
	/* Dispatch for the hardware overlay */
	{
		NULL, /*blend_surf_needed_plb,*/
		alter_ovl_plb,
		query_ovl_plb,
		query_max_size_ovl_plb,
	},
	/* Dispatch for the software overlay */
	{
		NULL, /*blend2_surf_needed_plb,*/
		alter_ovl2_plb,
		query_ovl2_plb,
		query_max_size_ovl2_plb,
	},
};


typedef struct _ovl_chipset_plb {
	unsigned int	num_linebuf;
	unsigned int	pixel_format;
	unsigned int	max_width;
	unsigned int	max_height;
} ovl_chipset_plb_t;

static ovl_chipset_plb_t ovl_chipset_plb[] = {
	{OVL_CONFIG_THREE_LINE_BUFF,
		(PF_DEPTH_16 | PF_TYPE_YUV_PACKED), 1280, 1080},
	{OVL_CONFIG_THREE_LINE_BUFF,
		(PF_DEPTH_8 | PF_TYPE_YUV_PLANAR), 1024, 1080},
	{OVL_CONFIG_THREE_LINE_BUFF,
		(PF_DEPTH_32 | PF_TYPE_ARGB), 640, 1080},
	{OVL_CONFIG_THREE_LINE_BUFF,
		(PF_DEPTH_32 | PF_TYPE_RGB), 640, 1080},

	{OVL_CONFIG_TWO_LINE_BUFF,
		(PF_DEPTH_16 | PF_TYPE_YUV_PACKED), 1920, 1088},
	{OVL_CONFIG_TWO_LINE_BUFF,
		(PF_DEPTH_8 | PF_TYPE_YUV_PLANAR), 1920, 1088},
	{OVL_CONFIG_TWO_LINE_BUFF,
		(PF_DEPTH_32 | PF_TYPE_ARGB), 960, 1080},
	{OVL_CONFIG_TWO_LINE_BUFF,
		(PF_DEPTH_32 | PF_TYPE_RGB), 960, 1080},
	{OVL_CONFIG_NO_LINE_BUFF, 0, 0, 0}
};

#ifdef DEBUG_BUILD_TYPE

static void ovl_dump_regs_plb(
	ovl_reg_image_plb_t *ovl_regs_plb)
{
	return;

	printk (KERN_ERR "************************************************\n");
	printk (KERN_ERR "OVERLAY REGISTER LISTING\n");

	printk (KERN_ERR "OVL REGS AT  =    0x%x  \n", (unsigned int)ovl_regs_plb);
	if(ovl_regs_plb->buffer0_yrgb_ptr)
		printk (KERN_ERR "RGB POINTER 0    =     0x%x  \n",
			ovl_regs_plb->buffer0_yrgb_ptr);
	if(ovl_regs_plb->buffer1_yrgb_ptr)
		printk (KERN_ERR "RGB POINTER 1    =     0x%x  \n",
			ovl_regs_plb->buffer1_yrgb_ptr);
	if(ovl_regs_plb->buffer0_u_ptr)
		printk (KERN_ERR "U POINTER 0      =     0x%x  \n",
			ovl_regs_plb->buffer0_u_ptr);
	if(ovl_regs_plb->buffer0_v_ptr)
		printk (KERN_ERR "V POINTER 0      =     0x%x  \n",
			ovl_regs_plb->buffer0_v_ptr);
	if(ovl_regs_plb->buffer1_u_ptr)
		printk (KERN_ERR "U POINTER 1      =     0x%x  \n",
			ovl_regs_plb->buffer1_u_ptr);
	if(ovl_regs_plb->buffer1_v_ptr)
		printk (KERN_ERR "V POINTER 1      =     0x%x  \n",
			ovl_regs_plb->buffer1_v_ptr);

	printk (KERN_ERR "RGB STRIDE       =     0x%x  \n",
		ovl_regs_plb->yrgb_stride);
	printk (KERN_ERR "UV STRIDE        =     0x%x  \n",
		ovl_regs_plb->uv_stride);
	printk (KERN_ERR "DST POS X        =     %d  \n",
		ovl_regs_plb->dest_pos_x_left);
	printk (KERN_ERR "DST POS Y        =     %d  \n",
		ovl_regs_plb->dest_pos_y_top);
	printk (KERN_ERR "DST WIDTH        =     %d  \n",
		ovl_regs_plb->dest_width_x);
	printk (KERN_ERR "DST HEIGHT       =     %d  \n",
		ovl_regs_plb->dest_height_y);
	printk (KERN_ERR "SRC WIDTH        =     %d  \n",
		ovl_regs_plb->source_yrgb_width);
	printk (KERN_ERR "SRC SWWIDTH      =     0x%x  \n",
		ovl_regs_plb->source_yrgb_width_swords);
	printk (KERN_ERR "SRC HEIGHT       =     %d  \n",
		ovl_regs_plb->source_yrgb_height);
	printk (KERN_ERR "UV SRC WIDTH     =     %d  \n",
		ovl_regs_plb->source_uv_width);
	printk (KERN_ERR "UV SRC SWWIDTH   =     %d  \n",
		ovl_regs_plb->source_uv_width_swords);
	printk (KERN_ERR "UV SRC HEIGHT    =     %d  \n",
		ovl_regs_plb->source_uv_height);
	printk (KERN_ERR "RGB SCALE        =     0x%x  \n",
		ovl_regs_plb->yrgb_scale);
	printk (KERN_ERR "UV SCALE         =     0x%x  \n",
		ovl_regs_plb->uv_scale);
	printk (KERN_ERR "COL CTL BRT CON  =     0x%x  \n",
		ovl_regs_plb->col_ctl_brt_con);
	printk (KERN_ERR "COL CTL SAT HUE  =     0x%x  \n",
		ovl_regs_plb->col_ctl_sat_hue);
	printk (KERN_ERR "DST COLOR KEY    =     0x%x  \n",
		ovl_regs_plb->dest_ckey_val);
	printk (KERN_ERR "DST COLOR KEY MASK =   0x%x  \n",
		ovl_regs_plb->dest_ckey_mask);
	printk (KERN_ERR "SRC COLOR KEY HI =     0x%x  \n",
		ovl_regs_plb->source_ckey_high);
	printk (KERN_ERR "SRC COLOR KEY LO =     0x%x  \n",
		ovl_regs_plb->source_ckey_low);
	printk (KERN_ERR "SRC COLOR KEY MASK =   0x%x  \n",
		ovl_regs_plb->source_ckey_mask);
	printk (KERN_ERR "OVL CONFIG       =     0x%x  \n", ovl_regs_plb->config);
	printk (KERN_ERR "OVL CMD          =     0x%x  \n", ovl_regs_plb->command);
	printk (KERN_ERR "FAST_V_DSCALE    =     0x%x  \n",
		ovl_regs_plb->vert_downscale);
	printk (KERN_ERR "************************************************\n");
}
#endif

/*----------------------------------------------------------------------
 * Function: ovl_check_pf_plb()
 * Parameters: unsigned int requested_pixel_format -
 *             according to definitions in igd_mode.h
 *
 * Description:
 *
 * Returns:
 *   TRUE on Success
 *   FALSE on The first pixel format that is supported
 *----------------------------------------------------------------------*/
static unsigned int ovl_check_pf_plb(
	igd_display_context_t *display,
	unsigned int requested_pixel_format)
{
	unsigned long *overlay_pfs;
	int temp_loop = 0;

	display->context->dispatch.get_pixelformats(
		(igd_display_h)display, NULL, NULL, &overlay_pfs, NULL, NULL);

	while(overlay_pfs[temp_loop]) {
		if(overlay_pfs[temp_loop] == requested_pixel_format) {
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

static unsigned int ovl_check_plb(igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	igd_timing_info_t *timing;
	ovl_chipset_plb_t *ovl_chip;
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

	/* DCT-PC99TA crashes with dotclock > 300MHz */
	if(timing->dclk >= 340000){
		EMGD_ERROR_EXIT("Cannot support dotclock > 340MHz for this SKU");
		return -IGD_ERROR_HWERROR;
	}

	/* The following parameters are only valid if the overlay is on, so
	 * return success if the overlay is being turned off. */
	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/*************************************************************************
	 * Ensure the overlay surface is ok and can be properly displayed.
	 * This ensures the following is valid:
	 *    - Ensure x1, x2, y1, y2 are pixel aligned
	 *    - 2 pixels or greater in width and height
	 *    - Pixel format is supported by the overlay
	 *    - Pitch is <= 8KB
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
			"Overlay source width or height is < 2 pixels (%dx%d)\n",
			src_rect->x2 - src_rect->x1, src_rect->y2 - src_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	if (FALSE == ovl_check_pf_plb(display, src_surf->pixel_format)) {
		EMGD_ERROR_EXIT("Overlay source pixel format unsupported (pf:0x%lx)",
			src_surf->pixel_format);
		return -IGD_ERROR_HWERROR;
	}

	if (src_surf->pitch > 8192) {
		EMGD_ERROR_EXIT("Overlay source pitch (%d) > 8KB",
			src_surf->pitch);
		return -IGD_ERROR_HWERROR;
	}

	ovl_chip = ovl_chipset_plb;
	while(ovl_chip->num_linebuf != OVL_CONFIG_NO_LINE_BUFF){
		if(((src_surf->pixel_format & IGD_PF_MASK) ==
				ovl_chip->pixel_format) &&
			(src_surf->width <= ovl_chip->max_width)) {
			break;
		}
		ovl_chip++;
	}
	if (ovl_chip->num_linebuf == OVL_CONFIG_NO_LINE_BUFF) {
		EMGD_ERROR_EXIT("Overlay source width (%d) > max supported",
			src_surf->width);
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
			"Overlay dest width or height is single pixel (%dx%d)\n",
			dest_rect->x2 - dest_rect->x1, dest_rect->y2 - dest_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	if ((dest_rect->x1 >= timing->width) ||
		(dest_rect->y1 >= timing->height)) {
		EMGD_ERROR_EXIT(
			"Overlay dest is panned off the screen (%d,%d)\n",
			dest_rect->x1, dest_rect->y1);
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}



static unsigned int ovl_update_src_plb(igd_display_context_t *display,
	ovl_reg_image_plb_t *ovl_regs_plb,
	igd_surface_t *src_surf,
	igd_rect_t    *src_rect)
{
	ovl_chipset_plb_t *ovl_chip;
	unsigned int      src_Bpp;
	unsigned int      src_uv_shift_x, src_uv_shift_y;
	unsigned short    src_w;
	unsigned short    src_h;

	EMGD_TRACE_ENTER;

	/* This is in Bytes per pixel */
	src_Bpp = IGD_PF_BPP(src_surf->pixel_format)/8;

	src_uv_shift_x = get_uv_shift_x(src_surf->pixel_format);
	src_uv_shift_y = get_uv_shift_y(src_surf->pixel_format);

	src_w = src_rect->x2 - src_rect->x1;
	src_h = src_rect->y2 - src_rect->y1;

	ovl_regs_plb->yrgb_stride      = (unsigned short)src_surf->pitch;
	ovl_regs_plb->uv_stride        = (unsigned short)src_surf->u_pitch;

	/* src width */
	ovl_regs_plb->source_yrgb_width      = src_w;
	ovl_regs_plb->source_uv_width        = src_w >> src_uv_shift_x;

	/* src width swords - This equation follows the B-Spec */
	ovl_regs_plb->source_yrgb_width_swords =
		((((((ovl_regs_plb->buffer0_yrgb_ptr +
		      (ovl_regs_plb->source_yrgb_width * src_Bpp) +
		      0x3F) >> 6) -
		    (ovl_regs_plb->buffer0_yrgb_ptr >> 6)) << 1) - 1) << 2);
	ovl_regs_plb->source_uv_width_swords   =
		((((((ovl_regs_plb->buffer0_u_ptr +
		      (ovl_regs_plb->source_uv_width * src_Bpp) +
		      0x3F) >> 6) -
		    (ovl_regs_plb->buffer0_u_ptr >> 6)) << 1) - 1) << 2 );


	/* src height */
	ovl_regs_plb->source_yrgb_height     = src_h;
	ovl_regs_plb->source_uv_height       = src_h >> src_uv_shift_y;

	/* src pixel format */
	switch(src_surf->pixel_format){
	case IGD_PF_YUV422_PACKED_YUY2:
		ovl_regs_plb->command |= OVL_CMD_YUV_422;
		break;
	case IGD_PF_YUV422_PACKED_UYVY:
		ovl_regs_plb->command |= OVL_CMD_YUV_422 | OVL_CMD_Y_SWAP;
		break;
	case IGD_PF_YUV420_PLANAR_I420: /* same as IYUV */
		ovl_regs_plb->command |= OVL_CMD_YUV_420P;
		break;
	case IGD_PF_YUV420_PLANAR_YV12:
		ovl_regs_plb->command |= OVL_CMD_YUV_420P | OVL_CMD_UV_SWAP;
		break;
	case IGD_PF_YUV420_PLANAR_NV12:
		ovl_regs_plb->command |= OVL_CMD_YUV_NV12;
		break;
	case IGD_PF_YUV410_PLANAR_YVU9:
		ovl_regs_plb->command |= OVL_CMD_YUV_410P;
		break;
	case IGD_PF_ARGB32_8888:
	case IGD_PF_xRGB32_8888:
		ovl_regs_plb->command |= OVL_CMD_RGB_8888;
		break;
	case IGD_PF_RGB16_565:
		ovl_regs_plb->command |= OVL_CMD_RGB_565;
		break;
	case IGD_PF_xRGB16_555:
	case IGD_PF_ARGB16_1555:
		ovl_regs_plb->command |= OVL_CMD_RGB_555;
		break;
	default:
		EMGD_ERROR_EXIT("Invalid pixel format: 0x%lx", src_surf->pixel_format);
		return -IGD_ERROR_HWERROR;
	}

	/* Turn off YUV to RGB conversion if the src is RGB */
	if (!(src_surf->pixel_format & PF_TYPE_YUV)) {
		ovl_regs_plb->config |= (1<<4);
	}

	ovl_chip = ovl_chipset_plb;
	ovl_regs_plb->config &= ~OVL_CONFIG_LINE_BUFF_MASK;
	while(ovl_chip->num_linebuf != OVL_CONFIG_NO_LINE_BUFF){
		if(((src_surf->pixel_format & IGD_PF_MASK) ==
				ovl_chip->pixel_format) &&
			(src_w <= ovl_chip->max_width)) {
			ovl_regs_plb->config |= ovl_chip->num_linebuf;
			break;
		}
		ovl_chip++;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


static unsigned int ovl_update_src_ptr_plb(igd_display_context_t *display,
	ovl_reg_image_plb_t *ovl_regs_plb,
        igd_surface_t *src_surf,
	igd_rect_t    *src_rect)
{
	unsigned int      src_Bpp;
	unsigned int      src_uv_shift_x, src_uv_shift_y;

	EMGD_TRACE_ENTER;

	/* This is in Bytes per pixel */
	src_Bpp = IGD_PF_BPP(src_surf->pixel_format)/8;
	src_uv_shift_x = get_uv_shift_x(src_surf->pixel_format);
	src_uv_shift_y = get_uv_shift_y(src_surf->pixel_format);

	/* src surface */
	ovl_regs_plb->buffer0_yrgb_ptr     =
		ovl_regs_plb->buffer1_yrgb_ptr =
		src_surf->offset +
		(src_rect->y1 * src_surf->pitch) + (src_rect->x1 * src_Bpp);

	/*
	 * The NV12 format has the UV pixels interleaved so the total
	 * width of the UV portion of the surface is the same as the
	 * Y width. Thus, don't do any shifting of the UV plane in the
	 * X direction.
	 */
	if (src_surf->pixel_format == IGD_PF_YUV420_PLANAR_NV12) {
		ovl_regs_plb->buffer0_u_ptr     =
			ovl_regs_plb->buffer1_u_ptr =
			src_surf->u_offset +
			((src_rect->y1>>src_uv_shift_y) * src_surf->u_pitch) +
			src_rect->x1;

		ovl_regs_plb->buffer0_v_ptr     =
			ovl_regs_plb->buffer1_v_ptr =
			src_surf->v_offset +
			((src_rect->y1>>src_uv_shift_y) * src_surf->v_pitch) +
			src_rect->x1;
	} else {
		ovl_regs_plb->buffer0_u_ptr     =
			ovl_regs_plb->buffer1_u_ptr =
			src_surf->u_offset +
			((src_rect->y1>>src_uv_shift_y) * src_surf->u_pitch) +
			(src_rect->x1>>src_uv_shift_x);

		ovl_regs_plb->buffer0_v_ptr     =
			ovl_regs_plb->buffer1_v_ptr =
			src_surf->v_offset +
			((src_rect->y1>>src_uv_shift_y) * src_surf->v_pitch) +
			(src_rect->x1>>src_uv_shift_x);
	}


	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


static unsigned int ovl_update_phase_plb(
	ovl_reg_image_plb_t *ovl_regs_plb,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect)
{

	EMGD_TRACE_ENTER;

	/*
	 * Set the Vertical/Horizontal Phase Registers.  Both Field0
	 * and Field1 are set, although Field1 is only used when
	 * interleaved.
	 */
	switch (src_surf->pixel_format) {
	case IGD_PF_YUV422_PACKED_YUY2:
	case IGD_PF_YUV422_PACKED_UYVY:
		/* YUV 422 */
		ovl_regs_plb->init_phase_shift = 0;

		/* Vertical Phase */
		if (ovl_regs_plb->config & OVL_CONFIG_THREE_LINE_BUFF) {
			ovl_regs_plb->yrgb_vert_phase_field0 = 0;
			ovl_regs_plb->yrgb_vert_phase_field1 = 0;
			ovl_regs_plb->uv_vert_phase_field0 = 0;
			ovl_regs_plb->uv_vert_phase_field1 = 0;
		} else {
			ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000; /*.5*/
			ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000; /*.5*/
			ovl_regs_plb->uv_vert_phase_field0   = 0x8000; /*.5*/
			ovl_regs_plb->uv_vert_phase_field1   = 0x8000; /*.5*/
		}

		/* Horizontal Phase */
		if (!(src_rect->x1 & 1)) {
			ovl_regs_plb->yrgb_hphase = 0;
			ovl_regs_plb->uv_hphase = 0;
		} else {
			ovl_regs_plb->init_phase_shift |= Y_HPP_PLUS1;
			ovl_regs_plb->yrgb_hphase = 0; /*1*/
			ovl_regs_plb->uv_hphase = 0x8000; /*.5*/
		}
		break;

	case IGD_PF_YUV420_PLANAR_I420:
	case IGD_PF_YUV420_PLANAR_YV12:
	case IGD_PF_YUV420_PLANAR_NV12:
		/* YUV 420 */
		ovl_regs_plb->init_phase_shift = 0;

		/* Vertical Phase */
		if (ovl_regs_plb->config & OVL_CONFIG_THREE_LINE_BUFF) {
			if (!(src_rect->y1 & 1)) {
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*0*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*1*/

				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD0_MINUS1;
				ovl_regs_plb->uv_vert_phase_field0 = 0xc000; /*-.25*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x4000; /*.25*/
			} else {
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*1*/
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*0*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x4000; /*.25*/
				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD1_MINUS1;
				ovl_regs_plb->uv_vert_phase_field1 = 0xc000; /*-.25*/
			}
		} else {
			if (!(src_rect->y1 & 1)) {
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000; /*.5*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000;/*1.5*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x4000; /*.25*/
				ovl_regs_plb->uv_vert_phase_field1 = 0xc000; /*.75*/
			} else {
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000;/*1.5*/
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000; /*.5*/

				ovl_regs_plb->uv_vert_phase_field0 = 0xc000; /*.75*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x4000; /*.25*/
			}
		}

		/* Horizontal Phase */
		if (!(src_rect->x1 & 1)) {
			ovl_regs_plb->yrgb_hphase = 0;
			ovl_regs_plb->uv_hphase = 0;
		} else {
			ovl_regs_plb->init_phase_shift |= Y_HPP_PLUS1;
			ovl_regs_plb->yrgb_hphase = 0; /*1*/
			ovl_regs_plb->uv_hphase = 0x8000; /*.5*/
		}
		break;

	case IGD_PF_YUV410_PLANAR_YVU9:
		/* YUV 410 */
		ovl_regs_plb->init_phase_shift = 0;

		/* Vertical Phase */
		if (ovl_regs_plb->config & OVL_CONFIG_THREE_LINE_BUFF) {
			switch (src_rect->y1 & 3) {
			default:
			case 0:
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*0*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*1*/

				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD0_MINUS1;
				ovl_regs_plb->uv_vert_phase_field0 = 0xa000; /*-.375*/
				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD1_MINUS1;
				ovl_regs_plb->uv_vert_phase_field1 = 0xe000; /*-.125*/
				break;

			case 1:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*1*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*1*/

				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD0_MINUS1;
				ovl_regs_plb->uv_vert_phase_field0 = 0xe000; /*-.125*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x2000; /*.125*/
				break;

			case 2:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*1*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*1*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x2000; /*.125*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x6000; /*.375*/
				break;

			case 3:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0; /*1*/
				ovl_regs_plb->yrgb_vert_phase_field1 = 0;/*0*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x6000; /*.375*/
				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD1_MINUS1;
				ovl_regs_plb->uv_vert_phase_field1 = 0xa000; /*-.375*/
				break;
			}
		} else {
			switch (src_rect->y1 & 3) {
			default:
			case 0:
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000; /*.5*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000;/*1.5*/

				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD0_MINUS1;
				ovl_regs_plb->uv_vert_phase_field0 = 0xc000; /*-.25*/
				ovl_regs_plb->uv_vert_phase_field1 = 0; /*0*/
				break;

			case 1:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000;/*1.5*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000; /*1.5*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x0; /*0*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x4000; /*.25*/
				break;

			case 2:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000;/*1.5*/
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD1_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000; /*1.5*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x4000; /*.25*/
				ovl_regs_plb->uv_vert_phase_field1 = 0x8000; /*.5*/
				break;

			case 3:
				ovl_regs_plb->init_phase_shift |= Y_VPP_FLD0_PLUS1;
				ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000;/*1.5*/
				ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000; /*.5*/

				ovl_regs_plb->uv_vert_phase_field0 = 0x8000; /*.5*/
				ovl_regs_plb->init_phase_shift |= UV_VPP_FLD1_MINUS1;
				ovl_regs_plb->uv_vert_phase_field1 = 0xc000; /*-.25*/
				break;
			}
		}

		/* Horizontal Phase */
		switch (src_rect->x1 & 3) {
		default:
		case 0:
			ovl_regs_plb->yrgb_hphase = 0;
			ovl_regs_plb->init_phase_shift |= UV_HPP_MINUS1;
			ovl_regs_plb->uv_hphase = 0xa000; /*-.375*/
			break;

		case 1:
			ovl_regs_plb->init_phase_shift |= Y_HPP_PLUS1;
			ovl_regs_plb->yrgb_hphase = 0; /*1*/
			ovl_regs_plb->init_phase_shift |= UV_HPP_MINUS1;
			ovl_regs_plb->uv_hphase = 0xe000; /*-.125*/
			break;

		case 2:
			ovl_regs_plb->init_phase_shift |= Y_HPP_PLUS2;
			ovl_regs_plb->yrgb_hphase = 0; /*2*/
			ovl_regs_plb->uv_hphase = 0x2000; /*.125*/
			break;

		case 3:
			ovl_regs_plb->init_phase_shift |= Y_HPP_PLUS2;
			ovl_regs_plb->yrgb_hphase = 0xffff; /*3*/
			ovl_regs_plb->uv_hphase = 0x6000; /*.375*/
			break;
		}
		break;

	default:
		/* RGB format */
		ovl_regs_plb->init_phase_shift = 0;

		/* Vertical Phase */
		if (ovl_regs_plb->config & OVL_CONFIG_THREE_LINE_BUFF) {
			ovl_regs_plb->yrgb_vert_phase_field0 = 0;
			ovl_regs_plb->yrgb_vert_phase_field1 = 0;
			ovl_regs_plb->uv_vert_phase_field0 = 0;
			ovl_regs_plb->uv_vert_phase_field1 = 0;
		} else {
			ovl_regs_plb->yrgb_vert_phase_field0 = 0x8000;
			ovl_regs_plb->yrgb_vert_phase_field1 = 0x8000;
			ovl_regs_plb->uv_vert_phase_field0   = 0x8000;
			ovl_regs_plb->uv_vert_phase_field1   = 0x8000;
		}

		/* Horizontal Phase */
		ovl_regs_plb->yrgb_hphase = 0;
		ovl_regs_plb->uv_hphase = 0;
		break;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}



/*----------------------------------------------------------------------
 * Function: ovl_update_scale_plb()
 *
 * Description: Will update only the scaling registers for the plb core
 *
 * Returns:
 *   N/A
 *----------------------------------------------------------------------*/

static unsigned int ovl_update_scale_plb(
	ovl_reg_image_plb_t *ovl_regs_plb,
	igd_surface_t *src_surf,
	igd_rect_t   *src_rect,
	igd_rect_t   *dest_rect,
	unsigned int flags)
{
	unsigned int uv_shift;
	unsigned int xscale, xscale_int, xscale_fract;
	unsigned int yscale, yscale_int, yscale_fract;
	unsigned int xscale_int_uv, xscale_fract_uv;
	unsigned int yscale_int_uv, yscale_fract_uv;

	EMGD_TRACE_ENTER;

	xscale = ((src_rect->x2 - src_rect->x1)<<12) /
		(dest_rect->x2 - dest_rect->x1);
	yscale = ((src_rect->y2 - src_rect->y1)<<12) /
		(dest_rect->y2 - dest_rect->y1);

	/* In interleaved mode, the y scale is /2 */
	if (flags & IGD_OVL_ALTER_INTERLEAVED) {
		yscale >>= 1;
	}

	xscale_int = (xscale & 0x3000) >> 12;
	xscale_fract = xscale & 0xfff;
	yscale_int = (yscale & 0x7ff000) >> 12;
	yscale_fract = yscale & 0xfff;

	uv_shift = get_uv_shift_x(src_surf->pixel_format);
	xscale_int_uv = ((xscale>>uv_shift) & 0x3000) >> 12;
	xscale_fract_uv = (xscale>>uv_shift) & 0xfff;

	uv_shift = get_uv_shift_y(src_surf->pixel_format);
	yscale_int_uv = ((yscale>>uv_shift) & 0x7ff000) >> 12;
	yscale_fract_uv = (yscale>>uv_shift) & 0xfff;

	ovl_regs_plb->yrgb_scale =
		(yscale_fract  << 20) |   /* Vert  Scale Fraction */
		(xscale_int    << 16) |   /* Horiz Scale Int */
		(xscale_fract  << 3);     /* Horiz Scale Fraction */

	ovl_regs_plb->uv_scale =
		(yscale_fract_uv << 20) | /* UV Vert  Scale Fraction */
		(xscale_int_uv   << 16) | /* UV Horiz Scale Int */
		(xscale_fract_uv << 3 );  /* UV Horiz Scale Fraction */
	ovl_regs_plb->vert_downscale =
		(yscale_int    << 16) |   /* Vert Scale Factor */
		yscale_int_uv;            /* UV Vert Scale Factor */

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}



/*----------------------------------------------------------------------
 * Function: ovl_update_video_quality_plb()
 *
 * Description:
 *   This function updates the contrast, brightness, and saturation of
 *   the overlay using the values specified in overlay_info.
 *
 * Returns:
 *   != 0 on Error
 *   0 on Success
 *----------------------------------------------------------------------*/

static int ovl_update_video_quality_plb(
	ovl_reg_image_plb_t *ovl_regs_plb,
	igd_surface_t       *src_surf,
	igd_ovl_video_quality_info_t *video_quality)
{
	int                          calc_brightness_tmp = 0;
	int                          calc_brightness     = 0;
	unsigned int                 calc_contrast_tmp   = 0;
	unsigned int                 calc_contrast       = 0;
	unsigned int                 calc_saturation_tmp = 0;
	unsigned int                 calc_saturation     = 0;

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

		ovl_regs_plb->col_ctl_brt_con = OVL_RGB_COLOR_DEF_CONT_BRGHT;
		ovl_regs_plb->col_ctl_sat_hue = OVL_RGB_COLOR_DEF_SATN_HUE;

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

	ovl_regs_plb->col_ctl_brt_con =
		(ovl_regs_plb->col_ctl_brt_con & 0xFFFFFF00) |
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

	ovl_regs_plb->col_ctl_brt_con =
		(ovl_regs_plb->col_ctl_brt_con & 0xF803FFFF) |
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

	ovl_regs_plb->col_ctl_sat_hue =
		(ovl_regs_plb->col_ctl_sat_hue & 0xFFFFFC00) |
		(calc_saturation & 0x3FF);

	/*************************************************************************
	 * Hue
	 *************************************************************************/
	/* Hue is always set to the default value.  It is based on the saturation
	 * value, and having a separate hue is of minimal value. */
	ovl_regs_plb->col_ctl_sat_hue =
		(ovl_regs_plb->col_ctl_sat_hue & 0xF800FFFF);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}



static void check_gamma(unsigned int *gamma)
{

	if (*gamma < IGD_OVL_GAMMA_MIN) {
		EMGD_ERROR("Gamma to small (0x%x in 24i.8f format), "
			"changing to Min Gamma (0.6)",
			*gamma);
		*gamma = IGD_OVL_GAMMA_MIN;
	}
	if (*gamma > IGD_OVL_GAMMA_MAX) {
		EMGD_ERROR("Gamma to large (0x%x in 24i.8f format), "
			"changing to Max Gamma (6.0)",
			*gamma);
		*gamma = IGD_OVL_GAMMA_MAX;
	}

	return;
}



/*-----------------------------------------------------------------------------
 * Function: ovl_update_gamma_plb()
 *
 * Description:
 *    This function sets the gamma correction values for the overlays.
 *
 * Returns:
 *   != 0 on Error
 *   IGD_SUCCESS on Success
 *---------------------------------------------------------------------------*/
static int ovl_update_gamma_plb(
	igd_display_context_t *display,
	igd_ovl_gamma_info_t * ovl_gamma)
{
	const int gamma_reg_input[OVL_TOTAL_GAMMA_REG] = {8, 16, 32, 64, 128, 192};
	const int gamma_reg_offset[OVL_TOTAL_GAMMA_REG] = {
		OVL_REG_ADDR_GAMMA0,
		OVL_REG_ADDR_GAMMA1,
		OVL_REG_ADDR_GAMMA2,
		OVL_REG_ADDR_GAMMA3,
		OVL_REG_ADDR_GAMMA4,
		OVL_REG_ADDR_GAMMA5
	};
	const unsigned int gamma_def[OVL_TOTAL_GAMMA_REG] = {
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

	/* If the overlay gamma is disabled, set it to the default */
	if ((ovl_gamma->flags & IGD_OVL_GAMMA_ENABLE) == IGD_OVL_GAMMA_DISABLE) {
		for (i = 0; i < OVL_TOTAL_GAMMA_REG; i++) {
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

	/* Ensure the gamma values are between MIN and MAX */
	check_gamma(&new_gamma_red_24i_8f);
	check_gamma(&new_gamma_green_24i_8f);
	check_gamma(&new_gamma_blue_24i_8f);

	/*
	 * Program RGB for each of the 6 gamma registers
	 */

	/* Since the OS_POW_FIX function can only take an integer base,
	 * we need to normalize the result by gamma_normal_x
	 */
	gamma_normal_r_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_red_24i_8f);
	gamma_normal_g_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_green_24i_8f);
	gamma_normal_b_24i_8f =  OS_POW_FIX(255, (1<<16)/new_gamma_blue_24i_8f);

	for( i = 0; i < OVL_TOTAL_GAMMA_REG; i++ )
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



/*----------------------------------------------------------------------
 * Function: ovl_update_coeff_plb()
 * Description: Function to calculate the filter coeffcient
 *              registers for plb
 * Notes in Usage:
 *
 *----------------------------------------------------------------------*/
static unsigned int ovl_update_coeff_plb(
	ovl_reg_image_plb_t *ovl_regs_plb,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	unsigned int        flags)
{
	unsigned int scale_int, scale_fpint;

	unsigned int dest_h = dest_rect->y2 - dest_rect->y1;
	unsigned int dest_w = dest_rect->x2 - dest_rect->x1;
	unsigned int src_h  = src_rect->y2  - src_rect->y1;
	unsigned int src_w  = src_rect->x2  - src_rect->x1;

	EMGD_TRACE_ENTER;

	/* FIXME: The coeff values are re-written for every alter_ovl call.
	 * This may cause issues or may be to slow?  If so, store the previous
	 * values and only re-write when they change. */

	/* In interleaved mode, the src_h is /2 */
	if (flags & IGD_OVL_ALTER_INTERLEAVED) {
		src_h >>= 1;
	}

	/* Y Horizontal */
	scale_int = ((ovl_regs_plb->yrgb_scale) >> 16) & 0x7;

	if (!scale_int) {
		/* upscale - clamp to 1.0 */
		scale_fpint = 1<<20;
	} else {
		scale_fpint = ((src_w << 20) / dest_w) / scale_int;
	}
    ovl_update_coeff_regs(5, scale_fpint, 1, 1,
                          (unsigned short *)ovl_regs_plb->y_horz_coeff_single);

	/* Y Vertical */
	scale_int = ((ovl_regs_plb->vert_downscale) >> 16) & 0x7ff;
	if (!scale_int) {
		/* upscale - clamp to 1.0 */
		scale_fpint = 1<<20;
	} else {
		scale_fpint = ((src_h << 20) / dest_h) / scale_int;
	}
	ovl_update_coeff_regs(3, scale_fpint, 0, 1,
                          (unsigned short *)ovl_regs_plb->y_vert_coeff_single);

	/* UV Horizontal */
	scale_int = ((ovl_regs_plb->uv_scale) >> 16) & 0x7;
	if (!scale_int) {
		/* upscale - clamp to 1.0 */
		scale_fpint = 1<<20;
	} else {
		scale_fpint = ((src_w << 20) / dest_w) / scale_int;
		scale_fpint >>= get_uv_shift_x(src_surf->pixel_format);
	}
	ovl_update_coeff_regs(3, scale_fpint , 1, 0,
                          (unsigned short *)ovl_regs_plb->uv_horz_coeff_single);

	/* UV Vertical */
	scale_int = (ovl_regs_plb->vert_downscale) & 0x7ff;
	if (!scale_int) {
		/* upscale - clamp to 1.0 */
		scale_fpint = 1<<20;
	} else {
		scale_fpint = ((src_h << 20) / dest_h) / scale_int;
		scale_fpint >>= get_uv_shift_y(src_surf->pixel_format);
	}
	ovl_update_coeff_regs(3, scale_fpint, 0, 0,
                          (unsigned short *)ovl_regs_plb->uv_vert_coeff_single);

	/* Adjust for 2-line Vertical Buffer */
	if((ovl_regs_plb->config & OVL_CONFIG_LINE_BUFF_MASK)==
		OVL_CONFIG_TWO_LINE_BUFF){
		ovl_update_coeff_regs(2, 0x10, 0, 1,
			(unsigned short *)ovl_regs_plb->y_vert_coeff_single);
		ovl_update_coeff_regs(2, 0x10, 0, 0,
			(unsigned short *)ovl_regs_plb->uv_vert_coeff_single);
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

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
	case IGD_PF_ARGB16_1555:
		output =
			((((input & 0x7c00)>>10)<<3)<<16) |
			((((input & 0x03e0)>>5 )<<3)<<8 ) |
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
		output = 0x00000000;
		break;
	case IGD_PF_RGB16_565:
		output = 0x00070307;
		break;
	case IGD_PF_ARGB16_1555:
		output = 0x00070707;
		break;
	case IGD_PF_ARGB8_INDEXED:
		output = 0x00ffff00;
		break;
	}

	return output;
}

/* Convert RGB to GBR for 32bpp and 16bpp pixel formats.
 * Do not convert YUV surfaces or RGB indexed pixel formats */
static unsigned int rgb_to_gbr(
	unsigned long pf,
	unsigned int input)
{
	unsigned int output;

	if (((pf & IGD_PF_TYPE_MASK) == PF_TYPE_ARGB) &&
		((pf & IGD_PF_DEPTH_MASK) != PF_DEPTH_8)) {
		output =
			((input & 0x00ff0000) >> 16) |
			((input & 0x0000ff00) << 8)  |
			((input & 0x000000ff) << 8);
	} else {
		output = input;
	}

	return output;
}


#ifndef OVL_PLB_CACHE_QUICK_SWAP  /* If no OVL_PLB_CACHE_QUICK_SWAP */

static unsigned int ovl_update_regs_plb(
	igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	ovl_reg_image_plb_t *ovl_regs_plb, *ovl_cache_plb ;
	int ret;

	EMGD_TRACE_ENTER;

	ovl_regs_plb = phys_to_virt(ovl_context->reg_update_phys);
	ovl_cache_plb = OS_ALLOC(sizeof(ovl_reg_image_plb_t));
	OS_MEMSET(ovl_cache_plb, 0, sizeof(ovl_reg_image_plb_t));

	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {
		/* Turn the overlay Off */
		ovl_regs_plb->command = 0;
		/* Always use buf 0 when turning the overlay off. */
		ovl_context->ovl_buff = 0;
		OS_FREE(ovl_cache_plb);
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/* Force value of y1 to even due hardware expects even number */
	dest_rect->y1 &= ~1;
	dest_rect->y2 = (dest_rect->y2 + 1) & ~1;
	dest_rect->x1 &= ~1;
	dest_rect->x2 = (dest_rect->x2 + 1) & ~1;

	/*************************************************************************
	 * Copy the information passed in to the HW overlay structure
	 *************************************************************************/
	/* Zero the config and command, since they will be OR'ed in with data
	 * below */
	ovl_cache_plb->config = 0;
	ovl_cache_plb->command = 0;

	/* Set overlay to the proper pipe */
	if (1 == PIPE(display)->pipe_num) {
		/* This is pipe B */
		ovl_cache_plb->config |= 1 << 18;
	}

	/* Interleaved/progressive and Odd/Even if interleaved */
	if (flags & IGD_OVL_ALTER_INTERLEAVED) {
		ovl_cache_plb->command |= OVL_CMD_FIELD_MODE;
		/* Need to enable FIELD SYNC OVERLAY FLIP in field mode. */
		ovl_cache_plb->command |= OVL_CMD_FIELD_SYNC_FLIP;
		/* HSD# 203821 Mplayer outputs single one buffer including both even and odd fields */
		if (flags & IGD_OVL_ALTER_FLIP_ODD) {
			ovl_cache_plb->command |= OVL_CMD_ACT_FLD1;
			/* HSD# 203821 To display odd field, starts from first odd field. */
			if (0 == src_rect->y1 & 1) {
				src_rect->y1 += 1;
			}
		} else {
			ovl_cache_plb->command |= OVL_CMD_ACT_FLD0;
			/* HSD# 203821 To display even field, starts from first even field. */
			if (0 != src_rect->y1 & 1) {
				src_rect->y1 += 1;
			}
		}
	} else {
		ovl_cache_plb->command |= OVL_CMD_FRAME_MODE;
	}

	/* Dest rect information */
	ovl_cache_plb->dest_pos_x_left        = (unsigned short)dest_rect->x1;
	ovl_cache_plb->dest_pos_y_top         = (unsigned short)dest_rect->y1;
	ovl_cache_plb->dest_width_x           =
		(unsigned short)(dest_rect->x2 - dest_rect->x1);
	ovl_cache_plb->dest_height_y          =
		(unsigned short)(dest_rect->y2 - dest_rect->y1);

	/* Src rect and surface information */

	ret = ovl_update_src_ptr_plb(display, ovl_cache_plb, src_surf, src_rect);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay updating src pointers failed");
		return ret;
	}

	ret = ovl_update_src_plb(display, ovl_cache_plb, src_surf, src_rect);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay updating src failed");
		return ret;
	}

	/* Scaling information including Vertical downscaling.
	 * Scaling should be guaranteed to work, since if the scale is not
	 * supported, it should have already been blended to a supported scale. */
	ret = ovl_update_scale_plb(ovl_cache_plb, src_surf, src_rect, dest_rect,
		flags);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay updating scaling failed");
		return ret;
	}

	/* Color control information */
	ret = ovl_update_video_quality_plb(ovl_cache_plb, src_surf,
		&ovl_info->video_quality);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay video quality failed");
		return ret;
	}
	ret = ovl_update_gamma_plb(display, &ovl_info->gamma);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay gamma failed");
		return ret;
	}

	/* Destination color key */
	EMGD_DEBUG("Color key.flags: 0x%lx", ovl_info->color_key.flags);
	if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {
		EMGD_DEBUG("Overlay Enable Dest Color Key");
		/* The mask and color key are different for the different
		 * pixel formats */
		ovl_cache_plb->dest_ckey_val = convert_color_key_to_hw(
			PLANE(display)->fb_info->pixel_format,
			ovl_info->color_key.dest);
		ovl_cache_plb->dest_ckey_mask = convert_color_key_to_mask(
			PLANE(display)->fb_info->pixel_format,
			ovl_info->color_key.dest);
		ovl_cache_plb->dest_ckey_mask |= 0x80000000;
	} else {
		EMGD_DEBUG("Overlay Disable Dest Color Key");
		ovl_cache_plb->dest_ckey_mask = 0x00000000;
	}

	/* Source Color key */
	if (ovl_info->color_key.flags & IGD_OVL_SRC_COLOR_KEY_ENABLE) {
		EMGD_DEBUG("Overlay Enable Src Color Key");
		ovl_cache_plb->source_ckey_high = convert_color_key_to_hw(
			src_surf->pixel_format,
			ovl_info->color_key.src_hi);
		ovl_cache_plb->source_ckey_high = rgb_to_gbr(
			src_surf->pixel_format,
			ovl_cache_plb->source_ckey_high);

		ovl_cache_plb->source_ckey_low = convert_color_key_to_hw(
			src_surf->pixel_format,
			ovl_info->color_key.src_lo);
		ovl_cache_plb->source_ckey_low = rgb_to_gbr(
			src_surf->pixel_format,
			ovl_cache_plb->source_ckey_low);

		ovl_cache_plb->source_ckey_mask = 0x07000000;
	} else {
		EMGD_DEBUG("Overlay Disable Src Color Key");
		ovl_cache_plb->source_ckey_mask = 0x00000000;
	}

	/* Coefficients - Must be after Scaling */
	ret = ovl_update_coeff_plb(ovl_cache_plb, src_surf, src_rect, dest_rect, flags);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay updating coefficient failed");
		return ret;
	}

	/* Phase information - Must be after Coefficients */
	ret = ovl_update_phase_plb(ovl_cache_plb, src_surf, src_rect);
	if (ret) {
		OS_FREE(ovl_cache_plb);
		EMGD_ERROR_EXIT("Overlay updating phase failed");
		return ret;
	}

	/* General overlay information.  Turn the overlay on and alternate
	 * between Buffer 0 and Buffer 1. */
	ovl_cache_plb->command = (ovl_cache_plb->command & 0xfffffff3) |
			ovl_context->ovl_buff | 1;
	ovl_context->ovl_buff ^= OVL_CMD_ACT_BUF1;

	/* Dump out the Overlay Update Registers if debugging */
	EMGD_VERBOSE(hal.dump_overlay_regs, ovl_dump_regs_plb(ovl_regs_plb));

	OS_MEMCPY(ovl_regs_plb, ovl_cache_plb, sizeof(ovl_reg_image_plb_t));
	OS_FREE(ovl_cache_plb);
	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

#else /* Else OVL_PLB_CACHE_QUICK_SWAP */

/* Poulsbo overlay cache structure */
static ovl_plb_cache_t ovl_cache;

/* Flag to signal the cache is invalid and needs
 * to be re-initialized */
static int ovl_cache_needs_init = TRUE;

static unsigned int ovl_update_regs_plb(
	igd_display_context_t *display,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	ovl_reg_image_plb_t *ovl_regs_plb,
		            *ovl_cache_regs;
	unsigned int         cache_changed;
	int                  ret;

	EMGD_TRACE_ENTER;

	/* get the pointers to the real regs, and our cached copy of them */
	ovl_regs_plb = phys_to_virt(ovl_context->reg_update_phys);
	ovl_cache_regs = &ovl_cache.ovl_regs;

	/* Fast path for turning off overlay. No need for cache */
	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {

		/* Turn the overlay Off */
		ovl_regs_plb->command = 0;

		/* if we were using the cache, turn it off there too */
		if (!ovl_cache_needs_init) {
			ovl_cache.ovl_regs.command = 0;
		}

		/* Reset the cache */
		ovl_cache_needs_init = TRUE;

		/* Always use buf 0 when turning the overlay off. */
		ovl_context->ovl_buff = 0;
		EMGD_TRACE_EXIT;
		return IGD_SUCCESS;
	}

	/* Force value to even due hardware expects even number */
	dest_rect->y1 &= ~1;
	dest_rect->y2 = (dest_rect->y2 + 1) & ~1;
	dest_rect->x1 &= ~1;
	dest_rect->x2 = (dest_rect->x2 + 1) & ~1;

	/* Init the cache if necessary */
	if (ovl_cache_needs_init) {


		/* Force every cache check to miss */
		OS_MEMSET(&ovl_cache, 0, sizeof(ovl_plb_cache_t));

		/* We just set our cached flags to 0, which might accidently
		 * match up with "OFF" for some important incoming flag
		 * bits, causing us to think we already handled them when
		 * we didn't.  So set our cached flags to the exact
		 * opposite of the incoming flags, which will force
		 * us to test and handle every single bit, regardless
		 * of whether it is on or off. */
		ovl_cache.flags = ~flags;

		/* init our cached registers */
		OS_MEMCPY(ovl_cache_regs,
			  ovl_regs_plb,
			  sizeof(ovl_reg_image_plb_t));

		/* initialization complete */
		ovl_cache_needs_init = FALSE;
	}

	/* See what has changed in the cache */
	cache_changed = get_cache_changes_plb (src_surf,
					   src_rect,
					   dest_rect,
					   ovl_info,
					   flags,
					   &ovl_cache);

	/* Perhaps the biggest challenge of caching the overlay
	 * state is what to do with the command and config regs.
	 * Normally we would clear command and config to 0 here,
	 * and let the update process set only the bits that are
	 * needed.  But doing this would invalidate our cache.
	 * Instead we are relying on the above call to
	 * get_cache_changes() to clear those bits in command
	 * and config that will be changing */

	/* Set overlay to the proper pipe */
	/* it is cheaper to just set this, than to test it and set it. */
	if (1 == PIPE(display)->pipe_num) {
		/* This is pipe B */
		ovl_cache_regs->config |= 1 << 18;
	} else {
		ovl_cache_regs->config &= ~(1 << 18);
	}

	if (cache_changed & IGD_OVL_PLB_UPDATE_FLAGS) {

		/* Interleaved/progressive and Odd/Even if interleaved. */
		if (flags & IGD_OVL_ALTER_INTERLEAVED) {
			ovl_cache_regs->command |= OVL_CMD_FIELD_MODE;
			/* enable FIELD SYNC OVERLAY FLIP in field mode. */
			ovl_cache_regs->command |= OVL_CMD_FIELD_SYNC_FLIP;
			/* HSD# 203821 Mplayer outputs single one buffer including both even and odd fields */
			if (flags & IGD_OVL_ALTER_FLIP_ODD) {
				ovl_cache_regs->command |= OVL_CMD_ACT_FLD1;
				/* HSD# 203821 To display odd field, starts from first odd field. */
				if (0 == (src_rect->y1 & 1)) {
					src_rect->y1 += 1;
				}
			} else {
				ovl_cache_regs->command |= OVL_CMD_ACT_FLD0;
				/* HSD# 203821 To display even field, starts from first even field. */
				if (0 != (src_rect->y1 & 1)) {
					src_rect->y1 += 1;
				}	
			}
		} else {
			ovl_cache_regs->command |= OVL_CMD_FRAME_MODE;
		}
	}

	/* Has our destination rectangle changed? */
	if (cache_changed & IGD_OVL_PLB_UPDATE_DEST) {
		ovl_cache_regs->dest_pos_x_left  =
			(unsigned short) dest_rect->x1;
		ovl_cache_regs->dest_pos_y_top   =
			(unsigned short) dest_rect->y1;
		ovl_cache_regs->dest_width_x     =
			(unsigned short) (dest_rect->x2 - dest_rect->x1);
		ovl_cache_regs->dest_height_y    =
			(unsigned short) (dest_rect->y2 - dest_rect->y1);

	}

	/* Always update the source pointers every frame */
	ret = ovl_update_src_ptr_plb(display,
				 ovl_cache_regs,
				 src_surf,
				 src_rect);
	if (ret) {
		/* Not good. Invalidate the entire cache and bail. */
		ovl_cache_needs_init = TRUE;
		EMGD_ERROR_EXIT("Overlay updating src pointers failed");
		return ret;
	}

	/* Did either the Src rect or surface change? */
	if (cache_changed & (IGD_OVL_PLB_UPDATE_SURF |
			     IGD_OVL_PLB_UPDATE_SRC  ) ) {

		ret = ovl_update_src_plb(display,
					 ovl_cache_regs,
					 src_surf,
					 src_rect);
		if (ret) {
			/* Not good. Invalidate the entire cache and bail. */
			ovl_cache_needs_init = TRUE;
			EMGD_ERROR_EXIT("Overlay updating src failed");
			return ret;
		}
	}

	/* Scaling information including Vertical downscaling.
	 * Scaling should be guaranteed to work, since if the scale
	 * is not supported, it should have already been blended
	 * to a supported scale. */
	if ( cache_changed & (IGD_OVL_PLB_UPDATE_SRC  |
			      IGD_OVL_PLB_UPDATE_SURF |
			      IGD_OVL_PLB_UPDATE_DEST  |
			      IGD_OVL_PLB_UPDATE_FLAGS) ) {

		ret = ovl_update_scale_plb(ovl_cache_regs,
					   src_surf,
					   src_rect,
					   dest_rect,
					   flags);
		if (ret) {
			/* Not good. Invalidate the entire cache and bail. */
			ovl_cache_needs_init = TRUE;
			EMGD_ERROR_EXIT("Overlay updating scaling failed");
			return ret;
		}
	}

	/* Did video quality change? */
	if (cache_changed & (IGD_OVL_PLB_UPDATE_VQ   |
			     IGD_OVL_PLB_UPDATE_SURF ) ) {
		/* Color control information */

		ret = ovl_update_video_quality_plb(ovl_cache_regs, src_surf,
						   &ovl_info->video_quality);
		if (ret) {
			/* Not good. Invalidate the entire cache and bail. */
			ovl_cache_needs_init = TRUE;
			EMGD_ERROR_EXIT("Overlay video quality failed");
			return ret;
		}
	}

	/* Did gamma change? */
	if (cache_changed & IGD_OVL_PLB_UPDATE_GAMMA) {

		ret = ovl_update_gamma_plb(display, &ovl_info->gamma);
		if (ret) {
			/* Not good. Invalidate the entire cache and bail. */
			ovl_cache_needs_init = TRUE;
			EMGD_ERROR_EXIT("Overlay gamma failed");
			return ret;
		}
	}

	/* Did color key change? */
	if (cache_changed & IGD_OVL_PLB_UPDATE_COLORKEY) {

		/* Destination color key */
		EMGD_DEBUG("Color key.flags: 0x%lx", ovl_info->color_key.flags);
		if (ovl_info->color_key.flags & IGD_OVL_DST_COLOR_KEY_ENABLE) {

			EMGD_DEBUG("Overlay Enable Dest Color Key");
			/* The mask and color key are different for the
			 * different pixel formats */
			ovl_cache_regs->dest_ckey_val =
				convert_color_key_to_hw(
					PLANE(display)->fb_info->pixel_format,
					ovl_info->color_key.dest);
			ovl_cache_regs->dest_ckey_mask =
				convert_color_key_to_mask(
					PLANE(display)->fb_info->pixel_format,
					ovl_info->color_key.dest);
			ovl_cache_regs->dest_ckey_mask |= 0x80000000;
		} else {
			EMGD_DEBUG("Overlay Disable Dest Color Key");
			ovl_cache_regs->dest_ckey_mask = 0x00000000;
		}

		/* Source Color key */
		if (ovl_info->color_key.flags & IGD_OVL_SRC_COLOR_KEY_ENABLE) {

			EMGD_DEBUG("Overlay Enable Src Color Key");
			ovl_cache_regs->source_ckey_high =
				convert_color_key_to_hw(
					src_surf->pixel_format,
					ovl_info->color_key.src_hi);
			ovl_cache_regs->source_ckey_high =
				rgb_to_gbr(src_surf->pixel_format,
					   ovl_cache_regs->source_ckey_high);

			ovl_cache_regs->source_ckey_low =
				convert_color_key_to_hw(
					src_surf->pixel_format,
					ovl_info->color_key.src_lo);
			ovl_cache_regs->source_ckey_low =
				rgb_to_gbr(src_surf->pixel_format,
					   ovl_cache_regs->source_ckey_low);

			ovl_cache_regs->source_ckey_mask = 0x07000000;
		} else {
			EMGD_DEBUG("Overlay Disable Src Color Key");
			ovl_cache_regs->source_ckey_mask = 0x00000000;
		}
	} /* end color key changes */


	/* Coefficients - Must be after Scaling */
	if (cache_changed & (IGD_OVL_PLB_UPDATE_SRC   |
			     IGD_OVL_PLB_UPDATE_SURF  |
			     IGD_OVL_PLB_UPDATE_DEST  |
			     IGD_OVL_PLB_UPDATE_FLAGS ) ) {

		    ret = ovl_update_coeff_plb(ovl_cache_regs,
					       src_surf,
					       src_rect,
					       dest_rect,
					       flags);
		    if (ret) {
			    /* Not good. Invalidate entire cache and bail. */
			    ovl_cache_needs_init = TRUE;
			    EMGD_ERROR_EXIT("Overlay update coefficient failed");
			    return ret;
		    }
	    }

	    /* Phase information - Must be after Coefficients */
	    if (cache_changed & (IGD_OVL_PLB_UPDATE_SRC |
			      IGD_OVL_PLB_UPDATE_SURF ) ) {

		    ret = ovl_update_phase_plb(ovl_cache_regs,
					       src_surf,
					       src_rect);
		    if (ret) {
			    /* Not good. Invalidate entire cache and bail. */
			    ovl_cache_needs_init = TRUE;
			    EMGD_ERROR_EXIT("Overlay updating phase failed");
			    return ret;
		    }
	    }

	    /* General overlay information.  Turn the overlay on and alternate
	     * between Buffer 0 and Buffer 1. */
	    ovl_cache_regs->command = (ovl_cache_regs->command & 0xfffffff3) |
					ovl_context->ovl_buff | 1;
	    ovl_context->ovl_buff ^= OVL_CMD_ACT_BUF1;

	    /* Dump out the Overlay Update Registers if debugging */
	    EMGD_VERBOSE(hal.dump_overlay_regs, ovl_dump_regs_plb(ovl_regs_plb));

	    /* Finally, transfer the cached regs to the real regs */
	    OS_MEMCPY(ovl_regs_plb,
		      ovl_cache_regs,
		      sizeof(ovl_reg_image_plb_t));

	    EMGD_TRACE_EXIT;
	    return IGD_SUCCESS;
}

#endif /* End if OVL_PLB_CACHE_QUICK_SWAP */

static unsigned int ovl_send_instr_plb(
	igd_display_context_t     *display,
	unsigned int      flags)
{
	unsigned char * mmio = MMIO(display);
	unsigned long tmp;

	EMGD_TRACE_ENTER;

	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_ON) {

		ovl_context->state = OVL_STATE_ON;
		/*
		 * If Overlay + FB Blend is requested and the FB is xRGB
		 * turn on the ARGB format.
		 */
		if(ovl_context->fb_blend_ovl) {
			tmp = EMGD_READ32(mmio +  PLANE(display)->plane_reg);
			if((tmp & 0x3c000000) == 0x18000000) {
				EMGD_WRITE32(tmp | 0x1c000000, mmio +  PLANE(display)->plane_reg);
				tmp = EMGD_READ32(mmio + PLANE(display)->plane_reg + 4);
				EMGD_WRITE32(tmp, mmio + PLANE(display)->plane_reg + 4);
			}
		}

	} else {

		if (ovl_context->fb_blend_ovl) {
			tmp = EMGD_READ32(mmio +  PLANE(display)->plane_reg);
			if((tmp & 0x3c000000) == 0x1c000000) {
				tmp = tmp & 0xc3FFFFFF;
				EMGD_WRITE32(tmp | 0x18000000, mmio +  PLANE(display)->plane_reg);
				tmp = EMGD_READ32(mmio + PLANE(display)->plane_reg + 4);
				EMGD_WRITE32(tmp, mmio + PLANE(display)->plane_reg + 4);
				OS_SLEEP(100);
			}
		}

		OS_SLEEP(1);

		/* if overlay is being turned OFF - ensure it's ON first */
		if (ovl_context->state == OVL_STATE_OFF) {
			/* Overlay is already off, no need to turn it off again */
			EMGD_TRACE_EXIT;
			return IGD_SUCCESS;
		}

		ovl_context->state = OVL_STATE_OFF;
	}

	/* Write the address of the memory buffer to the Overlay Update
	 * Address Register causes the HW to load the new values from the
	 * memory on the next VBLANK */
	EMGD_WRITE32((ovl_context->reg_update_phys | 0x1), mmio + 0x30000);

	ovl_context->sync = 0;

	display->context->dispatch.sync(display,
		IGD_PRIORITY_NORMAL,
		&ovl_context->sync,
		IGD_SYNC_NONBLOCK);

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}




static int alter_ovl_plb(igd_display_context_t *display,
			 igd_surface_t       *src_surf,
			 igd_rect_t          *src_rect,
			 igd_rect_t          *dest_rect,
			 igd_ovl_info_t      *ovl_info,
			 unsigned int         flags)
{
	int ret;

	EMGD_TRACE_ENTER;
	/* Dump overlay parameters for debugging */
	/*
	printk (KERN_ERR " alter_ovl_plb  Entry."
		"offset=0x%X "
		"pitch=%d "
		"width=%d "
		"height=%d \n"
		"pixel_format=0x%X "
		"flags=0x%X "
		"virt_addr=0x%X "
		"pvr2d_mem_info=0x%X "
		"pvr2d_context_h=0x%X "
		"hPVR2DFlipChain=0x%X \n"
		"src_x1=%d "
		"src_x2=%d "
		"src_y1=%d "
		"src_y2=%d "
		"src width=%d "
		"src height=%d \n"
		"dest_x1=%d "
		"dest_x2=%d "
		"dest_y1=%d "
		"dest_y2=%d "
		"dest width=%d "
		"dest height=%d \n"
		"color_key.src_lo=0x%X "
		"color_key.src_hi=0x%X "
		"color_key.dest=0x%X "
		"color_key.flags=0x%X "
		"flags=0x%X "
		,
		(unsigned int) src_surf->offset ,
		(unsigned int) src_surf->pitch ,
		(unsigned int) src_surf->width ,
		(unsigned int) src_surf->height ,
		(unsigned int) src_surf->pixel_format ,
		(unsigned int) src_surf->flags ,
		(unsigned int) src_surf->virt_addr ,
		(unsigned int) src_surf->pvr2d_mem_info ,
		(unsigned int) src_surf->pvr2d_context_h ,
		(unsigned int) src_surf->hPVR2DFlipChain ,
		(unsigned int) src_rect->x1,
		(unsigned int) src_rect->x2,
		(unsigned int) src_rect->y1,
		(unsigned int) src_rect->y2,
		(unsigned int) (src_rect->x2 - src_rect->x1),
		(unsigned int) (src_rect->y2 - src_rect->y1),
		(unsigned int) dest_rect->x1,
		(unsigned int) dest_rect->x2,
		(unsigned int) dest_rect->y1,
		(unsigned int) dest_rect->y2,
		(unsigned int) (dest_rect->x2 - dest_rect->x1),
		(unsigned int) (dest_rect->y2 - dest_rect->y1),
		(unsigned int) ovl_info->color_key.src_lo,
		(unsigned int) ovl_info->color_key.src_hi,
		(unsigned int) ovl_info->color_key.dest,
		(unsigned int) ovl_info->color_key.flags,
		(unsigned int) flags
		);
	*/
	/* Dump overlay surface contents, for debugging */
	/*
	if (flags & IGD_OVL_ALTER_ON)
	{
		int i;
		unsigned char *ptr = src_surf->virt_addr;

		for (i = 0; i<10; ++i) {
			printk(KERN_ERR
			       "%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X"
			       "%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X"
			       "%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X"
			       "%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X",
			       ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],
			       ptr[5],ptr[6],ptr[7],ptr[8],ptr[9],
			       ptr[10],ptr[11],ptr[12],ptr[13],ptr[14],
			       ptr[15],ptr[16],ptr[17],ptr[18],ptr[19],
			       ptr[20],ptr[21],ptr[22],ptr[23],ptr[24],
			       ptr[25],ptr[26],ptr[27],ptr[28],ptr[29],
			       ptr[30],ptr[31],ptr[32],ptr[33],ptr[34],
			       ptr[35],ptr[36],ptr[37],ptr[38],ptr[39]);
			ptr += src_surf->pitch;
		}
	}
	*/

	/* Check to ensure the overlay can be used given the current mode as
	 * well as what the IAL is asking for.  If not return an error. */
	ret = ovl_check_plb(display, src_surf, src_rect, dest_rect, ovl_info,
			    flags);
	if (ret) {
		EMGD_ERROR_EXIT("Overlay Check failed");
		return ret;
	}

	/* Check if last flip is still pending.
	 * This is necessary for the following reasons:
	 *    - If the previous instructions have not been processed, then the
	 *      ovl_regs_plb is still in use and can not be overwritten.
	 */
	if ((FALSE == query_ovl_plb(
				    (igd_display_h)display,
				    IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE)) &&
	    (flags & IGD_OVL_ALTER_ON)) {
		/* Only return an error if the overlay is on.  If turning it off,
		 * allow it to continue, since something may have failed and we
		 * should try our best to turn the overlay off. */
		EMGD_ERROR_EXIT("Query Overlay failed");
		return -IGD_ERROR_HWERROR;
	}

	/* Update all Overlay Update Registers */
	ret = ovl_update_regs_plb(display, src_surf, src_rect,
				  dest_rect, ovl_info,
				  flags);
	if (ret) {
		EMGD_ERROR_EXIT("Overlay Update Registers failed");
		return ret;
	}

	/* Send the instructions to the command queue */
	ret = ovl_send_instr_plb(display, flags);

	EMGD_TRACE_EXIT;
	return ret;
}



static int query_ovl_plb(igd_display_h display_h,
	unsigned int flags)
{
	igd_display_context_t *display = (igd_display_context_t *)display_h;
	os_alarm_t timeout;

	EMGD_TRACE_ENTER;

	switch (flags) {

	case IGD_OVL_QUERY_IS_HW_SUPPORTED:
		/* This is the first overlay, so HW overlay is supported */
		break;

	case IGD_OVL_QUERY_IS_LAST_FLIP_DONE:
		/* If there no sync to wait on, then the last flip is done, and the
		 * Register Update has occured, simply return TRUE (Flip done).
		 */
		/*if (!(ovl_context->sync)) {
			return TRUE;
		}*/

		/* Check to see if the last flip instruction has been executed.  If not
		 * return FALSE (Flip not done). */
		/*if(display->context->dispatch.sync(
			display,
			IGD_PRIORITY_NORMAL,
			&ovl_context->sync,
			IGD_SYNC_NONBLOCK)) {
			EMGD_DEBUG("Overlay Sync Check - Flip not done");
			return FALSE;
		}*/

		/* Check to see if the register update is complete.  If not return
		 * FALSE (Flip not done). */
		if(!(EMGD_READ32(MMIO(display) + 0x30008) & 0x80000000)) {
			EMGD_DEBUG("Overlay Status Check - Register Update not done");
			return FALSE;
		}

		/* Now that we know the last flip is done and the register update is
		 * complete, set the sync to 0 and return TRUE (Flip done). */
		ovl_context->sync = 0;
		break;

	case IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE:
		/* Wait for 200 milliseconds for the last flip to complete.  If not
		 * done in that time, there is likely a hardware problem so return
		 * FALSE. */
		timeout = OS_SET_ALARM(200);
		do {
			if (TRUE ==
				query_ovl_plb(display_h, IGD_OVL_QUERY_IS_LAST_FLIP_DONE)) {
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

static int query_max_size_ovl_plb(
	igd_display_h display_h,
	unsigned long pf,
	unsigned int *max_width,
	unsigned int *max_height)
{
	ovl_chipset_plb_t *ovl_chip;

	EMGD_TRACE_ENTER;

	ovl_chip = ovl_chipset_plb;
	*max_width = 0;
	*max_height = 0;
	while(ovl_chip->num_linebuf != OVL_CONFIG_NO_LINE_BUFF){
		if(((pf & IGD_PF_MASK) == ovl_chip->pixel_format) &&
			(ovl_chip->max_width > *max_width)) {
			*max_width = ovl_chip->max_width;
			*max_height = ovl_chip->max_height;
		}
		ovl_chip++;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

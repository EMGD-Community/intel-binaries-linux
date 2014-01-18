/*
 *-----------------------------------------------------------------------------
 * Filename: ovl_virt.h
 * $Revision: 1.9 $
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
 *  This is the internal header file for overlay. It should be not be
 *  by any other module besides the overlay module itself. It contains the
 *  neccessary hardware virtualized structures and functions internal to
 *  overlay
 *-----------------------------------------------------------------------------
 */

#ifndef _OVL_VIRT_H
#define _OVL_VIRT_H

/* None of the IALs use the OVL_SUPPORT_*.  Exclude it for now. */
/* None of the IALs use the OVL_RULE_MUST_*.  Exclude it for now. */

/* Overlay HW range of values for color control and gamma correction*/
#define OVL_HW_DEF_BRIGHT      750L
#define OVL_HW_MIN_BRIGHT      0L
#define OVL_HW_MAX_BRIGHT      10000L

#define OVL_HW_DEF_CONT	    10000L
#define OVL_HW_MIN_CONT	    0L
#define OVL_HW_MAX_CONT	    20000L

#define OVL_HW_DEF_SAT	        10000L
#define OVL_HW_MIN_SAT		    0L
#define OVL_HW_MAX_SAT	        20000L

#define OVL_HW_DEF_HUE			0L
#define OVL_HW_MIN_HUE			-180L
#define OVL_MHW_AX_HUE			180L

#define OVL_HW_DEF_GAMMA       1L
#define OVL_HW_MAX_GAMMA       500L
#define OVL_HW_MIN_GAMMA       1L

#define OVL_REPROG_OP_NONE            0
#define OVL_REPROG_OP_TURNON_PRIM     1
#define OVL_REPROG_OP_TURNON_SECND    2
#define OVL_REPROG_OP_TURNOFF_PRIM    3
#define OVL_REPROG_OP_TURNOFF_SECND   4
#define OVL_REPROG_OP_SWITCH_TO_PRIM  5
#define OVL_REPROG_OP_SWITCH_TO_SECND 6
#define OVL_REPROG_OP_REVERT_SECND	  7
#define OVL_REPROG_OP_REVERT_PRIM	  8

enum {
	OVL_STATE_OFF = 0,
	OVL_STATE_ON,
};

#define OVL_PRIMARY   0
#define OVL_SECONDARY 1
#define OVL_MAX_HW    2  /* Maximum number of overlays */

/* Define the maximum number of blend surfaces which can be used */
#define MAX_BLEND_SURF 3

typedef struct _ovl_context{
	ovl_dispatch_t (*dispatch)[];  /* Pointer to an array */
	unsigned int  state;
	unsigned long reg_update_offset;
	unsigned long reg_update_phys;
	unsigned long sync;
	unsigned long sync2;
	unsigned int blend_surf_num[OVL_MAX_HW];
	igd_surface_t blend_surf[OVL_MAX_HW][MAX_BLEND_SURF];
	unsigned int ovl_buff;
	unsigned int fb_blend_ovl;
	unsigned short reg_allocated;
	igd_surface_t  *saved_src_surf;
	igd_rect_t     *saved_src_rect;
	igd_rect_t     *saved_dest_rect;
	igd_ovl_info_t *saved_ovl_info;
	unsigned int   saved_flags;
	igd_display_context_t * ovl_display_km[OVL_MAX_HW];
	unsigned int ovl_display_swapped;
} ovl_context_t;

extern ovl_context_t ovl_context[];

#endif /*_OVL_VIRT_H*/

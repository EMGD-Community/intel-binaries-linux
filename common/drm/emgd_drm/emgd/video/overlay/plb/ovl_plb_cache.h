/*
 *----------------------------------------------------------------------------
 * Filename: ovl_plb_cache.h
 * $Revision: 1.5 $
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
 *  This is the internal header file for overlay caching. It should be not be
 *  by any other module besides the overlay module itself.
 *-----------------------------------------------------------------------------
 */
#ifndef _OVL_PLB_CACHE_H
#define _OVL_PLB_CACHE_H

#include <rb.h>
#include "ovl_regs_plb.h"
#include "ovl2_regs_plb.h"

/* Turns the overlay cache on and off. */
#define OVL_PLB_CACHE_QUICK_SWAP


/*
 * In order to cache the overlay registers we need a structure
 * that defines and wraps them.  We already have one for the primary
 * display, called ovl_reg_image_plb_t.
 * Here we define a structure for the secondary display
 */

typedef struct _ovl2_reg_plb_cache{
	unsigned int control;
	unsigned int start;
	unsigned int plane_control;
	unsigned int plane_start;

	unsigned int pitch;
	unsigned int dest_rect_x1y1;
	unsigned int dest_rect_x2y2;
	unsigned int cont_bright;
	unsigned int satn_hue;
	unsigned int gamma_regs[OVL2_TOTAL_GAMMA_REG];
	unsigned int colorkey_hw;
	unsigned int colorkey_mask;
	unsigned int ckey_high;
	unsigned int ckey_low;
	unsigned int ckey_enable;

} ovl2_reg_plb_cache_t;

#define MMIO_OFFSET_PLB2_PITCH          0x72188
#define MMIO_OFFSET_PLB2_DEST_RECT_X1Y1 0x7218C
#define MMIO_OFFSET_PLB2_DEST_RECT_X2Y2 0x72190
#define MMIO_OFFSET_PLB2_CONT_BRIGHT    0x721D0
#define MMIO_OFFSET_PLB2_SATN_HUE       0x721D4
#define MMIO_OFFSET_PLB2_COLORKEY_HW    0x14
#define MMIO_OFFSET_PLB2_COLORKEY_MASK  0x18
#define MMIO_OFFSET_PLB2_CKEY_LOW       0x72194
#define MMIO_OFFSET_PLB2_CKEY_HIGH      0x721a0
#define MMIO_OFFSET_PLB2_CKEY_ENABLE    0x72198
#define MMIO_OFFSET_PLB2_CONTROL        0x72180
#define MMIO_OFFSET_PLB2_START          0x72184


/*
 * This stucture caches the overlay state, so we don't have to
 * re-program everything for every single frame
 */
typedef struct _ovl_plb_cache {
	igd_surface_t        src_surf;
	igd_rect_t           src_rect;
	igd_rect_t           dest_rect;
	igd_ovl_info_t       ovl_info;
	unsigned int         flags;
	ovl_reg_image_plb_t  ovl_regs;
	ovl2_reg_plb_cache_t ovl2_regs;
} ovl_plb_cache_t, *povl_plb_cache_t;


/* Flags for things that might have changed in the cache */
#define IGD_OVL_PLB_UPDATE_DEST     (1 << 0)
#define IGD_OVL_PLB_UPDATE_SRC      (1 << 1)
#define IGD_OVL_PLB_UPDATE_SURF     (1 << 2)
#define IGD_OVL_PLB_UPDATE_INFO     (1 << 3)
#define IGD_OVL_PLB_UPDATE_FLAGS    (1 << 4)
#define IGD_OVL_PLB_UPDATE_VQ       (1 << 5)
#define IGD_OVL_PLB_UPDATE_GAMMA    (1 << 6)
#define IGD_OVL_PLB_UPDATE_COLORKEY (1 << 7)


/*
 * Caching helper functions.  Implemented in ovl_plb.c, but used by
 * the ovl2 and micro_ovl modules as well.
 */

/* Tells if a rectangle has changed */
int is_changed_rect(igd_rect_t *old_rect, igd_rect_t *new_rect);

/* Tells if a surface has changed */
int is_changed_surf(igd_surface_t *old_surf, igd_surface_t *new_surf);

/* Tells if the color key has changed */
 int is_changed_color_key(igd_ovl_info_t *old_info, igd_ovl_info_t *new_info) ;

/* Tells if the video quality has changed */
int is_changed_vq(igd_ovl_info_t *old_info, igd_ovl_info_t *new_info);

/* Tells if the gamma has changed */
int is_changed_gamma(igd_ovl_info_t *old_info, igd_ovl_info_t *new_info);

/* Copies a source rectangle to the cache */
void copy_src_rect_plb(igd_rect_t *src_rect, povl_plb_cache_t ovl_cache);

/* Copies a dest rectangle to the cache */
void copy_dest_rect_plb(igd_rect_t *dest_rect, povl_plb_cache_t ovl_cache);

/* Copies a surface to the cache */
void copy_surf_plb(igd_surface_t *src_surf, povl_plb_cache_t ovl_cache);

/* Copies the color key to the cache */
void copy_color_key_plb(igd_ovl_info_t *ovl_info, povl_plb_cache_t ovl_cache);

/* Copies the video quality info to the cache */
void copy_vq_plb(igd_ovl_info_t *ovl_info, povl_plb_cache_t ovl_cache);

/* Copies the gamma to the cache */
void copy_gamma_plb(igd_ovl_info_t *ovl_info, povl_plb_cache_t ovl_cache);

/* Checks to see what, if anything has changed.
 * Clears bits in the command and config register that are invalid.
 * Returns a set of flags telling what changed */
unsigned int get_cache_changes_plb(
			       igd_surface_t  *src_surf,
			       igd_rect_t     *src_rect,
			       igd_rect_t     *dest_rect,
			       igd_ovl_info_t *ovl_info,
			       unsigned int    flags,
			       povl_plb_cache_t ovl_cache);



#endif /* _OVL_PLB_CACHE_H */

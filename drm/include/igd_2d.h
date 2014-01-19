/*
 *-----------------------------------------------------------------------------
 * Filename: igd_2d.h
 * $Revision: 1.6 $
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
 *  This is a header file for the Intel GFX commands.
 *  This includes commands specific to Intel hardware and structures specific
 *  to Intel hardware.  All other commands and structures are available
 *  through GFX.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_2D_H
#define _IGD_2D_H

/* Needed for igd_display_h and pixel_format. */
#include <igd_mode.h>
#include <igd_render.h>
#include <igd_appcontext.h>

/* byte mask */
#define IGD_ALPHA_CHANNEL 0x200000
#define IGD_RGB_CHANNEL   0x100000

/* flags for XY_SETUP_CLIP_BLT */
#define IGD_CLIP_ENABLED		0x01
#define IGD_CLIP_DISABLED		0x00

/* flags for igd_mono_src_t->pat_select_transp_mode for functions
 *  igd_mono_src_copy_blt
 *  igd_full_mono_src_blt
 *  igd_full_mono_pat_blt
 *  igd_full_mono_pat_mono_src_blt
 */
#define IGD_SRC_TRANSP_MODE_BACKGROUND	(0x0 << 29)
#define IGD_SRC_TRANSP_MODE_ENABLED		(0x1 << 29)

/* flags for igd_mono_pat_t->pat_select_transp_mode for functions
 *  igd_full_mono_pat_blt
 *  igd_full_mono_pat_mono_src_blt
*/
#define IGD_PAT_SOLID                   (0x1 << 31)
#define IGD_PAT_NO_SOLID                (0x0 << 31)

/* flags for igd_mono_pat_t->pat_select_transp_mode for functions
 *  igd_mono_pat_blt
 *  igd_full_mono_pat_blt
 *  igd_full_mono_pat_mono_src_blt
 * FIXME: Does this work for igd_full_mono_pat_blt???
 */
#define IGD_PAT_TRANSP_MODE_BACKGROUND	(0x0 << 28)
#define IGD_PAT_TRANSP_MODE_ENABLED		(0x1 << 28)


/* chroma transparency range mode */
#define IGD_NO_TRANSP             0
#define IGD_SRC_RGB_TRANSP        1
#define IGD_SRC_RGB_ALPHA_TRANSP  2
#define IGD_DEST_RGB_TRANSP       3
#define IGD_DEST_RGB_ALPHA_TRANSP 4


typedef struct igd_chroma {
	unsigned int transp_range_mode;
	unsigned int color_low;
	unsigned int color_high;
} igd_chroma_t;

typedef struct igd_mono_src {
	unsigned int data_bit_pos;
	unsigned int mono_src_transp_mode;
	/*
	 * FIXME: Maybe. Immediate mode instructions need a virtual
	 *  address. The rest need an offset. Hence there are two
	 *  address values in this structure. The type of instructure
	 *  determines which one is used.  This is a bit confusing for
	 *  the IAL layer. Ideally, these would be combined into one value.
	 */
	unsigned long addr;
	unsigned char *virt_addr;  /* Used by Immediate mode instructions */
	unsigned int bgcolor;
	unsigned int fgcolor;
	int mono_src_size;
} igd_mono_src_t;

/* fixed pattern not supported yet */
typedef struct igd_mono_pat {
	unsigned int pat_select_transp_mode;
	unsigned int horiz_seed;
	unsigned int vert_seed;
	unsigned int bgcolor;
	unsigned int fgcolor;
	unsigned int data0;
	unsigned int data1;
} igd_mono_pat_t;

typedef struct igd_pat {
	unsigned int horiz_seed;
	unsigned int vert_seed;
	unsigned long base_addr;
    unsigned char *virt_addr;
} igd_pat_t;



/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->setup_clip_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_rect_t *dest_rect,
 *    igd_appcontext_h appcontext_handle,
 *    unsigned int flags);
 *
 * Description:
 *  XY_SETUP_CLIP_BLT: used to change the clip coordinate registers
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */



/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->color_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    unsigned int byte_mask,
 *    unsigned int color,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  color_blt: copy solid_color to dest at specific address
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */


/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->rgb_color_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    unsigned int byte_mask,
 *    unsigned int color,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */



/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->pat_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    unsigned int byte_mask,
 *    igd_pat_t *pat,
 *    igd_chroma_t *chroma,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  pat_blt() copy a color pattern to the dest surface
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */


/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->mono_pat_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    unsigned int byte_mask,
 *    igd_mono_pat_t *pat,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  mono_pat_blt(): copy a monochrome pattern to the dest surface
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */

/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->src_copy_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    igd_surface_t *src,
 *    igd_coord_t *src_coord,
 *    unsigned int byte_mask,
 *    igd_chroma_t *chroma,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  src_copy_blt: copy a source surface to the dest surface
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */


/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->mono_src_copy_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    igd_mono_src_t *src,
 *    unsigned int byte_mask,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  mono_src_copy_blt: color source copy with chroma-keying
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */

/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->full_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    igd_surface_t *src,
 *    igd_coord_t *src_coord,
 *    unsigned int byte_mask,
 *    igd_pat_t *pat,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  full_blt: copy with specification of destination, source
 *  and pattern
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */



/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->full_mono_src_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    igd_mono_src_t *src,
 *    unsigned int byte_mask,
 *    igd_pat_t *pat,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  full_mono_src_blt: copy with specification of destination, source
 *  and pattern where source is monochrome
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */



/*----------------------------------------------------------------------
 * Function:
 *   int dispatch->full_mono_pat_blt(igd_display_h display_handle,
 *     int priority,
 *     igd_surface_t *dest,
 *     igd_rect_t *dest_rect,
 *     igd_coord_t *src_coord,
 *     igd_surface_t *src,
 *     unsigned int byte_mask,
 *     igd_mono_pat_t *pat,
 *     unsigned int raster_ops,
 *     igd_appcontext_h appcontext,
 *     unsigned int flags);
 *
 * Description:
 *  full_mono_pat_blt copy with specification of destination, source
 *  and pattern where pattern is monochrome
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */



/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->full_mono_pat_mono_src_blt(igd_display_h display_handle,
 *    int priority,
 *    igd_surface_t *dest,
 *    igd_rect_t *dest_rect,
 *    igd_mono_src_t *src,
 *    unsigned int byte_mask,
 *    igd_mono_pat_t *pat,
 *    unsigned int raster_ops,
 *    igd_appcontext_h appcontext,
 *    unsigned int flags);
 *
 * Description:
 *  full_mono_pat_mono_src_blt copy with specification of destination, source
 *  and pattern where patter and source are monochrome
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */

#endif /* _IGD_2D_H */

/*
 *-----------------------------------------------------------------------------
 * Filename: decode.h
 * $Revision: 1.7 $
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
 *  This is the internal header file for the video decode module.
 *-----------------------------------------------------------------------------
 */


#ifndef _IGD_DECODE_H
#define _IGD_DECODE_H

#include <igd_render.h>  /* For surface definition */

#define IGD_VD_MAX_SURFACES 2048

#define VID_SURFACE(ctx, id) \
	(((ctx->context_info.surfmap == NULL) ? &surface_list[id] : &surface_list[ctx->context_info.surfmap->slist_index[id]]))

typedef struct _igd_vd_surfmap_table {
	int active_entries;
	int total_surfaces;
	int *slist_index;
} igd_vd_surfmap_table_t;

typedef struct _igd_vd_surface_list {
	int allocated;
	igd_surface_t *surface;
	int chroma_offset;
	int stride_mode;
    /* Used to store driver private data, e.g. decoder specific intermediate status data
     * extra_info[0-3]: used for decode
     * extra_info[4]: surface fourcc
     * extra_info[5]: surface skippeded or not for encode
     */
	int extra_info[6];
	igd_region_t buf;
	igd_vd_surfmap_table_t* surfmap;
} igd_vd_surface_list_t;

struct _igd_video_context {
	igd_vp_name_t profile;
	igd_ve_name_t entrypoint;
	igd_vd_surface_list_t *current_render_target;
	igd_vd_surfmap_table_t *surfmap;

	/*
	 * The actual hardware/profile context is much larger. The above
	 * members are device indpendent.
	 */
};

#endif /*_IGD_DECODE_H*/

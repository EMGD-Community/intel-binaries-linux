/*
 *-----------------------------------------------------------------------------
 * Filename: memlist.h
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
 *  This file contains the header information for the chunk layer of gmm.
 *  It should not be included outside gmm.
 *-----------------------------------------------------------------------------
 */

#ifndef _MEMLIST_H
#define _MEMLIST_H

#include <gart.h>
#include <igd_gmm.h>
#include <context.h>
#include <drmP.h>

#define GMM_NODE_FREE  0x0
#define GMM_NODE_USED  0x1
#define GMM_NODE_FIXED 0x2

/****************************************************************************
 * gmm_node_t
 *  This structure is an individual node representing an range of memory.
 *  It may be allocated or free.
 ****************************************************************************/
typedef struct _gmm_node {
	struct _gmm_node *next;
	struct _gmm_node *previous;
	unsigned long start;
	unsigned long end;
	unsigned long size;
	unsigned long status;
	os_gart_allocation_t gart_handle;
	unsigned long phys;
#ifdef DEBUG_BUILD_TYPE
	char desc[GMM_DEBUG_DESC_SIZE];
	unsigned long pixel_format; /* Pixel Format == 0 for Regions */
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int type;
	unsigned long flags;
#endif
}gmm_node_t;

#ifdef DEBUG_BUILD_TYPE
extern unsigned long _gmm_debug_pixel_format;
extern unsigned int _gmm_debug_width;
extern unsigned int _gmm_debug_height;
extern unsigned int _gmm_debug_pitch;
extern unsigned int _gmm_debug_type;
extern unsigned long _gmm_debug_flags;

#define GMM_SET_DEBUG_INFO(pf, w, h, p, t, f) \
	_gmm_debug_pixel_format = pf;    \
	_gmm_debug_width = w;			 \
	_gmm_debug_height = h;			 \
	_gmm_debug_pitch = p;			 \
	_gmm_debug_type = t;			 \
	_gmm_debug_flags = f;

#else
#define GMM_SET_DEBUG_INFO(pf, w, h, p, t, f)
#endif

/*
 * Flags used when creating chunks or when requesting space from existing
 * chunks.
 */
#define GMM_CHUNK_FREE      0x0
#define GMM_CHUNK_LINEAR    0x1
#define GMM_CHUNK_FENCED    0x2
#define GMM_CHUNK_YMAJOR    0x4
/*
 * Reserved chunks are marked with this flag. Only a single surface may
 * be allocated in reserved chunks. Reserved chunks may or may not have
 * a fence pre-allocated.
 */
#define GMM_CHUNK_RESERVED  0x8
#define GMM_CHUNK_TYPE_MASK 0xf

typedef struct _gmm_mem_buffer  {
	unsigned long size;
	unsigned long type;
	unsigned long physical;
	struct page **pages;
	int page_count;
	int vmalloc_flag;
} gmm_mem_buffer_t;

typedef struct _gmm_chunk {
	/* Next chunk in the list */
	struct _gmm_chunk *next;
	/* Previous chunk in the list */
	struct _gmm_chunk *previous;
	/* Current chunk usage state */
	enum {
		INUSE_ALLOCATED,
		FREE_ALLOCATED,  /* gmm-allocated pages attached */
		INUSE_IMPORTED,
		FREE_IMPORTED    /* address space only; no pages attached */
	} usage;
	/* The offset of the allocated memory, after alignment */
	unsigned long offset;
	/* The number of bytes of memory requested/allocated for this chunk */
	unsigned long size;
	/* The number of pages of memory required for "size" bytes */
	unsigned long pages;
	/* A boolean flag indicating if the chunk's memory has been bound by AGP */
	unsigned long bound;
	/* Either AGP_PHYS_MEMORY or AGP_NORMAL_MEMORY */
	unsigned long type;
	gmm_mem_buffer_t *gtt_mem;
	void *addr;
	unsigned long ref_cnt;
	/* An array of page addresses (created by gmm_get_page_list() */
	unsigned long *page_addresses;
} gmm_chunk_t;

typedef struct _gmm_context_t {
	unsigned long max_mem;
	gmm_chunk_t *head_chunk;
	gmm_chunk_t *tail_chunk;
	igd_memstat_t memstat;
	igd_context_t *context;
	igd_gtt_info_t *gtt;
} gmm_context_t;

#endif

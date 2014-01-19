/*
 *-----------------------------------------------------------------------------
 * Filename: igd_gmm.h
 * $Revision: 1.13 $
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
 *  This is the public header file for gmm. It should be included by any
 *  client driver or igd module that requires gmm interaction.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_MM_H
#define _IGD_MM_H

#include <igd_mode.h>
#include <igd_render.h>

/*!
 * @addtogroup gmm_group
 *
 * <B>Relavent Dispatch Functions</B>
 *  - _igd_dispatch::gmm_alloc_surface()
 *  - _igd_dispatch::gmm_alloc_region()
 *  - _igd_dispatch::gmm_free()
 *  - _igd_dispatch::gmm_memstat()
 *  - _igd_dispatch::gmm_alloc_cached()
 *  - _igd_dispatch::gmm_free_cached()
 *  - _igd_dispatch::gmm_flush_cache()
 *
 * @{
 */

/*!
 * @name Surface Alignment Flags
 * @anchor surface_info_flags
 *
 * Surface Alignment Flags are passed to the _igd_dispatch::gmm_alloc_surface()
 * function as an indicator for how the surface will be used.
 *
 * - IGD_SURFACE_RENDER The surface may be used as a rendering target,
 *     this must be set for all color buffers.
 * - IGD_SURFACE_TEXTURE The surface may be used as a texture input.
 * - IGD_SURFACE_CURSOR Surface may be used as a cursor
 * - IGD_SURFACE_OVERLAY Surface may be used with the overlay
 * - IGD_SURFACE_DISPLAY This surface is suitable for use with the display
 *   engine. This flag must be set for any render target that may later
 *   be flipped to the front buffer.
 * - IGD_SURFACE_DEPTH The surface may be used as a depth (Z) buffer.
 * - IGD_SURFACE_DRI2 The surface is a DRI2 allocation (back buffer,
 *   fake front buffer, etc.).  It should be allocated on the general
 *   heap rather than the default '2D' heap.
 *
 * - IGD_SURFACE_YMAJOR If the surface is tiled, it is tiled with the
 *     walk in the YMajor direction. This flag is output only unless the
 *     allocation type is Fixed TF.
 * - IGD_SURFACE_XMAJOR If the surface is tiled, it is tiled with the
 *     walk in the YMajor direction. This flag is output only unless the
 *     allocation type is Fixed TF.
 * - IGD_SURFACE_TILED Surface is tiled. This flag is output only unless
 *     the allocation type is Fixed TF.
 * - IGD_SURFACE_FENCED Surface is fenced. This flaf is output only unless
 *     the allocation type is Fixed TF.
 *
 * Additionally all FB flags can be populated
 * See: @ref fb_info_flags
 *
 * @{
 */
#define IGD_SURFACE_RENDER    0x00000001
#define IGD_SURFACE_TEXTURE   0x00000002
#define IGD_SURFACE_CURSOR    0x00000004
#define IGD_SURFACE_OVERLAY   0x00000008
#define IGD_SURFACE_DISPLAY   0x00000010
#define IGD_SURFACE_DEPTH     0x00000020
#define IGD_SURFACE_VIDEO     0x00000040
#define IGD_SURFACE_VIDEO_ENCODE     0x00000080
#define IGD_SURFACE_DRI2      0x00000100

#define IGD_SURFACE_WALK_MASK 0x00001000
#define IGD_SURFACE_YMAJOR    0x00001000
#define IGD_SURFACE_XMAJOR    0x00000000
#define IGD_SURFACE_TILED     0x00002000
#define IGD_SURFACE_FENCED    0x00004000
#define IGD_SURFACE_SYS_MEM   0x00008000
#define IGD_SURFACE_PHYS_PTR  0x00010000
#define IGD_SURFACE_CI		0x00020000
#define IGD_SURFACE_CI_V4L2_MAP	0x00040000		/*will be set when map_method=0 without WA*/
#define IGD_SURFACE_CI_TEXTSTREAM		0x00080000
/*
 * Mipmap flags are only valid on mipmap surfaces.
 * Planes flags are only valid on volume surfaces.
 */
#define IGD_MIPLEVEL_SHIFT    16
#define IGD_MIPLEVEL_MASK     (0xf<<IGD_MIPLEVEL_SHIFT)
#define IGD_NUMPLANES_SHIFT   20
#define IGD_NUMPLANES_MASK    (0x0ff00000UL)

/*
 * These flags exist in the Framebuffer info data structure also.
 * They are here to insure that no overlap of flags happens.
 */
#define IGD_SURFACE_RESERVED_FLAGS 0xf0000000

/*! @} */

/*!
 * @name Surface Allocation Types
 * @anchor alloc_surface_types
 *
 * These types are used with the _igd_dispatch::alloc_surface() call to
 * control the allocation behavior.
 *
 *  - IGD_GMM_ALLOC_TYPE_NORMAL Normal surface allocation. GMM will
 *      determine the best tiling and fencing options and return that
 *      information in the surface flags.
 *  - IGD_GMM_ALLOC_TYPE_FIXED_TF Surface allocation will be done using
 *      the tiled and fenced options from the surface flags. GMM will
 *      not override these flags but will fail an allocation if the
 *      tiling options would result in an unusable surface.
 *  - IGD_GMM_ALLOC_TYPE_RESERVATION The offset provided in the call
 *      was reserved earlier. The provided offset must be used. This
 *      may be used with the Fixed bit enabled.
 * @{
 */
#define IGD_GMM_ALLOC_TYPE_NORMAL       0x0000
#define IGD_GMM_ALLOC_TYPE_FIXED_TF     0x0001
#define IGD_GMM_ALLOC_TYPE_RESERVATION  0x0002

/*! @} */

/*!
 * @name Mipmap layout type
 * @anchor alloc_mipmap
 *
 * These constants are used when allocating surfaces to know how many
 * offsets to expect GMM to return.
 *
 * - IGD_GMM_SURFACE_LAYOUT_RIGHT LOD1 be on the right of LOD0
 * - IGD_GMM_SURFACE_LAYOUT_BELOW LOD1 is below LOD0
 * - IGD_GMM_SURFACE_LAYOUT_LEGACY Traditional format, used by 
 *   915 and below
 * @{
 */

#define IGD_GMM_SURFACE_LAYOUT_RIGHT    0x0001
#define IGD_GMM_SURFACE_LAYOUT_BELOW    0x0002
#define IGD_GMM_SURFACE_LAYOUT_LEGACY   0x0003
/*! @} */

/*!
 * @name Surface Allocation Constants
 * @anchor alloc_surface_constants
 *
 * These constants are used when allocating surfaces to know how many
 * offsets to expect GMM to return.
 *
 * - IGD_MAX_MIPLEVELS The maximum number of miplevels returned by GMM.
 * - IGD_MAX_VOLUME_PLANES The maximum number of planes in a volume map.
 * - IGD_MAX_VOLUME_SLICES The maximum number of slices in a mipped volume map.
 *    There are 50% fewer planes with each successive mip level.
 *    The number of slices can be calculated as,
 *     (planes*2 - 1) - ((planes>>lod) - 1)
 * @{
 */
#define IGD_MAX_MIPLEVELS 12
#define IGD_MAX_VOLUME_PLANES 256
#define IGD_MAX_VOLUME_SLICES 511
/*! @} */

/*!
 * @name Region Alignment Flags
 * @anchor alloc_region_flags
 *
 * @{
 */
#define IGD_GMM_REGION_ALIGN_MMAP       0x00000001
#define IGD_GMM_REGION_ALIGN_SIZE       0x00000002 /* Size Aligned */
#define IGD_GMM_REGION_ALIGN_CONTEXT    0x00000003
#ifdef D3D_DPM_ALLOC
#define IGD_GMM_DOUBLE_MAP              0x00000004 /* mapped to SGX/DPM space */
#endif
#define IGD_GMM_REGION_ALIGN_64K        0x00000010
/*! @} */

/*!
 * @name Region Allocation Types
 * @anchor alloc_region_types
 *
 * @{
 */
#define IGD_GMM_REGION_TYPE_DMA         0x00000001
#define IGD_GMM_REGION_TYPE_RING        0x00000002
#define IGD_GMM_REGION_TYPE_OVLREG      0x00000003
#define IGD_GMM_REGION_TYPE_CONTEXT     0x00000004
#define IGD_GMM_REGION_TYPE_HWSTATUS    0x00000005
#define IGD_GMM_REGION_TYPE_BPL			0x00000006
#define IGD_GMM_REGION_TYPE_PERSISTENT	0x00000007
#define IGD_GMM_REGION_TYPE_OVLREG64	0x00000008
#define IGD_GMM_REGION_TYPE_RANDEC0		0x00000009
#define IGD_GMM_REGION_TYPE_RANDEC1		0x0000000A
/*! @} */

/*!
 * @name Reservation Flags
 * @anchor alloc_reservation_flags
 *
 * @{
 */
#define IGD_GMM_RESERVATION_FENCED       0x00000001
/*! @} */


/*!
 * @brief Memory stats returned from _igd_dispatch::gmm_memstat()
 *
 *  This data structure is used to retrieve the memory usage statistics
 *  from the HAL with a dispatch->gmm_memstat() function call.
 */
typedef struct _igd_memstat {
	/*! @brief The total memory space available (Bytes) */
	unsigned long total_avail;
	/*! @brief Total memory used (Bytes) */
	unsigned long total_used;
	/*! @brief Maximum offscreen memory allowed (Bytes) */
	unsigned long offscreen_max;
	/*! @brief The amount of offscreen memory currently used (Bytes) */
	unsigned long offscreen_used;
	unsigned long offscreen_static_used;
} igd_memstat_t;


struct emgd_ci_meminfo_t{
	unsigned long v4l2_offset;
	unsigned long virt;
	unsigned long size;
	unsigned int used;
	unsigned long vbufqueue_handle;
};
/*!
 * @name GMM Alloc Cached Flags
 * @anchor gmm_alloc_cached_flags
 *
 * These flags are used with the _igd_dispatch::gmm_alloc_cached() dispatch
 * function.
 *
 * - IGD_GMM_FLUSH_ALL: Flush out all cached surfaces if needed.
 * - 0: Flush out no surfaces.
 *
 * @{
 */
#define IGD_GMM_FLUSH_ALL    0x1
/*! @} */

typedef unsigned long (*_gmm_get_pvtheap_size)(void);
typedef unsigned long (*_gmm_gmm_get_cache_mem)(void);

typedef int (*_gmm_alloc_surface_fn_t)(
	unsigned long *offset,
	unsigned long pixel_format,
	unsigned int *width,
	unsigned int *height,
	unsigned int *pitch,
	unsigned long *size,
	unsigned int type,
	unsigned long *flags);

typedef int (*_gmm_get_num_surface_fn_t)(
	unsigned long *count);

typedef int (*_gmm_get_surface_list_fn_t)(
	unsigned long allocated_size,
	unsigned long *list_size,
	igd_surface_list_t **surface_list);

typedef int (*_gmm_alloc_region_fn_t)(
	unsigned long *offset,
	unsigned long *size,
	unsigned int type,
	unsigned long flags);

typedef int (*_gmm_virt_to_phys_fn_t)(
	unsigned long offset,
	unsigned long *physical);

typedef void (*_gmm_free_fn_t)(unsigned long offset);

typedef int (*_gmm_memstat_fn_t)(igd_memstat_t *memstat);

typedef int (*_gmm_alloc_cached_fn_t)(igd_display_h display_handle,
	igd_surface_t *surface, unsigned int flags);

typedef void (*_gmm_free_cached_fn_t)(igd_display_h display_handle,
	igd_surface_t *surface,
	unsigned long sync_id);

typedef int (*_gmm_alloc_cached_region_fn_t)(igd_display_h display_handle,
	unsigned long *offset, 
	unsigned long *size,
	unsigned int type,
	unsigned int region_flags,
	unsigned int flags);

typedef void (*_gmm_free_cached_region_fn_t)(igd_display_h display_handle,
	unsigned long offset, 
	unsigned long size,
	unsigned int type,
	unsigned int region_flags,
	unsigned long sync_id_write,
	unsigned long sync_id_read);

typedef int (*_gmm_flush_cache_fn_t)(void);


/*
 * This interface is used in debug driver only. An IAL or HAL component
 * may point this at a string (char[80]) prior to a surface/region
 * allocation. A copy of this string will be stored with the allocation
 * information and will be seen in a GMM dump.
 */
#define GMM_DEBUG_DESC_SIZE 80
extern char gmm_debug_desc[];

#if defined(DEBUG_BUILD_TYPE) && defined(CONFIG_GMM)

/*
 * This function is only used to copy the debug description name into
 * the debug string. This way we don't have to worry about a strcpy
 * being available.
 * Note: This function is not a real strcpy. It is hardcoded for the
 * needs of the gmm debug string copy.
 */
static __inline void _gmm_strcpy(const char *src)
{
	int i;

	for(i=0; i<GMM_DEBUG_DESC_SIZE; i++) {
		gmm_debug_desc[i] = src[i];
		if(!gmm_debug_desc[i]) {
			return;
		}
	}
	gmm_debug_desc[GMM_DEBUG_DESC_SIZE-1] = '\0';
	return;
}

static __inline void _gmm_strcpy2(char *dest, const char *src)
{
	int i;

	for(i=0; i<GMM_DEBUG_DESC_SIZE; i++) {
		dest[i] = src[i];
		if(!dest[i]) {
			return;
		}
	}
	dest[GMM_DEBUG_DESC_SIZE-1] = '\0';
	return;
}

#define GMM_SET_DEBUG_NAME(src) _gmm_strcpy(src)
#define GMM_SET_DEBUG_NAME2(dest, src) _gmm_strcpy2(dest, src)
#else
#define GMM_SET_DEBUG_NAME(src)
#define GMM_SET_DEBUG_NAME2(dest, src)
#endif

/*! @} */

#endif

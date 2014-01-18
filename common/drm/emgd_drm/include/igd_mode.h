/*
 *-----------------------------------------------------------------------------
 * Filename: igd_mode.h
 * $Revision: 1.17 $
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_MODE_H_
#define _IGD_MODE_H_

/*!
 * @defgroup pixel_formats Pixel Format Definitions
 * @ingroup display_group
 *
 * Pixel Format Definitions used in FB info data structure and
 * throughout IGD API functions. Pixel formats are comprised
 * of a Depth and Colorspace component combined (OR'd) with
 * a unique number. Pixel formats and their components are
 * defined with the following defines.
 *
 * @{
 */

/*!
 * This is a just to make use of the Pixel format explicit. This could
 * be replaced with an ENUM in the future.
 */
typedef unsigned long igd_pf_t;

/*!
 * @name Masks
 * Masks that may be used to seperate portions of the pixel format
 * definitions.
 *
 * @note Pixel Formats are maintained such that
 * (pixel_format & IGD_PF_FMT_MASK) >> IGD_PF_FMT_SHIFT
 * will always provide a unique index that can be used in a
 * lookup table.
 * @{
 */
#define IGD_PF_MASK                             0x0000FFFF
#define IGD_PF_TYPE_MASK                        0x0000FF00
#define IGD_PF_DEPTH_MASK                       0x000000FF
#define IGD_PF_FMT_MASK                         0x00FF0000
#define IGD_PF_FMT_SHIFT                        16
/*! @} */

/*!
 * @name Depths
 * Pixel Format depths in Bits per pixel.
 * @{
 */
#define PF_DEPTH_1                              0x00000001
#define PF_DEPTH_2                              0x00000002
#define PF_DEPTH_4                              0x00000004
#define PF_DEPTH_8                              0x00000008
#define PF_DEPTH_12                             0x0000000c
#define PF_DEPTH_16                             0x00000010
#define PF_DEPTH_24                             0x00000018
#define PF_DEPTH_32                             0x00000020
#define PF_DEPTH_64                             0x00000040
/*! @} */

/* Unknown Pixel Format */
#define IGD_PF_UNKNOWN							0x00000000

/*!
 * @name Colorspace
 * Colorspace components of the overall pixel format definition.
 * Several types may be OR'd together.
 * To check for equivalent types use  IGD_PF_TYPE(pf) == TYPE
 *
 * @{
 */
#define PF_TYPE_ALPHA                           0x00000100
#define PF_TYPE_RGB                             0x00000200
#define PF_TYPE_YUV                             0x00000400
#define PF_TYPE_PLANAR                          0x00000800
#define PF_TYPE_OTHER                           0x00001000
#define PF_TYPE_RGB_XOR                         0x00002000 /* Cursor */
#define PF_TYPE_COMP                            0x00004000 /* Compressed */

#define PF_TYPE_ARGB (PF_TYPE_ALPHA | PF_TYPE_RGB)
#define PF_TYPE_YUV_PACKED (PF_TYPE_YUV)
#define PF_TYPE_YUV_PLANAR (PF_TYPE_YUV | PF_TYPE_PLANAR)

/*! @} */

/*!
 * @name 8 Bit RGB Pixel Formats
 * @note Depth is bits per index (8)
 * @{
*/
#define IGD_PF_ARGB8_INDEXED     (PF_DEPTH_8  | PF_TYPE_ARGB | 0x00010000)
#define IGD_PF_ARGB4_INDEXED     (PF_DEPTH_4  | PF_TYPE_ARGB | 0x00020000)
/*! @} */

/*!
 * @name 16 Bit RGB Pixel Formats
 * @note Depth is bits per pixel (16)
 * @{
 */
#define IGD_PF_ARGB16_4444       (PF_DEPTH_16 | PF_TYPE_ARGB | 0x00030000)
#define IGD_PF_ARGB16_1555       (PF_DEPTH_16 | PF_TYPE_ARGB | 0x00040000)
#define IGD_PF_RGB16_565         (PF_DEPTH_16 | PF_TYPE_RGB  | 0x00050000)
#define IGD_PF_xRGB16_555        (PF_DEPTH_16 | PF_TYPE_RGB  | 0x00060000)
/*! @} */

/*!
 * @name 24 Bit RGB Pixel Formats
 * @note Depth is bits per pixel (24)
 * @{
 */
#define IGD_PF_RGB24             (PF_DEPTH_24 | PF_TYPE_RGB  | 0x00070000)
/*! @} */

/*!
 * @name 32 Bit RGB Pixel Formats
 * @note Depth is bits per pixel including unused bits (32).
 * @{
 */
#define IGD_PF_xRGB32_8888       (PF_DEPTH_32 | PF_TYPE_RGB  | 0x00080000)
#define IGD_PF_xRGB32            IGD_PF_xRGB32_8888
#define IGD_PF_ARGB32            (PF_DEPTH_32 | PF_TYPE_ARGB | 0x00090000)
#define IGD_PF_ARGB32_8888       IGD_PF_ARGB32
#define IGD_PF_xBGR32_8888       (PF_DEPTH_32 | PF_TYPE_RGB  | 0x000a0000)
#define IGD_PF_xBGR32            IGD_PF_xBGR32_8888
#define IGD_PF_ABGR32            (PF_DEPTH_32 | PF_TYPE_ARGB | 0x000b0000)
#define IGD_PF_ABGR32_8888       IGD_PF_ABGR32
/*! @} */

/*!
 * @name YUV Packed Pixel Formats
 * @note Depth is Effective bits per pixel
 *
 * @{
 */
#define IGD_PF_YUV422_PACKED_YUY2 (PF_DEPTH_16| PF_TYPE_YUV_PACKED| 0x000c0000)
#define IGD_PF_YUV422_PACKED_YVYU (PF_DEPTH_16| PF_TYPE_YUV_PACKED| 0x000d0000)
#define IGD_PF_YUV422_PACKED_UYVY (PF_DEPTH_16| PF_TYPE_YUV_PACKED| 0x000e0000)
#define IGD_PF_YUV422_PACKED_VYUY (PF_DEPTH_16| PF_TYPE_YUV_PACKED| 0x000f0000)
#define IGD_PF_YUV411_PACKED_Y41P (PF_DEPTH_12| PF_TYPE_YUV_PACKED| 0x00100000)
#define IGD_PF_YUV444_PACKED_AYUV (PF_DEPTH_32| PF_TYPE_YUV_PACKED| 0x00340000)
/*! @} */

/*!
 * @name YUV Planar Pixel Formats
 * @note Depth is Y plane bits per pixel
 * @{
 */
#define IGD_PF_YUV420_PLANAR_I420 (PF_DEPTH_8| PF_TYPE_YUV_PLANAR| 0x00110000)
#define IGD_PF_YUV420_PLANAR_IYUV IGD_PF_YUV420_PLANAR_I420
#define IGD_PF_YUV420_PLANAR_YV12 (PF_DEPTH_8| PF_TYPE_YUV_PLANAR| 0x00120000)
#define IGD_PF_YUV410_PLANAR_YVU9 (PF_DEPTH_8| PF_TYPE_YUV_PLANAR| 0x00130000)
#define IGD_PF_YUV420_PLANAR_NV12 (PF_DEPTH_8| PF_TYPE_YUV_PLANAR| 0x00140000)
/*! @} */

/*!
 * @name Cursor Pixel Formats
 * @{
 *
 * RGB_XOR_2: Palette = {Color 0, Color 1, Transparent, Invert Dest }
 * RGB_T_2: Palette = {Color 0, Color 1, Transparent, Color 2 }
 * RGB_2: Palette = {Color 0, Color 1, Color 2, Color 3 }
 */
#define IGD_PF_RGB_XOR_2      (PF_DEPTH_2  | PF_TYPE_RGB_XOR | 0x00150000)
#define IGD_PF_RGB_T_2        (PF_DEPTH_2  | PF_TYPE_RGB     | 0x00160000)
#define IGD_PF_RGB_2          (PF_DEPTH_2  | PF_TYPE_RGB     | 0x00170000)
/*! @} */

/*!
 * @name Bump Pixel Formats
 * @note Depth is bits per pixel
 * @{
 */
#define IGD_PF_DVDU_88             (PF_DEPTH_16 | PF_TYPE_OTHER | 0x00180000)
#define IGD_PF_LDVDU_655           (PF_DEPTH_16 | PF_TYPE_OTHER | 0x00190000)
#define IGD_PF_xLDVDU_8888         (PF_DEPTH_32 | PF_TYPE_OTHER | 0x001a0000)
/*! @} */

/*!
 * @name Compressed Pixel Formats
 * @note Depth is effective bits per pixel
 * @{
 */
#define IGD_PF_DXT1               (PF_DEPTH_4 | PF_TYPE_COMP | 0x001b0000)
#define IGD_PF_DXT2               (PF_DEPTH_8 | PF_TYPE_COMP | 0x001c0000)
#define IGD_PF_DXT3               (PF_DEPTH_8 | PF_TYPE_COMP | 0x001d0000)
#define IGD_PF_DXT4               (PF_DEPTH_8 | PF_TYPE_COMP | 0x001e0000)
#define IGD_PF_DXT5               (PF_DEPTH_8 | PF_TYPE_COMP | 0x001f0000)
/*! @} */

/*!
 * @name Depth Buffer Formats
 * @note Depth is bits per pixel
 * @{
 */
#define IGD_PF_Z16               (PF_DEPTH_16 | PF_TYPE_OTHER | 0x00200000)
#define IGD_PF_Z24               (PF_DEPTH_32 | PF_TYPE_OTHER | 0x00210000)
#define IGD_PF_S8Z24             (PF_DEPTH_32 | PF_TYPE_OTHER | 0x00220000)
/*! @} */

/*!
 * @name Other Pixel Formats
 *
 * - I8 8bit value replicated to all color channels.
 * - L8 8bit value replicated to RGB color channels, Alpha is 1.0.
 * - A8 8bit Alpha with RGB = 0.
 * - AL88 8bit Alpha, 8bit color replicated to RGB channels.
 * - AI44 4bit alpha 4bit palette color.
 * - IA44 4bit alpha 4bit palette color.
 * @{
 */
#define IGD_PF_I8               (PF_DEPTH_8 | PF_TYPE_OTHER | 0x00230000)
#define IGD_PF_L8               (PF_DEPTH_8 | PF_TYPE_OTHER | 0x00240000)
#define IGD_PF_A8               (PF_DEPTH_8 | PF_TYPE_ALPHA | 0x00250000)
#define IGD_PF_AL88             (PF_DEPTH_16 | PF_TYPE_ALPHA | PF_TYPE_OTHER | 0x00260000)
#define IGD_PF_AI44             (PF_DEPTH_8 | PF_TYPE_OTHER | 0x00270000)
#define IGD_PF_IA44             (PF_DEPTH_8 | PF_TYPE_OTHER | 0x00280000)

#define IGD_PF_L16              (PF_DEPTH_16 | PF_TYPE_OTHER | 0x00290000)
#define IGD_PF_ARGB32_2101010   (PF_DEPTH_32 | PF_TYPE_ARGB  | 0x002a0000)
#define IGD_PF_AWVU32_2101010	(PF_DEPTH_32 | PF_TYPE_OTHER | 0x002b0000)
#define IGD_PF_QWVU32_8888		(PF_DEPTH_32 | PF_TYPE_OTHER | 0x002c0000)
#define IGD_PF_GR32_1616        (PF_DEPTH_32 | PF_TYPE_OTHER | 0x002d0000)
#define IGD_PF_VU32_1616        (PF_DEPTH_32 | PF_TYPE_OTHER | 0x002e0000)

/*!
 * @name Floating-Point formats
 *
 * - R16F Floating-point format 16-bits for the Red Channel
 * - GR32_1616F 32-bit float format using 16 bits for red and green channel
 * - R32F IEEE Floating-Point s23e8 32-bits for the Red Channel
 * - ABGR32_16161616F - This is a 64-bit FP format.
 * - Z32F 32-bit float 3D depth buffer format.
 * @{
 */
#define IGD_PF_R16F                 (PF_DEPTH_16 | PF_TYPE_OTHER | 0x002f0000)
#define IGD_PF_GR32_1616F           (PF_DEPTH_32 | PF_TYPE_OTHER | 0x00300000)
#define IGD_PF_R32F                 (PF_DEPTH_32 | PF_TYPE_OTHER | 0x00310000)
#define IGD_PF_ABGR64_16161616F     (PF_DEPTH_64 | PF_TYPE_ARGB  | 0x00320000)
#define IGD_PF_Z32F                 (PF_DEPTH_32 | PF_TYPE_OTHER | 0x00330000)
/*! @} */

/*!
 * @name IGD_PF_NEXT array length helper
 *
 * This helper should always be set to the next available PF id. In this
 * manner any array that uses the id as an index can be defined as
 * unsigned long lookup_table[IGD_PF_NEXT] = {...}; and will then generate
 * compile warnings if the pixel format list length changes.
 */
#define IGD_PF_NEXT 0x35
/*!
 * @name Helper Macros
 * @{
 */

/*! Gets bits per pixel from pixel format */
#define IGD_PF_BPP(pf)   ((pf) & IGD_PF_DEPTH_MASK)
/*! Gets bits per pixel from pixel format */
#define IGD_PF_DEPTH(pf) IGD_PF_BPP(pf)
/*! Gets Bytes per pixel from pixel format */
#define IGD_PF_BYPP(pf)  ((IGD_PF_DEPTH(pf)+0x7)>>3)
/*! Gets Bytes required for line of pixels */
#define IGD_PF_PIXEL_BYTES(pf, np)  (((IGD_PF_BPP(pf)*np) +0x7)>>3)
/*! Gets numeric pf */
#define IGD_PF_NUM(pf) ((pf>>16) & 0xff)
/*! Gets pf type */
#define IGD_PF_TYPE(pf)  (pf & IGD_PF_TYPE_MASK)

/*! @} */
/*! @} */


/*
 * NOTE: When Adding pixel formats you must add correct definitions to
 * any pixel format lookup tables. See igd_2d.c
 */


/*!
 * @addtogroup display_group
 *
 * <B>Relavent Dispatch Functions</B>
 * - _igd_dispatch::query_dc()
 * - _igd_dispatch::query_mode_list()
 * - _igd_dispatch::free_mode_list()
 * - _igd_dispatch::alter_displays()
 *
 * @{
 */

/*!
 * The opaque display handle, returned from igd_alloc_display().
 * A display handle is needed for each physical display device.
 */
typedef void* igd_display_h;

/*!
 * The opaque timing_info handle.
 */
typedef void* igd_timing_info_h;

/*!
 * The maximum number of displays available in the display configurations
 * below.
 */
#define IGD_MAX_DISPLAYS 2

/*!
 * @brief An opaque driver handle.
 *
 * This is an opaque driver handle. It is returned during driver init
 * and remains usable for the life of the driver.
 */
typedef void* igd_driver_h;

/*!
 * @name Display Configuration Definition
 * @anchor dc_defines
 *
 * The display configuration (dc) is a unique 32bit identifier that fully
 * describes all displays in use and how they are attached to planes and
 * pipes to form Single, Clone, Twin and Extended display setups.
 *
 * The DC is treated as 8 nibbles of information (nibble = 4 bits). Each
 * nibble position in the 32bit DC corresponds to a specific role as
 * follows:
 *
<PRE>
    0x12345678
      ||||||||-- Legacy Display Configuration (Single, Twin, Clone, Ext)
      |||||||--- Port Number for Primary Pipe Master
      ||||||---- Port Number for Primary Pipe Twin 1
      |||||----- Port Number for Primary Pipe Twin 2
      ||||------ Port Number for Primary Pipe Twin 3
      |||------- Port Number for Secondary Pipe Master
      ||-------- Port Number for Secondary Pipe Twin 1
      |--------- Port Number for Secondary Pipe Twin 2
</PRE>
 *
 * The legacy Display Configuration determines if the display is in Single
 * Twin, Clone or extended mode using the following defines. When a complex
 * (>2 displays) setup is defined, the legacy configuration will describe
 * only a portion of the complete system.
 *
 * - IGD_DISPLAY_CONFIG_SINGLE: A single primary display only.
 * - IGD_DISPLAY_CONFIG_CLONE: Two (or more) displays making use of two
 *    display pipes. In this configuration two sets of display timings are
 *    used with a single source data plane.
 * - IGD_DISPLAY_CONFIG_TWIN: Two (or more) displays using a single display
 *    pipe. In this configuration a single set of display timings are
 *    used for multiple displays.
 * - IGD_DISPLAY_CONFIG_VEXT: Two (or more) displays making use of two
 *    display pipes and two display planes, however only one contiguous
 *    Frame buffer has been allocated that spans bothe displays vertically.
 *    In this configuration two sets of display timings are used and two
 *    source data planes.
 * - IGD_DISPLAY_CONFIG_EXTENDED: Two (or more) displays making use of two
 *    display pipes and two display planes. In this configuration two sets
 *    of display timings are used and two source data planes.
 *
 * @{
 */
#define IGD_DISPLAY_CONFIG_SINGLE   0x1
#define IGD_DISPLAY_CONFIG_CLONE    0x2
#define IGD_DISPLAY_CONFIG_TWIN     0x4
#define IGD_DISPLAY_CONFIG_VEXT	    0x5
#define IGD_DISPLAY_CONFIG_EXTENDED 0x8
#define IGD_DISPLAY_CONFIG_MASK     0xf
/*! @} */

/*!
 * @name IGD_DC_PORT_NUMBER
 *
 * Given a display configuration value and an index, return the port
 * number at that position.
 */
#define IGD_DC_PORT_NUMBER(dc, i) (unsigned short) ((dc >> (i * 4)) & 0x0f)
#define IGD_DC_PRIMARY(dc) (IGD_DC_PORT_NUMBER(dc,1))
#define IGD_DC_SECONDARY(dc) (IGD_DC_PORT_NUMBER(dc,5))


/*!
 * @name IGD_DC_SINGLE
 * For a given dc, return true if it is in single display mode
 */
#define IGD_DC_SINGLE(dc)   ((dc & 0xf) == IGD_DISPLAY_CONFIG_SINGLE)
/*!
 * @name IGD_DC_TWIN
 * For a given dc, return true if it is in twin display mode
 */
#define IGD_DC_TWIN(dc)     ((dc & 0xf) == IGD_DISPLAY_CONFIG_TWIN)
/*!
 * @name IGD_DC_CLONE
 * For a given dc, return true if it is in clone display mode
 */
#define IGD_DC_CLONE(dc)    ((dc & 0xf) == IGD_DISPLAY_CONFIG_CLONE)
/*!
 * @name IGD_DC_VEXT
 * For a given dc, return true if it is in vertically extended display mode
 */
#define IGD_DC_VEXT(dc) ((dc & 0xf) == IGD_DISPLAY_CONFIG_VEXT)
/*!
 * @name IGD_DC_EXTENDED
 * For a given dc, return true if it is in extended display mode
 */
#define IGD_DC_EXTENDED(dc) ((dc & 0xf) == IGD_DISPLAY_CONFIG_EXTENDED)

/*!
 * @name Query DC Flags
 * @anchor query_dc_flags
 *
 *   - IGD_QUERY_DC_ALL: Query All usable DCs.
 *   - IGD_QUERY_DC_PREFERRED: Returns only preferred dc's from the list
 *      of all usable dc's. Twin mode dc's are eliminated if an equivalent
 *      Clone dc is available. Only the most useful/flexible combination
 *      of 3 displays is returned rather than all combinations of the 3
 *      displays.
 *   - IGD_QUERY_DC_INIT: Returns a pointer to the initial dc to be used.
 *   - IGD_QUERY_DC_EXISTING: Returns a pointer to the initial dc that
 *      most closely matches the DC in use by the hardware. This can be
 *      used by an IAL to "discover" what the vBIOS has arranged and use
 *      that configuration. (NOT IMPLEMENTED)
 * @{
 */
#define IGD_QUERY_DC_ALL        0x0
#define IGD_QUERY_DC_PREFERRED  0x1
#define IGD_QUERY_DC_INIT       0x2
#define IGD_QUERY_DC_EXISTING   0x3
/*! @} */


/*!
 * @name DC index Flags
 * @anchor dc_index_flags
 *
 *  - IGD_DC_IDX_PRIMARY_MASTER: index to the primary display pipe master
 *  - IGD_DC_IDX_PRIMARY_TWIN1: index to the first primary display pipe's twin
 *  - IGD_DC_IDX_PRIMARY_TWIN2: index to the second primary display pipe's twin
 *  - IGD_DC_IDX_PRIMARY_TWIN3: index to the third primary display pipe's twin
 *  - IGD_DC_IDX_SECONDARY_MASTER: index to the secondary display pipe master
 *  - IGD_DC_IDX_SECONDARY_TWIN1: index to the first secondary display pipe's
 *     twin
 *  - IGD_DC_IDX_SECONDARY_TWIN2: index to the second secondary display pipe's
 *     twin
 * @{
 */
#define IGD_DC_IDX_PRIMARY_MASTER        0x1
#define IGD_DC_IDX_PRIMARY_TWIN1         0x2
#define IGD_DC_IDX_PRIMARY_TWIN2         0x3
#define IGD_DC_IDX_PRIMARY_TWIN3         0x4
#define IGD_DC_IDX_SECONDARY_MASTER      0x5
#define IGD_DC_IDX_SECONDARY_TWIN1       0x6
#define IGD_DC_IDX_SECONDARY_TWIN2       0x7
/*! @} */


/*!
 * @name Framebuffer Info Flags
 * @anchor fb_info_flags
 *
 * These bits may be set in the flags member of the _igd_framebuffer_info
 * data structure to alter the HALs behavior when calling
 * dispatch::alter_displays() All other flag bits must be 0 when calling.
 *
 *  - IGD_VBIOS_FB: The framebuffer info is not populated because the mode
 *     is being set with a VGA/VESA mode number. The HAL should instead
 *     return the framebuffer information to the caller when the mode
 *     has been set.
 *  - IGD_ENABLE_DISPLAY_GAMMA: The framebuffer data will be color corrected
 *     by using the 3*256 entry palette lookup table. Red, Green, and Blue
 *     8 bit color values will be looked up and converted to another value
 *     during display. The pixels are not modified. The caller must also
 *     set the palette using _igd_dispatch::set_palette_entries()
 *  - IGD_REUSE_FB: The HAL will use the incoming framebuffer info as the
 *     framebuffer. The HAL will allocate a framebuffer only if
 *     frambuffer_info.allocated == 0 and this is the first use of the
 *     display. If the offset has been changed, the caller is now responsible
 *     for freeing the previous framebuffer surface, and the HAL now fully
 *     owns the incoming surface, freeing it on shutdown etc.
 *  - IGD_MIN_PITCH: The allocation will use the input value
 *      of pitch as the minimum acceptable pitch. This allows a caller
 *      to allocate a surface with "extra" room between the width and
 *      pitch so that the width may later be expanded (use with caution).
 *
 * Additionally all surface flags will be populated by the HAL and
 * returned.
 * See: @ref surface_info_flags
 *
 * @{
 */
#define IGD_VBIOS_FB                            0x80000000
#define IGD_ENABLE_DISPLAY_GAMMA				0x40000000
#define IGD_REUSE_FB                            0x20000000
#define IGD_MIN_PITCH                           0x10000000
#define IGD_FB_FLAGS_MASK                       0xf0000000
/* @} */

/*
 * Right now, these flags are only used by igd_alter_displays when calling into
 * the 3rd party dc code to indicate which flip-chains to
 * disable.
 */
#define IGD_DISPLAY_PRIMARY                     0x1
#define IGD_DISPLAY_SECONDARY 					0x2
#define IGD_DISPLAY_ALL                         (IGD_DISPLAY_PRIMARY | IGD_DISPLAY_SECONDARY)

/*!
 * @brief Framebuffer related mode setting data.
 *  See _igd_dispatch::alter_displays()
 *
 * Data structure that contains details relavent to the framebuffer
 * configuration. Used with igd_alter_display() to alter the framebuffer
 * size and format.
 *
 * The framebuffer may be larger or smaller than the display timings. When
 * larger the output will be cropped and can be used in a panning
 * configuration. When smaller the output will be centered.
 *
 */
typedef struct _igd_framebuffer_info {
	unsigned int width;           /*!< @brief width of fb */
	unsigned int height;          /*!< @brief height of fb */
	/*!
	 * @brief Output pitch in bytes of the fb
	 *
	 * The pitch is the number of bytes from the first pixel of a line to
	 * the first pixel of the next line. This can, and usually will, be
	 * larger than width * depth for alignment purposes.
	 */
	unsigned int screen_pitch;
	unsigned long fb_base_offset; /*!< @brief memory location of the fb */
	/*!< @brief Memory location of the currently visible buffer
	 *
	 * This is what is being displayed on the port, which may be another buffer
	 * (e.g. a PVR services swap-chain buffer) and not the frame buffer.
	 */
	unsigned long visible_offset;
	
	/* this is the offset that will be restored when swithcing back to dih mode from
	 * dihclone mode and also when unlock_plane is called.
	 */

	unsigned long saved_offset;
	unsigned int lock;
	/*!
	 * @brief pixel format of the fb. See @ref pixel_formats
	 *
	 * This member is an input to the _igd_dispatch::alter_displays()
	 * function and must therefore be set to a pixel format that the
	 * hardware is capable of displaying as a framebuffer. The list
	 * of framebuffer pixel formats that are available can be queried
	 * by calling igd_dispatch::get_pixelformats()
	 */
	unsigned long pixel_format;
	unsigned long flags;          /*!< @brief See: @ref fb_info_flags */
	unsigned int allocated;       /*!< @brief 1 when allocated, 0 otherwise */
} igd_framebuffer_info_t, *pigd_framebuffer_info_t;


/*!
 * @name Display Info Flags
 *
 * Flags used with Display Info data structure to control boolean information.
 *
 * @note These flags are identical to the flags used with the port driver
 *  SDK. If any of these flags change, the corresponding flag in pd.h must
 *  also change.
 *
 * @{
 */

/* Enable/Disable Display  */
#define IGD_DISPLAY_DISABLE       0x00000000
#define IGD_DISPLAY_ENABLE        0x00000001

#define IGD_SCAN_MASK             0x80000000
#define IGD_SCAN_PROGRESSIVE      0x00000000
#define IGD_SCAN_INTERLACE        0x80000000

#define IGD_PIX_LINE_DOUBLE_MASK  0x60000000
#define IGD_DOUBLE_LINE_AND_PIXEL 0x60000000
#define IGD_LINE_DOUBLE           0x40000000
#define IGD_PIXEL_DOUBLE          0x20000000
#define IGD_MODE_TEXT             0x10000000

#define IGD_VSYNC_HIGH            0x08000000
#define IGD_HSYNC_HIGH            0x04000000
#define IGD_BLANK_LOW             0x02000000
#define IGD_MODE_VESA             0x01000000 /* VGA/VESA mode number is valid*/

#define IGD_MODE_STALL            0x00800000  /* Flag to enable stall signal */
#define IGD_MODE_SCALE            0x00400000  /* Request NATIVE pipe timings */

#define IGD_ASPECT_16_9           0x00200000  /* 16:9 aspect ratio */

#define IGD_MODE_DTD              0x00080000   /* Read from EDID */
#define IGD_MODE_DTD_USER         0x00040000   /* User defined timing */
#define IGD_MODE_DTD_FP_NATIVE    0x00020000   /* Native fp timing */
#define IGD_MODE_SUPPORTED        0x00010000

#define IGD_MODE_FACTORY          0x00008000   /* Factory supported mode */
#define IGD_MODE_RB               0x00004000   /* Reduced blanking mode */

/*
 * Port type definitions to be used in display_info flags.
 * Also used in port_type field of internal port data structure.
 */
#define IGD_PORT_MASK             0x000FF000
#define IGD_PORT_ANY              0x000FF000
#define IGD_PORT_ANALOG           0x00001000
#define IGD_PORT_DIGITAL          0x00002000
#define IGD_PORT_SDVO             IGD_PORT_DIGITAL
#define IGD_PORT_LVDS             0x00004000
#define IGD_PORT_RGBA             0x00008000 /* Enhanced DVO (Sunizona) */
#define IGD_PORT_TV               0x00010000 /* Integrated TV (Alviso)  */
#define IGD_PORT_SDVO_ST          0x00020000
#define IGD_PORT_SDVO_ST_GPIO     0x00040000


/* Atom E6xx requires LPC device, define a port type to differentiate
 * with other devices such as 0:2:0, 0:3:0 */
#define IGD_PORT_LPC              0x80000000   /* LPC device 0:31:0 */

/*
 * Standard port definitions
 */
#define IGD_PORT_TYPE_TV          0x1
#define IGD_PORT_TYPE_SDVOB       0x2
#define IGD_PORT_TYPE_SDVOC       0x3
#define IGD_PORT_TYPE_LVDS        0x4
#define IGD_PORT_TYPE_ANALOG      0x5

/*! Max ports
 *  This is the maximum number of ports currently defined to encompass all
 *  hardware configurations. This should always match the largest defined
 *  port number in the port tables. Currently the largest port number is 5
 *  for the analog port.  If a larger port number is defined, then this
 *  number must be increased.
 */
#define IGD_MAX_PORTS             5


/*! @} */

/*!
 * @brief Display timing information for use with
 *   _igd_dispatch::alter_displays()
 */
typedef struct _igd_display_info {
	unsigned short width;
	unsigned short height;
	unsigned short refresh;
	unsigned long dclk;              /* in KHz */
	unsigned short htotal;
	unsigned short hblank_start;
	unsigned short hblank_end;
	unsigned short hsync_start;
	unsigned short hsync_end;
	unsigned short vtotal;
	unsigned short vblank_start;
	unsigned short vblank_end;
	unsigned short vsync_start;
	unsigned short vsync_end;
	short mode_number;
	unsigned long flags;
	unsigned short x_offset;
	unsigned short y_offset;
	/* unsigned short port_number; */
	/* void *pd_private_ptr; */  /* Pointer for use by the PD for any purpose. */
	void *private_ptr;  /* INTERNAL pointer for use by main driver only */
	unsigned short reserved_dd;     /* Reserved for device dependant layer */
	unsigned short reserved_dd_ext; /* Reserved for device dependant layer */
} igd_display_info_t, *pigd_display_info_t;


/*!
 * @name Query Modes Flags (deprecated)
 * @deprecated To be removed in IEGD 5.1
 *
 * Flags for use in igd_query_display function call
 *
 * @{
 */
#define IGD_QUERY_ALL_MODES   0x01
#define IGD_QUERY_PORT_MODES  0x00
/*! @} */


/*!
 * @name Query Mode List Flags
 * @anchor query_mode_list_flags
 *
 * Flags for use with _igd_dispatch::query_mode_list()
 *  - IGD_QUERY_PRIMARY_MODES: Query mode list for the primary pipe.
 *  - IGD_QUERY_SECONDARY_MODES: Query modes list for the secondary pipe.
 *  - IGD_QUERY_LIVE_MODES: Query the live modes instead of filtering
 *      and allocing a new list. This may be ORed with other bits.
 * @{
 */
#define IGD_QUERY_PRIMARY_MODES   0x0
#define IGD_QUERY_SECONDARY_MODES 0x1
#define IGD_QUERY_LIVE_MODES      0x2
/*! @} */


/*!
 * @name timing table end of list marker
 *
 * This value is used to mark the end of a timing list (igd_display_info_t).
 * It is stored in the width field.  This must match the value for
 * PD_TIMING_LIST_END in src/include/pd.h
 *
 * @{
 */
#define IGD_TIMING_TABLE_END     0xffff
/*! @} */

/*!
 * @name Alter Displays Flags
 * @anchor alter_displays_flags
 *
 * These flags are passed to _igd_dispatch::alter_displays() and are
 * defined as follows:
 * - IGD_TEST When set, this flag indicates that the mode should only be
 *    tested. The hardware will not be modified.
 * - IGD_FORCE_ALTER When set the hardware will be modified even if the
 *    mode is the same as the current mode.
 * - IGD_CLEAR_FB Clear the framebuffer to black after the mode set.
 *
 * These flags control which ring buffers are allocated for the display.
 * These ring buffers are used with all rendering functions and designate
 * the target for the rendering commands.
 *
 * - IGD_ALLOC_PRIORITY_NORMAL Command slot used for normal commands
 * - IGD_ALLOC_PRIORITY_INTERRUPT Command slot with highest priority, often
 *    used to send commands during interrupt time. This command slot is
 *    not fully functional.
 * - IGD_ALLOC_PRIORITY_POWER In theory this command slot could send power
 *    events with a higher priority than a regular command slot. In
 *    practice it is unused.
 * - IGD_ALLOC_PRIORITY_BIN Binner ring buffer. This command slot requires
 *    knowledge of how the hardware binner works.
 *
 * @bug The Priority Flags overlap with other alter_displays flags and
 *  need to be relocated for IEGD 5.1
 *
 * @{
 */
#define IGD_TEST                     0x08000000
#define IGD_FORCE_ALTER              0x04000000
#define IGD_CLEAR_FB                 0x02000000

#define IGD_ALLOC_PRIORITY_NORMAL    0x10000000
#define IGD_ALLOC_PRIORITY_INTERRUPT 0x20000000
#define IGD_ALLOC_PRIORITY_POWER     0x40000000
#define IGD_ALLOC_PRIORITY_BIN       0x80000000
/*! @} */

/*!
 * Display handle list and Display info list should be terminated with
 * this tag. List may contain nulls, they will be skipped.
 *
 * @note This value is -1 so it will work the same on 32 and 64 bit platforms
 */
#define IGD_LIST_END -1




/*!
 * @name Cursor Info Flags
 * @anchor cursor_info_flags
 *
 * Flags for use in the Cursor Info Structure
 *
 * @{
 */
#define IGD_CURSOR_ON              0x00000001
#define IGD_CURSOR_OFF             0x00000000
#define IGD_CURSOR_LOAD_ARGB_IMAGE 0x00000010
#define IGD_CURSOR_LOAD_XOR_IMAGE  0x00000020

#define IGD_CURSOR_GAMMA           0x00000002
#define IGD_CURSOR_LOCAL           0x00000004  /* Local Memory */
#define IGD_CLONE_CURSOR           0x00000001
/*! @} */

/*!
 * @brief Cursor Plane information
 *
 * This data structure is used as input and output to the
 * _igd_dispatch::alter_cursor() dispatch function.
 */
typedef struct _igd_cursor_info {
	unsigned long width;
	unsigned long height;
	unsigned long pixel_format;
	/*
	 * Use the XOR offset for all 2 bit formats, and the ARGB offset for
	 * 32bit formats. If either offset is 0 it should be assumed that the
	 * HAL cannot support it.
	 */
	unsigned long xor_offset;
	unsigned long xor_pitch;
	unsigned long argb_offset;
	unsigned long argb_pitch;
	long x_offset;
	long y_offset;
	long hot_x;
	long hot_y;
	unsigned long palette[4];
	unsigned long flags;
	unsigned long rotation;
	unsigned long render_scale_x;
	unsigned long render_scale_y;
	unsigned short off_screen;
} igd_cursor_info_t;

/*!
 * @name Get Scanline Defines
 * @anchor get_scanline_defs
 *
 * These defines are returned from the _igd_dispatch::get_scanline()
 * dispatch function in the place of scanline number when no number exists.
 *
 * @{
 */
#define IGD_IN_VBLANK -1
/*! @} */




/*!
 * @name Acess i2c Flags
 * @anchor access_i2c_flags
 *
 * These flags are used to control the flow of data in the
 * igd_dispatch::access_i2c() dispatch function.
 *
 * @{
 */
#define IGD_I2C_READ   0x00000001
#define IGD_I2C_WRITE  0x00000002
/*! @} */

/*!
 * @brief I2C data packet for use with _igd_dispatch::access_i2c()
 */
typedef struct _igd_i2c_reg {
	/*! @brief Serial bus id [0..6] for alm core, and 0 for whitney core */
	unsigned char bus_id;
	/*! @brief Data address byte of the device */
	unsigned char dab;
	/*! @brief Device register */
	unsigned char reg;
	/*! @brief Buffer for register values */
	unsigned char *buffer;
	/*! @brief number of bytes to read or write */
	unsigned char num_bytes;
	/*! @brief i2c bus speed in KHz */
	unsigned long i2c_speed;
} igd_i2c_reg_t;



typedef struct _igd_port_info {
	char pd_name[64];
	unsigned long driver_version;	/* Formatted as pd_version_t */
	unsigned long port_num;			/* Port Number */
	unsigned long display_type;		/* Type of display */
	int connected;					/* 1 - Display connected, 0 - Otherwise */
	char port_name[8];             /* Default port name (DVOA, sDVO-A, etc.) */
	unsigned long flags;           /* Attribute flags, currently used for interlace */
} igd_port_info_t;



typedef struct _kdrm_driver_set_sync_refresh {
        igd_display_h   display_handle;
        unsigned int    start_line;
        unsigned int    bottom_line;
} emgd_drm_driver_set_sync_refresh_t;



/*!
 * @name Alloc Display Flags (deprecated)
 *
 * Flags for use with the igd_alloc_display function call.
 * - IGD_NEW_ALL Default, request for new plane, pipe, port
 * - IGD_NEW_PIPE Request for an additional pipe & port, using the same plane
 *    from previous allocation
 * - IGD_NEW_PORT Request for an addition port, using the same pipe and plane
 *    from previous allocation
 *
 * @{
 */
#define IGD_NEW_MASK           0x00700000
#define IGD_NEW_ALL            0x00400000
#define IGD_NEW_PIPE           0x00200000
#define IGD_NEW_PORT           0x00100000
/*! @} */

typedef int (*_igd_query_modes_fn_t)(igd_display_h display_handle,
	igd_display_info_t **mode_list);
/*! @} */

#endif /* _IGD_MODE_H_ */

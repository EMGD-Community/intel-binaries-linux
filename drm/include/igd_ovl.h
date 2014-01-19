/*
 *-----------------------------------------------------------------------------
 * Filename: igd_ovl.h
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
 *  This is the external header file for overlay. It should be included
 *  by ial and/or other HAL modules that require overlay interaction.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_OVL_H
#define _IGD_OVL_H

/*!
 * @defgroup overlay_group Overlay
 * @ingroup video_group
 *
 * The overlay module is responsible for the hardware overlay and the secondary
 * overlay (sometimes referred to as plane C).
 *
 * <B>Relevent Dispatch Fucntions</B>
 * - _igd_dispatch::alter_ovl()
 * - _igd_dispatch::query_ovl()
 * - _igd_dispatch::query_max_size_ovl()
 * @{
 */

/*----------------------------------------------------------------------------
 * Overlay Alter Flags
 *--------------------------------------------------------------------------*/
/*!
 * @name Alter Overlay Flags
 * @anchor alter_ovl_flags
 *
 * Flags for use with _igd_dispatch::alter_ovl()
 * - IGD_OVL_ALTER_OFF: Turn the overlay off
 * - IGD_OVL_ALTER_ON: Turn the overlay on
 *
 * - IGD_OVL_ALTER_PROGRESSIVE: The overlay is progressive.  Only valid when
 *     the overlay is on
 * - IGD_OVL_ALTER_INTERLEAVED: The overlay is interleaved/bobbed.  Only valid
 *     when the overlay is on
 *
 * - IGD_OVL_ALTER_FLIP_EVEN: Flip the even field.  Only valid when the overlay
 *     is on
 * - IGD_OVL_ALTER_FLIP_ODD: Flip the odd field.  Only valid when the overlay
 *     is on
 * @{
 */
#define IGD_OVL_ALTER_OFF 0
#define IGD_OVL_ALTER_ON  1

#define IGD_OVL_ALTER_PROGRESSIVE   0
#define IGD_OVL_ALTER_INTERLEAVED   2

#define IGD_OVL_ALTER_FLIP_EVEN     0
#define IGD_OVL_ALTER_FLIP_ODD      4

#define IGD_FW_VIDEO_OFF	        8

#define IGD_OVL_FORCE_USE_DISP   0x10
#define IGD_OVL_OSD_ON_SPRITEC   0x20
#define IGD_OVL_GET_SURFACE_DATA 0x50

/* Note: VEXT: Keep this bitwise! */
#define IGD_OVL_ALTER_VEXT_PRIMARY			0x100
#define IGD_OVL_ALTER_VEXT_SECONDARY		0x200
/*! @} */

/* These are not actually used by any IAL
#define IGD_OVL_ALTER_MIRROR_NONE  0
#define IGD_OVL_ALTER_MIRROR_H     0x10
#define IGD_OVL_ALTER_MIRROR_V     0x20
#define IGD_OVL_ALTER_MIRROR_HV    (IGD_OVL_FLAGS_MIRROR_H|IGD_OVL_FLAGS_MIRROR_V) */

/*----------------------------------------------------------------------------
 * Overlay Query Flags
 *--------------------------------------------------------------------------*/
/*!
 * @name Query Overlay Flags
 * @anchor query_ovl_flags
 *
 * Flags for use with _igd_dispatch::query_ovl()
 * These flags ARE EXCLUSIVE - ONLY ONE AT A TIME can be used during a
 *    single call to igd_query_ovl
 *
 * - IGD_OVL_QUERY_IS_HW_SUPPORTED: Can the hardware support an overlay for
 *   this display_h?  This will return the same value no matter if the overlay
 *   is on or off, so the IAL must use some other method to determine if the
 *   overlay is in use.
 * - IGD_OVL_QUERY_IS_LAST_FLIP_DONE: Has the last flip completed?
 * - IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE: Wait until the last flip is complete.
 *   Returns TRUE if the last flip was successfully completed.  Returns FALSE
 *   if there was a HW issue where the last flip did not occur.
 * - IGD_OVL_QUERY_IS_ON: Is the hardware overlay currently on?  This does not
 *   include the secondary overlay, only the hardware overlay.
 * - IGD_OVL_QUERY_IS_GAMMA_SUPPORTED: Is the hardware supports GAMMA
 *   correction?
 * - IGD_OVL_QUERY_IS_VIDEO_PARAM_SUPPORTED: Is the hardware supports video
 *   parameters (brightness, contrast, saturation) correction?
 * @{
 */
#define IGD_OVL_QUERY_IS_HW_SUPPORTED                     0x00000001
#define IGD_OVL_QUERY_IS_LAST_FLIP_DONE                   0x00000002
#define IGD_OVL_QUERY_WAIT_LAST_FLIP_DONE                 0x00000003
#define IGD_OVL_QUERY_IS_ON                               0x00000004
#define IGD_OVL_QUERY_IS_GAMMA_SUPPORTED                  0x00000005
#define IGD_OVL_QUERY_IS_VIDEO_PARAM_SUPPORTED            0x00000006
#define IGD_OVL_QUERY_MASK                                0x0000000F
	/* Ensure no bits conflict with IGD_OVL_FORCE_USE_DISP above */
/*! @} */

/* This is now in igd_query_max_size_ovl
 * Get the maximum width/height for the src_surface pixel format
 * return: max width and height parameters; should always return TRUE.
#define IGD_OVL_QUERY_MAX_WIDTH_HEIGHT                    0x00000003
 */

/* This should not be needed.  Since the IAL is always passing in valid
 * gamma information, the IAL should not query the HAL for the information.
#define IGD_OVL_QUERY_COLOR_CORRECT_INFO                  0x00000005*/


/*----------------------------------------------------------------------------
 * Overlay Color Key
 *--------------------------------------------------------------------------*/
/*!
 * @name Overlay Color Key Flags
 * @anchor igd_ovl_color_key_info_flags
 *
 * Flags for use with @ref igd_ovl_color_key_info_t
 * Enables and disables the src and dest color key for the overlay
 *
 * IGD_OVL_DST_BLEND_ENABLE enabled blending the contents of the overlay
 * with the contents of the frambuffer using the alpha from the framebuffer.
 * (As if the framebuffer was on top of the overlay) This feature requires
 * that dest colorkey be enabled.
 *
 * @{
 */
#define IGD_OVL_SRC_COLOR_KEY_DISABLE		0x00000000
#define IGD_OVL_SRC_COLOR_KEY_ENABLE		0x00000001

#define IGD_OVL_DST_COLOR_KEY_DISABLE		0x00000000
#define IGD_OVL_DST_COLOR_KEY_ENABLE		0x00000002

#define IGD_OVL_DST_BLEND_ENABLE            0x00000004
/*! @} */

/*!
 * @brief Overlay Color Key
 *
 * The src_lo, src_hi, and dest color key are in the following pixel format
 * based on the pixel format of the src and dest surface.
 * Note: Alpha is always ignored.
 *
 * - xRGB32        = x[31:24] R[23:16] G[15:8] B[7:0]
 * - xRGB16_555    = x[31:15] R[14:10] G[9:5]  B[4:0]
 * - ARGB8_INDEXED = x[31:8] Index Color[7:0]
 * - YUV surf      = x[31:24] Y[23:16] U[15:8] V[7:0]
 *
 * If the source pixel (overlay) is within the src_lo and src_hi color key
 * (inclusive) for all color components, then the destination pixel is
 * displayed.  Otherwise the source pixel is displayed.
 *
 * If the source and dest color key are both enabled, what happens???
 */
typedef struct _igd_ovl_color_key_info {
	/*! Low end src color key value */
	unsigned long        src_lo;
	/*! High end src color key value */
	unsigned long        src_hi;
	/*! Dest color key value.  If the destination pixel matches the
	 *   dest color key then the souce pixel from the video surface is displayed.
	 *   Otherwise, the destination pixel is displayed */
	unsigned long        dest;
	/*! Enable and disable the src and dest color key.
	 *   See @ref igd_ovl_color_key_info_flags */
	unsigned long        flags;
} igd_ovl_color_key_info_t;

/*----------------------------------------------------------------------------
 * Overlay Video Quality
 *--------------------------------------------------------------------------*/
/*!
 * @brief Overlay Video Quality
 *
 * All have a range between 0x0000 and 0xFFFF with default = 0x8000
 */
typedef struct _igd_ovl_video_quality_info{
	unsigned short		contrast;
	unsigned short		brightness;
	unsigned short		saturation;
	unsigned short		hue;
} igd_ovl_video_quality_info_t;


/*----------------------------------------------------------------------------
 *  Overlay Gamma
 *--------------------------------------------------------------------------*/
/*!
 * @name Overlay Gamma Flags
 * @anchor igd_ovl_gamma_info_flags
 *
 * Flags for use with @ref igd_ovl_gamma_info_t.
 * Enables and disables the gamma for the overlay
 *
 * @{
 */
#define IGD_OVL_GAMMA_DISABLE               0x00000000
#define IGD_OVL_GAMMA_ENABLE				0x00000001
/* UM only flag to indicate individual 
 * plane color correction values should be applied */
#define IGD_OVL_GAMMA_OVERRIDE               0x00000002
/*! @} */

/*!
 * @name Overlay Gamma Min/Max
 * @anchor igd_ovl_gamma_info_range
 *
 * Minimum and maximum gamma values for red, green, and blue with
 * @ref igd_ovl_gamma_info_t.  These values are in 24i.8f format.
 * Minimum value is 0.6
 * Maximum value is 6.0
 *
 * @{
 */
#define IGD_OVL_GAMMA_DEFAULT   0x100  /* 1.0 */
#define IGD_OVL_GAMMA_MIN       0x09A  /* 0.6 */
#define IGD_OVL_GAMMA_MAX       0x600  /* 6.0 */
/*! @} */

/*!
 * @brief Overlay Gamma
 *
 * Red, green, and blue values must be between min and max values.
 * This value is in 24i.8f format.
 * See @ref igd_ovl_gamma_info_range
 */
typedef struct _igd_ovl_gamma_info{
	unsigned int		red;
	unsigned int		green;
	unsigned int		blue;
	/*! Enable and disable the gamma for the overlay.
	 *   See @ref igd_ovl_gamma_info_flags */
	unsigned int		flags;
} igd_ovl_gamma_info_t;
/*! @} */

/*----------------------------------------------------------------------------
 * Overlay Info
 *--------------------------------------------------------------------------*/
/*!
 * @brief Overlay Information (includes color key, video quality, and gamma)
 */
typedef struct _igd_ovl_info{
	igd_ovl_color_key_info_t			color_key;
	igd_ovl_video_quality_info_t		video_quality;
	igd_ovl_gamma_info_t				gamma;
} igd_ovl_info_t;

typedef struct _video_surface_data {
	int flags;
	void *file;
} vid_surf_data_t;

#define OVL_PRIMARY   0
#define OVL_SECONDARY 1
#define OVL_MAX_HW    2  /* Maximum number of overlays */

/* Define the maximum number of blend surfaces which can be used */
#define MAX_BLEND_SURF 3

/* User mode overlay context */
typedef struct _ovl_um_context {
	/* OVLADD add additional overlay initialization parameters here. */

	int in_dihclone;
	int vext;
	int chiptype;
	void *dispatch; /* can't make this igd_dispatch_t due to circular reference */
	unsigned long dc;
	unsigned int  state;

	igd_display_h primary;
	void *primary_pipe; /* intel_pipe_data_t */

	igd_display_h secondary;
	void *secondary_pipe; /*intel_pipe_data_t */

	unsigned int blend_surf_num[OVL_MAX_HW];
	igd_surface_t blend_surf[OVL_MAX_HW][MAX_BLEND_SURF];

	vid_surf_data_t *vid_surf_data_ptr;
	igd_display_h ovl_display_um[OVL_MAX_HW];
	igd_ovl_info_t ovl_info_um[OVL_MAX_HW];
	igd_display_h active_single_ovl;
} ovl_um_context_t;



#endif /*_IGD_OVL_H*/

#define WAIT_FOR_FLIP 1
#define FLIP_DONE 0

/*
 *-----------------------------------------------------------------------------
 * Filename: user_config.h
 * $Revision: 1.8 $
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
 *  Data structure containing the initial display configuration information of
 *  the EMGD kernel module.
 *-----------------------------------------------------------------------------
 */
#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#include "igd_init.h"
#include "igd_mode.h"

/*
 * Splash Screen data provided by the user.
 */
typedef struct _emgd_drm_splash_screen {
	unsigned long bg_color;
	unsigned long x;
	unsigned long y;
	unsigned long width;
	unsigned long height;
} emgd_drm_splash_screen_t;

/*
 * Splash Video data provided by the user.
 */
typedef struct _emgd_drm_splash_video {
	unsigned long offset;
	unsigned long pixel_format;
	unsigned long src_width;
	unsigned long src_height;
	unsigned long src_pitch;
	unsigned long dst_x;
	unsigned long dst_y;
	unsigned long dst_width;
	unsigned long dst_height;
} emgd_drm_splash_video_t;

/**
 * User-configurable parameters.  This structure is the basis for the
 * user_config.c" file, which allows compile-time customization of the EMGD DRM
 * module.
 *
 * Besides the igd_param_t values, the other options in this structure
 * correspond to EMGD module parameters of the same name.  Most are only
 * applicable if the init option is non-zero.  There is one additional module
 * parameter ("portorder") that corresponds to the port_order member of the
 * igd_param_t structure.
 */
typedef struct _emgd_drm_config {
	/**
	 * Whether the EMGD kernel/DRM module should initialize the display at
	 * startup time (1=yes, 0=no).
	 */
	int init;
	/** The display configuration to use if initializing the display. */
	int dc;
	/** The width to use if initializing the display. */
	int width;
	/** The height to use if initializing the display. */
	int height;
	/** The refresh rate to use if initializing the display. */
	int refresh;
	/** Enable Kernel Mode Set */
	int kms;
	/** Overlay Brightness */
	unsigned long ovl_brightness;
	/** Overlay Contrast */
	unsigned long ovl_contrast;
	/** Overlay Saturation */
	unsigned long ovl_saturation;
	/** Overlay Hue */
	unsigned long ovl_hue;
	/** Overlay Gamma Correction - Red */
	unsigned long ovl_gamma_red;
	/** Overlay Gamma Correction - Green */
	unsigned long ovl_gamma_green;
	/** Overlay Gamma Correction - Blue */
	unsigned long ovl_gamma_blue;
	/** The splash screen data if specified by the user. */
	emgd_drm_splash_screen_t *ss_data;
	/** The splash video data if specified by the user. */
	emgd_drm_splash_video_t *sv_data;
	/** Array of other parameters (one per configid), used by the hardware
	 * abstraction layer code.
	 */
	igd_param_t **hal_params;
	/** Enable V2G Camera Module **/
#ifdef SUPPORT_V2G_CAMERA	
	int v2g;
#endif	
} emgd_drm_config_t;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: config_default.h
 * $Revision: 1.16 $
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
 *  This file is used in conjunction with the platform's config.h to
 *  gererate a full set of build defines. This file should provide defaults
 *  for defines such that a platform's config.h can include only the
 *  minimal set of non-standard options.
 *  Defines should be named such that:
 *  CONFIG_<FOO>: Is defined or undefined suitable for use with ifdef and
 *  can be used with the build system's DIRS_FOO or OBJECTS_FOO. Any
 *  CONFIG_FOO added here must also have an entry in config_helper.c.
 *  CONFIG_ENABLE_FOO: Should be defined always and defined to a 1 or 0.
 *  This is suitble for use in if(CONFIG_ENABLE_FOO) and expected that
 *  a compiler will optimize away if(0)'s.
 *  CONFIG_LIMIT_FOO: Should prevent some default set of FOO defines
 *  from being included. For instance CONFIG_LIMIT_MODES prevents the
 *  long default list of default modes from being used and instead the
 *  platform's config.h must define the requested modes manually.
 *-----------------------------------------------------------------------------
 */

#ifndef _HAL_CONFIG_DEFAULT_H
#define _HAL_CONFIG_DEFAULT_H

#include <igd_version.h>

#ifndef CONFIG_MICRO
#define CONFIG_FULL
#endif

#ifndef CONFIG_LIMIT_CORES
#define CONFIG_PLB
#define CONFIG_TNC
#endif /* CONFIG_LIMIT_CORES */

#ifdef CONFIG_DEPRECATED
#if 0 /* WHT Modules need some updating */
#define CONFIG_810
#define CONFIG_810DC
#define CONFIG_810E
#define CONFIG_815
#define CONFIG_830
#define CONFIG_835
#define CONFIG_845
#define CONFIG_855
#define CONFIG_865
#define CONFIG_915GD
#define CONFIG_915AL
#define	CONFIG_945G
#define	CONFIG_945GM
#define	CONFIG_945GME
#define CONFIG_Q35
#define CONFIG_965G
#define CONFIG_965GM
#define CONFIG_CTG
#define CONFIG_Q45
#define CONFIG_PNV
#endif
#endif

#ifndef CONFIG_LIMIT_MODULES
#define CONFIG_INIT
#define CONFIG_REG
#define CONFIG_POWER
#define CONFIG_MODE
#define CONFIG_DSP
#define CONFIG_PI
#define CONFIG_PD
#define CONFIG_APPCONTEXT
#define CONFIG_OVERLAY
#endif /* CONFIG_LIMIT_MODULES */

#ifndef CONFIG_LIMIT_PDS
#define CONFIG_PD_ANALOG
#define CONFIG_PD_LVDS
#define CONFIG_PD_TV
#define CONFIG_PD_HDMI
#define CONFIG_PD_SDVO
#define CONFIG_PD_SOFTPD
#define CONFIG_PD_CH7036
#endif

#ifdef CONFIG_DEPRECATED
#ifndef CONFIG_LIMIT_PDS
#define CONFIG_PD_SII164
#define CONFIG_PD_CH7009
#define CONFIG_PD_TL955
#define CONFIG_PD_RGBA
#define CONFIG_PD_NS2501
#define CONFIG_PD_TH164
#define CONFIG_PD_FS454
#define CONFIG_PD_NS387
#define CONFIG_PD_CX873
#define CONFIG_PD_FS460
#define CONFIG_PD_CH7017
#define CONFIG_PD_TI410
#endif
#endif

#ifndef CONFIG_DEBUG_FLAGS
#define CONFIG_DEBUG_FLAGS			  \
	0, /* Command Module */			  \
		0,	/* DSP Module */		  \
		0,	/* Mode Module */		  \
		0,	/* Init Module */		  \
		0,	/* Overlay Module */	  \
		0,	/* Power Module */		  \
		0,	/* 2D Module */			  \
		0,	/* Blend Module */		  \
		0,	/* State Module */		  \
		0,	/* GMM Module */		  \
		0,	/* Gart Module */		  \
		0,	/* OAL Module */		  \
		0,	/* Interrupt Module */	  \
		0,	/* Port Driver Module */  \
		0,	/* Video Decode Module */ \
		0,	/* PVR 3-Ptr Disp Drv */  \
		0,	/* Buffer Class Module */ \
\
		0,	/* Global Tracing */	  \
		0,	/* Global Instructions */ \
		0,	/* Global Debug */		  \
\
		0,	/* Verbose Blend Stats */ \
		0,	/* Verbose Overlay Dump */\
		0,	/* Verbose Cmd Dump */	  \
		0,	/* Verbose GMM Dump */    \
		0	/* Verbose Shader Dump */

#endif

#ifndef CONFIG_DEBUG_IAL_FLAGS
#define CONFIG_DEBUG_IAL_FLAGS 0
#endif

/* we ensure IAL's that do not support hw binning has this flag as '0' */
#ifndef CONFIG_ENABLE_BINNING
#define CONFIG_ENABLE_BINNING 0
#endif

#ifndef CONFIG_ENABLE_THREADS
#define CONFIG_ENABLE_THREADS 0
#endif



/*
 * These Meta-Defines should not be set in the config.h. They are enabled
 * here based on more granular defines that come from config.h. For instance
 * CONFIG_NAP should be enabled when any chips from the NAP family are enabled.
 */
#if defined(CONFIG_810) || defined(CONFIG_810DC) || defined(CONFIG_810E) ||\
	defined(CONFIG_815)
#define CONFIG_WHT
#endif

#if defined(CONFIG_830) || defined(CONFIG_835) || defined(CONFIG_845) ||\
	defined(CONFIG_855) || defined(CONFIG_865)
#define CONFIG_ALM
#endif

#if defined(CONFIG_915GD) || defined(CONFIG_915AL) || defined(CONFIG_945G) ||\
	defined(CONFIG_945GM) || defined(CONFIG_945GME) || defined(CONFIG_Q35) ||\
	defined(CONFIG_PNV)
#define CONFIG_NAP
#endif

#if defined(CONFIG_965G) || defined(CONFIG_965GM) || \
	defined(CONFIG_CTG) || defined(CONFIG_Q45)
#define CONFIG_GN4
#endif


#endif /* _HAL_CONFIG_DEFAULT_H */


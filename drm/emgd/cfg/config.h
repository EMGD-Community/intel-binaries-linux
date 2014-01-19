/*
 *-----------------------------------------------------------------------------
 * Filename: config.h
 * $Revision: 1.14 $
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
 *  This file contains the compile options for the IGD compile. It is included
 *  by all IGD OAL and RAL modules. Do not remove valid options from this
 *  file, simply comment them out.
 *  Eventually a config tool will auto generate this file based on selected
 *  options.
 *-----------------------------------------------------------------------------
 */

#ifndef _HAL_CONFIG_H
#define _HAL_CONFIG_H

/*
 * Which Cores are supported
 *
 * Use Defaults
 */

/*
 * This macro configures the DRM/kernel's EMGD_DEBUG() and EMGD_DEBUG_S() macros to
 * use the KERN_INFO message priority, instead of the normal KERN_DEBUG message
 * priority.  This is useful for bugs (e.g. crashes) where dmesg can't be used
 * to obtain debug messages.
 */
/* #define CONFIG_USE_INFO_PRIORITY */


/*
 * Which of the optional modules are included in the build
 * for the most part this is for modules that need an init
 * or power entry point.
 *
 * Use Defaults.
 */

/*
 * Default FB/Display Resolution
 */
#define CONFIG_DEFAULT_WIDTH  640
#define CONFIG_DEFAULT_HEIGHT 480
#define CONFIG_DEFAULT_PF     IGD_PF_ARGB32


/*
  power modes supported
  0 -don't support
  1 - support

  Use Defaults.
*/

/*
 * Turn off fences for performance analysis. 3d makes use of "Use Fences"
 * So this will make fences regions become linear but everything should
 * still work.
 *
 * #define CONFIG_NOFENCES
 */

/* Don't enable Dynamic port driver loading for simple driver. For simple,
 * one can limit the port drivers by enabling CONFIG_LIMIT_PDS to
 * required port drivers *
 *
 * Enable Dynamic port driver loading
 *
 * #define IGD_DPD_ENABLED 1 */

/* Enable required port drivers. */
#define CONFIG_LIMIT_PDS 1
#define CONFIG_PD_ANALOG 0
#define CONFIG_PD_LVDS   1
#define CONFIG_PD_SDVO   1
#define CONFIG_PD_TV     0 /* Integrated TV for NAPA */
#define CONFIG_PD_CH7036  1

#define CONFIG_LINK_PD_LVDS
#define CONFIG_LINK_PD_SDVO
#define CONFIG_LINK_PD_CH7036

#define CONFIG_DECODE

#define CONFIG_ST

#include <config_default.h>

#endif


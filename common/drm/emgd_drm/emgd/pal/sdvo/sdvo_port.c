/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_port.c
 * $Revision: 1.31 $
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
 *  Port driver interface functions
 *-----------------------------------------------------------------------------
 */


#include <linux/kernel.h>


#include "sdvo_port.h"

#define PFIT_CONTROL		0x61230  /* Panel Fitting Control Register Address*/
#define PFIT_PIPEB_ENABLED	0xA0000000
#define CH7308_VENDOR_ID	2
#define CH7308_DEVICE_ID	0x41
#define CH7308_REVISION_ID	1

/* .......................................................................... */

int sdvo_open(pd_callback_t *p_callback, void **p_context);
int sdvo_init_device(void *p_context);
int sdvo_get_timing_list(void *p_context, pd_timing_t *p_in_list,
	pd_timing_t **pp_out_list);
int sdvo_set_mode(void *p_context, pd_timing_t *p_mode, unsigned long flags);
int sdvo_post_set_mode(void *p_context, pd_timing_t *p_mode,
	unsigned long flags);
int sdvo_get_attributes(void *p_context, unsigned long *p_num_attr,
	pd_attr_t **pp_list);
int sdvo_set_attributes(void *p_context, unsigned long num_attr,
	pd_attr_t *p_list);
unsigned long sdvo_validate(unsigned long cookie);
int sdvo_close(void *p_context);
int sdvo_set_power(void *p_context, unsigned long state);
int sdvo_get_power(void *p_context, unsigned long *p_state);
int sdvo_save(void *p_context, void **pp_state, unsigned long flags);
int sdvo_restore(void *p_context, void *p_state, unsigned long flags);
int sdvo_get_port_status(void *p_context, pd_port_status_t *port_status);
#ifndef CONFIG_MICRO
int sdvo_set_tv_settings(void *p_context, i2c_reg_t tv_format);
#endif
char *sdvo_get_output_type(sdvo_display_output_t out, unsigned char byte);
int sdvo_init_attribute_table(sdvo_device_context_t *p_ctx);
void sdvo_convert_pd_timing_to_dtd(pd_timing_t *p_mode, sdvo_dtd_t *p_dtd,
	unsigned long dclk);
void sdvo_convert_dtd_to_pd_timing(sdvo_dtd_t *p_dtd, pd_timing_t *p_mode);

#ifndef CONFIG_MICRO
static sdvo_status_t sdvo_reset_encoder(sdvo_device_context_t *p_ctx);
#endif

static pd_version_t  g_sdvo_version = {3, 4, 0, 0};
static unsigned long g_sdvo_dab_list[] = {0x70, 0x72, PD_DAB_LIST_END};

static pd_driver_t	 g_sdvo_drv = {
	PD_SDK_VERSION,
	"SDVO Port Driver",
	0,
	&g_sdvo_version,
	PD_DISPLAY_FP,
	PD_FLAG_CLOCK_MASTER,
	g_sdvo_dab_list,
	1000,
	sdvo_validate,
	sdvo_open,
	sdvo_init_device,
	sdvo_close,
	sdvo_set_mode,
	sdvo_post_set_mode,
	sdvo_set_attributes,
	sdvo_get_attributes,
	sdvo_get_timing_list,
	sdvo_set_power,
	sdvo_get_power,
	sdvo_save,
	sdvo_restore,
	sdvo_get_port_status
};

pd_timing_t sdvo_sdtv_timing_table[] =
{
#ifndef CONFIG_MICRO
#if 0
	/* byte 0 */
	{
		320, 200,              /* width, height */
		60, 19140,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		319, 1286,             /* hblank_start, hblank_end (will be set) */
		561, 593,              /* hsync_start, hsync_end (will be set) */
		249,                   /* vtotal */
		199, 249,              /* vblank_start, vblank_end (will be set) */
		216, 218,              /* vsync_start, vsync_end (will be set) */
		648,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		320, 240,              /* width, height */
		60, 22970,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		319, 1286,             /* hblank_start, hblank_end (will be set) */
		561, 593,              /* hsync_start, hsync_end (will be set) */
		299,                   /* vtotal */
		239, 299,              /* vblank_start, vblank_end (will be set) */
		260, 262,              /* vsync_start, vsync_end (will be set) */
		658,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		400, 300,              /* width, height */
		60, 28330,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		399, 1286,             /* hblank_start, hblank_end (will be set) */
		621, 653,              /* hsync_start, hsync_end (will be set) */
		369,                   /* vtotal */
		299, 369,              /* vblank_start, vblank_end (will be set) */
		322, 324,              /* vsync_start, vsync_end (will be set) */
		662,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		640, 350,              /* width, height */
		60, 32930,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		639, 1286,             /* hblank_start, hblank_end (will be set) */
		801, 833,              /* hsync_start, hsync_end (will be set) */
		429,                   /* vtotal */
		349, 429,              /* vblank_start, vblank_end (will be set) */
		376, 378,              /* vsync_start, vsync_end (will be set) */
		768,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		640, 400,              /* width, height */
		60, 37520,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		639, 1286,             /* hblank_start, hblank_end (will be set) */
		801, 833,              /* hsync_start, hsync_end (will be set) */
		489,                   /* vtotal */
		399, 489,              /* vblank_start, vblank_end (will be set) */
		429, 431,              /* vsync_start, vsync_end (will be set) */
		772,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#else
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                   /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                   /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif
#endif
	{
		640, 480,              /* width, height */
		60, 45180,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		639, 1286,             /* hblank_start, hblank_end (will be set) */
		801, 833,              /* hsync_start, hsync_end (will be set) */
		589,                   /* vtotal */
		479, 589,              /* vblank_start, vblank_end (will be set) */
		516, 518,              /* vsync_start, vsync_end (will be set) */
		599,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#ifndef CONFIG_MICRO
#if 0
	{
		704, 480,              /* width, height */
		60, 45180,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		703, 1286,             /* hblank_start, hblank_end (will be set) */
		849, 881,              /* hsync_start, hsync_end (will be set) */
		589,                   /* vtotal */
		479, 589,              /* vblank_start, vblank_end (will be set) */
		516, 518,              /* vsync_start, vsync_end (will be set) */
		776,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		704, 576,              /* width, height */
		60, 54370,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		703, 1286,             /* hblank_start, hblank_end (will be set) */
		849, 881,              /* hsync_start, hsync_end (will be set) */
		709,                   /* vtotal */
		575, 709,              /* vblank_start, vblank_end (will be set) */
		620, 622,              /* vsync_start, vsync_end (will be set) */
		786,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 1 */
	{
		720, 350,              /* width, height */
		60, 32930,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		719, 1286,             /* hblank_start, hblank_end (will be set) */
		861, 893,              /* hsync_start, hsync_end (will be set) */
		429,                   /* vtotal */
		349, 429,              /* vblank_start, vblank_end (will be set) */
		376, 378,              /* vsync_start, vsync_end (will be set) */
		790,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		720, 400,              /* width, height */
		60, 37520,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		719, 1286,             /* hblank_start, hblank_end (will be set) */
		861, 893,              /* hsync_start, hsync_end (will be set) */
		489,                   /* vtotal */
		399, 489,              /* vblank_start, vblank_end (will be set) */
		429, 431,              /* vsync_start, vsync_end (will be set) */
		800,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#else
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif

	{
		720, 480,              /* width, height */
		60, 45180,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		719, 1286,             /* hblank_start, hblank_end (will be set) */
		861, 893,              /* hsync_start, hsync_end (will be set) */
		589,                   /* vtotal */
		479, 589,              /* vblank_start, vblank_end (will be set) */
		516, 518,              /* vsync_start, vsync_end (will be set) */
		804,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

#if 0
	{
		720, 540,              /* width, height */
		60, 51310,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		719, 1286,             /* hblank_start, hblank_end (will be set) */
		861, 893,              /* hsync_start, hsync_end (will be set) */
		669,                   /* vtotal */
		539, 669,              /* vblank_start, vblank_end (will be set) */
		583, 585,              /* vsync_start, vsync_end (will be set) */
		808,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#else
	{
		0, 0,                  /* width, height */
		0, 0,                  /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif

	{
		720, 576,              /* width, height */
		60, 36250,             /* refresh (will be set), dot clock */
		857,                   /* htotal */
		719, 857,              /* hblank_start, hblank_end (will be set) */
		754, 786,              /* hsync_start, hsync_end (will be set) */
		709,                   /* vtotal */
		575, 709,              /* vblank_start, vblank_end (will be set) */
		620, 622,              /* vsync_start, vsync_end (will be set) */
		818,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
	    0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

#if 0
	{
		768, 576,              /* width, height */
		60, 54370,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		767, 1286,             /* hblank_start, hblank_end (will be set) */
		897, 929,              /* hsync_start, hsync_end (will be set) */
		709,                   /* vtotal */
		575, 709,              /* vblank_start, vblank_end (will be set) */
		620, 622,              /* vsync_start, vsync_end (will be set) */
		822,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#else
	{
		0, 0,                  /* width, height */
		0, 0,                  /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif
#endif
	{
		800, 600,              /* width, height */
		60, 56670,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		799, 1286,             /* hblank_start, hblank_end (will be set) */
		921, 953,              /* hsync_start, hsync_end (will be set) */
		739,                   /* vtotal */
		599, 739,              /* vblank_start, vblank_end (will be set) */
		646, 648,              /* vsync_start, vsync_end (will be set) */
		601,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#ifndef CONFIG_MICRO
#if 0
	{
		832, 624,              /* width, height */
		60, 58970,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		831, 1286,             /* hblank_start, hblank_end (will be set) */
		945, 977,              /* hsync_start, hsync_end (will be set) */
		769,                   /* vtotal */
		623, 769,              /* vblank_start, vblank_end (will be set) */
		672, 674,              /* vsync_start, vsync_end (will be set) */
		832,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 2 */
	{
		920, 766,              /* width, height */
		60, 71990,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		919, 1286,             /* hblank_start, hblank_end (will be set) */
		1011, 1043,            /* hsync_start, hsync_end (will be set) */
		939,                   /* vtotal */
		765, 939,              /* vblank_start, vblank_end (will be set) */
		823, 825,              /* vsync_start, vsync_end (will be set) */
		836,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#else
	{
		0, 0,                  /* width, height */
		0, 0,                  /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,                  /* width, height */
		0, 0,                  /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif

	{
		1024, 768,             /* width, height */
		60, 72750,             /* refresh (will be set), dot clock */
		1286,                  /* htotal */
		1023, 1286,            /* hblank_start, hblank_end (will be set) */
		1089, 1121,            /* hsync_start, hsync_end (will be set) */
		949,                   /* vtotal */
		767, 949,              /* vblank_start, vblank_end (will be set) */
		828, 830,              /* vsync_start, vsync_end (will be set) */
		609,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1280, 1024,            /* width, height */
		60, 128660,            /* refresh (will be set), dot clock */
		1715,                  /* htotal */
		1279, 1715,            /* hblank_start, hblank_end (will be set) */
		1388, 1420,            /* hsync_start, hsync_end (will be set) */
		1259,                  /* vtotal */
		1023, 1259,            /* vblank_start, vblank_end (will be set) */
		1086, 1088,            /* vsync_start, vsync_end (will be set) */
		611,                   /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif
	PD_TIMING_TABLE_END
};
pd_timing_t sdvo_hdtv_timing_table[] =
{
	/* byte 0 */
	{
		640, 480,              /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},


	{
		800, 600,              /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#ifndef CONFIG_MICRO
	{
		1024, 768,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1280, 960,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1400, 1050,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1600, 1200,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1920, 1440,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		2048, 1536,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 1 */
	{
		2560, 1920,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL ,                 /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 2 */
	{
		848, 480,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1064, 600,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1280, 720,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1360, 768,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1704, 960,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1864, 1050,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1920, 1080,            /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		2128, 1200,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 3 */
	{
		2560, 1400,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		2728, 1536,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,             /* width, height */
		0, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 4 */
	{
		768, 480,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		960, 600,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1152, 720,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1124, 768,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1536, 960,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1680, 1050,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1728, 1080,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		1920, 1200,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 5 */
	{
		2304, 1440,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		2456, 1536,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 6 */
	{
		1280, 1024,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					 /* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},

	/* byte 7 */
	{
		1280, 768,             /* width, height */
		60, 0,                 /* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		PD_MODE_SUPPORTED|     /* this mode is supported */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
	{
		0, 0,					/* width, height */
		0, 0,					/* refresh (will be set), dot clock */
		0,                     /* htotal */
		0, 0,                  /* hblank_start, hblank_end (will be set) */
		0, 0,                  /* hsync_start, hsync_end (will be set) */
		0,                     /* vtotal */
		0, 0,                  /* vblank_start, vblank_end (will be set) */
		0, 0,                  /* vsync_start, vsync_end (will be set) */
		0,                     /* mode number */
		0,                     /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extenstion pointer (will bet set) */
		0, 0				   /* Reserved */
	},
#endif
	PD_TIMING_TABLE_END
};

/* .......................................................................... */
/* .......................................................................... */
/*============================================================================
	Function	:	pd_init is the first function that is invoked by IEG driver.

	Parameters	:	handle : not used

	Remarks     :   pd_init initializes pd_driver_t structure and registers the
					port driver with IEG driver by calling pd_register function

	Returns     :	Status returned by pd_register function.
	------------------------------------------------------------------------- */
int PD_MODULE_INIT(sdvo_init, (void *handle))
{
	int status;

	PD_DEBUG("sdvo: pd_init()");

	status = pd_register(handle, &g_sdvo_drv);
	if (status != PD_SUCCESS) {
		PD_DEBUG("sdvo: Error ! pd_init: pd_register() failed with "
				  "status=%#x", status);
	}
	return status;
}


/*----------------------------------------------------------------------
 * Function: sdvo_exit()
 *
 * Description: This is the exit function for SDVO port driver to unload
 *              the driver.
 *
 * Parameters:  None.
 *
 * Return:      PD_SUCCESS(0)  success
 *              PD_ERR_XXXXXX  otherwise
 *----------------------------------------------------------------------*/
int PD_MODULE_EXIT(sdvo_exit, (void))
{
	return (PD_SUCCESS);
} /* end sdvo_exit() */


/*	============================================================================
	Function	:	sdvo_open is called for each combination of port and dab
					registers to detect the SDVO device.

	Parameters	:	p_callback : Contains pointers to read_regs/write_regs
								functions to access I2C registes.

					pp_context	  : Pointer to port driver allocated context
								structure is returned in this argument

	Remarks     :	sdvo_open detects the presence of SDVO device for specified
					port.

	Returns     :	PD_SUCCESS If SDVO device is detected
					PD_ERR_xxx On Failure
	------------------------------------------------------------------------- */
int sdvo_open(pd_callback_t *p_callback, void **pp_context)
{
	sdvo_device_context_t *p_ctx;
	sdvo_status_t status;
	sdvo_output_flags_t output_flags;
	unsigned long power_state;
#ifndef CONFIG_MICRO
	i2c_reg_t reg;
#endif

	PD_DEBUG("sdvo: sdvo_open(): port_num=%ld", p_callback->port_num);

	p_ctx = pd_malloc(sizeof(sdvo_device_context_t));
	if (p_ctx == NULL) {
		PD_ERROR("sdvo: Error ! sdvo_open: pd_malloc() failed");
		return PD_ERR_NOMEM;
	}

	pd_memset(p_ctx, 0, sizeof(sdvo_device_context_t));

	*pp_context = p_ctx;
	p_ctx->p_callback = p_callback;

#ifndef CONFIG_MICRO
	/* reset context to avoid wrong timing list */
	if ((p_ctx->dev_cap.vendor_id != VENDOR_ID_LAPIS)
		&& (p_ctx->dev_cap.device_id != 0x1) && p_callback->reset ){
		PD_DEBUG("Resetting encoder\n");
		sdvo_reset_encoder(p_ctx);
	}
#endif

	status = sdvo_get_device_capabilities(p_ctx, &p_ctx->dev_cap);
	if (status != SS_SUCCESS) {

		PD_DEBUG("sdvo: Error ! sdvo_open: sdvo_get_device_capabilities() "
				  "failed with status=%d",  status);
		pd_free(p_ctx);
		return PD_ERR_NODEV;
	}

	PD_DEBUG("sdvo: Device Capabilities:");
	PD_DEBUG("sdvo: vendor=%d, device=%d, revision=%d,",
			  p_ctx->dev_cap.vendor_id, p_ctx->dev_cap.device_id,
			  p_ctx->dev_cap.revision_id);
	PD_DEBUG("sdvo: version_major=%d, version_minor=%d, num sDVO inputs=%d",
			  p_ctx->dev_cap.version_major, p_ctx->dev_cap.version_minor,
			  p_ctx->dev_cap.num_inputs);
	PD_DEBUG("sdvo: Scaling support: smooth=%d, sharp=%d, up=%d, down=%d,",
			  p_ctx->dev_cap.smooth_scaling, p_ctx->dev_cap.sharp_scaling,
			  p_ctx->dev_cap.up_scaling,
			  p_ctx->dev_cap.down_scaling);
	PD_DEBUG("sdvo: Stall support=%d", p_ctx->dev_cap.stall);

	PD_DEBUG("sdvo: out0:%s",
			  sdvo_get_output_type(p_ctx->dev_cap.output.out0, 0));
	PD_DEBUG("sdvo: out1:%s",
			  sdvo_get_output_type(p_ctx->dev_cap.output.out1, 1));

	/* Detecting STMicro device */
	if ((p_ctx->dev_cap.vendor_id == 0x82) &&
		(p_ctx->dev_cap.device_id==0x01)) {

		p_ctx->st_sdvo = 1;
	}

	/* ...................................................................... */
	p_ctx->out_type = p_ctx->dev_cap.output;

	/*Workaround:
	 * st_sdvo has two potential modes lvds and rgba
	 * As we dont have the relevant information from xorg (attr/9(
	 * at this point to correctly identify which one the user wants
	 * and as set target will fail if sent 2 output types
	 * This if has been added
	 */
	if(!p_ctx->st_sdvo)
	{
		status = sdvo_set_target_output(p_ctx, p_ctx->out_type);

	}

	status = sdvo_get_supported_power_states(p_ctx, &p_ctx->supp_pwr_states);
	if (status == SS_SUCCESS) {
		PD_DEBUG("sdvo: Supported Power States = %#x",
			p_ctx->supp_pwr_states);
	} else {
		PD_DEBUG("sdvo: GetSupported Power States FAILED with status = %d",status);
	}

	/* ...................................................................... */
	/*	Assume SDVO0 connected to Port B and SDVO1 connected to Port C		  */
	/*	However the correct way to determine SDVO port to device mapping at   */
	/*	run-time is via Get_Trained_Inputs opcode                             */
	p_ctx->inp_dev = SDVO0;


#ifndef CONFIG_MICRO
	/*We want this in (Windows XP driver) AND
	(in VBIOS when LVDS is not linked.)*/
	if ((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS)
		&& (p_ctx->dev_cap.device_id == 0x1)){
		status = sdvo_set_target_input(p_ctx, p_ctx->inp_dev);
		if (status != SS_SUCCESS) {
			PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_target_input()"
				"failed with status=%d", status);
			return PD_ERR_INTERNAL;
		}



		/*Workaround:
			 * st_sdvo has two potential modes lvds and rgba
			 * As we dont have the relevant information from xorg (attr/9(
			 * at this point to correctly identify which one the user wants
			 * and as set target will fail if sent 2 output types
			 * This if has been added
			 */
		if(!p_ctx->st_sdvo)
			status = sdvo_set_target_output(p_ctx, p_ctx->out_type);

		if (status != SS_SUCCESS) {
			PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_target_output()"
				"failed with status=%d", status);
			return PD_ERR_INTERNAL;
		}


		/* ...................................................................... */
		status = sdvo_get_supported_power_states(p_ctx, &p_ctx->supp_pwr_states);
		if (status == SS_SUCCESS) {
			PD_DEBUG("sdvo: Supported Power States = %#x",
			p_ctx->supp_pwr_states);
		}
	}
#endif


	/* This code is to find out whether a TV encoder is used to get VGA display
	 * or TVout display */

	/* Read the power state */
	sdvo_get_power(p_ctx, &power_state);

	/* set device to DO before performing connection detect attempt */
	sdvo_set_power(p_ctx, PD_POWER_MODE_D0);

	status = sdvo_get_attached_displays(p_ctx, &output_flags);
	/*
	 * sdvo_get_attached_displays returns "0" when sdvo cable is
	 *  disconnected. Need to make sure out_type flags not
	 *  overridden.
	 */
	if (status == SS_SUCCESS && output_flags.flags != 0) {
		p_ctx->out_type.flags &= output_flags.flags;
	} else {
		/* If display detection fails, reset the hardware and try again: */
		PD_ERROR("get_attached_display() did not succeed; trying HW reset");
		status = sdvo_reset(p_ctx);
		PD_DEBUG("sdvo_reset returned %u", status);
		status = sdvo_get_attached_displays(p_ctx, &output_flags);
		if (status == SS_SUCCESS && output_flags.flags != 0) {
			p_ctx->out_type.flags &= output_flags.flags;
		} else {
			PD_ERROR("get_attached_display() did not succeed again; giving up");
			return PD_ERR_NODEV;
		}
	}

	/* If component video (YPbPr) is connected, for some reason, both ypbpr
	 * and s_video are 1 when sdvo_get_attached_displays is called. We need
	 * to set only ypbpr to 1 because, apparently, the attribute table
	 * would not be created if multiple outputs are on (as in this case).
	 */
	if (p_ctx->out_type.flags & TV_YPBPR_DISP_MASK) {
		p_ctx->out_type.flags &= TV_YPBPR_DISP_MASK;
	}

	/* restore the previous power state */
	sdvo_set_power(p_ctx, power_state);
	g_sdvo_drv.flags &= ~(PD_FLAG_CLK_SOURCE|PD_FLAG_PIPE_MASTER);

	/* Set the display type */
	if (p_ctx->out_type.flags & CRT_EXT_DISP_MASK) {
		g_sdvo_drv.type	= PD_DISPLAY_CRT_EXT;
	} else if (p_ctx->out_type.flags & DRGB_DISP_MASK) {
		g_sdvo_drv.type = PD_DISPLAY_DRGB;
	} else if (p_ctx->out_type.flags & FP_DISP_MASK) {
		status = SDVO_IS_HDMI_SUPPORTED(p_ctx);
		if (status == SS_SUCCESS) {
			PD_DEBUG("sdvo: HDMI transmiter ");
			g_sdvo_drv.type = PD_DISPLAY_HDMI_EXT;
		} else{
			g_sdvo_drv.type	= PD_DISPLAY_FP;
		}
	} else if (p_ctx->out_type.flags & TV_DISP_MASK) {
		g_sdvo_drv.type = PD_DISPLAY_TVOUT;

		/* TV display is PIPE master and also generates clock */
		g_sdvo_drv.flags |= (PD_FLAG_CLK_SOURCE|PD_FLAG_PIPE_MASTER);

		/* CH7021: Initialize H/V Overscan */
		sdvo_write_i2c_reg(p_ctx, 0x38, 0x1F);
		sdvo_write_i2c_reg(p_ctx, 0x39, 0x1F);

#ifndef CONFIG_MICRO
		/*Set optimal TV settings*/

		/*identify tv output systems in terms of line, and frame rate*/
		sdvo_read_i2c_reg(p_ctx, 0x36, &reg);

		if (reg != 0 && reg != 0x24) //not 480i, and 576i system
			reg = 0xFF;

		status = sdvo_set_tv_settings(p_ctx,reg);

		if (status != SS_SUCCESS) {
			PD_DEBUG("sdvo: Error ! sdvo_open(): sdvo_set_tv_settings "
				"failed with status=%d",  status);
		}
#endif

	}

	/* Indicate main driver about upscaling */
	if (p_ctx->dev_cap.up_scaling && g_sdvo_drv.type != PD_DISPLAY_CRT_EXT) {
		g_sdvo_drv.flags |= PD_FLAG_UP_SCALING;
	} else {
		/* Remove the upscaling dev caps for VGA bypass. There are functions
		 * that decides on upscaling based on the caps and not the flags. */
		p_ctx->dev_cap.up_scaling = 0;
	}

	/* ...................................................................... */
	/*	CH7308 up-scaling workaround */
	if ((p_ctx->dev_cap.vendor_id == 2) && (p_ctx->dev_cap.device_id == 0x41) &&
		(p_ctx->dev_cap.revision_id == 1)) {
		i2c_reg_t reg;
		sdvo_read_i2c_reg(p_ctx, 0x5D, &reg);
		reg |= 1;
		sdvo_write_i2c_reg(p_ctx, 0x5D, reg);
	}
	/* ...................................................................... */

	/* CH7021A XUD VGAbypass */
	if (((p_ctx->dev_cap.vendor_id == 2) &&
		((p_ctx->dev_cap.device_id==0xC2)||(p_ctx->dev_cap.device_id==0x42)))&&
		(p_ctx->dev_cap.revision_id == 2) &&
		(g_sdvo_drv.type == PD_DISPLAY_CRT_EXT)) {
		i2c_reg_t reg;
		sdvo_read_i2c_reg(p_ctx, 0x7F, &reg);
		reg |= 1;
		sdvo_write_i2c_reg(p_ctx, 0x7F, reg);

		sdvo_read_i2c_reg(p_ctx, 0x6C, &reg);
		reg |= 0x03;
		sdvo_write_i2c_reg(p_ctx, 0x6C, reg);

		sdvo_read_i2c_reg(p_ctx, 0x7F, &reg);
		reg &= 0xFE;
		sdvo_write_i2c_reg(p_ctx, 0x7F, reg);
	}

	return PD_SUCCESS;
}

/*	============================================================================
	Function	:	sdvo_init_device is called to initialize a SDVO device

	Parameters	:	p_context : Pointer to port driver allocated context
					structure

	Remarks     :

	Returns     :	PD_SUCCESS	If initialization is successful
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_init_device(void *p_context)
{
	PD_DEBUG("sdvo: sdvo_init_device()");
#ifndef CONFIG_MICRO
	if(g_sdvo_drv.type == PD_DISPLAY_HDMI_EXT){
		/* Initialize transmitter audio capability if available */
		SDVO_INIT_HDMI(p_context);
	}
#endif
	return PD_SUCCESS;
}

/*	============================================================================
	Function	:	sdvo_get_timing_list is called to get the list of display
					modes supported by the SDVO device and the display.

	Parameters	:	p_context: Pointer to port driver allocated context
								structure
					p_in_list: List of display modes supported by the IEG driver
					pp_out_list: List of modes supported by the SDVO device

	Remarks     :

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_get_timing_list(void *p_context, pd_timing_t *p_in_list,
	pd_timing_t **pp_out_list)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	unsigned short pixel_range[2];
	pd_dvo_info_t sdvo_info = {0, 0, 0, 0, 0, 0, 0, 0};
	pd_display_info_t sdvo_display_info = {0, 0, 0, 0, NULL};
	int ret = PD_SUCCESS;
	pd_timing_t *tv_timing;
	unsigned char hdtv = 0; /* 0: standard ; 1: high definition */

	PD_DEBUG("sdvo: sdvo_get_timing_list()");

	/* ...................................................................... */
	status = sdvo_set_target_input(p_ctx, p_ctx->inp_dev);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_get_timing_list: sdvo_set_target_input()"
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	status = sdvo_get_input_pixel_clock_range(p_ctx, pixel_range);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_get_timing_list: "
				  "sdvo_get_input_pixel_clock_range() failed with status=%d",
				  status);
		sdvo_info.min_dclk = 0;
		sdvo_info.max_dclk = 0;
	} else {
		sdvo_info.min_dclk = (unsigned long)pixel_range[0] * 10;
		if (p_ctx->st_sdvo) {
			sdvo_info.min_dclk = 25000;
		}
		sdvo_info.max_dclk = (unsigned long)pixel_range[1] * 10;
	}

	sdvo_info.upscale   = p_ctx->dev_cap.up_scaling;
	sdvo_info.downscale = p_ctx->dev_cap.down_scaling;
	sdvo_display_info.panel_fit = p_ctx->up_scaling;

	if (!(p_ctx->out_type.flags & TV_DISP_MASK)) {
		/* For VGA, DVI, LVDS and HDMI displays,
		 * call helper function to filter the modes */
		ret = pd_filter_timings(p_ctx->p_callback->callback_context,
			p_in_list, &p_ctx->p_mode_table, &sdvo_info, &sdvo_display_info);

		/* Helper function returns the below values */
		p_ctx->native_dtd = sdvo_display_info.native_dtd;
		p_ctx->fp_width = sdvo_display_info.width;
		p_ctx->fp_height = sdvo_display_info.height;
		*pp_out_list = p_ctx->p_mode_table;
	} else {
		sdvo_create_preferred_timings_t pref_timings;
		sdvo_dtd_t dtd_in;
		unsigned char params[6] = {0,0,0,0,0,0};
		unsigned char ret_value[8] = {0,0,0,0,0,0,0,0};
		unsigned long *temp = (unsigned long *)&params[2];
		unsigned char i=0, j=0;

		status = sdvo_get_tv_output_format(p_ctx,params);
		if (params[0] > 0 || params[1] > 0 || (params[2] > 0 &&
			params[2] < 0x08)) {
			/* SDTV support */
			status = sdvo_get_sdtv_resolution_support(p_ctx,params,ret_value);
			tv_timing = sdvo_sdtv_timing_table;

		} else {
			/* HDTV support */
			(*temp) >>= 3;
			status = sdvo_get_scaled_hdtv_resolution_support(
				p_ctx, &params[2], ret_value);
			tv_timing = sdvo_hdtv_timing_table;
			hdtv = 1;
		}


		/* For TV displays generate supported timing table */

		while (tv_timing->width != PD_TIMING_LIST_END) {
			/* Fill-in pref timings */
			/* Go through bits 0 to 7 of the returned byte and check to see if
			 * it is on. If yes, calculate the preferred timing */

			for (j=0; j<8; j++) {
				tv_timing->mode_info_flags &= ~PD_MODE_SUPPORTED;
				/* Cannot request preferred input timing for width/height = 0 */
				if (!tv_timing->width || !tv_timing->height) {
					tv_timing++;
					continue;
				}
				if (ret_value[i] & BIT(j)) {
					pd_memset(&pref_timings, 0, sizeof(pref_timings));
					/* pref_timings.pixel_clock=(unsigned short)(t1->dclk/10);*/
					pref_timings.active_horz_pixels = tv_timing->width;
					pref_timings.active_vert_pixels = tv_timing->height;
					status = sdvo_get_preferred_input_timings(p_ctx,
						&pref_timings, &dtd_in);

					if (status != SS_SUCCESS) {
						PD_ERROR("sdvo_get_timing_list: %ux%u cannot be "
							"supported status = %u",
							tv_timing->width, tv_timing->height, status);
					} else {
						PD_DEBUG("sdvo_get_timing_list: "
							"device returned values = %ux%u dclk=%u",
							dtd_in.horz_active, dtd_in.vert_active,
							dtd_in.pixel_clock);
					}
					if (dtd_in.horz_active && dtd_in.vert_active) {
						sdvo_convert_dtd_to_pd_timing(&dtd_in, tv_timing);
					}
					PD_DEBUG("sdvo_get_timing_list: after conversion: "
						"device returned values = %ux%u dclk=%lu",
						tv_timing->width, tv_timing->height,
						tv_timing->dclk);
					tv_timing->refresh = 60;     /* This is for OS to see */
					tv_timing->mode_info_flags |= PD_MODE_SUPPORTED;
				} /* end checking ret_value[i] */
				tv_timing++;
			} /* end for loop */
			i++; /* grab next returned byte */
		} /* end while timing list */

		if (hdtv) {
			*pp_out_list = sdvo_hdtv_timing_table;
		}
		else {
			*pp_out_list = sdvo_sdtv_timing_table;
		}
	}
	 /* end tv if */
	return ret;
}

/*	============================================================================
	Function	:	sdvo_set_mode is called to test if specified mode can be
					supported or to set it.

	Parameters	:	p_context: Pointer to port driver allocated context
					p_mode	: New mode
					flags	: In test mode it is set to PD_SET_MODE_FLAG_TEST

	Remarks     :	sdvo_set_mode first verifies that the new mode is
					supportable.
					If not it returns an error status
					If the flags is not set to PD_SET_MODE_FLAG_TEST it sets the
					new mode.

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_set_mode(void *p_context, pd_timing_t *p_mode, unsigned long flags)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	sdvo_dtd_t dtd_out, dtd_in;
	sdvo_clock_rate_mult_t clock_mult;
	unsigned char b_use_scaled_timing;
	pd_timing_t *p_native_mode;
	unsigned long dclk;

	PD_DEBUG("sdvo: sdvo_set_mode() mode=%dx%d@%d",
			  p_mode->width, p_mode->height, p_mode->refresh);

	p_native_mode = p_ctx->native_dtd;

	if (p_ctx->out_type.flags & TV_DISP_MASK) {
		p_native_mode = p_mode;
	}

	/* ...................................................................... */
	status = sdvo_set_target_input(p_ctx, p_ctx->inp_dev);
	if (status != SS_SUCCESS) {

		PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_target_input()"
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	/*Workaround:
		 * st_sdvo has two potential modes lvds and rgba
		 * As we dont have the relevant information from xorg (attr/9(
		 * at this point to correctly identify which one the user wants
		 * and as set target will fail if sent 2 output types
		 * This if has been added
		 */
	if(!p_ctx->st_sdvo)
		status = sdvo_set_target_output(p_ctx, p_ctx->out_type);

	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_target_output()"
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	/* ...................................................................... */
	/*	Test mode.	Specified mode is supported by this device return success */
	if (flags & PD_SET_MODE_FLAG_TEST) {
		return PD_SUCCESS;
	}

	/* ...................................................................... */
	/* Set power state to D0 */
	sdvo_set_power(p_context, PD_POWER_MODE_D0);
	/* ...................................................................... */
	/*	Initialize the DTD structure and set output and input timings */
	b_use_scaled_timing = FALSE;
	/* p_ctx->unscaled_Mode.width = 0; */

	dclk = p_mode->dclk;

	/*	If this sDVO device supports scaling and if native mode was specified */
	/*	get the preferred input timings for native output timings */
	if (((p_native_mode) &&
		((p_native_mode->width != p_mode->width) ||
		 (p_native_mode->height != p_mode->height)) &&

		((p_ctx->up_scaling) || (p_ctx->dev_cap.down_scaling))) ||

		(p_ctx->out_type.flags & TV_DISP_MASK)) {

		sdvo_create_preferred_timings_t pref_timings;

		sdvo_convert_pd_timing_to_dtd(p_native_mode, &dtd_out,
			p_native_mode->dclk);

		status = sdvo_set_output_timings(p_ctx, &dtd_out);
		if (status != SS_SUCCESS) {
			PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_output_timings() "
					  "failed with status=%d", status);
			return PD_ERR_INTERNAL;
		}

		pd_memset(&pref_timings, 0, sizeof(pref_timings));
		pref_timings.pixel_clock = (unsigned short)(p_native_mode->dclk / 10);
		pref_timings.active_horz_pixels	= p_mode->width;
		pref_timings.active_vert_pixels	= p_mode->height;

		if (!(p_ctx->out_type.flags & TV_DISP_MASK)) {
			pref_timings.b_scaled = 1;
			if (p_native_mode->mode_info_flags & PD_SCAN_INTERLACE) {
				pref_timings.b_interlaced = 1;
			}
		}

		status = sdvo_get_preferred_input_timings(p_ctx, &pref_timings,&dtd_in);
		if (status != SS_SUCCESS) {
			PD_ERROR("sdvo: Error ! sdvo_set_mode: "
				"sdvo_get_preferred_input_timings() failed with status=%d",
				status);
		} else {
			PD_DEBUG("sdvo: sdvo_set_mode: pref_in_timings = %ux%u dclk=%u",
				dtd_in.horz_active, dtd_in.vert_active, dtd_in.pixel_clock);
			b_use_scaled_timing = TRUE;
			dclk = p_native_mode->dclk;
		}
	}

	/* ...................................................................... */
	/*	Determine clock rate multiplier */
	if (dclk > 100000) {			/*	100-200 MHz */
		clock_mult = CRM_1X;
	} else if (dclk > 50000) {	/*	50-100 Mhz */
		clock_mult = CRM_2X;
	} else {							/*	25-50 Mhz */
		clock_mult = CRM_4X;
	}

	PD_DEBUG("sdvo: Clock Multiplier = %d dclk = %lu", clock_mult, dclk);

	/* ...................................................................... */
	/*	SI 1364 Autozone switch workaround */
	if ((p_ctx->dev_cap.vendor_id == 4) && (p_ctx->dev_cap.device_id == 0xAA)) {
		i2c_reg_t data;
		if (clock_mult == CRM_1X) {
			data = 0x4D;
		} else {
			data = 0x49;
		}
		sdvo_write_i2c_reg(p_ctx, 0x51, data);
	}

	/* ...................................................................... */
	if (! b_use_scaled_timing) {
		sdvo_convert_pd_timing_to_dtd(p_mode, &dtd_out, p_mode->dclk);
		status = sdvo_set_output_timings(p_ctx, &dtd_out);
		if ((status != SS_SUCCESS) && (status != SS_NOT_SUPPORTED)) {
			PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_output_timings() "
					  "failed with status=%d", status);
			return PD_ERR_INTERNAL;
		}
		dtd_in = dtd_out;
	}

#ifndef CONFIG_MICRO
	/*We want this in (Windows XP driver) AND
	(in VBIOS when LVDS is not linked.)*/
	/* This workaround needed for LAPIS solution only.
	It would probably give problem if the SDVO is connected to
	analog display, but that is not the usage model for LAPIS.
	*/
	if ((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS)
		&& (p_ctx->dev_cap.device_id == 0x1)){
		/* The LAPIS SDVO receiver to return “Invalid Argument” when:
		(1) Horizontal Active   < 600
		(2) Horizontal Blanking < 16
		(3) HSync pulse width  < 2
		(4) (HSync Offset + HSync pulse width) > Horizontal Blanking
		(5) Vertical Active      < 480
		(6) Vertical Blanking    < 3
		(7) VSync pulse width  < 1
		(8) (VSync Offset + VSync pulse width) > Vertical Blanking
		(9) Interlace mode or unacceptable sync ( Part2/4 must be "00011xxx" )
		(10) SDVO STALL mode
		(11) SDVO Scaling mode

		We have seen that (4) and (8) are problematic. The work around is
		to moved Horizontal VBlank and Vertical VBlank so it is always
		at least 1 pixel bigger than offset + pulse width.

		There is no problem with Chrontel 7307 even with
		horz_vblank <  (HSync Offset + HSync pulse width)
		and I assume it is the same with other SDVO card.
		*/
		if ((dtd_in.horz_sync_offset + dtd_in.horz_sync_pulse_width) >
					dtd_in.horz_blanking){
			dtd_in.horz_blanking = dtd_in.horz_sync_offset +
					dtd_in.horz_sync_pulse_width + 1;
		}

		if ((dtd_in.vert_sync_offset + dtd_in.vert_sync_pulse_width) >
					dtd_in.vert_blanking){
			dtd_in.vert_blanking = dtd_in.vert_sync_offset +
					dtd_in.vert_sync_pulse_width + 1;
		}

	}
#endif
	/*
	 * Disable panel fitting for CH7308 card on Pipe B if enabled, because it
	 * handles its own upscaling
	 */
	if ((p_ctx->dev_cap.vendor_id == CH7308_VENDOR_ID) &&
			(p_ctx->dev_cap.device_id == CH7308_DEVICE_ID) &&
			(p_ctx->dev_cap.revision_id == CH7308_REVISION_ID)) {
		pd_reg_t reg_list[2];

		reg_list[0].reg = PFIT_CONTROL;
		reg_list[1].reg = PD_REG_LIST_END;

		/*Read the current value of the panel fitting control register*/
		p_ctx->p_callback->read_regs(p_ctx->p_callback->callback_context,
				reg_list, PD_REG_MIO);

		PD_DEBUG("PFIT_CONTROL 0x%lx = 0x%lx", PFIT_CONTROL,reg_list[0].value);

		if((reg_list[0].value & PFIT_PIPEB_ENABLED) == PFIT_PIPEB_ENABLED ){
			/* Disable panel fitting */
			reg_list[0].value = 0x0;
			p_ctx->p_callback->write_regs(p_ctx->p_callback->callback_context,
					reg_list, PD_REG_MIO);
			PD_DEBUG(" Value 0x%lx written to PFIT_CONTROL 0x%lx",
					reg_list[0].value,PFIT_CONTROL);
		}
	}

	status = sdvo_set_input_timings(p_ctx, &dtd_in);
	if ((status != SS_SUCCESS) && (status != SS_NOT_SUPPORTED)) {
		PD_ERROR("sdvo: Error ! sdvo_set_mode: sdvo_set_input_timings() "
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	status = sdvo_set_clock_rate_multiplier(p_ctx, clock_mult);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error!sdvo_set_mode: sdvo_set_clock_rate_multiplier() "
				  "failed with status=%d",  status);
		return PD_ERR_INTERNAL;
	}

	return PD_SUCCESS;
}

/*	============================================================================
	Function	:	sdvo_post_set_mode

	Parameters	:	p_context: Pointer to port driver allocated context
					p_mode	:
					flags	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
int sdvo_post_set_mode(void *p_context, pd_timing_t *p_mode,
						  unsigned long flags)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	i2c_reg_t trained = 0;
	sdvo_output_flags_t out_flags[2];
	int pd_status = PD_SUCCESS;
	sdvo_output_flags_t output_flags;
	/*sdvo_output_flags_t is_attached display;*/

	PD_DEBUG("sdvo: sdvo_post_set_mode()");

#ifndef CONFIG_MICRO
	/* This is a workaround specific to LAPIS */
	if((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS)
		&& (p_ctx->dev_cap.device_id == 0x1)){

		pd_timing_t local_p_mode;
		/* pd_reg_t reg_list[2]; */
		/* unsigned long temp, portreg; */
		local_p_mode = *p_mode;

		sdvo_reset_encoder(p_context);
		/* sdvo_reset(p_context);*/ /* THIS is workaround for LAPIS SDVO flashing issue.*/
		sdvo_set_power(p_context, 0);
		if (p_ctx->display_pwr_state == 0x0)
		{

			pd_attr_t *p_attr_temp =
						pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
						PD_ATTR_ID_RB_SWAP_MODE, 0 );

			/* RB SWAP */
			sdvo_alter_static_attr(p_ctx, p_attr_temp, p_attr_temp->current_value);
	
			sdvo_set_mode(p_context, &local_p_mode,0);
		} else {
			sdvo_set_power(p_context, p_ctx->display_pwr_state);
		}

	}
#endif

	status = sdvo_get_trained_inputs(p_ctx, &trained);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_get_trained_inputs() failed with "
				  "status=%d", status);
	} else {
		if (trained == 0) {
			PD_DEBUG("sdvo: Error ! sdvo_post_set_mode: "
					  "No inputs are trained");
		} else {
			PD_DEBUG("sdvo: Success ! sdvo_post_set_mode: "
					  "Inputs are trained");
		}
	}

	status = sdvo_get_attached_displays(p_ctx, &output_flags);
	if (status == SS_SUCCESS) {
		PD_DEBUG("sdvo: get_attached_display returned => [0x%x]",
			output_flags.flags);
	} else {
		/* TODO: What if display detection fails? */
		/* pd_status = PD_ERR_NODEV; */
		output_flags.flags = 0;
	}

	/* To enable YPbPr, the attribute 9 must be set to 16 in xorg.conf;
	 * if it set, then p_ctx->out_type.flags would be changed to 16 in,
	 * sdvo_set_attributes. Here, we are just making sure that if YPbPr
	 * is enabled, then output_flags must be changed to 16. This needs to
	 * be done because, when component video (ypbpr) is connected, both ypbpr
	 * and s_video are 1 when read from the registers (sdvo_get_attached_displays).
	 */
	if (p_ctx->out_type.flags & TV_YPBPR_DISP_MASK) {
		output_flags.flags &= TV_YPBPR_DISP_MASK;
	}

	if (output_flags.flags == 0) {
		output_flags.flags = p_ctx->out_type.flags;
	}else if(output_flags.flags != p_ctx->out_type.flags){
		/* enable switching tv output type on-the-fly;
		* also to enable cvbs+svideo simul display */
		/* NOTE: Removed on-the-fly support with PRD change.
		p_ctx->out_type.flags will always retain the registry
		settings. Setting multiple bits out_type.flags for
		sdvo_set_target_output will caused INVALID_ARGUMENT error
		which in turn causes un-trained output. */
		/*p_ctx->out_type.flags = output_flags.flags; */
	}

	/* set inout map */
	PD_DEBUG("sdvo_post_set_mode: out_type.flags:[0x%x]",
		output_flags.flags);

	/* For ST SDVO device, when the jumper is set to LVDS or
	 * DRGB, output flags get set to 0xC0.  We need to then
	 * set the output flags to only DRGB.
	 */
	if (p_ctx->st_sdvo) {
		PD_DEBUG("sdvo_post_set_mode: out_type.flags [0x%x]->[0x%x]",
			output_flags.flags, p_ctx->out_type.flags);
		output_flags.flags = p_ctx->out_type.flags;
	}

	PD_DEBUG("sdvo_post_set_mode: out0:%s, out 1:%s",
		sdvo_get_output_type(p_ctx->out_type.out0, 0),
		sdvo_get_output_type(p_ctx->out_type.out1, 1));

	/* set inout map */
	if(output_flags.flags & TV_DISP_MASK){
		/* Ensure only one output (SVIDEO) is used since in out map
		* command can only handle one active port. SDVO specs also
		* states that any tv setting would be applied to all connectors */
		out_flags[0].flags = TV_SVIDEO_DISP_MASK;
	}else{
		out_flags[0].flags = output_flags.flags;
		/* in0 = required output type */
	}
	out_flags[1].flags = 0;                     /* in1 = 0 */
	status = sdvo_set_in_out_map(p_ctx, out_flags);
	/* Some encoders doesn't support INOUT Map */
	if (status != SS_SUCCESS && status != SS_NOT_SUPPORTED) {
		PD_ERROR("sdvo: Error ! sdvo_post_set_mode: sdvo_set_in_out_map() "
			"failed with status=%d",  status);
		pd_status = PD_ERR_INTERNAL;
	}

	/* set active outputs */
	status = sdvo_set_active_outputs(p_ctx, output_flags);
	if (status != SS_SUCCESS) {

		PD_ERROR("sdvo: Error ! sdvo_post_set_mode: sdvo_set_active_outputs() "
			"failed with status=%d",  status);
		pd_status = PD_ERR_INTERNAL;
	}
#ifndef CONFIG_MICRO
	status = SDVO_IS_HDMI_SUPPORTED(p_context);
	if(status == SS_SUCCESS){
		SDVO_CONFIGURE_HDMI(p_ctx);
	}
#endif
	/* Wait for panel power up sequence to complete */
	pd_usleep((unsigned long)(p_ctx->t1+p_ctx->t2)*1000 + 10);

	return pd_status;
}


/*	============================================================================
	Function	:	sdvo_get_attributes is called to get the list of all the
					available attributes

	Parameters	:	p_context: Pointer to port driver allocated context structure
					p_Num	: Return the total number of attributes
					pp_list	: Return the list of port driver attributes

	Remarks     :	sdvo_get_attributes calls SDVO interface functions to get all
					available range,bool and list attributes supported by the
					SDVO device

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_get_attributes(void *p_context, unsigned long *p_num_attr,
	pd_attr_t **pp_list)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	unsigned long i;

	PD_DEBUG("sdvo: sdvo_get_attributes()");

	/* No attributes for external CRT */
	if (p_ctx->out_type.flags & CRT_EXT_DISP_MASK) {
		PD_DEBUG("No attributes available for SDVO CRT.");
		*p_num_attr = 0;
		return 0;
	}
#ifndef CONFIG_MICRO
	/* This is a workaround specific to LAPIS */
	if(p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS
		&& p_ctx->dev_cap.device_id == 0x1){
		/* 	TODO: sdvo_set_target_output return pending when the power state is D3 on
		* 	ML7213 A0. other sdvo cards don't have this problem and LAPIS don't see
		* 	this problem on their site. verify this on A1 and remove the code if
		* 	it return success */
		sdvo_set_power(p_ctx, PD_POWER_MODE_D0);
	}
#endif

	status = sdvo_set_target_output(p_ctx, p_ctx->out_type);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_get_attributes: sdvo_set_target_output()"
			"failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	if (p_ctx->p_attr_table == NULL) {
		PD_DEBUG("sdvo: sdvo_get_attributes: "
			"Attribute Table not initialized");
		return PD_ERR_INTERNAL;
	}
	/* ...................................................................... */
	for (i = 0; i < p_ctx->num_attrs ; i++) {
		pd_attr_t *p_attr_cur = &p_ctx->p_attr_table[i];

		if ((p_attr_cur->flags & PD_ATTR_FLAG_DYNAMIC) == 0) {
			status = sdvo_query_static_attr(p_ctx, p_attr_cur);
		} else {
			if ((p_attr_cur->id >= PD_ATTR_ID_FP_PWR_T1) &&
				(p_attr_cur->id <= PD_ATTR_ID_FP_PWR_T5)) {
				status = sdvo_query_panel_pwr_seq_attr(p_ctx,
					(pd_range_attr_t *)p_attr_cur);
			} else if (p_attr_cur->type == PD_ATTR_TYPE_RANGE) {
				status = sdvo_query_range_attr(p_ctx,
					(pd_range_attr_t *)p_attr_cur);
			} else if (p_attr_cur->type == PD_ATTR_TYPE_BOOL) {
				status = sdvo_query_bool_attr(p_ctx,
					(pd_bool_attr_t *)p_attr_cur);
			} else if (p_attr_cur->type == PD_ATTR_TYPE_LIST) {
				status = sdvo_query_list_attr(p_ctx,
					(pd_list_attr_t *)p_attr_cur);
				i += ((pd_list_attr_t *)p_attr_cur)->num_entries;
			}

			if (status != SS_SUCCESS) {
				PD_ERROR("sdvo: Error ! sdvo_get_attributes: "
					"Failed to query attr[%ld]='%s', id=%ld",
					i, SDVO_GET_ATTR_NAME(p_attr_cur), p_attr_cur->id);
			}
		}
	}

	/* ...................................................................... */
	*pp_list	= p_ctx->p_attr_table;
	*p_num_attr	= p_ctx->num_attrs ;
	return PD_SUCCESS;
}

#ifndef CONFIG_MICRO
int sdvo_set_tv_settings(void *p_context, i2c_reg_t tv_format)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;

	unsigned char chron_tv_code;

	PD_DEBUG("sdvo: sdvo_set_tv_settings: tv_format [0x%x]\n",tv_format);


	if ((tv_format == 0xFF) || (p_ctx->out_type.flags & BIT(2)) ){
		chron_tv_code = tv_format; /*HDTV or SDTV and composite type*/
	} else {
		chron_tv_code = tv_format | 0x01; /*include s-video and everything else*/
	}

	PD_DEBUG("sdvo: sdvo_set_tv_settings: chron_tv_code [0x%x]\n",chron_tv_code);

	return sdvo_set_tv_optimal_settings(p_ctx,chron_tv_code);

}
#endif

/*	============================================================================
	Function	:	sdvo_set_attributes is called to modify one or more display
					attributes

	Parameters	:	p_context: Pointer to port driver allocated context structure
					num     : Number of attributes
					p_list	: List of attributes

	Remarks     :	sdvo_set_attributes scans the attribute list to find the ones
					that are to be modified by checking flags field in each
					attribute for PD_ATTR_FLAG_VALUE_CHANGED bit. If this bit is
					set it will call SDVO interface functions to set the new
					value for the attribute.

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_set_attributes(void *p_context, unsigned long num_attrs,
	pd_attr_t *p_list)
{

	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	sdvo_output_flags_t out_flags;
	unsigned long i;
	static unsigned char set_tvformat = 1;
	unsigned short tvformat = 1;      /* Index 1 is for NTSC */
	int found_display_setting = 0;

	PD_DEBUG("sdvo: sdvo_set_attributes(): num_attrs=%ld", num_attrs);
	/* Search for PD_ATTR_ID_DISPLAY in the incoming attribute table, if it   */
	/* present and if sDVO supports > 1 displays use it to set TargetOutput   */
	for (i = 0; i < num_attrs; i++) {
		if ((p_list[i].flags & PD_ATTR_FLAG_VALUE_CHANGED) == 0) {
			continue;
		}

		/* overwrite the connected display with requested display */
		if (p_list[i].id == PD_ATTR_ID_DISPLAY) {
			p_ctx->out_type.flags = (unsigned short)p_list[i].current_value;

			PD_DEBUG("sdvo: Success ! sdvo_set_attributes: "
				"attr='%s', id=%ld, current_value=%ld",
				SDVO_GET_ATTR_NAME((&p_list[i])), p_list[i].id,
				p_list[i].current_value);
			/*	Clear attribute changed flag */
			p_list[i].flags &= ~PD_ATTR_FLAG_VALUE_CHANGED;
			found_display_setting = 1;
			break;
		}
	}


	/*in the situation where we have 2 potential outputs (e.g. lvds and drgb_
	 * and a preference has not been set in the xorg
	 * then we try and set an appropriate default, while outputting an error
	 * message
	 */
	if(!found_display_setting && p_ctx->st_sdvo){
			int defaultFound = 0;
			printk("\n[EMGD] SDVO: ERROR !\n"
					"\n\tALL/1/Port/2/Attr/9 is NOT set!"
					"\n\tYou may want to set the value to either"
					"\n\t\t0x40: for lvds "
					"\n\tor"
					"\n\t\t0x00: for VGA"
					"\n\tCurrent Value: %x ",
					 p_ctx->out_type.flags);

			/*attempt default setting*/
			out_flags.flags = p_ctx->out_type.flags;
			if(out_flags.out0.lvds){
				/*clear*/
				out_flags.flags = 0x0;
				/*set just lvds*/
				out_flags.out0.lvds = 1;
				defaultFound = 1;

			}
			else if(out_flags.out0.drgb){
				/*clear*/
				out_flags.flags = 0x0;
				/*set just lvds*/
				out_flags.out0.drgb = 1;
				defaultFound = 1;
			}
			/*Should out1 be allowed a seperate set up? */
			else if(out_flags.out1.drgb){
				/*clear*/
				out_flags.flags = 0x0;
				/*set just lvds*/
				out_flags.out1.drgb = 1;
				defaultFound = 1;

			}
			else if(out_flags.out1.drgb){
				/*clear*/
				out_flags.flags = 0x0;
				/*set just lvds*/
				out_flags.out1.drgb = 1;
				defaultFound = 1;

			}

			if(defaultFound)
			{
				printk("\n[EMGD] SDVO: Defaulting To: %x\n", out_flags.flags);
				p_ctx->out_type.flags = out_flags.flags;
			}
			else
			{
				printk("\n[EMGD] SDVO: Error ! no appropriate default for p_ctx->out_type.flags!\n");
			}

	}

	/* ...................................................................... */
	status = sdvo_set_target_output(p_ctx, p_ctx->out_type);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error ! sdvo_set_attributes: sdvo_set_target_output()"
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}


	/* Here will try to get the supported power states again.  For multiple
	 * displays functionality card, the supported power state will only can get
      * succesfully after the output display type been seted.
	 */
	status = sdvo_get_supported_power_states(p_ctx, &p_ctx->supp_pwr_states);

	/* ...................................................................... */
	/*	Create attribute table if not already created                         */
	if (p_ctx->p_attr_table == NULL) {
		sdvo_init_attribute_table(p_ctx);
	}

	for (i = 0; i < num_attrs; i++) {
		pd_attr_t *p_attr;
		unsigned long new_value;

		if ((p_list[i].flags & PD_ATTR_FLAG_VALUE_CHANGED) == 0) {
			continue;
		}

		/* Special handling for TVFormat in vbios */
		if (p_list[i].id == PD_ATTR_ID_TVFORMAT) {
			set_tvformat = 1;
			tvformat = (unsigned short) p_list[i].current_value;
		}

		/*	Clear attribute changed flag */
		p_list[i].flags &= ~PD_ATTR_FLAG_VALUE_CHANGED;

		new_value = p_list[i].current_value;

		p_attr = pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
						p_list[i].id, 0);
		if (p_attr == NULL) {
			PD_DEBUG("sdvo: Error ! pd_get_attr() failed for attr "
					  "id=%ld", p_list[i].id);
			continue;
		}

		/* .................................................................. */
		if(!(p_attr->flags & PD_ATTR_FLAG_DYNAMIC)) {
			status = sdvo_alter_static_attr(p_ctx, p_attr, new_value);
		} else {
			if ((p_attr->id >= PD_ATTR_ID_FP_PWR_T1) &&
				(p_attr->id <= PD_ATTR_ID_FP_PWR_T5)) {
				status = sdvo_alter_panel_pwr_seq_attr(p_ctx,
					(pd_range_attr_t *)p_attr, new_value);
			} else if (p_attr->type == PD_ATTR_TYPE_RANGE) {
				status = sdvo_alter_range_attr(p_ctx, (pd_range_attr_t *)p_attr,
					  new_value);
				if (p_attr->id == PD_ATTR_ID_VERT_OVERSCAN) {
					p_attr->flags |= PD_ATTR_FLAG_SETMODE;
				}
			} else if (p_attr->type == PD_ATTR_TYPE_BOOL) {
				status = sdvo_alter_bool_attr(p_ctx, (pd_bool_attr_t *)p_attr,
					new_value);
			} else if (p_attr->type == PD_ATTR_TYPE_LIST) {
				status = sdvo_alter_list_attr(p_ctx, (pd_list_attr_t *)p_attr,
					new_value);
				if (p_attr->id == PD_ATTR_ID_TVFORMAT) {
					p_attr->flags |= PD_ATTR_FLAG_SETMODE;
				}
				/* Note: Incoming attribute list may or maynot have the
				 * list entries, so don't skip.
				i += ((pd_list_attr_t *)p_attr)->num_entries;
				p_attr += ((pd_list_attr_t *)p_attr)->num_entries; */

			} else if (p_attr->type == PD_ATTR_TYPE_LIST_ENTRY) {
				/* Skip the list entries */
			} else {
				PD_ERROR("sdvo: Error ! sdvo_set_attributes: "
					  "Unknown attr='%s', id=%ld, type=%ld",
					  SDVO_GET_ATTR_NAME(p_attr), p_attr->id, p_attr->type);
				status = SS_UNSUCCESSFUL;
			}
		}

		/* .................................................................. */
		if (status == SS_SUCCESS) {
			p_attr->current_value = new_value;
			PD_DEBUG("sdvo: Success ! sdvo_set_attributes: "
				"attr='%s', id=%ld, current_value=%ld",
				SDVO_GET_ATTR_NAME(p_attr), p_attr->id, p_attr->current_value);

		} else {
			PD_ERROR("sdvo: Error ! sdvo_set_attributes: Failed to set "
				"attr='%s', id=%ld", SDVO_GET_ATTR_NAME(p_attr), p_attr->id);
		}
	}

#ifdef CONFIG_MICRO
	if ((p_ctx->out_type.flags & TV_DISP_MASK) && set_tvformat) {
		unsigned char params[8] = {0,0,0,0,0,0,0,0};
		/* Special handling of TVFormat attribute in VBIOS, because
		 * it wasn't enabled in the list attributes. */
		/* TVFormat    Byte       Byte
		 *             Number     Value
		 * --------    ------     -----
		 *     1       0          0x01
		 *     2       0          0x02
		 *     3       0          0x04
		 *     4       0          0x08
		 *     5       0          0x10
		 *     6       0          0x20
		 *     7       0          0x40
		 *     8       0          0x80
		 *     9       1          0x01
		 *     10      1          0x02
		 *     11      1          0x04
		 *     12      1          0x08
		 *     13      1          0x10
		 *     ...
		 *
		 *     40      4          0x80
		 *     41      5          0x01
		 *     42      5          0x02
		 *     43      5          0x04
		 *     44      5          0x08
		 *     45      5          0x10
		 */
		tvformat %= 46;      /* TVformat <= 45 */
		params[tvformat/8] = 1<<((tvformat%8)-1);
		status = sdvo_execute_command(p_ctx, SET_TV_OUTPUT_FORMAT, 8,
			params, 0, NULL);
		PD_DEBUG("sdvo_set_attributes(): Set TVFormat = %u:"
			"params=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x status=%u",
			tvformat,
			params[0], params[1], params[2],
			params[3], params[4], params[5],
			params[6], params[7], status);
		set_tvformat = 0;
	}
#endif
	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_set_power is called to change the power state of the
					device

	Parameters	:	p_context: Pointer to port driver allocated context structure
					state	: New power state

	Remarks     :

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_set_power(void *p_context, unsigned long state)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_output_flags_t active_outputs;
	sdvo_status_t status = SS_SUCCESS;
	unsigned short power_state;

	PD_DEBUG("sdvo: sdvo_set_power(): requested state=%#lx", state);

	/* Bits 7-4 in supp_pwr_states are display power states:
	 * Find next supported low power state if requested state isn't
	 * supported. */
	power_state = 0x1 << state;
	for (;power_state<=0x8; power_state<<=1) {
		if ((p_ctx->supp_pwr_states>>4) & power_state) {
			break;
		}
	}

	/* Set display power state if it is a supported power state */
	if (power_state <= 0x80) {
		if (!p_ctx->st_sdvo) {
			status = sdvo_set_display_power_state(p_ctx, (i2c_reg_t) power_state);
		}
		PD_DEBUG("sdvo: sdvo_set_power: "
			"display_power_state = %#x status = %d",
			(i2c_reg_t)power_state, status);
	} else {
		PD_DEBUG("sdvo: sdvo_set_power: No supported display power state");
	}

	/* Bits 3-0 in supp_pwr_states are device power states:
	 * Find next supported low power state if requested state isn't
	 * supported. */
	power_state = 0x1 << state;
	for (;power_state<=0x8; power_state<<=1) {
		if (p_ctx->supp_pwr_states & power_state) {
			break;
		}
	}

	/* To avoid LCD flickering avoid setting the current state */
	if (sdvo_get_power_state(p_ctx, &p_ctx->device_pwr_state) == SS_SUCCESS) {
		if ((p_ctx->device_pwr_state & 0x0f) == power_state) {
			PD_DEBUG("sdvo: sdvo_set_power: Device is already"
				"in %u power state", power_state);
			return PD_SUCCESS;
		}
	}


	/*	Disable the output if status is not POWER ON */
	if (state != PD_POWER_MODE_D0) {
		status = sdvo_get_active_outputs(p_ctx, &active_outputs);
		if (status != SS_SUCCESS) {
			PD_ERROR("sdvo: Error ! sdvo_set_power: "
				"sdvo_get_active_outputs() failed with status=%d",
				status);
		} else {
			active_outputs.flags = active_outputs.flags &
				(~p_ctx->out_type.flags);
			status = sdvo_set_active_outputs(p_ctx, active_outputs);
			if (status != SS_SUCCESS) {
				PD_ERROR("sdvo: Error ! sdvo_set_power: "
					"sdvo_set_active_outputs() failed with status=%d",
					status);
			}
			/* Wait for panel power down sequence to complete */
			pd_usleep((unsigned long)(p_ctx->t3+p_ctx->t4+p_ctx->t5)*1000 + 10);
		}
	}

	/* Set device power state if it is a supported power state */
	if (power_state <= 0x8) {
		if (!p_ctx->st_sdvo) {
			status = sdvo_set_power_state(p_ctx, (i2c_reg_t)power_state);
		}
		PD_DEBUG("sdvo: sdvo_set_power: "
			"device_power_state = %#x status = %d",
			(i2c_reg_t)power_state, status);
		if (status == SS_SUCCESS || status == SS_POWER_ON_STATE) {
			p_ctx->device_pwr_state = (unsigned char) power_state;
		}
	} else {
		PD_DEBUG("sdvo: sdvo_set_power: No supported device power state");
	}

#ifdef DEBUG
	{
		unsigned char pstate;
		status = sdvo_get_power_state(p_ctx, &pstate);
		PD_DEBUG("sdvo: sdvo_set_power: new sdvo pwr_state=%#x", power_state);
	}
#endif
	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_get_power is called to get the current power state

	Parameters	:	p_context: Pointer to port driver allocated context structure
					p_state	: Returns the current power state

	Remarks     :

	Returns     :	PD_SUCCESS	On Success
					PD_ERR_xxx	On Failure
	------------------------------------------------------------------------- */
int sdvo_get_power(void *p_context, unsigned long *p_state)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status;
	unsigned char power_state;

	PD_DEBUG("sdvo: sdvo_get_power()");

	status = sdvo_get_power_state(p_ctx, &power_state);
	if (status != SS_SUCCESS) {
		PD_ERROR("sdvo: Error! sdvo_get_power_state() failed with status=%d",
			status);
	}

	/* ...................................................................... */
	PD_DEBUG("sdvo: Current Power state=%#x", power_state);

	/*
	 * power_state has both the display and the encoder power state.
	 * The display state is in the upper nybble and the encoder state
	 * is in the lower nybble.  Currently we only care about the
	 * encoder state so mask off the upper mybble.
	 */
	switch ((power_state & 0x0f)) {
		case 0x1:        /* Bit 0 */
			*p_state = PD_POWER_MODE_D0;
			break;
		case 0x2:        /* Bit 1 */
			*p_state = PD_POWER_MODE_D1;
			break;
		case 0x4:        /* Bit 2 */
			*p_state = PD_POWER_MODE_D2;
			break;
		case 0x8:        /* Bit 3 */
		default:
			*p_state = PD_POWER_MODE_D3;
			break;
	}

	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_save is called to save the default state of registers

	Parameters	:	p_context: Pointer to port driver allocated context structure
					pp_state : Returs a pointer to list of SDVO registers
					terminated with PD_REG_LIST_END.
					flags	: Not used

	Remarks		:	sdvo_save does not save any registers.

	Returns		:	PD_SUCCESS
	------------------------------------------------------------------------- */
int sdvo_save(void *p_context, void **pp_state, unsigned long flags)
{
#ifndef CONFIG_MICRO
	sdvo_device_context_t *pd_context = (sdvo_device_context_t *)p_context;
	sdvo_state_t   *reg_state;
	int            ret, status;
	sdvo_dtd_t p_dtd;

	PD_DEBUG("sdvo: sdvo_save()");
	/* Allocate memory to save the current regs */
	reg_state = (sdvo_state_t *) pd_malloc(sizeof(sdvo_state_t));
	if (!reg_state) {
		return (PD_ERR_NOMEM);
	}
	pd_memset(reg_state, 0, sizeof(sdvo_state_t));

	/* Save the current state */
	ret = sdvo_get_power(pd_context, &(reg_state->power_state));
	if (ret) {
		pd_free(reg_state);
		return ret;
	}

	/* Save current timings */
	ret = sdvo_set_power(p_context, PD_POWER_MODE_D0);
	if ((status = sdvo_get_input_timings(pd_context, &p_dtd)) != SS_SUCCESS) {
		if (status != SS_SUCCESS) {
			/* Standard VGA mode 3+ timings */
			p_dtd.pixel_clock = 2832;
			p_dtd.horz_active = 720;
			p_dtd.horz_blanking = 162;
			p_dtd.vert_active = 400;
			p_dtd.vert_blanking = 35;
			p_dtd.horz_sync_offset = 45;
			p_dtd.horz_sync_pulse_width = 108;
			p_dtd.vert_sync_offset = 12;
			p_dtd.vert_sync_pulse_width = 2;
			p_dtd.dtd_flags = 0x1c;
			p_dtd.sdvo_flags = 0;
			PD_ERROR("sdvo: Failed to save current timings - command not "
						"supported.");
			PD_ERROR("sdvo: Standard 720x400 VGA timings will be used.");
		}
	}
	sdvo_convert_dtd_to_pd_timing(&p_dtd, &(reg_state->timing));

	/* In case someone calls save again without restore, free the previous
     * state */
	if (NULL != *pp_state) {
		pd_free(*pp_state);
	}

	*pp_state = (void *)reg_state;

	/* restore power state, just in case */
	ret = sdvo_set_power(pd_context, reg_state->power_state);
#endif
	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_restore is called to restore the registers which were
					save previously via a call to sdvo_save

	Parameters	:	p_context: Pointer to port driver allocated context structure
					p_state	: List of SDVO registers
					flags	: Not used

	Remarks	:

	Returns	:	PD_SUCCESS
	------------------------------------------------------------------------- */
int sdvo_restore(void *p_context, void *p_state, unsigned long flags)
{
#ifndef CONFIG_MICRO
	sdvo_device_context_t *pd_context = (sdvo_device_context_t *)p_context;
	sdvo_state_t *reg_state = (sdvo_state_t *)p_state;
	pd_attr_t attr_tmp;
	int ret;
	unsigned long i;
	/*i2c_reg_t reg;*/



	PD_DEBUG("sdvo: sdvo_restore()");
	if (!reg_state) {
		PD_ERROR("restore got a NULL saved state.");
		return PD_ERR_NULL_STATE;
	}

	sdvo_reset_encoder(p_context);    /* Reset the sdvo device to known state for good
							   * start. */
	/* Add the code to process the CH7022 card */
	if ((pd_context->dev_cap.vendor_id == 0x02) &&
		((pd_context->dev_cap.device_id  == 0xc2) ||
		 (pd_context->dev_cap.device_id  == 0x42)) &&
		((pd_context->p_attr_table[0].current_value == 0x04) ||
		 (pd_context->p_attr_table[0].current_value == 0x08))) {
		ret = sdvo_set_target_input(p_context, pd_context->inp_dev);
		ret = sdvo_set_target_output(p_context, pd_context->out_type);
		ret = sdvo_set_power(p_context, PD_POWER_MODE_D0);
		ret = sdvo_set_power(p_context, PD_POWER_MODE_D3);

		/* CH7021: Initialize H/V Overscan */
		if (pd_context->out_type.flags & TV_DISP_MASK) {
			sdvo_write_i2c_reg(p_context, 0x38, 0x1F);
			sdvo_write_i2c_reg(p_context, 0x39, 0x1F);
		}
	} else {
		/*
		 * 0X5Acircuitry enable register for 7308
		 * BIT0=1 triggers power up sequence
		 * BIT0=0 triggers power down sequence
		 * BIT3=1 enable bit for circuitry access
		 */
		/* removing this as this seems to cause screen corruption after killx */
		/*if ((pd_context->dev_cap.vendor_id == 2) && (pd_context->dev_cap.device_id == 0x41)){
			sdvo_read_i2c_reg(pd_context, 0x5A, &reg);
			reg |= 9;
			sdvo_write_i2c_reg(pd_context, 0x5A, reg);
		}*/
#endif
		/* Restore the saved mode */
		ret = sdvo_set_power(p_context, PD_POWER_MODE_D0);
		reg_state->timing.refresh = 70;  /* Is refresh needed? */
		sdvo_set_mode(p_context, &(reg_state->timing), 0);
		sdvo_post_set_mode(p_context, &(reg_state->timing), 0);
		if (!(flags & PD_NO_RESTORE_FREE_STATE)) {
			/* Restore the power state */
			ret = sdvo_set_power(p_context, reg_state->power_state);
			if (ret) {
				return ret;
			}
		}
	}

	/* restore the sdvo attributes. */
	/* The below should be sufficient for general sdvo + ch7308 */
	/* TODO, add more attributes as neccessary for other cards */
	for(i=0;i<pd_context->num_attrs;i++) {
		switch (pd_context->p_attr_table[i].id) {
		case PD_ATTR_ID_2_CHANNEL_PANEL:
		case PD_ATTR_ID_LVDS_PANEL_TYPE:
		case PD_ATTR_ID_DITHER:
		case PD_ATTR_ID_PANEL_PROTECT_HSYNC:
		case PD_ATTR_ID_PANEL_PROTECT_VSYNC:
		case PD_ATTR_ID_TEXT_TUNING:
		case PD_ATTR_ID_PANEL_DEPTH:
		case PD_ATTR_ID_SSC:
		case PD_ATTR_ID_DISPLAY:
		case PD_ATTR_ID_FP_PWR_T1:
		case PD_ATTR_ID_FP_PWR_T2:
		case PD_ATTR_ID_FP_PWR_T3:
		case PD_ATTR_ID_FP_PWR_T4:
		case PD_ATTR_ID_FP_PWR_T5:
			pd_context->p_attr_table[i].flags |= PD_ATTR_FLAG_VALUE_CHANGED;
			sdvo_set_attributes(p_context, 1, &pd_context->p_attr_table[i]);
			break;

		/* TV format is a list attribute, need to alter in different way */
		case PD_ATTR_ID_TVFORMAT:
			if (pd_context->p_attr_table[i].type == PD_ATTR_TYPE_LIST) {
				pd_memcpy(&attr_tmp, &pd_context->p_attr_table[i],
							sizeof(pd_attr_t));
				attr_tmp.flags |= PD_ATTR_FLAG_VALUE_CHANGED;
				sdvo_set_attributes(p_context, 1, &attr_tmp);
			}
		default:
			break;
		}
	}

	pd_free(p_state);
	p_state = NULL;

	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_validate

	Parameters	:	cookie

	Remarks     :	sdvo_Valite returns the cookie it received as an argument

	Returns     :	cookie
	------------------------------------------------------------------------- */
unsigned long sdvo_validate(unsigned long cookie)
{
	PD_DEBUG("sdvo: sdvo_validate()");
	return cookie;
}


/*	============================================================================
	Function	:	sdvo_close is the last function to be called in the port
					driver

	Parameters	:	p_context: Pointer to port driver allocated context structure

	Remarks     :

	Returns     :	PD_SUCCESS
	------------------------------------------------------------------------- */
int sdvo_close(void *p_context)
{
#ifndef CONFIG_MICRO
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	/* Shutdown device if it is tv to avoid flickering */
	if (p_ctx->out_type.flags & TV_DISP_MASK) {
		sdvo_set_power(p_context, PD_POWER_MODE_D3);
	}
	PD_DEBUG("sdvo: sdvo_close()");

	if (p_ctx->p_attr_table != NULL) {

		pd_free(p_ctx->p_attr_table);
		p_ctx->p_attr_table = NULL;
		p_ctx->num_attrs	 = 0;
	}

	if (p_ctx->p_mode_table) {
		pd_free(p_ctx->p_mode_table);
		p_ctx->p_mode_table = NULL;
	}
	pd_free(p_ctx);
#endif
	return PD_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_get_port_status is called to get the status of the
					display

	Parameters	:	p_context: Pointer to port driver allocated context structure
					port_status : Returns display type and connection state

	Returns     :	PD_SUCCESS or PD_ERR_XXX
	------------------------------------------------------------------------- */
int sdvo_get_port_status(void *p_context, pd_port_status_t *port_status)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_output_flags_t out_flags;
	sdvo_display_output_t disp_out;
	sdvo_status_t status;
	unsigned long  pwr_state;

	port_status->display_type = 0;
	port_status->connected	  = PD_DISP_STATUS_UNKNOWN;


	/* Save the current power state and turn on the port device's power
	 * so we can query it
	 */
	sdvo_get_power(p_context, &pwr_state);
	sdvo_set_power(p_context, PD_POWER_MODE_D0);

	status = sdvo_get_attached_displays(p_ctx, &out_flags);

	sdvo_set_power(p_context, pwr_state);

	if (status != SS_SUCCESS) {
		/* TODO: Certain SDVO cards (CH7022) would return SS_PENDING. Need to
		 * Identify correct setting so the card behaves correctly. For now
		 * we would fallback to out_type kept in the context.
		 */
		if(status == SS_PENDING){
			out_flags.flags = p_ctx->out_type.flags;
		}else{
			PD_ERROR("sdvo: Error ! sdvo_get_port_status: "
				"sdvo_get_attached_displays() failed with status=%d", status);
			return PD_ERR_INTERNAL;
		}
	}

	if (p_ctx->st_sdvo) {
		out_flags.flags = p_ctx->out_type.flags;

		if (p_ctx->out_type.flags == 0x40) {
			out_flags.out0.lvds = 1;
			out_flags.out0.drgb = 0;
			out_flags.out1.lvds = 0;
			out_flags.out1.drgb = 0;
		} else {
			out_flags.out0.lvds = 0;
			out_flags.out0.drgb = 1;
			out_flags.out1.lvds = 0;
			out_flags.out1.drgb = 0;
		}
	}

	if (p_ctx->inp_dev == SDVO0) {
		disp_out = out_flags.out0;
	} else {
		disp_out = out_flags.out1;
	}

	port_status->connected = PD_DISP_STATUS_ATTACHED;

	if (disp_out.tmds) {
		status = SDVO_IS_HDMI_SUPPORTED(p_ctx);
		if (status == SS_SUCCESS) {
			PD_DEBUG("sdvo: HDMI transmiter ");
			g_sdvo_drv.type = PD_DISPLAY_HDMI_EXT;
			port_status->display_type = PD_DISPLAY_HDMI_EXT;
		} else{
			g_sdvo_drv.type	= PD_DISPLAY_FP;
			port_status->display_type = PD_DISPLAY_FP;
		}
	} else if (disp_out.lvds) {
		port_status->display_type = PD_DISPLAY_LVDS_EXT;
	} else if (disp_out.rgb) {
		port_status->display_type = PD_DISPLAY_CRT_EXT;
	} else if (disp_out.drgb) {
		port_status->display_type = PD_DISPLAY_DRGB;
	} else if (disp_out.cvbs || disp_out.scart ||
			   disp_out.svid || disp_out.yprpb) {
		port_status->display_type = PD_DISPLAY_TVOUT;
	} else {
		port_status->connected = PD_DISP_STATUS_DETACHED;
	}
	return PD_SUCCESS;
}

char *sdvo_get_output_type(sdvo_display_output_t out, unsigned char byte)
{
	static char buff[40] = "";
	int indx = 0;

	if (out.tmds) {

		pd_strcpy(&buff[indx], " tmds");
		indx += 5;
	}

	if (out.rgb) {

		pd_strcpy(&buff[indx], " rgb");
		indx += 4;
	}

	if (out.cvbs) {

		pd_strcpy(&buff[indx], " cvbs");
		indx += 5;
	}

	if (out.svid) {

		pd_strcpy(&buff[indx], " svid");
		indx += 5;
	}

	if (out.yprpb) {

		pd_strcpy(&buff[indx], " yprpb");
		indx += 6;
	}

	if (out.scart) {

		pd_strcpy(&buff[indx], " scart");
		indx += 6;
	}

	if (out.lvds) {

		pd_strcpy(&buff[indx], " lvds");
		indx += 5;
	}

	/* DRGB defined in byte0 of SDVOOutputFlagStructure */
	if (byte == 0 && out.drgb) {
		pd_strcpy(&buff[indx], " drgb");
		indx += 5;
	}

	if (indx == 0) {

		pd_strcpy(buff, " None");
	}

	return buff;
}


/*	============================================================================
	Function	:	sdvo_convert_pd_timing_to_dtd

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
void sdvo_convert_pd_timing_to_dtd(pd_timing_t *p_mode, sdvo_dtd_t *p_dtd,
							   unsigned long dclk)
{
	pd_memset(p_dtd, 0, sizeof(sdvo_dtd_t));

	p_dtd->pixel_clock			= (unsigned short)(dclk / 10);

	p_dtd->horz_active			= p_mode->width;
	p_dtd->horz_blanking		= p_mode->hblank_end - p_mode->hblank_start;
	p_dtd->horz_sync_offset		= p_mode->hsync_start - p_mode->width + 1;
	p_dtd->horz_sync_pulse_width= p_mode->hsync_end - p_mode->hsync_start;

	p_dtd->vert_active			= p_mode->height;
	p_dtd->vert_blanking		= p_mode->vblank_end - p_mode->vblank_start;
	p_dtd->vert_sync_offset		= p_mode->vsync_start - p_mode->height + 1;
	p_dtd->vert_sync_pulse_width= p_mode->vsync_end - p_mode->vsync_start;

	/* ...................................................................... */
	p_dtd->dtd_flags = (BIT(4) | BIT(3));     /*	Digital Seperate */

	if (p_mode->mode_info_flags & PD_SCAN_INTERLACE) {

		p_dtd->dtd_flags |= BIT(7);
	}

	if (p_mode->mode_info_flags & PD_HSYNC_HIGH) {	/*	HSync + */

		p_dtd->dtd_flags |= BIT(1);
	}

	if (p_mode->mode_info_flags & PD_VSYNC_HIGH) {	/*	VSync + */

		p_dtd->dtd_flags |= BIT(2);
	}

	/*	....................................................................  */
	p_dtd->sdvo_flags = 0;
}


/*	============================================================================
	Function	:	sdvo_convert_dtd_to_pd_timing

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
void sdvo_convert_dtd_to_pd_timing(sdvo_dtd_t *p_dtd, pd_timing_t *p_mode)
{
	pd_memset(p_mode, 0, sizeof(pd_timing_t));

	p_mode->dclk		= p_dtd->pixel_clock * 10L;

	p_mode->width		= p_dtd->horz_active;
	p_mode->hsync_start	= p_dtd->horz_sync_offset + p_dtd->horz_active - 1;
	p_mode->hsync_end	= p_dtd->horz_sync_pulse_width + p_mode->hsync_start;
	p_mode->hblank_start= p_dtd->horz_active - 1;
	p_mode->hblank_end	= p_dtd->horz_blanking + p_mode->hblank_start;
	p_mode->htotal		= p_dtd->horz_active + p_dtd->horz_blanking - 1;

	p_mode->height		= p_dtd->vert_active;
	p_mode->vsync_start	= p_dtd->vert_sync_offset + p_dtd->vert_active - 1;
	p_mode->vsync_end	= p_dtd->vert_sync_pulse_width + p_mode->vsync_start;
	p_mode->vblank_start= p_dtd->vert_active - 1;
	p_mode->vblank_end	= p_dtd->vert_blanking + p_mode->vblank_start;
	p_mode->vtotal		= p_dtd->vert_active + p_dtd->vert_blanking - 1;

	/*	....................................................................  */
	if (p_dtd->dtd_flags & BIT(7)) {
		p_mode->mode_info_flags |= PD_SCAN_INTERLACE;
	}

	if (p_dtd->dtd_flags & BIT(1)) {
		p_mode->mode_info_flags |= PD_HSYNC_HIGH;
	}

	if (p_dtd->dtd_flags & BIT(2)) {
		p_mode->mode_info_flags |= PD_VSYNC_HIGH;
	}
}

/*	============================================================================
	Function	:	sdvo_init_attribute_table

	Parameters	:	p_ctx

	Remarks     :

	Returns     :	PD_SUCCESS or PD_ERR_XXX
	------------------------------------------------------------------------- */
int sdvo_init_attribute_table(sdvo_device_context_t *p_ctx)
{
	unsigned long num_attrs, num_static_attrs, num_pwr_seq_attrs;
	unsigned long num_range_attrs, num_bool_attrs, num_list_attrs;
	unsigned char *p_table;
	sdvo_status_t status;

	/* .................................................................. */
	status = sdvo_set_target_input(p_ctx, p_ctx->inp_dev);
	if (status != SS_SUCCESS) {

		PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: sdvo_set_target_input()"
				  "failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}

	status = sdvo_set_target_output(p_ctx, p_ctx->out_type);
	if (status != SS_SUCCESS) {

		PD_ERROR("sdvo: Error ! sdvo_init_attribute_table:"
			"sdvo_set_target_output()failed with status=%d", status);
		return PD_ERR_INTERNAL;
	}


	/* .................................................................. */
	/*	Get the number of available attributes from the SDVO interface    */
	num_static_attrs	= sdvo_get_static_attrs(p_ctx, NULL);
	num_pwr_seq_attrs	= sdvo_get_panel_pwr_seq_attrs(p_ctx, NULL);
	num_range_attrs		= sdvo_get_range_attrs(p_ctx, NULL);
	num_bool_attrs      = sdvo_get_bool_attrs(p_ctx, NULL);
	num_list_attrs      = sdvo_get_list_attrs(p_ctx, NULL);
	p_ctx->num_attrs = num_static_attrs + num_pwr_seq_attrs +
					   num_range_attrs + num_bool_attrs + num_list_attrs;

	if (p_ctx->num_attrs == 0) {

		PD_ERROR("sdvo: Warning ! sdvo_init_attribute_table: "
				  "No attributes found");

		return PD_SUCCESS;
	}

	/* .................................................................. */
	/*	Allocate memory to save all available port attributes.            */
	/*	Allocate space for extra attribute so Query_Attr function can be   */
	/*  called using that space for the last+1 attribute.                 */
	/*  Query and save all the available attributes.                      */
	p_ctx->p_attr_table = pd_malloc((p_ctx->num_attrs + 1) * sizeof(pd_attr_t));
	if (p_ctx->p_attr_table == NULL) {

		PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
				  "pd_malloc(p_attr_table) failed");

		p_ctx->num_attrs = 0;
		return PD_ERR_NOMEM;
	}

	pd_memset(p_ctx->p_attr_table, 0, (p_ctx->num_attrs + 1) *
		sizeof(pd_attr_t));

	p_table = (unsigned char *)p_ctx->p_attr_table;

	/* .................................................................. */
	if (num_static_attrs > 0) {

		num_attrs = sdvo_get_static_attrs(p_ctx, (pd_attr_t *)p_table);
		if (num_attrs != num_static_attrs) {

			PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
					  "sdvo_get_static_attrs() returned %ld entries, expected %ld",
					  num_attrs, num_static_attrs);

		} else {

			p_table += (num_static_attrs * sizeof(pd_attr_t));
		}
	}

	if (num_pwr_seq_attrs > 0) {

		num_attrs = sdvo_get_panel_pwr_seq_attrs(p_ctx,
			(pd_range_attr_t *)p_table);
		if (num_attrs != num_pwr_seq_attrs) {

			PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
					  "sdvo_get_panel_pwr_seq_attrs() returned %ld entries,"
					  " expected %ld", num_attrs, num_pwr_seq_attrs);

		} else {

			p_table += (num_pwr_seq_attrs * sizeof(pd_attr_t));
		}
	}

	if (num_range_attrs > 0) {

		num_attrs = sdvo_get_range_attrs(p_ctx, (pd_range_attr_t *)p_table);
		if (num_attrs != num_range_attrs) {

			PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
					  "sdvo_get_range_attrs() returned %ld entries,"
					  " expected %ld", num_attrs, num_range_attrs);

		} else {

			p_table += (num_range_attrs * sizeof(pd_attr_t));
		}
	}

	if (num_bool_attrs > 0) {

		num_attrs = sdvo_get_bool_attrs(p_ctx, (pd_bool_attr_t *)p_table);
		if (num_attrs != num_bool_attrs) {

			PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
					  "sdvo_get_bool_attrs() returned %ld entries, expected %ld",
					  num_attrs, num_bool_attrs);

		} else {

			p_table += (num_bool_attrs * sizeof(pd_attr_t));
		}
	}

	if (num_list_attrs > 0) {

		num_attrs = sdvo_get_list_attrs(p_ctx, (pd_list_attr_t *)p_table);
		if (num_attrs != num_list_attrs) {

			PD_ERROR("sdvo: Error ! sdvo_init_attribute_table: "
					  "sdvo_get_list_attrs() returned %ld entries, expected %ld",
					  num_attrs, num_list_attrs);
		} else {

			p_table += (num_list_attrs * sizeof(pd_attr_t));
		}
	}
	return PD_SUCCESS;
}

int sdvo_is_multi_display_device(sdvo_device_context_t *p_ctx)
{
	unsigned short i;
	int b_multi_display = 0;

	for (i = 0; i < 16; i++) {
		if (p_ctx->dev_cap.output.flags & (1 << i)) {
			if (!b_multi_display) {
				b_multi_display = 1;
			} else {
				return TRUE;
			}
		}
	}
	return 0;
}

#ifndef CONFIG_MICRO
static sdvo_status_t sdvo_reset_encoder(sdvo_device_context_t *p_ctx)
{
	sdvo_status_t ret_stat;
	if ((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS)
		&& (p_ctx->dev_cap.device_id == 0x1)){

		pd_attr_t *p_attr_temp =
			pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
				PD_ATTR_ID_RB_SWAP_MODE, 0 );

		if (!(p_attr_temp->flags & PD_ATTR_FLAG_NEED_RESTORE)){
			sdvo_query_static_attr(p_ctx, p_attr_temp);
			p_attr_temp->flags |= PD_ATTR_FLAG_NEED_RESTORE;
		}



		sdvo_reset (p_ctx);
		pd_usleep(10);
		sdvo_reset (p_ctx);

		ret_stat = sdvo_reset (p_ctx);

	} else {
		ret_stat = sdvo_reset (p_ctx);
	}

	return ret_stat;
}
#endif


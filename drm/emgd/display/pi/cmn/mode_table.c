/*
 *-----------------------------------------------------------------------------
 * Filename: mode_table.c
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
 *  This file contains the mode resolution parameters.
 *-----------------------------------------------------------------------------
 */

#define TRUE 1
#define FALSE 0

#include <config.h>
#include <mode.h>

/*
 * If CONFIG_LIMIT_MODES is not defined then ALL modes should be included.
 * Keep the defines here so that when adding new modes it is apparent that
 * you should add it to the list below.
 */
#ifndef CONFIG_LIMIT_MODES
#define CONFIG_MODE_640x480x60
#define CONFIG_MODE_640x480x70
#define CONFIG_MODE_640x480x72
#define CONFIG_MODE_640x480x75
#define CONFIG_MODE_640x480x85
#define CONFIG_MODE_640x480x100
#define CONFIG_MODE_640x480x120
#define CONFIG_MODE_720x480x60
#define CONFIG_MODE_720x576x50
#define CONFIG_MODE_800x480x60
#define CONFIG_MODE_800x600x60
#define CONFIG_MODE_800x600x70
#define CONFIG_MODE_800x600x72
#define CONFIG_MODE_800x600x75
#define CONFIG_MODE_800x600x85
#define CONFIG_MODE_800x600x100
#define CONFIG_MODE_800x600x120
#define CONFIG_MODE_960x540x60
#define CONFIG_MODE_1024x768x60
#define CONFIG_MODE_1024x768x70
#define CONFIG_MODE_1024x768x75
#define CONFIG_MODE_1024x768x85
#define CONFIG_MODE_1024x768x100
#define CONFIG_MODE_1024x768x120
#define CONFIG_MODE_1152x864x60
#define CONFIG_MODE_1152x864x70
#define CONFIG_MODE_1152x864x72
#define CONFIG_MODE_1152x864x75
#define CONFIG_MODE_1152x864x85
#define CONFIG_MODE_1152x864x100
#define CONFIG_MODE_1280x720x60
#define CONFIG_MODE_1280x720x75
#define CONFIG_MODE_1280x720x85
#define CONFIG_MODE_1280x720x100
#define CONFIG_MODE_1280x768x60
#define CONFIG_MODE_1280x768x75
#define CONFIG_MODE_1280x768x85
#define CONFIG_MODE_1280x960x60
#define CONFIG_MODE_1280x960x75
#define CONFIG_MODE_1280x960x85
#define CONFIG_MODE_1280x1024x60
#define CONFIG_MODE_1280x1024x70
#define CONFIG_MODE_1280x1024x72
#define CONFIG_MODE_1280x1024x75
#define CONFIG_MODE_1280x1024x85
#define CONFIG_MODE_1280x1024x100
#define CONFIG_MODE_1280x1024x120
#define CONFIG_MODE_1366x768x60
#define CONFIG_MODE_1400x1050x60
#define CONFIG_MODE_1400x1050x75
#define CONFIG_MODE_1400x1050x85
#define CONFIG_MODE_1600x900x60
#define CONFIG_MODE_1600x900x75
#define CONFIG_MODE_1600x900x85
#define CONFIG_MODE_1600x900x100
#define CONFIG_MODE_1600x900x120
#define CONFIG_MODE_1600x1200x60
#define CONFIG_MODE_1600x1200x65
#define CONFIG_MODE_1600x1200x70
#define CONFIG_MODE_1600x1200x72
#define CONFIG_MODE_1600x1200x75
#define CONFIG_MODE_1600x1200x85
#define CONFIG_MODE_1600x1200x100
#define CONFIG_MODE_1600x1200x120
#define CONFIG_MODE_1856x1392x60
#define CONFIG_MODE_1856x1392x75
#define CONFIG_MODE_1920x1080x50
#define CONFIG_MODE_1920x1080x60
#define CONFIG_MODE_1920x1080x75
#define CONFIG_MODE_1920x1080x85
#define CONFIG_MODE_1920x1080x100
#define CONFIG_MODE_1920x1200x60
#define CONFIG_MODE_1920x1200x75
#define CONFIG_MODE_1920x1440x60
#define CONFIG_MODE_1920x1440x75
#define CONFIG_MODE_1920x1440x85
#define CONFIG_MODE_2048x1536x60
#define CONFIG_MODE_2048x1536x75
/* CEA timings */
#define CONFIG_CEA_MODE_640x480px60
#ifndef CONFIG_MICRO
/* These are timings that has duplicate except the difference in aspect
   ratio. Since IEGD does not support diffrent aspect ratio removing
   these CEA timings for now */
/*
#define CONFIG_CEA_MODE_720x480px60
#define CONFIG_CEA_MODE_720x576px50*/
#define CONFIG_CEA_MODE_720x480p_ax60
#define CONFIG_CEA_MODE_720x576p_ax50
#define CONFIG_CEA_MODE_1280x720p_ax50
#define CONFIG_CEA_MODE_1280x720p_ax60
#define CONFIG_CEA_MODE_1920x1080p_ax50
#define CONFIG_CEA_MODE_1920x1080p_ax60
#define CONFIG_CEA_MODE_1920x1080i_ax50
#define CONFIG_CEA_MODE_1920x1080i_ax60
#endif /* CONFIG_MICRO */

#endif

/*---------------------------------------------------------------------------
 * Timing tables for CRT modes.
 *---------------------------------------------------------------------------
 */

igd_timing_info_t crt_timing_table[] =
{
/* TODO: Add VESA standard REDUCED BLANKING (RB) timings to the table */
#ifdef CONFIG_MODE_640x480x60
	{
		640, 480,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		647, 791,              /* hblank_start, hblank_end */
		655, 751,              /* hsync_start, hsync_end */
		524,                   /* vtotal */
		487, 516,              /* vblank_start, vblank_end */
		489, 491,              /* vsync_start, vsync_end */
		0x101,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x70
	{
		640, 480,              /* width, height */
		70, 28560,             /* refresh, dot clock */
		815,                   /* htotal */
		639, 815,              /* hblank_start, hblank_end */
		663, 727,              /* hsync_start, hsync_end */
		499,                   /* vtotal */
		479, 499,              /* vblank_start, vblank_end */
		480, 483,              /* vsync_start, vsync_end */
		0x101,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x72
	{
		640, 480,              /* width, height */
		72, 31500,             /* refresh, dot clock */
		831,                   /* htotal */
		647, 823,              /* hblank_start, hblank_end */
		663, 703,              /* hsync_start, hsync_end */
		519,                   /* vtotal */
		487, 511,              /* vblank_start, vblank_end */
		488, 491,              /* vsync_start, vsync_end */
		0x101,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x75
	{
		640, 480,              /* width, height */
		75, 31500,             /* refresh, dot clock */
		839,                   /* htotal */
		639, 839,              /* hblank_start, hblank_end */
		655, 719,              /* hsync_start, hsync_end */
		499,                   /* vtotal */
		479, 499,              /* vblank_start, vblank_end */
		480, 483,              /* vsync_start, vsync_end */
		0x101,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x85
	{
		640, 480,              /* width, height */
		85, 36000,             /* refresh, dot clock */
		831,                   /* htotal */
		639, 831,              /* hblank_start, hblank_end */
		695, 751,              /* hsync_start, hsync_end */
		508,                   /* vtotal */
		479, 508,              /* vblank_start, vblank_end */
		480, 483,              /* vsync_start, vsync_end */
		0x101,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x100
	{
		640, 480,              /* width, height */
		100, 43163,            /* refresh, dot clock */
		847,                   /* htotal */
		639, 847,              /* hblank_start, hblank_end */
		679, 743,              /* hsync_start, hsync_end */
		508,                   /* vtotal */
		479, 508,              /* vblank_start, vblank_end */
		480, 483,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_640x480x120
	{
		640, 480,              /* width, height */
		120, 52406,            /* refresh, dot clock */
		847,                   /* htotal */
		639, 847,              /* hblank_start, hblank_end */
		679, 743,              /* hsync_start, hsync_end */
		514,                   /* vtotal */
		479, 514,              /* vblank_start, vblank_end */
		480, 483,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_720x400x70
	/* NOTE: The above define is not auto-enabled. This is the VGA magic mode */
	{   /* VGA Mode 2+,3+ */
		720, 400,              /* width, height */
		70, 28322,             /* refresh, dot clock */
		899,                   /* htotal */
		719, 899,              /* hblank_start, hblank_end */
		773, 881,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		399, 448,              /* vblank_start, vblank_end */
		412, 414,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_VSYNC_HIGH |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_720x480x60
	{
		720, 480,              /* width, height */
		60, 27000,             /* refresh, dot clock */
		857,                   /* htotal */
		719, 857,              /* hblank_start, hblank_end */
		735, 797,              /* hsync_start, hsync_end */
		524,                   /* vtotal */
		479, 524,              /* vblank_start, vblank_end */
		488, 494,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_720x576x50
	{
		720, 576,              /* width, height */
		50, 27500,             /* refresh, dot clock */
		864,                   /* htotal */
		719, 864,              /* hblank_start, hblank_end */
		732, 795,              /* hsync_start, hsync_end */
		625,                   /* vtotal */
		575, 625,              /* vblank_start, vblank_end */
		581, 587,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
	/* OFF BY Default, enable when PLB is merged */
#ifdef CONFIG_MODE_800x480x60
	{
		800, 480,              /* width, height */
		60, 33231,             /* refresh, dot clock */
		1055,                  /* htotal */
		799, 1055,             /* hblank_start, hblank_end */
		863, 991,              /* hsync_start, hsync_end */
		524,                   /* vtotal */
		479, 524,              /* vblank_start, vblank_end */
		500, 502,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x60
	{
		800, 600,              /* width, height */
		60, 40000,             /* refresh, dot clock */
		1055,                  /* htotal */
		799, 1055,             /* hblank_start, hblank_end */
		839, 967,              /* hsync_start, hsync_end */
		627,                   /* vtotal */
		599, 627,              /* vblank_start, vblank_end */
		600, 604,              /* vsync_start, vsync_end */
		0x103,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x70
	{
		800, 600,              /* width, height */
		70, 45500,             /* refresh, dot clock */
		1039,                  /* htotal */
		799, 1039,             /* hblank_start, hblank_end */
		839, 919,              /* hsync_start, hsync_end */
		624,                   /* vtotal */
		599, 624,              /* vblank_start, vblank_end */
		600, 603,              /* vsync_start, vsync_end */
		0x103,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x72
	{
		800, 600,              /* width, height */
		72, 50000,             /* refresh, dot clock */
		1039,                  /* htotal */
		799, 1039,             /* hblank_start, hblank_end */
		855, 975,              /* hsync_start, hsync_end */
		665,                   /* vtotal */
		599, 665,              /* vblank_start, vblank_end */
		636, 642,              /* vsync_start, vsync_end */
		0x103,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x75
	{
		800, 600,              /* width, height */
		75, 49500,             /* refresh, dot clock */
		1055,                  /* htotal */
		799, 1055,             /* hblank_start, hblank_end */
		815, 895,              /* hsync_start, hsync_end */
		624,                   /* vtotal */
		599, 624,              /* vblank_start, vblank_end */
		600, 603,              /* vsync_start, vsync_end */
		0x103,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x85
	{
		800, 600,              /* width, height */
		85, 56250,             /* refresh, dot clock */
		1047,                  /* htotal */
		799, 1047,             /* hblank_start, hblank_end */
		831, 895,              /* hsync_start, hsync_end */
		630,                   /* vtotal */
		599, 630,              /* vblank_start, vblank_end */
		600, 603,              /* vsync_start, vsync_end */
		0x103,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x100
	{
		800, 600,              /* width, height */
		100, 68179,            /* refresh, dot clock */
		1071,                  /* htotal */
		799, 1071,             /* hblank_start, hblank_end */
		847, 935,              /* hsync_start, hsync_end */
		635,                   /* vtotal */
		599, 635,              /* vblank_start, vblank_end */
		600, 603,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_800x600x120
	{
		800, 600,              /* width, height */
		120, 83950,            /* refresh, dot clock */
		1087,                  /* htotal */
		799, 1087,             /* hblank_start, hblank_end */
		855, 943,              /* hsync_start, hsync_end */
		642,                   /* vtotal */
		599, 642,              /* vblank_start, vblank_end */
		600, 603,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_960x540x60
	{
		960, 540,              /* width, height */
		60, 40785,             /* refresh, dot clock */
		1215,                  /* htotal */
		959, 1215,             /* hblank_start, hblank_end */
		991, 1087,             /* hsync_start, hsync_end */
		558,                   /* vtotal */
		539, 558,              /* vblank_start, vblank_end */
		540, 543,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x60
	{
		1024, 768,             /* width, height */
		60, 65000,             /* refresh, dot clock */
		1343,                  /* htotal */
		1023, 1343,            /* hblank_start, hblank_end */
		1047, 1183,            /* hsync_start, hsync_end */
		805,                   /* vtotal */
		767, 805,              /* vblank_start, vblank_end */
		770, 776,              /* vsync_start, vsync_end */
		0x105,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x70
	{
		1024, 768,             /* width, height */
		70, 75000,             /* refresh, dot clock */
		1327,                  /* htotal */
		1023, 1327,            /* hblank_start, hblank_end */
		1047, 1183,            /* hsync_start, hsync_end */
		805,                   /* vtotal */
		767, 805,              /* vblank_start, vblank_end */
		770, 776,              /* vsync_start, vsync_end */
		0x105,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x75
	{
		1024, 768,             /* width, height */
		75, 78750,             /* refresh, dot clock */
		1311,                  /* htotal */
		1023, 1311,            /* hblank_start, hblank_end */
		1039, 1135,            /* hsync_start, hsync_end */
		799,                   /* vtotal */
		767, 799,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x105,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x85
	{
		1024, 768,             /* width, height */
		85, 94500,             /* refresh, dot clock */
		1375,                  /* htotal */
		1023, 1375,            /* hblank_start, hblank_end */
		1071, 1167,            /* hsync_start, hsync_end */
		807,                   /* vtotal */
		767, 807,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x105,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x100
	{
		1024, 768,             /* width, height */
		100, 113310,           /* refresh, dot clock */
		1391,                  /* htotal */
		1023, 1391,            /* hblank_start, hblank_end */
		1095, 1207,            /* hsync_start, hsync_end */
		813,                   /* vtotal */
		767, 813,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1024x768x120
	{
		1024, 768,             /* width, height */
		120, 139050,           /* refresh, dot clock */
		1407,                  /* htotal */
		1023, 1407,            /* hblank_start, hblank_end */
		1103, 1215,            /* hsync_start, hsync_end */
		822,                   /* vtotal */
		767, 822,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity H+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x60
	{
		1152, 864,             /* width, height */
		60, 81624,             /* refresh, dot clock */
		1519,                  /* htotal */
		1151, 1519,            /* hblank_start, hblank_end */
		1215, 1335,            /* hsync_start, hsync_end */
		894,                   /* vtotal */
		863, 894,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x70
	{
		1152, 864,             /* width, height */
		70, 97000,             /* refresh, dot clock */
		1535,                  /* htotal */
		1151, 1535,            /* hblank_start, hblank_end */
		1223, 1343,            /* hsync_start, hsync_end */
		899,                   /* vtotal */
		863, 899,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x72
	{
		1152, 864,             /* width, height */
		72, 100000,            /* refresh, dot clock */
		1535,                  /* htotal */
		1151, 1535,            /* hblank_start, hblank_end */
		1223, 1343,            /* hsync_start, hsync_end */
		900,                   /* vtotal */
		863, 900,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x75
	{
		1152, 864,             /* width, height */
		75, 108000,            /* refresh, dot clock */
		1599,                  /* htotal */
		1151, 1599,            /* hblank_start, hblank_end */
		1215, 1343,            /* hsync_start, hsync_end */
		899,                   /* vtotal */
		863, 899,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x85
	{
		1152, 864,             /* width, height */
		85, 120000,            /* refresh, dot clock */
		1551,                  /* htotal */
		1151, 1551,            /* hblank_start, hblank_end */
		1223, 1351,            /* hsync_start, hsync_end */
		906,                   /* vtotal */
		863, 906,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1152x864x100
	{
		1152, 864,             /* width, height */
		100, 144000,           /* refresh, dot clock */
		1567,                  /* htotal */
		1151, 1567,            /* hblank_start, hblank_end */
		1231, 1359,            /* hsync_start, hsync_end */
		914,                   /* vtotal */
		863, 914,              /* vblank_start, vblank_end */
		864, 867,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x720x60
	{
		1280, 720,             /* width, height */
		60, 74481,             /* refresh, dot clock */
		1663,                  /* htotal */
		1279, 1663,            /* hblank_start, hblank_end */
		1335, 1471,            /* hsync_start, hsync_end */
		745,                   /* vtotal */
		719, 745,              /* vblank_start, vblank_end */
		720, 723,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x720x75
	{
		1280, 720,             /* width, height */
		75, 96000,             /* refresh, dot clock */
		1695,                  /* htotal */
		1279, 1695,            /* hblank_start, hblank_end */
		1351, 1487,            /* hsync_start, hsync_end */
		751,                   /* vtotal */
		719, 751,              /* vblank_start, vblank_end */
		720, 723,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x720x85
	{
		1280, 720,             /* width, height */
		85, 110000,            /* refresh, dot clock */
		1711,                  /* htotal */
		1279, 1711,            /* hblank_start, hblank_end */
		1359, 1495,            /* hsync_start, hsync_end */
		755,                   /* vtotal */
		719, 755,              /* vblank_start, vblank_end */
		720, 723,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x720x100
	{
		1280, 720,             /* width, height */
		100, 131850,           /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1367, 1503,            /* hsync_start, hsync_end */
		762,                   /* vtotal */
		719, 762,              /* vblank_start, vblank_end */
		720, 723,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x768x60
	{
		1280, 768,             /* width, height */
		60, 80136,             /* refresh, dot clock */
		1679,                  /* htotal */
		1279, 1679,            /* hblank_start, hblank_end */
		1343, 1479,            /* hsync_start, hsync_end */
		794,                   /* vtotal */
		767, 794,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x768x75
	{
		1280, 768,             /* width, height */
		75, 102977,            /* refresh, dot clock */
		1711,                  /* htotal */
		1279, 1711,            /* hblank_start, hblank_end */
		1359, 1495,            /* hsync_start, hsync_end */
		801,                   /* vtotal */
		767, 801,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x768x85
	{
		1280, 768,             /* width, height */
		85, 118532,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1367, 1503,            /* hsync_start, hsync_end */
		806,                   /* vtotal */
		767, 806,              /* vblank_start, vblank_end */
		768, 771,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x960x60
	{
		1280, 960,             /* width, height */
		60, 108000,            /* refresh, dot clock */
		1799,                  /* htotal */
		1279, 1799,            /* hblank_start, hblank_end */
		1375, 1487,            /* hsync_start, hsync_end */
		999,                   /* vtotal */
		959, 999,              /* vblank_start, vblank_end */
		960, 963,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x960x75
	{
		1280, 960,             /* width, height */
		75, 129859,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1367, 1503,            /* hsync_start, hsync_end */
		1001,                  /* vtotal */
		959, 1001,             /* vblank_start, vblank_end */
		960, 963,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x960x85
	{
		1280, 960,             /* width, height */
		85, 148500,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1343, 1503,            /* hsync_start, hsync_end */
		1010,                  /* vtotal */
		959, 1010,             /* vblank_start, vblank_end */
		960, 963,              /* vsync_start, vsync_end */
		0x00,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x60
	{
		1280, 1024,            /* width, height */
		60, 108000,            /* refresh, dot clock */
		1687,                  /* htotal */
		1279, 1687,            /* hblank_start, hblank_end */
		1327, 1439,            /* hsync_start, hsync_end */
		1065,                  /* vtotal */
		1023, 1065,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x107,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x70
	{
		1280, 1024,            /* width, height */
		70, 129000,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1367, 1503,            /* hsync_start, hsync_end */
		1065,                  /* vtotal */
		1023, 1065,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x107,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x72
	{
		1280, 1024,            /* width, height */
		72, 133000,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1367, 1503,            /* hsync_start, hsync_end */
		1066,                  /* vtotal */
		1023, 1066,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x107,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x75
	{
		1280, 1024,            /* width, height */
		75, 135000,            /* refresh, dot clock */
		1687,                  /* htotal */
		1279, 1687,            /* hblank_start, hblank_end */
		1295, 1439,            /* hsync_start, hsync_end */
		1065,                  /* vtotal */
		1023, 1065,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x107,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x85
	{
		1280, 1024,            /* width, height */
		85, 157500,            /* refresh, dot clock */
		1727,                  /* htotal */
		1279, 1727,            /* hblank_start, hblank_end */
		1343, 1503,            /* hsync_start, hsync_end */
		1071,                  /* vtotal */
		1023, 1071,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x107,                 /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x100
	{
		1280, 1024,            /* width, height */
		100, 190960,           /* refresh, dot clock */
		1759,                  /* htotal */
		1279, 1759,            /* hblank_start, hblank_end */
		1375, 1519,            /* hsync_start, hsync_end */
		1084,                  /* vtotal */
		1023, 1084,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1280x1024x120
	{
		1280, 1024,            /* width, height */
		120, 233790,           /* refresh, dot clock */
		1775,                  /* htotal */
		1279, 1775,            /* hblank_start, hblank_end */
		1383, 1527,            /* hsync_start, hsync_end */
		1096,                  /* vtotal */
		1023, 1096,            /* vblank_start, vblank_end */
		1024, 1027,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1366x768x60
	{
		1366, 768,             /* width, height */
		60, 74057,             /* refresh, dot clock */
		1663,                  /* htotal */
		1365, 1663,            /* hblank_start, hblank_end */
		1429, 1557,            /* hsync_start, hsync_end */
		775,                   /* vtotal */
		767, 775,              /* vblank_start, vblank_end */
		768, 769,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH |       /* polarity V+ */
		IGD_HSYNC_HIGH,        /* polarity H+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif

#ifdef CONFIG_MODE_1400x1050x60
	{
		1400, 1050,            /* width, height */
		60, 122000,            /* refresh, dot clock */
		1879,                  /* htotal */
		1399, 1879,            /* hblank_start, hblank_end */
		1487, 1639,            /* hsync_start, hsync_end */
		1086,                  /* vtotal */
		1049, 1086,            /* vblank_start, vblank_end */
		1050, 1053,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1400x1050x75
	{
		1400, 1050,            /* width, height */
		75, 155851,            /* refresh, dot clock */
		1895,                  /* htotal */
		1399, 1895,            /* hblank_start, hblank_end */
		1495, 1647,            /* hsync_start, hsync_end */
		1095,                  /* vtotal */
		1049, 1095,            /* vblank_start, vblank_end */
		1050, 1053,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1400x1050x85
	{
		1400, 1050,            /* width, height */
		85, 179260,            /* refresh, dot clock */
		1911,                  /* htotal */
		1399, 1911,            /* hblank_start, hblank_end */
		1503, 1655,            /* hsync_start, hsync_end */
		1102,                  /* vtotal */
		1049, 1102,            /* vblank_start, vblank_end */
		1050, 1053,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x900x60
	{
		1600, 900,             /* width, height */
		60, 119000,            /* refresh, dot clock */
		2127,                  /* htotal */
		1599, 2127,            /* hblank_start, hblank_end */
		1695, 1863,            /* hsync_start, hsync_end */
		931,                   /* vtotal */
		899, 931,              /* vblank_start, vblank_end */
		900, 903,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x900x75
	{
		1600, 900,             /* width, height */
		75, 152000,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1703, 1879,            /* hsync_start, hsync_end */
		939,                   /* vtotal */
		899, 939,              /* vblank_start, vblank_end */
		900, 903,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x900x85
	{
		1600, 900,             /* width, height */
		85, 175000,            /* refresh, dot clock */
		2175,                  /* htotal */
		1599, 2175,            /* hblank_start, hblank_end */
		1711, 1887,            /* hsync_start, hsync_end */
		944,                   /* vtotal */
		899, 944,              /* vblank_start, vblank_end */
		900, 903,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x900x100
	{
		1600, 900,             /* width, height */
		100, 208900,           /* refresh, dot clock */
		2191,                  /* htotal */
		1599, 2191,            /* hblank_start, hblank_end */
		1719, 1895,            /* hsync_start, hsync_end */
		952,                   /* vtotal */
		899, 952,              /* vblank_start, vblank_end */
		900, 903,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x900x120
	{
		1600, 900,             /* width, height */
		120, 255686,           /* refresh, dot clock */
		2207,                  /* htotal */
		1599, 2207,            /* hblank_start, hblank_end */
		1727, 1903,            /* hsync_start, hsync_end */
		964,                   /* vtotal */
		899, 964,              /* vblank_start, vblank_end */
		900, 903,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x60
	{
		1600, 1200,            /* width, height */
		60, 162000,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1663, 1855,            /* hsync_start, hsync_end */
		1249,                  /* vtotal */
		1199, 1249,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x65
	{
		1600, 1200,            /* width, height */
		65, 175500,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1663, 1855,            /* hsync_start, hsync_end */
		1249,                  /* vtotal */
		1199, 1249,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x70
	{
		1600, 1200,            /* width, height */
		70, 189000,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1663, 1855,            /* hsync_start, hsync_end */
		1249,                  /* vtotal */
		1199, 1249,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x72
	{
		1600, 1200,            /* width, height */
		72, 195000,            /* refresh, dot clock */
		2175,                  /* htotal */
		1599, 2175,            /* hblank_start, hblank_end */
		1711, 1887,            /* hsync_start, hsync_end */
		1250,                  /* vtotal */
		1199, 1250,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x75
	{
		1600, 1200,            /* width, height */
		75, 202500,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1663, 1855,            /* hsync_start, hsync_end */
		1249,                  /* vtotal */
		1199, 1249,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x85
	{
		1600, 1200,            /* width, height */
		85, 229500,            /* refresh, dot clock */
		2159,                  /* htotal */
		1599, 2159,            /* hblank_start, hblank_end */
		1663, 1855,            /* hsync_start, hsync_end */
		1249,                  /* vtotal */
		1199, 1249,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_HSYNC_HIGH|        /* polarity H+ */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x100
	{
		1600, 1200,            /* width, height */
		100, 280640,           /* refresh, dot clock */
		2207,                  /* htotal */
		1599, 2207,            /* hblank_start, hblank_end */
		1727, 1903,            /* hsync_start, hsync_end */
		1270,                  /* vtotal */
		1199, 1270,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1600x1200x120
	{
		1600, 1200,            /* width, height */
		120, 343210,           /* refresh, dot clock */
		2223,                  /* htotal */
		1599, 2223,            /* hblank_start, hblank_end */
		1735, 1911,            /* hsync_start, hsync_end */
		1285,                  /* vtotal */
		1199, 1285,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1856x1392x60
	{
		1856, 1392,            /* width, height */
		60, 218250,            /* refresh, dot clock */
		2527,                  /* htotal */
		1855, 2527,            /* hblank_start, hblank_end */
		1951, 2175,            /* hsync_start, hsync_end */
		1438,                  /* vtotal */
		1391, 1438,            /* vblank_start, vblank_end */
		1392, 1395,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1856x1392x75
	{
		1856, 1392,            /* width, height */
		75, 288000,            /* refresh, dot clock */
		2559,                  /* htotal */
		1855, 2559,            /* hblank_start, hblank_end */
		1983, 2207,            /* hsync_start, hsync_end */
		1499,                  /* vtotal */
		1391, 1499,            /* vblank_start, vblank_end */
		1392, 1395,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1080x50
	{
		1920, 1080,            /* width, height */
		50, 141446,            /* refresh, dot clock */
		2543,                  /* htotal */
		1919, 2543,            /* hblank_start, hblank_end */
		2031, 2231,            /* hsync_start, hsync_end */
		1111,                  /* vtotal */
		1079, 1111,            /* vblank_start, vblank_end */
		1080, 1083,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1080x60
	{
		1920, 1080,            /* width, height */
		60, 172800,            /* refresh, dot clock */
		2575,                  /* htotal */
		1919, 2575,            /* hblank_start, hblank_end */
		2039, 2247,            /* hsync_start, hsync_end */
		1117,                  /* vtotal */
		1079, 1117,            /* vblank_start, vblank_end */
		1080, 1083,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1080x75
	{
		1920, 1080,            /* width, height */
		75, 220640,            /* refresh, dot clock */
		2607,                  /* htotal */
		1919, 2607,            /* hblank_start, hblank_end */
		2055, 2263,            /* hsync_start, hsync_end */
		1127,                  /* vtotal */
		1079, 1127,            /* vblank_start, vblank_end */
		1080, 1083,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1080x85
	{
		1920, 1080,            /* width, height */
		85, 252930,            /* refresh, dot clock */
		2623,                  /* htotal */
		1919, 2623,            /* hblank_start, hblank_end */
		2063, 2271,            /* hsync_start, hsync_end */
		1133,                  /* vtotal */
		1079, 1133,            /* vblank_start, vblank_end */
		1080, 1083,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1080x100
	{
		1920, 1080,            /* width, height */
		100, 302020,           /* refresh, dot clock */
		2639,                  /* htotal */
		1919, 2639,            /* hblank_start, hblank_end */
		2071, 2279,            /* hsync_start, hsync_end */
		1143,                  /* vtotal */
		1079, 1143,            /* vblank_start, vblank_end */
		1080, 1083,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1200x60
	{
		1920, 1200,            /* width, height */
		60, 193156,            /* refresh, dot clock */
		2591,                  /* htotal */
		1919, 2591,            /* hblank_start, hblank_end */
		2047, 2255,            /* hsync_start, hsync_end */
		1241,                  /* vtotal */
		1199, 1241,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1200x75
	{
		1920, 1200,            /* width, height */
		75, 246590,            /* refresh, dot clock */
		2623,                  /* htotal */
		1919, 2623,            /* hblank_start, hblank_end */
		2063, 2271,            /* hsync_start, hsync_end */
		1252,                  /* vtotal */
		1199, 1252,            /* vblank_start, vblank_end */
		1200, 1203,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1440x60
	{
		1920, 1440,            /* width, height */
		60, 234000,            /* refresh, dot clock */
		2599,                  /* htotal */
		1919, 2599,            /* hblank_start, hblank_end */
		2047, 2255,            /* hsync_start, hsync_end */
		1499,                  /* vtotal */
		1439, 1499,            /* vblank_start, vblank_end */
		1440, 1443,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1440x75
	{
		1920, 1440,            /* width, height */
		75, 297000,            /* refresh, dot clock */
		2639,                  /* htotal */
		1919, 2639,            /* hblank_start, hblank_end */
		2063, 2287,            /* hsync_start, hsync_end */
		1499,                  /* vtotal */
		1439, 1499,            /* vblank_start, vblank_end */
		1440, 1443,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_1920x1440x85
	{
		1920, 1440,            /* width, height */
		85, 341350,            /* refresh, dot clock */
		2655,                  /* htotal */
		1919, 2655,            /* hblank_start, hblank_end */
		2071, 2287,            /* hsync_start, hsync_end */
		1511,                  /* vtotal */
		1439, 1511,            /* vblank_start, vblank_end */
		1440, 1443,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_2048x1536x60
	{
		2048, 1536,            /* width, height */
		60, 266950,            /* refresh, dot clock */
		2799,                  /* htotal */
		2047, 2799,            /* hblank_start, hblank_end */
		2199, 2423,            /* hsync_start, hsync_end */
		1588,                  /* vtotal */
		1535, 1588,            /* vblank_start, vblank_end */
		1536, 1539,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_MODE_2048x1536x75
	{
		2048, 1536,            /* width, height */
		75, 340480,            /* refresh, dot clock */
		2831,                  /* htotal */
		2047, 2831,            /* hblank_start, hblank_end */
		2215, 2439,            /* hsync_start, hsync_end */
		1602,                  /* vtotal */
		1535, 1602,            /* vblank_start, vblank_end */
		1536, 1539,            /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE|  /* mode info flags */
		IGD_VSYNC_HIGH,        /* polarity V+ */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
	PD_TIMING_TABLE_END
};

int crt_timing_table_size = sizeof(crt_timing_table);

/*---------------------------------------------------------------------------
 * Timing tables for VGA modes
 *---------------------------------------------------------------------------
 */
igd_timing_info_t vga_timing_table[] =
{
	{   /* VGA Mode 0 */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x00,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_LINE_DOUBLE |
		IGD_PIXEL_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 1 */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x01,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_LINE_DOUBLE |
		IGD_PIXEL_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 2 */
		640, 200,              /* width, height */
		70, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		679, 775,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x02,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 3 */
		640, 200,              /* width, height */
		70, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		679, 775,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x03,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 4 */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x04,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 5 */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x05,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 6 */
		640, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 784,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x06,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 7 */
		720, 350,              /* width, height */
		60, 28322,             /* refresh, dot clock */
		899,                   /* htotal */
		719, 881,              /* hblank_start, hblank_end */
		764, 872,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x07,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode d */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x0d,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_PIXEL_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode e */
		640, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x0e,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode f */
		640, 350,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x11,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 10 */
		640, 350,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x12,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 0* */
		320, 350,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x13,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_PIXEL_DOUBLE |
		IGD_MODE_TEXT |
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 1* */
		320, 350,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		399,                   /* htotal */
		319, 383,              /* hblank_start, hblank_end */
		343, 383,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x14,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_PIXEL_DOUBLE |
		IGD_MODE_TEXT |
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 2* */
		640, 350,              /* width, height */
		70, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		679, 775,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x15,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 3* */
		640, 350,              /* width, height */
		70, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		679, 775,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		354, 441,              /* vblank_start, vblank_end */
		386, 388,              /* vsync_start, vsync_end */
		0x16,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_HSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 0+,1+ */
		360, 400,              /* width, height */
		60, 28322,             /* refresh, dot clock */
		449,                   /* htotal */
		359, 431,              /* hblank_start, hblank_end */
		386, 431,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x17,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_PIXEL_DOUBLE |
		IGD_MODE_TEXT |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 2+,3+ */
		720, 400,              /* width, height */
		70, 28322,             /* refresh, dot clock */
		899,                   /* htotal */
		719, 881,              /* hblank_start, hblank_end */
		764, 872,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x18,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 7+ */
		720, 400,              /* width, height */
		60, 28322,             /* refresh, dot clock */
		899,                   /* htotal */
		719, 881,              /* hblank_start, hblank_end */
		764, 872,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x19,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_TEXT |
		IGD_VSYNC_HIGH |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 11 */
		640, 480,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		524,                   /* vtotal */
		486, 515,              /* vblank_start, vblank_end */
		489, 491,              /* vsync_start, vsync_end */
		0x1A,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 12 */
		640, 480,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		524,                   /* vtotal */
		486, 515,              /* vblank_start, vblank_end */
		489, 491,              /* vsync_start, vsync_end */
		0x1B,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
	{   /* VGA Mode 13 */
		320, 200,              /* width, height */
		60, 25175,             /* refresh, dot clock */
		799,                   /* htotal */
		639, 783,              /* hblank_start, hblank_end */
		671, 767,              /* hsync_start, hsync_end */
		448,                   /* vtotal */
		405, 440,              /* vblank_start, vblank_end */
		411, 413,              /* vsync_start, vsync_end */
		0x1C,                  /* mode number */
		IGD_MODE_VESA |        /* VESA/VGA mode */
		IGD_LINE_DOUBLE |
		IGD_PIXEL_DOUBLE |
		IGD_MODE_SUPPORTED |
		IGD_SCAN_PROGRESSIVE,  /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},

	PD_TIMING_TABLE_END
};

/* CEA standard timings: Get them from CEA-861. This is following DPG.
   We will replace the mode number to CEA*/
igd_timing_info_t cea_timing_table[] =
{
#ifdef CONFIG_CEA_MODE_640x480px60
	{
		640, 480,              /* width, height */
		60, 25200,             /* refresh, dot clock */
		800,                   /* htotal */
		640, 799,              /* hblank_start, hblank_end */
		656, 751,              /* hsync_start, hsync_end */
		525,                   /* vtotal */
		480, 524,              /* vblank_start, vblank_end */
		490, 491,              /* vsync_start, vsync_end */
		1,					   /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA,		   /* mode info flags */
		0, 0,                  /* x, y offset */
		0,				   /* device dependant */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_720x480px60
	{
		720, 480,              /* width, height */
		60, 27000,             /* refresh, dot clock */
		858,                   /* htotal */
		720, 857,              /* hblank_start, hblank_end */
		736, 797,              /* hsync_start, hsync_end */
		525,                   /* vtotal */
		480, 524,              /* vblank_start, vblank_end */
		489, 494,              /* vsync_start, vsync_end */
		2,					   /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA,		   /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_720x480p_ax60
	{
		720, 480,              /* width, height */
		60, 27000,             /* refresh, dot clock */
		858,                   /* htotal */
		720, 857,              /* hblank_start, hblank_end */
		736, 797,              /* hsync_start, hsync_end */
		525,                   /* vtotal */
		480, 524,              /* vblank_start, vblank_end */
		489, 494,              /* vsync_start, vsync_end */
		3,                     /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA |          /* mode info flags */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_720x576px50
	{
		720, 576,              /* width, height */
		50, 27000,             /* refresh, dot clock */
		864,                   /* htotal */
		720, 863,              /* hblank_start, hblank_end */
		732, 795,              /* hsync_start, hsync_end */
		625,                   /* vtotal */
		575, 624,              /* vblank_start, vblank_end */
		581, 585,              /* vsync_start, vsync_end */
		17,                    /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA,		   /* mode info flags */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_720x576p_ax50
	{
		720, 576,              /* width, height */
		50, 27000,             /* refresh, dot clock */
		864,                   /* htotal */
		720, 863,              /* hblank_start, hblank_end */
		732, 795,              /* hsync_start, hsync_end */
		625,                   /* vtotal */
		575, 624,              /* vblank_start, vblank_end */
		581, 585,              /* vsync_start, vsync_end */
		18,                    /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA |		   /* mode info flags */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_1280x720p_ax50
	{
		1280, 720,             /* width, height */
		50, 74250,             /* refresh, dot clock */
		1980,                  /* htotal */
		1280, 1979,            /* hblank_start, hblank_end */
		1720, 1759,            /* hsync_start, hsync_end */
		750,                   /* vtotal */
		720, 749,              /* vblank_start, vblank_end */
		725, 729,              /* vsync_start, vsync_end */
		19,                    /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE |
		PD_MODE_CEA |		   /* mode info flags */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_1280x720p_ax60
	{
		1280, 720,             /* width, height */
		60, 74250,             /* refresh, dot clock */
		1650,                  /* htotal */
		1280, 1649,            /* hblank_start, hblank_end */
		1390, 1429,            /* hsync_start, hsync_end */
		750,                   /* vtotal */
		720, 749,              /* vblank_start, vblank_end */
		725, 729,              /* vsync_start, vsync_end */
		4,                     /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE | /* mode info flags */
		IGD_VSYNC_HIGH |
		PD_MODE_CEA |		   /* polarity V+ */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
/* KIV. Not proper timimgs, rerefer to DPG updates */
#ifdef CONFIG_CEA_MODE_1920x1080p_ax50
	{
		1920, 1080,            /* width, height */
		50, 148500,            /* refresh, dot clock */
		2640,                  /* htotal */
		1920, 2639,            /* hblank_start, hblank_end */
		2448, 2491,            /* hsync_start, hsync_end */
		1125,                  /* vtotal */
		1080, 1124,            /* vblank_start, vblank_end */
		1084, 1088,            /* vsync_start, vsync_end */
		31,                    /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE | /* mode info flags */
		IGD_VSYNC_HIGH |
		PD_MODE_CEA |		   /* polarity V+ */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_1920x1080p_ax60
	{
		1920, 1080,            /* width, height */
		60, 148500,            /* refresh, dot clock */
		2200,                  /* htotal */
		1920, 2199,            /* hblank_start, hblank_end */
		2008, 2051,            /* hsync_start, hsync_end */
		1125,                  /* vtotal */
		1080, 1124,            /* vblank_start, vblank_end */
		1084, 1088,            /* vsync_start, vsync_end */
		16,                     /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_PROGRESSIVE | /* mode info flags */
		IGD_VSYNC_HIGH |
		PD_MODE_CEA |	       /* polarity V+ */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_1920x1080i_ax50
	{
		1920, 1080,            /* width, height */
		25, 74250,             /* refresh, dot clock */
		2640,                  /* htotal */
		1920, 2639,            /* hblank_start, hblank_end */
		2448, 2491,            /* hsync_start, hsync_end */
		1124,                  /* vtotal */
		1080, 1123,            /* vblank_start, vblank_end */
		1084, 1093,            /* vsync_start, vsync_end */
		20,					   /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_INTERLACE |   /* mode info flags */
		IGD_VSYNC_HIGH |
		PD_MODE_CEA |		   /* polarity V+ */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
#ifdef CONFIG_CEA_MODE_1920x1080i_ax60
	{
		1920, 1080,            /* width, height */
		30, 74250,             /* refresh, dot clock */
		2200,                  /* htotal */
		1920, 2199,            /* hblank_start, hblank_end */
		2008, 2051,            /* hsync_start, hsync_end */
		1124,                  /* vtotal */
		1080, 1123,            /* vblank_start, vblank_end */
		1084, 1093,            /* vsync_start, vsync_end */
		5,					   /* mode number */
		PD_MODE_SUPPORTED |    /* enable the mode */
		IGD_SCAN_INTERLACE |   /* mode info flags */
		IGD_VSYNC_HIGH |
		PD_MODE_CEA |		   /* polarity V+ */
		PD_ASPECT_16_9,        /* Aspect ratio 16:9 */
		0, 0,                  /* x, y offset */
		NULL,                  /* mode extension pointer */
		0,				   /* device dependant */
		0				   /* device dependant */
	},
#endif
	PD_TIMING_TABLE_END
};

#ifndef CONFIG_MICRO
int cea_timing_table_size = sizeof(cea_timing_table);

/* CEA timings for parsing purpose */
type_std_t cea_std_lookup[] =
{
	{  640,    400,   60,    0 },
	{  720,    480,   60,    0 },
	{  720,    480,   60,    PD_ASPECT_16_9 },
	{ 1280,    720,   60,    PD_ASPECT_16_9 },
	{ 1920,   1080,   60,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    480,   60,    PD_SCAN_INTERLACE },
	{  720,    480,   60,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    240,   60,    0 },

	{  720,    240,   60,    PD_ASPECT_16_9 },
	{ 2880,    480,   60,    PD_SCAN_INTERLACE },
	{ 2880,    480,   60,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 2880,    240,   60,    0 },
	{ 2880,    240,   60,    PD_ASPECT_16_9 },
	{ 1440,    480,   60,    0 },
	{ 1440,    480,   60,    PD_ASPECT_16_9 },
	{ 1920,   1080,   60,    PD_ASPECT_16_9 },

	{  720,    576,   50,    0 },
	{  720,    576,   50,    PD_ASPECT_16_9 },
	{ 1280,    720,   50,    PD_ASPECT_16_9 },
	{ 1920,   1080,   50,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    576,   50,    PD_SCAN_INTERLACE },
	{  720,    576,   50,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    288,   50,    0 },
	{  720,    288,   50,    PD_ASPECT_16_9 },

	{ 2880,    576,   50,    PD_SCAN_INTERLACE },
	{ 2880,    576,   50,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 2880,    288,   50,    0 },
	{ 2880,    288,   50,    PD_ASPECT_16_9 },
	{ 1440,    576,   50,    0 },
	{ 1440,    576,   50,    PD_ASPECT_16_9 },
	{ 1920,   1080,   50,    PD_ASPECT_16_9 },
	{ 1920,   1080,   24,    PD_ASPECT_16_9 },

	{ 1920,   1080,   25,    PD_ASPECT_16_9 },
	{ 1920,   1080,   30,    PD_ASPECT_16_9 },
	{ 2880,    480,   60,    0 },
	{ 2880,    480,   60,    PD_ASPECT_16_9 },
	{ 2880,    576,   50,    0 },
	{ 2880,    576,   50,    PD_ASPECT_16_9 },
	{ 1920,   1080,   50,    PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 1920,   1080,   100,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },

	{ 1280,    720,   100,   PD_ASPECT_16_9 },
	{  720,    576,   100,   0 },
	{  720,    576,   100,   PD_ASPECT_16_9 },
	{  720,    576,   100,   PD_SCAN_INTERLACE },
	{  720,    576,   100,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 1920,   1080,   120,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 1280,    720,   120,   PD_ASPECT_16_9 },
	{  720,    480,   120,   0 },

	{  720,    480,   120,   PD_ASPECT_16_9 },
	{  720,    480,   120,   PD_SCAN_INTERLACE },
	{  720,    480,   120,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    576,   200,   0 },
	{  720,    576,   200,   PD_ASPECT_16_9 },
	{  720,    576,   200,   PD_SCAN_INTERLACE },
	{  720,    576,   200,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{  720,    480,   240,   0 },

	{  720,    480,   240,   PD_ASPECT_16_9 },
	{  720,    480,   240,   PD_SCAN_INTERLACE },
	{  720,    480,   240,   PD_SCAN_INTERLACE | PD_ASPECT_16_9 },
	{ 1280,    720,   24,    PD_ASPECT_16_9 },
	{ 1280,    720,   25,    PD_ASPECT_16_9 },
	{ 1280,    720,   30,    PD_ASPECT_16_9 },
};

int cea_std_lookup_size = sizeof(cea_std_lookup)/sizeof(type_std_t);

#endif

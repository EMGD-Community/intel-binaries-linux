/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_attr.c
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
 *  Functions to
 *  Create Display Attribute table
 *  Query and Alter attributes
 *-----------------------------------------------------------------------------
 */

#include "sdvo_port.h"

/* .......................................................................... */
#ifndef CONFIG_MICRO
#define MAKE_NAME(x)    x
#else
#define MAKE_NAME(x)    NULL
#endif


/* .......................................................................... */
/*    Panel Power Sequencing Parameters */
static pd_range_attr_t g_panel_power_attr[] =
{
		/*       ID                      Type                 Name */
		/*       Flags   Default Value   Current Value   Min  Max Step */
	PD_MAKE_ATTR(PD_ATTR_ID_FP_PWR_T1,   PD_ATTR_TYPE_RANGE, "FP Power T1",
				PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 1),

	PD_MAKE_ATTR(PD_ATTR_ID_FP_PWR_T2,   PD_ATTR_TYPE_RANGE, "FP Power T2",
				PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 1),

	PD_MAKE_ATTR(PD_ATTR_ID_FP_PWR_T3,   PD_ATTR_TYPE_RANGE, "FP Power T3",
				PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 1),

	PD_MAKE_ATTR(PD_ATTR_ID_FP_PWR_T4,   PD_ATTR_TYPE_RANGE, "FP Power T4",
				PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 1),

	PD_MAKE_ATTR(PD_ATTR_ID_FP_PWR_T5,   PD_ATTR_TYPE_RANGE, "FP Power T5",
				PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 1),
};

static pd_range_attr_t g_fp_attr[] = {
	/* Do not change the order of entries */
		/*       ID                     Type                Name */
		/* Flags  Default Value   Current Value   Min  Max Step */
	PD_MAKE_ATTR(PD_ATTR_ID_DISPLAY,    PD_ATTR_TYPE_RANGE, "Display Type",
		 PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0x4000, 1),

	PD_MAKE_ATTR(PD_ATTR_ID_PANEL_FIT,  PD_ATTR_TYPE_BOOL,  "PanelFit",
		 PD_ATTR_FLAG_USER_INVISIBLE, 0, 0, 0, 0, 0),

	PD_MAKE_ATTR(PD_ATTR_ID_TEXT_TUNING,PD_ATTR_TYPE_RANGE, "Text Enhancement",
		0,            0,              0,           0,  4,  1),

	PD_MAKE_ATTR(PD_ATTR_ID_RB_SWAP_MODE, PD_ATTR_TYPE_BOOL,"RB Swap", 0, 0x00,	0x00, 0, 0, 0),
};
#ifndef CONFIG_MICRO
static sdvo_tv_optimal_settings_t g_sdtv_op_settings[] =
/*	chron_tv_code [7:0]
 *		bits[7:2] = tv_format
 *			480i = 0x00 ; 576i = 0x24 ; other TV line formats = 0xFF => for higher
 *       resolution, YPbPr is the implied connector
 *		bits [1:0] = output type
 *			CVBS = 0x00, SVIDEO = 0x01, YPbPr = 0x02, SCART = 0x03
 *	chron_tv_code,		hue,		saturation,		brightness,		contrast */
{
	/* SDTV [480i, 576i] - analog format w/
	 * CVBS / SVIDEO / SCART / YPbPr connector */
	/* NTSC_M Composite */
	{0x00,	0x3F,		0x35,			0x81,			0x2F},
	/* NTSC_M Svideo */
	{0x01,	0x3D,		0x30,			0x81,			0x2F},
	/* PAL_B/D/G/H/I/K/L Composite */
	{0x24,	0x41,		0x38,			0x74,			0x3B},
	/* PAL_B/D/G/H/I/K/L Svideo */
	{0x25,	0x3F,		0x30,			0x74,			0x3B},
	/* SECAM xxx- */

	/* EDTV [ 480p, 576p] + HDTV [720p, 1080i/p] ) -
	 * analog format w/ YPbPr connector */
	{0xFF,	0x40,		0x45,			0x80,			0x2C},
};
#endif
/* .......................................................................... */
static sdvo_range_attr_data_t g_range_data[] =
/*  ID                          Name                              Bytes*/
/*  opcode_max                  opcode_get              opcode_set */
{
 {PD_ATTR_ID_BRIGHTNESS,        MAKE_NAME("Brightness"),            2,
  GET_MAX_BRIGHTNESS,           GET_BRIGHTNESS,         SET_BRIGHTNESS},

 {PD_ATTR_ID_PANEL_DEPTH,       MAKE_NAME("Panel Depth"),           1,
  GET_LVDS_PANEL_INFORMATION,   GET_LVDS_PANEL_INFORMATION,
													SET_LVDS_PANEL_INFORMATION},
#ifndef CONFIG_MICRO
 {PD_ATTR_ID_FP_BACKLIGHT_EN,   MAKE_NAME("Backlight Level"),       1,
  GET_MAX_BACKLIGHT_LEVEL,      GET_BACKLIGHT_LEVEL,    SET_BACKLIGHT_LEVEL},

 {PD_ATTR_ID_FLICKER,			MAKE_NAME("Flicker Filter"),		2,
  GET_MAX_FLICKER_FILTER,		GET_FLICKER_FILTER, 	SET_FLICKER_FILTER},

 {PD_ATTR_ID_2D_FLICKER,		MAKE_NAME("2D Flicker Filter"),		2,
  GET_MAX_2D_FLICKER_FILTER,	GET_2D_FLICKER_FILTER,	SET_2D_FLICKER_FILTER},

 {PD_ATTR_ID_SATURATION,		MAKE_NAME("Saturation"),				2,
  GET_MAX_SATURATION,			GET_SATURATION, 		SET_SATURATION},

 {PD_ATTR_ID_HUE,				MAKE_NAME("Hue"),					2,
  GET_MAX_HUE,					GET_HUE,				SET_HUE},

 {PD_ATTR_ID_CONTRAST,			MAKE_NAME("Contrast"), 			2,
  GET_MAX_CONTRAST, 			GET_CONTRAST,			SET_CONTRAST},

 {PD_ATTR_ID_HORZ_OVERSCAN, 	MAKE_NAME("Horz Overscan"),			2,
  GET_MAX_HORIZONTAL_OVERSCAN,  GET_HORIZONTAL_OVERSCAN,SET_HORIZONTAL_OVERSCAN},

 {PD_ATTR_ID_VERT_OVERSCAN, 	MAKE_NAME("Vert Overscan"),			2,
  GET_MAX_VERTICAL_OVERSCAN,	GET_VERTICAL_OVERSCAN,	SET_VERTICAL_OVERSCAN},

 {PD_ATTR_ID_HPOSITION, 		MAKE_NAME("Horz Position"),			2,
  GET_MAX_HORIZONTAL_POSITION,  GET_HORIZONTAL_POSITION,SET_HORIZONTAL_POSITION},

 {PD_ATTR_ID_VPOSITION, 		MAKE_NAME("Vert Position"),			2,
  GET_MAX_VERTICAL_POSITION,	GET_VERTICAL_POSITION,	SET_VERTICAL_POSITION},

 {PD_ATTR_ID_SHARPNESS, 		MAKE_NAME("Sharpness"),				2,
  GET_MAX_SHARPNESS,			GET_SHARPNESS,			SET_SHARPNESS},

 {PD_ATTR_ID_CHROMA_FILTER, 	MAKE_NAME("TV Chroma Filter"), 		2,
  GET_MAX_TV_CHROMA_FILTER, 	GET_TV_CHROMA_FILTER,	SET_TV_CHROMA_FILTER},

 {PD_ATTR_ID_LUMA_FILTER,		MAKE_NAME("TV Luma Filter"),		2,
  GET_MAX_TV_LUMA_FILTER,		GET_TV_LUMA_FILTER, 	SET_TV_LUMA_FILTER},

 {PD_ATTR_ID_ADAPTIVE_FLICKER,	MAKE_NAME("Adaptive Flicker Filter"),	2,
  GET_MAX_ADAPTIVE_FLICKER_FILTER, GET_ADAPTIVE_FLICKER_FILTER, SET_ADAPTIVE_FLICKER_FILTER},

 {PD_ATTR_ID_SSC,				MAKE_NAME("Spread Spectrum Clocking"), 1,
  GET_MAX_SSC,					GET_SSC,				SET_SSC},
#endif
};


/* .......................................................................... */
static sdvo_bool_attr_data_t g_bool_data[] =
/*	ID								Name					Bit_Mask_Cur	Bit_Mask_Def	 */
/*	opcode_get				opcode_set */
{
 {PD_ATTR_ID_2_CHANNEL_PANEL, MAKE_NAME("Dual Channel"),    0x10,           0,
  GET_LVDS_PANEL_INFORMATION, SET_LVDS_PANEL_INFORMATION},

 /* 0-spwg, 1-openldi */
 {PD_ATTR_ID_LVDS_PANEL_TYPE, MAKE_NAME("LVDS Panel Type"), 0x04,	        0,
  GET_LVDS_PANEL_INFORMATION, SET_LVDS_PANEL_INFORMATION},

 {PD_ATTR_ID_DITHER,          MAKE_NAME("Dither"),          0x01,        0x02,
  GET_DITHER,                 SET_DITHER},

#ifndef CONFIG_MICRO
 {PD_ATTR_ID_DOT_CRAWL,     MAKE_NAME("Dot Crawl"),							0x01,	0x02,
  GET_DOT_CRAWL,			SET_DOT_CRAWL},

 {PD_ATTR_ID_PANEL_PROTECT_HSYNC,	MAKE_NAME("HSync Panel Protection"),	0x01,	0,
  GET_LVDS_PANEL_PROTECTION, SET_LVDS_PANEL_PROTECTION},

 {PD_ATTR_ID_PANEL_PROTECT_VSYNC,	MAKE_NAME("VSync Panel Protection"),	0x02,	0,
  GET_LVDS_PANEL_PROTECTION, SET_LVDS_PANEL_PROTECTION},

 {PD_ATTR_ID_PANEL_PROTECT_PIXCLK,	MAKE_NAME("Pixel Clock Protection"),	0x04,	0,
  GET_LVDS_PANEL_PROTECTION, SET_LVDS_PANEL_PROTECTION},
#endif
};

/* .......................................................................... */
#ifndef CONFIG_MICRO
/* List entries for the TV format, the header are in g_list_header[] */
static sdvo_attr_list_entry_t g_list_entry_tv_format[] =
{
	{PD_TV_STD_NTSC_M,   MAKE_NAME("NTSC_M"),   0, 0x01},
	{PD_TV_STD_NTSC_M_J, MAKE_NAME("NTSC_J"),   0, 0x02},
	{PD_TV_STD_NTSC_433, MAKE_NAME("NTSC_433"),	0, 0x04},
	{PD_TV_STD_PAL_B,    MAKE_NAME("PAL_B"),    0, 0x08},
	{PD_TV_STD_PAL_D,    MAKE_NAME("PAL_D"),    0, 0x10},
	{PD_TV_STD_PAL_G,    MAKE_NAME("PAL_G"),    0, 0x20},
	{PD_TV_STD_PAL_H,    MAKE_NAME("PAL_H"),    0, 0x40},
	{PD_TV_STD_PAL_I,    MAKE_NAME("PAL_I"),    0, 0x80},

	{PD_TV_STD_PAL_M,   MAKE_NAME("PAL_M"),     1, 0x01},
	{PD_TV_STD_PAL_N,   MAKE_NAME("PAL_N"),     1, 0x02},
	{PD_TV_STD_PAL_NC,  MAKE_NAME("PAL_NC"),    1, 0x04},
	{PD_TV_STD_PAL_60,  MAKE_NAME("PAL_60"),    1, 0x08},
	{PD_TV_STD_SECAM_B,	MAKE_NAME("SECAM_B"),   1, 0x10},
	{PD_TV_STD_SECAM_D,	MAKE_NAME("SECAM_D"),   1, 0x20},
	{PD_TV_STD_SECAM_G,	MAKE_NAME("SECAM_G"),   1, 0x40},
	{PD_TV_STD_SECAM_K,	MAKE_NAME("SECAM_K"),   1, 0x80},

	{PD_TV_STD_SECAM_K1, MAKE_NAME("SECAM_K1"), 2, 0x01},
	{PD_TV_STD_SECAM_L,  MAKE_NAME("SECAM_L"),  2, 0x02},
	{PD_TV_STD_SECAM_60, MAKE_NAME("SECAM_60"), 2, 0x04},

	{PD_HDTV_STD_SMPTE_240M_1080i59, MAKE_NAME("SMPTE_240M_1080i59"), 2, 0x08},
	{PD_HDTV_STD_SMPTE_240M_1080i60, MAKE_NAME("SMPTE_240M_1080i60"), 2, 0x10},
	{PD_HDTV_STD_SMPTE_260M_1080i59, MAKE_NAME("SMPTE_260M_1080i59"), 2, 0x20},
	{PD_HDTV_STD_SMPTE_260M_1080i60, MAKE_NAME("SMPTE_260M_1080i60"), 2, 0x40},
	{PD_HDTV_STD_SMPTE_274M_1080i50, MAKE_NAME("SMPTE_274M_1080i50"), 2, 0x80},

	{PD_HDTV_STD_SMPTE_274M_1080i59, MAKE_NAME("SMPTE_274M_1080i59"), 3, 0x01},
	{PD_HDTV_STD_SMPTE_274M_1080i60, MAKE_NAME("SMPTE_274M_1080i60"), 3, 0x02},
	{PD_HDTV_STD_SMPTE_274M_1080p23, MAKE_NAME("SMPTE_274M_1080p23"), 3, 0x04},
	{PD_HDTV_STD_SMPTE_274M_1080p24, MAKE_NAME("SMPTE_274M_1080p24"), 3, 0x08},
	{PD_HDTV_STD_SMPTE_274M_1080p25, MAKE_NAME("SMPTE_274M_1080p25"), 3, 0x10},
	{PD_HDTV_STD_SMPTE_274M_1080p29, MAKE_NAME("SMPTE_274M_1080p29"), 3, 0x20},
	{PD_HDTV_STD_SMPTE_274M_1080p30, MAKE_NAME("SMPTE_274M_1080p30"), 3, 0x40},
	{PD_HDTV_STD_SMPTE_274M_1080p50, MAKE_NAME("SMPTE_274M_1080p50"), 3, 0x80},

	{PD_HDTV_STD_SMPTE_274M_1080p59, MAKE_NAME("SMPTE_274M_1080p59"), 4, 0x01},
	{PD_HDTV_STD_SMPTE_274M_1080p60, MAKE_NAME("SMPTE_274M_1080p60"), 4, 0x02},
	{PD_HDTV_STD_SMPTE_295M_1080i50, MAKE_NAME("SMPTE_295M_1080i50"), 4, 0x04},
	{PD_HDTV_STD_SMPTE_295M_1080p50, MAKE_NAME("SMPTE_295M_1080p50"), 4, 0x08},
	{PD_HDTV_STD_SMPTE_296M_720p59,  MAKE_NAME("SMPTE_296M_720p59"),  4, 0x10},
	{PD_HDTV_STD_SMPTE_296M_720p60,  MAKE_NAME("SMPTE_296M_720p60"),  4, 0x20},
	{PD_HDTV_STD_SMPTE_296M_720p50,  MAKE_NAME("SMPTE_296M_720p50"),  4, 0x40},
	{PD_HDTV_STD_SMPTE_293M_480p59,  MAKE_NAME("SMPTE_293M_480p59"),  4, 0x80},

	{PD_HDTV_STD_SMPTE_170M_480i59,  MAKE_NAME("MPTE_170M_480i59"),   5, 0x01},
	{PD_HDTV_STD_ITURBT601_576i50,   MAKE_NAME("ITURBT601_576i50"),   5, 0x02},
	{PD_HDTV_STD_ITURBT601_576p50,   MAKE_NAME("TURBT601_576p50"),    5, 0x04},
	{PD_HDTV_STD_EIA_7702A_480i60,   MAKE_NAME("EIA_7702A_480i60"),   5, 0x08},
	{PD_HDTV_STD_EIA_7702A_480p60,   MAKE_NAME("EIA_7702A_480p60"),   5, 0x10},
	{0, NULL, 0, 0}
};

/* Enable below attributes once upper layers hooked up copyprotection API */

/* List entries for the analog source, the header are in g_list_header[] */
static sdvo_attr_list_entry_t g_list_entry_analog_src[] =
{
	{PD_ANALOG_SRC_NO_DATA,          MAKE_NAME("No Data"),                 0, 0x00},
	{PD_ANALOG_SRC_PRE_RECORDED,     MAKE_NAME("Pre-recorded Packaged"),   0, 0x20},
	{PD_ANALOG_SRC_NOT_PRE_RECORDED, MAKE_NAME("Not Analog Pre-recorded"), 0, 0x30},
	{0, NULL, 0, 0}
};

/* List entries for the scan information, the header are in g_list_header[] */
static sdvo_attr_list_entry_t g_list_entry_scan_info[] =
{
	{PD_SCAN_NO_DATA,      MAKE_NAME("No Data"),      0, 0x00},
	{PD_SCAN_OVERSCANNED,  MAKE_NAME("Overscanned"),  0, 0x40},
	{PD_SCAN_UNDERSCANNED, MAKE_NAME("Underscanned"), 0, 0x80},
	{0, NULL, 0, 0}
};

/* List entries for the aspect ratio, the header are in g_list_header[] */
static sdvo_attr_list_entry_t g_list_entry_aspect_ratio[] =
{
	{PD_SCAN_NO_DATA,      MAKE_NAME("No Data"), 1, 0x00},
	{PD_SCAN_OVERSCANNED,  MAKE_NAME("4:3"),     1, 0x01},
	{PD_SCAN_UNDERSCANNED, MAKE_NAME("16:9"),    1, 0x02},
	{0, NULL, 0, 0}
};

/* List entries for the active format, the header are in g_list_header[] */
static sdvo_attr_list_entry_t g_list_entry_active_format[] =
{
	{PD_ACTIVE_FORMAT_NO_DATA,
	 MAKE_NAME("No Data"),                               1, 0x00},
	{PD_ACTIVE_FORMAT_SQUARE_PIXELS,
	 MAKE_NAME("Square Pixels"),                         1, 0x20},
	{PD_ACTIVE_FORMAT_4_3_CENTER,
	MAKE_NAME("4:3 Center"),                             1, 0x24},
	{PD_ACTIVE_FORMAT_16_9_CENTER,
	MAKE_NAME("16:9 Center"),                            1, 0x28},
	{PD_ACTIVE_FORMAT_14_9_CENTER,
	MAKE_NAME("14:9 Center"),                            1, 0x2C},
	{PD_ACTIVE_FORMAT_16_9_LETTERBOX_TOP,
	MAKE_NAME("16:9 Letterbox(Top)"),                    1, 0x08},
	{PD_ACTIVE_FORMAT_14_9_LETTERBOX_TOP,
	MAKE_NAME("14:9 Letterbox(Top)"),                    1, 0x0C},
	{PD_ACTIVE_FORMAT_16_9_LETTERBOX_CENTER,
	MAKE_NAME("16:9 Letterbox(Center)"),                 1, 0x10},
	{PD_ACTIVE_FORMAT_4_3_SHOOT_PROTECT_14_9_CENTER,
	MAKE_NAME("4:3(with shoot & protect 14:9 center)"),	 1, 0x34},
	{PD_ACTIVE_FORMAT_16_9_SHOOT_PROTECT_14_9_CENTER,
	MAKE_NAME("16:9(with shoot & protect 14:9 center)"), 1, 0x38},
	{PD_ACTIVE_FORMAT_16_9_SHOOT_PROTECT_4_3_CENTER,
	MAKE_NAME("16:9(with shoot & protect 4:3 center)"),	 1, 0x3C},
	{0, NULL, 0, 0}
};

static sdvo_attr_list_header_t g_list_header[] =
{
	{45, PD_ATTR_ID_TVFORMAT, MAKE_NAME("TV Out Format"),
	 GET_TV_OUTPUT_FORMAT, SET_TV_OUTPUT_FORMAT,
	 {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	 g_list_entry_tv_format},

/* Enable below attributes once upper layers hooked up copyprotection API */

    { 3, PD_ATTR_ID_ANALOG_SRC, MAKE_NAME("Analog Source"),
	 GET_ANCILLARY_VIDEO_INFORMATION, SET_ANCILLARY_VIDEO_INFORMATION,
	 {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	 g_list_entry_analog_src},

	{ 3, PD_ATTR_ID_SCAN_INFO, MAKE_NAME("Scan Information"),
	 GET_ANCILLARY_VIDEO_INFORMATION, SET_ANCILLARY_VIDEO_INFORMATION,
	 {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	 g_list_entry_scan_info},

	{ 3, PD_ATTR_ID_PICTURE_ASPECT_RATIO, MAKE_NAME("Picture Aspect Ratio"),
	 GET_ANCILLARY_VIDEO_INFORMATION, SET_ANCILLARY_VIDEO_INFORMATION,
	 {0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	 g_list_entry_aspect_ratio},

	{11, PD_ATTR_ID_ACTIVE_FORMAT, MAKE_NAME("Active Format Aspect Ratio"),
	 GET_ANCILLARY_VIDEO_INFORMATION, SET_ANCILLARY_VIDEO_INFORMATION,
	 {0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	 g_list_entry_active_format},

	{ 0 }
};

#endif

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_set_tv_optimal_settings(sdvo_device_context_t *p_ctx,
	unsigned char chron_tv_code)
{

	sdvo_status_t status = SS_SUCCESS;
	pd_attr_t *p_attr;
	int i;


	for (i= 0;i <5;i ++)
		if (chron_tv_code == g_sdtv_op_settings[i].tv_code) {
			/* set hue */
			p_attr = pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
					PD_ATTR_ID_HUE, 0);
			if (NULL != p_attr)
			{
				status = sdvo_alter_range_attr(p_ctx, (pd_range_attr_t *)p_attr,
						g_sdtv_op_settings[i].hue);
				p_attr->current_value = g_sdtv_op_settings[i].hue; //update table
			}

			/* set saturation */
			p_attr = pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
					PD_ATTR_ID_SATURATION, 0);
			if (NULL != p_attr)
			{
				status = sdvo_alter_range_attr(p_ctx, (pd_range_attr_t *)p_attr,
						g_sdtv_op_settings[i].saturation);

				p_attr->current_value = g_sdtv_op_settings[i].saturation;
			}

			/* set brightness */
			p_attr = pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
					PD_ATTR_ID_BRIGHTNESS, 0);
			if (p_attr)
			{
				status = sdvo_alter_range_attr(p_ctx, (pd_range_attr_t *)p_attr,
						g_sdtv_op_settings[i].brightness);

				p_attr->current_value = g_sdtv_op_settings[i].brightness;
			}

			/* set contrast */
			p_attr = pd_get_attr(p_ctx->p_attr_table, p_ctx->num_attrs,
					PD_ATTR_ID_CONTRAST, 0);
			if (p_attr)
			{
				status = sdvo_alter_range_attr(p_ctx, (pd_range_attr_t *)p_attr,
						g_sdtv_op_settings[i].contrast);

				p_attr->current_value = g_sdtv_op_settings[i].contrast;
			}
		}

	return status;
}
#endif

/* .......................................................................... */
/* .......................................................................... */
/*	============================================================================
	Function	:	sdvo_get_static_attrs

	Parameters	:	p_attr : Returns the list of avaliable static attributes

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
unsigned long sdvo_get_static_attrs(sdvo_device_context_t *p_ctx,
	pd_attr_t *p_attr_list1)
{
	int num_attrs = 0;
	pd_range_attr_t *p_attr_list = (pd_range_attr_t *)p_attr_list1;

	if (p_attr_list == NULL) {

		/*	Just return the number of available attributes */

		if (p_ctx->out_type.flags & FP_DISP_MASK) {
			num_attrs ++;

			/* Enable Text Tuning */
			if ((p_ctx->out_type.flags & LVDS_DISP_MASK) &&
				(p_ctx->dev_cap.up_scaling)) {
				num_attrs++;
			}
		}

		/*	Add Display Attribute only if sDVO returned more than one
			display type from GetDeviceCapabilities */
		if (sdvo_is_multi_display_device(p_ctx)) {
			num_attrs++;
		}

		/*  Red Blue Swap for DRGB output */
		if((p_ctx->out_type.flags & DRGB_DISP_MASK) && (p_ctx->dev_cap.vendor_id == 0x81)){
			num_attrs++;
		}
		return num_attrs;
	}

	/* ...................................................................... */
	/*	Fill in the available attributes */

	/*	Width and Height Attributes are available for LVDS and TMDS only */
	if (p_ctx->out_type.flags & FP_DISP_MASK) {
		p_attr_list[num_attrs] = g_fp_attr[1];   /* PANELFIT */
		/* There are initial values */
		p_attr_list[num_attrs].current_value = p_ctx->dev_cap.up_scaling;
		p_ctx->up_scaling = p_ctx->dev_cap.up_scaling;
		num_attrs++;

		/* Enable Text Tuning */
		if ((p_ctx->out_type.flags & LVDS_DISP_MASK) &&
			(p_ctx->dev_cap.up_scaling)) {
			p_attr_list[num_attrs] = g_fp_attr[2];   /* Text tuning */
			num_attrs++;
		}
	}

	/*	Add Display Attribute only if sDVO returned more than one
		display type from GetDeviceCapabilities */
	if (sdvo_is_multi_display_device(p_ctx)) {
		p_attr_list[num_attrs] = g_fp_attr[0];
		p_attr_list[num_attrs].default_value = p_ctx->dev_cap.output.flags;
		p_attr_list[num_attrs].current_value = p_ctx->out_type.flags;
		num_attrs++;
	}

	/*  Red Blue Swap for DRGB output */
	if((p_ctx->out_type.flags & DRGB_DISP_MASK) && (p_ctx->dev_cap.vendor_id == 0x81)){
		p_attr_list[num_attrs] = g_fp_attr[3];
		num_attrs++;
	}


#ifdef DEBUG_BUILD_TYPE
	/* ...................................................................... */
	{
		int i;
		for (i = 0; i < num_attrs; i++) {
			pd_attr_t *p_attr_cur = (pd_attr_t *)&p_attr_list[i];
			PD_DEBUG("sdvo : sdvo_get_static_attrs : "
			  "Adding attr='%s', id=%ld, type=%ld, default=%ld, current=%ld",
			  SDVO_GET_ATTR_NAME(p_attr_cur), p_attr_cur->id, p_attr_cur->type,
			  p_attr_cur->default_value, p_attr_cur->current_value);
		}
	}
#endif

	/* ...................................................................... */
	return num_attrs;
}


/*	============================================================================
	Function	:	sdvo_query_static_attr

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_query_static_attr(sdvo_device_context_t *p_ctx,
	pd_attr_t *p_attr)
{
	int b_error = 1;
	i2c_reg_t temp_reg;

	switch (p_attr->id) {
		case PD_ATTR_ID_DISPLAY :
			p_attr->current_value = p_ctx->out_type.flags;
			b_error = 0;
			break;

		case PD_ATTR_ID_PANEL_FIT:
			if (p_ctx->out_type.flags & FP_DISP_MASK) {
				p_attr->current_value = p_ctx->up_scaling;
				b_error = 0;
			}
			break;

		case PD_ATTR_ID_TEXT_TUNING:
			p_attr->current_value = p_ctx->text_tune;
			b_error = 0;
			break;
		case PD_ATTR_ID_RB_SWAP_MODE:
			{
				sdvo_read_i2c_reg(p_ctx, 0x30, &temp_reg);
				if (!(p_attr->flags & PD_ATTR_FLAG_NEED_RESTORE)){
					p_attr->current_value = (temp_reg & 0x2)?1:0;
				}
				/*Impossible to return correct value since the encoder
				has been reset.*/

				b_error = 0;
			}
			break;
	}

	/*	..................................................................... */
	if (b_error) {

		PD_ERROR("sdvo : Error ! sdvo_query_static_attr : "
			"Invalid attr='%s, id=%ld",
			SDVO_GET_ATTR_NAME(p_attr), p_attr->id);

		return SS_INVALID_ARGUMENT;
	}

	return SS_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_alter_static_attr

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_alter_static_attr(sdvo_device_context_t *p_ctx,
	pd_attr_t *p_attr, unsigned long new_value)
{
	int b_error = 1;
	i2c_reg_t temp_reg;

	switch (p_attr->id) {
		case PD_ATTR_ID_PANEL_FIT:
			if (p_ctx->out_type.flags & FP_DISP_MASK) {
				p_ctx->up_scaling = (unsigned char)
				((p_ctx->dev_cap.up_scaling)?new_value:0);
				b_error = 0;
			}
			break;

		case PD_ATTR_ID_TEXT_TUNING:
			if (sdvo_set_upscalar_coefficient(p_ctx,(i2c_reg_t)new_value) ==
				SS_SUCCESS) {
				p_ctx->text_tune = (i2c_reg_t)new_value;
			}
			b_error = 0;
			break;
		case PD_ATTR_ID_RB_SWAP_MODE:
			sdvo_read_i2c_reg(p_ctx, 0x30, &temp_reg);
			if(new_value){
				temp_reg |= 0x2;
				p_attr->current_value = 1;
			}else{
				temp_reg &= ~(0x2);
				p_attr->current_value = 0;
			}
			p_attr->flags |= PD_ATTR_FLAG_SETMODE;
			p_attr->flags &= ~PD_ATTR_FLAG_NEED_RESTORE;
			sdvo_write_i2c_reg(p_ctx, 0x30, temp_reg);
			b_error = 0;
			break;
	}

	/*	..................................................................... */
	if (b_error) {
		PD_ERROR("sdvo : Error ! sdvo_alter_static_attr : "
			"Invalid attr='%s, id=%ld",
			SDVO_GET_ATTR_NAME(p_attr), p_attr->id);
		return SS_INVALID_ARGUMENT;
	}
	return SS_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_get_panel_pwr_seq_attrs

	Parameters	:	pp_Attr : Returns the list of avaliable attributes

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
unsigned long sdvo_get_panel_pwr_seq_attrs(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr)
{
	sdvo_status_t status;
	i2c_reg_t ret_val[8];
	int i;

	status = sdvo_execute_command(p_ctx,
		GET_MAX_PANEL_POWER_SEQUENCING_PARAMETER,
		0, NULL,
		7, ret_val);
	if (status != SS_SUCCESS) {
		return 0;
	}

	if (p_attr == NULL) {
		/*	Just return the number of available attributes */
		return ARRAY_SIZE(g_panel_power_attr);
	}

	/* ...................................................................... */
	g_panel_power_attr[0].max=((unsigned long)(ret_val[5]&0x03)<<8)|ret_val[0];
	g_panel_power_attr[1].max=((unsigned long)(ret_val[5]&0x0C)<<6)|ret_val[1];
	g_panel_power_attr[2].max=((unsigned long)(ret_val[5]&0x30)<<4)|ret_val[2];
	g_panel_power_attr[3].max=((unsigned long)(ret_val[5]&0xC0)<<2)|ret_val[3];
	g_panel_power_attr[4].max=((unsigned long)(ret_val[6]&0x03)<<8)|ret_val[4];

	for (i = 0; i < 5; i++)	{
		pd_range_attr_t *p_attr_cur = &g_panel_power_attr[i];
		p_attr_cur->flags |= PD_ATTR_FLAG_DYNAMIC;
#ifndef CONFIG_MICRO
		status = sdvo_query_panel_pwr_seq_attr(p_ctx, p_attr_cur);
		if (status != SS_SUCCESS) {
			return 0;
		}
		p_attr_cur->default_value = p_attr_cur->current_value;
#endif
		PD_DEBUG("sdvo : sdvo_get_panel_pwr_seq_attrs : "
		  "Adding attr='%s', id=%ld, default=%ld, current=%ld, min=%ld, "
		  "max=%ld, step=%ld",
		  SDVO_GET_ATTR_NAME(p_attr_cur), p_attr_cur->id,
		  p_attr_cur->default_value, p_attr_cur->current_value,
		  p_attr_cur->min, p_attr_cur->max, p_attr_cur->step);

	}

	pd_memcpy(p_attr, g_panel_power_attr, sizeof(g_panel_power_attr));
	return ARRAY_SIZE(g_panel_power_attr);
}


/*	============================================================================
	Function	:

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_query_panel_pwr_seq_attr(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr)
{
#ifndef CONFIG_MICRO
	sdvo_status_t status;
	i2c_reg_t ret_val[8];

	status = sdvo_execute_command(p_ctx, GET_PANEL_POWER_SEQUENCING_PARAMETER,
								 0, NULL,
								 7, ret_val);
	if (status != SS_SUCCESS) {

		return status;
	}

	switch (p_attr->id) {

		case PD_ATTR_ID_FP_PWR_T1 :
			p_attr->current_value = p_ctx->t1 =
				((unsigned short)(ret_val[5] & 0x03) << 8) | ret_val[0];
			break;

		case PD_ATTR_ID_FP_PWR_T2 :
			p_attr->current_value = p_ctx->t2 =
				((unsigned short)(ret_val[5] & 0x0C) << 6) | ret_val[1];
			break;

		case PD_ATTR_ID_FP_PWR_T3 :
			p_attr->current_value = p_ctx->t3 =
				((unsigned short)(ret_val[5] & 0x30) << 4) | ret_val[2];
			break;

		case PD_ATTR_ID_FP_PWR_T4 :
			p_attr->current_value = p_ctx->t4 =
				((unsigned short)(ret_val[5] & 0xC0) << 2) | ret_val[3];
			break;

		case PD_ATTR_ID_FP_PWR_T5 :
			p_attr->current_value = p_ctx->t5 =
				((unsigned short)(ret_val[6] & 0x03) << 8) | ret_val[4];
			break;

		default :
			return SS_INVALID_ARGUMENT;
	}
#endif
	return SS_SUCCESS;
}


/*	============================================================================
	Function	:	sdvo_alter_panel_pwr_seq_attr

	Parameters	:

	Remarks     :

	Returns     :
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_alter_panel_pwr_seq_attr(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr,
	unsigned long new_value)
{
	sdvo_status_t status;
	i2c_reg_t arg[8];
	i2c_reg_t new_val_low  = (i2c_reg_t)(new_value & 0xFF);
	i2c_reg_t new_val_high = (i2c_reg_t)((new_value & 0x0300) >> 8);

	if ((new_value < p_attr->min) || (new_value > p_attr->max)) {
		PD_DEBUG("Requested values out of range, new_value = %lu.",
			new_value);
		return SS_INVALID_ARGUMENT;
	}

	status = sdvo_execute_command(p_ctx, GET_PANEL_POWER_SEQUENCING_PARAMETER,
		0, NULL, 7, arg);
	if (status != SS_SUCCESS) {
		PD_DEBUG("GET_PANEL_POWER_SEQUENCING_PARAMETER failed status = %u",
			status);
		return status;
	}

	switch (p_attr->id) {
		case PD_ATTR_ID_FP_PWR_T1 :
			p_ctx->t1 = (unsigned short)new_value;
			arg[0] = new_val_low;
			arg[5] &= 0xFC;
			arg[5] |= new_val_high;
			break;

		case PD_ATTR_ID_FP_PWR_T2 :
			p_ctx->t2 = (unsigned short)new_value;
			arg[1] = new_val_low;
			arg[5] &= 0xF3;
			arg[5] |= (new_val_high << 2);
			break;

		case PD_ATTR_ID_FP_PWR_T3 :
			p_ctx->t3 = (unsigned short)new_value;
			arg[2] = new_val_low;
			arg[5] &= 0xCF;
			arg[5] |= (new_val_high << 4);
			break;

		case PD_ATTR_ID_FP_PWR_T4 :
			p_ctx->t4 = (unsigned short)new_value;
			arg[3] = new_val_low;
			arg[5] &= 0x3F;
			arg[5] |= (new_val_high << 6);
			break;

		case PD_ATTR_ID_FP_PWR_T5 :
			p_ctx->t5 = (unsigned short)new_value;
			arg[4] = new_val_low;
			arg[6] &= 0xFC;
			arg[6] |= new_val_high;
			break;

		default :
			return SS_INVALID_ARGUMENT;
	}

	status = sdvo_execute_command(p_ctx, SET_PANEL_POWER_SEQUENCING_PARAMETER,
								 7, arg,
								 0, NULL);
	return status;
}


/*	============================================================================
	Function	:	sdvo_get_range_attrs returns all the available range
					attributes

	Parameters	:	pp_Attr : Returns the list of avaliable range attributes

	Remarks     :	sdvo_get_range_attrs returns all the range attributes
					from g_range_data table that are supported by the SDVO device.
					It allocates memory for all possible range attributes.
					The caller is responsible for adding the range attributes to
					port driver attribute table and freeing the memory allocated
					for pp_Attr.

	Returns     :	Number of range attributes returned in pp_Attr list.
	------------------------------------------------------------------------- */
unsigned long sdvo_get_range_attrs(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr)
{
	unsigned long num_attrs, i;
	sdvo_status_t status;
	unsigned char byte_ret[4];

	/*	..................................................................... */
	if (p_attr == NULL) {
		/*	Just return the number of available attributes */
		num_attrs = 0;

		for (i = 0; i < ARRAY_SIZE(g_range_data); i++) {
			status = sdvo_execute_command(p_ctx, g_range_data[i].opcode_max,
							0, NULL,
							(i2c_reg_t)(g_range_data[i].num_bytes * 2),
							byte_ret);
			if (status != SS_SUCCESS) {
				continue;
			}

			status = sdvo_execute_command(p_ctx, g_range_data[i].opcode_get,
										 0, NULL,
										 (i2c_reg_t)(g_range_data[i].num_bytes),
										 byte_ret);
			if (status != SS_SUCCESS) {
				continue;
			}

			num_attrs++;
		}

		return num_attrs;
	}

	/*	..................................................................... */
	num_attrs = 0;

	for (i = 0; i < ARRAY_SIZE(g_range_data); i++) {

		pd_range_attr_t *p_attr_cur = &p_attr[num_attrs];

		status = sdvo_execute_command(p_ctx, g_range_data[i].opcode_max,
									 0, NULL,
									 (i2c_reg_t)(g_range_data[i].num_bytes * 2),
									 byte_ret);
		if (status != SS_SUCCESS) {

			continue;
		}

		p_attr_cur->id = g_range_data[i].id;
		p_attr_cur->type = PD_ATTR_TYPE_RANGE;
		p_attr_cur->flags |= PD_ATTR_FLAG_DYNAMIC;

#ifndef CONFIG_MICRO
		pd_strcpy(p_attr_cur->name, g_range_data[i].p_name);
#endif

		p_attr_cur->default_value = g_range_data[i].num_bytes == 1 ?
									byte_ret[1] : byte_ret[2] + (byte_ret[3] << 8);
		p_attr_cur->min	= 0;
		p_attr_cur->step = 1;
		p_attr_cur->max	= g_range_data[i].num_bytes == 1 ?
			byte_ret[0] : byte_ret[0] + (byte_ret[1] << 8);

		status = sdvo_query_range_attr(p_ctx, p_attr_cur);
		if (status != SS_SUCCESS) {
			continue;
		}

		PD_DEBUG("sdvo : sdvo_get_range_attrs : "
				  "Adding attr='%s', id=%ld, default=%ld, current=%ld, min=%ld, "
				  "max=%ld, step=%ld",
				  SDVO_GET_ATTR_NAME(p_attr_cur), p_attr_cur->id,
				  p_attr_cur->default_value, p_attr_cur->current_value,
				  p_attr_cur->min, p_attr_cur->max, p_attr_cur->step);

		num_attrs++;
	}

	/*	..................................................................... */
	return num_attrs;
}


/*	============================================================================
	Function	:	sdvo_query_range_attr is called to query the current value
					of a specific range attribute

	Parameters	:	p_attr : Pointer to range attribute

	Remarks     :	sdvo_query_range_attr scans the g_range_data list to find
					the range attribute of id specified in p_attr.
					If found it executes the opcode_get command to get the current
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_query_range_attr(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr)
{
	int i;
	sdvo_status_t status;

	for (i = 0; i < ARRAY_SIZE(g_range_data); i++) {

		if (g_range_data[i].id == p_attr->id) {

			status = sdvo_execute_command(p_ctx, g_range_data[i].opcode_get,
									   0, NULL,
									   g_range_data[i].num_bytes,
									   (unsigned char *)&p_attr->current_value);

			if (status == SS_SUCCESS) {
				/* Special case for Panel depth */
				if (g_range_data[i].id == PD_ATTR_ID_PANEL_DEPTH) {
					p_attr->min = 18;
					p_attr->step = 6;
					p_attr->max = 24;
					p_attr->default_value = 18;
					if (p_attr->current_value & 0x01) {
						p_attr->current_value = 24;
					} else {
						p_attr->current_value = 18;
					}
				}
			}
			return status;
		}
	}

	return SS_INVALID_ARGUMENT;
}


/*	============================================================================
	Function	:	sdvo_alter_range_attr is called to set the new value of a
					specific range attribute

	Parameters	:	p_attr : Pointer to range attribute

	Remarks     :	sdvo_query_range_attr scans the g_range_data list to find
					the range attribute of id specified in p_attr.
					If found it executes the opcode_set command to set the new
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_alter_range_attr(sdvo_device_context_t *p_ctx,
	pd_range_attr_t *p_attr, unsigned long new_value)
{
	int i;
	sdvo_status_t status;


	if ((new_value < p_attr->min) || (new_value > p_attr->max)) {

		return SS_INVALID_ARGUMENT;
	}

	for (i = 0; i < ARRAY_SIZE(g_range_data); i++) {

		if (g_range_data[i].id == p_attr->id) {
			/* Special case handling for panel depth attribute */
			if (p_attr->id == PD_ATTR_ID_PANEL_DEPTH) {
				i2c_reg_t cur_value;
				status = sdvo_execute_command(p_ctx,
					g_range_data[i].opcode_get,
					0, NULL,
					1, &cur_value);
				if (status != SS_SUCCESS) {
					return status;
				}

				if (new_value == 24) {
					/* 24-bit display */
					new_value = cur_value | 0x01; /* set bits [1:0] = 01 */
				} else {
					/* 18-bit display */
					new_value = cur_value & 0xFC; /* set bits [1:0] = 00 */
				}
			}

			/*Special case handling for CH7308 EMI on/off operation */

			if (p_attr->id == PD_ATTR_ID_SSC) {

				i2c_reg_t reg;
				if( new_value > 0) /* turn on EMI for value range from 1 - 15 */
				{

					sdvo_read_i2c_reg(p_ctx, 0x3C, &reg);
					sdvo_write_i2c_reg(p_ctx, 0x3C, (reg & 0xFC));
					sdvo_read_i2c_reg(p_ctx, 0x3E, &reg);
					sdvo_write_i2c_reg(p_ctx, 0x3E, (reg | 0x80));
				}
				else /* turn off EMI */
				{
					sdvo_read_i2c_reg(p_ctx, 0x3C, &reg);
					sdvo_write_i2c_reg(p_ctx, 0x3C, (reg | 0x01));
					sdvo_read_i2c_reg(p_ctx, 0x3E, &reg);
					sdvo_write_i2c_reg(p_ctx, 0x3E, (reg & 0x7F));

				}


			}


			return sdvo_execute_command(p_ctx, g_range_data[i].opcode_set,
								   g_range_data[i].num_bytes,
								   (unsigned char *)&new_value,
								   0, NULL);
		}
	}

	return SS_INVALID_ARGUMENT;
}


/*	============================================================================
	Function	:	sdvo_get_bool_attrs returns all the available boolean
					attributes

	Parameters	:	pp_Attr : Returns the list of avaliable boolean attributes

	Remarks     :	sdvo_get_bool_attrs returns all the boolean attributes
					from g_bool_data table that are supported by the SDVO device.
					It allocates memory for all possible boolean attributes.
					The caller is responsible for adding the booelan attributes
					to the port driver attribute table and freeing the memory
					allocated for pp_Attr.

	Returns     :	Number of boolean attributes returned in pp_Attr list.
	------------------------------------------------------------------------- */
unsigned long sdvo_get_bool_attrs(sdvo_device_context_t *p_ctx,
	pd_bool_attr_t *p_attr)
{
	unsigned long num_attrs, i;
	sdvo_status_t status;

	/*	..................................................................... */
	if (p_attr == NULL) {

		/*	Just return the number of available attributes */
		i2c_reg_t ret_value;

		num_attrs = 0;

		for (i = 0; i < ARRAY_SIZE(g_bool_data); i++) {
			if((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS) &&
				(p_ctx->dev_cap.device_id == 0x1) &&
	            ((g_bool_data[i].id == PD_ATTR_ID_PANEL_PROTECT_HSYNC) ||
	            (g_bool_data[i].id == PD_ATTR_ID_PANEL_PROTECT_VSYNC))) {

	            continue;
	        }
			status = sdvo_execute_command(p_ctx, g_bool_data[i].opcode_get,
						 0, NULL,
						 1, &ret_value);
			if (status != SS_SUCCESS) {

				continue;
			}

			num_attrs++;
		}

		return num_attrs;
	}

	/*	..................................................................... */
	num_attrs = 0;

	for (i = 0; i < ARRAY_SIZE(g_bool_data); i++) {

		pd_bool_attr_t *p_attr_cur = &p_attr[num_attrs];

		p_attr_cur->id   = g_bool_data[i].id;
		p_attr_cur->type = PD_ATTR_TYPE_BOOL;
		p_attr_cur->flags |= PD_ATTR_FLAG_DYNAMIC;

		if((p_ctx->dev_cap.vendor_id == VENDOR_ID_LAPIS) &&
			(p_ctx->dev_cap.device_id == 0x1) &&
	        ((g_bool_data[i].id == PD_ATTR_ID_PANEL_PROTECT_HSYNC) ||
	        (g_bool_data[i].id == PD_ATTR_ID_PANEL_PROTECT_VSYNC))) {

	        continue;
	    }


		status = sdvo_query_bool_attr(p_ctx, p_attr_cur);
		if (status != SS_SUCCESS)
			continue;

		p_attr_cur->default_value = p_attr_cur->current_value;

#ifndef CONFIG_MICRO
		pd_strcpy(p_attr_cur->name, g_bool_data[i].p_name);
#endif

		PD_DEBUG("sdvo : sdvo_get_bool_attrs : "
				  "Adding attr='%s', id=%ld, default=%ld, current=%ld",
				  SDVO_GET_ATTR_NAME(p_attr_cur), p_attr_cur->id,
				  p_attr_cur->default_value, p_attr_cur->current_value);

		num_attrs++;
	}

	/*	..................................................................... */
	return num_attrs;
}


/*	============================================================================
	Function	:	sdvo_query_bool_attr is called to query the current value
					of a specific boolean attribute

	Parameters	:	p_attr : Pointer to boolean attribute


	Remarks     :	sdvo_query_bool_attr scans the g_bool_data list to find
					the boolean attribute of id specified in p_attr.
					If found it executes the opcode_get command to get the current
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_query_bool_attr(sdvo_device_context_t *p_ctx,
	pd_bool_attr_t *p_attr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_bool_data); i++) {

		if (g_bool_data[i].id == p_attr->id) {

			sdvo_status_t status;
			i2c_reg_t ret_value;

			status = sdvo_execute_command(p_ctx, g_bool_data[i].opcode_get,
										 0, NULL,
										 1, &ret_value);

			if (status == SS_SUCCESS) {
				if (ret_value & g_bool_data[i].bit_mask_cur) {
					p_attr->current_value = 1;
				} else {
					p_attr->current_value = 0;
				}

				if (g_bool_data[i].bit_mask_def != 0) {
					p_attr->default_value = ret_value & g_bool_data[i].bit_mask_def;
				}
			}

			return status;
		}
	}

	return SS_INVALID_ARGUMENT;
}


/*	============================================================================
	Function	:	sdvo_alter_bool_attr is called to set the new value of a
					specific boolean attribute

	Parameters	:	p_attr : Pointer to boolean attribute

	Remarks     :	sdvo_alter_range_attr scans the g_bool_data list to find
					the bool attribute of id specified in p_attr.
					If found it executes the opcode_set command to set the new
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_alter_bool_attr(sdvo_device_context_t *p_ctx,
	pd_bool_attr_t *p_attr, unsigned long new_value)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_bool_data); i++) {
		if (g_bool_data[i].id == p_attr->id) {
			sdvo_status_t status;
			i2c_reg_t cur_value;

			status = sdvo_execute_command(p_ctx, g_bool_data[i].opcode_get,
										 0, NULL,
										 1, &cur_value);
			if (status != SS_SUCCESS) {
				return status;
			}
#ifndef CONFIG_MICRO
			if(p_attr->id == PD_ATTR_ID_DITHER){
				cur_value &= 0;
			}
#endif
			if (new_value) {
				cur_value |= g_bool_data[i].bit_mask_cur;
			} else {
				cur_value &= ~g_bool_data[i].bit_mask_cur;
			}

			status = sdvo_execute_command(p_ctx, g_bool_data[i].opcode_set,
										 1, &cur_value,
										 0, NULL);
			return status;
		}
	}

	return SS_INVALID_ARGUMENT;
}


/*	============================================================================
	Function	:	sdvo_get_list_attrs returns all the available list
					attributes

	Parameters	:	p_attr : Returns the list of avaliable list attributes

	Remarks     :	sdvo_get_list_attrs returns all the list attributes
					from g_list_data table that are supported by the SDVO device.
					It allocates memory for all possible list attributes.
					The caller is responsible for adding the list attributes
					to the port driver attribute table and freeing the memory
					allocated for p_attr.

	Returns     :	Number of list attributes returned in pp_Attr list.
	------------------------------------------------------------------------- */
unsigned long sdvo_get_list_attrs(sdvo_device_context_t *p_ctx,
	pd_list_attr_t *p_attr)
{
#ifndef CONFIG_MICRO
	unsigned long num_attrs, i, j;
	sdvo_status_t status;
	sdvo_attr_list_entry_t *list_item;

	list_item = NULL;
	num_attrs = 0;
	i = j = 0;

	/*	..................................................................... */
	if (p_attr == NULL) {

		/*	Just return the number of available attributes */
		i2c_reg_t ret_value[8];
		while (g_list_header[i].num_entries) {

			status = sdvo_execute_command(p_ctx, g_list_header[i].opcode_get,
										 0, NULL, 8, ret_value);

			if (status == SS_SUCCESS) {
				num_attrs += (g_list_header[i].num_entries + 1);
			}

			++i;	/*	Skip to the next list hdr */
		}

		return num_attrs;
	}

	while (g_list_header[i].num_entries) {

		pd_list_attr_t *p_hdr = &p_attr[num_attrs];

		p_hdr->type	= PD_ATTR_TYPE_LIST;
		p_hdr->id = g_list_header[i].id;
		p_hdr->num_entries = g_list_header[i].num_entries;
		p_hdr->flags |= PD_ATTR_FLAG_DYNAMIC;

		status = sdvo_query_list_attr(p_ctx, p_hdr);
		if (status != SS_SUCCESS) {
			++i; /* Skip to the next list header */
			continue;
		}

		pd_strcpy(p_hdr->name, g_list_header[i].p_name);
		p_hdr->default_index = p_hdr->current_index;

		PD_DEBUG("sdvo : sdvo_get_list_attrs : "
				  "Adding attr='%s', id=%ld, default=%ld, current=%ld, "
				  "num_entries=%ld",
				  SDVO_GET_ATTR_NAME(p_hdr), p_hdr->id, p_hdr->default_index, p_hdr->current_index,
				  p_hdr->num_entries);

		++num_attrs;
		/*	Copy the list entries */
		list_item = g_list_header[i].attr_list;
		for (j = 0; j < g_list_header[i].num_entries; ++j, ++num_attrs) {

			pd_list_entry_attr_t *p_entry =
				(pd_list_entry_attr_t *)&p_attr[num_attrs];

			p_entry->id		= g_list_header[i].id;  /* Header ID */
			p_entry->type	= PD_ATTR_TYPE_LIST_ENTRY;
			p_entry->value	= list_item[j].id;      /* Entry ID */
			p_entry->flags  |= PD_ATTR_FLAG_DYNAMIC;

			pd_strcpy(p_entry->name, list_item[j].p_name);
			PD_DEBUG("sdvo : sdvo_get_list_attrs : Entry[%ld]=%s, id=%ld, "
					  "value=%ld",
					  j, SDVO_GET_ATTR_NAME(p_entry), p_entry->id,
					  p_entry->value);
		}

		++i;   /*	Skip to the next list header */
	}

	/*	..................................................................... */
	return num_attrs;
#else
	return 0;
#endif
}


/*	============================================================================
	Function	:	sdvo_query_list_attr is called to query the current value
					of a specific list attribute

	Parameters	:	p_attr : Pointer to list header attribute

	Remarks     :	sdvo_query_list_attr scans the g_list_data list to find
					the list attribute of id specified in p_attr.
					If found it executes the opcode_get command to get the current
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_query_list_attr(sdvo_device_context_t *p_ctx,
	pd_list_attr_t *p_attr)
{
#ifndef CONFIG_MICRO
	i2c_reg_t ret_value[8];
	int i, j;
	sdvo_status_t status;
	sdvo_attr_list_entry_t *list_item;

	i = 0;
	list_item = NULL;
	while (g_list_header[i].num_entries) {

		if (g_list_header[i].id != p_attr->id) {
			++i;	/*	Skip to the next list hdr */
			continue;
		}

		status = sdvo_execute_command(p_ctx, g_list_header[i].opcode_get,
									 0, NULL,
									 8, ret_value);
		if (status != SS_SUCCESS) {

			return status;
		}

		for (j = 0; j < 8; ++j) {
			ret_value[j] &= g_list_header[i].bit_masks[j];
		}

		list_item = g_list_header[i].attr_list;
		for (j = 0; j < g_list_header[i].num_entries; ++j) {

			if (ret_value[list_item[j].reg_offset] == list_item[j].value) {
				p_attr->current_index = j + 1;

				return SS_SUCCESS;
			}
		}

		/* If current_index isn't found then the default hardware state is
		 * not in one of the known states, then set it to the first one */
		p_attr->current_index = 1;
		return sdvo_alter_list_attr(p_ctx, p_attr, 1);
	}

	PD_ERROR("sdvo : Warning ! sdvo_query_list_attr : Unable to find list "
	    "header");
#endif
	return SS_INVALID_ARGUMENT;
}


/*	============================================================================
	Function	:	sdvo_alter_list_attr is called to set the new value of a
					specific list attribute

	Parameters	:	p_attr : Pointer to list attribute

	Remarks     :	sdvo_alter_list_attr scans the g_list_data list to find
					the list attribute of id specified in p_attr.
					If found it executes the opcode_set command to set the new
					value of the attribute.

	Returns     :	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_alter_list_attr(sdvo_device_context_t *p_ctx,
	pd_list_attr_t *p_attr, unsigned long new_index)
{
#ifndef CONFIG_MICRO
	unsigned short i, j;
	sdvo_status_t status;
	i2c_reg_t ret_value[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	sdvo_attr_list_entry_t *list_item;

	if (new_index > p_attr->num_entries) {
		return SS_INVALID_ARGUMENT;
	}

	list_item = NULL;
	i = 0;
	while (g_list_header[i].num_entries) {
		if (g_list_header[i].id != p_attr->id) {
			++i;   /*	Skip to the next list hdr */
			continue;
		}

		/*	Found the list header */
		/*	Get the current value of the attribute(s) */
		status = sdvo_execute_command(p_ctx, g_list_header[i].opcode_get,
			0, NULL, 8, ret_value);
		if (status != SS_SUCCESS) {
			return status;
		}

		/* Mask out the parameter bits */
		for (j = 0; j < 8; ++j) {
			ret_value[j] &= ~g_list_header[i].bit_masks[j];
		}

		list_item = g_list_header[i].attr_list;  /* Get the list entries */
		/* Set new index bits */
		ret_value[list_item[new_index - 1].reg_offset] |=
			list_item[new_index - 1].value;

		status = sdvo_execute_command(p_ctx, g_list_header[i].opcode_set,
			8, ret_value, 0, NULL);
		/* If opcode_set(new_value) is failed, assuming it will stay at old value.
		 * If this isn't the case, then opcode_set(old_value) is required. */
		if (status != SS_SUCCESS) {
			/* Set old value */
		}
		return status;
	}
#endif
	return SS_INVALID_ARGUMENT;
}

/*
 *-----------------------------------------------------------------------------
 * Filename: displayid.c
 * $Revision: 1.11 $
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
 *  This file contains functions to parse DisplayID into a data strucuture.
 *  Supported DisplayID versions:
 *  VESA DisplayID Standard Verion 1 12/13/2007
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <io.h>
#include <memory.h>

#include <igd_errno.h>

#include <displayid.h>
#include <pi.h>

/*!
 * @addtogroup display_group
 * @{
 */

#ifndef CONFIG_NO_DISPLAYID

/* IMP NOTE:
 *     Keep the order of datablocks as it is.
 *     DisplayID parser directly access the offset using tag as an index.
 */
unsigned short db_offset[] = {
#ifdef CONFIG_MICRO
	OS_OFFSETOF(displayid_t, dummy_db),        /* PRODUCTID        0x00 */
	OS_OFFSETOF(displayid_t, display_params),  /* DISPLAY_PARAMS   0x01 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* COLOR_CHARS      0x02 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_1_DETAIL  0x03 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_2_DETAIL  0x04 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_3_SHORT   0x05 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_4_DMTID   0x06 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* VESA_TIMING_STD  0x07 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* CEA_TIMING_STD   0x08 */
	OS_OFFSETOF(displayid_t, timing_range),    /* VIDEO_RANGE      0x09 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* SERIAL_NUMBER    0x0A */
	OS_OFFSETOF(displayid_t, dummy_db),        /* ASCII_STRING     0x0B */
	OS_OFFSETOF(displayid_t, display_dev),     /* DISPLAY_DEVICE   0x0C */
	OS_OFFSETOF(displayid_t, lvds),            /* LVDS_INTERFACE   0x0D */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TRANSFER_CHAR    0x0E */
	OS_OFFSETOF(displayid_t, display_intf),    /* DISPLAY_INTF     0x0F */
	OS_OFFSETOF(displayid_t, dummy_db),        /* STEREO_INTF      0x10 */
#else
	OS_OFFSETOF(displayid_t, productid),       /* PRODUCTID        0x00 */
	OS_OFFSETOF(displayid_t, display_params),  /* DISPLAY_PARAMS   0x01 */
	OS_OFFSETOF(displayid_t, color_char),      /* COLOR_CHARS      0x02 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_1_DETAIL  0x03 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_2_DETAIL  0x04 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_3_SHORT   0x05 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* TIMING_4_DMTID   0x06 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* VESA_TIMING_STD  0x07 */
	OS_OFFSETOF(displayid_t, dummy_db),        /* CEA_TIMING_STD   0x08 */
	OS_OFFSETOF(displayid_t, timing_range),    /* VIDEO_RANGE      0x09 */
	OS_OFFSETOF(displayid_t, serial_num),      /* SERIAL_NUMBER    0x0A */
	OS_OFFSETOF(displayid_t, general_string),  /* ASCII_STRING     0x0B */
	OS_OFFSETOF(displayid_t, display_dev),     /* DISPLAY_DEVICE   0x0C */
	OS_OFFSETOF(displayid_t, lvds),            /* LVDS_INTERFACE   0x0D */
	OS_OFFSETOF(displayid_t, transfer_char),   /* TRANSFER_CHAR    0x0E */
	OS_OFFSETOF(displayid_t, display_intf),    /* DISPLAY_INTF     0x0F */
	OS_OFFSETOF(displayid_t, stereo_intf),     /* STEREO_INTF      0x10 */
#endif


#if 0
	/* Vendor specific tag is out of order, so it cannot be here. See
	 * case DATABLOCK_VENDOR_SPECIFIC for implementation. */
	OS_OFFSETOF(displayid_t, vendor),          /* VENDOR_SPECIFIC  0x7F */
#endif
};

type_std_t vesa_std_lookup[] =
{
	/* width  height refresh flags */
	/* byte 0 bit 0->7 */
	{  640,    350,    85,   0                 },   /* bit 0 */
	{  640,    400,    85,   0                 },   /* bit 1 */
	{  720,    400,    85,   0                 },   /* bit 2 */
	{  640,    480,    60,   0                 },   /* bit 3 */
	{  640,    480,    72,   0                 },   /* bit 4 */
	{  640,    480,    75,   0                 },   /* bit 5 */
	{  640,    480,    85,   0                 },   /* bit 6 */
	{  800,    600,    56,   0                 },   /* bit 7 */

	/* byte 1 bit 0->7 */
	{  800,    600,    60,   0                 },   /* bit 0 */
	{  800,    600,    72,   0                 },   /* bit 1 */
	{  800,    600,    75,   0                 },   /* bit 2 */
	{  800,    600,    85,   0                 },   /* bit 3 */
	{  800,    600,   120,   PD_MODE_RB        },   /* bit 4 */
	{  848,    480,    60,   0                 },   /* bit 5 */
	{ 1024,    768,    43,   PD_SCAN_INTERLACE },   /* bit 6 */
	{ 1024,    768,    60,   0                 },   /* bit 7 */

	/* byte 2 bit 0->7 */
	{ 1024,    768,    70,   0                 },   /* bit 0 */
	{ 1024,    768,    75,   0                 },   /* bit 1 */
	{ 1024,    768,    85,   0                 },   /* bit 2 */
	{ 1024,    768,   120,   PD_MODE_RB        },   /* bit 3 */
	{ 1152,    864,    75,   0                 },   /* bit 4 */
	{ 1280,    768,    60,   PD_MODE_RB        },   /* bit 5 */
	{ 1280,    768,    60,   0                 },   /* bit 6 */
	{ 1280,    768,    75,   0                 },   /* bit 7 */

	/* byte 3 bit 0->7 */
	{ 1280,    768,    85,   0                 },   /* bit 0 */
	{ 1280,    768,   120,   PD_MODE_RB        },   /* bit 1 */
	{ 1280,    800,    60,   PD_MODE_RB        },   /* bit 2 */
	{ 1280,    800,    60,   0                 },   /* bit 3 */
	{ 1280,    800,    75,   0                 },   /* bit 4 */
	{ 1280,    800,    85,   0                 },   /* bit 5 */
	{ 1280,    800,   120,   PD_MODE_RB        },   /* bit 6 */
	{ 1280,    960,    60,   0                 },   /* bit 7 */

	/* byte 4 bit 0->7 */
	{ 1280,    960,    85,   0                 },   /* bit 0 */
	{ 1280,    960,   120,   PD_MODE_RB        },   /* bit 1 */
	{ 1280,   1024,    60,   0                 },   /* bit 2 */
	{ 1280,   1024,    75,   0                 },   /* bit 3 */
	{ 1280,   1024,    85,   0                 },   /* bit 4 */
	{ 1280,   1024,   120,   PD_MODE_RB        },   /* bit 5 */
	{ 1360,    768,    60,   0                 },   /* bit 6 */
	{ 1360,    768,   120,   PD_MODE_RB        },   /* bit 7 */

	/* byte 5 bit 0->7 */
	{ 1400,   1050,    60,   PD_MODE_RB        },   /* bit 0 */
	{ 1400,   1050,    60,   0                 },   /* bit 1 */
	{ 1400,   1050,    75,   0                 },   /* bit 2 */
	{ 1400,   1050,    85,   0                 },   /* bit 3 */
	{ 1400,   1050,   120,   PD_MODE_RB        },   /* bit 4 */
	{ 1440,    900,    60,   PD_MODE_RB        },   /* bit 5 */
	{ 1440,    900,    60,   0                 },   /* bit 6 */
	{ 1440,    900,    75,   0                 },   /* bit 7 */

	/* byte 6 bit 0->7 */
	{ 1440,    900,    85,   0                 },   /* bit 0 */
	{ 1440,    900,   120,   PD_MODE_RB        },   /* bit 1 */
	{ 1600,   1200,    60,   0                 },   /* bit 2 */
	{ 1600,   1200,    65,   0                 },   /* bit 3 */
	{ 1600,   1200,    70,   0                 },   /* bit 4 */
	{ 1600,   1200,    75,   0                 },   /* bit 5 */
	{ 1600,   1200,    85,   0                 },   /* bit 6 */
	{ 1600,   1200,   120,   PD_MODE_RB        },   /* bit 7 */

	/* byte 7 bit 0->7 */
	{ 1680,   1050,    60,   PD_MODE_RB        },   /* bit 0 */
	{ 1680,   1050,    60,   0                 },   /* bit 1 */
	{ 1680,   1050,    75,   0                 },   /* bit 2 */
	{ 1680,   1050,    85,   0                 },   /* bit 3 */
	{ 1680,   1050,   120,   PD_MODE_RB        },   /* bit 4 */
	{ 1792,   1344,    60,   0                 },   /* bit 5 */
	{ 1792,   1344,    75,   0                 },   /* bit 6 */
	{ 1792,   1344,   120,   PD_MODE_RB        },   /* bit 7 */

	/* byte 8 bit 0->7 */
	{ 1856,   1392,    60,   0                 },   /* bit 0 */
	{ 1856,   1392,    75,   0                 },   /* bit 1 */
	{ 1856,   1392,   120,   PD_MODE_RB        },   /* bit 2 */
	{ 1920,   1200,    60,   PD_MODE_RB        },   /* bit 3 */
	{ 1920,   1200,    60,   0                 },   /* bit 4 */
	{ 1920,   1200,    75,   0                 },   /* bit 5 */
	{ 1920,   1200,    85,   0                 },   /* bit 6 */
	{ 1920,   1200,   120,   PD_MODE_RB        },   /* bit 7 */

	/* byte 9 bit 0->7 */
	{ 1920,   1440,    60,   0                 },   /* bit 0 */
	{ 1920,   1440,    75,   0                 },   /* bit 1 */
	{ 1920,   1440,   120,   PD_MODE_RB        },   /* bit 2 */
	{ 2560,   1600,    60,   PD_MODE_RB        },   /* bit 3 */
	{ 2560,   1600,    60,   0                 },   /* bit 4 */
	{ 2560,   1600,    75,   0                 },   /* bit 5 */
	{ 2560,   1600,    85,   0                 },   /* bit 6 */
	{ 2560,   1600,   120,   PD_MODE_RB        },   /* bit 7 */
};

#ifndef CONFIG_MICRO
/*!
 * Function to replace common timings in 1st list with 2nd list, 2nd list
 * is unchanged.
 *
 * @param dtds1
 * @param dtds2
 *
 * @return void
 */
void replace_vesa_dtds_with_cea_dtds(igd_timing_info_t *dtds1,
	igd_timing_info_t *dtds2)
{
	igd_timing_info_t *temp;

	if (!dtds2 || !dtds1) {
		return;
	}

	while (dtds1->width != IGD_TIMING_TABLE_END) {
		temp = dtds2;

		while (temp->width != IGD_TIMING_TABLE_END) {
			if ((temp->width   == dtds1->width) &&
				(temp->height  == dtds1->height) &&
				(temp->refresh == dtds1->refresh)) {
				dtds1->mode_info_flags &= ~PD_MODE_SUPPORTED;
			}
			temp++;
		}
		dtds1++;
	}
}
#endif

#ifdef DEBUG_FIRMWARE
/*!
 *
 * @param db
 *
 * @return void
 */
void displayid_print_datablock(datablock_t *db)
{
	unsigned char payload_string[800];
	unsigned char i, j;

	/* Get the payload data into a string */
	OS_MEMSET(payload_string, 0, sizeof(payload_string));
	for (i=0, j=0; i<db->payload; i++,j+=5) {
		payload_string[j] = '0';
		payload_string[j+1] = 'x';
		if ((db->payload_data[i]>>4) <= 0x9) {
			payload_string[j+2] = '0' + (db->payload_data[i]>>4);
		} else {
			payload_string[j+2] = 'A' + (db->payload_data[i]>>4) - 0xA;
		}
		if ((db->payload_data[i] & 0x0F) <= 0x9) {
			payload_string[j+3] = '0' + (db->payload_data[i]&0x0F);
		} else {
			payload_string[j+3] = 'A' + (db->payload_data[i]&0x0F) - 0xA;
		}
		payload_string[j+4] = ' ';
	}
	payload_string[j] = '\0';

	EMGD_DEBUG("Tag = %u", db->tag);
	EMGD_DEBUG("Version = %u", db->revision);
	EMGD_DEBUG("Payload = %u", db->payload);
	EMGD_DEBUG("Payload data = %s", payload_string);
}

/*!
 *
 * @param buffer
 * @param did
 *
 * @return void
 */
void displayid_print(unsigned char *buffer, displayid_t *did)
{
	unsigned short i;
	display_params_t *dp = &did->display_params;
	timing_range_t   *tr = &did->timing_range;
	lvds_display_t   *ld = &did->lvds;
	display_dev_t    *dd = &did->display_dev;
	display_intf_t   *di = &did->display_intf;
#ifndef CONFIG_MICRO
	productid_t      *pi = &did->productid;
	color_char_t     *cc = &did->color_char;
	serial_number_t  *sn = &did->serial_num;
	general_string_t *gs = &did->general_string;
	transfer_char_t  *tc = &did->transfer_char;
	stereo_intf_t    *si = &did->stereo_intf;
	vendor_t         *vi = &did->vendor;
#endif

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("DisplayID Version: %d", did->version);
	EMGD_DEBUG("DisplayID Revision: %d", did->revision);
	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("Size of different structures:");
	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("     displayid_t = %d", sizeof(displayid_t));
	EMGD_DEBUG("display_params_t = %d", sizeof(display_params_t));
	EMGD_DEBUG("     type1_dtd_t = %d", sizeof(type1_dtd_t));
	EMGD_DEBUG("     type2_dtd_t = %d", sizeof(type2_dtd_t));
	EMGD_DEBUG("     type3_cvt_t = %d", sizeof(type3_cvt_t));
	EMGD_DEBUG("      type_std_t = %d", sizeof(type_std_t));
	EMGD_DEBUG("  timing_range_t = %d", sizeof(timing_range_t));
	EMGD_DEBUG("   display_dev_t = %d", sizeof(display_dev_t));
	EMGD_DEBUG("  lvds_display_t = %d", sizeof(lvds_display_t));
	EMGD_DEBUG("  display_intf_t = %d", sizeof(display_intf_t));
	EMGD_DEBUG("        dummy_db = %d", 256);
	EMGD_DEBUG("         timings = %d",
		sizeof(pd_timing_t)*DISPLAYID_MAX_NUM_TIMINGS);
	EMGD_DEBUG("           attrs = %d",
		sizeof(pd_attr_t)*DISPLAYID_MAX_ATTRS);

#ifndef CONFIG_MICRO
	EMGD_DEBUG("     productid_t = %d", sizeof(productid_t));
	EMGD_DEBUG("    color_char_t = %d", sizeof(color_char_t));
	EMGD_DEBUG(" serial_number_t = %d", sizeof(serial_number_t));
	EMGD_DEBUG("general_string_t = %d", sizeof(general_string_t));
	EMGD_DEBUG(" transfer_char_t = %d", sizeof(transfer_char_t));
	EMGD_DEBUG("   stereo_intf_t = %d", sizeof(stereo_intf_t));
	EMGD_DEBUG("        vendor_t = %d", sizeof(vendor_t));

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("PRODUCT ID DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)pi);
	EMGD_DEBUG("       vendor = %c%c%c",
		pi->vendor[0], pi->vendor[1], pi->vendor[2]);
	EMGD_DEBUG(" product_code = %u", pi->product_code);
	EMGD_DEBUG("serial_number = %lu", pi->serial_number);
	EMGD_DEBUG("    manf_week = %u", pi->manf_week);
	EMGD_DEBUG("    manf_year = %u", pi->manf_year+2000);
	EMGD_DEBUG("   string_len = %u", pi->string_size);
	EMGD_DEBUG("       string = %s", pi->string);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("COLOR CHARACTERISTICS DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)cc);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("SERIAL NUMBER DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)sn);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("GENERAL PURPOSE ASCII STRING DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)gs);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("TRANSFER CHARACTERISTICS DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)tc);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("STEREO INTERFACE DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)si);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("VENDOR SPECIFIC DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)vi);
#endif

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("DISPLAY PARAMETERS DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)dp);
	EMGD_DEBUG("horz_image_size = %u", dp->horz_image_size);
	EMGD_DEBUG("vert_image_size = %u", dp->vert_image_size);
	EMGD_DEBUG("    horz_pixels = %u", dp->horz_pixels);
	EMGD_DEBUG("    vert_pixels = %u", dp->vert_pixels);
	EMGD_DEBUG(" deinterlacable = %u", dp->deinterlacing);
	EMGD_DEBUG("   fixed_timing = %u", dp->fixed_timing);
	EMGD_DEBUG("      fixed_res = %u", dp->fixed_res);
	EMGD_DEBUG("   aspect_ratio = %u", dp->aspect_ratio);
	EMGD_DEBUG(" native_color_depth(bppc) = %u", dp->native_color_depth+1);
	EMGD_DEBUG("overall_color_depth(bppc) = %u", dp->overall_color_depth+1);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("VIDEO TIMING RANGESS DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)tr);
	EMGD_DEBUG("  min_dclk = %lu KHz", tr->min_dclk);
	EMGD_DEBUG("  max_dclk = %lu KHz", tr->max_dclk);
	EMGD_DEBUG(" min_hrate = %u KHz", tr->min_hrate);
	EMGD_DEBUG(" max_hrate = %u KHz", tr->max_hrate);
	EMGD_DEBUG("min_hblank = %u pixels", tr->min_hblank);
	EMGD_DEBUG(" min_vrate = %u Hz", tr->min_vrate);
	EMGD_DEBUG(" max_vrate = %u Hz", tr->max_vrate);
	EMGD_DEBUG("min_vblank = %u lines", tr->min_vblank);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("LVDS DISPLAY DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)ld);
	EMGD_DEBUG("min_T1 = %u ms", ld->min_t1/10);
	EMGD_DEBUG("max_T1 = %u ms", ld->max_t1*2);
	EMGD_DEBUG("max_T2 = %u ms", ld->max_t2*2);
	EMGD_DEBUG("max_T3 = %u ms", ld->max_t3*2);
	EMGD_DEBUG("min_T4 = %u ms", ld->min_t4*10);
	EMGD_DEBUG("min_T5 = %u ms", ld->min_t5*10);
	EMGD_DEBUG("min_T6 = %u ms", ld->min_t6*10);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("DISPLAY DEVICE DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)dd);
	EMGD_DEBUG("   horz_pixel_count = %u", dd->horz_pixel_count);
	EMGD_DEBUG("   vert_pixel_count = %u", dd->vert_pixel_count);
	EMGD_DEBUG("display_color_depth = %u", dd->display_color_depth);

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("DISPLAY INTERFACE DATA BLOCK");
	DISPLAYID_PRINT_LINE();
	displayid_print_datablock((datablock_t *)di);
	EMGD_DEBUG("         num_channels = %u", di->num_channels);
	EMGD_DEBUG("            intf_type = %u", di->intf_type);
	EMGD_DEBUG("      RGB_color_depth = %u", di->rgb_color_depth);
	EMGD_DEBUG("YCrCb_444_color_depth = %u", di->ycbcr_444_color_depth);
	EMGD_DEBUG("YCrCb_422_color_depth = %u", di->ycbcr_422_color_depth);
	if(di->intf_type == INTERFACE_LVDS) {
		EMGD_DEBUG("         openldi = %u", di->lvds.openldi);
	}

	DISPLAYID_PRINT_LINE();
	EMGD_DEBUG("Detailed Timing Descriptors");
	DISPLAYID_PRINT_LINE();

	for (i=0; i<did->num_timings; i++) {
		EMGD_DEBUG("DTD: %u", i+1);
		EMGD_DEBUG("          dclk = %lu", did->timings[i].dclk);
		EMGD_DEBUG("       hactive = %u", did->timings[i].width);
		EMGD_DEBUG("        htotal = %u", did->timings[i].htotal);
		EMGD_DEBUG("  hblank_start = %u", did->timings[i].hblank_start);
		EMGD_DEBUG("   hsync_start = %u", did->timings[i].hsync_start);
		EMGD_DEBUG("     hsync_end = %u", did->timings[i].hsync_end);
		EMGD_DEBUG("    hblank_end = %u", did->timings[i].hblank_end);
		EMGD_DEBUG("       vactive = %u", did->timings[i].height);
		EMGD_DEBUG("        vtotal = %u", did->timings[i].vtotal);
		EMGD_DEBUG("  vblank_start = %u", did->timings[i].vblank_start);
		EMGD_DEBUG("   vsync_start = %u", did->timings[i].vsync_start);
		EMGD_DEBUG("     vsync_end = %u", did->timings[i].vsync_end);
		EMGD_DEBUG("    vblank_end = %u", did->timings[i].vblank_end);
		EMGD_DEBUG("        native = %u",
			(did->timings[i].mode_info_flags&PD_MODE_DTD_FP_NATIVE)?1:0);
		EMGD_DEBUG("     interlace = %u",
			(did->timings[i].mode_info_flags&PD_SCAN_INTERLACE)?1:0);
		EMGD_DEBUG("hsync_polarity = %s",
			(did->timings[i].mode_info_flags & PD_HSYNC_HIGH)?
			"ACTIVE HIGH":"ACTIVE LOW");
		EMGD_DEBUG("vsync_polarity = %s",
			(did->timings[i].mode_info_flags & PD_VSYNC_HIGH)?
			"ACTIVE HIGH":"ACTIVE LOW");
		DISPLAYID_PRINT_LINE();
	}

	/* Print the attributes */
	if (did->num_attrs) {
		EMGD_DEBUG("\tAttr\tID\tVALUE");
		EMGD_DEBUG("----------------------");
		for (i=0; i<did->num_attrs; i++) {
			EMGD_DEBUG("\t%u\t%lu\t%lu", i+1, did->attr_list[i].id,
				did->attr_list[i].current_value);
		}
		EMGD_DEBUG("----------------------");
	}
}
#endif

/*!
 * Function to convert Type I - Detailed to pd_timing_t
 *
 * @param timing
 * @param dtd
 *
 * @return void
 */
void convert_type1_to_pd(pd_timing_t *timing, type1_dtd_t *dtd)
{
	unsigned long refresh;
	timing->dclk =                            /* change to KHz */
		((unsigned long)dtd->dclk.lsb_dclk|
		((unsigned long)dtd->dclk.msb_dclk<<16))*10;

	/* DisplayID fields are 0 based but should be interpreted as 1-based.
	 * For example hsync_width value can be read as 0-65,535 pixels but
	 * interpreted as 1-65,536. So, to get the right value add 1.
	 * But pd_timing_t values are 0 based except width and height,
	 * so care should be taken while converting DisplayID fields into
	 * pd_timing_t values */
	timing->hblank_start = dtd->hactive;
	timing->width = dtd->hactive + 1;
	timing->hblank_end = timing->hblank_start + dtd->hblank + 1;
	timing->hsync_start = timing->hblank_start + dtd->hsync_offset + 1;
	timing->hsync_end = timing->hsync_start + dtd->hsync_width + 1;
	timing->htotal = timing->hblank_end;

	timing->vblank_start = dtd->vactive;
	timing->height = dtd->vactive + 1;
	timing->vblank_end = timing->vblank_start + dtd->vblank + 1;
	timing->vsync_start = timing->vblank_start + dtd->vsync_offset + 1;
	timing->vsync_end = timing->vsync_start + dtd->vsync_width + 1;
	timing->vtotal = timing->vblank_end;

	refresh = ((timing->dclk * 1000L)/timing->htotal)/timing->vtotal;
	timing->refresh = (unsigned short) refresh;

	timing->mode_info_flags = PD_MODE_DTD|PD_MODE_SUPPORTED;
	if (dtd->hsync_polarity) {
		timing->mode_info_flags |= PD_HSYNC_HIGH;
	}
	if (dtd->vsync_polarity) {
		timing->mode_info_flags |= PD_VSYNC_HIGH;
	}
	if (dtd->interlaced) {
		timing->mode_info_flags |= PD_SCAN_INTERLACE;
	}
	if (dtd->preferred) {
		timing->mode_info_flags |= PD_MODE_DTD_FP_NATIVE;
	}
}

/*!
 * Function to convert Type II - Detailed to pd_timing_t
 *
 * @param timing
 * @param dtd
 *
 * @return void
 */
void convert_type2_to_pd(pd_timing_t *timing, type2_dtd_t *dtd)
{
	unsigned long refresh;
	timing->dclk =                            /* change to KHz */
		((unsigned long)dtd->dclk.lsb_dclk|
		((unsigned long)dtd->dclk.msb_dclk<<16))*10;

	/* DisplayID fields are 0 based but should be interpreted as 1-based.
	 * For example hsync_width value can be read as 0-15 OCTETs but
	 * interpreted as 1-16 OCTETs. So, to get the right value add 1.
	 * But pd_timing_t values are 0 based except width and height,
	 * so care should be taken while converting DisplayID fields into
	 * pd_timing_t values */
	timing->width = (dtd->hactive + 1) * 8;    /* change to pixels */
	timing->hblank_start = timing->width - 1;
	timing->hblank_end = timing->hblank_start + (dtd->hblank + 1) * 8;
	timing->hsync_start = timing->hblank_start + (dtd->hsync_offset + 1) * 8;
	timing->hsync_end = timing->hsync_start + (dtd->hsync_width + 1) * 8;
	timing->htotal = timing->hblank_end;

	timing->vblank_start = dtd->vactive;
	timing->height = dtd->vactive + 1;
	timing->vblank_end = timing->vblank_start + dtd->vblank + 1;
	timing->vsync_start = timing->vblank_start + dtd->vsync_offset + 1;
	timing->vsync_end = timing->vsync_start + dtd->vsync_width + 1;
	timing->vtotal = timing->vblank_end;

	refresh = ((timing->dclk * 1000L)/timing->htotal)/timing->vtotal;
	timing->refresh = (unsigned short) refresh;

	timing->mode_info_flags = PD_MODE_DTD|PD_MODE_SUPPORTED;
	if (dtd->interlaced) {
		timing->mode_info_flags |= PD_SCAN_INTERLACE;
	}
	if (dtd->preferred) {
		timing->mode_info_flags |= PD_MODE_DTD_FP_NATIVE;
	}
}

/*!
 * Function to filter timing table based on range block
 *
 * @param tt
 * @param range
 * @param firmware_type
 *
 * @return void
 */
void displayid_filter_range_timings(pd_timing_t *tt, timing_range_t *range,
	unsigned char firmware_type)
{
	unsigned short hfreq;

#define _HUNDRETHS(_n, _d)  ((100*_n)/_d)-((100*_n_d)/100),

	#ifdef DEBUG_FIRMWARE
	char result_str[60];
	unsigned char pass_count = 0;
	unsigned char fail_count = 0;

	EMGD_DEBUG("Range limits:");
	EMGD_DEBUG("\tmin_dclk = %lu KHz max_dclk = %lu KHz",
		range->min_dclk, range->max_dclk);
	EMGD_DEBUG("\t   h_min = %u h_max = %u KHz",
		range->min_hrate, range->max_hrate);
	EMGD_DEBUG("\t   v_min = %u v_max = %u",
		range->min_vrate,range->max_vrate);
	EMGD_DEBUG("WIDTH\tHEIGHT\tREFRESH\tH-FREQ\tDOTCLOCK\tRESULT");
	EMGD_DEBUG("     \t      \t (Hz)  \t (KHz)\t (MHz)  \t      ");
	EMGD_DEBUG("=====\t======\t=======\t======\t========\t======");
#endif

	/* If the display is a discreate frequency display, don't enable any
	 * intermediate timings. Only continuous frequency displays requires
	 * enabling range timings */
	if (range->discrete_display) {
		EMGD_DEBUG("Discrete display: Ranges aren't used.");
		return;
	}

	/* If no timing table return */
	if (tt == NULL) {
		return;
	}

	/* Mark the timings that fall in the ranges */
	/* Compare
	 *     dclk in KHz
	 *     hfreq in KHz
	 *     vfreq in Hz */
	while(tt->width != IGD_TIMING_TABLE_END) {
		hfreq = (unsigned short)(tt->dclk/(unsigned long)tt->htotal); /* KHz */
		if ((tt->dclk    >= (unsigned long)range->min_dclk)&&  /* compare KHz */
			(tt->dclk    <= (unsigned long)range->max_dclk)&&  /* compare KHz */
			(tt->refresh >= range->min_vrate) &&   /* compare Hz */
			(tt->refresh <= range->max_vrate) &&   /* compare Hz */
			(hfreq       >= range->min_hrate) &&   /* compare KHz */
			(hfreq       <= range->max_hrate) &&   /* compare KHz */
			(tt->hblank_end - tt->hblank_start) > range->min_hblank &&
			(tt->vblank_end - tt->vblank_start) > range->min_vblank) {
			tt->mode_info_flags |= PD_MODE_SUPPORTED;
#ifdef DEBUG_FIRMWARE
			if (tt->mode_info_flags & PD_MODE_SUPPORTED) {
				EMGD_DEBUG("%5u\t%6u\t%7u\t%6u.%2u\t%8u.%2u\tPASSED",
					tt->width, tt->height, tt->refresh,
					tt->dclk/tt->htotal,
					_HUNDRETHS(tt->dclk,tt->htotal),
					tt->dclk/1000,
					_HUNDRETHS(tt->dclk,1000);

				pass_count++;
			} else {
			}
#endif
		} else {
			/* Unmark the mode that falls out of range */
			/* DTD, FACTORY and NATIVE timings are "GOLD" even if they
			 * fall outside the range limits */
			if (!(tt->mode_info_flags &
				(PD_MODE_DTD|PD_MODE_FACTORY|PD_MODE_DTD_FP_NATIVE))) {
#ifdef DEBUG_FIRMWARE
				if ((tt->dclk <            /* compare KHz */
						(unsigned long)range->min_dclk)||
					(tt->dclk >
						(unsigned long)range->max_dclk)) {
					OS_MEMCPY(result_str, "FAILED DCLK    \0", 16);
					fail_count++;
				} else if ((tt->refresh > range->max_vrate) ||
					(tt->refresh < range->min_vrate)) {
					OS_MEMCPY(result_str, "FAILED REFRESH \0", 16);
					fail_count++;
				} else if ((hfreq < range->min_hrate) ||
					(hfreq > range->max_hrate)) {
					OS_MEMCPY(result_str, "FAILED H-FREQ  \0", 16);
					fail_count++;
				} else if ((tt->hblank_end-tt->hblank_start) <
					range->min_hblank){
					OS_MEMCPY(result_str, "FAILED MIN_HBLK\0", 16);
				} else if ((tt->vblank_end-tt->vblank_start) <
					range->min_vblank){
					OS_MEMCPY(result_str, "FAILED MIN_VBLK\0", 16);
				}
				EMGD_DEBUG("%5u\t%6u\t%7u\t%6u.%2u\t%8u.%2u\t%s",
					tt->width, tt->height, tt->refresh,
					tt->dclk/tt->htotal,
					_HUNDRETHS(tt->dclk,tt->htotal),
					tt->dclk/1000,
					_HUNDRETHS(tt->dclk,1000),
					result_str);
					((float) tt->dclk)/1000, result_str);

				/* TODO: For multiple range blocks, don't disable the modes
				 * that are outside the range. We already started with
				 * an "empty supported table" */

				/* But above assertion of "empty supported table" broke
				 * if EDID ETF rules were met to enable all timings.
				 * See edid.c for ETF conditions. So below line
				 * cannot be commented out to support multiple range
				 * blocks for DisplayID. */
#endif
				tt->mode_info_flags &= ~PD_MODE_SUPPORTED;
			}
		}
		tt++;
		if (tt->width == IGD_TIMING_TABLE_END && tt->extn_ptr) {
			tt = tt->extn_ptr;
		}
	}
#ifdef DEBUG_FIRMWARE
	EMGD_DEBUG("pass count = %u, fail count = %u total = %u",
		pass_count, fail_count, pass_count+fail_count);
#endif
} /* end displayid_filter_range_timings() */

#define VESA_STD    1
#define CEA_STD     2

/*!
 * Function to enable std timings: VESA STD or CEA STD
 *
 * @param tt1
 * @param db_data
 * @param lookup
 * @param num_lookup
 * @param std_type
 *
 * @return void
 */
void displayid_enable_std_timings(pd_timing_t *tt1, unsigned char *db_data,
	type_std_t *lookup, unsigned short num_lookup, unsigned char std_type)
{
	unsigned short i;
	pd_timing_t    *tt;
	/* If no timing table return. This can happen if no edid_avail set not to
	 * use std timings */
	if (!tt1) {
		return;
	}

	/* For every factory supported mode, enable it in the timing table */
	for (i = 0; i < num_lookup; i++) {
		tt = tt1;
		/* i>>3 is nothing but dividing by 8, that gives the byte number,
		 * i&0x7 is nothing but getting the bit position in that byte */
		if (db_data[i>>3] & 1<<(i&0x7)) {
			while(tt->width != IGD_TIMING_TABLE_END) {
				if (lookup[i].width == tt->width &&
					lookup[i].height == tt->height &&
#if 0
					(!((lookup[i].flags & (PD_SCAN_INTERLACE|PD_MODE_RB)) ^
					(tt->mode_info_flags & (PD_SCAN_INTERLACE|PD_MODE_RB)))) &&
#endif
					(!((lookup[i].flags & PD_SCAN_INTERLACE) ^
					(tt->mode_info_flags & PD_SCAN_INTERLACE))) &&
					(!((lookup[i].flags & PD_ASPECT_16_9) ^
					(tt->mode_info_flags & PD_ASPECT_16_9))) &&
					lookup[i].refresh == tt->refresh) {
					tt->mode_info_flags |= (PD_MODE_FACTORY|PD_MODE_SUPPORTED);
					break;
				}
				tt++;
				if (tt->width == IGD_TIMING_TABLE_END && tt->extn_ptr) {
					tt = tt->extn_ptr;
				}
			}
		}
	}
}

static void displayid_parse_orientation_info (unsigned char orientation_blob,
	igd_DID_rotation_info_t * rotation_info) {

	unsigned int def_orientation = DEFAULT_ORIENTATION(orientation_blob);
	unsigned int zero_pixel = ZERO_PIXEL(orientation_blob);
	unsigned int scan_dir = SCAN_DIRECTION(orientation_blob);

	/* Start with no rotation */
	rotation_info->rotation = 0;
	rotation_info->flip = 0;


	if (def_orientation >= DEF_ORIENTATION_UNKNOWN) {
		/* Display ID blob is corrupted or unknown configuration set */
		EMGD_DEBUG("DisplayID: Unknown default orientation value: %d",
			def_orientation);
		return;
	}

	if (scan_dir == SCAN_DIRECTION_LONG) {
		if (def_orientation == DEF_ORIENTATION_PORTRAIT) {
			rotation_info->flip =
				(zero_pixel == ZP_UPPER_LEFT || zero_pixel == ZP_LOWER_RIGHT);
			rotation_info->rotation += 90;
		} else { /* Landscape */
			rotation_info->flip =
				(zero_pixel == ZP_UPPER_RIGHT || zero_pixel == ZP_LOWER_LEFT);
		}

	} else if (scan_dir == SCAN_DIRECTION_SHORT) {
		if (def_orientation == DEF_ORIENTATION_PORTRAIT) {
			rotation_info->flip =
				(zero_pixel == ZP_UPPER_RIGHT || zero_pixel == ZP_LOWER_LEFT);
		}
		else { /* Landscape */
			rotation_info->flip =
				(zero_pixel == ZP_UPPER_LEFT || zero_pixel == ZP_LOWER_RIGHT);
			rotation_info->rotation += 90;
		}
	} else { /* Unknown scan direction */
		EMGD_DEBUG("DisplayID: Unknown scan direction value: %d", scan_dir);
		return;
	}

	/* zero pixel is in the lower-half: need to rotate by 180 degs */
	if (zero_pixel == ZP_LOWER_LEFT || zero_pixel == ZP_LOWER_RIGHT)
		rotation_info->rotation += 180;
}

/*!
 * Function to parse DisplayID
 *
 * @param buffer
 * @param did
 * @param timing_table
 * @param count
 * @param upscale
 *
 * @return void
 */
int displayid_parse(
		unsigned char *buffer,
		displayid_t   *did,
		pd_timing_t   *timing_table,
		int           count,
		unsigned char upscale)
{
	//unsigned char e = 0;
	unsigned char checksum = 0, bytes_left;
	unsigned short i;
	unsigned short did_size;
#ifndef CONFIG_MICRO
	pd_timing_t   *cea_tmg_table;
#endif
	/* Read 4 bytes: (DisplayID Header)
	 *       version, revision
	 *       payload
	 *       display product type identifier
	 *       number of extensions */
	*(unsigned long *) did = *(unsigned long *)buffer;

	/* Check for version and revision */
	if (did->version != 1 && did->revision != 0) {
		EMGD_DEBUG("DisplayID Version %d.%d Unknown. Will Ignore.",
			did->version, did->revision);
		return DISPLAYID_NOT_SUPPORTED;
	}

	if (did->payload > 251) {
		EMGD_DEBUG("DispID: Error: payload = %u not in [0..251]", did->payload);
		return DISPLAYID_ERROR_PARSE;
	}

	/* Check sum check */
	/* +5 is for 5 mandatory bytes */
	did_size = (unsigned short) (did->payload + 5);
	EMGD_DEBUG("DisplayID size = %u", did_size);
	for (i = 0; i < did_size; i++) {
		checksum += buffer[i];
	}

	/* bytes_left starts without DisplayID header */
	bytes_left = did->payload;
	/* current pointer is at 4 not at 5, because checksum byte is at the end */
	buffer += 4;

	if (checksum) {
		EMGD_DEBUG("DisplayID checksum is incorrect! Will ignore.");
		return DISPLAYID_ERROR_PARSE;
	}

	/* DisplayID parsing should start by disabling all modes.
	 * Based on DisplayID data blocks modes will be enabled. */
	enable_disable_timings(timing_table, 0);

	/* Repeat for all extensions */
	//e = did->num_extensions;
	//while (e) {
	{
		//if (e != did->num_extensions) {
			/* TODO: If there aren't enough bytes left in the buffer,
			 * call I2C read function to read next DisplayID section */

			/* Skip next section header 4 bytes */
			//bytes_left -= 4;
			//break;
		//}

		/* Parse Data Blocks */
		/* Check minimum number of bytes required for Data Block were left */
		while ((bytes_left > 3) && (bytes_left >= (buffer[2]+3))) {
			unsigned char *db_data;
			unsigned char payload = buffer[2] + 3;

			/* displayid->datablock = buffer (for payload bytes) */
			if (buffer[0] < sizeof(db_offset)/sizeof(unsigned short)) {
				OS_MEMCPY(((unsigned char*)did) + db_offset[buffer[0]],
					buffer, payload);
			}

			/* db_data points to payload data after db header (3 bytes),
			 * Note: dummy_db offset is used for some DATA BLOCKS. See
			 *       db_offset table above. */
			db_data = (unsigned char *) &did->dummy_db[3];

			switch (buffer[0]) {
			/* Supported in Driver and VBIOS */
			case DATABLOCK_DISPLAY_PARAMS:
				/* Use following fields for fp_info:
				 *     embedded use:      fixed timing
				 *     horizontal pixels: fp_width
				 *     vertical pixels:   fp_height */
				did->attr_list[did->num_attrs].id = PD_ATTR_ID_PANEL_DEPTH;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					(did->display_params.overall_color_depth+1)*3;
				break;

			case DATABLOCK_TIMING_1_DETAIL:
				/* One Type I block can have multiple DTDs */
				while (payload>=20&&did->num_timings<DISPLAYID_MAX_NUM_TIMINGS){
					convert_type1_to_pd(&did->timings[did->num_timings++],
						(type1_dtd_t *)db_data);
					db_data += 20;
					payload -= 20;
					bytes_left -= 20;
					buffer += 20;
				}
				/* Mark the end of the list */
				did->timings[did->num_timings].width = IGD_TIMING_TABLE_END;
				break;

			case DATABLOCK_TIMING_2_DETAIL:
				/* One Type II block can have multiple DTDs */
				while (payload>=11&&did->num_timings<DISPLAYID_MAX_NUM_TIMINGS){
					convert_type2_to_pd(&did->timings[did->num_timings++],
						(type2_dtd_t *)db_data);
					db_data += 11;
					payload -= 11;
					bytes_left -= 11;
					buffer += 11;
				}
				did->timings[did->num_timings].width = IGD_TIMING_TABLE_END;
				break;

			case DATABLOCK_VESA_TIMING_STD:
				/* VESA Standard Timings */
				displayid_enable_std_timings(
					timing_table,
					db_data,
					vesa_std_lookup,
					sizeof(vesa_std_lookup)/sizeof(type_std_t),
					VESA_STD);
				break;

			case DATABLOCK_VIDEO_RANGE:
				/* convert from Hz/10,000 -> KHz by multiplying by 10 */
				did->timing_range.min_dclk =
					((unsigned long)did->timing_range.mindclk.lsb_min_dclk|
					((unsigned long)did->timing_range.mindclk.msb_min_dclk
						<<16))*10;

				did->timing_range.max_dclk =
					((unsigned long)did->timing_range.maxdclk.lsb_max_dclk|
					((unsigned long)did->timing_range.maxdclk.msb_max_dclk
						<<16))*10;
				displayid_filter_range_timings(timing_table,&did->timing_range,
					PI_FIRMWARE_DISPLAYID);
				break;

			case DATABLOCK_DISPLAY_DEVICE:
				/* Get panel color depth */
				did->attr_list[did->num_attrs].id = PD_ATTR_ID_PANEL_DEPTH;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					(did->display_dev.display_color_depth+1)*3;
				displayid_parse_orientation_info(did->display_dev.orientation,
												&(did->rotation_info));
				break;

			case DATABLOCK_LVDS_INTERFACE:
				/* Get T1-T5 values */
				did->attr_list[did->num_attrs].id = PD_ATTR_ID_FP_PWR_T1;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					did->lvds.max_t1*2 + did->lvds.max_t2*2;

				did->attr_list[did->num_attrs].id = PD_ATTR_ID_FP_PWR_T2;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					did->lvds.min_t5*10;

				did->attr_list[did->num_attrs].id = PD_ATTR_ID_FP_PWR_T3;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					did->lvds.min_t6*10;

				did->attr_list[did->num_attrs].id = PD_ATTR_ID_FP_PWR_T4;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					did->lvds.max_t3*2;

				did->attr_list[did->num_attrs].id = PD_ATTR_ID_FP_PWR_T5;
				did->attr_list[did->num_attrs].flags=PD_ATTR_FLAG_VALUE_CHANGED;
				did->attr_list[did->num_attrs++].current_value =
					did->lvds.min_t4*10 + did->lvds.max_t1*2;
				break;

			case DATABLOCK_DISPLAY_INTF:
				if (did->display_intf.intf_type == INTERFACE_LVDS) {
					/* Get number of channels: 0=singlechannel 1=dualchannel */
					did->attr_list[did->num_attrs].id =
						PD_ATTR_ID_2_CHANNEL_PANEL;
					did->attr_list[did->num_attrs].flags =
						PD_ATTR_FLAG_VALUE_CHANGED;
					if (did->display_intf.num_channels == 2) {
						did->attr_list[did->num_attrs++].current_value = 1;
					}

					/* Get panel type value: 0=normal 1=OpenLDI */
					did->attr_list[did->num_attrs].id =
						PD_ATTR_ID_LVDS_PANEL_TYPE;
					did->attr_list[did->num_attrs].flags =
						PD_ATTR_FLAG_VALUE_CHANGED;
					did->attr_list[did->num_attrs++].current_value =
						did->display_intf.lvds.openldi;
				}

				break;

#ifndef CONFIG_MICRO
			/* Support in Driver only */
			case DATABLOCK_PRODUCTID:
				break;

			case DATABLOCK_SERIAL_NUMBER:
				break;

			case DATABLOCK_ASCII_STRING:
				break;

			case DATABLOCK_VENDOR_SPECIFIC:
				/* Because vendor specific datablock tag is out-of-order,
				 * copy data from buffer to vendor structure */
				OS_MEMCPY(&did->vendor, buffer, buffer[2] + 3);
				break;

			/* Future support in Driver and VBIOS */
			case DATABLOCK_TIMING_3_SHORT:
				break;

			case DATABLOCK_TIMING_4_DMTID:
				break;

			/* Future support in Driver */
			case DATABLOCK_COLOR_CHARS:
				break;

			case DATABLOCK_CEA_TIMING_STD:
				cea_tmg_table = (igd_timing_info_t *)
					OS_ALLOC(cea_timing_table_size);
				OS_MEMCPY(cea_tmg_table, cea_timing_table,
					cea_timing_table_size);
				/* Disable the CEA timings */
				enable_disable_timings(cea_tmg_table, 0);
				displayid_enable_std_timings(
					cea_tmg_table,
					db_data,
					cea_std_lookup,
					(unsigned short)cea_std_lookup_size,
					CEA_STD);

				replace_vesa_dtds_with_cea_dtds(timing_table, cea_tmg_table);
				cea_tmg_table[cea_timing_table_size-1].extn_ptr =
					(void *)timing_table;
				timing_table = cea_tmg_table;
				break;
#endif
			case DATABLOCK_TRANSFER_CHAR:
				break;
			}

			/* Subtract data block payload */
			bytes_left -= payload;
			buffer += payload;
		}
		/* Extension count */
		//e--;
	}

	return 0;
}

#endif


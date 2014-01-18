/*
 *-----------------------------------------------------------------------------
 * Filename: edid.c
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
 *  This file is contains all necessary functions for EDID reading and
 *  parsing into a data strucutures.
 *  Supported EDID versions:
 *  EDID 1.3 (backward compatible with 1.1, 1.2)
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <io.h>

#include <igd_errno.h>

#include <memory.h>
#include <displayid.h>
#include <edid.h>
#include <pi.h>

/*!
 * @addtogroup display_group
 * @{
 */

/* Function to read EDID */
/* Local functions */
static void edid_mark_standard_timings(
	unsigned char *buffer,
	pd_timing_t   *timings,
	int           established);

static void edid_mark_detailed_timings(
	unsigned char *buffer,
	edid_t        *edid,
	pd_timing_t   *timings,
	unsigned char upscale);

static void edid_parse_monitor_name(
	unsigned char *buffer,
	char          *name);

/* Edid Releated Constants */

static const unsigned char name_blockid[] = {0x00, 0x00, 0x00, 0xfc};
static const unsigned char range_blockid[] = {0x00, 0x00, 0x00, 0xfd};
static const unsigned char st_blockid[] = {0x00, 0x00, 0x00, 0xfa};
static const unsigned char timings_mask[] = {0xff, 0xff, 0xff, 0x00};
extern igd_timing_info_t cea_timing_table[];
/*
 * Translation from the "Established" bit field position to a
 * hactive, vactive, refresh that can be searched for in the
 * timings list.
 */
static unsigned char established_table[] = {
	/* 800x600 @ 60Hz */
	0x20, 0x03, 0x58, 0x02, 0x3c, 0x00,
	/* 800x600 @ 56Hz */
	0x20, 0x03, 0x58, 0x02, 0x38, 0x00,
	/* 640x480 @ 75Hz */
	0x80, 0x02, 0xe0, 0x01, 0x4b, 0x00,
	/* 640x480 @ 72Hz */
	0x80, 0x02, 0xe0, 0x01, 0x48, 0x00,
	/* 640x480 @ 67Hz */
	0x80, 0x02, 0xe0, 0x01, 0x43, 0x00,
	/* 640x480 @ 60Hz */
	0x80, 0x02, 0xe0, 0x01, 0x3c, 0x00,
	/* 720x400 @ 88Hz */
	0xd0, 0x02, 0x90, 0x01, 0x58, 0x00,
	/* 720x400 @ 70Hz */
	0xd0, 0x02, 0x90, 0x01, 0x46, 0x00,

	/* 1280x1024 @ 75Hz */
	0x00, 0x05, 0x00, 0x04, 0x4b, 0x00,
	/* 1024x768 @ 75Hz */
	0x00, 0x04, 0x00, 0x03, 0x4b, 0x00,
	/* 1024x768 @ 70Hz */
	0x00, 0x04, 0x00, 0x03, 0x46, 0x00,
	/* 1024x768 @ 60Hz */
	0x00, 0x04, 0x00, 0x03, 0x3c, 0x00,
	/* 1024x768 @ 43Hz (i) */
	0x00, 0x04, 0x00, 0x03, 0x2b, 0x00,
	/* 832x624 @ 75Hz */
	0x40, 0x03, 0x70, 0x02, 0x4b, 0x00,
	/* 800x600 @ 75Hz */
	0x20, 0x03, 0x58, 0x02, 0x4b, 0x00,
	/* 800x600 @ 72Hz */
	0x20, 0x03, 0x58, 0x02, 0x48, 0x00,

	/* 1152x870 @ 75Hz */
	0x80, 0x04, 0x66, 0x03, 0x4b, 0x00
};

#ifdef DEBUG_FIRMWARE
/*!
 *
 * @param timing
 * @param edid
 *
 * @return void
 */
static void print_supported_timings(
	pd_timing_t *timing, edid_t *edid)
{
	unsigned short i, count = 0;
	if (timing != NULL) {
		while (timing->width != IGD_TIMING_TABLE_END) {
			if (timing->mode_info_flags & PD_MODE_SUPPORTED) {
				EMGD_DEBUG("\t%4u\t%4u\t%4u", timing->width,
					timing->height, timing->refresh);
				count++;
			}

			timing++;
			if (timing->width == IGD_TIMING_TABLE_END && timing->extn_ptr) {
				timing = timing->extn_ptr;
			}
		}
	}

	if (edid && edid->num_timings) {
		for (i=0; i<edid->num_timings; i++) {
			if (edid->timings[i].mode_info_flags & PD_MODE_SUPPORTED) {
				EMGD_DEBUG("\t%4u\t%4u\t%4u", edid->timings[i].width,
					edid->timings[i].height, edid->timings[i].refresh);
				count++;
			}
		}
	}
	EMGD_DEBUG("Total supported timings = %u", count);
}

/*!
 * Dump the EDID to the kernel messages for debug
 *
 * @param buffer
 * @param size
 *
 * @return void
 */
void firmware_dump(unsigned char *buffer, unsigned short size)
{
	unsigned short i;
	EMGD_DEBUG("---------------------------------------------------------");
	if (*(unsigned long *) &buffer[0] == 0xFFFFFF00 &&
		*(unsigned long *) &buffer[4] == 0x00FFFFFF) {
		size = 128;
		EMGD_DEBUG("EDID DUMP (size = %u):", size);
	} else {
		/* For DisplayID payload is at byte 2 */
		/* +5 is for 5 mandatory bytes */
		size = buffer[1]+5;
		EMGD_DEBUG("DisplayID DUMP (size = %u):", size);
	}
	EMGD_DEBUG("-------+-------------------------------------------------");
	EMGD_DEBUG("Offset | Data.....");
	EMGD_DEBUG("-------+-------------------------------------------------");

	for (i=0; i<size; i+=8) {
		EMGD_DEBUG("  0x%2.2x | 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x, 0x%2.2x",
			i, buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
			buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
	}
	if (i>size) {
		EMGD_DEBUG("Note: Ignore last row last %u bytes.", i-size);
	}
	EMGD_DEBUG("-------+-------------------------------------------------");
	EMGD_DEBUG(" ");
} /* edid_dump() */

/*!
 * Print the EDID Structure
 *
 * @param buffer
 * @param size
 *
 * @return void
 */
void edid_print(edid_t *edid)
{
	int i;

	EMGD_DEBUG("EDID Version: %d", edid->version);
	EMGD_DEBUG("EDID Revision: %d", edid->revision);
	EMGD_DEBUG("Vendor %s", edid->vendor);
	EMGD_DEBUG("Model: %s", edid->name);
	EMGD_DEBUG("Product Code: %ld", edid->product_code);
	EMGD_DEBUG("Serial Number: %lu", edid->serial_number);
	EMGD_DEBUG("Manufactored week: %d", edid->manf_week);
	EMGD_DEBUG("Manufactored year: %ld", edid->manf_year);
	EMGD_DEBUG("DPMS Flags: 0x%u", edid->dpms);
	EMGD_DEBUG("Max Pixel Clock: %lu KHz", edid->range.max_dclk);
	EMGD_DEBUG("H Range: %u - %u KHz",
		edid->range.min_hrate, edid->range.max_hrate);
	EMGD_DEBUG("V Range: %u - %u Hz",
		edid->range.min_vrate, edid->range.max_vrate);

	for (i=0; i<edid->num_timings; i++) {
		EMGD_DEBUG("Detailied Timing Descriptor(DTD) %u", i+1);
		EMGD_DEBUG("          dclk = %lu", edid->timings[i].dclk);
		EMGD_DEBUG("       hactive = %u", edid->timings[i].width);
		EMGD_DEBUG("        htotal = %u", edid->timings[i].htotal);
		EMGD_DEBUG("  hblank_start = %u", edid->timings[i].hblank_start);
		EMGD_DEBUG("   hsync_start = %u", edid->timings[i].hsync_start);
		EMGD_DEBUG("     hsync_end = %u", edid->timings[i].hsync_end);
		EMGD_DEBUG("    hblank_end = %u", edid->timings[i].hblank_end);
		EMGD_DEBUG("       vactive = %u", edid->timings[i].height);
		EMGD_DEBUG("        vtotal = %u", edid->timings[i].vtotal);
		EMGD_DEBUG("  vblank_start = %u", edid->timings[i].vblank_start);
		EMGD_DEBUG("   vsync_start = %u", edid->timings[i].vsync_start);
		EMGD_DEBUG("     vsync_end = %u", edid->timings[i].vsync_end);
		EMGD_DEBUG("    vblank_end = %u", edid->timings[i].vblank_end);
		EMGD_DEBUG("     interlace = %u",
			(edid->timings[i].mode_info_flags&PD_SCAN_INTERLACE)?1:0);
		EMGD_DEBUG("hsync_polarity = %s",
			(edid->timings[i].mode_info_flags & PD_HSYNC_HIGH)?
			"ACTIVE HIGH":"ACTIVE LOW");
		EMGD_DEBUG("vsync_polarity = %s",
			(edid->timings[i].mode_info_flags & PD_VSYNC_HIGH)?
			"ACTIVE HIGH":"ACTIVE LOW");
	}
} /* edid_print() */
#endif

/*!
 * Function to disable all timings before marking supportecd timings
 *
 * @param timing
 * @param enable
 *
 * @return void
 */
void enable_disable_timings(pd_timing_t *timing, unsigned char enable)
{
	if (!timing) {
		return;
	}

	while(timing->width != IGD_TIMING_TABLE_END) {
		if (enable) {
			timing->mode_info_flags |= PD_MODE_SUPPORTED;
		} else {
			timing->mode_info_flags &= ~PD_MODE_SUPPORTED;
		}
		timing++;
		if (timing->width == IGD_TIMING_TABLE_END && timing->extn_ptr) {
			timing = timing->extn_ptr;
		}
	}
}

/*!
 * Function to enable timings based on port driver flags
 *
 * @param timing
 * @param enable
 *
 * @return void
 */
void enable_scaled_timings(pd_timing_t *timing, pd_timing_t *dtd,
	unsigned char upscale)
{

	if (!upscale || !dtd || !timing) {
		return;
	}

	/* If port driver supports up scaling, enable all smaller modes. */
	while(timing->width != IGD_TIMING_TABLE_END) {
		if ((timing->width <= dtd->width) &&
			(timing->height <= dtd->height) &&
			(timing->refresh <= dtd->refresh)) {
			timing->mode_info_flags |= IGD_MODE_SUPPORTED;
		}
		timing++;

		/* If reached the first table END,
		 * then check for the added modes */
		if (timing->width == IGD_TIMING_TABLE_END &&
			timing->extn_ptr) {
			timing = timing->extn_ptr;
		}
	}
}

/*!
 * Parse an Edid Structure from a provided 128 byte buffer of data.
 * This function will return EDID_READ_AGAIN if the driver should
 * read another 128 bytes and call this function again. This will
 * happen with EDID structures with one or more extended 128 byte regions.
 * count is 0 on the first call and incremented with each extra 128
 * byte buffer.
 *
 * This parser skips information that is not important to the driver.
 * There should be an interface to provide the whole EDID to userspace
 * where more advanced parsing can happen. If you need something exotic
 * out of the EDID consider having a user app do it instead of making
 * This in-kernel EDID parser large.
 *
 * Timings found in the "Established" and "Standard" bitfields or
 * extension blocks will be looked up in the provided timings list.
 * If a match is found the timings will be marked with
 * PD_MODE_SUPPORTED.
 *
 * If the EDID provides frequency ranges all timings within the range will
 * be marked with PD_MODE_SUPPORTED in timing_table.
 *
 * If a driver uses a single timings list for multiple displays the driver
 * will have to copy the flags mentioned above into a bit range specific
 * to each display (in the driver specific bit range).
 *
 * @param buffer
 * @param edid
 * @param timing_table
 * @param count
 * @param upscale
 *
 * @return 0 on success
 * @return EDID_ERROR_PARSE on failure
 */
int edid_parse(
		unsigned char *buffer,
		edid_t        *edid,
		pd_timing_t   *timing_table,
		int           count,
		unsigned char upscale)
{
	int           chksum = 0, i, j;
	unsigned long version, vendor, temp32;
	pd_timing_t   *timing;

	if (!edid || !buffer) {
		return EDID_ERROR_PARSE;
	}

	/* No count means, edid_parse() called for first time */
	if (!count) {
		OS_MEMSET(edid, 0, sizeof(edid_t));
	}

	/* Check the checksum */
	for (i=0; i<128; i++) {
		chksum += buffer[i];
	}

	if ((chksum & 0xff) != 0) {
		EMGD_DEBUG("EDID checksum is incorrect! Will ignore.");
		return EDID_ERROR_PARSE;
	}

	/* FIXME: Verify this, I think extra EDID blocks have no header */
	if (count) {
		/*	goto BLOCKS; */
		return EDID_ERROR_PARSE;
	}

	/* Header already checked. Skip 8 bytes */
	buffer+=8;

	/* Vendor Name */
	vendor = (buffer[0]<<8) | buffer[1];
	*(unsigned long *)edid->vendor = ((vendor>>10) + 0x40) +
		((((vendor>>5) & 0x1f) + 0x40)<<8) +
		((unsigned long)((vendor & 0x1f) + 0x40)<<16);
	buffer+=2;

	/* Product Code */
	edid->product_code = (buffer[1]<<8) | buffer[0];
	buffer+=2;

	/* Serial Number 4 bytes (SKIP)*/
	edid->serial_number = buffer[0] |
				(buffer[1]<<8) |
				((unsigned long)(buffer[2])<<16) |
				((unsigned long)(buffer[3])<<24);
	buffer += 4;

	/* Manufactured Week 1 byte (SKIP) */
	edid->manf_week = buffer[0];
	buffer++;

	/* Manufactured Year 1 byte (SKIP) */
	edid->manf_year = 1990 + buffer[0];
	buffer++;

	/* EDID Version/Revision 0x12-0x13 */
	version = (buffer[0]<<8) | buffer[1];
	edid->version = buffer[0];
	edid->revision = buffer[1];
	buffer += 2;

	switch(version) {
	case 0x0:
	case 0x0101:
	case 0x0102:
	case 0x0103:
		/* Possibly do something based on this */
		break;
	default:
		EMGD_DEBUG("EDID Version %d.%d Unknown. Will Ignore.",
			edid->version, edid->revision);
		return EDID_ERROR_PARSE;
		break;
	}

	/* Basic Display Params (SKIP) */
	buffer+=4;

	/* DPMS and Features 1 byte  */
	edid->dpms = (buffer[0]>>5);
	edid->display_type = (buffer[0]>>3) & 0x3;
	edid->standard_color = (buffer[0]>>2) & 0x1;
	edid->preferred_timing = (buffer[0]>>1) & 0x1;
	edid->gtf = (buffer[0] & 0x1);
	buffer++;

	/* Color 10 bytes (SKIP)*/
	buffer+=10;

	/* EDID parsing should start by disabling all modes.
	 * Based on EDID data, modes will be enabled. */
	enable_disable_timings(timing_table, 0);

	/* Based on the established timings provided in the EDID, mark the
	 * timings in the table as below (as per meeting with MikeD on 7/11/03):
	 *
	 * 1. If all VESA ETF and 640x480 modes are supported,
	 *     then mark all modes in the table as supported + DTD modes
	 * 2. If all VESA ETF 60Hz modes are supported,
	 *     then mark all 60Hz modes in the table as supported + DTD modes
	 * 3. Default:
	 *     Enable only ETF modes in the table + DTD modes
	 *
	 * Established Timings 3 bytes (SKIP bits 6:0 of byte 3 which contains
	 * manufactorer's proprietary timings), so there are 17 established
	 * timings. */

	/* Move byte2 bit7 to byte2 bit0. This is done to maintain
	 * continuation along with other 16 established timings */
	temp32 = buffer[0] | ((unsigned long)(buffer[1])<<8) |
		((unsigned long)(buffer[2]>>7)<<16);

	/* Here are the bit definitions:
	 *  Byte 0
	 *       bit 0 - 800 x 600 @ 60Hz    VESA
	 *       bit 1 - 800 x 600 @ 56Hz    VESA
	 *       bit 2 - 640 x 480 @ 75Hz    VESA
	 *       bit 3 - 640 x 480 @ 72Hz    VESA
	 *       bit 4 - 640 x 480 @ 67Hz    Apple, Mac II
	 *       bit 5 - 640 x 480 @ 60Hz    IBM, VGA
	 *       bit 6 - 720 x 400 @ 88Hz    IBM, XGA2
	 *       bit 7 - 720 x 400 @ 70Hz    IBM, VGA
	 *  Byte 1
	 *       bit 0 - 1280 x 1024 @ 75Hz  VESA
	 *       bit 1 - 1024 x  768 @ 75Hz  VESA
	 *       bit 2 - 1024 x  768 @ 70Hz  VESA
	 *       bit 3 - 1024 x  768 @ 60Hz  VESA
	 *       bit 4 - 1024 x  768 @ 87Hz  IBM (Interlaced)
	 *       bit 5 -  832 x  624 @ 75Hz  Apple, Mac II
	 *       bit 6 -  800 x  600 @ 75Hz  VESA
	 *       bit 7 -  800 x  600 @ 72Hz  VESA
	 *  Byte 2
	 *       bit 0 - 1152 x  870 @ 75Hz  Apple, Mac II
	 *
	 * Note:
	 *  Byte2 bit 0 used to be at Byte 2 bit 7 but moved to bit 0 for sake of
	 *  continuality.
	 */
	for (i=0; i<=16; i++) {
		if (temp32 & (1L<<i)) {
			edid_mark_standard_timings(&established_table[i*6],
				timing_table, 1);
		}
	}

#ifdef DEBUG_FIRMWARE
	EMGD_DEBUG("Supported timings after FIRST established timings.");
	print_supported_timings(timing_table, edid);
#endif

	if (timing_table != NULL) {
		/* Mark the other timings in the table, as per above notes. */
		timing = timing_table;
		while (timing->width != IGD_TIMING_TABLE_END) {
			if ((temp32 & 0xCF2F) == 0xCF2F) {            /* Case 1 */
				timing->mode_info_flags |= PD_MODE_SUPPORTED;
			} else if (((temp32 & 0xCF2F) == 0x821) &&    /* Case 2 */
				(timing->refresh == 60)) {
				timing->mode_info_flags |= PD_MODE_SUPPORTED;
			}

			timing++;
			if (timing->width == IGD_TIMING_TABLE_END && timing->extn_ptr) {
				timing = timing->extn_ptr;
			}
		}
	}

	buffer+=3;

	/* Standard Timings 16 bytes, Look these up in the standard
	 * timings table to find the match. Skip any that are not found
	 * in the table.
	 */
	for (i=0; i<8; i++) {
		/* First 8 standard timings */
		edid_mark_standard_timings(buffer, timing_table, 0);
		buffer+=2;
	}

#ifdef DEBUG_FIRMWARE
	EMGD_DEBUG("Supported timings after SECOND established timings.");
	print_supported_timings(timing_table, edid);
#endif

	/* BLOCKS: */
	/* Blocks of Data */
	for (i=0; i<4; i++) {
		if (*(unsigned long *)buffer == *(unsigned long *)name_blockid) {
			/* Monitor Name */
			edid_parse_monitor_name(&buffer[5], edid->name);
			buffer+=18;
			continue;
		}
		if (*(unsigned long *)buffer == *(unsigned long *)range_blockid) {
			/* Monitor Limits */
			edid->range_set = 1;
			edid->range.min_vrate = buffer[5];      /* Hz */
			edid->range.max_vrate = buffer[6];      /* Hz */
			edid->range.min_hrate = buffer[7];      /* KHz */
			edid->range.max_hrate = buffer[8];      /* KHz */
			edid->range.max_dclk  =
				(unsigned long)buffer[9]*10000L; /* convert from MHz/10->KHz */
			/* Following are always zero:
			 * edid->range.min_dclk
			 * edid->range->min_hblank
			 * edid->range->min_vblank */

			/* Call common function to filter timings based on range limits */
			displayid_filter_range_timings(timing_table, &edid->range,
				PI_FIRMWARE_EDID);

			/* Fixme GTF */
			buffer+=18;
			continue;
		}
		if (*(unsigned long *)buffer == *(unsigned long *)st_blockid) {
			/* Additional 6 Standard Timings */
			buffer+=5;
			for (j=0; j<12; j+=2) {
				edid_mark_standard_timings(&buffer[j], timing_table, 0);
			}
			buffer+=13;
			continue;
		}
		if (*(unsigned long *)buffer & *(unsigned long *)timings_mask) {
			/* Detailed Timings */
			if (edid->num_timings >= NUM_TIMINGS-1) {
				continue;
			}
			edid_mark_detailed_timings(buffer, edid, timing_table, upscale);
			buffer+=18;
			continue;
		}
		/* Don't handle Block types that don't matter */
		buffer+=18;
	} /* end for loop */

#ifdef DEBUG_FIRMWARE
	EMGD_DEBUG("FINAL supported timings .");
	print_supported_timings(timing_table, edid);
#endif

	/* Extensions */
	if (*buffer) {
		return EDID_READ_AGAIN;
	}

	return 0;
} /* end edid_parse() */

/*!
 * Given the two byte Standard Timings Identifier, Mark the timing
 * that matches in the timings array.
 * If "established" is set, the two byte "EDID id" will be used,
 * otherwise the buffer should contain the hactive, followed by
 * the vactive followed by the refresh, all in 16 bit LSB format.
 *
 * @param buffer
 * @param timings
 * @param established
 *
 * @return void
 */
static void edid_mark_standard_timings(
		unsigned char *buffer,
		pd_timing_t   *timings,
		int           established)
{
	int hactive, vactive=0, refresh, aspect;

	if ((!established) && (buffer[0] == 0x01) && (buffer[1] == 0x01)) {
		return;
	}
	if (established) {
		hactive = buffer[0] | (buffer[1]<<8);
		vactive = buffer[2] | (buffer[3]<<8);
		refresh = buffer[4] | (buffer[5]<<8);
	}
	else {
		hactive = (buffer[0] + 31)<<3;
		refresh = (buffer[1] & 0x1f) + 60;
		aspect = buffer[1]>>6;
		switch(aspect) {
		case 0x0:
			vactive = (hactive*10)>>4;
			break;
		case 0x1:
			vactive = (hactive*3)>>2;
			break;
		case 0x2:
			vactive = (hactive<<2)/5;
			break;
		case 0x3:
			vactive = (hactive*9)>>4;
			break;
		default:
			EMGD_ERROR("Invalid aspect ratio in EDID.");
			return;
		}
	}

	if (timings != NULL) {
		/* Look for mode in table */
		while (timings->width != IGD_TIMING_TABLE_END) {
			/* By default no mode is supported. */
			if ((timings->refresh == refresh) &&
				(timings->width == hactive) &&
				(timings->height == vactive)) {
				timings->mode_info_flags |= PD_MODE_SUPPORTED;
				return;
			}

			timings++;

			if (timings->width == IGD_TIMING_TABLE_END && timings->extn_ptr) {
				timings = timings->extn_ptr;
			}
		}
	}

	return;
} /* end edid_mark_standard_timings() */

/*!
 * Parse a detailed timing block from the buffer provided and set the
 * data in the pd_timing_t structure provided.
 *
 * @param buffer
 * @param edid
 * @param timing_table
 * @param upscale
 *
 * @return void
 */
static void edid_mark_detailed_timings(
		unsigned char *buffer,
		edid_t        *edid,
		pd_timing_t   *timing_table,
		unsigned char upscale)
{
	unsigned long temp;
	pd_timing_t *timings = &(edid->timings[edid->num_timings]);

	timings->dclk = ((unsigned long)(buffer[1]<<8) | buffer[0]) * 10;  /* KHz */
	timings->width = ((buffer[4] & 0xf0)<<4) | buffer[2];
	if (timings->width == 0) {
		/* Bail out as DTD is invalid. This happens as we are estimating the
		 * amount of availabel DTD since manufacturer does not follow CEA
		 * format */
		timings->width = IGD_TIMING_TABLE_END;
		return;
	}
	timings->hblank_start = timings->width - 1;
	timings->hblank_end = timings->width +
		(((buffer[4]& 0xf)<<8) | buffer[3]) - 1;
	timings->height = ((buffer[7] & 0xf0)<<4) | buffer[5];
	timings->vblank_start = timings->height - 1;
	timings->vblank_end = timings->height +
		(((buffer[7]& 0xf)<<8)| buffer[6]) - 1;
	timings->hsync_start = timings->hblank_start +
		(((buffer[11] & 0xc0)<<2) | buffer[8]);
	timings->hsync_end = timings->hsync_start +
		(((buffer[11] & 0x30)<<4) | buffer[9]);
	timings->vsync_start = timings->vblank_start +
		(((buffer[11] & 0xc)<<2) | (buffer[10]>>4));
	timings->vsync_end = timings->vsync_start +
		(((buffer[11] & 0x3)<<4) | (buffer[10] & 0xf));
	timings->htotal = timings->hblank_end;
	timings->vtotal = timings->vblank_end;

	/* SKIP BORDER */
	timings->mode_info_flags |= (buffer[17] & 0x80)?PD_SCAN_INTERLACE:0;

	/* Need to divide width by 2 and set PIXEL_DOUBLE flag in order to properly */
	/* handle clock doubled modes like 1440(720)x480i and 1440(720)x576i */
	if( (timings->mode_info_flags & PD_SCAN_INTERLACE) && (timings->width == 1440) &&
	    ((timings->height == 480) || (timings->height == 576)) )
	{
		timings->width /= 2;
		timings->mode_info_flags |= IGD_PIXEL_DOUBLE;
	}else if((timings->mode_info_flags & PD_SCAN_INTERLACE)){
		/* The height needs to be multiplied by 2 here so OS would prepare the
		   correct frame buffer size. Pipe programming needs to be aware of this */
		timings->height			*= 2;
		timings->vtotal			*= 2;
		timings->vblank_start	*= 2;
		timings->vblank_end		*= 2;
		timings->vsync_start	*= 2;
		timings->vsync_end		*= 2;
	}

	/* Set sync polarities */
	if((buffer[17] & 0x18)==0x18) { /* Bit 4 and 3 above = 11*/
		/* if Bit 2 = 1, then vsync polarity = positive */
		timings->mode_info_flags |= (buffer[17] & 0x04)? IGD_VSYNC_HIGH: 0;
		/* if Bit 1 = 1, then hsync polarity = positive */
		timings->mode_info_flags |= (buffer[17] & 0x02)? IGD_HSYNC_HIGH: 0;
	}

	if((buffer[17] & 0x18)==0x10) { /* Bit 4 and 3 above = 10*/
		/* if Bit 1 = 1, then hsync polarity = positive */
		timings->mode_info_flags |= (buffer[17] & 0x02)? IGD_HSYNC_HIGH: 0;
	}

	if (timings->dclk == 0) {
		timings->dclk = 1;
	}
	if (timings->htotal == 0) {
		timings->htotal = 1;
	}
	if (timings->vtotal == 0) {
		timings->vtotal = 1;
	}

	temp = (unsigned long)timings->htotal * (unsigned long)timings->vtotal;
	timings->refresh = (unsigned short)((timings->dclk * 1000)/temp);
	timings->mode_info_flags |= (PD_MODE_SUPPORTED | PD_MODE_DTD);

	/* Save in the detailed timings of EDID */
	edid->num_timings++;
	/* Mark the end of the list */
	edid->timings[edid->num_timings].width = IGD_TIMING_TABLE_END;

	/* Enable scale timings */
	enable_scaled_timings(timing_table, timings, upscale);
} /* end edid_mark_detailed_timings() */

/*!
 * Get the 13 (or less) character Monitor name from the buffer and
 * pad with nulls.
 *
 * @param buffer
 * @param name
 *
 * @return void
 */
static void edid_parse_monitor_name(unsigned char *buffer, char *name)
{
	int i;
	for (i=0; i<13; i++) {
		if (buffer[i] == 0x0a) {
			name[i] = 0;
			break;
		}
		name[i] = buffer[i];
	}
	while (i<14) {
		name[i] = 0;
		i++;
	}
} /* edid_parse_monitor_name() */

#ifndef CONFIG_MICRO
/*!
 *
 * @param edid
 *
 * @return 0
 */
int parse_audio_block(edid_t *edid)
{
	int i;
	cea_audio_format_t *tmp_audio;

	for(i=0; i<(edid->cea->total_short_audio_desc); i++){
		tmp_audio = edid->cea->short_audio_desc + i;
		if(tmp_audio->audio_format_code == CEA_AUDIO_LPCM){
			if(tmp_audio->_48khz){
				edid->cea->audio_cap[0].max_channels = tmp_audio->max_channels;
				edid->cea->audio_cap[0]._20bit = tmp_audio->_20bit;
				edid->cea->audio_cap[0]._24bit = tmp_audio->_24bit;
			}
			if(tmp_audio->_96khz){
				edid->cea->audio_cap[1].max_channels = tmp_audio->max_channels;
				edid->cea->audio_cap[1]._20bit = tmp_audio->_20bit;
				edid->cea->audio_cap[1]._24bit = tmp_audio->_24bit;
				if(tmp_audio->_88khz){
					edid->cea->_44ms = 1;
				}
			}
			if(tmp_audio->_192khz){
				edid->cea->audio_cap[2].max_channels = tmp_audio->max_channels;
				edid->cea->audio_cap[2]._20bit = tmp_audio->_20bit;
				edid->cea->audio_cap[2]._24bit = tmp_audio->_24bit;
				if(tmp_audio->_176khz){
					edid->cea->_44ms = 1;
				}
			}
		}
	}
	return 0;
}

/*!
 *
 * @param buffer
 * @param list
 * @param block_size
 * @param total_bytes
 *
 * @return 0 no read will happen
 * @return total_blocks
 */
int read_data_block_collection(unsigned char *buffer,
							   char **list,
							   unsigned int block_size,/* Block size in byte */
							   unsigned int total_bytes)
{
	unsigned int total_blocks;
	unsigned int i;

	total_blocks = total_bytes/block_size;
	if(total_bytes <= 0 ){
		/* No read will happen */
		return 0;
	}

	if(*list !=NULL){
		OS_FREE(*list);
	}

	*list = OS_ALLOC((sizeof(char)*total_bytes));
	for(i=0;i<total_blocks;i++){
		OS_MEMCPY(*list+i, buffer, block_size);
		buffer+=block_size;
	}

	return total_blocks;
}

/*!
 *
 * @param buffer
 * @param edid
 * @param timings
 * @param count
 * @param upscale
 *
 * @return 0 on success
 * @return EDID_ERROR_PARSE on failure
 */
int edid_parse_cea (unsigned char *buffer,
					edid_t        *edid,
					pd_timing_t   *timings,
					int           count,
					unsigned char upscale)
{
	unsigned int offset,i = 4,timing_in_edid = 0, j;
	unsigned int tag_code, chksum=0;
	unsigned char *data_block_buffer;
	pd_timing_t     *cea_timings = NULL;
	pd_timing_t		*temp_timings;
	unsigned short header;

	if (!buffer) {
		return EDID_ERROR_PARSE;
	}
	/* This holds the buffer */
	offset = buffer[2];
	/* Check the checksum */
	for( i=0; i<128; i++) {
		chksum += buffer[i];
	}
	if ((chksum & 0xff) != 0) {
		EMGD_ERROR("EDID checksum is incorrect! Will ignore.");
		return EDID_ERROR_PARSE;
	}

	header = buffer[0];

	/* DI-Extension(header == 0x40) Unsupported for now */
	if(header != 0x02){	/* CEA-Extension */
		EMGD_ERROR("Unknown Extension");
		return EDID_ERROR_PARSE;
	}

	if(edid->cea == NULL){
		edid->cea = (cea_extension_t *) OS_ALLOC(sizeof(cea_extension_t));
	}
	if(edid->cea == NULL){
		EMGD_ERROR("Run out of memory");
		return 1;
	}
	OS_MEMSET(edid->cea, 0 , (sizeof(cea_extension_t)));
	edid->cea->rev_number	= buffer [1];
	edid->cea->caps			= buffer [3];

	// no data block found or data block exceeds maximum buffer size
	if(offset == 0 || offset > 124) {
		EMGD_ERROR("No data block and no DTD ");
		return 1;
	}

	/* General Format of CEA Data Block Collection
	// -----------+--------------------+--------------------------------------+
	//            |Byte#   |Bits5-7    |       Bits 0-4                       |
	// -----------|--------------------+--------------------------------------+
	//            |  1     | Video Tag |Length = total #of video			  |
	//            |        |    Code   |bytes following this byte (L1)        |
	//            |--------------------+--------------------------------------+
	//  Video     |  2     | CEA Short Video Descriptor 1                     |
	//  Data      |--------+--------------------------------------------------|
	//  Block     |  3     | CEA Short Video Descriptor 2                     |
	//            |--------+--------------------------------------------------|
	//            | ...    | ...                                              |
	//            |-----------------------------------------------------------+
	//            | 1+L1   | CEA Short Video Descriptor L1                    |
	// -----------+--------------------+--------------------------------------+
	//            | 2+L1   | Audio Tag |Length = total #of audio bytes		  |
	//            |        |    Code   |following this byte (L2)              |
	//            |--------------------+--------------------------------------+
	//  Audio     | 3+L1   |                                                  |
	//  Data      |--------+                                                  |
	//  Block     | 4+L1   | CEA Short Audio Descriptor 1                     |
	//            |--------+                                                  |
	//            | 5+L1   |                                                  |
	//            |-----------------------------------------------------------+
	//            | ...    |                                                  |
	//            |        |                                                  |
	//            |        |                                                  |
	//            | ...    |                                                  |
	//            |------------------------------------------------------------
	//            |L1+L2   |                                                  |
	//            |--------|                                                  |
	//            |1+L1+L2 | CEA Short Audio Descriptor L2/3                  |
	//            |--------|                                                  |
	//            |2+L1+L2 |                                                  |
	// -----------+-----------------------------------------------------------+
	//            |3+L1+L2 |  Speaker  |Length = total #of SA bytes following |
	//            |        | Tag Code  |this byte (L1)                        |
	//  Speaker   |-----------------------------------------------------------+
	//  Allocation|4+L1+L2 |                                                  |
	//  Data      |--------|                                                  |
	//  Block     |5+L1+L2 | Speaker Allocation Data Block Payload(3 bytes)   |
	//            |--------|                                                  |
	//            |6+L1+L2 |                                                  |
	// -----------+-----------------------------------------------------------+
	//            |7+L1+L2 | VSDB  Tag |Length = total #of VSDB bytes		  |
	//            |        |    Code   |following this byte (L1)              |
	//  Vendor    |-----------------------------------------------------------+
	//  Specific  |8+L1+L2 |                                                  |
	//  Data      |--------|                                                  |
	//  Block     |9+L1+L2 | 24-bit IEEE Registration Identifier (LSB first)  |
	//            |--------|                                                  |
	//            |10+L1+L2|                                                  |
	//            |-----------------------------------------------------------+
	//            | ...    | Vendor Specific Data block Payload               |
	// -----------+-----------------------------------------------------------+*/
	data_block_buffer=&buffer[4];
	/* Start reading data block collection */
	i = 0;
	while(i<offset){
		unsigned int total_bytes;
		tag_code = (unsigned int)*data_block_buffer & 0xE0;
		total_bytes = (unsigned int)*data_block_buffer & 0x1F;
		data_block_buffer++;
		i++;
		switch(tag_code){
			case CEA_VIDEO_DATA_BLOCK:
				/* Reading Short Video Descriptor block */
				edid->cea->total_short_video_desc =read_data_block_collection
				   (data_block_buffer,
					(char **)&edid->cea->short_video_desc,
					1,/* Block size in byte */
					total_bytes);
				break;
			case CEA_AUDIO_DATA_BLOCK:
				/* Reading Short Audio Descriptor block */
				edid->cea->total_short_audio_desc = read_data_block_collection
					(data_block_buffer,
					(char **)&edid->cea->short_audio_desc,
					3,/* Block size in byte */
					total_bytes);
				/* uncomment this codes if this info is requred in the future */
				/*for(j=0; j<total_bytes; j+=3){
					OS_MEMCPY
						(&edid->cea->misc_data[j],
						(edid->cea->short_audio_desc + (j/3)),
						sizeof(cea_audio_format_t));
				}
				edid->cea->sadc = (unsigned char)
					edid->cea->total_short_audio_desc; */
				break;
			case CEA_VENDOR_DATA_BLOCK:
				/* Reading Vendor Specific Descriptor block */
				edid->cea->vendor_block.vendor_block_size = (unsigned char)total_bytes;
				OS_MEMCPY(edid->cea->vendor_block.vendor_ieee_id, data_block_buffer, 3);
				read_data_block_collection
					(data_block_buffer,
					(char **)&edid->cea->vendor_data_block,
					3,
					total_bytes);
				break;
			case CEA_SPEAKER_DATA_BLOCK:
				/* Reading Speaker Data Descriptor block */
				for(j=0; j<3; j++){
					edid->cea->speaker_alloc_block[j] = data_block_buffer[j];
				}
				break;
			default:
				/* Ignore unknown block? */
				EMGD_ERROR("Unknown Tag Code! : CEA extended data block");
				break;
		}
		data_block_buffer+=(total_bytes);
		i+=(total_bytes);
	};

	/* DPG codes show that they are manufacturer who doesn't follow the spec
	   Calculate the maximum possible DTD from whatever empty space left.
	   The Maximum DTD is 6*/
	 i=(unsigned int)(128-offset-1)/18;
	 i = (i>6)?6:i;

	buffer+=offset;
	while(i>0){
		if (edid->num_timings >= NUM_TIMINGS-1) {
				break;
		}
		edid_mark_detailed_timings(buffer, edid, timings, upscale);
		buffer+=18;
		i--;
	};

	/* Parse audio block to know panel audio capability */
	parse_audio_block(edid);

	cea_timings = (igd_timing_info_t *) OS_ALLOC(cea_timing_table_size);
	OS_MEMCPY(cea_timings, cea_timing_table, cea_timing_table_size);

	for(i=0; i<(unsigned int)edid->cea->total_short_video_desc; i++){
		EMGD_DEBUG("Video Code %d",(int)(edid->cea->short_video_desc + i)->code);
		temp_timings = cea_timings;
		timing_in_edid = 0;
		while (temp_timings->width != IGD_TIMING_TABLE_END){
			/* Look for matching tag code */
			if((edid->cea->short_video_desc + i)->code == temp_timings->mode_number){
				j=0;
				while(j<edid->num_timings){
					/* Ignore if we have already read the timng through DTD
					   dclk would be sufficient to differentiate between progressive
					   and interlace */
					if(temp_timings->width == edid->timings[j].width &&
					   temp_timings->height == edid->timings[j].height &&
					   temp_timings->refresh == edid->timings[j].refresh &&
					   temp_timings->dclk == edid->timings[j].dclk){
						timing_in_edid = 1;
						break;
					}
					j++;
				}
				/* Add the timing if we have not done so through DTD */
				if(!(timing_in_edid)){
					if (edid->num_timings >= NUM_TIMINGS-1) {
							break;
					}
					OS_MEMCPY(&edid->timings[edid->num_timings],
							  temp_timings, sizeof(pd_timing_t));
					edid->timings[edid->num_timings].mode_number = 0;
					edid->timings[edid->num_timings].mode_info_flags
						|= (PD_MODE_SUPPORTED | PD_MODE_DTD);
					edid->num_timings++;
					edid->timings[edid->num_timings].width = IGD_TIMING_TABLE_END;
					/*enable_scaled_timings(timings,
						&edid->timings[edid->num_timings], upscale);*/
				}
				break;
			}
			temp_timings++;
		}
	}

	OS_FREE(cea_timings);

#ifdef DEBUG_FIRMWARE
	EMGD_ERROR("FINAL supported timings after CEA .");
	print_supported_timings(timings, edid);
#endif
	return 0;
}
#endif

/*!
 *
 * @param buffer
 * @param edid
 * @param timings
 * @param count
 * @param upscale
 *
 * @return 0 on success
 * @return 1 on failure
 */
int edid_ext_parse(
		unsigned char *buffer,
		edid_t        *edid,
		pd_timing_t   *timings,
		int           count,
		unsigned char upscale)
{
	unsigned int timing_in_edid = 0, j;
#ifndef CONFIG_MICRO
	if(edid_parse_cea(buffer, edid, timings, count, upscale)){
		EMGD_ERROR("Reading cea extension ERROR!");
		return 1;
	}
#endif

	/* Check to see if 640x480 is part of EDID timing if not add it in.
	   CEA panels does not support standard timing 640x480 */
	for(j=0; j<edid->num_timings; j++){
		if(edid->timings[j].width == 640 &&
		   edid->timings[j].height == 480 &&
		   edid->timings[j].refresh == 60 ){
			timing_in_edid = 1;
			break;
		}
	}
	if((!timing_in_edid) && (edid->num_timings < NUM_TIMINGS-1)){
		OS_MEMCPY(&edid->timings[edid->num_timings],
							  cea_timing_table, sizeof(pd_timing_t));
		edid->timings[edid->num_timings].mode_number = 0x101;
		edid->timings[edid->num_timings].mode_info_flags
			|= (PD_MODE_SUPPORTED | PD_MODE_DTD);
		edid->num_timings++;
		edid->timings[edid->num_timings].width = IGD_TIMING_TABLE_END;
	}

	return 0;
}/* end edid_parse_ext() */


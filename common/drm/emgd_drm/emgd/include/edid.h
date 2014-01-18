/*
 *-----------------------------------------------------------------------------
 * Filename: edid.h
 * $Revision: 1.7 $
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
 *  Header file for EDID.
 *  Supported EDID versions:
 *  EDID 1.3 (backward compatible with 1.1, 1.2)
 *-----------------------------------------------------------------------------
 */

/*
 * This is a basic EDID parser. It only parses the parts of the EDID that
 * are useful for developing timings and identifying the monitor. Items
 * like monitor serial numbers, chromaticity settings etc. are not useful
 * in the display drivers. There should be a simple interface to provide
 * the complete unparsed EDID to IAL, so that such information can be
 * used by a IAL driver if need be.
 *
 * timings_t: This is a structure that contains one set of display timings.
 *  It is complete in that it contains all 12 timings fields instead of
 *  the popular 8. Some of the "Established" EDID timings require all 12
 *  fields. It also allows for both established flags and driver specific
 *  flags to make it flexible enough to work for most drivers. The
 *  EDID parser will mark the flags with FB_TIMINGS_PREFERRED if they are
 *  found to be listed in the EDID. This does not include modes within
 *  the the sync ranges (if provided), only "Established" "Standard" and
 *  "Detailed" timings.
 *
 * edid_t: This structure will be populated by the EDID parser. There
 *  are three important areas of information that impact display timings.
 *  The h_max,h_min,v_max,v_min represent the range of timings that the
 *  display is capable of accepting. No timings outside this range should
 *  be used by the driver if a range is provided.
 *
 *  Any "Detailed" timings will be fully parsed and returned in
 *  the timings array (only the first 12 are parsed, it is very unlikely
 *  that any EDID exist that contain this many).
 *
 *  Suggested driver behavior when setting display timings would be to
 *  first attempt to use "Detailed" timings provided by the monitor. Then
 *  attempt to use the "Standard" timings that were marked as PREFERRED,
 *  then as a last alternative, use a nonstandard set of timings within
 *  the provided range.
 *
 */

#ifndef _EDID_H
#define _EDID_H

#include <displayid.h>
#include <pd.h>

/* This structure holds all of the parsed EDID information.*/
#define NUM_TIMINGS 12

/* EDID return values */
#define EDID_ERROR_PARSE  1
#define EDID_READ_AGAIN   2

typedef struct _established_timing {
	unsigned long width;
	unsigned long height;
	unsigned long refresh;
} established_timing_t;

typedef struct _edid {
	unsigned char     version;            /* Edid Version */
	unsigned char     revision;           /* Edid Revision */
	char              vendor[4];          /* Vendor Name code */
	unsigned long     product_code;       /* Vendor assigned code */
	unsigned long     serial_number;      /* 32-bit serial number */
	unsigned char     manf_week;          /* Manufactored week number */
	unsigned long     manf_year;          /* Manufactored year */
	unsigned char     standard_color;
	unsigned char     preferred_timing;   /* Use first timing Provided */
	unsigned char     dpms;
	unsigned char     display_type;
	unsigned char     gtf;
	unsigned char     range_set;          /* EDID Contains Valid Range Data */
	timing_range_t    range;
	unsigned char     num_timings;
	pd_timing_t       timings[NUM_TIMINGS];
	char              name[14];
	cea_extension_t	  *cea;				  /* CEA extension based on 861-B */
} edid_t;

/* Functions */
int edid_parse(
		unsigned char *buffer,
		edid_t        *edid,
		pd_timing_t   *timings,
		int           count,
		unsigned char upscale);

void enable_disable_timings(pd_timing_t *timing, unsigned char enable);
void enable_scaled_timings(pd_timing_t *timing, pd_timing_t *dtd,
	unsigned char upscale);
void firmware_dump(unsigned char *buffer, unsigned short size);
void edid_print(edid_t *edid);

int edid_ext_parse(
		unsigned char *buffer,
		edid_t        *edid,
		pd_timing_t   *timings,
		int           count,
		unsigned char upscale);
#endif

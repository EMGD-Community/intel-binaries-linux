/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_hdmi.h
 * $Revision: 1.6 $
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
 *  Main include file for SDVO port driver HDMI interface function
 *-----------------------------------------------------------------------------
 */

#ifndef _SDVO_PORT_HDMI_H_
#define _SDVO_PORT_HDMI_H_

#include <config.h>
#include <igd_pd.h>
#include <pd.h>
#include <pd_print.h>
#include "sdvo_port.h"
#include "sdvo_intf.h"

/* VBios abstraction macro */
#if !defined(CONFIG_MICRO)
#define SDVO_HDMI
#endif


#if defined(SDVO_HDMI)

#define SDVO_HDMI_ELD_BUFFER_SIZE			256
#define SDVO_HDMI_AVI_BUFFER_SIZE			17
#define SDVO_HDMI_SPD_BUFFER_SIZE			29
#define	SDVO_HDMI_ELD_BUFFER				0			/* ELD buffer index */
#define SDVO_HDMI_AVI_BUFFER				1			/* AVI buffer index */
#define SDVO_HDMI_SPD_BUFFER				2			/* SPD buffer index */
#define SDVO_HDMI_ELD_INFO_TYPE				0x00		/* ELD buffer type */
#define SDVO_HDMI_AVI_INFO_TYPE				0x82		/* AVI buffer type */
#define SDVO_HDMI_SPD_INFO_TYPE				0x83		/* SPD buffer type */
#define SDVO_HDMI_AVI_BUFFER_SIZE			17
#define SDVO_HDMI_TRANSMIT_DISABLE			0x00
#define SDVO_HDMI_TRANSMIT_ONCE				0x80
#define SDVO_HDMI_TRANSMIT_EVERY			0xC0

#define SDVO_HDMI_VENDOR_NAME				"Intel"
#define SDVO_HDMI_VENDOR_DESCRIPTION		"IEGD Driver"
#define SDVO_HDMI_INTEL_VENDOR_NAME_SIZE		6
#define SDVO_HDMI_IEGD_VENDOR_DESCRIPTION_SIZE	12

#define SDVO_HDMI_VENDOR_NAME_SIZE			8
#define SDVO_HDMI_VENDOR_DESCRIPTION_SIZE	16
#define SDVO_HDMI_SPD_SOURCE_PC				0x09

#define DVI_MODE	0
#define HDMI_MODE	1
sdvo_status_t sdvo_hdmi_transmitter(sdvo_device_context_t *p_context);
sdvo_status_t sdvo_hdmi_support(void *p_context);
sdvo_status_t sdvo_hdmi_audio_characteristic(sdvo_device_context_t *p_context);
sdvo_status_t sdvo_hdmi_configure(sdvo_device_context_t *p_ctx);
sdvo_status_t sdvo_hdmi_pr_and_color(sdvo_device_context_t *p_context);
sdvo_status_t sdvo_hdmi_send_eld(sdvo_device_context_t *p_ctx);
sdvo_status_t sdvo_hdmi_avi_info_frame(sdvo_device_context_t *p_context);
sdvo_status_t sdvo_hdmi_spd_info_frame(sdvo_device_context_t *p_context);
sdvo_status_t sdvo_hdmi_write_buffer
	(sdvo_device_context_t *p_ctx,unsigned char index,
	unsigned char offset, unsigned char *input, int len);

typedef union
{
	unsigned char value;
	struct {
		unsigned char eld_valid: 1;
		unsigned char presense_detect: 1;
		unsigned char cp_ready: 1;
		unsigned char reserved: 5;
	};
} sdvo_audio_state_t;
/* AVI Info Frames version 2 */
typedef struct
{
	unsigned char type;
	unsigned char version;
	unsigned char length;
	unsigned char chksum;
} sdvo_info_header_t;

typedef union
{
	unsigned char data[SDVO_HDMI_AVI_BUFFER_SIZE];
	struct {
		sdvo_info_header_t	header;
		union {
			unsigned char data1;
			struct {
				unsigned char scan_info: 2;
				unsigned char bar_info: 2;
				unsigned char active_format: 1;
				unsigned char color_space: 2;
				unsigned char reserved1: 1;
			};
		};
		union {
			unsigned char data2;
			struct {
				unsigned char format_aspect_ratio: 4;
				unsigned char pic_aspect_ratio: 2;
				unsigned char colorimetry: 2;
			};
		};
		union {
			unsigned char data3;
			struct {
				unsigned char pic_scaling: 2;
				unsigned char reserved2: 6;
			};
		};
		union {
			unsigned char data4;
			struct {
				unsigned char video_id_code: 7;
				unsigned char reserved3: 1;
			};
		};
		union {
			unsigned char data5;
			struct {
				unsigned char pixel_rep: 4;
				unsigned char reserved4: 4;
			};
		};
		unsigned char bar1;
		unsigned char bar2;
		unsigned char bar3;
		unsigned char bar4;
		unsigned char bar5;
		unsigned char bar6;
		unsigned char bar7;
		unsigned char bar8;
	};
} sdvo_avi_info_t;

typedef union
{
	unsigned char data[SDVO_HDMI_SPD_BUFFER_SIZE];
	struct {
		sdvo_info_header_t	header;
		unsigned char vendor_name[SDVO_HDMI_VENDOR_NAME_SIZE];
		unsigned char product_desc[SDVO_HDMI_VENDOR_DESCRIPTION_SIZE];
		unsigned char source_device;
	};
} sdvo_spd_info_t;


#define SDVO_INIT_HDMI(a)			sdvo_hdmi_audio_characteristic(a)
#define SDVO_IS_HDMI_SUPPORTED(a)	sdvo_hdmi_support(a)
#define SDVO_CONFIGURE_HDMI(a)		sdvo_hdmi_configure(a)
#else
#define SDVO_INIT_HDMI(a)
#define SDVO_IS_HDMI_SUPPORTED(a)		SS_NOT_SUPPORTED
#define SDVO_CONFIGURE_HDMI(a)
#endif  /* SDVO_HDMI */

#endif  /*  _SDVO_PORT_HDMI_H_ */

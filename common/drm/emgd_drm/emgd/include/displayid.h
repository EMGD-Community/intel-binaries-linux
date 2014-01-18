/*
 *-----------------------------------------------------------------------------
 * Filename: displayid.h
 * $Revision: 1.10 $
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
 *  Header file for DisplayID.
 *  Supported DisplayID versions:
 *  VESA DisplayID Standard Verion 1 12/13/2007
 *-----------------------------------------------------------------------------
 */

#ifndef _DISPLAYID_H
#define _DISPLAYID_H

#include <pd.h>

/* Max limits for DisplayID structure members */
#define DISPLAYID_MAX_RANGES        5
#define DISPLAYID_MAX_NUM_TIMINGS   5
/* Possible attributes from DisplayID:
 *     T1, T2, T3, T4, T5
 *     dual_channel
 *     panel_depth
 *     panel_type */
#define DISPLAYID_MAX_ATTRS        10

/* DisplayID return values */
#define DISPLAYID_ERROR_PARSE      1
#define DISPLAYID_READ_AGAIN       2
#define DISPLAYID_NOT_SUPPORTED    3

/* Print line macro */
#define DISPLAYID_PRINT_LINE() \
	EMGD_DEBUG("----------------------------------------------------------")

/* DisplayID Product Type Identifiers */
#define PRODUCTID_EXTENSION        0x00
#define PRODUCTID_TEST             0x01
#define PRODUCTID_PANEL            0x02
#define PRODUCTID_STANDALONE       0x03
#define PRODUCTID_TV_RECEIVER      0x04
#define PRODUCTID_REPEATER         0x05

/* DisplayID Data Block Tags */
#define DATABLOCK_PRODUCTID        0x00
#define DATABLOCK_DISPLAY_PARAMS   0x01
#define DATABLOCK_COLOR_CHARS      0x02
#define DATABLOCK_TIMING_1_DETAIL  0x03
#define DATABLOCK_TIMING_2_DETAIL  0x04
#define DATABLOCK_TIMING_3_SHORT   0x05
#define DATABLOCK_TIMING_4_DMTID   0x06
#define DATABLOCK_VESA_TIMING_STD  0x07
#define DATABLOCK_CEA_TIMING_STD   0x08
#define DATABLOCK_VIDEO_RANGE      0x09
#define DATABLOCK_SERIAL_NUMBER    0x0A
#define DATABLOCK_ASCII_STRING     0x0B
#define DATABLOCK_DISPLAY_DEVICE   0x0C
#define DATABLOCK_LVDS_INTERFACE   0x0D
#define DATABLOCK_TRANSFER_CHAR    0x0E
#define DATABLOCK_DISPLAY_INTF     0x0F
#define DATABLOCK_STEREO_INTF      0x10
#define DATABLOCK_VENDOR_SPECIFIC  0x7F
#define DATABLOCK_RESERVED         0x80

/*
 * typedef struct __attribute__((packed)) {
 * } foo_t;
 * This isn't available with Microsoft C compiler
 * So use pragma to enable bytewise packing and at the end
 * put it back to default.
 */
#pragma pack(1)
/* General Data Block header - Max 251 bytes */
typedef struct _datablock {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;
	unsigned char  payload_data[248];
} datablock_t;

/* Video Timing Range Limits Data Block.
 * Note:
 *   It is used to convey to a source the supported range of vertical and
 *   horizontal frequencies along with the maximum supported pixel clock.
 *   For a continuous frequency device any timing that lies within the
 *   specified range will ensure a displayable image. The picture may not
 *   be properly sized or centered, as guaranteed with explicitly reported
 *   timing, but however the display should synchronize to the timing.
 */

typedef struct _timing_range {
	unsigned char  tag;                 /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	struct {
		unsigned short lsb_min_dclk;
		unsigned char  msb_min_dclk;
	} mindclk;
	struct {
		unsigned short lsb_max_dclk;
		unsigned char  msb_max_dclk;
	} maxdclk;
	unsigned char  min_hrate;           /* KHz */
	unsigned char  max_hrate;           /* KHz */
	unsigned short min_hblank;          /* Pixels */
	unsigned char  min_vrate;           /* Hz */
	unsigned char  max_vrate;           /* Hz */
	unsigned short min_vblank;          /* Lines */

	/* Timing support flags */
	unsigned char  reserved1:4;
	unsigned char  discrete_display:1;
	unsigned char  vesa_cvt_rb:1;
	unsigned char  vesa_cvt_std:1;
	unsigned char  interlaced:1;

	/* Because there is no way to define a 3-byte quantity it is not possible
	 * to directly read the dclks into member variables automatically.
	 * Therefore 1st read into above dclk bytes and then copy values into
	 * below fields at the time of parsing. */
	/* Below members are outside Data Block definition */
	unsigned long  min_dclk;            /* KHz */
	unsigned long  max_dclk;            /* KHz */
} timing_range_t;

/* Data Block Display params - Video input id */
#define VIDEO_INPUT_LVDS    0x01
#define VIDEO_INPUT_TMDS    0x02
#define VIDEO_INPUT_RSDS    0x03
#define VIDEO_INPUT_DVID    0x04
#define VIDEO_INPUT_DVII    0x05
#define VIDEO_INPUT_VESA_M1 0x06
#define VIDEO_INPUT_HDMI_A  0x07
#define VIDEO_INPUT_HDMI_B  0x08
#define VIDEO_INPUT_MDDI    0x09
#define VIDEO_INPUT_PORT    0x0A
#define VIDEO_INPUT_1394    0x0B
#define VIDEO_INPUT_USB     0x0C

/* Aspect Ratios */
#define TIMING_AR_1_1       0x0    /* 3-bits */
#define TIMING_AR_5_4       0x1
#define TIMING_AR_4_3       0x2
#define TIMING_AR_15_9      0x3
#define TIMING_AR_16_9      0x4
#define TIMING_AR_16_10     0x5

/* Data Block Display params - feature support flags */
typedef struct _display_params {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned short horz_image_size;    /* horizontal image size in 0.1 mm */
	unsigned short vert_image_size;    /* vertical image size in 0.1 mm */
	unsigned short horz_pixels;        /* horizontal pixels */
	unsigned short vert_pixels;        /* vertical pixels */

	/* Display feature support flags */
	unsigned char  deinterlacing:1;    /* de-interlacing capable display */
	unsigned char  reserved1:1;
	unsigned char  fixed_res:1;        /* fixed resolution display */
	unsigned char  fixed_timing:1;     /* fixed timing display */
	unsigned char  dpm_capable:1;      /* VESA DPM capable display */
	unsigned char  audio_override:1;   /* override audio on video interface */
	unsigned char  seperate_audio:1;   /* seperate audio input is available */
	unsigned char  audio_on_video:1;   /* audio available on video interface */

	unsigned char  transfer_gamma;     /* transfer characteristic gamma */
	unsigned char  aspect_ratio;       /* aspect ratio */
	unsigned char  native_color_depth:4; /* color bit depth */
	unsigned char  overall_color_depth:4;/* color bit depth */
} display_params_t;


/* Product Identification Data Block */
typedef struct _productid {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	char           vendor[3];          /* vendor name code, no NULL char */
	unsigned short product_code;       /* product code */
	unsigned long  serial_number;      /* 32-bit serial number */
	unsigned char  manf_week;          /* manufactured week number */
	unsigned char  manf_year;          /* # of years after 2000 */
	unsigned char  string_size;        /* product id string size */
	char           string[237];        /* product id string max 236 + NULL */
} productid_t;

/* Color Characteristices Data Block */
typedef struct _color_char {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  white_points:4;
	unsigned char  primaries:3;
	unsigned char  temporal_color:1;

	unsigned short color_x_value:12;   /* 1.5 bytes */
	unsigned short color_y_value:12;   /* 1.5 bytes: 0.5 from above */
} color_char_t;

/* Type 1 - DTD - 20 bytes
 * Notes:
 * 1. In a Data Block, maximum of 12DTDs x 20bytes = 240bytes can fit.
 * 2. As part of parsing, DTD data will be read into type1_dtd_t then
 *    converted into pd_timing_t for IEGD consumption.
 * 3. There is no limit on maximum number of type 1 timings.
 *    They can be present not only in basic section but also in the
 *    extension section as well.
 */
typedef struct _type1_dtd {
	struct {
		unsigned short lsb_dclk;
		unsigned char  msb_dclk;
	} dclk;                                  /*  02-00 [07:00] */

	/* timing options - 1 byte */
	unsigned char  aspect_ratio:3;
	unsigned char  reserved:1;
	unsigned char  interlaced:1;
	unsigned char  stereo_3d_support:2;
	unsigned char  preferred:1;

	unsigned short hactive;
	unsigned short hblank;
	unsigned short hsync_offset:15;
	unsigned short hsync_polarity:1;
	unsigned short hsync_width;
	unsigned short vactive;
	unsigned short vblank;
	unsigned short vsync_offset:15;
	unsigned short vsync_polarity:1;
	unsigned short vsync_width;
} type1_dtd_t;

/* Type 2 - DTD - 11 bytes
 * Notes:
 * 1. In a Data Block, maximum of 22DTDs x 11bytes = 242bytes can fit.
 * 2. As part of parsing, DTD data will be read into type2_dtd_t then
 *    converted into pd_timing_t for IEGD consumption.
 * 3. There is no limit on maximum number of type 2 timings.
 *    They can be present not only in basic section but also in the
 *    extension section as well.
 */
typedef struct _type2_dtd {                   /* bytes: [bits] */
	struct {
		unsigned short lsb_dclk;
		unsigned char  msb_dclk;
	} dclk;                                  /*  02-00 [07:00] */

	/* timing options - 1 byte */
	unsigned char  reserved:4;                /*     03 [03:00] */
	unsigned char  interlaced:1;              /*     03 [04:04] */
	unsigned char  stereo_3d_support:2;       /*     03 [06:05] */
	unsigned char  preferred:1;               /*     03 [07:07] */

	unsigned short hactive:9;                 /*  05-04 [08:00] */
	unsigned short hblank:7;                  /*  05-04 [15:09] */
	unsigned char  hsync_width:4;             /*     06 [03:00] */
	unsigned char  hsync_offset:4;            /*     06 [07:04] */
	unsigned short vactive:12;                /*  08-07 [11:00] */
	unsigned short reserved1:4;               /*     08 [15:12] */

	unsigned char  vblank;                    /*     09 [07:00] */
	unsigned char  vsync_width:4;             /*     10 [03:00] */
	unsigned char  vsync_offset:4;            /*     10 [07:04] */
} type2_dtd_t;


/* Type 3 - DTD - CVT - 3 bytes
 * Notes:
 * 1. In a Data Block, maximum of 82timings x 3bytes = 246bytes can fit.
 * 2. As part of parsing, data will be read into type3_cvt_t then
 *    it will be made available for IEGD consumption
 *    either
 *         by marking in standard timing table
 *    or
 *         creating and inserting in pd_timing_t
 *    based on CVT formula.
 * 3. There is no limit on maximum number of type 3 timings.
 *    They can be present not only in basic section but also in the
 *    extension section as well.
 */
typedef struct _type3_cvt {                   /* bytes: [bits] */
	/* byte 0 - timing options */
	unsigned char  aspect_ratio:4;
	unsigned char  cvt_formula:3;
	unsigned char  preferred:1;

	/* byte 1 - horizontal active */
	unsigned char  hactive;

	/* byte 2 - other info */
	unsigned char  refresh:7;
	unsigned char  interlaced:1;
} type3_cvt_t;

/* Structure to emulate bit fields for Type1 Std or Type2 CEA timings */
typedef struct _type_std {
	unsigned short width;
	unsigned short height;
	unsigned short refresh;
	unsigned long  flags;         /* currently interlaced, rb, aspect ratio */
} type_std_t;

/* Display Device Data Block */
/* Analog interface subtype codes */
#define ANALOG_15HD_VGA     0x0
#define ANALOG_VESA_NAVI_V  0x1
#define ANALOG_VESA_NAVI_D  0x2

/* Device technology types */
#define TECH_CRT            0x0
#define TECH_LVDS           0x1
#define TECH_PLASMA         0x2
#define TECH_EL             0x3   /* Electroluminescent */
#define TECH_INORG          0x4
#define TECH_ORG            0x5
#define TECH_FED            0x6   /* FED or sim. "cold cathode" */
#define TECH_EP             0x7   /* Electrophoretic */
#define TECH_EC             0x8   /* Electrochromic */
#define TECH_EM             0x9   /* Electromechanical */
#define TECH_EW             0xA   /* Electrowetting */
#define TECH_OTHER          0xF   /* Othertype not defined here */

/* Device technoloty sub-types */
#define TECH_CRT_MONO       0x0   /* Mono chrome CRT */
#define TECH_CRT_STD        0x1   /* Standard tricolor CRT */
#define TECH_CRT_OTHER      0x2   /* other or undefined */

#define TECH_LVDS_PASSIVE_TN 0x0  /* Passive matrix TN */
#define TECH_LVDS_PASSIVE_CL 0x1  /* Passive matrix cholesteric LC */
#define TECH_LVDS_PASSIVE_FE 0x2  /* Passive matrix ferroelectric LC */
#define TECH_LVDS_PASSIVE_OT 0x3  /* Passive matrix other LC */
#define TECH_LVDS_ACTIVE_TN  0x4  /* Active matrix TN */
#define TECH_LVDS_ACTIVE_IPS 0x5  /* Active matrix IPS */
#define TECH_LVDS_ACTIVE_VA  0x6  /* Active matrix VA */
#define TECH_LVDS_ACTIVE_OCB 0x7  /* Active matrix OCB */
#define TECH_LVDS_ACTIVE_FL  0x8  /* Active matrix ferroelectric */
#define TECH_LVDS_OTHER      0xF  /* Other LVDS technology not defined above */

#define TECH_PLASMA_DC       0x0  /* DC plasma */
#define TECH_PLASMA_AC       0x1  /* AC plasma */

/* Content protection support */
#define CP_NOCP              0x0
#define CP_DTCP              0x2
#define CP_DPCP              0x3  /* Display port content protection */

/* Display physical orientation information */
#define DEFAULT_ORIENTATION(a) 		((a & 0xC0) >> 6)
#define ZERO_PIXEL(a) 				((a & 0x0C) >> 2)
#define SCAN_DIRECTION(a) 			(a & 0x03)

#define SCAN_DIRECTION_LONG 		0x1 /* Display scan direction (2 bits) */
#define SCAN_DIRECTION_SHORT 		0x2

#define ZP_UPPER_LEFT 				0x0 /* Zero pixel position (2 bits) */
#define ZP_UPPER_RIGHT 				0x1
#define ZP_LOWER_LEFT 				0x2
#define ZP_LOWER_RIGHT 				0x3

#define DEF_ORIENTATION_LANDSCAPE 	0x0 /* Default orientation (2 bits) */
#define DEF_ORIENTATION_PORTRAIT 	0x1
#define DEF_ORIENTATION_UNKNOWN 	0x2


typedef struct _display_device {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  tech_subtype:4;
	unsigned char  tech_type:4;
	unsigned char  reserved1:2;
	unsigned char  can_on_off_bklt:1;  /* This bit tells backlight can be
										* switched on or off using the control
										* method defined */
	unsigned char  can_change_bklt:1;  /* This bit tells backlight intensity
										* can be changed using the control
										* method defined */
	unsigned char  op_mode:4;

	unsigned short horz_pixel_count;
	unsigned short vert_pixel_count;
	unsigned char  aspect_ratio;
	unsigned char  orientation;
	unsigned char  subpixel_info;
	unsigned char  horz_pitch;
	unsigned char  vert_pitch;

	unsigned char  display_color_depth:4;
	unsigned char  reserved2:4;
	unsigned char  response_time;
} display_dev_t;

/* LVDS Interface Timing and Power Sequencing */
typedef struct _lvds_display {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  max_t1:4;
	unsigned char  min_t1:4;

	unsigned char  max_t2:6;
	unsigned char  reserved2:2;

	unsigned char  max_t3:6;
	unsigned char  reserved3:2;

	unsigned char  min_t4:7;
	unsigned char  reserved4:1;

	unsigned char  min_t5:6;
	unsigned char  reserved5:2;

	unsigned char  min_t6:6;
	unsigned char  reserved6:2;
} lvds_display_t;

/* Transfer Characteristics Data Block */
typedef struct _transfer_char {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;
	unsigned char  luminance;
} transfer_char_t;

/* Vendor specific Data Block */
typedef struct _vendor {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;
	unsigned char  vendor_id[3];                /* no NULL char */
	unsigned char  vendor_specific_data[246];   /* 245 + NULL */
} vendor_t;

/* Stereo methods */
#define STEREO_FIELD_SEQUENTIAL      0x00
#define STEREO_SIDE_BY_SIDE          0x01
#define STEREO_PIXEL_INTERLEAVED     0x02
#define STEREO_DUAL_INTERFACE        0x03
#define STEREO_MULTIVIEW             0x04
#define STEREO_PROPRIETARY           0xFF

/* Stereo Display Interface Data Block */
typedef struct _stereo_intf {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  stereo_intf_payload;
	unsigned char  stereo_intf_method;
	union {
		unsigned char  stereo_polarity;     /* Field Sequential Stereo */
		unsigned char  view_identity;       /* Side-by-side Stereo */
		unsigned char  pattern8x8[8];       /* Pixel Interleaved Stereo */
		unsigned char  polarity_mirroring;  /* Dual Interface Stereo */
		struct {                            /* Multiview stereo */
			unsigned char  num_views;
			unsigned char  interleaving_method;
		} multiview;
		/* Note: In case of proprietary stereo method, the parameters can be
		 * somewhere in the Vendor Specific Block */
		unsigned char  stereo_params[8];
	};
} stereo_intf_t;

/* Display Interface Data Block */
#define INTERFACE_ANALOG    0x0
#define INTERFACE_LVDS      0x1
#define INTERFACE_TMDS      0x2
#define INTERFACE_RSDS      0x3
#define INTERFACE_DVID      0x4
#define INTERFACE_DVII_A    0x5
#define INTERFACE_DVII_D    0x6
#define INTERFACE_HDMI_A    0x7
#define INTERFACE_HDMI_B    0x8
#define INTERFACE_MDDI      0x9
#define INTERFACE_DISP_PORT 0xA
#define INTERFACE_PROP_DIG  0xB

/* Interface color depth */
#define INTERFACE_6BPPC            0x01   /* 6 bits per primary color */
#define INTERFACE_8BPPC            0x02   /* 8 bits per primary color */
#define INTERFACE_10BPPC           0x04   /* 10 bits per primary color */
#define INTERFACE_12BPPC           0x08   /* 12 bits per primary color */
#define INTERFACE_14BPPC           0x10   /* 14 bits per primary color */
#define INTERFACE_16BPPC           0x20   /* 16 bits per primary color */

typedef struct _display_intf {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  num_channels:4;
	unsigned char  intf_type:4;

	unsigned char  intf_revision:4;
	unsigned char  intf_version:4;

	unsigned char  rgb_color_depth:6;
	unsigned char  reserved2:2;

	unsigned char  ycbcr_444_color_depth:6;
	unsigned char  reserved3:2;

	unsigned char  ycbcr_422_color_depth:6;
	unsigned char  reserved4:2;

	unsigned char  cp_type:3;
	unsigned char  reserved5:2;
	unsigned char  color_encoding:3;

	unsigned char  cp_revision:4;
	unsigned char  cp_version:4;

	unsigned char  ss_percent:4;
	unsigned char  reserved6:2;
	unsigned char  ss_type:2;

	union {
		struct {
			/* Interface type: LVDS */
			unsigned char  support_3_3v:1;
			unsigned char  support_5v:1;
			unsigned char  support_12v:1;
			unsigned char  support_2_8v:1;
			unsigned char  openldi:1;
			unsigned char  reserved5:3;

			unsigned char  data_strobe:1;
			unsigned char  de_polarity:1;
			unsigned char  de_mode:1;
			unsigned char  reserved6:5;
		} lvds;
		struct {
			/* Interface type: Proprietary Digital Interface */
			unsigned char  data_strobe:1;
			unsigned char  de_polarity:1;
			unsigned char  de_mode:1;
			unsigned char  reserved7:5;

			unsigned char  reserved8;
		} pdi;
		unsigned short bytes9_10;
	};
} display_intf_t;

/* Serial Number String */
typedef struct _serial_number {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  serial_num[249];    /* 248 + NULL */
} serial_number_t;

/* General Purpose ASCII String */
typedef struct _general_string {
	unsigned char  tag;                /* data block type */
	unsigned char  revision:3;
	unsigned char  reserved:5;
	unsigned char  payload;

	unsigned char  string[249];        /* 248 + NULL */
} general_string_t;

/* This structure holds all of the parsed DisplayID information.*/
/* 5-required bytes:
 *  1-byte  - version, revision
 *  1-byte  - payload
 *  1-byte  - number of extensions
 *  1-byte  - product type identifier
 *  1-byte  - checksum
 */
typedef struct _displayid {
	unsigned char    revision:4;         /* HEADER: revision */
	unsigned char    version:4;          /* version */
	unsigned char    payload;            /* payload excluding 5 required bytes*/
	unsigned char    ptype_id;           /* DisplayID product type identifier */
	unsigned char    num_extensions;     /* number of extension sections */
	unsigned char    check_sum;          /* FOOTER: last byte of the section */

	display_params_t display_params;     /* display params block */
	timing_range_t   timing_range;       /* timing range block */
	lvds_display_t   lvds;               /* LVDS interface & display block */
	display_dev_t    display_dev;        /* display device block */
	display_intf_t   display_intf;       /* display interface block */
	unsigned char    dummy_db[256];      /* dummy datablock */

	/* Video Timing Modes Type I, II DTDs into pd_timing_t */
	unsigned char   num_timings;
	pd_timing_t     timings[DISPLAYID_MAX_NUM_TIMINGS];

	/* DisplayID attributes */
	unsigned char   num_attrs;
	pd_attr_t       attr_list[DISPLAYID_MAX_ATTRS];

#ifndef CONFIG_MICRO
	productid_t      		productid;       /* product id block */
	color_char_t     		color_char;      /* color characteristics block */
	serial_number_t  		serial_num;      /* serial number ASCII string */
	general_string_t 		general_string;  /* general purpose ASCII string */
	transfer_char_t  		transfer_char;   /* transfer characteristics block */
	stereo_intf_t    		stereo_intf;     /* stereo display interface */
	vendor_t 				vendor;          /* vendor specific data */
	igd_DID_rotation_info_t rotation_info;   /* display orientation data */
#endif
} displayid_t;

/* Pack back to default */
#pragma pack()

/* Functions */
void displayid_filter_range_timings(pd_timing_t *tt, timing_range_t *range,
	unsigned char firmware_type);
int displayid_parse(unsigned char *buffer, displayid_t   *displayid,
		pd_timing_t *timings, int count, unsigned char upscale);

void displayid_dump(unsigned char *buffer, unsigned short size);
void displayid_print(unsigned char *buffer, displayid_t *displayid);

#endif

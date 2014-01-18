/*
 *-----------------------------------------------------------------------------
 * Filename: igd_pd.h
 * $Revision: 1.13 $
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_PD_H_
#define _IGD_PD_H_

#define PD_ATTR_LIST_END     0xFFFFFFFF

#define PD_NAME_SIZE      64

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

/*!
 * @addtogroup display_group
 *
 * @{
 */

/*
 * Display attributes (igd_attr_t):
 *    id   - Any associated id. See below list for known attribute ids.
 *
 *    type - type of the attribute. (RANGE/LIST/BOOL/BUFFER)
 *
 *    name - Name of the attribute. For example 'brightness', 'contrast'
 *
 *    flags - any flags for the attribute
 *
 *            PD_ATTR_FLAG_SETMODE - IGD will set this flag.
 *                                    If this flag is set, then changing
 *                                    the attribute value required set mode
 *                                    operation.
 *
 *                                    Caller (IAL) should check for this flag
 *                                    after calling igd_set_attrs(). If this
 *                                    flag is set, then IAL has to do a setmode
 *                                    operation via igd_alter_display().
 *
 *            PD_ATTR_FLAG_VALUE_CHANGED - Caller will set this flag.
 *                                          If this flag is set, means the
 *                                          attribute value was changed to
 *                                          the value mentioned.
 *
 *                                          Caller will specify SS that the
 *                                          value was changed.
 *
 *            PD_ATTR_FLAG_DONT_APPLY -    If this flag is set, means the
 *                                          attribute value will be applied
 *                                          to new value at the time of next
 *                                          setmode operation.
 *
 *
 *            PD_ATTR_FLAG_USER_INVISIBLE - If this flag is set, means the
 *                                          attribute is invisible to end-user
 *                                          and not available for run-time
 *                                          changes.
 *
 *    If the type is PD_ATTR_TYPE_RANGE, then this structure contains further
 *    details about this attribute (igd_range_attr_t):
 *      default_value  - Default value
 *      current_value  - Current value
 *      min            - Minimum value for this attribute
 *      max            - Maximum value for this attribute
 *      step           - Value changes in increments of 'step'
 *
 *    If the type is PD_ATTR_TYPE_LIST, then this structure contains further
 *    details about this attribute (igd_list_attr_t):
 *      default_index  - this is the index into above list to point
 *                              default value for this attribute
 *
 *      current_index  - this is the index into above list to point
 *                              current value for this attribute
 *
 *      num_entries    - number values avaliable for this list attribute,
 *                       for example, for TVFormat attribute, the available
 *                       values can be NTSC, NTSC_M, PAL, PAL_D etc etc...
 *
 *      _pad           - [reserved] Pad bytes to make all attributes
 *                       structures with same size.
 *
 *    If the type is PD_ATTR_TYPE_LIST, then there are num_entries structures
 *    of igd_list_entry_attr_t will be there (igd_list_entry_attr_t):
 *      name           - name is overloaded for example for TVFormat attribute
 *                       the names will be "NTSC", "NTSC_M" etc etc...
 *
 *      flags          - same as above flags.
 *
 *      value          - Internal value for "NTSC", i.e., the #define value.
 *
 *      _pad           - [reserved] Pad bytes to make all attributes
 *                       structures with same size.
 *
 *    If the type is PD_ATTR_TYPE_BOOL, then this structure contains further
 *    details about this attribute (igd_bool_attr_t):
 *      default_value  - Default value (TRUE/FALSE)
 *
 *      current_value  - Current value (TRUE/FALSE)
 *
 *      _pad           - [reserved] Pad bytes to make all attributes
 *                       structures with same size.
 *
 *    If the type is PD_ATTR_TYPE_BUFFER, then there is a buffer pointer
 *    in the structure to pass larger chunks of data. Originator owns the
 *    buffer.
 *      buffer_size    - size of the buffer
 *
 *      buffer         - pointer to buffer
 *
 */
#define PD_ATTR_TYPE_RANGE      0x1
#define PD_ATTR_TYPE_LIST       0x2
#define PD_ATTR_TYPE_BOOL       0x3
#define PD_ATTR_TYPE_LIST_ENTRY 0x4
#define PD_ATTR_TYPE_BUFFER     0x5

/*!
 * @name Attribute ID definitions
 * @anchor attr_id_defs
 *
 *  These IDs specify what the attribute is and also index into the list of
 * attributes. See get_attrs() and set_attrs() for more information.
 *
 * @{
 */
#define PD_ATTR_ID_BRIGHTNESS      0x00
#define PD_ATTR_ID_CONTRAST        0x01
#define PD_ATTR_ID_HUE             0x02
#define PD_ATTR_ID_FLICKER         0x03
#define PD_ATTR_ID_HPOSITION       0x04
#define PD_ATTR_ID_VPOSITION       0x05
#define PD_ATTR_ID_HSCALE          0x06
#define PD_ATTR_ID_VSCALE          0x07
#define PD_ATTR_ID_TVFORMAT        0x08  /* See below for list of TV formats */
#define PD_ATTR_ID_DISPLAY         0x09  /* See below for list of displays */
#define PD_ATTR_ID_LUMA_FILTER     0x0A
#define PD_ATTR_ID_CHROMA_FILTER   0x0B
#define PD_ATTR_ID_TEXT_FILTER     0x0C
#define PD_ATTR_ID_TVOUT_TYPE      0x0E
#define PD_ATTR_ID_SATURATION      0x0F
#define PD_ATTR_ID_PANEL_FIT       0x12  /* Up/Down scale yes/no */
#define PD_ATTR_ID_SCALING_RATIO   0x13
#define PD_ATTR_ID_FP_BACKLIGHT_EN 0x14  /* Enable Flat panel backlight      */
#define PD_ATTR_ID_FP_PWR_T1       0x15  /* VDD active - DVO CLK/DATA active */
#define PD_ATTR_ID_FP_PWR_T2       0x16  /* DVO active - Backlight enable    */
#define PD_ATTR_ID_FP_PWR_T3       0x17  /* Backlight disable - DVO inactive */
#define PD_ATTR_ID_FP_PWR_T4       0x18  /* DVO inactive - VDD inactive      */
#define PD_ATTR_ID_FP_PWR_T5       0x19  /* VDD inactive - VDD active        */
#define PD_ATTR_ID_PANEL_DEPTH     0x1A  /* Range, 18/24 panel */
#define PD_ATTR_ID_2_CHANNEL_PANEL 0x1B  /* Boolean, dual-channel panel ? */
#define PD_ATTR_ID_GANG_MODE       0x1C  /* Boolean, gang display or not? */
#define PD_ATTR_ID_GANG_MODE_EVEN_ODD 0x1D  /* Boolean, gang display even/odd */
#define PD_ATTR_ID_REVERSE_DVO_DATA   0x1E  /* Request reverse DVO data order */
#define PD_ATTR_ID_SHARPNESS       0x1F
#define PD_ATTR_ID_HW_CONFIG       0x20
#define PD_ATTR_ID_HORZ_FILTER     0x21
#define PD_ATTR_ID_VERT_FILTER     0x22
#define PD_ATTR_ID_FB_GAMMA        0x23
#define PD_ATTR_ID_FB_BRIGHTNESS   0x24
#define PD_ATTR_ID_FB_CONTRAST     0x25
#define PD_ATTR_ID_EXTENSION       0x26
#define PD_ATTR_ID_2D_FLICKER      0x27
#define PD_ATTR_ID_ADAPTIVE_FLICKER     0x28
#define PD_ATTR_ID_HORZ_OVERSCAN        0x29
#define PD_ATTR_ID_VERT_OVERSCAN        0x2A
#define PD_ATTR_ID_SSC                  0x2B
#define PD_ATTR_ID_DOT_CRAWL            0x2C
#define PD_ATTR_ID_DITHER               0x2D
#define PD_ATTR_ID_PANEL_PROTECT_HSYNC  0x2E
#define PD_ATTR_ID_PANEL_PROTECT_VSYNC  0x2F
#define PD_ATTR_ID_PANEL_PROTECT_PIXCLK 0x30
#define PD_ATTR_ID_LVDS_PANEL_TYPE      0x31
#define PD_ATTR_ID_ANALOG_SRC           0x34
#define PD_ATTR_ID_SCAN_INFO            0x35
#define PD_ATTR_ID_PICTURE_ASPECT_RATIO 0x36
#define PD_ATTR_ID_ACTIVE_FORMAT        0x37
#define PD_ATTR_ID_GANG_MODE_DVOCLKINV  0x38
#define PD_ATTR_ID_VGA_2X_IMAGE         0x39
#define PD_ATTR_ID_TEXT_TUNING          0x3A
#define PD_ATTR_ID_MAINTAIN_ASPECT_RATIO 0x3B
#define PD_ATTR_ID_FIXED_TIMING         0x3C
#define PD_ATTR_ID_COLORIMETRY			0x3D   /* HDMI Attributes */
#define PD_ATTR_ID_PIXEL_REPLICATION	0x3E
#define PD_ATTR_ID_INTERRUPT_TYPE       0x3F   /* sdvo interrupt type */
/* Port Driver alternative name */
#define PD_ATTR_ID_PORT_NAME	        0x44
/* Port ddc_reg */
#define PD_ATTR_ID_PORT_DDC_REG			0x45
#define PD_ATTR_ID_PWM_INTENSITY		0x46
#define PD_ATTR_ID_INVERTER_FREQ		0x47
#define PD_ATTR_ID_BLM_LEGACY_MODE		0x48
#define PD_ATTR_ID_RB_SWAP_MODE			0x49
//************ port driver attributes defined for ch7036 port driver **************//
#define PD_ATTR_ID_HFLIP			0x54
#define PD_ATTR_ID_VFLIP			0x55

//hdmi audio
#define PD_ATTR_ID_AUDIO_I2S_FORMAT		0x56
#define PD_ATTR_ID_AUDIO_I2S_POLARITY           0x57
#define PD_ATTR_ID_AUDIO_I2S_LENGH		0x58

#define PD_ATTR_ID_AUDIO_TYPE			0x59 //1- I2S, 0: SPDIF

//display mode tables supported by ch7036
#define PD_ATTR_ID_HDMI_OUT_MODE		0x5A
#define PD_ATTR_ID_DVI_OUT_MODE			0x5B
#define PD_ATTR_ID_CRT_OUT_MODE			0x5C


#define PD_ATTR_ID_DITHER_BYPASS		0x5D //to bypass Dither block - note: PD_ATTR_ID_DITHER is for dither selection: 18-18, 18-24, etc...
#define PD_ATTR_ID_EDID_BYPASS			0x5E //1: bypass (use fixed built-in table, 0: read panel EDID)

//1:hdmi, 0: dvi- note: PD_ATTR_ID_HDMI_CHANNEL will be  ‘inactive’ after EDID reading is
//implemented and selected, e.g. PD_ATTR_ID_EDID_BYPASS =0
#define PD_ATTR_ID_HDMI_CHANNEL			0x5F
#define PD_ATTR_ID_PLL_REF_DLY			0x60
#define PD_ATTR_ID_PLL_REF_FBDLY		0x61
#define PD_ATTR_ID_ROTATE			0x62
#define PD_ATTR_ID_HSCALE_CRT			0x63
#define PD_ATTR_ID_VSCALE_CRT			0x64

#define PD_ATTR_ID_LOAD_FIRMWARE		0x65
#define PD_ATTR_ID_REFRESH			0x66
#define PD_ATTR_ID_DWNSCAL_BYPASS		0x57 //1.2.5.pd: new pd attributes to handle the downscalling quality issue


//************ end of ch7036 defined port driver attribute ********** //

#define PD_ATTR_ID_NUM_IDS			0x67  /* Always make this last */

/*! @} */

/* Bit fields. */
#define PD_ATTR_FLAG_SETMODE         0x01
#define PD_ATTR_FLAG_VALUE_CHANGED   0x02
#define PD_ATTR_FLAG_DONT_SET_IMM    0x04
#define PD_ATTR_FLAG_USER_INVISIBLE  0x08   /* Attr invisible to enduser */
#define PD_ATTR_FLAG_PD_INVISIBLE    0x10   /* Attr invisible to PD */
#define PD_ATTR_FLAG_DYNAMIC         0x20   /* Attribute queried from sDVO */
#define PD_ATTR_FLAG_NOTUSED         0x80   /* Attribute not used, this flag
											 * supercedes all other flags */
#define PD_ATTR_FLAG_NEED_RESTORE	 0x100  /* Reset occured and register value is in limbo.
												During this time we need to keep
												the value of the attr in our table
												unchanged. */
#define PD_ATTR_FLAG_GENERAL		 0x1000 /* This flag is used by HAL to querry
											 * general attributes */
#define PD_QUERY_GENERAL_ATTR		PD_ATTR_ID_NUM_IDS + 2
/* TVoutput types PD_ATTR_ID_TVOUT_TYPE */
#define PD_TVOUT_COMPOSITE    0x01
#define PD_TVOUT_SVIDEO       0x02
#define PD_TVOUT_COMPONENT    0x03
#define PD_TVOUT_CPSTSV       0x04          /* Composite and S-Video */
#define	PD_TVOUT_SCARTRGB     0x05          /* SCART output */
#define	PD_TVOUT_VGADAC       0x06          /* VGA Outout */

/* TV Standard Option */
/* Similar TV formats need to have the same last nibble.
 * Example for PAL the last nibble is 0x2, for NTSC it is
 * 0x1 and so forth.
 */
#define PD_TV_STD_NTSC_M       0x01    /* USA, 75 IRE Setup */
#define PD_TV_STD_NTSC_M_J     0x11    /* Japan,  0 IRE Setup */
#define PD_TV_STD_NTSC_433     0x21
#define PD_TV_STD_NTSC_N       0x31

#define PD_TV_STD_PAL_B        0x02    /* Australia, U.K., N.Z etc */
#define PD_TV_STD_PAL_G        0x12
#define PD_TV_STD_PAL_D        0x22
#define PD_TV_STD_PAL_H        0x32
#define PD_TV_STD_PAL_I        0x42
#define PD_TV_STD_PAL_M        0x52
#define PD_TV_STD_PAL_N        0x62
#define PD_TV_STD_PAL_60       0x72
#define PD_TV_STD_PAL_NC       0x82

#define PD_TV_STD_SECAM_L      0x03    /* France */
#define PD_TV_STD_SECAM_L1     0x13
#define PD_TV_STD_SECAM_B      0x23
#define PD_TV_STD_SECAM_D      0x33
#define PD_TV_STD_SECAM_G      0x43
#define PD_TV_STD_SECAM_H      0x53
#define PD_TV_STD_SECAM_K      0x63
#define PD_TV_STD_SECAM_K1     0x73
#define PD_TV_STD_SECAM_60     0x83

#define PD_TV_STD_HDTV_480P    0x04
#define PD_TV_STD_HDTV_720P    0x14
#define PD_TV_STD_HDTV_1080I   0x24

/* These HDTV defines are added for Int TV in 915GM */
#define PD_TV_STD_HDTV_480I_59			0x34  /* 59Hz */
#define PD_TV_STD_HDTV_480P_59			0x44
#define PD_TV_STD_HDTV_480P_60			0x54

#define PD_TV_STD_HDTV_576I_50			0x64
#define PD_TV_STD_HDTV_576P_50			0x74

#define PD_TV_STD_HDTV_720P_50			0x84
#define PD_TV_STD_HDTV_720P_59			0x94
#define PD_TV_STD_HDTV_720P_60			0xA4

#define PD_TV_STD_HDTV_1080I_50			0xB4
#define PD_TV_STD_HDTV_1080I_59			0xC4
#define PD_TV_STD_HDTV_1080I_60			0xD4

#define PD_TV_STD_HDTV_1080P_60			0xE4

/* Defines for SDVO TV */
#define PD_HDTV_STD_SMPTE_240M_1080i59  0x101
#define PD_HDTV_STD_SMPTE_240M_1080i60  0x102

#define PD_HDTV_STD_SMPTE_260M_1080i59  0x103
#define PD_HDTV_STD_SMPTE_260M_1080i60  0x104

#define PD_HDTV_STD_SMPTE_274M_1080i50  0x105
#define PD_HDTV_STD_SMPTE_274M_1080i59  0x106
#define PD_HDTV_STD_SMPTE_274M_1080i60  0x107
#define PD_HDTV_STD_SMPTE_274M_1080p23  0x108
#define PD_HDTV_STD_SMPTE_274M_1080p24  0x109
#define PD_HDTV_STD_SMPTE_274M_1080p25  0x10A
#define PD_HDTV_STD_SMPTE_274M_1080p29  0x10B
#define PD_HDTV_STD_SMPTE_274M_1080p30  0x10C
#define PD_HDTV_STD_SMPTE_274M_1080p50  0x10D
#define PD_HDTV_STD_SMPTE_274M_1080p59  0x10E
#define PD_HDTV_STD_SMPTE_274M_1080p60  0x10F

#define PD_HDTV_STD_SMPTE_295M_1080i50  0x110
#define PD_HDTV_STD_SMPTE_295M_1080p50  0x111

#define PD_HDTV_STD_SMPTE_296M_720p59   0x112
#define PD_HDTV_STD_SMPTE_296M_720p60   0x113
#define PD_HDTV_STD_SMPTE_296M_720p50   0x114

#define PD_HDTV_STD_SMPTE_293M_480p59   0x115
#define PD_HDTV_STD_SMPTE_170M_480i59   0x116

#define PD_HDTV_STD_ITURBT601_576i50    0x117
#define PD_HDTV_STD_ITURBT601_576p50    0x118
#define PD_HDTV_STD_EIA_7702A_480i60    0x119
#define PD_HDTV_STD_EIA_7702A_480p60    0x11A

/* Supported display types */
/* Port drivers for DVO devices shouldn't use
 *    PD_DISPLAY_CRT,
 *    PD_DISPLAY_TVOUT_INT,
 *        and
 *    PD_DISPLAY_LVDS_INT. These are for internal on board GMCH displays..
 *
 * Only following display types are available for port drivers
 * for DVO devices:
 *    PD_DISPLAY_CRT_EXT
 *    PD_DISPLAY_TVOUT
 *    PD_DISPLAY_TVFP (i.e., PD_DISPLAY_TVOUT|PD_DISPLAY_FP)
 *    PD_DISPLAY_LVDS_EXT
 *    PD_DISPLAY_FP
 *    PD_DISPLAY_RGBA
 *    PD_DISPLAY_DRGB
 *
 * Also port drivers can make new combinations, for example,
 * if an DVO encoder supports both LVDS and TVOut displays, it can use
 *    PD_DISPLAY_LVDS_EXT | PD_DISPLAY_TVOUT
 */
#define PD_DISPLAY_CRT         0x00000001    /* GMCH on board CRT */
#define PD_DISPLAY_TVOUT       0x00000002    /* TVOUT display type */
#define PD_DISPLAY_FP          0x00000004    /* Flat panel type */
#define PD_DISPLAY_TVFP        0x00000006    /* Codec supports TV + FP */
#define PD_DISPLAY_RGBA        0x00000008    /* RGBA display */
#define PD_DISPLAY_LVDS_EXT    0x00000010    /* External local flat panel */
#define PD_DISPLAY_LVDS_INT    0x00000020    /* GMCH on board LVDS */
#define PD_DISPLAY_CRT_EXT     0x00000040    /* External CRT display */
#define PD_DISPLAY_TVOUT_INT   0x00000080    /* GMCH on board TVout */
#define PD_DISPLAY_HDMI_INT    0x00000100    /* HDMI on board */
#define PD_DISPLAY_HDMI_EXT    0x00000200    /* External HDMI */
#define PD_DISPLAY_DP_INT      0x00000400    /* Display Port on board */
#define PD_DISPLAY_DRGB        0x00000800    /* SDVO DRGB display */
/* new display types introduced by LVDS converter chip, CH7036 */
#define PD_DISPLAY_LVDS_LHDV   0x00001000  /* lvds converter- converts lvds signals to 
												lvds pass-thru, hdmi, dvi, or vga */


#ifdef CONFIG_MICRO
#define PD_MAKE_ATTR(id, type, name, flags, def, cur, a, b, c) \
	{id, type, flags, def, cur, a, b, c}
#else
#define PD_MAKE_ATTR(id, type, name, flags, def, cur, a, b, c) \
	{id, type, name, flags, def, cur, a, b, c}
#endif

typedef struct _igd_DID_rotation_info_t {
	int rotation;
	int flip;
} igd_DID_rotation_info_t;

/* IMP NOTE: All below structures should be with same size.
 *         igd_attr_t            : General attribute structure
 *         igd_range_attr_t      : Range type attribute structure
 *         igd_list_attr_t       : List type attribute
 *         igd_list_entry_attr_t : Entry for a list
 *         igd_bool_attr_t       : Boolean type attribute
 *         igd_extension_attr_t  : Extension type attribute
 *         igd_buffer_attr_t     : Buffer type attribute
 */
typedef struct _igd_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char name[32];
#endif
	unsigned long flags;
	unsigned long default_value;
	unsigned long current_value;
	unsigned long _pad0;
	unsigned long _pad1;
	unsigned long _pad2;
}igd_attr_t;

typedef struct _igd_range_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char          name[32];
#endif
	unsigned long flags;
	unsigned long default_value;
	unsigned long current_value;
	unsigned long min;
	unsigned long max;
	unsigned long step;
}igd_range_attr_t;

typedef struct _igd_list_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char          name[32];
#endif
	unsigned long flags;
	unsigned long default_index;
	unsigned long current_index;
	unsigned long num_entries;
	unsigned long _pad0;
	unsigned long _pad1;
}igd_list_attr_t;

typedef struct _igd_list_entry_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char          name[32];
#endif
	unsigned long flags;
	unsigned long value;
	unsigned long _pad0;
	unsigned long _pad1;
	unsigned long _pad2;
	unsigned long _pad3;
}igd_list_entry_attr_t;

typedef struct _igd_bool_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char          name[32];
#endif
	unsigned long flags;
	unsigned long default_value;
	unsigned long current_value;
	unsigned long _pad0;
	unsigned long _pad1;
	unsigned long _pad2;
}igd_bool_attr_t;

typedef struct _igd_buffer_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char          name[32];
#endif
	unsigned long flags;
	unsigned long buffer_size;
	void          *buffer;
	unsigned long _pad0;
	unsigned long _pad1;
	unsigned long _pad2;
} igd_buffer_attr_t;

typedef struct _igd_extension_attr {
	unsigned long id;
	unsigned long type;
#ifndef CONFIG_MICRO
	char name[32];
#endif
	unsigned long flags;
	unsigned long default_value;
	unsigned long current_value;
	igd_attr_t    *extension;
} igd_extension_attr_t;


/*! @} */

#endif /* _IGD_PD_H_ */

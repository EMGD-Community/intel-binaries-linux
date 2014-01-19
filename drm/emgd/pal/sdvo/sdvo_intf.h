/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_intf.h
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
 *  Definitions for SDVO interface
 *-----------------------------------------------------------------------------
 */

#ifndef _SDVO_INTF_H_
#define _SDVO_INTF_H_

/*  ......................................................................... */
/*  Opcode, Status Code, Register definitions */

typedef enum {
    /*  General */
    RESET = 0x1,
    GET_DEVICE_CAPABILITIES,
    GET_TRAINED_INPUTS,
    GET_ACTIVE_OUTPUTS,
    SET_ACTIVE_OUTPUTS,

    /*  Picture Routing  */
    GET_IN_OUT_MAP = 0x6,
    SET_IN_OUT_MAP,

    GET_ATTACHED_DISPLAYS = 0x0B,

    /* Hot Plug */
    GET_HOT_PLUG_SUPPORT = 0x0C,
    SET_ACTIVE_HOT_PLUG,
    GET_ACTIVE_HOT_PLUG,
    GET_INTERRUPT_EVENT_SOURCE,

    /*  Input/Output Timings */
    SET_TARGET_INPUT = 0x10,
    SET_TARGET_OUTUT,
    GET_INPUT_TIMINGS_PART1,
    GET_INPUT_TIMINGS_PART2,
    SET_INPUT_TIMINGS_PART1,
    SET_INPUT_TIMINGS_PART2,
    SET_OUTPUT_TIMINGS_PART1,
    SET_OUTPUT_TIMINGS_PART2,
    GET_OUTPUT_TIMINGS_PART1,
    GET_OUTPUT_TIMINGS_PART2,
    CREATE_PREFERRED_INPUT_TIMINGS,
    GET_PREFERRED_INPUT_TIMINGS_PART1,
    GET_PREFERRED_INPUT_TIMINGS_PART2,
    GET_INPUT_PIXEL_CLOCK_RANGE,
    GET_OUTPUT_PIXEL_CLOCK_RANGE,
    GET_SUPPORTED_SDVO_CLOCK_RATE_MULTIPLIERS,
    GET_SDVO_CLOCK_RATE_MULTIPLIER,
    SET_SDVO_CLOCK_RATE_MULTIPLIER,

    GET_SUPPORTED_TV_OUTPUT_FORMATS = 0x27,
    GET_TV_OUTPUT_FORMAT,
    SET_TV_OUTPUT_FORMAT,

    /*  Power Management */
    GET_SUPPORTED_POWER_STATES = 0x2A,
    GET_POWER_STATE,
    SET_POWER_STATE,

    /*  Panel Power Sequencing */
    GET_MAX_PANEL_POWER_SEQUENCING_PARAMETER = 0x2D,
    GET_PANEL_POWER_SEQUENCING_PARAMETER,
    SET_PANEL_POWER_SEQUENCING_PARAMETER,

    GET_MAX_BACKLIGHT_LEVEL,
    GET_BACKLIGHT_LEVEL,
    SET_BACKLIGHT_LEVEL,

    GET_AMBIENT_LIGHT,
    SET_AMBIENT_LIGHT,

    SET_UPSCALAR_COEFFICIENTS = 0x36,

    GET_SSC,
    SET_SSC,

    /*  Content Protection */
    GET_ANCILLARY_VIDEO_INFORMATION,
    SET_ANCILLARY_VIDEO_INFORMATION,

    /*  Picture Enhancements */
    GET_MAX_FLICKER_FILTER = 0x4D,
    GET_FLICKER_FILTER,
    SET_FLICKER_FILTER,

    GET_ADAPTIVE_FLICKER_FILTER,
    SET_ADAPTIVE_FLICKER_FILTER,

    GET_MAX_2D_FLICKER_FILTER,
    GET_2D_FLICKER_FILTER,
    SET_2D_FLICKER_FILTER,

    GET_MAX_SATURATION,
    GET_SATURATION,
    SET_SATURATION,

    GET_MAX_HUE,
    GET_HUE,
    SET_HUE,

    GET_MAX_BRIGHTNESS,
    GET_BRIGHTNESS,
    SET_BRIGHTNESS,

    GET_MAX_CONTRAST,
    GET_CONTRAST,
    SET_CONTRAST,

    GET_MAX_HORIZONTAL_OVERSCAN,
    GET_HORIZONTAL_OVERSCAN,
    SET_HORIZONTAL_OVERSCAN,

    GET_MAX_VERTICAL_OVERSCAN,
    GET_VERTICAL_OVERSCAN,
    SET_VERTICAL_OVERSCAN,

    GET_MAX_HORIZONTAL_POSITION,
    GET_HORIZONTAL_POSITION,
    SET_HORIZONTAL_POSITION,

    GET_MAX_VERTICAL_POSITION,
    GET_VERTICAL_POSITION,
    SET_VERTICAL_POSITION,

    GET_MAX_SHARPNESS,
    GET_SHARPNESS,
    SET_SHARPNESS,

    GET_DOT_CRAWL,
    SET_DOT_CRAWL,

    GET_DITHER,
    SET_DITHER,

    GET_MAX_TV_CHROMA_FILTER,
    GET_TV_CHROMA_FILTER,
    SET_TV_CHROMA_FILTER,

    GET_MAX_TV_LUMA_FILTER,
    GET_TV_LUMA_FILTER,
    SET_TV_LUMA_FILTER,

    /*  Control Bus Switch */
    SET_CONTROL_BUS_SWITCH = 0x7A,

    /*  Picture Enhancements */
    GET_MAX_ADAPTIVE_FLICKER_FILTER = 0x7B,

    /*  Power Management */
    SET_DISPLAY_POWER_STATE = 0x7D,

    /*  Panel Power Sequencing */
    GET_MAX_SSC = 0x7E,
    GET_LVDS_PANEL_INFORMATION,
    SET_LVDS_PANEL_INFORMATION,
    GET_LVDS_PANEL_PROTECTION,
    SET_LVDS_PANEL_PROTECTION,

	GET_SDTV_RESOLUTION_SUPPORT = 0x83,
	GET_SUPPORTED_ENHANCEMENTS,
	GET_SCALED_HDTV_RESOLUTION_SUPPORT,

	/*  General */
	GET_FIRMWARE_VERSION = 0x86,
	DETECT_LEGACY_VGA = 0x87,

	/* HDMI */
	PREPARE_TO_GENERATE_HDCPAN = 0x8A,
	SET_PIXEL_REPLICATION = 0x8B,
	GET_PIXEL_REPLICATION,
	GET_COLORIMETRY_CAPABILITIES,
	SET_CURRENT_COLORIMETRY,
	GET_CURRENT_COLORIMETRY,
	GET_AUDIO_ENCRYPTION_PREFERENCE,
	SET_AUDIO_STATE,
	GET_AUDIO_STATE,
	SET_HDMI_BUFFER_INDEX,
	GET_HDMI_BUFFER_INDEX,
	GET_HDMI_BUFFER_INFO,
	SET_HDMI_BUFFER_AUDIO_VIDEO_SPLIT,
	GET_HDMI_BUFFER_AUDIO_VIDEO_SPLIT,
	SET_HDMI_BUFFER_DATA,
	GET_HDMI_BUFFER_DATA,
	SET_HDMI_BUFFER_TRANSMIT_RATE,
	GET_HDMI_BUFFER_TRANSMIT_RATE,
	GET_HDMI_AUDIO_TRANSMISSION_CHARACTERISTICS,
	GET_SUPPORTED_DIGITAL_ENCODING_MODES, /* 0x9D, */
	GET_DIGITAL_ENCODING_MODE,
	SET_DIGITAL_ENCODING_MODE,
} sdvo_opcode_t;

typedef enum {
    SS_POWER_ON_STATE,       /*  00: */
    SS_SUCCESS,              /*  01: Command successfully completed */
    SS_NOT_SUPPORTED,        /*  02: Invalid Opcode */
    SS_INVALID_ARGUMENT,     /*  03: Opcode is valid but arguments are not */
    SS_PENDING,              /*  04: Opcode is not completed yet */
    SS_TARGET_UNSPECIFIED,   /*  05: SetTargetInput/Output not called */
    SS_SCALING_UNSUPPORTED,  /*  06: Device does not support scaling */

    /*  New Errors */
    SS_UNSUCCESSFUL = 10,    /*  10: General Failure */
    SS_INVALID_RETURN,       /*  11: Unexpected return value */
    SS_WRITE_FAILED,         /*  12: Write Operation failed */
    SS_READ_FAILED,          /*  13: Read Operation failed */
} sdvo_status_t;

#define SDVO_MAX_ARGS           8
#define SDVO_MAX_RETURNS        8

#define SDVO_REG_ARG_START      0x07
#define SDVO_REG_ARG_END        0x00
#define SDVO_REG_OPCODE         0x08
#define SDVO_REG_STATUS         0x09
#define SDVO_REG_RETURN_START   0x0A
#define SDVO_REG_RETURN_END     0x11

#define SDVO_MAX_RETRIES        3

/*  ........................................................................ */
/*  Structure definitions for Commands */

typedef unsigned char i2c_reg_t;

/* SDVOOutputFlagStructure Byte 0 */
typedef struct {
    i2c_reg_t tmds  : 1;
    i2c_reg_t rgb   : 1;
    i2c_reg_t cvbs  : 1;
    i2c_reg_t svid  : 1;
    i2c_reg_t yprpb : 1;
    i2c_reg_t scart : 1;
    i2c_reg_t lvds  : 1;
    i2c_reg_t drgb  : 1;
} sdvo_display_output_t;

/* Display masks */
#define FP_DISP_MASK (BIT(0)|BIT(6)|BIT(7)|BIT(8)|BIT(14)) /*TMDS/LVDS/DRGB*/
#define TV_DISP_MASK (BIT(2)|BIT(3)|BIT(4)|BIT(5)) /*CVBS0/SVID0/YPRPB0/SCART0*/
#define CRT_EXT_DISP_MASK (BIT(1)|BIT(9))   /* External CRT */
#define LVDS_DISP_MASK    (BIT(6)|BIT(14))  /* LVDS */
#define TMDS_DISP_MASK (BIT(0)|BIT(8))
#define TV_SVIDEO_DISP_MASK (BIT(3))   /* S-VIDEO */
#define TV_YPBPR_DISP_MASK (BIT(4))    /* YPBPR/Yipper */
#define DRGB_DISP_MASK  (BIT(7))       /* DRGB0 */
typedef struct {
	union {
		struct {
			sdvo_display_output_t out0;
			sdvo_display_output_t out1;
		};
		unsigned short flags;
	};
} sdvo_output_flags_t;

typedef struct {
    i2c_reg_t vendor_id;         /* 00: Vendor ID */
    i2c_reg_t device_id;         /* 01: Device ID */
    i2c_reg_t revision_id;       /* 02: Revision ID */
    i2c_reg_t version_minor;     /*  03 : sDVO Minor Version (00) */
    i2c_reg_t version_major;     /*  04 : sDVO Major Version (01) */
                                 /*  05 : Device capabilities */
    i2c_reg_t num_inputs    :2;  /*  10 : Number of sDVO inputs (1 or 2) */
    i2c_reg_t smooth_scaling:1;  /*   2 : Smooth (Graphics) Scaling */
    i2c_reg_t sharp_scaling :1;  /*   3 : Sharp (Text) scaling */
    i2c_reg_t up_scaling    :1;  /*   4 : Up Scaling */
    i2c_reg_t down_scaling  :1;  /*   5 : Down Scaling */
    i2c_reg_t stall         :1;  /*   6 : Stall */
    i2c_reg_t reserved      :1;  /*   7 : Reserved */
    sdvo_output_flags_t output;  /* 06&07: Ouput supported by sDVO device */
} sdvo_device_capabilities_t;

typedef struct {
    unsigned short pixel_clock;         /*  Pixel clock / 10,000 */
    unsigned short active_horz_pixels;  /*  12 bits of horizontal active */
    unsigned short active_vert_pixels;  /*  12 bits of vertical active */
    unsigned char  b_interlaced :1;     /*  1:Interlaced, 0: Non-Interlaced */
    unsigned char  b_scaled     :1;     /*  1:Image to be scaled, 0:Otherwise*/
    unsigned char  resv_bits    :6;     /*  Reserved bits */
    unsigned char  resv_byte;
} sdvo_create_preferred_timings_t;

typedef enum {
    CRM_1X = BIT(0),
    CRM_2X = BIT(1),
    CRM_4X = BIT(3),
} sdvo_clock_rate_mult_t;

/*  Detailed Timing Descriptor */
typedef struct {
    unsigned short pixel_clock;      /*  Pixel Clock / 10,000 */
    unsigned short horz_active;      /*  Horizontal Active */
    unsigned short horz_blanking;    /*  Horizontal Blanking */
    unsigned short vert_active;      /*  Vertical Active */
    unsigned short vert_blanking;    /*  Vertical Blanking */
    unsigned short horz_sync_offset;  /*  Pixels from blanking start */
    unsigned short horz_sync_pulse_width;
    unsigned char  vert_sync_offset;
    unsigned char  vert_sync_pulse_width;
    unsigned char  dtd_flags;
    unsigned char  sdvo_flags;
} sdvo_dtd_t;

typedef enum {
    SDVO0,
    SDVO1,
} sdvo_target_input_t;

typedef struct {
    i2c_reg_t dev_D0 :1; /*  SDVO On */
    i2c_reg_t dev_D1 :1;
    i2c_reg_t dev_D2 :1;
    i2c_reg_t dev_D3 :1; /*  SDVO Off */
    i2c_reg_t disp_D0:1; /*  Display On      : Vsync/Hsync/Video Active */
    i2c_reg_t disp_D1:1; /*  Display Standby : Vsync Active, Hsync/Video Off */
    i2c_reg_t disp_D2:1; /*  Display Suspend : Hsync Active, Vsync/Video Off */
    i2c_reg_t disp_D3:1; /*  Display Off     : All signals Off */
} sdvo_power_states_t;

typedef struct {
	struct{
		i2c_reg_t minor_DVI_rev :4;
		i2c_reg_t major_DVI_rev :4;
	};
	struct{
		i2c_reg_t minor_HDMI_rev:4;
		i2c_reg_t major_HDMI_rev:4;
	};

    i2c_reg_t reserved[6]; 		/* reserved for future use */
} sdvo_digital_encode_modes_t;

typedef struct{
	sdvo_digital_encode_modes_t	version;
}sdvo_hdmi_context_t;


typedef struct _sdvo_device_context {
    sdvo_target_input_t        inp_dev;
    sdvo_output_flags_t        out_type;
	sdvo_device_capabilities_t dev_cap;
    pd_callback_t             *p_callback;
	pd_attr_t                 *p_attr_table;
	unsigned long              num_attrs;
	pd_timing_t               *p_mode_table;
	pd_timing_t               *native_dtd;
	unsigned short             fp_width;
	unsigned short             fp_height;
	unsigned char              supp_pwr_states;
	unsigned char              device_pwr_state, display_pwr_state;
	unsigned char              up_scaling;
	unsigned short             t1;   /* power state delays */
	unsigned short             t2;   /* power state delays */
	unsigned short             t3;   /* power state delays */
	unsigned short             t4;
	unsigned short             t5;
	unsigned short             text_tune;
	sdvo_hdmi_context_t		   hdmi;
	unsigned short             st_sdvo;
} sdvo_device_context_t;

typedef struct sdvo_state {
	unsigned long power_state;
	pd_timing_t timing;
} sdvo_state_t;

/*  ......................................................................... */
sdvo_status_t sdvo_execute_command(sdvo_device_context_t *p_ctx,
	sdvo_opcode_t opcode,
	i2c_reg_t num_args, i2c_reg_t *p_arg,
	i2c_reg_t num_returns, i2c_reg_t *p_ret_value);

sdvo_status_t sdvo_reset(sdvo_device_context_t *p_ctx);
sdvo_status_t sdvo_get_device_capabilities(sdvo_device_context_t *p_ctx,
	sdvo_device_capabilities_t *p_Dev_Cap);
sdvo_status_t sdvo_get_trained_inputs(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Out);
sdvo_status_t sdvo_get_active_outputs(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t *p_Out_Flags);
sdvo_status_t sdvo_set_active_outputs(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags);

sdvo_status_t sdvo_get_in_out_map(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags[2]);
sdvo_status_t sdvo_set_in_out_map(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags[2]);
sdvo_status_t sdvo_set_upscalar_coefficient(sdvo_device_context_t *p_ctx,
	i2c_reg_t upscalar_coefficient);

sdvo_status_t sdvo_get_attached_displays(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t *p_Out_Flags);

sdvo_status_t sdvo_set_target_input(sdvo_device_context_t *p_ctx,
	sdvo_target_input_t input);
sdvo_status_t sdvo_set_target_output(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags);

sdvo_status_t sdvo_get_input_timings(sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd);
sdvo_status_t sdvo_set_input_timings(sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd);

sdvo_status_t sdvo_get_output_timings(sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd);
sdvo_status_t sdvo_set_output_timings(sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd);

sdvo_status_t sdvo_get_input_pixel_clock_range(sdvo_device_context_t *p_ctx,
	unsigned short p_Pixel[2]);

sdvo_status_t sdvo_get_preferred_input_timings(sdvo_device_context_t *p_ctx,
	sdvo_create_preferred_timings_t *p_Timings,
	sdvo_dtd_t *p_dtd);
sdvo_status_t sdvo_get_clock_rate_multiplier(sdvo_device_context_t *p_ctx,
	sdvo_clock_rate_mult_t *p_mult);
sdvo_status_t sdvo_set_clock_rate_multiplier(sdvo_device_context_t *p_ctx,
	sdvo_clock_rate_mult_t mult);

sdvo_status_t sdvo_get_supported_power_states(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_pwr_state);
sdvo_status_t sdvo_get_power_state(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Power_State);
sdvo_status_t sdvo_set_power_state(sdvo_device_context_t *p_ctx,
	i2c_reg_t pwr_state);
sdvo_status_t sdvo_set_display_power_state(sdvo_device_context_t *p_ctx,
	i2c_reg_t display_state);
#if !defined(CONFIG_MICRO) /* These are not used in vBIOS */
sdvo_status_t sdvo_set_digital_encoding_mode(sdvo_device_context_t *p_ctx,
	i2c_reg_t digital_encoding_mode);
sdvo_status_t sdvo_get_hdmi_audio_transmission_char(
	sdvo_device_context_t *p_ctx, i2c_reg_t *hdmi_reg);
sdvo_status_t sdvo_get_supported_encoding_modes(sdvo_device_context_t *p_ctx,
    sdvo_digital_encode_modes_t *digital_encoding_modes);
sdvo_status_t sdvo_get_digital_encoding_mode(sdvo_device_context_t *p_ctx,
	i2c_reg_t *digital_encoding_mode);
/* ------------------------------------------------------------------------- */
#endif
sdvo_status_t sdvo_get_min_max_pixel_clock(sdvo_device_context_t *p_ctx,
	unsigned long *p_min_clock, unsigned long *p_max_clock);

sdvo_status_t sdvo_get_tv_output_format(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_TV_Output_Format_Flag);

sdvo_status_t sdvo_get_sdtv_resolution_support(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Requested_SDTV_Format_Flag, i2c_reg_t *p_Resolution_Support_Flags);

sdvo_status_t sdvo_get_scaled_hdtv_resolution_support(
	sdvo_device_context_t *p_ctx, i2c_reg_t *p_Requested_HDTV_Format_Flag,
	i2c_reg_t *p_Resolution_Support_Flags);

sdvo_status_t sdvo_execute_command_read(sdvo_device_context_t *p_ctx,
	i2c_reg_t num_returns, i2c_reg_t *p_ret_value);

#endif  /*  _SDVO_INTF_H_ */


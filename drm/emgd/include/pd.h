/*
 *-----------------------------------------------------------------------------
 * Filename: pd.h
 * $Revision: 1.12 $
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

#ifndef _PD_H_
#define _PD_H_

#include <igd_pd.h>
#include <igd_debug.h>

/* PD SDK version 2 bytes,
 *    1st byte is major version,
 *    second byte is minor version.
 */

/* Update the version number each time there is an
 * API Change or Change in data structures
 */
#define PD_SDK_VERSION    0x0300             /* PD SDK Version */

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

/* This needs to contain the unsigned long for vBIOS. */
#ifndef BIT
#define BIT(x) (1L<<x)
#endif

/* Macro to return:
 *    minimum, if the value < min value
 *    maximum, if the value > max value
 *    value, otherwise
 */
#define MINMAX(val, min, max) ((val < min) ? min : ((val > max) ? max : val))

#define PD_REG_LIST_END      0xFFFFFFFF
#define PD_DAB_LIST_END      0xFFFFFFFF
#define PD_TIMING_LIST_END   0xFFFF

/* Error codes used for return values */
#define  PD_SUCCESS           0x00    /* Success */
#define  PD_ERR_NOMEM         0x01    /* Unable to allocated requested mem */
#define  PD_ERR_NODEV         0x02    /* No Device/Codec found */
#define  PD_ERR_NODIS         0x03    /* No display found */
#define  PD_ERR_INVALID_PTR   0x04    /* Invalid pointer passed */
#define  PD_ERR_NULL_PTR      0x05    /* Null ptr passed */
#define  PD_ERR_DISPLAY_TYPE  0x06    /* Unsupported display type specified */
#define  PD_ERR_NOPORT_AVAIL  0x07    /* No port available */
#define  PD_ERR_INTERNAL      0x08    /* Critical internal error */
#define  PD_ERR_INVALID_POWER 0x09    /* Invalid power state specified */
#define  PD_ERR_HAND_SHAKE    0x0A    /* Magic cookie hand shake failed */
#define  PD_ERR_MODE_NOTSUPP  0x0B    /* Unsupported mode */
#define  PD_ERR_I2C_READ      0x0C    /* I2C read error */
#define  PD_ERR_I2C_WRITE     0x0D    /* I2C write error */
#define  PD_ERR_NULL_STATE    0x0E    /* Saved state is NULL */
/* PD_ERR_NOATTR removed. Return success with 0 count instead */
#define  PD_ERR_NO_TIMINGS    0x10    /* No timing list */
#define  PD_ERR_INVALID_ATTR  0x11    /* Invalid attribute */
#define  PD_ERR_INCORR_ATTR_VALUE 0x12 /* Incorrect attr value */
#define  PD_ERR_ATTR_CANT_CHANGE  0x13 /* Attribute value cannot be changed */
#define  PD_ERR_VER_MISMATCH      0x14 /* PD SDK version mismatch */
#define  PD_ERR_UNSUCCESSFUL      0x15 /* Operation unsuccessful */

typedef struct _pd_reg {
	unsigned long reg;
	unsigned long value;
}pd_reg_t;


/* The following structures holds CEA EDID Extension data
 * Edid and possibly display id would use this structure as
 * well. PD callback would have a callback function to one the
 * structure to retrieve EDID like data (ELD)
 */


#define CEA_IEEE_HDMI_ID	0x000C03
/* Aspect Ratio */
#define PD_ASPECT_RATIO_NO_DATA 0
#define PD_ASPECT_RATIO_4_3 	1
#define PD_ASPECT_RATIO_16_9	2
/* ELD status */
#define ELD_AVAIL	0x01
#define ELD_CANNED	0x02
/* Transmitter audio charactheristic */
#define PD_AUDIO_CHAR_AVAIL	0x10
/* CEA data block type */
#define CEA_VIDEO_DATA_BLOCK	0x40
#define CEA_AUDIO_DATA_BLOCK	0x20
#define CEA_VENDOR_DATA_BLOCK	0x60
#define CEA_SPEAKER_DATA_BLOCK	0x80
/* Audio block Tag Code */
#define CEA_AUDIO_LPCM		1
#define CEA_AUDIO_AC3		2
#define CEA_AUDIO_MPG1		3
#define CEA_AUDIO_MP3		4
#define CEA_AUDIO_MPG2		5
#define CEA_AUDIO_AAC		6
#define CEA_AUDIO_DTS		7
#define CEA_AUDIO_ATRAC		8
#define CEA_AUDIO_SACD		9
#define CEA_AUDIO_DD_PLUS	10
#define CEA_AUDIO_DTS_HD	11
#define CEA_AUDIO_MLP		12
/* Pixel Replication */
#define PIX_REPLICATION_0	1
#define PIX_REPLICATION_1	2
#define PIX_REPLICATION_3	4
/* Quantization */
#define HDMI_QUANTIZATION_RGB_256	0x00
#define HDMI_QUANTIZATION_RGB_220	0x01
#define HDMI_QUANTIZATION_YUV_422	0x02
#define HDMI_QUANTIZATION_YUV_44	0x03
/* Colorimetry */
#define HDMI_COLORIMETRY_NODATA		0x00
#define HDMI_COLORIMETRY_ITU601		0x01 /* SMPTE 170M, ITU601 */
#define HDMI_COLORIMETRY_ITU709		0x02
/* Audio CAP(48,96,192)Khz refer to audio_cap in CEA ELD extension*/
#define CAP_48_KHZ	0
#define CAP_96_KHZ	1
#define CAP_192_KHZ	2
/* Vendor Specific Data Block */
#define VSBD_LATENCY_FIELD			8
typedef struct _cea_audio_format_t{
	union{
		unsigned char byte1;
		struct{
			unsigned char max_channels		: 3;
			unsigned char audio_format_code : 4;
			unsigned char reserve_byte1		: 1;
		};
	};
	union{
		unsigned char byte2;
		struct{
			unsigned char _32khz			: 1;
			unsigned char _44khz 			: 1;
			unsigned char _48khz			: 1;
			unsigned char _88khz			: 1;
			unsigned char _96khz			: 1;
			unsigned char _176khz			: 1;
			unsigned char _192khz			: 1;
			unsigned char reserve_byte2		: 1;
		};
	};
	/* 3rd byte differs between compressed & uncompressed audio */
	union{
		unsigned char max_bitrate;
		struct{
			unsigned char _16bit		: 1;
			unsigned char _20bit		: 1;
			unsigned char _24bit		: 1;
			unsigned char reserve_byte3	: 5;
		};
	};

}cea_audio_format_t;

typedef struct _speaker_allocation_data_t{
	struct{
		unsigned char	rsvd	: 1;
		unsigned char	rlc_rrc : 1;
		unsigned char	flc_frc : 1;
		unsigned char	rc		: 1;
		unsigned char	rl_rr	: 1;
		unsigned char	fc		: 1;
		unsigned char	lfe		: 1;
		unsigned char	fl_fr	: 1;
	};
	unsigned char reserved[2];
}speaker_allocation_data_t;

typedef union {
	unsigned char value;
	struct {
		unsigned char code: 7;
		unsigned char native: 1;
	};
} cea_video_blk_t;

typedef struct _audio_capability_t{
	unsigned char max_channels;
	unsigned char _20bit;
	unsigned char _24bit;
}audio_capability_t;

typedef struct _vsdb_t{
	union{
		unsigned char header;
		struct{
			unsigned char vendor_block_size: 5;
			unsigned char tag : 3;
		};
	};
	unsigned char	vendor_ieee_id[3];
	unsigned char	src_phy_add[2];
	unsigned char	support_ai;
	unsigned char	max_tmds_clck;
	union{
		unsigned char latency_fields;
		struct{
			unsigned char reserve	: 6;
			unsigned char i_latency	: 1;
			unsigned char p_latency	: 1;
		};
	};
	unsigned char p_video_latency;
	unsigned char p_audio_latency;
	unsigned char i_video_latency;
	unsigned char i_audio_latency;

}vsdb_t;

typedef struct _cea_extension{
	unsigned char	rev_number;
	unsigned char	canned_eld;
	union{
		unsigned char	caps;
		struct{
			unsigned char	total_native_format : 4;
			unsigned char	YCC_422				: 1;
			unsigned char	YCC_444				: 1;
			unsigned char	audio_support		: 1;
			unsigned char	underscan_support	: 1;
		};
	};

	/* Short Video Descriptor */
	int						total_short_video_desc;
	cea_video_blk_t			*short_video_desc;
	/* Short Audio Descriptor */
	int						total_short_audio_desc;
	cea_audio_format_t		*short_audio_desc;
	/* Vendor Descriptor Block */
	unsigned char			*vendor_data_block;

	unsigned char		audio_flag;		/* Define is ELD status */
	unsigned char		NPL;
	unsigned char		K0;
	unsigned char		K1;
	audio_capability_t	audio_cap[3];	/* Panel audio capability (48,96,192)Khz*/

	/* AVI Info frames data */
	int					pixel_rep;		/* Pixel replication */
	int					quantization;	/* Quantization */
	int					aspect_ratio;	/* Aspect Ratio */
	int					colorimetry;	/* Colorimetry */
	int					video_code;
	/* Consolidate ELD information here */
	union{
		unsigned char eld_ptr[256];
		struct {
			union {
				unsigned char version;
				struct {
					unsigned char cea_ver: 3;
					unsigned char eld_ver: 5;
				};
			};
			union {
				unsigned char capability;
				struct {
					unsigned char hdcp: 1;
					unsigned char repeater: 1;
					unsigned char _44ms: 1;
					unsigned char reserved1: 5;
				};
			};
			union {
				unsigned short length;
				struct {
					unsigned short mnl: 3;
					unsigned short vsdbl: 3;
					unsigned short sadc: 4;
					unsigned short reserved2: 6;
				};
			};
			unsigned short	manu_id;
			unsigned short	prod_id;
			unsigned char	LPCM_CAD[3];	/* LPCM for ELD */
			unsigned char	speaker_alloc_block[3];
			vsdb_t			vendor_block;	/* 13 byte */
			unsigned char	misc_data[229];
		};
	};
}cea_extension_t;

/* Following are the callback functions provided to port driver. It also
 * provides a call back context. Every time port driver has to pass this
 * callback context when calling a callback function.
 *
 * read_regs:     Port driver has to pass a list of pd_reg_t's which
 *                it want to read. The list has to end with PD_I2C_LIST_END.
 *                The values read will be set to the for every list->reg,
 *                value will be set in list->value.
 *
 *                Return value == 0 on success
 *                             != 0 on failure
 *
 * write_regs:    Port driver has to pass a list of pd_reg_t's which
 *                it want to write. The list has to end with PD_LIST_END.
 *                Caller has to provide already masked values. This callback
 *                function doesn't do any masking. If, port driver wants to
 *                write only some bits of a reg, first it has to read the
 *                register and then set bits and write the register.
 *
 *                Return value == 0 on success
 *                             != 0 on failure
 */

#define PD_REG_I2C    0x00000001
#define PD_REG_LPC    0x00000002
#define PD_REG_DMA    0x00000003
#define PD_REG_PCI    0x00000004
#define PD_REG_MIO    0x00000005   /* MMIO 32 bits    */
#define PD_REG_PIO8   0x00000006   /* Port IO 1 byte */
#define PD_REG_PIO16  0x00000007   /* Port IO 2 bytes */
#define PD_REG_PIO32  0x00000008   /* Port IO 4 bytes */
#define PD_REG_MIO8   0x00000009   /* MMIO 8 bits     */
#define PD_REG_BRIDGE_OPCODE  0x0000000D /* Host bridge read and write */
#define PD_REG_DDC            0x0000000E /* Read the ddc */
#define PD_REG_DDC_FW            0x0000000F /* Read the ddc */

typedef int (*pd_read_regs_p)(void *callback_context, pd_reg_t *list,
		unsigned long type);
typedef int (*pd_write_regs_p)(void *callback_context, pd_reg_t *list,
		unsigned long type);

typedef struct _pd_callback {
	void *callback_context;
	pd_read_regs_p  read_regs;
	pd_write_regs_p write_regs;
	unsigned long port_num;			/*	Added for SDVO port driver */
	cea_extension_t	**eld;			/* EDID like Data */
	unsigned char reset;
}pd_callback_t;

/* IMP NOTE: All below structures should be with same size.
 *         pd_attr_t            : General attribute structure
 *         pd_range_attr_t      : Range type attribute structure
 *         pd_list_attr_t       : List type attribute
 *         pd_list_entry_attr_t : Entry for a list
 *         pd_bool_attr_t       : Boolean type attribute
 *         pd_buffer_attr_t     : Buffer type attribute
 */
typedef igd_attr_t            pd_attr_t;
typedef igd_range_attr_t      pd_range_attr_t;
typedef igd_list_attr_t       pd_list_attr_t;
typedef igd_list_entry_attr_t pd_list_entry_attr_t;
typedef igd_bool_attr_t       pd_bool_attr_t;
typedef igd_buffer_attr_t     pd_buffer_attr_t;

#define ATTR(a)   ((pd_attr_t *)a)               /* General attr */
#define RATTR(a)  ((pd_range_attr_t *)a)         /* Range attr */
#define LHATTR(a) ((pd_list_attr_t *)a)          /* List head attr */
#define LEATTR(a) ((pd_list_entry_attr_t *)a)    /* List entry attr */
#define BATTR(a)  ((pd_bool_attr_t *)a)          /* Bool attr */
#define BUATTR(a) ((pd_buffer_attr_t *)a)        /* Buffer attr */

/* Timing structure flag defines */
#define PD_SCAN_INTERLACE        0x80000000
#define PD_LINE_DOUBLE           0x40000000
#define PD_PIXEL_DOUBLE          0x20000000
#define PD_MODE_TEXT             0x10000000  /* VGA Text mode */

#define PD_HSYNC_HIGH            0x08000000
#define PD_VSYNC_HIGH            0x04000000
#define PD_BLANK_LOW             0x02000000
#define PD_MODE_VESA             0x01000000 /* VGA/VESA mode number is valid */

#define PD_MODE_STALL            0x00800000   /* Flag to enable stall signal */
#define PD_MODE_SCALE            0x00400000   /* Request NATIVE timings */

#define PD_ASPECT_16_9           0x00200000   /* 16:9 aspect ratio, otherwise it is 4:3 */
#define PD_MODE_CEA				 0x00100000

#define PD_MODE_DTD              0x00080000   /* Read from EDID */
#define PD_MODE_DTD_USER         0x00040000   /* User defined timing */
#define PD_MODE_DTD_FP_NATIVE    0x00020000   /* Native fp timing */
#define PD_MODE_SUPPORTED        0x00010000

#define PD_MODE_FACTORY          0x00008000   /* Factory supported mode */
#define PD_MODE_RB               0x00004000   /* Reduced blanking mode */

/* Macro to put at the end timing table */
#define  PD_TIMING_TABLE_END \
{\
	PD_TIMING_LIST_END, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
	NULL, 0, 0\
}\

/* Timing structure */
typedef struct _pd_timing {
	unsigned short width;           /* width */
	unsigned short height;          /* height */
	unsigned short refresh;         /* refresh rate */
	unsigned long  dclk;            /* refresh rate dot clock in kHz */
	unsigned short htotal;          /* horizontal total */
	unsigned short hblank_start;    /* horizontal blank start */
	unsigned short hblank_end;      /* horizontal blank end */
	unsigned short hsync_start;     /* horizontal sync start */
	unsigned short hsync_end;       /* horizontal sync end */
	unsigned short vtotal;          /* vertical total */
	unsigned short vblank_start;    /* vertical blank start */
	unsigned short vblank_end;      /* vertical blank end */
	unsigned short vsync_start;     /* vertical sync start */
	unsigned short vsync_end;       /* vertical sync end */
	short mode_number; /* VGA or VESA mode number */
	unsigned long mode_info_flags; /* Valid Flags
									   - PD_SCAN_INTERLACE
									   - PD_LINE_DOUBLE
									   - PD_PIXEL_DOUBLE
									   - PD_HSYNC_HIGH
									   - PD_VSYNC_HIGH
									   - PD_MODE_SUPPORTED
									   - PD_MODE_DTD */
	unsigned short x_offset;
	unsigned short y_offset;
	void *extn_ptr;  /* INTERNAL pointer for use by main driver only */
	unsigned short reserved_dd; /* Reserved for device dependant layer */
	unsigned short reserved_dd_ext; /* Reserved for device dependant layer */
} pd_timing_t;

/* Bit field flags for pd_driver_t->driver_flags */
#define PD_FLAG_GANG_MODE       0x00000001   /* driver is running in gang mode*/
#define PD_FLAG_CLK_SOURCE      0x00000002   /* clk source is port driver */
#define PD_FLAG_PIPE_MASTER     0x00000004   /* master port driver */
#define PD_FLAG_REV_DATA_ORDER  0x00000008   /* reverse data order requested */
#define PD_FLAG_I740_DATA_ORDER 0x00000010   /* For I740 Data ordering. If this
												flag isn't set, then it is
												Flat panel data ordering */
#define PD_FLAG_DUAL_DVO        0x00000020   /* Same port driver will be loaded
											  * on both DVOB & DVOC with same
											  * DAB */
#define PD_FLAG_GANG_MODE_EVEN_ODD 0x00000040   /* pd wants gang mode in even
												 * pixels on one DVO port &
												 * odd pixels on other DVO port.
												 * Default is upper half pixel
												 * on one DVO port and lower
												 * half on the other DVO port.
												 */
#define PD_FLAG_UP_SCALING          0x00000080 /* pd supports up-scaling */
#define PD_FLAG_DOWN_SCALING        0x00000100 /* pd supports down-scaling */
#define PD_FLAG_CLOCK_MASTER        0x00000200 /* clock master port driver */
#define PD_FLAG_GANG_MODE_DVOCLKINV 0x00000400 /* GangMode DVO Clk inversion */
#define PD_FLAG_NO_VGA_2X_IMAGE     0x00000800 /* Gang Mode operation might
												* request this flag */

/* Flag for set_mode function */
/* Though these are bit fields, both cannot be used at same time */
#define PD_SET_MODE_PIPE_A    0x1
#define PD_SET_MODE_PIPE_B    0x2
#define PD_SET_MODE_FLAG_TEST 0x4

/* Flags for save and restore */
#define PD_NO_RESTORE_FREE_STATE 0x1

/* Power Modes */
#define PD_POWER_MODE_D0  0x0
#define PD_POWER_MODE_D1  0x1
#define PD_POWER_MODE_D2  0x2
#define PD_POWER_MODE_D3  0x3

#define PD_FILE_SIZE       8   /* File name size for port driver excluding
								* extension. Extension can be max of 3 chars. */

/* Following is the abstraction of port driver. Port driver passes this
 * driver information to display driver as part of registering itself with
 * display driver. pd_register(pd_driver_t *) is called from port driver
 * in its init function which is called whenever port driver is loaded.
 *
 * Port driver provides following list of functions to display driver.
 * Display driver calls these functions to do the required operations.
 *
 * type         - supporting display type for the port driver
 *
 * flags        - flags for the driver. And cannot be changed at runtime.
 *                Once these flags are passed to main driver, they will be
 *                used while setting the mode.
 *
 * context      - Driver's contextual information. The driver can save
 *                all needed state information in this opaque pointer
 *                such that the driver is entirely reentrant. No writeable
 *                data may be saved in any non-stack variables by a driver
 *                therefore all necessary data must be saved in a private
 *                structure and stored in this pointer. This pointer is
 *                passed to the driver with each function call.
 *
 * validate     - Display driver calls this function to validate the port
 *                driver by passing a magic cookie. Port driver does some
 *                magic operation and returns another cookie back to caller.
 *                If the returned cookie has correct value then the driver
 *                passes the validation, otherwise driver cannot communicate
 *                with display driver.
 *
 * open         - This function is to detect for any port device (codec).
 *                pd_callback_t has all the required callback functions
 *                to do this operation.
 *
 * init_device  - This function detects and initializes any display devices
 *                attached to the codec.
 *
 *                For ex: a multi function encoder can support a FlatPanel or
 *                TVout display. It has to detect and check for any
 *                Flatpanel or TVout display device attached, then it
 *                initializes the display devices.
 *
 * pd_close     - closes the display device and frees any memory allocated.
 *
 * set_mode     - sets/tests a mode. Display (or main) driver calls this
 *                function to set a mode when the encoder in D3 power state.
 *                At this time, DVO timings are off.
 *                If encoder wants to run setmode in any other power state,
 *                then first thing it needs to do is enter into that power
 *                state and then do setmode.
 *
 *                When this function returns it is expected that:
 *                   1. Either encoder is in D0 power state or
 *                      it defers until post_set_mode().
 *                   2. In case of external encoder (example: TVOut encoder)
 *                      driving the reference clock, the clock is running.
 *
 * post_set_mode- This is called after GMCH DVO timings are on.
 *                This function can be null if there is nothing to do.
 *                This is used to do any post processing on the encoder
 *                after GMCH starts driving the timings. When this function
 *                returns it is expected that the encoder is in D0 power state.
 *                Parameters:
 *                       context - port driver context
 *                       mode    - current timing
 *                       flags   - not used (for future use).
 *
 * set_attrs    - to set provided list of attributes.
 *
 * get_attrs    - to get list of attributes.
 *
 * get_timing_list - returns the list of supported modes for the current
 *                   attributes. For example if the TVFormat is NTSC, this
 *                   function will return only the modes supported for NTSC.
 *
 * set_power    - to set the power state of the display device.
 *
 * get_power    - to get the current power state of the display device.
 *
 * pd_save      - to save the current state of the registers.
 *                  - returns the current state in the double pointer 'state'.
 *
 * pd_restore   - to restore to the passed state. This functions frees the
 *                resources allocated that state.
 *                  - pass the previously saved state in 'state'.
 *
 *                If PD_NO_RESTORE_FREE_STATE is set, then it just frees
 *                the resources without restoring the state.
 *
 * pd_get_port_status - to get the status of port/display
 *
 *
 * All functions return value:
 *         == 0 - if success
 *         != 0 - if failure
 */


/* Note on version AA.BB.CC.DD
 * AA - Major version
 * BB - Minor version
 * CC - Patch version if any
 * DD - Bug fixes if any
 *
 * 01.00.00.00 is the first initial major version */
typedef struct _pd_version {
	unsigned char major;
	unsigned char minor;
	unsigned char patch;
	unsigned char bug_fix;
} pd_version_t;


typedef enum {
	PD_DISP_STATUS_DETACHED = 0,        /* Display is not connected */
	PD_DISP_STATUS_ATTACHED,            /* Display is connected     */
	PD_DISP_STATUS_UNKNOWN              /* Unable to determine      */
} pd_display_status_t;

/* Port/Display information */
typedef struct _pd_port_status {
	pd_display_status_t connected;      /* Display status                */
	unsigned long display_type;         /* Type of display, if connected */
} pd_port_status_t;

typedef struct _pd_driver {
	unsigned long pd_sdk_version;        /* PD SDK version - interface ver */
	char          name[PD_NAME_SIZE];    /* Descriptive name of port driver */
	char          num_devices;           /* Number of devices it is driving */
	pd_version_t  *version;              /* Driver version */
	unsigned long type;
	unsigned long flags;
	unsigned long *dab_list;             /* PD_DAB_LIST_END terminated list */
	unsigned long i2c_speed;             /* Encoder I2C speed in KHz */
	unsigned long (*validate)(unsigned long cookie);
	int (*open)       (pd_callback_t *callback, void **context);
	int (*init_device)(void *context);
	int (*pd_close)   (void *context);
	int (*set_mode)   (void *context, pd_timing_t *mode, unsigned long flags);
	int (*post_set_mode)(void *context, pd_timing_t *mode, unsigned long flags);
	int (*set_attrs)  (void *context, unsigned long num, pd_attr_t *list);
	int (*get_attrs)  (void *context, unsigned long *num, pd_attr_t **list);
	int (*get_timing_list)(void*context,pd_timing_t*in_list,pd_timing_t**list);
	int (*set_power)  (void *context, unsigned long state);
	int (*get_power)  (void *context, unsigned long *state);
	int (*pd_save)    (void *context, void **state, unsigned long flags);
	int (*pd_restore) (void *context, void *state, unsigned long flags);
	int (*pd_get_port_status) (void *context, pd_port_status_t *port_status);
} pd_driver_t;

/* Mode filter helper function for port drivers */
/* Structure representing encoder capabilities for mode filtering */
typedef struct _pd_dvo_info {
	unsigned long min_dclk;               /* Min clock */
	unsigned long max_dclk;               /* Max clock */
	unsigned char upscale;                /* Is upscalable? */
	unsigned char downscale;              /* Is downscalable? */
	unsigned short upscale_min_width;     /* Min upscale width */
	unsigned short upscale_min_height;    /* Min upscale height */
	unsigned short downscale_max_width;   /* Max downscale width */
	unsigned short downscale_max_height;  /* Max downscale height */
} pd_dvo_info_t;

typedef struct _pd_display_info {
	unsigned char panel_fit;   /* Incoming */
	unsigned char fixed_res;   /* Outgoing */
	unsigned short width;      /* Outgoing */
	unsigned short height;     /* Outgoing */
	pd_timing_t   *native_dtd; /* Outgoing */
} pd_display_info_t;

/* All following functions are provided to port drivers as a binary file.
 *
 * pd_register: This function is called by port driver to register with
 *              display driver. It passes its driver structure to display
 *              driver.
 *
 * pd_malloc:   This function is used to allocate any dynamic memory required
 *              in port driver. Port driver will pass the size of the memory
 *              it is requesting in bytes.
 *
 *              Return value == pointer to the allocated memory on success
 *                           == NULL if failed to allocated the memory
 *
 * pd_memset:   This function is used to set the passed byte in memory
 *              for size bytes.
 *
 * pd_memcpy:   This function is used to copy size bytes from src pointer to
 *              dest pointer.
 *
 * free:        This function is used to free the memory previously allocated
 *              using malloc callback function.
 *
 * usleep:      This function is used to get any delay in port driver.
 *              'usec' is specified in micro seconds.
 *
 *              No return value.
 * ui_usleep:   This function is used to get any delay in port driver uniterrupted.
 *              'usec' is specified in micro seconds.
 *
 *              No return value.
 *
 * printf:      This function is used to do any debug prints for port driver.
 *              This has exactly the same syntax and usage of standard
 *              printf in 'C'.
 *
 * strcpy:      This function is used to copy src string into dest string.
 *              This has exactly the same syntax and usage of standard
 *              strcpy in 'C'.
 *
 * filter:      This function is to filter the incoming mode list based on
 *              dvo_info. Memory is allocated for outgoing list. No changes
 *              to in_list.
 */
int   pd_register(void *handle, pd_driver_t *driver);
void *pd_malloc(unsigned long size);
void *pd_memset(void *address, int c, unsigned long size);
void *pd_memcpy(void *dst, void *src, unsigned long size);
void  pd_free  (void *address);

void  pd_usleep(unsigned long usec);
void  pd_ui_usleep(unsigned long usec);
char *pd_strcpy(char *dest, char const *src);
int  *pd_printf(const char *format, ...);
int   pd_check_attr(pd_attr_t *curr, pd_attr_t *in);
/* Mode filter helper function for port drivers */
int   pd_filter_timings(void *context, pd_timing_t *inlist, pd_timing_t **olist,
	pd_dvo_info_t *dvo_info, pd_display_info_t *display_info);

/* pd_get_attr() :  To return the request attr from the list.
 * In case of 'attr_id' is a list type attribute, then caller
 * can request either LIST head itself or list entry for that
 * attribute. */
#define PD_GET_ATTR_LIST            0x0001
#define PD_GET_ATTR_LIST_ENTRY      0x0002
pd_attr_t *pd_get_attr(pd_attr_t *attr_list, unsigned long num_attrs,
		unsigned long attr_id, unsigned long flag);

/*
// Typedefinitions
typedef int        (*pd_register_p)  (void *handle, pd_driver_t *driver);
typedef void      *(*pd_malloc_p)(unsigned long size);
typedef void      *(*pd_memset_p)(void *address, int c, unsigned long size);
typedef void      *(*pd_memcpy_p)(void *dst, void *src, unsigned long size);
typedef void       (*pd_free_p)  (void *address);
typedef void       (*pd_usleep_p)(unsigned long usec);
typedef char      *(*pd_strcpy_p)(char *dest, char const *src);
typedef int       *(*pd_printf_p)(const char *format, ...);
typedef int        (*pd_check_attr_p)(pd_attr_t *curr, pd_attr_t *in);
typedef pd_attr_t *(*pd_get_attr_p)(pd_attr_t *attr_list,
		unsigned long num_attrs, unsigned long attr_id, unsigned long flag);
typedef emgd_debug_t *(*pd_get_emgd_debug_p)( void );
typedef unsigned long *(*pd_get_dropped_debug_messages_p)( void );
typedef void *(*pd_get_debug_log_mutex_p)( void );

// Mode filter helper function for port drivers
typedef int        (*pd_filter_timings_p)(void *context, pd_timing_t *inlist,
		pd_timing_t **olist, pd_dvo_info_t *dvo_info,
		pd_display_info_t *display_info);
*/
#endif /* _PD_H_ */


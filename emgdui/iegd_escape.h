/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: iegd_escape.h
 * $Revision: 1.23 $
 *-----------------------------------------------------------------------------
 * INTEL CONFIDENTIAL
 * Copyright (2002-2008) Intel Corporation All Rights Reserved.
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 *
 *
 *-----------------------------------------------------------------------------
 * Description:
 *  This file defines the API for communication with the EGD driver's
 *  proprietary escape interfaces. These interfaces provide a consistent
 *  mechanism to control features of EGD across multiple operating systems
 *  and implementations.
 *-----------------------------------------------------------------------------
 */

#ifndef _EGD_ESCAPE_H
#define _EGD_ESCAPE_H

#define MAX_NAME_LEN 256

#define INTEL_ESCAPE_NOT_SUPPORTED  -1
#define INTEL_ESCAPE_SUCCESS         0
#define INTEL_ESCAPE_ERROR           1
#define INTEL_ESCAPE_STATUS_NOERROR  0

#define INTEL_ESCAPE_NO_REPLY                0x01000 /* don't send reply */

#define INTEL_ESCAPE_GET_CURRENT_MODE        0x20000
#define INTEL_ESCAPE_GET_NUM_MODES           0x20001
#define INTEL_ESCAPE_GET_AVAIL_MODES         0x20002
#define INTEL_ESCAPE_ENABLE_PORT             0x20005
#define INTEL_ESCAPE_MULTI_DISPLAY           0x20006 /* deprecated */
#define INTEL_ESCAPE_GET_NUM_PD_ATTRIBUTES   0x20007
#define INTEL_ESCAPE_GET_AVAIL_PD_ATTRIBUTES 0x20008
#define INTEL_ESCAPE_SET_PD_ATTRIBUTES       0x20009
#define INTEL_ESCAPE_I2C_CONFIG              0x2000A /* Not support on X */
#define INTEL_ESCAPE_I2C_ACCESS              0x2000B /* Not support on X */
#define INTEL_ESCAPE_GET_NUM_DISPLAYS        0x2000C /* deprecated */
#define INTEL_ESCAPE_ENUM_DISPLAYS           0x2000D /* deprecated */
#define INTEL_ESCAPE_SET_ROTATION_FLIP       0x2000E
#define INTEL_ESCAPE_GET_ROTATION_FLIP       0x2000F
#define INTEL_ESCAPE_GET_DRIVER_INFO         0x20012
#define INTEL_ESCAPE_GET_PORT_INFO           0x20014
#define INTEL_ESCAPE_GET_CURRENT_DC          0x20015
#define INTEL_ESCAPE_GET_DC_LIST             0x20016
#define INTEL_ESCAPE_SET_DC                  0x20017
#define INTEL_ESCAPE_GET_NUM_DC              0x20018
#define INTEL_ESCAPE_GET_OVL_COLOR_PARAMS    0x20019
#define INTEL_ESCAPE_SET_OVL_COLOR_PARAMS    0x2001A
#define INTEL_ESCAPE_NOOP                    0x20020
#define INTEL_ESCAPE_NOOP_DRAWABLE          (0x20021 | INTEL_ESCAPE_NO_REPLY)
#define INTEL_ESCAPE_GET_DEBUG               0x20022
#define INTEL_ESCAPE_SET_DEBUG               0x20023
#define INTEL_ESCAPE_GET_EDID_INFO           0x20024 /* Num same as Windows */
#define INTEL_ESCAPE_QUERY_COMPOSITE		 0x20025
#define INTEL_ESCAPE_GET_NUM_SURFACE		 0x20026
#define INTEL_ESCAPE_GET_SURFACE_LIST		 0x20027
#define INTEL_ESCAPE_QUERY_GOLDEN_HTOTAL     0x20028

/* X driver specific commands */
#define INTEL_ESCAPE_OVERLAY                 0x20100
#define INTEL_ESCAPE_BLEND                   0x20101
#define INTEL_ESCAPE_GRAB_OVERLAY            0x20102
#define INTEL_ESCAPE_RELEASE_OVERLAY         0x20103
#define INTEL_ESCAPE_VIDEO_BLEND             0x20104

/* Direct Rendering Related Commands */
#define INTEL_ESCAPE_BUFFER_INFO             0x20201
#define INTEL_ESCAPE_ALLOC_SURFACE           0x20202
#define INTEL_ESCAPE_FREE                   (0x20203 | INTEL_ESCAPE_NO_REPLY)
#define INTEL_ESCAPE_SYNC                    0x20204
#define INTEL_ESCAPE_ALLOC_REGION            0x20205
#define INTEL_ESCAPE_EXEC                    0x20207
#define INTEL_ESCAPE_FILL                    0x20208
#define INTEL_ESCAPE_BLIT                    0x20209
#define INTEL_ESCAPE_BLIT_OFFSCREEN_TO_FB    0x2021A
#define INTEL_ESCAPE_SURFACE_FROM_PIXMAP     0x2021B
#define INTEL_ESCAPE_DRI                     0x2021C
#define INTEL_ESCAPE_CREATE_CONTEXT          0x2021D
#define INTEL_ESCAPE_DESTROY_CONTEXT        (0x2021E | INTEL_ESCAPE_NO_REPLY)
#define INTEL_ESCAPE_ALLOC_RANDEC			 0x2021F

/* CE specific commands */
/* FIXME - these maybe redundant?! see DRI escapes above! */
#define INTEL_ESCAPE_VIDEO_BUF_CREATE        0x20301
#define INTEL_ESCAPE_VIDEO_BUF_DESTROY       0x20302
#define INTEL_ESCAPE_VIDEO_BUF_EXECUTE       0x20303
#define INTEL_ESCAPE_CLIENTVIRT_TO_EGDSURF 0x20304
#define INTEL_ESCAPE_ALLOC_CONTEXT           0x20305
#define INTEL_ESCAPE_FREE_CONTEXT            0x20306
#define INTEL_ESCAPE_GET_CLIENT_DD_SURF      0x20307
#define INTEL_ESCAPE_QUEUE_SWAP_DDFLIP_SURF  0x20308


/* WinSys specific commands */
#define INTEL_ESCAPE_WINSYS                  0x20400
#define INTEL_ESCAPE_PRESENT                 0x20401

/* Video Decode Design For Test commands */
#define INTEL_ESCAPE_VIDEO_CRC_START         0x20500
#define INTEL_ESCAPE_VIDEO_CRC_STOP          0x20501
#define INTEL_ESCAPE_GET_CRC                 0x20502
#define INTEL_ESCAPE_GET_CRC_DEBUG_INFO      0x20503
#define INTEL_ESCAPE_GET_SURFACE_DATA	     0x20504
/* Video encode */
#define INTEL_ESCAPE_GET_FRAME_SKIP         	0x20601
#define INTEL_ESCAPE_REGISTER_SHARED_SURFACE 	0x20602

/* Video frames per second command */
#define INTEL_ESCAPE_GET_VID_RENDERING_INFO  0x20600


#define INTERLACE_FLAG                       0x80000000

#ifdef CONFIG_DEBUG
static __inline char *intel_esc_str(unsigned int esc)
{

	switch (esc) {
	case INTEL_ESCAPE_GET_CURRENT_MODE:
		return "INTEL_ESCAPE_GET_CURRENT_MODE";
	case INTEL_ESCAPE_GET_NUM_MODES:
		return "INTEL_ESCAPE_GET_NUM_MODES";
	case INTEL_ESCAPE_GET_AVAIL_MODES:
		return "INTEL_ESCAPE_GET_AVAIL_MODES";
	case INTEL_ESCAPE_ENABLE_PORT:
		return "INTEL_ESCAPE_ENABLE_PORT";
	case INTEL_ESCAPE_MULTI_DISPLAY:
		return "INTEL_ESCAPE_MULTI_DISPLAY";
	case INTEL_ESCAPE_GET_NUM_PD_ATTRIBUTES:
		return "INTEL_ESCAPE_GET_NUM_PD_ATTRIBUTES";
	case INTEL_ESCAPE_GET_AVAIL_PD_ATTRIBUTES:
		return "INTEL_ESCAPE_GET_AVAIL_PD_ATTRIBUTES";
	case INTEL_ESCAPE_SET_PD_ATTRIBUTES:
		return "INTEL_ESCAPE_SET_PD_ATTRIBUTES";
	case INTEL_ESCAPE_I2C_CONFIG:
		return "INTEL_ESCAPE_I2C_CONFIG";
	case INTEL_ESCAPE_I2C_ACCESS:
		return "INTEL_ESCAPE_I2C_ACCESS";
	case INTEL_ESCAPE_GET_NUM_DISPLAYS:
		return "INTEL_ESCAPE_GET_NUM_DISPLAYS";
	case INTEL_ESCAPE_ENUM_DISPLAYS:
		return "INTEL_ESCAPE_ENUM_DISPLAYS";
	case INTEL_ESCAPE_SET_ROTATION_FLIP:
		return "INTEL_ESCAPE_SET_ROTATION_FLIP";
	case INTEL_ESCAPE_GET_ROTATION_FLIP:
		return "INTEL_ESCAPE_GET_ROTATION_FLIP";
	case INTEL_ESCAPE_GET_DRIVER_INFO:
		return "INTEL_ESCAPE_GET_DRIVER_INFO";
	case INTEL_ESCAPE_GET_PORT_INFO:
		return "INTEL_ESCAPE_GET_PORT_INFO";
	case INTEL_ESCAPE_GET_CURRENT_DC:
		return "INTEL_ESCAPE_GET_CURRENT_DC";
	case INTEL_ESCAPE_GET_DC_LIST:
		return "INTEL_ESCAPE_GET_DC_LIST";
	case INTEL_ESCAPE_SET_DC:
		return "INTEL_ESCAPE_SET_DC";
	case INTEL_ESCAPE_GET_NUM_DC:
		return "INTEL_ESCAPE_GET_NUM_DC";
	case INTEL_ESCAPE_GET_OVL_COLOR_PARAMS:
		return "INTEL_ESCAPE_GET_OVL_COLOR_PARAMS";
	case INTEL_ESCAPE_SET_OVL_COLOR_PARAMS:
		return "INTEL_ESCAPE_SET_OVL_COLOR_PARAMS";
	case INTEL_ESCAPE_NOOP:
		return "INTEL_ESCAPE_NOOP";
	case INTEL_ESCAPE_NOOP_DRAWABLE:
		return "INTEL_ESCAPE_NOOP_DRAWABLE";
	case INTEL_ESCAPE_GET_DEBUG:
		return "INTEL_ESCAPE_GET_DEBUG";
	case INTEL_ESCAPE_SET_DEBUG:
		return "INTEL_ESCAPE_SET_DEBUG";
	case INTEL_ESCAPE_GET_EDID_INFO:
		return "INTEL_ESCAPE_GET_EDID_INFO";
	case INTEL_ESCAPE_OVERLAY:
		return "INTEL_ESCAPE_OVERLAY";
	case INTEL_ESCAPE_BLEND:
		return "INTEL_ESCAPE_BLEND";
	case INTEL_ESCAPE_VIDEO_BLEND:
	  	return "INTEL_ESCAPE_VIDEO_BLEND";
	case INTEL_ESCAPE_BUFFER_INFO:
		return "INTEL_ESCAPE_BUFFER_INFO";
	case INTEL_ESCAPE_ALLOC_SURFACE:
		return "INTEL_ESCAPE_ALLOC_SURFACE";
	case INTEL_ESCAPE_FREE:
		return "INTEL_ESCAPE_FREE";
	case INTEL_ESCAPE_SYNC:
		return "INTEL_ESCAPE_SYNC";
	case INTEL_ESCAPE_ALLOC_REGION:
		return "INTEL_ESCAPE_ALLOC_REGION";
	case INTEL_ESCAPE_EXEC:
		return "INTEL_ESCAPE_EXEC";
	case INTEL_ESCAPE_FILL:
		return "INTEL_ESCAPE_FILL";
	case INTEL_ESCAPE_BLIT:
		return "INTEL_ESCAPE_BLIT";
	case INTEL_ESCAPE_BLIT_OFFSCREEN_TO_FB:
		return "INTEL_ESCAPE_BLIT_OFFSCREEN_TO_FB";
	case INTEL_ESCAPE_SURFACE_FROM_PIXMAP:
		return "INTEL_ESCAPE_SURFACE_FROM_PIXMAP";
	case INTEL_ESCAPE_REGISTER_SHARED_SURFACE:
		return "INTEL_ESCAPE_REGISTER_SHARED_SURFACE";
	case INTEL_ESCAPE_VIDEO_CRC_START:
		return "INTEL_ESCAPE_VIDEO_CRC_START";
	case INTEL_ESCAPE_VIDEO_CRC_STOP:
		return "INTEL_ESCAPE_VIDEO_CRC_STOP";
	case INTEL_ESCAPE_GET_CRC:
		return "INTEL_ESCAPE_GET_CRC";
	case INTEL_ESCAPE_GET_CRC_DEBUG_INFO:
		return "INTEL_ESCAPE_GET_CRC_DEBUG_INFO";
	case INTEL_ESCAPE_GET_VID_RENDERING_INFO:
		return "INTEL_ESCAPE_GET_VID_RENDERING_INFO";
	case INTEL_ESCAPE_CREATE_CONTEXT:
		return "INTEL_ESCAPE_CREATE_CONTEXT";
	case INTEL_ESCAPE_DESTROY_CONTEXT:
		return "INTEL_ESCAPE_DESTROY_CONTEXT";
	case INTEL_ESCAPE_GET_NUM_SURFACE:
		return "INTEL_ESCAPE_GET_NUM_SURFACE";
	case INTEL_ESCAPE_GET_SURFACE_DATA:
		return "INTEL_ESCAPE_GET_SURFACE_DATA";
	default:
		return "INTEL_ESCAPE Invalid";
	}
}
#endif



/*
 * Data structures used by the escape I/O structures defined in the
 * section below
 */

/* Corners of a rectangle.  Should be identical to igd_rect_t */
typedef struct _iegd_rect {
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
} iegd_rect_t, *piegd_rect_t;


/* Coefficients for converting YUV to RGB, must be identical to
   igd_yuv_coeffs */
typedef struct _iegd_yuv_coeffs {
	char ry;
	char ru;
 	char rv;
 	char gy;
	char gu;
	char gv;
	char by;
	char bu;
	char bv;

 	short r_const;
 	short g_const;
 	short b_const;

 	unsigned char r_shift;
	unsigned char g_shift;
	unsigned char b_shift;
} iegd_yuv_coeffs, *piegd_yuv_coeffs;



/*
 * The following typedefs define the input and output structures
 * that are passed between the driver and the client.  These are
 * versioned, and must not be changed without incrementing the
 * protocal version number.
 */

typedef enum _mode_change_t {
	NO_MODE_SET_REQUIRED,
	MODE_SET_REQUIRED,
} mode_change_t;

typedef struct _iegd_esc_status {
	mode_change_t status;
} iegd_esc_status_t;

/* Port number input */
typedef struct _iegd_esc_port_in {
	unsigned long port_number;
} iegd_esc_port_in_t;

/* DC value input/output.  On output an array of this type may be returned */
typedef struct _iegd_esc_dc {
	unsigned long dc;
} iegd_esc_dc_t;

/* For the functions that return a count it will return this */
typedef struct _iegd_esc_count {
	unsigned long count;
} iegd_esc_count_t;


/* Port attributes */
typedef struct _iegd_esc_attr {
	unsigned long id;
	unsigned long type;
	char name[32];
	unsigned long flags;
	unsigned long default_value;
	unsigned long current_value;
	unsigned long _pad0;
	unsigned long _pad1;
	unsigned long _pad2;
} iegd_esc_attr_t;

/* Input to driver when setting a display mode */
typedef struct _iegd_esc_mode {
	unsigned long width;
	unsigned long height;
	unsigned long refresh;
	unsigned long depth;
	unsigned long port;  /* needed to independently set clone's mode */
} iegd_esc_mode_t;

/* Input to driver when querying mode table */
typedef struct _iegd_esc_mode_in {
	unsigned long port_number;
	unsigned long dc;
} iegd_esc_mode_in_t;

/* Input to driver to enable/disable a port */
typedef struct _iegd_esc_port_ctrl {
	int port;
	int enable;
} iegd_esc_port_ctrl_t;

/* Input to driver with a new attribute list */
typedef struct _iegd_esc_set_attr {
	unsigned long port_number;
	iegd_esc_attr_t attribute;
} iegd_esc_set_attr_t;


/* Input to driver with new DC, timings, and framebuffer config */
typedef struct _iegd_esc_set_dc {
	unsigned long dc;
	struct _timing {
		unsigned short width;
		unsigned short height;
		unsigned short refresh;
		unsigned long flags;		/* Default: 0x0 */
	} iegd_timings[2];
	struct _fb {
		unsigned short width;
		unsigned short height;
		unsigned short bit_depth;
	} iegd_fb_info[2];
} iegd_esc_set_dc_t;


/* Input to driver with surface information */
typedef struct _iegd_esc_surface_info {
	unsigned long screen;           /* Screen number */
	unsigned long surface_id;       /* Surface to put */
	unsigned long drawable_id;      /* drawable */
	unsigned short src_x;           /* source X */
	unsigned short src_y;           /* source Y */
	unsigned short src_w;           /* source width */
	unsigned short src_h;           /* source height */
	unsigned short dst_x;           /* destination X */
	unsigned short dst_y;           /* destination Y */
	unsigned short dst_w;           /* destination width */
	unsigned short dst_h;           /* destination height */
	unsigned long flags;            /* surface flags */
} iegd_esc_surface_info_t;

/* Input to driver with drawable info */
typedef struct _iegd_esc_drawable_info {
	unsigned long drawable_id;
} iegd_esc_drawable_info_t;

/* Input to driver with rotation/flip information */
typedef struct _iegd_esc_set_rotation_flip {
	unsigned long port_number;
	unsigned long rotation; /* rotation in degrees (0, 90, 180, 270) */
	unsigned long flip;     /* horizontal flip, zero or non-zero */
} iegd_esc_set_rotation_flip_t;

/* Output from driver with rotation/flip information */
typedef struct _iegd_esc_get_rotation_flip {
	unsigned long rotation; /* rotation in degrees (0, 90, 180, 270) */
	unsigned long flip;     /* horizontal flip, zero or non-zero */
} iegd_esc_get_rotation_flip_t;

/* Output from driver */
typedef struct _iegd_esc_driver_info {
	char name[MAX_NAME_LEN];       /* EGD Driver Name                */
	char chipset[MAX_NAME_LEN];    /* Chipset name                    */
	unsigned long major;           /* Major version                   */
	unsigned long minor;           /* Minor version                   */
	unsigned long build;           /* Build number                    */
	char date[MAX_NAME_LEN];       /* Date of build                   */
	unsigned short config_id;      /* Current PCF config id */
	unsigned long device_id;       /* PCI Device ID */
	unsigned long revision_id;     /* PCI Revision ID */
} iegd_esc_driver_info_t;

/* Output from driver with information about any port */
typedef struct _iegd_esc_port_info {
	unsigned long width;            /* Current mode: screen width     */
	unsigned long height;           /* Current mode: screen height    */
	unsigned long depth;            /* Current mode: screen depth     */
	unsigned long refresh;          /* Current mode: screen refresh   */
	unsigned long edid;             /* Edid enable, 0 or 1            */

	unsigned long enable;           /* Port is enabled or disabled    */
	unsigned long timing_owner;     /* Set if port owns timings       */

	char pd_name[MAX_NAME_LEN];     /* Port driver name see pd.h      */
	char user_alias[MAX_NAME_LEN];  /* User(via IAL reg/cfg file)port name */
	unsigned long pd_version;       /* Port driver version see pd.h   */
	unsigned long pd_type;          /* Port driver type see pd.h      */
	unsigned long display_id;       /* display currently associated with port*/
	unsigned long flags;			/* Display flag. Default: 0x0	  */
} iegd_esc_port_info_t;

/* Output from driver with current mode list */
typedef struct _iegd_esc_mode_list {
	unsigned short width;
	unsigned short height;
	unsigned short refresh;
	unsigned long dclk;              /* in KHz */
	unsigned short h_sync_start;
	unsigned short h_sync_length;
	unsigned short h_blank_start;
	unsigned short h_blank_length;
	unsigned short v_sync_start;
	unsigned short v_sync_length;
	unsigned short v_blank_start;
	unsigned short v_blank_length;
	unsigned short mode_number;
	unsigned long flags;
	/* Reserved for device dependant layer. Now used to store the HTotal Delta
	 * For B0 Workaround. */
	unsigned short reserved_dd; 
	unsigned short reserved_dd_ext; 
} iegd_esc_mode_list_t;

/* EDID info returned */
typedef struct _iegd_esc_edid_info {
	unsigned char edid[128];
} iegd_esc_edid_info_t;

/*
 * Get/set debug info for drivers with debugging enabled.
 */
typedef struct _iegd_esc_debug_info {
	unsigned long hal_flags;
	unsigned long ial_flags;
} iegd_esc_debug_info_t;


#define GAMMA_FLAG          0x1
#define BRIGHTNESS_FLAG     0x2
#define CONTRAST_FLAG       0x4
#define SATURATION_FLAG     0x8
#define COLORKEY_FLAG       0x20
#define OVL_COLOR_FLAG      0x10

/*
 * the following escape structure is for dynamically changing the overlay
 * color attributes or frame buffer gamma correction
 *  ** this backdoor is temporary and is for MPD only! **
 */
typedef struct _iegd_esc_color_params {
	unsigned char flag; /* 0x1 = gamma, 0x2 = brightness */
						/* 0x4 = contrast, 0x8 = saturation */
						/*0x10 = Overlay colors; if not set for FB */
	unsigned int gamma;
	unsigned int brightness;
	unsigned int contrast;
	unsigned int saturation;
	unsigned int colorkey;
} iegd_esc_color_params_t;

typedef struct _iegd_esc_i2c_config {
	char bus;  /* I2C bus */
	char dab;  /* 7 bit device address byte */
	unsigned long speed; /* i2c bus speed */
	unsigned long enable; /* Internal use only */
} iegd_esc_i2c_config_t;

#define INTEL_I2C_MODE_READ  1
#define INTEL_I2C_MODE_WRITE 2

typedef struct _iegd_esc_i2c_access {
	char addr; /* Register offset */
	char data; /* Data to write (or read from device) */
	unsigned long mode; /* read or write */
} iegd_esc_i2c_access_t;

#define MULTI_TYPE_SINGLE      0x1
#define MULTI_TYPE_CLONE       0x2
#define MULTI_TYPE_TWIN        0x4
#define MULTI_TYPE_EXTENDED    0x8

/*
 * The buffer info escape details the front buffer information for
 * the requested screen. It also returns the sync slot used by that screen.
 * This escape is used with direct clients but can be called before the
 * direct connection is established.
 */
typedef struct _iegd_esc_buffer_info {
	unsigned int screen;
} iegd_esc_buffer_info_t;

typedef struct _iegd_esc_buffer_info_reply {
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int pixel_format;
	unsigned int flags;
	unsigned int offset;
	unsigned int sync_slot;
} iegd_esc_buffer_info_reply_t;

/*
 * Structures needed for surface allocation and free from the internal
 * memory manager.
 */
typedef struct _iegd_esc_alloc_surface {

	unsigned int drm_context;
	unsigned int surface_type;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int pixel_format;
	unsigned int flags;
} iegd_esc_alloc_surface_t;

typedef struct _iegd_esc_alloc_surface_reply {
	unsigned int drm_context;
	int ret;
	unsigned int buffer_id;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int flags;
	unsigned int offset;
	void *mem_handle;
} iegd_esc_alloc_surface_reply_t;

typedef struct _iegd_esc_alloc_region {
	unsigned int drm_context;
	unsigned int region_type;
	unsigned int size;
	unsigned int flags;
} iegd_esc_alloc_region_t;

typedef struct _iegd_esc_alloc_region_reply {
	int ret;
	unsigned int buffer_id;
	unsigned int offset;
	unsigned int size;
	void *mem_handle;
} iegd_esc_alloc_region_reply_t;

typedef struct _iegd_esc_free {
	unsigned int drm_context;
	unsigned int offset;
} iegd_esc_free_t;

typedef struct _iegd_esc_get_num_surface_reply {
	int ret;
	unsigned long count;
} iegd_esc_get_num_surface_reply_t;

typedef struct _iegd_esc_get_surface_list {
	unsigned long allocated_size;
} iegd_esc_get_surface_list_t;

typedef struct _iegd_esc_get_surface_list_reply {
	unsigned long offset;
	unsigned long size;
} iegd_esc_get_surface_list_reply_t;



/*
 * Drawable is needed for exec to make sure that the correct
 * ring buffers and syncs are used. These may be different on a
 * per-display basis.
 */
typedef struct _iegd_esc_exec {
	unsigned int drm_context;
	unsigned int drawable;
	unsigned int size;
	/* data */
} iegd_esc_exec_t;

typedef enum _iegd_surface_state {
	EGD_SURFACE_STATE_GOOD = 0,
	EGD_SURFACE_STATE_BAD,
	EGD_SURFACE_STATE_FLIPPED,
	EGD_SURFACE_STATE_PIXMAP
} iegd_surface_state_t;

typedef enum _iegd_sfp_success {
	EGD_SFP_SUCCESS = 0,
	EGD_SFP_FAILURE
} iegd_sfp_success_state_t;

typedef struct _iegd_esc_exec_reply {
	iegd_surface_state_t surface_state;
	unsigned int sync_id;
} iegd_esc_exec_reply_t;

typedef struct _iegd_esc_fill {
	unsigned int drm_context;
	unsigned int buffer_id;
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
	unsigned int color;
} iegd_esc_fill_t;

#define EGD_REQUEST_FLIP    0x1
#define EGD_REQUEST_ASYNC   0x2
#define EGD_REQUEST_WAIT    0x4
#define EGD_REQUEST_DEFAULT 0x8

typedef struct _iegd_esc_blit {
	unsigned int drm_context;
	unsigned int src_buffer_id;
	unsigned int dest_buffer_id;
	unsigned int drawable;
	unsigned int color;
	unsigned int src_x1;
	unsigned int src_y1;
	unsigned int src_x2;
	unsigned int src_y2;
	unsigned int dest_x1;
	unsigned int dest_y1;
	unsigned int dest_x2;
	unsigned int dest_y2;
	unsigned int flags;
} iegd_esc_blit_t;

typedef struct _iegd_esc_sfp {
	unsigned int drm_context;
	unsigned int glxdrawable;
	unsigned int buffer;
} iegd_esc_sfp_t;

typedef struct _iegd_esc_blit_reply {
	iegd_surface_state_t surface_state;
	unsigned long sync_id;
} iegd_esc_blit_reply_t;

typedef struct _iegd_esc_sfp_reply {
	iegd_sfp_success_state_t sfp_success;

	unsigned long pixel_format;
	unsigned long offset;
	unsigned int  pitch;
	unsigned int  height;
	unsigned long flags;
} iegd_esc_sfp_reply_t;


/* Values used in iegd_esc_overlay_t.flags */ 
#define EGD_OVERLAY_COLORKEY       0x1
#define EGD_OVERLAY_BLEND          0x2
#define EGD_OVERLAY_SUBPICT        0x4
#define EGD_OVERLAY_ALPHA          0x8  /* Global Alpha for Subpict */
#define EGD_OVERLAY_CHROMA         0x10 /* Chroma for the Subpict */
#define EGD_OVERLAY_SUBPICT_DIRTY  0x20
#define EGD_OVERLAY_FIELD          0x40
#define EGD_OVERLAY_FRAME          0x00
#define EGD_OVERLAY_TOP            0x80
#define EGD_OVERLAY_BOTTOM         0x00
#define EGD_OVERLAY_WAIT_FLIP_DONE 0x100
#define EGD_OVERLAY_IS_FLIP_DONE   0x200
#define EGD_OVERLAY_MASK           0x400 /* escape_blend: has mask surf */
#define EGD_OVERLAY_DEST           0x800 /* escape_blend: has dest surf */
#define EGD_OVERLAY_DIFFUSE        0x1000 /* escape_blend: A8 + diffuse */
#define EGD_OVERLAY_ROT90          0x2000 /* rotation of src */
#define EGD_OVERLAY_ROT180         0x4000
#define EGD_OVERLAY_ROT270         0x6000 /* 90 plus 180 */
#define EGD_OVERLAY_FLIP           0x8000
#define EGD_OVERLAY_CHROMA_DST     0x10000
#define EGD_OVERLAY_ALPHA_SRC      0x20000
#define EGD_OVERLAY_CLIP           0x40000
#define EGD_OVERLAY_PROC_AMP       0x80000

#define EGD_OVERLAY_FILTER_NEAREST          0x00100000
#define EGD_OVERLAY_FILTER_BILINEAR         0x00200000
#define EGD_OVERLAY_FILTER_ANISOTROPIC      0x00400000
#define EGD_OVERLAY_FILTER_MASK             0x00700000

#define EGD_OVERLAY_OFF           		0x80000000
#define EGD_FW_VIDEO_OFF          		0x40000000
#define EGD_OVERLAY_SKIP_COLORKEY_FILL          0x20000000

typedef struct _iegd_esc_overlay {
	unsigned int drm_context;
	unsigned int src_buffer_id;
	unsigned int msk_buffer_id;
	unsigned int dst_buffer_id;
	unsigned int subpict_buffer_id;
	unsigned int subpict_buffer2_id;
	unsigned int drawable;
	unsigned int src_x1;
	unsigned int src_y1;
	unsigned int src_x2;
	unsigned int src_y2;
	unsigned int dest_x1;
	unsigned int dest_y1;
	unsigned int dest_x2;
	unsigned int dest_y2;
	unsigned int sub_x;
	unsigned int sub_y;
	unsigned int sub_dest_x;
	unsigned int sub_dest_y;
	unsigned int sub_width;
	unsigned int sub_height;
	unsigned int global_alpha; /* Also used for diffuse colour */
	unsigned int chroma_low;
	unsigned int chroma_high;
	unsigned int flags;
	unsigned int num_clips;
	unsigned int scaling_workaround;
	/* OPTIONAL clip_data x1,y1 -> x2, y2 */
	/* OPTIONAL mask_data x1,y1 -> x2, y2 */
	/* OPTIONAL proc amp data - 6 floats */
} iegd_esc_overlay_t;

typedef struct _iegd_esc_overlay_reply {
	iegd_surface_state_t surface_state;
	unsigned int sync_id;
} iegd_esc_overlay_reply_t;



#define EGD_MAX_SUBPICS         1

/* In this structure we are assuming there can be at most six subpictures
 * even though we currently only support one.  If the subpicture format is
 * AI44 then the caller is responsible for filling the coefficients and
 * allocating enough contiguous memory for the palette, i.e. the caller must
 * allocate a memory that is of size (size of iegd_esc_video_blend +
 * size of palette)
 *
 * We are breaking away from prior escape structures by using custom
 * data structures, e.g. iegd_rect_t.  This is to make the content more
 * manageable.
 */
typedef struct _iegd_esc_video_blend {
	unsigned int drm_context;
	unsigned int drawable;

	/* Source, Destination and Subpicture Surfaces.  Even though
	 * EGD_MAX_SUBPICS is 6, we really only support 1 */
	unsigned int dst_buffer_id;
	unsigned int src_buffer_id;
	unsigned int subpic_buffer_ids[EGD_MAX_SUBPICS];

	/* Source and Destination Rectangles */
	iegd_rect_t dest_rect;
	iegd_rect_t src_rect;
	iegd_rect_t subpic_src_rects[EGD_MAX_SUBPICS];
	iegd_rect_t subpic_dest_rects[EGD_MAX_SUBPICS];

	/* Render Flags */
	unsigned int flags;

	/* Subpicture Palette YUV->RGB Coefficients */
	iegd_yuv_coeffs yuv_coeffs[EGD_MAX_SUBPICS];

	/* Subpicture Palette */
	unsigned int palette_size[EGD_MAX_SUBPICS];

	/* This must be at the end of this structure. */
	void *Reserved;
} iegd_esc_video_blend_t;

typedef struct _iegd_esc_query_composite {
	unsigned int drawable;
} iegd_esc_query_composite_t;

typedef struct _iegd_esc_query_composite_reply {
	iegd_surface_state_t surface_state;
} iegd_esc_query_composite_reply_t;
/*
 * This is a CE-specific structure for handling video acceleration.  This
 * structure is purposely made generic so that all video acceleration
 * functions can share this structure instead of defining a custom one
 * for each function.
 */
#define EGD_EXECUTE_TYPE_VIDEO         1
#define EGD_EXECUTE_TYPE_BLEND_3D      2 
#define EGD_EXECUTE_TYPE_2D            3

typedef struct _iegd_esc_video_buff_operation {
	void          * ctx;
	unsigned char * virt;
	unsigned long   offset;
	long            type;
		/* only during buff execution - see EGD_EXECUTE_TYPE_XXX above */
	long            size;
	int             buffer_id;
	char            start_of_data; /* !!MUST be last member!! */
} iegd_esc_video_buff_operation_t;

#define EGD_CLIENT_TYPE_3D      0x00000001
#define EGD_CLIENT_TYPE_VIDEO   0x00000002
#define EGD_CLIENT_TYPE_BLEND   0x00000004

typedef struct _iegd_esc_video_accel_ctx{
	void * user_proc_handle; /* provided by the client */
	unsigned long client_type; /* provided by the client - definitions above */
	void * iegd_client_context; /* returned by the driver */
} iegd_esc_video_accel_ctx_t;

#define EGD_SYNC_TYPE_OP_MASK       0x0FFFFFFF
#define EGD_SYNC_TYPE_BLOCK         0x80000000
	/* perform the sync operation as a blocking call   *
	 * only can be 'OR'-ed with EGD_SYNC_TYPE_CHECK_X */
#define EGD_SYNC_TYPE_GET_ID        0x00000001
	/* get a sync number at current hardware rendering status */
#define EGD_SYNC_TYPE_CHECK_ID      0x00000002
	/* sync on the provided sync id       *
	 * number ifs its a non-blocking call,*
	 * status will be 0 for SUCCESS       *
	 * or non-zero for busy               */
#define EGD_SYNC_TYPE_CHECK_CONTEXT_2D    0x00000004
#define EGD_SYNC_TYPE_CHECK_CONTEXT_VIDEO 0x00000008
#define EGD_SYNC_TYPE_CHECK_CONTEXT_3D    0x00000010
	/* do a sync on the last execute_     *
	 * buffer operation for this context  */

typedef struct _iegd_esc_sync{
	void * client_ctx;
	unsigned long sync_type; /* above EGD_SYNC_TYPE_XXX */
	unsigned long sync_id;
	int retval;
		/* 0 for success, non-zero for busy or failed */
} iegd_esc_sync_t;

typedef struct _iegd_esc_overlay_quick_bypass {
	void * client_ctx;
	int src_priv_surf_id;
	int dst_ddraw_surf_id;
} iegd_esc_overlay_quick_bypass_t;

typedef struct _iegd_esc_wndobj_setup {
	void *hwnd;
	int enable;
} iegd_esc_wndobj_setup_t;

typedef struct _iegd_esc_wndobj_setup_reply {
	unsigned int drawable_id;
} iegd_esc_wndobj_setup_reply_t;

typedef struct _iegd_esc_dri_reply {
	unsigned int  enabled;
} iegd_esc_dri_reply_t;

typedef struct _iegd_esc_reg_shared_surface {
	unsigned int drm_context;
	unsigned int mix_buffer_id;
	unsigned int iegd_buffer_id;
	unsigned int width;
	unsigned int height;
	unsigned int pixel_format;		
	unsigned int offset;
	unsigned int flag_value; /*0x01 - means dst surf registration , 0x02 - src*/
} iegd_esc_reg_shared_surface_t;

typedef struct _iegd_esc_create_context {
	unsigned int screen;
} iegd_esc_create_context_t;

typedef struct _iegd_esc_context {
	unsigned int drm_context;
} iegd_esc_context_t;

typedef struct _iegd_esc_local_time {
	int sec;
	int min;
	int hour;
	int mday;
	int mon;
	int year;
	int wday;
	int yday;
	int is_dst;
} igd_esc_local_time_t;

typedef struct _iegd_esc_video_fps {
	unsigned int is_overlay;
	unsigned int is_hardware_decode;
	unsigned int is_interlaced_stream;
	int frame_count;
	igd_esc_local_time_t start_time;
	igd_esc_local_time_t end_time;
} igd_esc_video_info_t;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_drm.h
 * $Revision: 1.64 $
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
#ifndef _EMGD_DRM_H_
#define _EMGD_DRM_H_

#include <linux/version.h>
#include <drm/drm.h>
#include <igd.h>
#include <igd_appcontext.h>
#include <igd_errno.h>
#include <igd_init.h>
#include <igd_mode.h>
#include <igd_ovl.h>
#include <igd_pd.h>
#include <igd_pi.h>
#include <igd_pwr.h>
#include <igd_version.h>

enum {
	CHIP_PSB_8108 = 0,
	CHIP_PSB_8109 = 1,
	CHIP_TC_4108 = 2
};

/* To differentiate different engines for command buffer submission */
#define PSB_ENGINE_2D 0
#define PSB_ENGINE_VIDEO 1
#define PSB_ENGINE_RASTERIZER 2
#define PSB_ENGINE_TA 3
#define PSB_ENGINE_HPRAST 4
#define TNC_ENGINE_ENCODE 5
#define PSB_ENGINE_COMPOSITOR_MMU 6


/* OEM flags for buffer allocation through the PVR2DCreateFlipChain */
#ifndef PVR2D_CREATE_FLIPCHAIN_SHARED
#define PVR2D_CREATE_FLIPCHAIN_SHARED		(1UL << 0)
#define PVR2D_CREATE_FLIPCHAIN_QUERY			(1UL << 1)
#define PVR2D_CREATE_FLIPCHAIN_OEMOVERLAY	(1UL << 2)
#define PVR2D_CREATE_FLIPCHAIN_AS_BLITCHAIN	(1UL << 3)
#endif

#define PVR2D_CREATE_FLIPCHAIN_OEMDISPLAY   (1UL << 4)
#define PVR2D_CREATE_FLIPCHAIN_OEMGENERAL   (1UL << 5)
#define PVR2D_CREATE_FLIPCHAIN_OEMFLIPCHAIN (1UL << 6)
#define PVR2D_CREATE_FLIPCHAIN_CI (1UL << 7)
#define PVR2D_CREATE_FLIPCHAIN_CI_V4L2_MAP (1UL << 8)
/* Different command */
#define CMD_VIDEO_STATE           1
#define CMD_VIDEO_INITIALIZE      2
#define CMD_VIDEO_UNINITIALIZE    3
#define CMD_VIDEO_CLOSE_CONTEXT	  4
#define CMD_VIDEO_CREATE_CONTEXT  5
#define CMD_VIDEO_SHUTDOWN        6
#define CMD_VIDEO_GET_FENCE_ID    7
#define CMD_VIDOE_GET_FRAME_SKIP  8
#define CMD_VIDEO_GET_MSVDX_STATUS  	  9

/* Video state */
#define VIDEO_STATE_FW_LOADED     	0x00000001
#define VIDEO_STATE_RENDEC_FREED        0x00000002

/* Different command for alter_ovl2 */
#define CMD_ALTER_OVL2			1
#define CMD_ALTER_OVL2_OSD		2

typedef enum _kdrm_test_pvrsrv_opcodes {
	SINGLE_DC_SWAPING        = 0,
	DIH_DC_DRAW_OPEN_DEV1    = 1,
	DIH_DC_MODE_CHANGE1_DEV1 = 2,
	DIH_DC_MODE_CHANGE2_DEV1 = 3,
	DIH_DC_CLOSE_DEV1        = 4,
	DIH_DC_DRAW_OPEN_DEV2    = 5,
	DIH_DC_MODE_CHANGE1_DEV2 = 6,
	DIH_DC_MODE_CHANGE2_DEV2 = 7,
	DIH_DC_CLOSE_DEV2        = 8,
} emgd_drm_test_pvrsrv_opcodes_t;


/*
 * This is where all the data structures used by the Koheo DRM interface are
 * defined.  These data structures are shared between and are used to pass data
 * between the user-space & kernel-space code for each ioctl.
 *
 * The naming convention is:  emgd_drm_<HAL-procedure-pointer-name>_t
 */

typedef struct _kdrm_alter_cursor {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	igd_cursor_info_t cursor_info; /* (DOWN) */
	/**
	 * A pointer to the cursor data (in user space), which must be copied
	 * separately from this struct.
	 */
	unsigned char *image; /* (DOWN) */
} emgd_drm_alter_cursor_t;


typedef struct _kdrm_alter_cursor_pos {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	igd_cursor_info_t cursor_info; /* (DOWN) */
} emgd_drm_alter_cursor_pos_t;


typedef struct _kdrm_alter_displays {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_info_t primary_pt_info; /* (DOWN) */
	igd_display_info_t secondary_pt_info; /* (DOWN) */
	igd_framebuffer_info_t primary_fb_info; /* (DOWN) */
	igd_framebuffer_info_t secondary_fb_info; /* (DOWN) */
	unsigned long dc; /* (DOWN) */
	unsigned long flags; /* (DOWN) */

	igd_display_h primary; /* (UP) Generated "opaque handle" */
	igd_display_h secondary; /* (UP) Generated "opaque handle" */
} emgd_drm_alter_displays_t;


typedef struct _kdrm_alter_ovl {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	igd_appcontext_h appcontext_h; /* (DOWN) a handle */
	/* NOTE: igd_surface_t contains a pointer to an igd_palette_info_t struct
	 * (member name "palette_info"), which contains a pointer to an "unsigned
	 * long" "palette".  Normally, each of these values must be copied from
	 * user to kernel space.  However, an investigation found that the
	 * igd_palette_info_t struct is not used for either alter_ovl() or
	 * set_surface() (and igd_get_surface_plb() sets the value to 0,
	 * a.k.a. NULL).  Thus, we're okay not copying this value.
	 */
	igd_surface_t src_surf; /* (DOWN) */
	igd_rect_t src_rect; /* (DOWN) */
	igd_rect_t dst_rect; /* (DOWN) */
	igd_ovl_info_t ovl_info; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
} emgd_drm_alter_ovl_t;

typedef struct _kdrm_alter_ovl2 {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	igd_surface_t src_surf; /* (DOWN) */
	igd_rect_t src_rect; /* (DOWN) */
	igd_rect_t dst_rect; /* (DOWN) */
	igd_ovl_info_t ovl_info; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
	int cmd; /* (DOWN) */
} emgd_drm_alter_ovl2_t;


typedef struct _kdrm_get_ovl_init_params {
    ovl_um_context_t *ovl_um_context;
    int rtn; /* (UP) - return value of HAL procedure */
} emgd_drm_get_ovl_init_params_t;


typedef struct _kdrm_appcontext_alloc {
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	int priority; /* (DOWN) */
	unsigned int flags; /* (DOWN) */
	igd_appcontext_h appcontext_h; /* (UP) a handle */
} emgd_drm_appcontext_alloc_t;


typedef struct _kdrm_appcontext_free {
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	int priority; /* (DOWN) */
	igd_appcontext_h appcontext_h; /* (DOWN) a handle */
} emgd_drm_appcontext_free_t;


typedef struct _kdrm_driver_save_restore {
	int rtn; /* (UP) - return value of HAL procedure */
} emgd_drm_driver_save_restore_t;


typedef struct _kdrm_enable_port {
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	unsigned long flag; /* (DOWN) */
	unsigned long test; /* (DOWN) */
} emgd_drm_enable_port_t;


typedef struct _kdrm_get_attrs {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	/** The number of attributes of memory allocated by user-space. */
	unsigned int allocated_size; /* (DOWN) */
	/** The number of attributes returned by the kernel. */
	unsigned long list_size; /* (UP) */
	/**
	 * A pointer to the user-space-allocated memory for the kernel to copy the
	 * returned attribute list into.  allocated_size is how many attributes
	 * this memory can contain.  list_size is how many attributes the kernel is
	 * returning (or wants to return).  If list_size > allocated_size, no data
	 * is copied back to user-space, and the user-space must re-allocate
	 * list_size amount of memory and try the ioctl again.
	 */
	igd_attr_t *attr_list; /* (UP) */
	int extended; /* (DOWN/UP) - true if there are/to get extended attributes */
} emgd_drm_get_attrs_t;

#define IGD_GET_DISPLAY_NO_3DD_REINIT 	0x1

typedef struct _kdrm_get_display {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned short port_number; /* (DOWN) */
	igd_framebuffer_info_t fb_info; /* (DOWN/UP) Caller alloc's; DRM returns */
	igd_display_info_t pt_info; /* (DOWN/UP) Caller alloc's; DRM returns */
	unsigned long flags; /* (DOWN) */
} emgd_drm_get_display_t;


typedef struct _kdrm_get_drm_config {
	int rtn; /* (UP) - return value of HAL procedure */
	/** Which configuration to use within "xorg.conf" */
	long config_id; /* (UP) */
	/** The set of compile-time parameters, specified in "user_config.c". */
	igd_param_t params; /* (UP) */
	/** The display config (e.g. 8 for DIH). */
	int display_config; /* (UP) */
	/*
	 * Build configuration (e.g., DDK version used, debug vs release, etc.);
	 * if these don't match what userspace was built with, the driver may not
	 * run properly.  (UP)
	 */
	igd_build_config_t build_config;
} emgd_drm_get_drm_config_t;


typedef struct _kdrm_get_EDID_block {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	unsigned char block_number; /* (DOWN) */
	unsigned char edid_block[128]; /* (UP) */
} emgd_drm_get_EDID_block_t;


typedef struct _kdrm_get_EDID_info {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	unsigned char edid_version; /* (UP) */
	unsigned char edid_revision; /* (UP) */
	unsigned long edid_size; /* (UP) */
} emgd_drm_get_EDID_info_t;


/*!
 * This enum is used to state which of the 5 pixel format lists is desired to
 * be returned in the emgd_drm_get_pixelformats_t struct (which only transfers
 * one list).
 */
typedef enum _kdrm_which_pixelformat {
	PIXEL_FORMAT_FRAMEBUFFER,
	PIXEL_FORMAT_CURSOR,
	PIXEL_FORMAT_OVERLAY,
	PIXEL_FORMAT_RENDER,
	PIXEL_FORMAT_TEXTURE
} emgd_drm_which_pixelformat_t;

/*!
 * The igd_dispatch_t.get_pixelformats() procedure returns up-to 5 different
 * lists of pixel formats.  However, for Koheo drivers, only one of these lists
 * is ever requested/returned.  Thus, only one list needs to be passed between
 * user/kernel space.  Memory for the list is allocated in user space, and a
 * pointer to it, along with its size and which format is desired, is passed to
 * the kernel in this struct.
 */
typedef struct _kdrm_get_pixelformats {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	emgd_drm_which_pixelformat_t format; /* (DOWN) */
	/** The number of pixelformats of memory allocated by user-space. */
	unsigned int allocated_size; /* (DOWN) */
	/** The number of pixelformats returned by the kernel. */
	unsigned int list_size; /* (UP) */
	unsigned long *format_list; /* (DOWN/UP) */
} emgd_drm_get_pixelformats_t;


typedef struct _kdrm_get_port_info {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	igd_port_info_t port_info; /* (UP) */
} emgd_drm_get_port_info_t;


typedef struct _kdrm_gmm_alloc_region {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long offset; /* (UP) */
	unsigned long size; /* (DOWN/UP) */
	unsigned int type; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
} emgd_drm_gmm_alloc_region_t;


typedef struct _kdrm_gmm_alloc_surface {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long offset; /* (UP) */
	unsigned long pixel_format; /* (DOWN) */
	unsigned int width; /* (DOWN/UP) */
	unsigned int height; /* (DOWN/UP) */
	unsigned int pitch; /* (UP) */
	unsigned long size; /* (DOWN/UP) */
	unsigned int type; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
} emgd_drm_gmm_alloc_surface_t;


typedef struct _kdrm_gmm_get_num_surface {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long count; /* (UP) */
} emgd_drm_gmm_get_num_surface_t;


typedef struct _kdrm_gmm_get_surface_list {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long allocated_size; /* (DOWN) */
	unsigned long list_size; /* (UP) */
	igd_surface_list_t **surface_list; /* (UP) */
} emgd_drm_gmm_get_surface_list_t;


typedef struct _kdrm_gmm_free {
	unsigned long offset; /* (DOWN) */
} emgd_drm_gmm_free_t;


typedef struct _kdrm_gmm_flush_cache {
	int rtn; /* (UP) - return value of HAL procedure */
} emgd_drm_gmm_flush_cache_t;

typedef struct _kdrm_video_cmd_buf {
	int rtn;
	int engine;
	unsigned long offset;
	void *mem_handle;
	unsigned long fence_id;
} emgd_drm_video_cmd_buf_t;

typedef struct _kdrm_device_info {
	unsigned long device_id;
	unsigned long revision_id;
	unsigned short bridge_id;
	unsigned long display_memory_size;
	unsigned long display_memory_offset;
} emgd_drm_device_info_t;

typedef struct _kdrm_init_video {
	int rtn;
	int cmd;
	int engine;
	union {
		struct {
			unsigned long base0;
			unsigned long base1;
			void *mem_handle_fw;
			unsigned long fw_size;
		};
		struct {
			unsigned long wb_offset;
			void *mem_handle_writeback;
			void *mem_handle_enc_fw;
		};
		unsigned long status;
		unsigned long context_id;
	};
} emgd_drm_init_video_t;

typedef struct _kdrm_video_get_info {
	int rtn;
	int engine;
	int cmd;
	int last_frame;
	unsigned long fence_id;
	unsigned long frame_skip;
	unsigned long queue_status; /* (UP) - return 1 if msvdx queue empty, else 0 */
	unsigned long mtx_msg_status; /* (UP) - return 1 if msvdx current message complete, else 0 */
} emgd_drm_video_get_info_t;

typedef struct _kdrm_video_flush_tlb {
	int rtn;
	int engine;
} emgd_drm_video_flush_tlb_t;

typedef struct _kdrm_preinit_mmu {
	int rtn;
	unsigned long memcontext;
} emgd_drm_preinit_mmu_t;

typedef struct _kdrm_get_display_info {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_info_t primary_pt_info; /* (UP) */
	igd_display_info_t secondary_pt_info; /* (UP) */
	igd_framebuffer_info_t primary_fb_info; /* (UP) */
	igd_framebuffer_info_t secondary_fb_info; /* (UP) */
	unsigned long dc; /* (UP) */
	unsigned long flags; /* (UP) */

	igd_display_h primary; /* (UP) Generated "opaque handle" */
	igd_display_h secondary; /* (UP) Generated "opaque handle" */
} emgd_drm_get_display_info_t;

typedef struct _kdrm_pan_display {
	/* Note: the return value is a long this time: */
	long rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned long x_offset; /* (DOWN) */
	unsigned long y_offset; /* (DOWN) */
} emgd_drm_pan_display_t;


typedef struct _kdrm_power_display {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	unsigned long power_state;  /* (DOWN) */
} emgd_drm_power_display_t;


typedef struct _kdrm_pwr_alter {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned int power_state; /* (DOWN) */
} emgd_drm_pwr_alter_t;


/*!
 * Memory for the list is allocated in user space, and a pointer to it, along
 * with its size, is passed to the kernel in this struct.
 */
typedef struct _kdrm_query_dc {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long request; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
	/** The number of DCs of memory allocated by user-space. */
	unsigned int allocated_size; /* (DOWN) */
	/** The number of DCs returned by the kernel. */
	unsigned int list_size; /* (UP) */
	unsigned long *dc_list; /* (UP) */
} emgd_drm_query_dc_t;


typedef struct _kdrm_query_max_size_ovl {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned long pf; /* (DOWN) */
	unsigned int max_width; /* (UP) */
	unsigned int max_height; /* (UP) */
} emgd_drm_query_max_size_ovl_t;


typedef struct _kdrm_query_ovl {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned long flags; /* (DOWN) */
} emgd_drm_query_ovl_t;


typedef struct _kdrm_query_mode_list {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long dc; /* (DOWN) */
	unsigned long flags; /* (DOWN) */
	/** The number of attributes of memory allocated by user-space. */
	unsigned int allocated_size; /* (DOWN) */
	/** The number of attributes returned by the kernel. */
	unsigned int list_size; /* (UP) */
	igd_display_info_t *mode_list; /* (DOWN/UP) */
} emgd_drm_query_mode_list_t;


typedef struct _kdrm_get_golden_htotal {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_info_t *in_mode; /* (DOWN/UP) */
	igd_display_info_t *out_mode; /* (DOWN/UP) */
} emgd_drm_get_golden_htotal_t;

typedef struct _kdrm_set_attrs {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned short port_number; /* (DOWN) */
	/** The number of attributes sent to the kernel. */
	unsigned int list_size; /* (DOWN) */
	/**
	 * Note: the attribute list is variable-sized data, but an ioctl must use a
	 * fixed-sized struct.  Thus, this struct contains both a pointer to the
	 * attribute list (in user space, which must be copied separately from this
	 * struct) and the number of attibutes (above).
	 */
	igd_attr_t *attr_list; /* (DOWN) */
} emgd_drm_set_attrs_t;


typedef struct _kdrm_set_palette_entry {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned long palette_entry; /* (DOWN) */
	unsigned long palette_color; /* (DOWN) */
} emgd_drm_set_palette_entry_t;


typedef struct _kdrm_set_surface {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	int priority; /* (DOWN) */
	igd_buffertype_t type; /* (DOWN) */
	/* NOTE: igd_surface_t contains a pointer to an igd_palette_info_t struct:
	 * see the description above, in the typedef emgd_drm_alter_ovl_t.
	 */
	igd_surface_t surface; /* (DOWN) */
	igd_appcontext_h appcontext; /* (DOWN) a handle */
	unsigned long flags; /* (DOWN) */
} emgd_drm_set_surface_t;

#define PRIMARY_DISPLAY 		0
#define SECONDARY_DISPLAY 		1
#define CLONE_PRIMARY 			PRIMARY_DISPLAY
#define CLONE_SECONDARY 		SECONDARY_DISPLAY
#define CLONE 				0
#define DIH   				1

#define DUAL_SCREEN_MAX_DISPLAY 	2
#define MAX_FFB_SURF_VEXT 		DUAL_SCREEN_MAX_DISPLAY
#define MAX_FFB_SURF_DIH 		1

typedef struct _kdrm_dihclone_set_surface {
	int rtn; /* (UP) - return value of HAL procedure */
	unsigned long dih_clone_display; /* (DOWN) - primary or secondary display to clone */
	unsigned long mode; /* (DOWN) - dih to clone or back to dih */
} emgd_drm_dihclone_set_surface_t;

typedef struct _kdrm_control_plane_format {
	/* Note on modification to the structure to accomodate both 
	 * Use PRIMARY/SECONDARY to indicate which display
	 * FB blend + overlay to turn ON/OFF. The relationship between 
	 * plane, pipe and port is transparent to the user.
	 * A qualifier (use_plane) is used to decide which model the user wants. 
	 */
	int rtn; /* (UP) - return value of HAL procedure */
	/* 	(DOWN) Turn off transparency by switching to XRGB format = 0
		Turn on transparency by switching to ARGB format = 1 */
	int enable; 
	union {
		/* 	(DOWN) Plane A = 0
			Plane B = 1 */
		int display_plane;
		/* 	(DOWN) Primary/Secondary display handle */
		igd_display_h primary_secondary_dsp;
	};
	/* If set, KMD will use the plane convention */
	unsigned int use_plane;
} emgd_drm_control_plane_format_t;


typedef struct _kdrm_set_overlay_display{
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h ovl_display[OVL_MAX_HW]; /* (DOWN) Overlay display handles */
} emgd_drm_set_overlay_display_t;


typedef struct _kdrm_sync {
	int rtn; /* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	int priority; /* (DOWN) */
	unsigned long in_sync; /* (DOWN/UP) */
	unsigned long flags; /* (DOWN) */
} emgd_drm_sync_t;


typedef struct _kdrm_driver_pre_init {
	int rtn; /* (UP) - return value of HAL procedure */
	/**
	 * The set of user-specified parameters, parsed at X server PreInit() time.
	 */
	igd_param_t params; /* (DOWN) */
	/**
	 * The results of the HAL's igd_get_config_info() procedure.  The
	 * user-space code allocates memory for this.
	 */
	igd_config_info_t config_info; /* (UP) */
	/* this is passed in by intelPreInit */
	unsigned long bpp;
} emgd_drm_driver_pre_init_t;


typedef struct _kdrm_driver_get_ports {
	int rtn; /* (UP) - return value of HAL procedure */
	/** An array of ports discovered by the device-specific HAL driver. */
	unsigned long ports[IGD_MAX_PORTS]; /* (UP) */
} emgd_drm_driver_get_ports_t;


typedef struct _kdrm_driver_get_chipset_info {
	unsigned long device_id;
	unsigned long revision_id;
	char name[40];
	char description[20];
} emgd_drm_driver_get_chipset_info_t;


typedef struct _kdrm_get_page_list {
	int rtn; /* (UP) - return value of kernel-mode test code() */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned long offset; /* (DOWN) - surface offset (ID) */
	unsigned long addr_count; /* (DOWN) - size of address list */
	unsigned long *addr_list; /* (UP) */
} emgd_drm_get_page_list_t;

typedef struct _kdrm_start_pvrsrv {
	int xserver; /* (DOWN) - 1 if called by X server, else 0 */
	int rtn; /* (UP) - return value of PVRSRVDrmLoad() */
} emgd_drm_start_pvrsrv_t;

typedef struct _kdrm_test_pvrsrv {
	emgd_drm_test_pvrsrv_opcodes_t opcode; /* (DOWN) */
	int rtn; /* (UP) - return value of kernel-mode test code() */
	char rtn_msg[1024]; /* (UP) */
} emgd_drm_test_pvrsrv_t;


typedef struct _kdrm_query_2d_caps_hwhint {
	unsigned long caps_val; /* (DOWN) */
	unsigned long *status; /* (UP) */
} emgd_drm_query_2d_caps_hwhint_t;

/* For Buffer Class FCB #17711*/
typedef struct _kdrm_bc_ts {
	int rtn;
	int width;
	int height;
	int stride;
	int is_continous;
	unsigned long dev_id;
	unsigned long buf_id;
	unsigned long buf_tag;
	unsigned long num_buf;	
	unsigned long pixel_format;
	unsigned long phyaddr;
	unsigned long virtaddr;
	unsigned int mapped;
} emgd_drm_bc_ts_t;

typedef struct _kdrm_unlock_planes {
	int rtn;					/* (UP) - return value of HAL procedure */
	igd_display_h display_handle; /* (DOWN) - an "opaque handle" */
	unsigned int screen_num; /*primary=0, secondary=1 */
} emgd_drm_unlock_planes_t;

/*
 * This is where all the IOCTL's used by the egd DRM interface are
 * defined.  This information is shared between the user space code and
 * the kernel module.
 */

#define BASE DRM_COMMAND_BASE

/*
 * EMGD-specific ioctls.  These get mapped to the device specific range
 * between 0x40 and 0x79.
 *
 * Client driver must use these values!
 */
#define DRM_IGD_ALTER_CURSOR         0x00
#define DRM_IGD_ALTER_CURSOR_POS     0x01
#define DRM_IGD_ALTER_DISPLAYS       0x02
#define DRM_IGD_ALTER_OVL            0x03
#define DRM_IGD_APPCTX_ALLOC         0x04
#define DRM_IGD_APPCTX_FREE          0x05
#define DRM_IGD_DRIVER_SAVE_RESTORE  0x06
#define DRM_IGD_ENABLE_PORT          0x07
#define DRM_IGD_GET_ATTRS            0x08
#define DRM_IGD_GET_DISPLAY          0x09
#define DRM_IGD_GET_EDID_BLOCK       0x0a
#define DRM_IGD_GET_EDID_INFO        0x0b
#define DRM_IGD_GET_PIXELFORMATS     0x0c
#define DRM_IGD_GET_PORT_INFO        0x0d
#define DRM_IGD_GMM_ALLOC_REGION     0x0e
#define DRM_IGD_GMM_ALLOC_SURFACE    0x0f
#define DRM_IGD_GMM_FREE             0x10
#define DRM_IGD_GMM_FLUSH_CACHE      0x11
#define DRM_IGD_GMM_GET_NUM_SURFACE  0x31
#define DRM_IGD_GMM_GET_SURFACE_LIST 0x32
#define DRM_IGD_GET_GOLDEN_HTOTAL    0x33
#define DRM_IGD_CONTROL_PLANE_FORMAT 0x34
#define DRM_IGD_QUERY_2D_CAPS_HWHINT 0x35
#define DRM_IGD_DIHCLONE_SET_SURFACE 0x36
#define DRM_IGD_SET_OVERLAY_DISPLAY  0x37
#define DRM_IGD_WAIT_VBLANK			 0x40

/*
 * The EMGD DRM includes the PVR DRM, and as such, includes the following PVR
 * DRM ioctls.  The numbering must be kept in sync with what is defined in
 * "emgd_shared.h":
 */
#define DRM_IGD_RESERVED_1          DRM_PVR_RESERVED1 /* Reserved */
#define DRM_IGD_RESERVED_2          DRM_PVR_RESERVED2 /* Reserved */
#define DRM_IGD_RESERVED_3          DRM_PVR_RESERVED3 /* Reserved */
#define DRM_IGD_RESERVED_4          DRM_PVR_RESERVED4 /* Reserved */
#define DRM_IGD_RESERVED_5          DRM_PVR_RESERVED5 /* Reserved */
#define DRM_IGD_PAN_DISPLAY         0x17
#define DRM_IGD_POWER_DISPLAY       0x18
#define DRM_IGD_PWR_ALTER           0x19
#define DRM_IGD_QUERY_DC            0x1a
#define DRM_IGD_QUERY_MAX_SIZE_OVL  0x1b
#define DRM_IGD_QUERY_OVL           0x1c
#define DRM_IGD_QUERY_MODE_LIST     0x1d
/*
 * Another PVR DRM ioctl that must be kept in sync with what is defined in
 * "emgd_shared.h":
 */
#define DRM_IGD_RESERVED_6          DRM_PVR_RESERVED6 /* Reserved */
#define DRM_IGD_SET_ATTRS           0x1f
#define DRM_IGD_SET_PALETTE_ENTRY   0x20
#define DRM_IGD_SET_SURFACE         0x21
#define DRM_IGD_SYNC                0x22
#define DRM_IGD_DRIVER_PRE_INIT     0x23
#define DRM_IGD_DRIVER_GET_PORTS    0x24
#define DRM_IGD_START_PVRSRV        0x25
#define DRM_IGD_TEST_PVRSRV         0x26
/* For Video (MSVDX/TOPAZ) */
#define DRM_IGD_VIDEO_CMD_BUF       0x27
#define DRM_IGD_GET_DEVICE_INFO     0x28
#define DRM_IGD_INIT_VIDEO          0x29
#define DRM_IGD_VIDEO_GET_INFO      0x2a
#define DRM_IGD_VIDEO_FLUSH_TLB     0x2b
/* For X driver */
#define DRM_IGD_GET_DRM_CONFIG      0x2c
#define DRM_IGD_GET_PAGE_LIST       0x2d
#define DRM_IGD_GET_OVL_INIT_PARAMS 0x2e
#define DRM_IGD_ALTER_OVL2          0x2f
#define DRM_IGD_GET_CHIPSET_INFO    0x30
#define DRM_IGD_GET_DISPLAY_INFO    0x38
#define DRM_IGD_PREINIT_MMU         0x39
#define  DRM_IGD_UNLOCK_PLANES       0x47

/* For Buffer Class of Texture Stream */
#define DRM_IGD_BC_TS_INIT			0x40
#define DRM_IGD_BC_TS_UNINIT		0x41
#define DRM_IGD_BC_TS_REQUEST_BUFFERS	0x42
#define DRM_IGD_BC_TS_RELEASE_BUFFERS	0x43
#define DRM_IGD_BC_TS_SET_BUFFER_INFO	0x44
#define DRM_IGD_BC_TS_GET_BUFFERS_COUNT	0x45
#define DRM_IGD_BC_TS_GET_BUFFER_INDEX	0x46
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
#define DRMFB_PITCH pitches[0]
#define DRMMODE_HANDLE handles[0]
#define DRM_MODE_FB_CMD_TYPE drm_mode_fb_cmd2
#else
#define DRMFB_PITCH pitch
#define DRMMODE_HANDLE handle
#define DRM_MODE_FB_CMD_TYPE drm_mode_fb_cmd
#endif

/*
 * egd IOCTLs.
 */
#define DRM_IOCTL_IGD_ALTER_CURSOR     DRM_IOWR(DRM_IGD_ALTER_CURSOR + BASE,\
		emgd_drm_alter_cursor_t)
#define DRM_IOCTL_IGD_ALTER_CURSOR_POS DRM_IOWR(DRM_IGD_ALTER_CURSOR_POS +\
		BASE,\
		emgd_drm_alter_cursor_pos_t)
#define DRM_IOCTL_IGD_ALTER_DISPLAYS   DRM_IOWR(DRM_IGD_ALTER_DISPLAYS + BASE,\
		emgd_drm_alter_displays_t)
#define DRM_IOCTL_IGD_ALTER_OVL        DRM_IOWR(DRM_IGD_ALTER_OVL + BASE,\
		emgd_drm_alter_ovl_t)
#define DRM_IOCTL_IGD_ALTER_OVL2        DRM_IOWR(DRM_IGD_ALTER_OVL2 + BASE,\
		emgd_drm_alter_ovl2_t)
#define DRM_IOCTL_IGD_APPCTX_ALLOC     DRM_IOWR(DRM_IGD_APPCTX_ALLOC + BASE,\
		emgd_drm_appcontext_alloc_t)
#define DRM_IOCTL_IGD_APPCTX_FREE      DRM_IOWR(DRM_IGD_APPCTX_FREE + BASE,\
		emgd_drm_appcontext_free_t)
#define DRM_IOCTL_IGD_DRIVER_SAVE_RESTORE DRM_IOWR(DRM_IGD_DRIVER_SAVE_RESTORE\
		+ BASE,\
		emgd_drm_driver_save_restore_t)
#define DRM_IOCTL_IGD_ENABLE_PORT      DRM_IOWR(DRM_IGD_ENABLE_PORT + BASE,\
		emgd_drm_enable_port_t)
#define DRM_IOCTL_IGD_GET_ATTRS        DRM_IOWR(DRM_IGD_GET_ATTRS + BASE,\
		emgd_drm_get_attrs_t)
#define DRM_IOCTL_IGD_GET_DISPLAY      DRM_IOWR(DRM_IGD_GET_DISPLAY + BASE,\
		emgd_drm_get_display_t)
#define DRM_IOCTL_IGD_GET_DRM_CONFIG   DRM_IOWR(DRM_IGD_GET_DRM_CONFIG + BASE,\
		emgd_drm_get_drm_config_t)
#define DRM_IOCTL_IGD_GET_EDID_BLOCK   DRM_IOWR(DRM_IGD_GET_EDID_BLOCK + BASE,\
		emgd_drm_get_EDID_block_t)
#define DRM_IOCTL_IGD_GET_EDID_INFO    DRM_IOWR(DRM_IGD_GET_EDID_INFO + BASE,\
		emgd_drm_get_EDID_info_t)
#define DRM_IOCTL_IGD_GET_PIXELFORMATS DRM_IOWR(DRM_IGD_GET_PIXELFORMATS +\
		BASE,\
		emgd_drm_get_pixelformats_t)
#define DRM_IOCTL_IGD_GET_PORT_INFO    DRM_IOWR(DRM_IGD_GET_PORT_INFO + BASE,\
		emgd_drm_get_port_info_t)
#define DRM_IOCTL_IGD_GMM_ALLOC_REGION DRM_IOWR(DRM_IGD_GMM_ALLOC_REGION +\
		BASE,\
		emgd_drm_gmm_alloc_region_t)
#define DRM_IOCTL_IGD_GMM_ALLOC_SURFACE DRM_IOWR(DRM_IGD_GMM_ALLOC_SURFACE +\
		BASE,\
		emgd_drm_gmm_alloc_surface_t)
#define DRM_IOCTL_IGD_GMM_GET_NUM_SURFACE DRM_IOW(\
		DRM_IGD_GMM_GET_NUM_SURFACE +\
		BASE,\
		emgd_drm_gmm_get_num_surface_t)
#define DRM_IOCTL_IGD_GMM_GET_SURFACE_LIST DRM_IOW(\
		DRM_IGD_GMM_GET_SURFACE_LIST +\
		BASE,\
		emgd_drm_gmm_get_surface_list_t)
#define DRM_IOCTL_IGD_GMM_FREE         DRM_IOW(DRM_IGD_GMM_FREE + BASE,\
		emgd_drm_gmm_free_t)
#define DRM_IOCTL_IGD_GMM_FLUSH_CACHE  DRM_IOR(DRM_IGD_GMM_FLUSH_CACHE + BASE,\
		emgd_drm_gmm_flush_cache_t)
#define DRM_IOCTL_IGD_PAN_DISPLAY      DRM_IOWR(DRM_IGD_PAN_DISPLAY + BASE,\
		emgd_drm_pan_display_t)
#define DRM_IOCTL_IGD_POWER_DISPLAY    DRM_IOWR(DRM_IGD_POWER_DISPLAY + BASE,\
		emgd_drm_power_display_t)
#define DRM_IOCTL_IGD_PWR_ALTER        DRM_IOWR(DRM_IGD_PWR_ALTER + BASE,\
		unsigned long)
#define DRM_IOCTL_IGD_QUERY_DC         DRM_IOWR(DRM_IGD_QUERY_DC + BASE,\
		emgd_drm_query_dc_t)
#define DRM_IOCTL_IGD_QUERY_MAX_SIZE_OVL DRM_IOWR(DRM_IGD_QUERY_MAX_SIZE_OVL +\
		BASE,\
		emgd_drm_query_max_size_ovl_t)
#define DRM_IOCTL_IGD_QUERY_OVL        DRM_IOWR(DRM_IGD_QUERY_OVL + BASE,\
		emgd_drm_query_ovl_t)
#define DRM_IOCTL_IGD_QUERY_MODE_LIST  DRM_IOWR(DRM_IGD_QUERY_MODE_LIST + BASE,\
		emgd_drm_query_mode_list_t)
#define DRM_IOCTL_IGD_GET_GOLDEN_HTOTAL  DRM_IOWR(DRM_IGD_GET_GOLDEN_HTOTAL + BASE,\
		emgd_drm_get_golden_htotal_t)
#define DRM_IOCTL_IGD_CONTROL_PLANE_FORMAT DRM_IOWR(DRM_IGD_CONTROL_PLANE_FORMAT + BASE,\
		emgd_drm_control_plane_format_t)
#define DRM_IOCTL_IGD_SET_OVERLAY_DISPLAY DRM_IOWR(DRM_IGD_SET_OVERLAY_DISPLAY + BASE,\
		emgd_drm_set_overlay_display_t)
#define DRM_IOCTL_IGD_SET_ATTRS        DRM_IOWR(DRM_IGD_SET_ATTRS + BASE,\
		emgd_drm_set_attrs_t)
#define DRM_IOCTL_IGD_SET_PALETTE_ENTRY DRM_IOWR(DRM_IGD_SET_PALETTE_ENTRY +\
		BASE,\
		emgd_drm_set_palette_entry_t)
#define DRM_IOCTL_IGD_SET_SURFACE      DRM_IOWR(DRM_IGD_SET_SURFACE + BASE,\
		emgd_drm_set_surface_t)
#define DRM_IOCTL_IGD_SYNC             DRM_IOWR(DRM_IGD_SYNC + BASE,\
		emgd_drm_sync_t)
#define DRM_IOCTL_IGD_DRIVER_PRE_INIT  DRM_IOWR(DRM_IGD_DRIVER_PRE_INIT + BASE,\
		emgd_drm_driver_pre_init_t)
#define DRM_IOCTL_IGD_DRIVER_GET_PORTS DRM_IOWR(DRM_IGD_DRIVER_GET_PORTS + BASE,\
		emgd_drm_driver_get_ports_t)
#define DRM_IOCTL_IGD_START_PVRSRV     DRM_IOWR(DRM_IGD_START_PVRSRV + BASE,\
		emgd_drm_start_pvrsrv_t)
#define DRM_IOCTL_IGD_TEST_PVRSRV      DRM_IOWR(DRM_IGD_TEST_PVRSRV + BASE,\
		emgd_drm_test_pvrsrv_t)

#define DRM_IOCTL_IGD_GET_OVL_INIT_PARAMS DRM_IOWR(DRM_IGD_GET_OVL_INIT_PARAMS + BASE,\
		emgd_drm_get_ovl_init_params_t)
#define DRM_IOCTL_IGD_GET_CHIPSET_INFO DRM_IOWR(DRM_IGD_GET_CHIPSET_INFO + BASE,\
		emgd_drm_driver_get_chipset_info_t)
#define DRM_IOCTL_IGD_QUERY_2D_CAPS_HWHINT DRM_IOWR(DRM_IGD_QUERY_2D_CAPS_HWHINT + BASE,\
		emgd_drm_query_2d_caps_hwhint_t)
#define DRM_IOCTL_IGD_GET_DISPLAY_INFO  DRM_IOR(DRM_IGD_GET_DISPLAY_INFO + BASE,\
		emgd_drm_get_display_info_t)

#define DRM_IOCTL_IGD_DIHCLONE_SET_SURFACE	DRM_IOWR(DRM_IGD_DIHCLONE_SET_SURFACE + BASE,\
		emgd_drm_dihclone_set_surface_t)

#define DRM_IOCTL_IGD_WAIT_VBLANK			DRM_IOWR(DRM_IGD_WAIT_VBLANK + BASE,\
		emgd_drm_driver_set_sync_refresh_t)


/* From pvr_bridge.h */
#define DRM_IOCTL_IGD_RESERVED_1       DRM_IOW(DRM_IGD_RESERVED_1 + BASE, \
		PVRSRV_BRIDGE_PACKAGE)
#define DRM_IOCTL_IGD_RESERVED_2       DRM_IO(DRM_IGD_RESERVED_2 + BASE)
#define DRM_IOCTL_IGD_RESERVED_3       DRM_IO(DRM_IGD_RESERVED_3 + BASE)
#define DRM_IOCTL_IGD_RESERVED_4       DRM_IO(DRM_IGD_RESERVED_4 + BASE)
#define DRM_IOCTL_IGD_RESERVED_5       DRM_IOWR(DRM_IGD_RESERVED_5 + BASE, \
		unsigned long)
#define DRM_IOCTL_IGD_RESERVED_6       DRM_IO(DRM_IGD_RESERVED_6 + BASE)

/* For Video (MXVDX/TOPAZ) */
#define DRM_IOCTL_IGD_VIDEO_CMD_BUF    DRM_IOR(DRM_IGD_VIDEO_CMD_BUF + BASE,\
		emgd_drm_video_cmd_buf_t)
#define DRM_IOCTL_IGD_GET_DEVICE_INFO  DRM_IOR(DRM_IGD_GET_DEVICE_INFO + BASE,\
		emgd_drm_device_info_t)
#define DRM_IOCTL_IGD_GET_PAGE_LIST    DRM_IOR(DRM_IGD_GET_PAGE_LIST + BASE,\
		emgd_drm_get_page_list_t)
#define DRM_IOCTL_IGD_INIT_VIDEO       DRM_IOR(DRM_IGD_INIT_VIDEO + BASE,\
		emgd_drm_init_video_t)
#define DRM_IOCTL_IGD_VIDEO_GET_INFO  DRM_IOR(DRM_IGD_VIDEO_GET_INFO + BASE,\
		emgd_drm_video_get_info_t)
#define DRM_IOCTL_IGD_VIDEO_FLUSH_TLB  DRM_IOR(DRM_IGD_VIDEO_FLUSH_TLB + BASE,\
		emgd_drm_video_flush_tlb_t)
#define DRM_IOCTL_IGD_PREINIT_MMU  DRM_IOR(DRM_IGD_PREINIT_MMU + BASE,\
		emgd_drm_preinit_mmu_t)
#define DRM_IOCTL_IGD_UNLOCK_PLANES  DRM_IOR(DRM_IGD_UNLOCK_PLANES + BASE,\
		emgd_drm_unlock_planes_t)
#define DRM_IOCTL_IGD_GET_DISPLAY_INFO  DRM_IOR(DRM_IGD_GET_DISPLAY_INFO + BASE,\
		emgd_drm_get_display_info_t)
/* For Buffer Class of Texture Stream */
#define DRM_IOCTL_IGD_BC_TS_INIT DRM_IOR(DRM_IGD_BC_TS_INIT + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_UNINIT DRM_IOR(DRM_IGD_BC_TS_UNINIT + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_REQUEST_BUFFERS DRM_IOR(DRM_IGD_BC_TS_REQUEST_BUFFERS + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_RELEASE_BUFFERS DRM_IOR(DRM_IGD_BC_TS_RELEASE_BUFFERS + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_SET_BUFFER_INFO DRM_IOR(DRM_IGD_BC_TS_SET_BUFFER_INFO + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_GET_BUFFERS_COUNT DRM_IOR(DRM_IGD_BC_TS_GET_BUFFERS_COUNT + BASE,\
		emgd_drm_bc_ts_t)
#define DRM_IOCTL_IGD_BC_TS_GET_BUFFER_INDEX DRM_IOR(DRM_IGD_BC_TS_GET_BUFFER_INDEX + BASE,\
		emgd_drm_bc_ts_t)
#endif

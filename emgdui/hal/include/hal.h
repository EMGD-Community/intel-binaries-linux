/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: hal.h
 * $Revision: 1.2 $
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
 *  This file contains data structures, enums and function prototypes of
 *  all the things that are shared in between different HAL layer files, and
 *  also the functions prototypes that are exposed to the IAL layer through
 *  a jump table.
 *-----------------------------------------------------------------------------
 */

#ifndef _HAL_H
#define _HAL_H

#include "halcmn.h"
#include <config.h>

/* Global tables */
extern hal_table_t g_cmnfunc_jump_table[];
extern hal_table_t g_internalfunc_jump_table[];
extern hal_table_t *g_main_jump_table;


/* This structure holds all of the parsed EDID information.*/
#define NUM_TIMINGS                 12
/* Timing structure flag defines */
#define PD_SCAN_INTERLACE           0x80000000
#define PD_HSYNC_HIGH               0x08000000
#define PD_VSYNC_HIGH               0x04000000
#define BIT(x)                      (1 << x)
#define TOTAL_VID_MEM               0x80000
#define EACH_JUMP                   0x800
#define EMGD_LIB_MAJOR              1
#define EMGD_LIB_MINOR              0
#define EMGD_LIB_BUILD              0


#define ARRAY_SIZE(p)               (sizeof(p)/sizeof((p)[0]))
#define GET_TO_MODE(m)              ((m * 4) << 0x0f)
#define GET_DISP_MODE(dc)           (dc & 0xf)
#define VALID_BYTES(num)            (num & 0xFFFF0)
#define GET_PRIMARY_PORT(dc)        ((dc >> 4) & 0x0f)

#ifdef WIN32
#define EXTENDED_TO_EXTENDED_CHECK(current_dc,dc_to_set,set_dc,scrn_info, \
	was_extended) \
	extended_to_extended_check(current_dc,dc_to_set,set_dc,scrn_info, \
	was_extended)
#else
#define EXTENDED_TO_EXTENDED_CHECK(current_dc,dc_to_set,set_dc,scrn_info, \
	was_extended) \
	EXIT_OK
#endif

/*!
 * @name IGD_DC_PORT_NUMBER
 *
 * Given a display configuration value and an index, return the port
 * number at that position.
 */
#define IGD_DC_PORT_NUMBER(dc, i) (unsigned short) ((dc >> (i * 4)) & 0x0f)


enum {

	PRIMARY_MASTER = 1,
	PRIMARY_TWIN1,
	PRIMARY_TWIN2,
	PRIMARY_TWIN3,
	SECONDARY_MASTER,
	SECONDARY_TWIN1,
	SECONDARY_TWIN2,

};

typedef struct _disp_mode_t {

	unsigned long disp_mode;
	char disp_mode_str[MAX_SIZE];

} disp_mode_t;

typedef struct _preferred_mode_t {

	unsigned short width;
	unsigned short height;
	unsigned short refresh;

} preferred_mode_t;

typedef struct _edid_video_in_def_t {

	union {

		struct {

			unsigned char   vsync_pulse:1;
			unsigned char   sync_green:1;
			unsigned char   comp_sync:1;
			unsigned char   separate_sync:1;
			unsigned char   b2b_setup:1;
			unsigned char   sig_lev_std:2;
			unsigned char   analog:1;
		};

		struct {
			unsigned char   dfp:1;
			unsigned char   reserved:6;
			unsigned char   digital:1;
		};
	};

} edid_video_in_def_t;


typedef struct _edid_feature_support_t {

	unsigned char   default_gtf:1;
	unsigned char   pref_timing_mode:1;
	unsigned char   s_rgb_color:1;
	unsigned char   disp_type:2;
	unsigned char   active_off:1;
	unsigned char   suspend:1;
	unsigned char   standby:1;

} edid_feature_support_t;


typedef struct _edid_color_char_t {

	unsigned char   gy:2;
	unsigned char   gx:2;
	unsigned char   ry:2;
	unsigned char   rx:2;

	unsigned char   wy:2;
	unsigned char   wx:2;
	unsigned char   by:2;
	unsigned char   bx:2;

	unsigned char   red_x;
	unsigned char   red_y;
	unsigned char   green_x;
	unsigned char   green_y;
	unsigned char   blue_x;
	unsigned char   blue_y;
	unsigned char   white_x;
	unsigned char   white_y;

} edid_color_char_t;


typedef union _edid_std_timing_t {

	struct {

		unsigned char   horz_active_pixels;
		unsigned char   refresh_rate:6;
		unsigned char   aspect_ratio:2;
	};

	struct {

		unsigned char   byte1;
		unsigned char   byte2;
	};

} edid_std_timing_t;


typedef struct _edid_dtd_t {

	unsigned short  pixel_clock;

	unsigned char   horz_active_low;
	unsigned char   horz_blanking_low;
	unsigned char   horz_blanking_high:4;
	unsigned char   horz_active_high:4;

	unsigned char   vert_active_low;
	unsigned char   vert_blanking_low;
	unsigned char   vert_blanking_high:4;
	unsigned char   vert_active_high:4;

	unsigned char   horz_sync_offset_low;
	unsigned char   horz_sync_pulse_width_low;

	unsigned char   vert_sync_pulse_width_low:4;
	unsigned char   vert_sync_offset_low:4;

	unsigned char   vert_sync_pulse_width_high:2;
	unsigned char   vert_sync_offset_high:2;
	unsigned char   horz_sync_pulse_width_high:2;
	unsigned char   horz_sync_offset_high:2;

	unsigned char   horz_image_size_low;
	unsigned char   vert_image_size_low;

	unsigned char   vert_image_size_high:4;
	unsigned char   horz_image_size_high:4;

	unsigned char   horz_border;
	unsigned char   vert_border;

	unsigned char   flags;

} edid_dtd_t;

typedef struct _edid_mon_desc_t {

	unsigned short  flag1;
	unsigned char   flag2;

	unsigned char   data_type_tag;
	unsigned char   flag3;

	union {

		unsigned char   desc_data[13];

		struct {

			unsigned char   min_vert_rate;
			unsigned char   max_vert_rate;
			unsigned char   min_horz;
			unsigned char   max_horz;
			unsigned char   max_pixel_clock;

			unsigned char   sec_timing_support;
			unsigned char   reserved;
			unsigned char   start_freq;
			unsigned char   c;
			unsigned short  m;
			unsigned char   k;
			unsigned char   j;

		} mon_range_limits;
	};

} edid_mon_desc_t;


typedef struct _edid_t {

	//	Header                              0x00:08
	unsigned char   hdr[8];             //	0x00 0xFF ... 0xFF 0x00

	//	Vendor/Product ID                   0x08:10
	unsigned short  mfg_name;
	unsigned short  prod_code;
	unsigned long   serial_num;
	unsigned char   mfg_week;
	unsigned char   mfg_year;

	//	Edid Structure Version/Revision     0x12:2
	unsigned char   version;
	unsigned char   revision;

	//	Basic Display Parameters/Features   0x14:5
	edid_video_in_def_t video_in_def;
	unsigned char   max_horz_image_size;        //	cm
	unsigned char   max_vert_image_size;        //	cm
	unsigned char   disp_gamma;
	edid_feature_support_t  feature_support;

	//	Color Characteristics               0x19:10
	edid_color_char_t   color_char;

	//	Established Timings                 0x23:3
	unsigned char   est_timings1;
	unsigned char   est_timings2;
	unsigned char   mfg_resv_timings;

	//	Standard Timing Identification      0x26:16
	edid_std_timing_t   std_tmg_id[8];

	//	Detailed Timing Descriptors         0x36:72
	edid_dtd_t  dtd1;

	edid_mon_desc_t mon_desc1;
	edid_mon_desc_t mon_desc2;
	edid_mon_desc_t mon_desc3;

	//	Extension Flag                      0x7E:1
	unsigned char   extFlag;

	//	Checksum                            0x7F:1
	unsigned char   checksum;

} edid_t;

typedef struct _port_type_conv_t {

	char alm_type[MAX_SIZE];
	char newer_type[MAX_SIZE];

} port_type_conv_t;


/* FUNCTIONS EXPOSED FROM THE HAL TO THE IAL */
/************************************************************************/
EXIT_CODE disable_port(IN iegd_hal_info_t *args);
EXIT_CODE enable_port(IN iegd_hal_info_t *args);
EXIT_CODE flip(IN iegd_hal_info_t *args);
EXIT_CODE get_chipset_name(IN iegd_hal_info_t *args);
EXIT_CODE get_cpu_info(IN iegd_hal_info_t *args);
EXIT_CODE get_dc_list_str(IN iegd_hal_info_t *args);
EXIT_CODE get_disp_info(IN iegd_hal_info_t *args);
EXIT_CODE get_disp_port_mapping(IN iegd_hal_info_t *args);
EXIT_CODE get_driver_version(IN iegd_hal_info_t *args);
EXIT_CODE get_drv_mode_list(IN iegd_hal_info_t *args);
EXIT_CODE get_cmos_settings(IN iegd_hal_info_t *args);
EXIT_CODE get_current_disp_mode(IN iegd_hal_info_t *args);
EXIT_CODE get_current_disp_config(IN iegd_hal_info_t *args);
EXIT_CODE get_edid_info(IN iegd_hal_info_t *args);
EXIT_CODE get_os_name(IN iegd_hal_info_t *args);
EXIT_CODE get_port_attrib(IN iegd_hal_info_t *args);
EXIT_CODE get_rot_status(IN iegd_hal_info_t *args);
EXIT_CODE get_sys_bios_version(IN iegd_hal_info_t *args);
EXIT_CODE get_sys_mem(IN iegd_hal_info_t *args);
EXIT_CODE get_cur_dc(IN iegd_hal_info_t *args);
EXIT_CODE rotate(IN iegd_hal_info_t *args);
EXIT_CODE set_display_mode(IN iegd_hal_info_t *args);
EXIT_CODE set_scrn_res(IN iegd_hal_info_t *args);
EXIT_CODE wait(IN iegd_hal_info_t *args);
EXIT_CODE get_video_fps(IN iegd_hal_info_t *args);
/************************************************************************/



/* INTERNAL FUNCTIONS USED BY THE HAL */
/************************************************************************/
unsigned long build_new_dc(IN unsigned long flag);
void conv_dc_to_str(IN unsigned long dc, OUT char *str_dc);
void decode_chipset_name(IN unsigned short chipset, OUT char *chipset_name);
void decode_display_status(IN int dsp_status, OUT char *str_dsp_status);
bool WINAPI decode_int_func(
	IN int func,
	OUT char *str_func);
unsigned long decode_port_num(IN char *port_type);
unsigned long decode_disp_mode(IN char *disp_mode_str);
void print_edid(IN unsigned char *edid);
EXIT_CODE en_dis_port(IN unsigned long is_enable, IN char *port_type);
EXIT_CODE get_cur_dc(OUT unsigned long *current_dc);
EXIT_CODE get_dc_list(
	OUT unsigned long *dc_list,
	IN unsigned long dc_list_size);
void get_display_caps(
	IN unsigned long caps,
	OUT iegd_hal_info_t *args,
	OUT int *index);
EXIT_CODE get_edid_from_file(IN char *filename);
EXIT_CODE get_num_dc(OUT unsigned long *num_dc);
EXIT_CODE get_num_timings(
	IN unsigned long current_dc,
	IN unsigned long port_location,
	OUT unsigned long *num_timings);
EXIT_CODE get_pd_attribs(IN iegd_hal_info_t *args);
bool get_stolen_mem_size(OUT unsigned long *stolen_memory);
EXIT_CODE get_timings(
	IN unsigned long current_dc,
	IN unsigned long mode_size,
	IN unsigned long port_location,
	OUT void *modes);
void init_hw_gen();
void print_mon_desc(IN edid_mon_desc_t *desc);
void print_dtd(IN edid_dtd_t *dtd);
unsigned long ret_intfunc_jump_table_size();
EXIT_CODE set_disp_mode(
	IN unsigned long disp_mode,
	IN unsigned long new_width,
	IN unsigned long new_height,
	IN unsigned long new_refresh,
	IN unsigned long current_dc);
EXIT_CODE set_timings_n_framebuffer(
	IN unsigned long display_mode,
	IN unsigned long mode_size,
	IN void *modes,
	IN iegd_esc_mode_t *new_mode,
	IN unsigned long index,
	OUT void *set_dc);
void strupper(IN char *src, OUT char *dest);
void uninit_internal();
EXIT_CODE change_to_single(
	IN iegd_esc_set_dc_t set_dc);
EXIT_CODE en_dis_port_num(IN unsigned long is_enable, IN int port_num);
EXIT_CODE enable_disable_port(IN iegd_esc_port_ctrl_t port_ctrl);
EXIT_CODE get_screen_info(IN scrn_info_t *disp);

/************************************************************************/

/************************************************************************/

#if !FOR_EXTERNAL_USE

#define INIT_HW_GEN()                    init_hw_gen()
#define UNINIT_INT()                     uninit_internal()

#else

#define INIT_HW_GEN()
#define UNINIT_INT()

#endif

#if !SOURCE_CODE_ONLY

#define DECODE_OS_FUNC(func, str_func)   decode_os_func(func, str_func)
#define RET_OSFUNC_JUMP_TABLE_SIZE()     ret_osfunc_jump_table_size()
#define OSFUNC_JUMP_TABLE                g_osfunc_jump_table
#define OS_GET_CHIPSET_NAME(ci)          os_get_chipset_name(ci)
#define OS_GET_SYS_BIOS_VERSION(args)    os_get_sys_bios_version(args)
#define RET_JUMP_TABLE_SIZE()            ret_intfunc_jump_table_size()
#define INT_FUNC_JUMP_TABLE              g_internalfunc_jump_table
#define DECODE_INT_FUNC(func, str_func)  decode_int_func(func, str_func)
#define RET_OSFUNC_JUMP_TABLE(size_ptr)  ret_osfunc_jump_table(size_ptr)

#else

#define DECODE_OS_FUNC(func, str_func)
#define RET_JUMP_TABLE_SIZE()            0
#define RET_OSFUNC_JUMP_TABLE_SIZE()     0
#define OSFUNC_JUMP_TABLE                NULL
#define OS_GET_CHIPSET_NAME(ci)          true
#define OS_GET_SYS_BIOS_VERSION(args)    true
#define INT_FUNC_JUMP_TABLE              NULL
#define DECODE_INT_FUNC(func, str_func)  false
#define RET_OSFUNC_JUMP_TABLE(size_ptr)  NULL

#endif
/************************************************************************/

#endif

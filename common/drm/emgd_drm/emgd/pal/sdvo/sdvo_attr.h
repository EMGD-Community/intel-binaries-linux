/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_attr.h
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
 *  Definitions for display attribute
 *-----------------------------------------------------------------------------
 */

#ifndef _SDVO_ATTR_H_
#define _SDVO_ATTR_H_

unsigned long sdvo_get_static_attrs(sdvo_device_context_t *p_ctx,   pd_attr_t *p_attr);
sdvo_status_t sdvo_query_static_attr(sdvo_device_context_t *p_ctx,  pd_attr_t *p_attr);
sdvo_status_t sdvo_alter_static_attr(sdvo_device_context_t *p_ctx,  pd_attr_t *p_attr,
	unsigned long new_value);

unsigned long sdvo_get_panel_pwr_seq_attrs(sdvo_device_context_t *p_ctx,   pd_range_attr_t *p_attr);
sdvo_status_t sdvo_query_panel_pwr_seq_attr(sdvo_device_context_t *p_ctx,  pd_range_attr_t *p_attr);
sdvo_status_t sdvo_alter_panel_pwr_seq_attr(sdvo_device_context_t *p_ctx,  pd_range_attr_t *p_attr,
	unsigned long new_value);

unsigned long sdvo_get_range_attrs(sdvo_device_context_t *p_ctx,   pd_range_attr_t *p_attr);
sdvo_status_t sdvo_query_range_attr(sdvo_device_context_t *p_ctx,  pd_range_attr_t *p_attr);
sdvo_status_t sdvo_alter_range_attr(sdvo_device_context_t *p_ctx,  pd_range_attr_t *p_attr,
	unsigned long new_value);

unsigned long sdvo_get_bool_attrs(sdvo_device_context_t *p_ctx,   pd_bool_attr_t *p_attr);
sdvo_status_t sdvo_query_bool_attr(sdvo_device_context_t *p_ctx,  pd_bool_attr_t *p_attr);
sdvo_status_t sdvo_alter_bool_attr(sdvo_device_context_t *p_ctx,  pd_bool_attr_t *p_attr,
	unsigned long new_value);

unsigned long sdvo_get_list_attrs(sdvo_device_context_t *p_ctx,   pd_list_attr_t *p_attr);
sdvo_status_t sdvo_query_list_attr(sdvo_device_context_t *p_ctx,  pd_list_attr_t *p_attr);
sdvo_status_t sdvo_alter_list_attr(sdvo_device_context_t *p_ctx,  pd_list_attr_t *p_attr,
	unsigned long new_value);

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_set_tv_optimal_settings(sdvo_device_context_t *p_ctx, unsigned char chron_tv_code);
#endif

/*	.......................................................................... */
/*	Attribute Definitions */

typedef struct {

	unsigned long id;
	char *p_name;
	unsigned char num_bytes;
	unsigned char opcode_max, opcode_get, opcode_set;

} sdvo_range_attr_data_t;

typedef struct {

	unsigned long id;
	char *p_name;
	unsigned char bit_mask_cur, bit_mask_def;
	unsigned char opcode_get, opcode_set;

} sdvo_bool_attr_data_t;

typedef struct {
	unsigned long id;
	char *p_name;
	unsigned char reg_offset;
	unsigned char value;
} sdvo_attr_list_entry_t;

typedef struct {
	unsigned char num_entries;
	unsigned long id;
	char *p_name;
	unsigned char opcode_get, opcode_set;
	unsigned char bit_masks[8];
	sdvo_attr_list_entry_t *attr_list;
} sdvo_attr_list_header_t;

#ifndef CONFIG_MICRO
typedef struct {
	unsigned char tv_code;
	unsigned char hue;
	unsigned char saturation;
	unsigned char brightness;
	unsigned char contrast;
} sdvo_tv_optimal_settings_t;
#endif

#define PD_LVDS_COLOR_DEPTH_18	0
#define PD_LVDS_COLOR_DEPTH_24	1

#define PD_LVDS_CONNECTOR_SPWG		0
#define PD_LVDS_CONNECTOR_OPEN_LDI	1

#define PD_ANALOG_SRC_NO_DATA			0
#define PD_ANALOG_SRC_PRE_RECORDED		2
#define PD_ANALOG_SRC_NOT_PRE_RECORDED	3

#define PD_SCAN_NO_DATA 		0
#define PD_SCAN_OVERSCANNED 	1
#define PD_SCAN_UNDERSCANNED	2

#define PD_ASPECT_RATIO_NO_DATA 0
#define PD_ASPECT_RATIO_4_3 	1
#define PD_ASPECT_RATIO_16_9	2

#define PD_ACTIVE_FORMAT_NO_DATA							0
#define PD_ACTIVE_FORMAT_SQUARE_PIXELS						8
#define PD_ACTIVE_FORMAT_4_3_CENTER 						9
#define PD_ACTIVE_FORMAT_16_9_CENTER						10
#define PD_ACTIVE_FORMAT_14_9_CENTER						11
#define PD_ACTIVE_FORMAT_16_9_LETTERBOX_TOP 				2
#define PD_ACTIVE_FORMAT_14_9_LETTERBOX_TOP 				3
#define PD_ACTIVE_FORMAT_16_9_LETTERBOX_CENTER				4
#define PD_ACTIVE_FORMAT_4_3_SHOOT_PROTECT_14_9_CENTER		13
#define PD_ACTIVE_FORMAT_16_9_SHOOT_PROTECT_14_9_CENTER 	14
#define PD_ACTIVE_FORMAT_16_9_SHOOT_PROTECT_4_3_CENTER		15

#endif	/*	_SDVO_ATTR_H_ */

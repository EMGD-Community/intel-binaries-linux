/*-----------------------------------------------------------------------------
* Copyright (c) Chrontel Inc.
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
* @file  ch7036_intf.h
* @version 1.2.5
*-----------------------------------------------------------------------------
*/



#ifndef _CH7036_INTF_H_
#define _CH7036_INTF_H_


#include <config.h>
#include <igd_pd.h>
#include <pd.h>
#include <pd_print.h>


#include "ch7036_typedef.h"
#include "ch7036_iic.h"
#include "ch7036.h"

#include "ch7036_fw.h"


#include "config_.h"

#ifdef T_LINUX
#include "asm/div64.h"
#endif


#define internal_lvds_context_t lvds_context_t

typedef unsigned char i2c_reg_t;



#define DITHER_18_TO_18					0
#define DITHER_18_TO_24					1
#define DITHER_24_TO_18					2
#define DITHER_24_TO_24					3


#define DEFAULT_POSITION				2048
#define DEFAULT_H_1080P_POSITION		2137
#define HDMI_DEFAULT_UNDERSCAN			15
#define CRT_DEFAULT_UNDERSCAN			18
#define DEFAULT_ROTATE					0
#define DEFAULT_HFLIP					0
#define DEFAULT_VFLIP					0
#define DEFAULT_TEXT_ENHANCE			7
#define DEF_PLL_REF_DLY					0
#define DEF_PLL_REF_FBDLY				1
#define DEF_LVDS_TXDRV_CTRL				0
#define DITHER_ENABLE					0
#define DITHER_BYPASS					1
#define MODE_6x4_BYPASS					0
#define MODE_8x6_7x4_BYPASS				1


typedef struct {
    i2c_reg_t dev_D0 :1; 
    i2c_reg_t dev_D1 :1;
    i2c_reg_t dev_D2 :1;
    i2c_reg_t dev_D3 :1; 
    i2c_reg_t disp_D0:1; 
    i2c_reg_t disp_D1:1; 
    i2c_reg_t disp_D2:1; 
    i2c_reg_t disp_D3:1; 
} ch7036_power_states_t;



typedef unsigned char ch7036_hpd_t;

#define CH7036HPD_RESERVED1						0x01  //force port status inquiry bit
#define CH7036HPD_CRT_ATTACHED					0x02 
#define CH7036HPD_CRT_STATUS_CHANGED			0x04 
#define CH7036HPD_CRT_EDID_PARSING_STATUS		0x08 

#define CH7036HPD_RESERVED2						0x10 //incorrect display choice bit
#define CH7036HPD_HDVI_ATTACHED					0x20 
#define CH7036HPD_HDVI_STATUS_CHANGED			0x40 
#define CH7036HPD_HDVI_EDID_PARSING_STATUS		0x80 


typedef struct _ch7036_device_context {

	
	void						*internal_lvds;	
	DEV_CONTEXT					*p_ch7xxx_context;  
    pd_callback_t				*p_callback;

	
	pd_attr_t					*p_ch7036_attr_table;	
	pd_attr_t					*p_lvds_attr_table;		

	unsigned long				ch7036_num_attrs; 
	unsigned long				lvds_num_attrs;	
	
	
	HDMI_OUT_MODE 				hdmi_mode_index;
	DVI_OUT_MODE 				dvi_mode_index;
	CRT_OUT_MODE				crt_mode_index;

	
	pd_timing_t					*p_lvds_table; 
	
	
	
	
	pd_timing_t					native_dtd; 

	
	unsigned short				fp_width;		
	unsigned short				fp_height;		

	unsigned char				dither_select; 

	
	unsigned long				pwr_state;		
	
	unsigned char				init_done;		
	
	


	uint8						use_firmware; 

	ch7036_hpd_t				hpd;
	uint8						man_sel_out;  //auto or manually select display output channel

	uint32						prev_outchannel;  
	
	void*						fw;
	void*						cedid; 
	void*						hedid; 
	uint8						downscaled[2]; 
	uint8						dwnscal_bypass; //1: remove 8x6,&7x4 when downscaling, 0: keep them
	
	uint32						last_emsg;	
	
} ch7036_device_context_t;




#if 0
#define PD_INTERNAL_LVDS_MODULE_OPEN(name, params) lvds_open params
#define PD_INTERNAL_LVDS_MODULE_POST_SET_MODE(name, params) lvds_post_set_mode params
#define PD_INTERNAL_LVDS_MODULE_SET_MODE(name, params) lvds_set_mode params
#define PD_INTERNAL_LVDS_MODULE_SET_POWER(name, params) lvds_set_power params
#define PD_INTERNAL_LVDS_MODULE_INIT_DEVICE(name, params) lvds_init_device params
#define PD_INTERNAL_LVDS_MODULE_CLOSE(name, params) lvds_close params
#define PD_INTERNAL_LVDS_MODULE_GET_ATTRIBUTES(name, params) lvds_get_attrs params
#define PD_INTERNAL_LVDS_MODULE_SET_ATTRIBUTES(name, params) lvds_set_attrs params
#define PD_INTERNAL_LVDS_MODULE_GET_TIMING_LIST(name, params) lvds_get_timing_list params

#endif



#define PD_INTERNAL_LVDS_MODULE_OPEN(name, params) internal_lvds_open params
#define PD_INTERNAL_LVDS_MODULE_POST_SET_MODE(name, params) internal_lvds_post_set_mode params
#define PD_INTERNAL_LVDS_MODULE_SET_MODE(name, params) internal_lvds_set_mode params
#define PD_INTERNAL_LVDS_MODULE_SET_POWER(name, params) internal_lvds_set_power params
#define PD_INTERNAL_LVDS_MODULE_GET_POWER(name, params) internal_lvds_get_power params
#define PD_INTERNAL_LVDS_MODULE_INIT_DEVICE(name, params) internal_lvds_init_device params
#define PD_INTERNAL_LVDS_MODULE_CLOSE(name, params) internal_lvds_close params
#define PD_INTERNAL_LVDS_MODULE_GET_ATTRIBUTES(name, params) internal_lvds_get_attrs params
#define PD_INTERNAL_LVDS_MODULE_SET_ATTRIBUTES(name, params) internal_lvds_set_attrs params
#define PD_INTERNAL_LVDS_MODULE_GET_TIMING_LIST(name, params) internal_lvds_get_timing_list params
#define PD_INTERNAL_LVDS_MODULE_SAVE(name, params) internal_lvds_save params
#define PD_INTERNAL_LVDS_MODULE_RESTORE(name, params) internal_lvds_restore params
#define PD_INTERNAL_LVDS_MODULE_GET_PORT_STATUS(name, params) internal_lvds_get_port_status params




ch7036_status_t ch7036_device_prepare(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_device_config(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_device_start(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_device_set_power(ch7036_device_context_t* p_ctx,unsigned long channel);


void ch7036_set_input_timing_info(ch7036_device_context_t *p_ctx,INPUT_INFO* pInput_Info);
void ch7036_set_output_timing_info (ch7036_device_context_t *p_ctx, OUTPUT_INFO* pOutput_Info);
void ch7036_set_prefer_timing_info(ch7036_device_context_t *p_ctx,PREFER_INFO* pPrefer_Info);



ch7036_status_t ch7036_load_firmware(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_get_attached_device(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_read_edid(ch7036_device_context_t* p_ctx, uint32 channel);
ch7036_status_t ch7036_get_hdvi_display_modes_supported(ch7036_device_context_t* p_ctx);


uint8 * ch7036_get_mode_name(uint32 channel, uint8 index);
//void ch7036_reset_edid_supported_modes(unsigned char *p_modes);
ch7036_status_t ch7036_parse_edid(ch7036_device_context_t* p_ctx);
ch7036_status_t ch7036_parse_cea_edid(ch7036_device_context_t* p_ctx);
void ch7036_parse_cea_video_data_block(uint8 blk_size, uint8* p_buff, ch7036_edid_blk_t* p_edid);
ch7036_status_t ch7036_set_edid_display_supported_attr(void *p_table, unsigned long num_attrs, unsigned char* p_downscaled, unsigned char* p_modes,int is_reset);
void ch7036_alter_display_table(int all, void *p_table,unsigned char* p_modes, void* val,unsigned long* p_invis,unsigned char is_invis,unsigned char is_6x4);

ch7036_status_t ch7036_parse_standard_edid(ch7036_device_context_t* p_ctx, uint32 channel);

void ch7036_parse_standard_timing(ch7036_edid_blk_t* p_edid, unsigned char* p_addtional_st);

void ch7036_parse_established_timing(ch7036_device_context_t* p_ctx, ch7036_edid_blk_t* p_edid);
void ch7036_parse_detailed_descriptor_blocks(ch7036_device_context_t* p_ctx, ch7036_edid_blk_t* p_edid);
void ch7036_parse_detailed_timing(OUT_FMT *p_dt, unsigned char* p_ebuf);


void ch7036_reset_mcu(DEV_CONTEXT* p_ch_ctx);
void ch7036_reset_datapath(DEV_CONTEXT* p_ch_ctx);
void ch7036_reset(ch7036_device_context_t* p_ctx);

extern uint32 GetLastErrorMessage(void);

#endif  

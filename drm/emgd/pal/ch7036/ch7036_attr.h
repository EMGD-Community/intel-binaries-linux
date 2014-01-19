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
* @file  ch7036_attr.h
* @version 1.2.5
*-----------------------------------------------------------------------------
*/



//#include <linux/kernel.h>

#ifndef _CH7036_ATTR_H_
#define _CH7036_ATTR_H_

typedef struct {
	unsigned long id;
	char *p_name;
	unsigned long flags; 

} ch7036_attr_list_entry_t;

typedef struct {
	unsigned char num_entries;
	unsigned long id;
	char *p_name;
	unsigned long default_index; 
	unsigned long current_index;
	ch7036_attr_list_entry_t *attr_list;
	
} ch7036_attr_list_header_t;


unsigned long ch7036_enumerate_attr_table(ch7036_device_context_t *p_ctx,pd_attr_t *p_attr,ch7036_attr_list_header_t* p_list_header);
ch7036_status_t ch7036_init_attribute_table(ch7036_device_context_t *p_ctx,ch7036_attr_list_header_t* p_list_header);


void ch7036_set_hdmi_sync_polarity(OUTPUT_INFO* pOutput_Info);
void ch7036_set_audio_type(INPUT_INFO* pInput_Info, uint8 id);
void ch7036_set_scaling (OUTPUT_INFO* pOutput_Info, unsigned long id, uint8 value );
ch7036_status_t ch7036_set_position(ch7036_device_context_t *p_ctx, uint8 attr_id, uint16 pos);


void ch7036_set_rotate (OUTPUT_INFO* pOutput_Info);
void ch7036_set_hflip (OUTPUT_INFO* pOutput_Info);
void ch7036_set_vflip (OUTPUT_INFO* pOutput_Info);

ch7036_status_t ch7036_set_output_channel(void* p_context, uint32 channel);
unsigned long ch7036_get_output_channel(void* p_context);
ch7036_status_t ch7036_alter_display_channel (ch7036_device_context_t *p_ctx);

pd_list_entry_attr_t * ch7036_get_updated_display_ouput_entry(ch7036_device_context_t *p_ctx, uint32 channel);

void ch7036_set_text_enhancement (ch7036_device_context_t *p_ctx, uint8 val);
void ch7036_set_quality_enhancement(ch7036_device_context_t *p_ctx, uint8 val);

void ch7036_set_pll_refdly(PREFER_INFO* pPrefer_Info);
void ch7036_set_pll_fbdly(PREFER_INFO* pPrefer_Info);
void ch7036_set_lvds_txdrv_ctrl(PREFER_INFO* pPrefer_Info);

void ch7036_set_dither(ch7036_device_context_t* p_ctx);

void ch7036_redo_edid_if_needed(ch7036_device_context_t *p_ctx, void* p_edidblk, uint32 channel);


#endif 

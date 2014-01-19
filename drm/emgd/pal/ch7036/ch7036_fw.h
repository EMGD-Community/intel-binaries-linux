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
* @file  ch7036_fw.h
* @version 1.2.2
*-----------------------------------------------------------------------------
*/


#ifndef H_FW_OPER_H
#define H_FW_OPER_H

#include "ch7036_typedef.h"


//#include <linux/kernel.h>

#define	CFG_CAP_EDID	0x01
#define CFG_CAP_HDCP	0x02

typedef unsigned char ch7036_attr_table_index_t;


typedef struct _FW7036_CFG {
    unsigned char size;	 
	unsigned char ver_major;
	unsigned char ver_minor;
	unsigned char did;
	unsigned char rid;
	unsigned char capbility;
	unsigned char reserved;
} FW7036_CFG;


#define LHFM_GET_VERSION		0x1F
#define LHFM_GET_HPD			0x06
#define LHFM_GET_EDID			0x01
#define LHFM_GET_MODEINFO		0x03
#define LHFM_GET_VGA_MODEINFO	0x33
#define LHFM_HDMI_ONOFF		0x04 
#define LHFM_REQUEST		0x40
#define LHFM_RET_ERROR		0x80

int LHFM_get_version(DEV_CONTEXT* p_ch7xxx_context,struct _FW7036_CFG* cfg);

void LHFM_enable_crt_hpd(DEV_CONTEXT* p_ch7xxx_context);
ch7036_status_t LHFM_get_crt_hpd(DEV_CONTEXT* p_ch7xxx_context);
int LHFM_hdmi_onoff(int turnon);
ch7036_status_t LHFM_get_hdmi_modeinfo(DEV_CONTEXT* p_ch7xxx_context,unsigned char *minfo);
ch7036_status_t LHFM_get_edid(DEV_CONTEXT* p_ch7xxx_context,unsigned char*edid, unsigned char* ebn, unsigned char flag);
int LHFM_load_firmware(DEV_CONTEXT* pDevContext);
void ch7036_dump( char *s, int size, unsigned char *regdata);

#define GET_VER					0x01
#define GET_HPD					0x02
#define GET_EDID				0x04
#define GET_HDMI_DISPLAY_MODES	0x08

#define GET_CRT_HPD				0x10
#define GET_CRT_EDID			0x20

#define CH7036_HDMI_DDC			0
#define CH7036_VGA_DDC			0x80

#define MAX_EDID_BLOCKS			2

#define MAX_I2C_BLOCK_SIZE		32




#define NUM_OF_BLOCKS                         0x0F  
#define DTD_SIZE                              0x12  
#define EDID_SIZE                             0x0100
#define EDID_STRUCTURE_VERSION                0x12
#define EDID_STRUCTURE_REVISION               0x13
#define EDID_FEATURE_SUPPORT                  0x18
#define EDID_ESTABLISHED_TIMINGS_1            0x23  
#define EDID_ESTABLISHED_TIMINGS_2            0x24  
#define EDID_MANUFACTURERS_RESERVED_TIMINGS   0x25  
#define EDID_STANDARD_TIMINGS                 0x26  
#define EDID_STANDARD_TIMINGS_BIT_DESCRIPTION 0x27  
#define EDID_DETAILED_TIMING_DESCRIPTION_1    0x36  
#define EDID_DETAILED_TIMING_DESCRIPTION_2    0x48  
#define EDID_DETAILED_TIMING_DESCRIPTION_3    0x5A  

#define EDID_EXTENSION_FLAG                   0x7E  

#define EDID_CEA_TAG                          0x80  
#define EDID_CEA_REVISION                     0x81  
#define EDID_CEA_DETAILED_TIMING_DATA_OFFSET  0x82  
#define EDID_CEA_VIDEO_DATA_BLOCK             0x84  
#define EDID_CEA_DATA_BLOCK					  0x84

#define EDID_CEA_VIDEO_DATA_BLOCK_2           0x96  
#define EDID_CEA_VIDEO_DATA_BLOCK_3           0xA8  
#define EDID_CEA_VIDEO_DATA_BLOCK_4           0xBA  
#define EDID_CEA_VIDEO_DATA_BLOCK_5           0xCC  
#define EDID_CEA_VIDEO_DATA_BLOCK_6           0xDE  

#define EDID_SPACE_INDEX_START                0x18  





typedef struct {
    unsigned char  is_supported;
    char mode_name[13];
	unsigned long index[3]; 

}established_timings_t, *p_established_timings;

typedef struct {
	unsigned int ha; 
	unsigned int va; 
	unsigned char refresh_rate;
}standard_timings_t, *p_standard_timings;



typedef struct {
	unsigned char pclk_lobyte;
	unsigned char pclk_hibyte;
	unsigned char hactive_lobyte;
	unsigned char hblank_lobyte;
	unsigned char hactive_blank;			
	unsigned char vactive_lobyte;
	unsigned char vblank_lobyte;
	unsigned char vactive_blank;			
	unsigned char hsync_off_lobyte;			
	unsigned char hsync_pulse_width;         
	unsigned char vsync_off_pulse_width;	
											
	unsigned char hsopw_vsopw;				
											
											 
											
	unsigned char himage_size_lobyte;		
	unsigned char vimage_size_lobyte;       
	unsigned char hvimage_size;				
											
	unsigned char hborder;
	unsigned char vborder; 
	unsigned char flags;
}dtd_t, *p_dtd;


typedef struct monitor_range {

	unsigned char  hrate_min;           
	unsigned char  hrate_max;            
	unsigned char  vrate_min;           
	unsigned char  vrate_max;            

	unsigned long  pclk_max;           

}monitor_range_t;


typedef struct detailed_timing_block {
	unsigned long data_tag; 
	OUT_FMT		dtiming; 
} dtblks_t;


typedef struct edid_blk {
	

	unsigned char						is_edid; 
	unsigned char						is_preferred_timing;  

	unsigned char						ebn; 
	unsigned char						edidblk[EDID_SIZE]; 

	established_timings_t				*etiming_I;
	established_timings_t				*etiming_II;
	established_timings_t				*etiming_man;

	standard_timings_t					stiming[8];

	standard_timings_t					stiming_x[6]; 
	monitor_range_t						rtiming; 

	dtblks_t							dtblk[4];
	
	
	unsigned char						supported_modes[15]; 
	ch7036_attr_table_index_t			modes[MAX_ATTR_LIST_SIZE]; 

} ch7036_edid_blk_t;


#endif

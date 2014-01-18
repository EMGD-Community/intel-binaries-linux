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
* @file  ch7036_typedef.h
* @version 1.2
*-----------------------------------------------------------------------------
*/


#ifndef _CH7036_TYPEDEF_H
#define _CH7036_TYPEDEF_H



typedef unsigned char				uint8;
typedef unsigned short				uint16;
typedef unsigned int				uint32;



typedef unsigned long long int 		uint64;

typedef signed char					int8;
typedef signed short				int16;
typedef signed int					int32;

typedef signed long long int		int64;







typedef uint32					ch_bool;
#define ch_true					1
#define ch_false				0



typedef struct{
	uint16 ht;
	uint16 ha;
	uint16 ho;
	uint16 hw;
	uint16 vt;
	uint16 va;
	uint16 vo;
	uint16 vw;
	uint16 hz;  
	uint16 stype; 

}TIMING, *PTIMING;


#define PIXEL_FMT_18BIT		0
#define PIXEL_FMT_24BIT		1

#define POL_HIGH			1
#define POL_LOW				0
#define POL_INVERT			1
#define POL_NO_INV			0

#define AUDIO_I2S			1
#define AUDIO_SPDIF			0

#define SCANTYPE_INTERLACED		0
#define SCANTYPE_PROGRESSIVE	1
#define TIMING_LIST_END			2

typedef struct{
	
	TIMING timing;
	uint32 rx_clk_khz;
	uint8  pixel_fmt;	
	uint8  hs_pol;		
	uint8  vs_pol;
	uint8  de_pol;
	uint8  data_ch_pol;	
	uint8  data_ch_invert;
	
	uint8  audio_type;	
	uint8  i2s_pol;
	uint8  i2s_len;
	uint8  i2s_fmt;
}INPUT_INFO, *PINPUT_INFO;




#define LVDS_DATA0_SEL			0
#define LVDS_DATA1_SEL			1
#define LVDS_DATA2_SEL			2
#define LVDS_DATA3_SEL			3
#define LVDS_CLOCK_SEL			4


#define LVDS_CHANNEL_SWAP_DEF		(LVDS_DATA0_SEL << 0) | (LVDS_DATA1_SEL << 4) | (LVDS_DATA2_SEL << 8) | (LVDS_DATA3_SEL << 12) | (LVDS_CLOCK_SEL << 16)
#define LVDS_CHANNEL_SWAP_OP1		(LVDS_DATA1_SEL << 0) | (LVDS_DATA2_SEL << 4) | (LVDS_DATA3_SEL << 8) | (LVDS_CLOCK_SEL << 12) | (LVDS_DATA0_SEL << 16)
#define LVDS_CHANNEL_SWAP_OP2		(LVDS_DATA2_SEL << 0) | (LVDS_DATA3_SEL << 4) | (LVDS_CLOCK_SEL << 8) | (LVDS_DATA0_SEL << 12) | (LVDS_DATA1_SEL << 16)
#define LVDS_CHANNEL_SWAP_OP3		(LVDS_DATA3_SEL << 0) | (LVDS_CLOCK_SEL << 4) | (LVDS_DATA0_SEL << 8) | (LVDS_DATA1_SEL << 12) | (LVDS_DATA2_SEL << 16)
#define LVDS_CHANNEL_SWAP_OP4		(LVDS_CLOCK_SEL << 0) | (LVDS_DATA0_SEL << 4) | (LVDS_DATA1_SEL << 8) | (LVDS_DATA2_SEL << 12) | (LVDS_DATA3_SEL << 16)
#define LVDS_CHANNAL_SWAP_OP5		(LVDS_DATA3_SEL << 0) | (LVDS_CLOCK_SEL << 4) | (LVDS_DATA2_SEL << 8) | (LVDS_DATA1_SEL << 12) | (LVDS_DATA0_SEL << 16)

typedef struct{
	uint32 channel_swap;	
	uint32 channel_pol;		
	uint32 pixel_fmt;		
}LVDS_FMT, *PLVDS_FMT;

#define AS_RATIO_4_3		1
#define AS_RATIO_16_9		2

typedef struct{
	uint8 is_dvi_mode;		
	uint8 format_index;		
	uint8 aspect_ratio;		
	uint8 channel_swap;		
	uint8 data_pol_invert;	
	uint8 hs_pol;			
	uint8 vs_pol;			
	uint8 protect_enable;	
}HDMI_FMT, *PHDMI_FMT;

typedef struct{
	uint8 channel_swap;		
	uint8 hs_pol;			
	uint8 vs_pol;			
	uint8 de_pol;			
}VGA_FMT, *PVGA_FMT;


#define CHANNEL_LVDS		(1 << 0)
#define CHANNEL_HDMI		(1 << 1)
#define CHANNEL_VGA			(1 << 2)
#define CHANNEL_DVI			(1 << 3)
#define CHANNEL_LVDS_HDMI 	(CHANNEL_LVDS | CHANNEL_HDMI)
#define CHANNEL_LVDS_VGA 	(CHANNEL_LVDS | CHANNEL_VGA)
#define CHANNEL_LVDS_DVI 	(CHANNEL_LVDS | CHANNEL_DVI)  
#define CHANNEL_AUTO_DETECT	0x10 


#define CHANNEL_LVDS_HDMI_VGA_OFF	0x0000  



#define ROTATE_0			0
#define ROTATE_NO			0
#define ROTATE_90			1
#define ROTATE_180			2
#define ROTATE_270			3

typedef struct{
	uint32 channel;			
	LVDS_FMT lvds_fmt;		
	HDMI_FMT hdmi_fmt;		
	VGA_FMT vga_fmt;		
	TIMING timing;			
	uint32 uclk_khz;		
	uint8  ds_percent_h;	
	uint8  ds_percent_v;	
	uint8  rotate;			
	uint8  h_flip;			
	uint8  v_flip;			
	uint16  h_position;			
	uint16  v_position;			
}OUTPUT_INFO, *POUTPUT_INFO;


#define HS_TOLERANCE_LEVEL0			0
#define HS_TOLERANCE_LEVEL1			1
#define HS_TOLERANCE_LEVEL2			3
#define HS_TOLERANCE_LEVEL3			7

#define RST_BIT_HSYNC				0
#define RST_BIT_VSYNC				1

#define MEM_CLK_FREQ_MAX			166000	
#define FBA_INC_MAX					3000	

#define THRRL_ADJUST_DEF			200	

#define THRRL_ADJUST_OP1			250		
#define THRRL_ADJUST_OP2			300		

typedef struct{
	uint32 mclk_khz;		
	
	uint8 uclkod_sel;		
	uint8 dat16_32b;		
	uint8 true24;			
	uint8 true_com;			
	
	uint8 lvds_out_hs_tolerance;	
	uint8 lvds_out_reset_bit_sel;	
	
	uint8 dither_filter_enable;		
	uint8 hscale_ratio_gate;		
	
	uint8 scale_line_adjust;		
	uint8  text_enhancement;	
			
	uint8 pll_ref_dly;
	uint8 pll_ref_fbdly;
	uint8 lvds_txdrv_ctrl;

	uint8 eye_bgtrim ;
	uint8 eye_dacg;
	uint8 eye_dri_demp;
	uint8 eye_dri_pll_cp;
	uint8 eye_dri_damp;
	uint8 eye_dri_pll_rlf;
	uint8 eye_rdac;
	
	uint8 reset; 
	uint8 vga_enable;

}PREFER_INFO, *PPREFER_INFO;



typedef struct{
	uint32 DeviceID;
	INPUT_INFO* pInput_Info;
	OUTPUT_INFO* pOutput_Info;
	PREFER_INFO* pPrefer_Info;
	void*	pd_context; 
}DEV_CONTEXT, *PDEV_CONTEXT;

typedef struct{
	uint32 fmt_index;
	uint32 clk_freq;
	uint32 aspect;
	TIMING timing;
}OUT_FMT, *POUT_FMT;

typedef enum hdmi{

	OUT_HDMI_640x480P_59 = 0,
	OUT_HDMI_640x480P_60,

	OUT_HDMI_720x480P_59,
	OUT_HDMI_720x480P_60, 

	OUT_HDMI_720x576P_50,

	OUT_HDMI_1280x720P_59,
	OUT_HDMI_1280x720P_60, 

	OUT_HDMI_1920x1080I_59, 
	OUT_HDMI_1920x1080I_60, 

	OUT_HDMI_1920x1080P_59,
	OUT_HDMI_1920x1080P_60, 

//	OUT_HDMI_720x576P_50, //2/2/12 grouped to downscaling modes < 13x7
	OUT_HDMI_1280x720P_50,
	OUT_HDMI_1920x1080I_50,
	
	OUT_HDMI_1920x1080P_50,

	OUT_HDMI_1920x1080P_23,
	OUT_HDMI_1920x1080P_24,
	OUT_HDMI_1920x1080P_25,
	OUT_HDMI_1920x1080P_29,
	OUT_HDMI_1920x1080P_30,

	OUT_HDMI_1920x1080I_100,
	OUT_HDMI_1280x720P_100,
	OUT_HDMI_720x576P_100,
	
	OUT_HDMI_1920x1080I_119,
	OUT_HDMI_1920x1080I_120,
    
	OUT_HDMI_1280x720P_119,
	OUT_HDMI_1280x720P_120,

	OUT_HDMI_720x480P_119,
	OUT_HDMI_720x480P_120,

	OUT_HDMI_720x576P_200,
    OUT_HDMI_1280x720P_239,
	OUT_HDMI_1280x720P_240, 
	OUT_HDMI_END,
}HDMI_OUT_MODE;

typedef enum dvi {

	OUT_DVI_640x480_60 = 0,

	OUT_DVI_640x480_72,
	OUT_DVI_640x480_75,
	OUT_DVI_720x400_70,

	OUT_DVI_800x600_56,
    OUT_DVI_800x600_60,
	OUT_DVI_800x600_72,
	OUT_DVI_800x600_75,

	OUT_DVI_1024x768_60,
	OUT_DVI_1024x768_70,
	OUT_DVI_1024x768_75,

    OUT_DVI_1152x864_60,
    OUT_DVI_1280x720_60,
	OUT_DVI_1280x800_60,
	OUT_DVI_1280x960_60,
	OUT_DVI_1280x1024_60,
	OUT_DVI_1280x1024_75,
	OUT_DVI_1360x768_60,
	OUT_DVI_1366x768_60,

	OUT_DVI_1400x1050_60,
	OUT_DVI_1400x1050_75, 
	

	OUT_DVI_1440x900_60,
	OUT_DVI_1440x1050_60,
	OUT_DVI_1600x900_60,
	OUT_DVI_1600x1200_60,
	OUT_DVI_1680x1050_60,

	OUT_DVI_1920x1080_60, 

	OUT_DVI_1920x1200_60, 

	OUT_DVI_END,
} DVI_OUT_MODE;

typedef enum crt{

	OUT_CRT_640x400_85 = 0,

	OUT_CRT_640x480_60,
	
	OUT_CRT_640x480_72,
	OUT_CRT_640x480_75,
	OUT_CRT_640x480_85,
	
	OUT_CRT_720x400_85,

	OUT_CRT_800x600_56,

	    
	OUT_CRT_800x600_60, 
	OUT_CRT_800x600_72,
	OUT_CRT_800x600_75,
	OUT_CRT_800x600_85, 

	OUT_CRT_1024x768_60,
	OUT_CRT_1024x768_70,
	OUT_CRT_1024x768_75, 
	OUT_CRT_1024x768_85, 
	

	
	OUT_CRT_1152x864_75,
	
	OUT_CRT_1280x768_60,  
	OUT_CRT_1280x768_75,  
	OUT_CRT_1280x768_85,  
	
	OUT_CRT_1280x960_60,
	OUT_CRT_1280x960_85, 

	OUT_CRT_1280x1024_60,
	OUT_CRT_1280x1024_75,
	OUT_CRT_1280x1024_85, 

	OUT_CRT_1360x768_60,   

	OUT_CRT_1400x1050_60,
	OUT_CRT_1400x1050_75,
	


	OUT_CRT_1440x900_60,
	OUT_CRT_1440x1050_60,

	OUT_CRT_1600x900_60, 
	OUT_CRT_1600x1200_60,  
	
	OUT_CRT_1920x1080_60, 

	

	OUT_CRT_END,
}CRT_OUT_MODE;

#define MAX_ATTR_LIST_SIZE 32




#define ERR_NO_ERROR						0

#define ERR_DEVICE_PREPARE					100
#define ERR_RESOLUTION_H_ODD				101
#define ERR_RESOLUTION_V_ODD				102
#define ERR_BANDWIDTH_OVERFLOW				103
#define ERR_RESOLUTION_OVERFLOW				104
#define ERR_DEVICE_NO_EXIST					105
#define ERR_NO_SUPPORT_TRUE24				106
#define ERR_NO_SUPPORT_TRUECOM				107
#define ERR_ROTATION_WITH_VAI				108 
#define ERR_FLIP_WITH_VAI					109

#define ERR_DEVICE_CONFIG					200
#define ERR_ASPECT_RATIO_NOMATCH			201
#define ERR_PLL1N2_WRONG					202
#define ERR_PLL1N3_WRONG					203
#define ERR_PLL1N1_WRONG					204
#define ERR_PLL3N8_WRONG					205
#define ERR_PLL2N5_WRONG					206
#define ERR_PLL2N6_WRONG					207
#define ERR_PLL2N7_WRONG					208
#define ERR_DMXTAL_WRONG					209
#define ERR_HDMI_CLOCK_NO_SUPPORT			210	

#define ERR_DEVICE_RUNNING					300
#define ERR_CAL_INC_DIV_ZERO				301		
#define	ERR_CAL_INC_NO_MATH					302



typedef enum {
    SS_POWER_ON_STATE,       
    SS_SUCCESS,              
    SS_NOT_SUPPORTED,        
    SS_INVALID_ARGUMENT,     
    SS_PENDING,              
    SS_TARGET_UNSPECIFIED,   
    SS_SCALING_UNSUPPORTED,  

	SS_MEM_ALLOC_ERR,

    
    SS_UNSUCCESSFUL = 10,    
    SS_INVALID_RETURN,       
    SS_WRITE_FAILED,         
    SS_READ_FAILED,          

	SS_FIRMWARE_ERR,
	SS_FIRMWARE_TIMEOUT,

	SS_CRT_HPD_NOTCONNECTED,
	SS_CRT_HPD_CONNECTED_TO_GRD,
	SS_DISPLAY_CHOICE_NOT_ALLOWED,


} ch7036_status_t;

#endif

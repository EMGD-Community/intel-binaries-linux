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
* @file  ch7036_reg_table.c
* @version 1.2.2
*-----------------------------------------------------------------------------
*/



#include "ch7036_reg_table.h"




MULTI_REG g_MultiRegTable[MUL_ID_END] = 
{
	{BASE_FLAG,       NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  3, 6,	NOOP_INDEX,   0, 7,		0},
	
	{HTI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0B,3, 6,	0x0D, 0, 7,		0},
	{VTI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x11,3, 5,	0x13, 0, 7,		0},
	{HAI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0B,0, 2,	0x0C, 0, 7,		0},
	{VAI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x11,0, 2,	0x12, 0, 7,		0},
	{HOI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0E,0, 2,	0x0F, 0, 7,		0},
	{VOI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x14,0, 2,	0x15, 0, 7,		0},
	{HWI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0E,3, 5,	0x10, 0, 7,		0},
	{VWI,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x14,3, 5,	0x16, 0, 7,		0},
	{RCLK,            NOOP_INDEX,  0, 0,	0x0F,0, 1,  0x14,0, 7,  0x15, 0, 7,     4},
	{DITHER_SEL1_SPP, NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x57, 7, 7,		4},	
	{DITHER_SEL0_SPP, NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x57, 6, 6,		4},
	{HSYNCP_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x41, 6, 6,		1},
	{VSYNCP_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x41, 5, 5,		1},
	{NP_INV0_SPP,     NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x58, 4, 4,		4},	
	{NP_INV1_SPP,     NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x58, 3, 3,		4},	
	{NP_INV2_SPP,     NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x58, 2, 2,		4},	
	{NP_INV3_SPP,     NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x58, 1, 1,		4},	
	{LVDS_IN_ORDER,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,	 0, 0,  NOOP_INDEX,  0, 0,  0x5F, 0, 0,		4},
	
	{LVDS0_SEQ_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5E, 5, 7,		4},	
	{LVDS1_SEQ_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5E, 2, 4,		4},	
	{LVDS2_SEQ_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,	 0, 0,  0x5F, 7, 7,		4},	
	{LVDS3_SEQ_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5F, 4, 6,		4},	
	{LVDSCLK_SEQ_SPP, NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5F, 1, 3,		4},	
	{HSYNC_CNT_TH_SPP,NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 5, 6,		4},	
	{PRBS_SET_SEL_SPP,NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x61, 1, 1,		4},
	{LVDS0_POL_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 4, 4,		4},	
    {LVDS1_POL_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 3, 3,		4},	
	{LVDS2_POL_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 2, 2,		4},	
	{LVDS3_POL_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 1, 1,		4},	
	{LVDSCLK_POL_SPP, NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x59, 0, 0,		4},	
	
	{HTO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1F,3, 6,	0x21, 0, 7,		0},
	{VTO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x25,3, 5,	0x27, 0, 7,		0},
	{HAO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1F,0, 2,	0x20, 0, 7,		0},
	{VAO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x25,0, 2,	0x26, 0, 7,		0},
	{HOO_HDMI,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x54,0, 2,	0x55, 0, 7,		0},
	{VOO_HDMI,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x57,0, 2,	0x58, 0, 7,		0},
	{HWO_HDMI,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x54,3, 5,	0x56, 0, 7,		0},
	{VWO_HDMI,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x57,3, 5,	0x59, 0, 7,		0},
	{HPO_I,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x19, 5, 5,		0},
	{VPO_I,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x19, 4, 4,		0},
	{DEPO_I,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x19, 3, 3,		0},
	{HPO_O,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 4, 4,		0},
	{VPO_O,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 3, 3,		0},
	{DEPO_O,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 5, 5,		0},
	
	{HD_DVIB,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0A, 2, 2,		0},
	{INTLC,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1E, 5, 5,		0},
	{HD_LV_POL,       NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x4C, 5, 5,		0},
	{HD_LV_SEQ,       NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x4C, 0, 4,		0},
	{HDMI_LVDS_SEL,   NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x7E, 7, 7,		0},
	{VSP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x24, 5, 5,		1},
	{HSP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x24, 4, 4,		1},
	{HDMIIN_HSP,	  NOOP_INDEX,  0, 0,    NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x61, 6, 6,		4},
	{HDMIIN_VSP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x61, 5, 5,		4},
	{HDMIIN_DEP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x61, 4, 4,		4},
	{M1M0,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x3D, 2, 3,		0},
	{C1C0,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x3D, 0, 1,		0},
	{VIC,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x40, 0, 5,		0},
	{COPY,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x4B, 6, 6,		0},
	{SCAN_EN,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6C, 0, 0,		4},
	{HAO_SCL,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6A,0, 7,	0x6B, 5, 7,		4},
	{VAO_SCL,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6B,0, 4,	0x6C, 2, 7,		4},
	
	{I2SPOL,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1E, 7, 7,		0},
	{I2S_SPDIFB,      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1E, 6, 6,		0},
	{I2S_LENGTH,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1E, 2, 3,		0},
	{I2SFMT,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1E, 0, 1,		0},
	{UCLK,            NOOP_INDEX,  0, 0,	0x0F,4, 5,  0x10,0, 7,  0x11, 0, 7 ,    4},
	
	{PCLK_NUM,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x28,  0, 7,0x29, 0, 7,		1},	
	
	{UCLKSEC,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x12, 4, 4,		1},
	{PLL1N1,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x55, 3, 5,		4},	
	{PLL1N2,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x55, 0, 2,		4},	
	{PLL1N3,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x56, 5, 7,		4},	
	{PLL3N8,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x13, 1, 2,		1},
	{A2,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5E, 0, 7,		0},
	{MCLK,            NOOP_INDEX,  0, 0,	0x0F,2, 3,  0x12,0, 7,  0x13, 0, 7,     4},
	{PLL2N5,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,  0x12,0, 1,	0x13, 7, 7,		1},
	{PLL2N54,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6E, 2, 2,		4},
	{PLL2N53,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6E, 1, 1,		4},
	{PLL2N6,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x69, 5, 6,		4},
	{PLL2N7,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x69, 3, 4,		4},
	{DIVXTAL,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6E, 3, 7,		4},
	{A1,			  0x5A,0, 7,	0x5B,0, 7,	0x5C,0, 7,	0x5D, 0, 7,		0},
	{A3,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x60, 0, 7,		4},
	{TXPLL_FFD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x67, 7, 7,		4},
	{DRI_PLL_N1,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0C, 3, 4,		1},
	{DRI_PLL_N3,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0C, 1, 2,		1},
	{UCLKOD_SEL,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x61, 7, 7,		4},

	{GCKSEL,          NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x54, 6, 6,		4},
	{TSTEN1,          NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x14, 6, 6,		1},
	
	{LNSEL,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x68, 1, 2,		0},
	{DAT16_32B,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x74, 7, 7,		0},
	{TRUE24,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x74, 3, 3,		0},
	{TRUE_COM,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x74, 4, 4,		0},
	{WRLEN,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2D, 6, 7,		0},
	{ROTATE,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2D, 4, 5,		0},
	{HFLIP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 7, 7,		0},
	{VFLIP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 6, 6,		0},
	{DNSMPEN,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x25, 6, 6,		0},
	{HADWSPP,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x60,0, 7,	0x61, 0, 2,		0},
	{FLTBP2,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x32, 7, 7,		0},
	{FLTBP1,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x32, 6, 6,		0},
	{BLK_H,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x64, 0, 6,		0},
	{FBA_INC,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6B,0, 7,	0x6C, 0, 3,		0},
	{SFM,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6A, 0, 0,		0},
	{THREN,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6A, 2, 2,		0},
	{THRRL,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6D,0, 7,	0x6E, 0, 2,		0},
	{WRFAST,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x6E, 3, 3,		0},
	{CHG_HL,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2F, 7, 7,		0},
	{HINCA,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2A,0, 7,	0x2B, 0, 2,		4},
	{HINCB,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2C,0, 7,	0x2D, 0, 2,		4},
	{VINCA,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E,0, 7,	0x2F, 0, 2,		4},
	{VINCB,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x30,0, 7,	0x31, 0, 2,		4},
	{HDINCA,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x32,0, 7,	0x33, 0, 2,		4},
	{HDINCB,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x34,0, 7,	0x35, 0, 2,		4},
	{HINC,			  NOOP_INDEX,  0, 0,	0x36,0, 4,	0x37,0, 7,	0x38, 0, 7,		4},
	{VINC,			  NOOP_INDEX,  0, 0,	0x39,0, 7,	0x3A,0, 7,  0x3B, 0, 7,		4},
	{HDINC,		      NOOP_INDEX,  0, 0,	0x3C,0, 7,	0x3D,0, 7,	0x3E, 0, 7,		4},
	{VSMST,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x70, 6, 7,		0},
	{MEMINIT,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0A, 7, 7,		0},
	{STOP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0A, 4, 4,		0},
	
	{HDMI_PD,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 0, 0,		0},
	{I2S_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 6, 6,		0},
	{SPDIF_PD,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 0, 0,		0},
	{DRI_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 3, 3,		0},
	{DRI_PD_SER,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x16, 0, 0,		1},
	{DRI_PD_PLL,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x16, 3, 3,		1}, 
	{DRI_PDDRI,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x08, 4, 7,		0},
	{CEC_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 4, 4,		0},
	{PD_DDC,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0E, 7, 7,		1},
	
	{LVDS_PD,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0A, 3, 3,		0},
	{RX_PD,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x63, 4, 7,		4},
	{RXPLL_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x64, 0, 0,		4},
    {TXPLL_PD,		  NOOP_INDEX,  0, 0,    NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x66, 1, 1,		4},
	{TXDRV_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x11, 4, 4,		1},
	{TXSER_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x1A, 4, 4,		1}, 
	
	{VGA_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 2, 2,		0},
	{PDDAC,		      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x08, 1, 3,		0},
	{DACSENCE,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x57, 1, 1,		4},
	{GCKOFF,          NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 6, 6,		0},
	{TV_BP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 5, 5,		0},
	{PDPLL1,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 1, 1,		0},
	{PDPLL0,	      NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x54, 4, 4,		4},

	{ICEN0,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x11, 7, 7,		1},

	{PD_PROM,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0F, 7, 7,		1},
	{PDMIO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 2, 2,		0},
	{PDIO,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 5, 5,		0},
	{HPD_PD,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 7, 7,		0},
	{SCLPD,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 4, 4,		0},
	{SDPD,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x09, 3, 3,		0},
	{MEMPD,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x0A, 5, 5,		0},
	{AUDDAC,          NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x5C, 2, 2,		4},
	
	{CRYS_FREQ_SPP,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,0x1C,	 0,	7,  0x1D, 0, 7,		0},
	{I2SCK_SEC, 	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 2, 2,		1},
	{SP_EN,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x21, 7, 7,		1},
	{HARD_SOFTB,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x58, 5, 5,		4},
	{MULT_I2CEN,	  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x4E, 0, 0,		1},
	{RXPLL_REFDLY,    NOOP_INDEX,  0, 0,    NOOP_INDEX,  0, 0,  NOOP_INDEX,  0, 0,  0x66, 5, 7,     4},
	{RXPLL_FBDLY,     NOOP_INDEX,  0, 0,    NOOP_INDEX,  0, 0,  NOOP_INDEX,  0, 0,  0x66, 2, 4,     4},
	{VFMT,            NOOP_INDEX,  0, 0,    NOOP_INDEX,  0, 0,  NOOP_INDEX,  0, 0,  0x2B, 0, 3,     0},

	{DBP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x19, 6, 6,		0},
	{CK_TVINV,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x07, 1, 1,		1},
	{DISPON,		  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x08, 0, 0,		1},

	{VP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x39,4, 7,	0x3B, 0, 7,		0},
	{HP,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x39,0, 3,	0x3A, 0, 7,		0},

	{TXTEN,			  NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x2E, 0, 2,		0},
	{ZRCTS,           NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	NOOP_INDEX,  0, 0,	0x24, 1, 1,		1},
	
};



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
* @file  ch7036_reg_table.h
* @version 1.2
*-----------------------------------------------------------------------------
*/

#ifndef _CH7036_REG_TABLE_H
#define _CH7036_REG_TABLE_H

#include "ch7036_typedef.h"

typedef enum{
	BASE_FLAG = 0,

	
	HTI,	VTI,	HAI,	VAI,	HOI,	VOI,	HWI,	VWI,	RCLK, 
	DITHER_SEL1_SPP,		DITHER_SEL0_SPP,		HSYNCP_SPP,		VSYNCP_SPP,
	NP_INV0_SPP,	NP_INV1_SPP,	NP_INV2_SPP,	NP_INV3_SPP,	LVDS_IN_ORDER,

	
	LVDS0_SEQ_SPP,	LVDS1_SEQ_SPP,	LVDS2_SEQ_SPP,	LVDS3_SEQ_SPP,	LVDSCLK_SEQ_SPP,
	HSYNC_CNT_TH_SPP,				PRBS_SET_SEL_SPP,				
	LVDS0_POL_SPP,	LVDS1_POL_SPP,	LVDS2_POL_SPP,	LVDS3_POL_SPP,	LVDSCLK_POL_SPP,		

	
	HTO,	VTO,	HAO,	VAO,	HOO_HDMI,		VOO_HDMI,		HWO_HDMI,	VWO_HDMI,	
	HPO_I,	VPO_I,	DEPO_I,	HPO_O,	VPO_O,			DEPO_O,	

	
	HD_DVIB,		INTLC,			HD_LV_POL,		HD_LV_SEQ,		HDMI_LVDS_SEL, 
	VSP,			HSP,			HDMIIN_HSP,		HDMIIN_VSP,		HDMIIN_DEP,
	M1M0,			C1C0,			VIC,
	COPY,			SCAN_EN,		HAO_SCL,		VAO_SCL,

	
	I2SPOL,			I2S_SPDIFB,		I2S_LENGTH,		I2SFMT,		
	UCLK,			PCLK_NUM,

	
	UCLKSEC,		PLL1N1,			PLL1N2,			PLL1N3,			PLL3N8,		A2,			MCLK, 
	PLL2N5,			PLL2N54,		PLL2N53,		PLL2N6,			PLL2N7,		DIVXTAL,
	A1,				A3,				TXPLL_FFD,		DRI_PLL_N1,		DRI_PLL_N3, UCLKOD_SEL,
	GCKSEL,         TSTEN1, 

	
	LNSEL,			DAT16_32B,		TRUE24,			TRUE_COM,		WRLEN,	
	ROTATE,			HFLIP,			VFLIP,
	DNSMPEN,		HADWSPP,		FLTBP2,			FLTBP1,			BLK_H,
	FBA_INC,		SFM,			THREN,			THRRL,			WRFAST,		CHG_HL,
	HINCA,			HINCB,			VINCA,			VINCB,			HDINCA,		HDINCB,
	HINC,			VINC,			HDINC,
	VSMST,			MEMINIT,		STOP,						
	
	
	HDMI_PD,		I2S_PD,			SPDIF_PD,		DRI_PD,			DRI_PD_SER,
	DRI_PD_PLL,		DRI_PDDRI,		CEC_PD,			PD_DDC,	
	LVDS_PD,		RX_PD,			RXPLL_PD,		TXPLL_PD,		TXDRV_PD,	TXSER_PD,	
	VGA_PD,			PDDAC,			DACSENCE,       GCKOFF, 
	TV_BP,			PDPLL1,			PDPLL0,			ICEN0,			PD_PROM,		PDMIO,		PDIO,		HPD_PD,
	SCLPD,			SDPD,			MEMPD,			AUDDAC,

	
	CRYS_FREQ_SPP,	I2SCK_SEC,		SP_EN,			HARD_SOFTB,		MULT_I2CEN,	RXPLL_REFDLY,
	RXPLL_FBDLY, VFMT,

	
	DBP,			CK_TVINV,		DISPON,
	
	VP,	HP,
	
	TXTEN,
	ZRCTS,
	
	MUL_ID_END,
	
}MULTI_REG_ID;

typedef struct{
	MULTI_REG_ID RegId;
	uint8 TopRegIndex;
	uint8 TopStartBit;
	uint8 TopEndBit;
	uint8 HighRegIndex;
	uint8 HighStartBit;
	uint8 HighEndBit;
	uint8 MiddleRegIndex;
	uint8 MiddleStartBit;
	uint8 MiddleEndBit;
	uint8 LowRegIndex;
	uint8 LowStartBit;
	uint8 LowEndBit;
	uint8 PageIndex;
}MULTI_REG, *PMULTI_REG;

extern MULTI_REG g_MultiRegTable[];

//issue- 11/15/11- thua 
//Intel C compiler complained negative constant (-1) comparison to unsigned char for checked build verion
//for free build, these warnings are treated as compiling error
//solution:
//define NOOP_REGINDEX as 2-complement of -1
#define NOOP_INDEX 0xFF

#endif

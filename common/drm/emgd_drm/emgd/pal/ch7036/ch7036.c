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
* @file  ch7036.c
* @version 1.2.2.1
*-----------------------------------------------------------------------------
*/



#include "ch7036_intf.h"


static uint32 g_nLastError;



static ch_bool convert_pll1n1_div(ch_bool pll1n1_to_div, uint8* pll1n1_addr, uint8* div_addr); 
static ch_bool convert_pll1n2_div(ch_bool pll1n2_to_div, uint8* pll1n2_addr, uint8* div_addr);
static ch_bool convert_pll1n3_div(ch_bool pll1n3_to_div, uint8* pll1n3_addr, uint8* div_addr);
static ch_bool convert_pll2n5_div(ch_bool pll2n5_to_div, uint8* pll2n5_addr, uint8* div_addr);
static ch_bool convert_pll2n6_div(ch_bool pll2n6_to_div, uint8* pll2n6_addr, uint8* div_addr);
static ch_bool convert_pll2n7_div(ch_bool pll2n7_to_div, uint8* pll2n7_addr, uint8* div_addr);
static ch_bool convert_pll3n8_div(ch_bool pll3n8_to_div, uint8* pll3n8_addr, uint8* div_addr);
static ch_bool convert_dmxtal_div(ch_bool dmxtal_to_div, uint8* dmxtal_addr, uint8* div_addr);


static ch_bool set_input_info(INPUT_INFO* pInput_Info);


static ch_bool set_output_info(OUTPUT_INFO* pOutput_Info);


static ch_bool set_prefer_info(PREFER_INFO* pPrefer_Info);


static ch_bool cal_and_set_clk_pll(DEV_CONTEXT* pDevContext);


static ch_bool cal_and_set_scaler(DEV_CONTEXT* pDevContext);


static ch_bool cal_and_set_power(DEV_CONTEXT* pDevContext);


static ch_bool post_cal_and_set(DEV_CONTEXT* pDevContext);


extern void ch7036_set_power_lvds(DEV_CONTEXT* pDevContext);
extern void ch7036_set_power_hdmi(DEV_CONTEXT* pDevContext);
extern void ch7036_set_power_crt(DEV_CONTEXT* pDevContext);


#define  DOUBLE_TO_INT		1000







ch_bool DeviceSetPower(DEV_CONTEXT* pDevContext, unsigned long flags)
{

	switch (flags) {
		default:
			ch7036_set_power_crt(pDevContext);
			ch7036_set_power_hdmi(pDevContext);
			ch7036_set_power_lvds(pDevContext);
			break;
	}

	return ch_true;

}

ch_bool DevicePrepare(DEV_CONTEXT* pDevContext)
{
	uint32 C;
	uint32 hao_t, vao_t, vai_t, val_t;
	uint32 hai_down;
	uint32 hai_sdram;
	uint32 lnsel;
	uint32 intlc = 0;
	uint32 blk_h;
	uint32 fba_inc;
	uint32 bandwidth;

	uint32 frame_rate_in, frame_rate_out;  
	uint32  r;                             

	INPUT_INFO* pInput_Info = pDevContext->pInput_Info;
	OUTPUT_INFO* pOutput_Info = pDevContext->pOutput_Info;
	PREFER_INFO* pPrefer_Info = pDevContext->pPrefer_Info;

	
	g_nLastError = ERR_NO_ERROR;

	
	I2CWrite(pDevContext,0x03, 0x04);
	pDevContext->DeviceID = I2CRead(pDevContext,0x50);
	if(pDevContext->DeviceID != 0x56)
	{
		g_nLastError = ERR_DEVICE_NO_EXIST;
		return ch_false;
	}

	
	
	if(pInput_Info->timing.ha % 2)
	{
		pInput_Info->timing.ha--;	
	}
	if(pInput_Info->timing.va % 2)
	{
		pInput_Info->timing.va--;	
	}

	
	
	if(pPrefer_Info->dat16_32b)
	{
		C = 100;
	}
	else if(pPrefer_Info->true24)
	{
		C = 100;
	}
	else
	{
		C = (pPrefer_Info->true_com) ? 75 : 50;
	}
	if(pOutput_Info->ds_percent_h)
	{
		hao_t = pOutput_Info->timing.ha * (100 - pOutput_Info->ds_percent_h) / 100;
	}
	else
	{
		hao_t = pOutput_Info->timing.ha;
	}
	if(pOutput_Info->ds_percent_v)
	{
		vao_t = pOutput_Info->timing.va * (100 - pOutput_Info->ds_percent_v) / 100;
	}
	else
	{
		vao_t = pOutput_Info->timing.va;
	}
	hao_t = hao_t + (hao_t % 2);
	vao_t = vao_t + (vao_t % 2);
	hai_down = pInput_Info->timing.ha;
	if(hai_down > hao_t && pOutput_Info->rotate != ROTATE_90 && pOutput_Info->rotate != ROTATE_270)
	{
		hai_down = hao_t;
	}

	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		val_t = 720;
	}
	else
	{
		if(pPrefer_Info->dat16_32b)
		{
			val_t = 720;
		}
		else
		{
			val_t = pPrefer_Info->true24 ? 720 : 1440;
		}
	}
	if(hai_down > val_t && (pOutput_Info->rotate != ROTATE_NO || pOutput_Info->h_flip || pOutput_Info->v_flip))
	{
		hai_down = val_t;
	}

	if(pOutput_Info->channel & CHANNEL_HDMI)
	{
		
		switch(pOutput_Info->hdmi_fmt.format_index)
		{
		case  5:
		case  6:
		case  7:
		case 10:
		case 11:
		case 20:
		case 21:
		case 22:
		case 25:
		case 26:
			intlc = 1;
			break;
		default:
			intlc = 0;
			break;		
		}
	}

	
	if(pOutput_Info->channel & CHANNEL_VGA)
	{
       intlc = 0;
	}

	val_t = intlc ? (vao_t / 2) : vao_t;
	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		if(hai_down <= val_t)
			lnsel = 3;
		else
			lnsel = 1;
	}
	else
	{
		if(pInput_Info->timing.va <= val_t)
			lnsel = 3;
		else
			lnsel = 1;
	}
	bandwidth = (pInput_Info->rx_clk_khz + pOutput_Info->uclk_khz * pInput_Info->timing.ha / hao_t * (4 - lnsel)) / 85 * C;
	if(bandwidth > MEM_CLK_FREQ_MAX)
	{
		if(lnsel >= 2)
		{
			g_nLastError = ERR_BANDWIDTH_OVERFLOW;
			return ch_false;
		}
		
		lnsel++;

		
		
		
		
		
		if(pInput_Info->rx_clk_khz >= 75428 && pOutput_Info->uclk_khz >= 94500)
			lnsel++;

		bandwidth = (pInput_Info->rx_clk_khz + pOutput_Info->uclk_khz * pInput_Info->timing.ha / hao_t * (4 - lnsel)) / 85 * C;
		if(bandwidth > MEM_CLK_FREQ_MAX)
		{
			g_nLastError = ERR_BANDWIDTH_OVERFLOW;
			return ch_false;
		}
		pPrefer_Info->scale_line_adjust = 1;
	}

	
	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		val_t = 100;
	}
	else
	{
		if(pPrefer_Info->dat16_32b)
		{
			val_t = 100;
		}
		else if(pPrefer_Info->true24)
		{
			val_t = 100;
		}
		else
		{
			val_t = pPrefer_Info->true_com ? 75 : 50;
		}
	}
	if(val_t == 75)
	{
		hai_sdram = (hai_down / 4) * 3 + (hai_down % 4);
	}
	else
	{
		hai_sdram = hai_down * val_t / 100;
	}
	if(pOutput_Info->rotate != ROTATE_NO || pOutput_Info->h_flip || pOutput_Info->v_flip)
	{
		blk_h = 45;
	}
	else if(hai_sdram <= 720 && pInput_Info->timing.va <= 720)
	{
		blk_h = 45;
	}
	else
	{
		blk_h = hai_sdram / 16;
		blk_h = (hai_sdram % 16) ? (blk_h + 1) : blk_h;
	}

	
	if((pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270) && pPrefer_Info->true24 == 0 && pPrefer_Info->true_com == 0 && pPrefer_Info->dat16_32b == 0)
	{
		vai_t = pInput_Info->timing.va / 2;
	}
	else
	{
		vai_t = pInput_Info->timing.va;
	}
	val_t = (vai_t % 16) ? (vai_t / 16 + 1) : (vai_t / 16);
	fba_inc = blk_h * val_t;

	
	frame_rate_in = ((pInput_Info->rx_clk_khz * 10000) / pInput_Info->timing.ht) * 10 / pInput_Info->timing.vt;
	frame_rate_out = ((pOutput_Info->uclk_khz * 10000) / pOutput_Info->timing.ht) * 10 / pOutput_Info->timing.vt;
	r = DOUBLE_TO_INT*frame_rate_in / frame_rate_out;

	
	if(pPrefer_Info->dat16_32b)
	{
		if(pPrefer_Info->true24)
		{
			g_nLastError = ERR_NO_SUPPORT_TRUE24;
			return ch_false;
		}
		if(pPrefer_Info->true_com)
		{
			g_nLastError = ERR_NO_SUPPORT_TRUECOM;
			return ch_false;
		}
		if(pOutput_Info->rotate == ROTATE_NO && pOutput_Info->h_flip == 0 && pOutput_Info->v_flip == 0)
		{
			if(fba_inc > FBA_INC_MAX)
			{
				
				
				
               if (( (DOUBLE_TO_INT/2) <= r) && ( r <= DOUBLE_TO_INT))
			   {
					if( ((4096*DOUBLE_TO_INT) / fba_inc) < (2*DOUBLE_TO_INT-r) )
					{
						g_nLastError = ERR_RESOLUTION_OVERFLOW;
						return ch_false;
					}
			   }
  
               if (( DOUBLE_TO_INT < r) && ( r <= (2*DOUBLE_TO_INT)))
			   {
					if( ((4096*DOUBLE_TO_INT) / fba_inc)  <
						(2*DOUBLE_TO_INT - (DOUBLE_TO_INT*DOUBLE_TO_INT)/r) )
					{
						g_nLastError = ERR_RESOLUTION_OVERFLOW;
						return ch_false;
					}
			   }		
			   
			}
		}
		else if(pOutput_Info->rotate != ROTATE_NO)	
		{
			if(pInput_Info->timing.va > 720)
			{
				g_nLastError = ERR_ROTATION_WITH_VAI;
				return ch_false;
			}
		}
		else 
		{
			if(pInput_Info->timing.va > 720)
			{
				g_nLastError = ERR_FLIP_WITH_VAI;
				return ch_false;
			}
		}
	}

	
	if(pPrefer_Info->dat16_32b == 0)
	{
		if(pOutput_Info->rotate == ROTATE_NO && pOutput_Info->h_flip == 0 && pOutput_Info->v_flip == 0)
		{	
			if(fba_inc > FBA_INC_MAX)	
			{
				
                if (( (DOUBLE_TO_INT/2) <= r) && ( r <= DOUBLE_TO_INT))
			    {
					if( ((4096*DOUBLE_TO_INT) / fba_inc) < (2*DOUBLE_TO_INT-r) )
					{
						g_nLastError = ERR_RESOLUTION_OVERFLOW;
						return ch_false;
					}
			    }
  
                if (( DOUBLE_TO_INT < r) && ( r <= (2*DOUBLE_TO_INT)))
			    {
					if( ((4096*DOUBLE_TO_INT) / fba_inc)  <
						(2*DOUBLE_TO_INT - (DOUBLE_TO_INT*DOUBLE_TO_INT)/r) )
					{
						g_nLastError = ERR_RESOLUTION_OVERFLOW;
						return ch_false;
					}
			    }		
			    

				if(pPrefer_Info->true24)
				{
					g_nLastError = ERR_NO_SUPPORT_TRUE24;
					return ch_false;
				}
				if(pPrefer_Info->true_com)
				{
					g_nLastError = ERR_NO_SUPPORT_TRUECOM;
					return ch_false;
				}
				g_nLastError = ERR_RESOLUTION_OVERFLOW;
				return ch_false;
			}
		}
		if(pOutput_Info->rotate == ROTATE_180)
		{
			if(pPrefer_Info->true_com)
			{
				g_nLastError = ERR_NO_SUPPORT_TRUECOM;
				return ch_false;
			}
			if(pPrefer_Info->true24)
			{
				if(pInput_Info->timing.va > 720)
				{
					g_nLastError = ERR_NO_SUPPORT_TRUE24;
					return ch_false;
				}
			}
			else
			{
				if(pInput_Info->timing.va > 720)
				{
					g_nLastError = ERR_ROTATION_WITH_VAI;
					return ch_false;
				}
			}
		}
		if(pOutput_Info->h_flip || pOutput_Info->v_flip)
		{
			if(pPrefer_Info->true_com)
			{
				g_nLastError = ERR_NO_SUPPORT_TRUECOM;
				return ch_false;
			}
			if(pInput_Info->timing.va > 720)
			{
				g_nLastError = ERR_FLIP_WITH_VAI;
				return ch_false;
			}
		}
		if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
		{
			if(pPrefer_Info->true_com)
			{
				g_nLastError = ERR_NO_SUPPORT_TRUECOM;
				return ch_false;
			}
			if(pPrefer_Info->true24)
			{
				if(pInput_Info->timing.va > 720)
				{
					g_nLastError = ERR_NO_SUPPORT_TRUE24;
					return ch_false;
				}
			}
			else
			{
				if(pInput_Info->timing.va > 1440)
				{
					g_nLastError = ERR_ROTATION_WITH_VAI;
					return ch_false;
				}
			}
		}
	}

	
	
	if(pPrefer_Info->true24 || pPrefer_Info->true_com)
	{
		pPrefer_Info->dither_filter_enable = 0;
	}

	

	
	iic_reset();

	
	if(pOutput_Info->channel & CHANNEL_HDMI || pOutput_Info->channel & CHANNEL_VGA) 
	{
	
		iic_write(0x03, 0x01);  
		iic_write(0x07, 0x7E);
		iic_write(0x09, 0x0B);
		iic_write(0x03, 0x00);
		iic_write_ex(STOP, 1);
	}

	return ch_true;
}

ch_bool DeviceConfig(DEV_CONTEXT* pDevContext)
{
	if(!set_input_info(pDevContext->pInput_Info))
		return ch_false;

	if(!set_output_info(pDevContext->pOutput_Info))
		return ch_false;

	if(!set_prefer_info(pDevContext->pPrefer_Info))
		return ch_false;

	if(!cal_and_set_clk_pll(pDevContext))
		return ch_false;

	if(!cal_and_set_scaler(pDevContext))
		return ch_false;

	if(!cal_and_set_power(pDevContext))
		return ch_false;

	return ch_true;
}

ch_bool DeviceRunning(DEV_CONTEXT* pDevContext)
{
	uint8 val_t;
	uint8 reg = 0x00;
	OUTPUT_INFO* pOutput_Info = pDevContext->pOutput_Info;
	PREFER_INFO* pPrefer_Info = pDevContext->pPrefer_Info;

	uint8 REFDLY = pPrefer_Info->pll_ref_dly;
	uint8 FBDLY = pPrefer_Info->pll_ref_fbdly;
	uint8 TXDRV_IT = pPrefer_Info->lvds_txdrv_ctrl;

	
	DeviceReset(pDevContext);

	
	
	DeviceSetup(pDevContext);
	
	
	I2CWrite(pDevContext,0x03, 0x01);
	
	
	
	
	reg = I2CRead(pDevContext,0x24) & 0xFD; 
	I2CWrite(pDevContext,0x24, reg);

	I2CWrite(pDevContext,0x03, 0x04);
	reg = I2CRead(pDevContext,0x66);
    reg = reg & 0x1F;
	reg = reg | ((uint8)REFDLY << 5);
	I2CWrite(pDevContext,0x66, reg); 

	I2CWrite(pDevContext,0x03, 0x04);
	reg = I2CRead(pDevContext,0x66);
    reg = reg & 0xE3;
	reg = reg | ((uint8)FBDLY << 2);
	I2CWrite(pDevContext,0x66, reg);

    I2CWrite(pDevContext,0x03, 0x04);
	reg = I2CRead(pDevContext,0x68) & 0xF0;
    reg = reg | ((uint8)TXDRV_IT);
	I2CWrite(pDevContext,0x68, reg); 

	



	
        
       I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0D);
	   if((pPrefer_Info->eye_dri_pll_rlf >= 0) && (pPrefer_Info->eye_dri_pll_rlf <= 3))
 	      reg = (reg & 0x9F) | (pPrefer_Info->eye_dri_pll_rlf << 5);
	   I2CWrite(pDevContext,0x0D, reg);

	   I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0C);
	   if((pPrefer_Info->eye_dri_pll_cp >= 0) && (pPrefer_Info->eye_dri_pll_cp <= 3))
 	      reg = (reg & 0x3F) | (pPrefer_Info->eye_dri_pll_cp << 6);
	   I2CWrite(pDevContext,0x0C, reg);

	   I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0B);
	   if((pPrefer_Info->eye_dri_damp >= 0) && (pPrefer_Info->eye_dri_damp <= 7))
 	      reg = (reg & 0x1F) | (pPrefer_Info->eye_dri_damp << 5);
	   I2CWrite(pDevContext,0x0B, reg);

       I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0A);
	   if((reg & 0x01)==0x00)
 	      reg = (reg | 0x01);
	   I2CWrite(pDevContext,0x0A, reg);

       I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0A);
	   if((pPrefer_Info->eye_dri_demp >= 0) && (pPrefer_Info->eye_dri_demp <= 15))
 	      reg = (reg & 0xE1) | (pPrefer_Info->eye_dri_demp << 1);
	   I2CWrite(pDevContext,0x0A, reg);

	   I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x08);
	   if((pPrefer_Info->eye_dacg >= 0) && (pPrefer_Info->eye_dacg <= 3))
 	      reg = (reg & 0x3F) | (pPrefer_Info->eye_dacg << 6);
	   I2CWrite(pDevContext,0x08, reg);

	   I2CWrite(pDevContext,0x03, 0x01);  
	   reg = I2CRead(pDevContext,0x0F);
	   if((pPrefer_Info->eye_rdac >= 0) && (pPrefer_Info->eye_rdac <= 7))
 	      reg = (reg & 0xC7) | (pPrefer_Info->eye_rdac << 3);
	   I2CWrite(pDevContext,0x0F, reg);

	   I2CWrite(pDevContext,0x03, 0x04);  
	   reg = I2CRead(pDevContext,0x54);
	   if((pPrefer_Info->eye_bgtrim >= 0) && (pPrefer_Info->eye_bgtrim <= 3))
 	      reg = (reg & 0xF3) | (pPrefer_Info->eye_bgtrim << 2);
	   I2CWrite(pDevContext,0x54, reg);


	

	
	if(pOutput_Info->channel & CHANNEL_HDMI || pOutput_Info->channel & CHANNEL_VGA) 
	{
		
		if(!post_cal_and_set(pDevContext)){
		   return ch_false;
	    }

		
		I2CWrite(pDevContext,0x03, 0x00);
		val_t = I2CRead(pDevContext,0x0A);
		I2CWrite(pDevContext,0x0A, val_t | 0x80); 
		I2CWrite(pDevContext,0x0A, val_t & 0x7F); 

		


		
		
		
		
		
		
	    
	    
		
		
		I2CWrite(pDevContext,0x03, 0x00);
		val_t = I2CRead(pDevContext,0x0A);
		I2CWrite(pDevContext,0x0A, val_t & 0xEF); 
		
		

	}

	return ch_true;
}

uint32 GetLastErrorMessage()
{
	return g_nLastError;
}



ch_bool set_input_info(INPUT_INFO* pInput_Info)
{
	
	iic_write_ex(HTI, pInput_Info->timing.ht);
	iic_write_ex(HAI, pInput_Info->timing.ha);
	iic_write_ex(HOI, pInput_Info->timing.ho);
	iic_write_ex(HWI, pInput_Info->timing.hw);
	iic_write_ex(VTI, pInput_Info->timing.vt);
	iic_write_ex(VAI, pInput_Info->timing.va);
	iic_write_ex(VOI, pInput_Info->timing.vo);
	iic_write_ex(VWI, pInput_Info->timing.vw);
	
	iic_write_ex(RCLK, pInput_Info->rx_clk_khz);
	
	
	switch(pInput_Info->pixel_fmt)
	{
		  case 0:
			 iic_write_ex(DITHER_SEL0_SPP, 0);
		     iic_write_ex(DITHER_SEL1_SPP, 0);
			 break;
		  case 1:
			 iic_write_ex(DITHER_SEL0_SPP, 1);
		     iic_write_ex(DITHER_SEL1_SPP, 0);
			 break;
		  case 2:
			 iic_write_ex(DITHER_SEL0_SPP, 0);
		     iic_write_ex(DITHER_SEL1_SPP, 1);
			 break;
		  case 3:
			 iic_write_ex(DITHER_SEL0_SPP, 1);
		     iic_write_ex(DITHER_SEL1_SPP, 1);
			 break;
		  default:
			iic_write_ex(DITHER_SEL0_SPP, 0);
		    iic_write_ex(DITHER_SEL1_SPP, 0);
			break;
	}

	
	
	iic_write_ex(HSYNCP_SPP, pInput_Info->hs_pol);
	iic_write_ex(VSYNCP_SPP, pInput_Info->vs_pol);
	
	iic_write_ex(HPO_I, pInput_Info->hs_pol);
	iic_write_ex(VPO_I, pInput_Info->vs_pol);
	iic_write_ex(DEPO_I, pInput_Info->de_pol);
	
	iic_write_ex(NP_INV0_SPP, (pInput_Info->data_ch_pol & (1 << 0)) ? 1 : 0);
	iic_write_ex(NP_INV1_SPP, (pInput_Info->data_ch_pol & (1 << 1)) ? 1 : 0);
	iic_write_ex(NP_INV2_SPP, (pInput_Info->data_ch_pol & (1 << 2)) ? 1 : 0);
	iic_write_ex(NP_INV3_SPP, (pInput_Info->data_ch_pol & (1 << 3)) ? 1 : 0);
	
	iic_write_ex(LVDS_IN_ORDER, pInput_Info->data_ch_invert);
	
	iic_write_ex(I2S_SPDIFB, pInput_Info->audio_type);
	if(pInput_Info->audio_type == AUDIO_I2S) {
		iic_write_ex(I2SPOL, pInput_Info->i2s_pol);
		iic_write_ex(I2S_LENGTH, pInput_Info->i2s_len);
		iic_write_ex(I2SFMT, pInput_Info->i2s_fmt);
	}

	return ch_true;
}

ch_bool set_output_info(OUTPUT_INFO* pOutput_Info)
{
	uint8 lvds0_seq, lvds1_seq, lvds2_seq, lvds3_seq, lvdsclk_seq;
	uint8 lvds0_pol, lvds1_pol, lvds2_pol, lvds3_pol, lvdsclk_pol;
	uint8 hd_dvib, intlc = 0, copy, hd_lv_pol, hd_lv_seq, hdmi_lvds_sel, hsp, vsp, m1m0, c1c0, vic; 
	uint32 hao_down, vao_down;

	
	LVDS_FMT* pLvdsFmt = &pOutput_Info->lvds_fmt;
	HDMI_FMT* pHdmiFmt = &pOutput_Info->hdmi_fmt;
	
	
	


	
		



	if(pOutput_Info->channel & CHANNEL_VGA || pOutput_Info->channel & CHANNEL_HDMI) {
		iic_write_ex(VP,pOutput_Info->v_position);
		iic_write_ex(HP,pOutput_Info->h_position);
	}
	

	
	if(pOutput_Info->channel & CHANNEL_HDMI || pOutput_Info->channel & CHANNEL_VGA)
	{
		iic_write_ex(HTO, pOutput_Info->timing.ht);
		iic_write_ex(HAO, pOutput_Info->timing.ha);
		iic_write_ex(HOO_HDMI, pOutput_Info->timing.ho);
		iic_write_ex(HWO_HDMI, pOutput_Info->timing.hw);
		iic_write_ex(VTO, pOutput_Info->timing.vt);
		iic_write_ex(VAO, pOutput_Info->timing.va);
		iic_write_ex(VOO_HDMI, pOutput_Info->timing.vo);
		iic_write_ex(VWO_HDMI, pOutput_Info->timing.vw);

		iic_write_ex(UCLK, pOutput_Info->uclk_khz);
		iic_write_ex(ROTATE, pOutput_Info->rotate);
		iic_write_ex(HFLIP, pOutput_Info->h_flip);
		iic_write_ex(VFLIP, pOutput_Info->v_flip);
		
		hao_down = pOutput_Info->timing.ha * (100 - pOutput_Info->ds_percent_h) / 100;
		hao_down = hao_down + (hao_down % 2);
		vao_down = pOutput_Info->timing.va * (100 - pOutput_Info->ds_percent_v) / 100;
		vao_down = vao_down + (vao_down % 2);
		if(pOutput_Info->ds_percent_h || pOutput_Info->ds_percent_v)
		{
			iic_write_ex(SCAN_EN, 1);
			iic_write_ex(HAO_SCL, hao_down);
			iic_write_ex(VAO_SCL, vao_down);
		}
		
		iic_write_ex(ROTATE, pOutput_Info->rotate);
		iic_write_ex(HFLIP, pOutput_Info->h_flip);
		iic_write_ex(VFLIP, pOutput_Info->v_flip);
	}
	
	if(pOutput_Info->channel & CHANNEL_LVDS)
	{
		
		lvds0_seq = (pLvdsFmt->channel_swap >>  0) & 0xF;
		lvds1_seq = (pLvdsFmt->channel_swap >>  4) & 0xF;
		lvds2_seq = (pLvdsFmt->channel_swap >>  8) & 0xF;
		lvds3_seq = (pLvdsFmt->channel_swap >> 12) & 0xF;
		lvdsclk_seq = (pLvdsFmt->channel_swap >>  16) & 0xF;
		iic_write_ex(LVDS0_SEQ_SPP, lvds0_seq);
		iic_write_ex(LVDS1_SEQ_SPP, lvds1_seq);
		iic_write_ex(LVDS2_SEQ_SPP, lvds2_seq);
		iic_write_ex(LVDS3_SEQ_SPP, lvds3_seq);
		iic_write_ex(LVDSCLK_SEQ_SPP, lvdsclk_seq);
		
		lvds0_pol = (pLvdsFmt->channel_pol >> 0) & 0x1;
		lvds1_pol = (pLvdsFmt->channel_pol >> 1) & 0x1;
		lvds2_pol = (pLvdsFmt->channel_pol >> 2) & 0x1;
		lvds3_pol = (pLvdsFmt->channel_pol >> 3) & 0x1;
		lvdsclk_pol = (pLvdsFmt->channel_pol >> 4) & 0x1;
		iic_write_ex(LVDS0_POL_SPP, lvds0_pol);
		iic_write_ex(LVDS1_POL_SPP, lvds1_pol);
		iic_write_ex(LVDS2_POL_SPP, lvds2_pol);
		iic_write_ex(LVDS3_POL_SPP, lvds3_pol);
		iic_write_ex(LVDSCLK_POL_SPP, lvdsclk_pol);
		
		
		switch(pLvdsFmt->pixel_fmt){
		  case 0:
			 iic_write_ex(DITHER_SEL0_SPP, 0);
		     iic_write_ex(DITHER_SEL1_SPP, 0);
			 break;
		  case 1:
			 iic_write_ex(DITHER_SEL0_SPP, 1);
		     iic_write_ex(DITHER_SEL1_SPP, 0);
			 break;
		  case 2:
			 iic_write_ex(DITHER_SEL0_SPP, 0);
		     iic_write_ex(DITHER_SEL1_SPP, 1);
			 break;
		  case 3:
			 iic_write_ex(DITHER_SEL0_SPP, 1);
		     iic_write_ex(DITHER_SEL1_SPP, 1);
			 break;
		  default:
			iic_write_ex(DITHER_SEL0_SPP, 0);
		    iic_write_ex(DITHER_SEL1_SPP, 0);
			break;
	    }

	}
	
	if((pOutput_Info->channel & CHANNEL_VGA)&&((pOutput_Info->channel & CHANNEL_HDMI)==0x00))
	{
		

		iic_write_ex(HPO_O, 1);
		iic_write_ex(VPO_O, 1);
		iic_write_ex(DEPO_O, 0);
	}
	else if((pOutput_Info->channel & CHANNEL_VGA)&&(pOutput_Info->channel & CHANNEL_HDMI))
	{

        iic_write_ex(HPO_O, 1);
		iic_write_ex(VPO_O, 0);
		iic_write_ex(DEPO_O, 0);
	}

	if((pOutput_Info->channel & CHANNEL_HDMI) && ((pOutput_Info->channel & CHANNEL_VGA) == 0x00))
	{
		 iic_write_ex(HDMIIN_HSP, pHdmiFmt->hs_pol ? 1:0);
		 iic_write_ex(HDMIIN_VSP, pHdmiFmt->vs_pol ? 1:0);
		 iic_write_ex(HDMIIN_DEP, 0);
	}
	if((pOutput_Info->channel & CHANNEL_HDMI)&&(pOutput_Info->channel & CHANNEL_VGA))
	{
		 iic_write_ex(HDMIIN_HSP, 0);
		 iic_write_ex(HDMIIN_VSP, 1);
		 iic_write_ex(HDMIIN_DEP, 1);
	}

	
	if(pOutput_Info->channel & CHANNEL_HDMI)
	{
		
		hd_dvib = pHdmiFmt->is_dvi_mode ? 0 : 1;
		iic_write_ex(HD_DVIB, hd_dvib);
		
		switch(pHdmiFmt->format_index)
		{
		case  5:
		case  6:
		case  7:
		case 10:
		case 11:
		case 20:
		case 21:
		case 22:
		case 25:
		case 26:
			intlc = 1;
			break;
		default:
			intlc = 0;
			break;		
		}
		iic_write_ex(INTLC, intlc);
		
		copy = 1;
		iic_write_ex(COPY, copy);
		
		hd_lv_pol = pHdmiFmt->data_pol_invert;
		iic_write_ex(HD_LV_POL, hd_lv_pol);
		
		hd_lv_seq = pHdmiFmt->channel_swap;
		iic_write_ex(HD_LV_SEQ, hd_lv_seq);
		
		hdmi_lvds_sel = 1;
		iic_write_ex(HDMI_LVDS_SEL, hdmi_lvds_sel);
		

		if (pOutput_Info->timing.ha > 720) {
			pOutput_Info->hdmi_fmt.hs_pol = POL_HIGH;
			pOutput_Info->hdmi_fmt.vs_pol = POL_HIGH;
			if(pOutput_Info->timing.ha == 1440)
			{
              pOutput_Info->hdmi_fmt.hs_pol = POL_LOW;
			  pOutput_Info->hdmi_fmt.vs_pol = POL_LOW;
			}
		}
		else {
			
			pOutput_Info->hdmi_fmt.hs_pol = POL_LOW;
			pOutput_Info->hdmi_fmt.vs_pol = POL_LOW;
		}

		hsp = pHdmiFmt->hs_pol;
		vsp = pHdmiFmt->vs_pol;
		iic_write_ex(HSP, hsp);
		iic_write_ex(VSP, vsp);
		
		
		switch(pHdmiFmt->format_index)
		{
		case 1:
		case 2:
		case 17:
			{
				if(pHdmiFmt->aspect_ratio == AS_RATIO_16_9) 
				{
					g_nLastError = ERR_ASPECT_RATIO_NOMATCH;
					return ch_false;
				}
			}
			break;
		case 4:
		case 5:
		case 16:
		case 19:
		case 20:
		case 31:
		case 32:
		case 33:
		case 34:
			{
				if(pHdmiFmt->aspect_ratio == AS_RATIO_4_3)	
				{
					g_nLastError = ERR_ASPECT_RATIO_NOMATCH;
					return ch_false;
				}
			}
			break;
		default:
			break;
		}
		m1m0 = pHdmiFmt->aspect_ratio;
		iic_write_ex(M1M0, m1m0);
		
		c1c0 = 2;
		iic_write_ex(C1C0, c1c0);
		
		vic = pHdmiFmt->format_index;
		iic_write_ex(VIC, vic);
	}

	if(pOutput_Info->channel & CHANNEL_VGA){
       intlc = 0;
	}

	iic_write_ex(INTLC, intlc);

	
	iic_write( 0x3,  0x1); 
	iic_write( 0x23, 0x63); 
	iic_write( 0x3,  0x0); 

	return ch_true;
}

ch_bool set_prefer_info(PREFER_INFO* pPrefer_Info)
{
	uint8 hsync_cnt_th, prbs_set_sel;
	uint8 dbp ;	
	
	iic_write_ex(MCLK, pPrefer_Info->mclk_khz);
	
	iic_write_ex(UCLKOD_SEL, pPrefer_Info->uclkod_sel);
	iic_write_ex(DAT16_32B, pPrefer_Info->dat16_32b);
	iic_write_ex(TRUE24, pPrefer_Info->true24);
	iic_write_ex(TRUE_COM, pPrefer_Info->true_com);
	
	hsync_cnt_th = pPrefer_Info->lvds_out_hs_tolerance;
	iic_write_ex(HSYNC_CNT_TH_SPP, hsync_cnt_th);
	prbs_set_sel = pPrefer_Info->lvds_out_reset_bit_sel;
	iic_write_ex(PRBS_SET_SEL_SPP, prbs_set_sel);
	
	dbp = pPrefer_Info->dither_filter_enable ? 0 : 1;
	iic_write_ex(DBP, dbp);
	


	iic_write_ex(TXTEN, pPrefer_Info->text_enhancement);


	



	return ch_true;
}


ch_bool cal_and_set_clk_pll(DEV_CONTEXT* pDevContext)
{
	uint8 pll1n2_reg, pll1n2_div;
	uint8 pll1n3_reg, pll1n3_div;
	uint8 pll1n1_reg = 0, pll1n1_div = 0;
	uint8 pll3n8_reg, pll3n8_div;
	uint8 pll2n5_reg, pll2n5_div;
	uint8 pll2n6_reg, pll2n6_div;
	uint8 pll2n7_reg, pll2n7_div;
	uint8 dmxtal_reg, dmxtal_div;
	
	uint32 a2_reg = 0;
	uint32 a1_reg = 0;
	uint32 a3_reg = 0;
	uint32 uclk2d_reg = 0;
	uint8 uclksec_reg = 0;
	uint8 dri_pll_n1_reg;
	uint8 dri_pll_n3_reg;
	
	uint32 val_t;
	uint32 val_t1, val_t2;

	uint8 gcksel = 0;
	uint8 tsten1 = 0; 
   	uint8 REV_ID;
	uint64 temp1;

	
	INPUT_INFO* pInput_Info = pDevContext->pInput_Info;
	OUTPUT_INFO* pOutput_Info = pDevContext->pOutput_Info;
	PREFER_INFO* pPrefer_Info = pDevContext->pPrefer_Info;

	I2CWrite(pDevContext,0x03, 0x04);
	REV_ID = I2CRead(pDevContext,0x51) & 0x0F;

	

	if(pPrefer_Info->reset == 1) 
	{
      I2CWrite(pDevContext,0x03, 0x01);
	  I2CWrite(pDevContext,0x14, I2CRead(pDevContext,0x14) | 0x40); 
	  I2CWrite(pDevContext,0x03, 0x04);
	  I2CWrite(pDevContext,0x54, I2CRead(pDevContext,0x54) | 0x40); 
	  tsten1 = 0x40;
	  gcksel = 0x40;
	  iic_write_ex(GCKSEL, 1);
	  iic_write_ex(TSTEN1, 1);
	}
	
	
	else if((pPrefer_Info->reset == 0) || (REV_ID >= 0x01)) 
	{
	  I2CWrite(pDevContext,0x03, 0x01);
	  I2CWrite(pDevContext,0x14, I2CRead(pDevContext,0x14) & 0xBF); 
	  I2CWrite(pDevContext,0x03, 0x04);
	  I2CWrite(pDevContext,0x54, 0x40 | (I2CRead(pDevContext,0x54) & 0xBF)); 
	  tsten1=0x00;
	  gcksel=0x00;
	  iic_write_ex(GCKSEL, 1);
	  iic_write_ex(TSTEN1, 0);
	}

	
	pll1n2_div = 4;
	pll1n3_div = 8;
	if(!convert_pll1n2_div(ch_false, &pll1n2_reg, &pll1n2_div))
	{
		g_nLastError = ERR_PLL1N2_WRONG;
		return ch_false;
	}
	if(!convert_pll1n3_div(ch_false, &pll1n3_reg, &pll1n3_div))
	{
		g_nLastError = ERR_PLL1N3_WRONG;
		return ch_false;
	}
	iic_write_ex(PLL1N2, pll1n2_reg);
	iic_write_ex(PLL1N3, pll1n3_reg);

	

	if(((gcksel == 0x40)&&(tsten1 == 0x00)) || ((gcksel == 0x00)&&(tsten1 == 0x00)))
	{
       
		
		
		
			pll1n1_div = 8;
		    pll1n1_reg = 3;
		
		
        
        
		
		
	  
	}
	else if((gcksel == 0x40)&&(tsten1 == 0x40)){
	  for(pll1n1_div=1; pll1n1_div<=64; pll1n1_div<<=1)
	  {
	 	val_t = pInput_Info->rx_clk_khz / pll1n1_div;
	    if(val_t >= 2300 && val_t <= 4600)
			break;
	  }
	  if(pll1n1_div > 64)
	  {
		g_nLastError = ERR_PLL1N1_WRONG;
		return ch_false;
	  }
	  if(!convert_pll1n1_div(ch_false, &pll1n1_reg, &pll1n1_div))
	  {
		g_nLastError = ERR_PLL1N1_WRONG;
		return ch_false;
	  }
	}

	iic_write_ex(PLL1N1, pll1n1_reg);

	
	for(pll3n8_div=1; pll3n8_div<=8; pll3n8_div<<=1)
	{
		val_t = pPrefer_Info->mclk_khz * pll3n8_div / 64;
		if(val_t >= 2300 && val_t <= 2600)
			break;
	}
	if(pll3n8_div > 8)
	{
		g_nLastError = ERR_PLL3N8_WRONG;
		return ch_false;
	}
	if(!convert_pll3n8_div(ch_false, &pll3n8_reg, &pll3n8_div))
	{
		g_nLastError = ERR_PLL3N8_WRONG;
		return ch_false;
	}
	iic_write_ex(PLL3N8, pll3n8_reg);

	

	if((gcksel == 0x40)&&(tsten1 == 0x40)){
		 val_t1 = (pInput_Info->rx_clk_khz << 12);
		 val_t2 = (pll1n1_div * pll3n8_div * pPrefer_Info->mclk_khz);
		 a2_reg = (val_t1 % val_t2) ? (val_t1 / val_t2 + 1) : (val_t1 / val_t2); 
		  
	}else if(((gcksel == 0x40)&&(tsten1 == 0x00))||((gcksel == 0x00)&&(tsten1 == 0x00))){
	  val_t1 = (27000 << 12);
	  val_t2 = (pll1n1_div * pll3n8_div * pPrefer_Info->mclk_khz);
	  
	 
	
	  
	  a2_reg = (val_t1 % val_t2) ? (val_t1 / val_t2 + 1) : (val_t1 / val_t2); 
	  if(pOutput_Info->uclk_khz == 148500){
         a2_reg = 0x73;      
	  }
	}	

	iic_write_ex(A2, a2_reg);

	
	if(pInput_Info->audio_type) 
	{
		uclksec_reg = pOutput_Info->uclk_khz / 10;
	}
	else 
	{
		

		

		 
		if( (0x40==gcksel)&&(0x40==tsten1) )  
		{
			uclk2d_reg = (pInput_Info->rx_clk_khz * 2 * pll1n2_div * pll1n3_div) / (pll1n1_div * 10);
		}
		if(0x00==tsten1 )   
		{
			uclk2d_reg = (27000 * 2 * pll1n2_div * pll1n3_div) / (pll1n1_div * 10);
		}

	}
	iic_write_ex(PCLK_NUM, uclk2d_reg);

	

	
	
	if((pOutput_Info->uclk_khz == 54000	||
												  pOutput_Info->uclk_khz == 72000	||
												  pOutput_Info->uclk_khz == 74250	||
												  pOutput_Info->uclk_khz == 108000	||
												  pOutput_Info->uclk_khz == 148500  ))
	{
		uclksec_reg = 1;
	}
	else
	{
		uclksec_reg = 0;
	}
	iic_write_ex(UCLKSEC, uclksec_reg);

	
	if(uclksec_reg == 0)
	{
		pll2n6_div = 8;
		pll2n7_div = 8;
		for(pll2n5_div=1; pll2n5_div<=8; ++pll2n5_div)
		{
			val_t = pOutput_Info->uclk_khz * pll2n5_div / 64;
			if(val_t >= 2300 && val_t <= 4600)
				break;
		}
		if(pll2n5_div > 8)
		{
			g_nLastError = ERR_PLL2N5_WRONG;
			return ch_false;
		}
		dmxtal_div = 1; 
		
		if(pPrefer_Info->uclkod_sel == 1)
		{
			if((gcksel == 0x40)&&(tsten1 == 0x40)){
				temp1 = (((uint64)pOutput_Info->uclk_khz) * pll1n1_div * pll2n5_div * (1 << 20));
#ifdef T_LINUX
	        	do_div(temp1 , pInput_Info->rx_clk_khz);
				a1_reg = (uint32)temp1;
#else
				a1_reg = (uint32)((uint64)temp1 / pInput_Info->rx_clk_khz);
#endif
				PD_DEBUG ("#1 a1_reg = 0x%.8X\n", a1_reg);
	      	} else if((gcksel == 0x40)&&(tsten1 == 0x00)){
				temp1 = (((uint64)pOutput_Info->uclk_khz) * pll1n1_div * pll2n5_div * (1 << 20));
#ifdef T_LINUX
	        	do_div(temp1 , (uint32)27000);
				a1_reg = (uint32)temp1;
#else
				a1_reg = (uint32)((uint64)temp1 / (uint32)27000);
#endif
				PD_DEBUG ("#2 a1_reg = 0x%.8X\n", a1_reg);
	       	}else if((gcksel == 0x00)&&(tsten1 == 0x00)){
				temp1 = (((uint64)pOutput_Info->uclk_khz) * pll1n1_div * pll2n5_div * (1 << 20));
#ifdef T_LINUX
	        	do_div(temp1 , (uint32)27000);
				a1_reg = (uint32)temp1;
#else
				a1_reg = (uint32)((uint64)temp1 / (uint32)27000);
#endif
				PD_DEBUG ("#3 a1_reg = 0x%.8X\n", a1_reg);
			}
		   iic_write_ex(A1, a1_reg);
		}
		else
		{
			if((gcksel == 0x40)&&(tsten1 == 0x40))
	       a3_reg = pInput_Info->rx_clk_khz * (1 << 12) / (pOutput_Info->uclk_khz * pll1n1_div * pll2n5_div);
          else if((gcksel == 0x40)&&(tsten1 == 0x00))
	       a3_reg = 27000 * (1 << 12) / (pOutput_Info->uclk_khz * pll1n1_div * pll2n5_div);
          else if((gcksel == 0x00)&&(tsten1 == 0x00))
	       a3_reg = 27000 * (1 << 12) / (pOutput_Info->uclk_khz * pll1n1_div * pll2n5_div);

			iic_write_ex(A3, a3_reg);
		}
	}
	else 
	{
		switch(pOutput_Info->uclk_khz)
		{
		case 54000:
			dmxtal_div = 6;
			pll2n7_div = 8;
			pll2n6_div = 6;
			pll2n5_div = 4;
			break;
		case 72000:
			dmxtal_div = 6;
			pll2n7_div = 8;
			pll2n6_div = 6;
			pll2n5_div = 3;
			break;
		case 74250:
			dmxtal_div = 6;    
			pll2n7_div = 11;
			pll2n6_div = 6;    
			pll2n5_div = 4;
			break;
		case 108000:
			dmxtal_div = 6;
			pll2n7_div = 8;
			pll2n6_div = 6;
			pll2n5_div = 2;
			break;
		case 148500:
			
			
			
			
			dmxtal_div = 6;  
			pll2n6_div = 6;  
			
            
			
			

			pll2n7_div = 11;
			pll2n5_div = 2;
			break;
		default:
			return ch_false;
		}
	}
	if(!convert_pll2n5_div(ch_false, &pll2n5_reg, &pll2n5_div))
	{
		g_nLastError = ERR_PLL2N5_WRONG;
		return ch_false;
	}
	iic_write_ex(PLL2N5, pll2n5_reg & 0x07);
	iic_write_ex(PLL2N53, pll2n5_reg >> 3 & 0x01);
	if(!convert_pll2n6_div(ch_false, &pll2n6_reg, &pll2n6_div))
	{
		g_nLastError = ERR_PLL2N6_WRONG;
		return ch_false;
	}
	iic_write_ex(PLL2N6, pll2n6_reg);
	if(!convert_pll2n7_div(ch_false, &pll2n7_reg, &pll2n7_div))
	{
		g_nLastError = ERR_PLL2N7_WRONG;
		return ch_false;
	}
	iic_write_ex(PLL2N7, pll2n7_reg);
	if(!convert_dmxtal_div(ch_false, &dmxtal_reg, &dmxtal_div))
	{
		g_nLastError = ERR_DMXTAL_WRONG;
		return ch_false;
	}
	iic_write_ex(DIVXTAL, dmxtal_reg);

	
	if(pInput_Info->rx_clk_khz > 40000)
	{
		iic_write_ex(TXPLL_FFD, 1);
	}

	
	if(pOutput_Info->channel & CHANNEL_HDMI)
	{
		val_t = pOutput_Info->uclk_khz;
		if(val_t < 25000 || val_t > 165000)
		{
			g_nLastError = ERR_HDMI_CLOCK_NO_SUPPORT;
			return ch_false;
		}
		if(val_t <= 40000)
		{
			dri_pll_n1_reg = 0;
			dri_pll_n3_reg = 0;
		}
		else if(val_t < 80000)
		{
			dri_pll_n1_reg = 1;
			dri_pll_n3_reg = 1;
		}
		else
		{
			dri_pll_n1_reg = 2;
			dri_pll_n3_reg = 2;
		}
		iic_write_ex(DRI_PLL_N1, dri_pll_n1_reg);
		iic_write_ex(DRI_PLL_N3, dri_pll_n3_reg);
	}

	
	iic_write_ex(CK_TVINV, 1);
	
	return ch_true;
}

ch_bool cal_and_set_scaler(DEV_CONTEXT* pDevContext)
{
	uint8 wrlen_reg;
	uint32 frame_rate_in, frame_rate_out, field_rate_out;
	uint32 hai_down, hai_sdram;
	uint8 fltbp2_reg, fltbp1_reg;
	uint8 dnsmpen_reg;
	uint32 hadwn_reg = 0;
	uint8 blk_h_reg;
	uint32 fba_inc_reg;
	uint8 sfm_reg;
	uint8 thren_reg;
	uint32 thrrl_reg;
	uint8 lnsel_reg;
	uint8 wrfast_reg;
	uint8 chg_hl_reg;
	uint8 vsmst_reg;

	uint32 val_t;
	uint32 hao_t;
	uint32 vao_t;
	uint32 vai_t;

	
	INPUT_INFO* pInput_Info = pDevContext->pInput_Info;
	OUTPUT_INFO* pOutput_Info = pDevContext->pOutput_Info;
	PREFER_INFO* pPrefer_Info = pDevContext->pPrefer_Info;

	
	wrlen_reg = 0;
	iic_write_ex(WRLEN, wrlen_reg);

	
	frame_rate_in = ((pInput_Info->rx_clk_khz * 10000) / pInput_Info->timing.ht) * 10 / pInput_Info->timing.vt;
	frame_rate_out = ((pOutput_Info->uclk_khz * 10000) / pOutput_Info->timing.ht) * 10 / pOutput_Info->timing.vt;
	field_rate_out = iic_read_ex(INTLC) ? (frame_rate_out * 2) : (frame_rate_out);

	
	hai_down = pInput_Info->timing.ha;
	if(pOutput_Info->ds_percent_h)
	{
		hao_t = pOutput_Info->timing.ha * (100 - pOutput_Info->ds_percent_h) / 100;
	}
	else
	{
		hao_t = pOutput_Info->timing.ha;
	}
	hao_t = hao_t + (hao_t % 2);
	if(hai_down > hao_t)
	{
		dnsmpen_reg = 1;
		hadwn_reg = hao_t;
		hai_down = hadwn_reg;
		fltbp2_reg = 1;
		fltbp1_reg = 1;
	}
	else
	{
		dnsmpen_reg = 0;
		fltbp2_reg = 1;
		fltbp1_reg = 1;
	}
	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		val_t = 720;
	}
	else
	{
		if(pPrefer_Info->dat16_32b)
		{
			val_t = 720;
		}
		else
		{
			val_t = pPrefer_Info->true24 ? 720 : 1440;
		}
	}
	if(hai_down > val_t && (pOutput_Info->rotate != ROTATE_NO || pOutput_Info->h_flip || pOutput_Info->v_flip))
	{
		dnsmpen_reg = 1;
		hadwn_reg = val_t;
		hai_down = hadwn_reg;
		fltbp2_reg = 1;
		fltbp1_reg = 1;
	}
	iic_write_ex(DNSMPEN, dnsmpen_reg);
	iic_write_ex(HADWSPP, hadwn_reg);
	
	if(pInput_Info->timing.ha * 100 / hao_t > pPrefer_Info->hscale_ratio_gate)
	{
		fltbp1_reg = 0;
	}
	iic_write_ex(FLTBP2, fltbp2_reg);
	iic_write_ex(FLTBP1, fltbp1_reg);

	
	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		val_t = 100;
	}
	else
	{
		if(pPrefer_Info->dat16_32b)
		{
			val_t = 100;
		}
		else if(pPrefer_Info->true24)
		{
			val_t = 100;
		}
		else
		{
			val_t = pPrefer_Info->true_com ? 75 : 50;
		}
	}
	if(val_t == 75)
	{
		hai_sdram = (hai_down / 4) * 3 + (hai_down % 4);
	}
	else
	{
		hai_sdram = hai_down * val_t / 100;
	}
	if(pOutput_Info->rotate != ROTATE_NO || pOutput_Info->h_flip || pOutput_Info->v_flip)
	{
		blk_h_reg = 45;
	}
	else if(hai_sdram <= 720 && pInput_Info->timing.va <= 720)
	{
		blk_h_reg = 45;
	}
	else
	{
		blk_h_reg = hai_sdram / 16;
		blk_h_reg = (hai_sdram % 16) ? (blk_h_reg + 1) : blk_h_reg;
	}
	iic_write_ex(BLK_H, blk_h_reg);

	
	if((pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270) && pPrefer_Info->true24 == 0 && pPrefer_Info->true_com == 0 && pPrefer_Info->dat16_32b == 0)
	{
		vai_t = pInput_Info->timing.va / 2;
	}
	else
	{
		vai_t = pInput_Info->timing.va;
	}
	val_t = (vai_t % 16) ? (vai_t / 16 + 1) : (vai_t / 16);
	fba_inc_reg = blk_h_reg * val_t;
	if(fba_inc_reg > 2048)
	{
		sfm_reg = 1;
		thren_reg = 1;
		val_t = (frame_rate_in * 100) / field_rate_out;
		if(val_t > 100)
		{
			
			
			thrrl_reg = pInput_Info->timing.vt * (100 - 10000 / val_t) / 100 + THRRL_ADJUST_DEF;
		}
		else
		{
			thrrl_reg = pInput_Info->timing.vt * (100 - val_t) / 100 + THRRL_ADJUST_DEF;
		}
	}
	else
	{
		sfm_reg = 0;
		thren_reg = 0;
		thrrl_reg = 0;
	}
	iic_write_ex(FBA_INC, fba_inc_reg);
	iic_write_ex(SFM, sfm_reg);
	iic_write_ex(THREN, thren_reg);
	iic_write_ex(THRRL, thrrl_reg);

	
	if(pOutput_Info->ds_percent_v)
	{
		vao_t = pOutput_Info->timing.va * (100 - pOutput_Info->ds_percent_v) / 100;
	}
	else
	{
		vao_t = pOutput_Info->timing.va;
	}
	vao_t = vao_t + (vao_t % 2);
	val_t = iic_read_ex(INTLC) ? (vao_t / 2) : vao_t;
	if(pOutput_Info->rotate == ROTATE_90 || pOutput_Info->rotate == ROTATE_270)
	{
		if(hai_down <= val_t)
			lnsel_reg = 3;
		else
			lnsel_reg = 1;
	}
	else
	{
		if(pInput_Info->timing.va <= val_t)
			lnsel_reg = 3;
		else
			lnsel_reg = 1;
	}
	if(pPrefer_Info->scale_line_adjust)
	{
		lnsel_reg++;
		if((pOutput_Info->ds_percent_v >= 2) && (pOutput_Info->ds_percent_v <= 12) &&
            (pOutput_Info->ds_percent_h <= 12) &&

			(pOutput_Info->channel & CHANNEL_VGA) &&
			(pOutput_Info->timing.ha == 1280) && (pOutput_Info->timing.va == 960) &&
			(pOutput_Info->timing.hz == 60) && (pInput_Info->rx_clk_khz >= 75428)&&
			(pInput_Info->timing.ha == 1280) && (pInput_Info->timing.va == 1024)&&
			(pInput_Info->timing.hz >= 49)){
			
				lnsel_reg++;		

		}

		


		if((pOutput_Info->ds_percent_v >= 2) && (pOutput_Info->ds_percent_v <= 12) &&
            (pOutput_Info->ds_percent_h >= 0) && (pOutput_Info->ds_percent_h <= 12) &&
			(pOutput_Info->channel & CHANNEL_VGA) &&
			(pOutput_Info->timing.ha == 1280) && (pOutput_Info->timing.va == 1024) &&
			(pOutput_Info->timing.hz == 60) && (pInput_Info->rx_clk_khz >= 75428)){
			
				lnsel_reg++;		

		}

		if((pOutput_Info->ds_percent_v >= 2 ) && 
            (pOutput_Info->ds_percent_h >= 2 ) && (pOutput_Info->channel & CHANNEL_VGA) &&
			(pOutput_Info->timing.ha == 1280) && (pOutput_Info->timing.va == 1024) &&
			(pOutput_Info->timing.hz == 75) && (pInput_Info->rx_clk_khz >= 75428)){
			
				lnsel_reg++;
		}

		if((pOutput_Info->ds_percent_v >= 6 ) && 
            (pOutput_Info->ds_percent_h >= 6 ) && (pOutput_Info->channel & CHANNEL_HDMI) &&
			(pOutput_Info->timing.ha == 1920) && (pOutput_Info->timing.va == 1080) &&
			(pOutput_Info->timing.hz == 50) && (pInput_Info->rx_clk_khz >= 75428)){
			
				lnsel_reg++;
		}
		if((pOutput_Info->ds_percent_v >= 6 ) && 
            (pOutput_Info->ds_percent_h >= 6 ) && (pOutput_Info->channel & CHANNEL_HDMI) &&
			(pOutput_Info->timing.ha == 1920) && (pOutput_Info->timing.va == 1080) &&
			(pOutput_Info->timing.hz == 59) && (pInput_Info->rx_clk_khz >= 75428)){
			
				lnsel_reg++;
		}
		if((pOutput_Info->ds_percent_v >= 6 ) && 
            (pOutput_Info->ds_percent_h >= 6 ) && (pOutput_Info->channel & CHANNEL_HDMI) &&
			(pOutput_Info->timing.ha == 1920) && (pOutput_Info->timing.va == 1080) &&
			(pOutput_Info->timing.hz == 60) && (pInput_Info->rx_clk_khz >= 75428)){
			
				lnsel_reg++;
		}
	}
	iic_write_ex(LNSEL, lnsel_reg);

	
	if(frame_rate_in >= field_rate_out)
		wrfast_reg = 1;
	else
		wrfast_reg = 0;
	iic_write_ex(WRFAST, wrfast_reg);

	
	chg_hl_reg = (	( 		(pPrefer_Info->dat16_32b == 0)		&&
					(pPrefer_Info->true24 == 0)		&&
					(pPrefer_Info->true_com == 0)		&&
				(	(pOutput_Info->rotate == 0)		&&
					(pOutput_Info->h_flip == 1)	) )	||
				(	(pOutput_Info->rotate == 1)		&&
					(pOutput_Info->h_flip == 0)	)	||
				(	(pOutput_Info->rotate == 3) 		&&
					(pOutput_Info->h_flip == 1) 	)	||
				(	(pOutput_Info->rotate == 2) 		&&
					(pOutput_Info->h_flip == 0)  )   ) 	? 1 : 0;
	iic_write_ex(CHG_HL, chg_hl_reg);

	
	vsmst_reg = 2;
	iic_write_ex(VSMST, vsmst_reg);

	return ch_true;	
}

ch_bool post_cal_and_set(DEV_CONTEXT* pDevContext)
{
	uint32 hdinc_reg, hdinca_reg, hdincb_reg;
	uint32 hinc_reg, hinca_reg, hincb_reg;
	uint32 vinc_reg, vinca_reg, vincb_reg;

	uint32 val_t;
	uint64 temp1;
	

	
	I2CWrite(pDevContext,0x03, 0x04);
	
	val_t = I2CRead(pDevContext,0x2A);
	hinca_reg = (val_t << 3) | (I2CRead(pDevContext,0x2B) & 0x07);
	
	val_t = I2CRead(pDevContext,0x2C);
	hincb_reg = (val_t << 3) | (I2CRead(pDevContext,0x2D) & 0x07);
	
	val_t = I2CRead(pDevContext,0x2E);
	vinca_reg = (val_t << 3) | (I2CRead(pDevContext,0x2F) & 0x07);
	
	val_t = I2CRead(pDevContext,0x30);
	vincb_reg = (val_t << 3) | (I2CRead(pDevContext,0x31) & 0x07);
	
	val_t = I2CRead(pDevContext,0x32); 
	hdinca_reg = (val_t << 3) | (I2CRead(pDevContext,0x33) & 0x07);
	
	val_t = I2CRead(pDevContext,0x34);
	hdincb_reg = (val_t << 3) | (I2CRead(pDevContext,0x35) & 0x07);

	
	I2CWrite(pDevContext,0x03, 0x04);

	
	if(iic_read_ex(DNSMPEN))
	{
		if(hdincb_reg == 0)
		{
			g_nLastError = ERR_CAL_INC_DIV_ZERO;
			return ch_false;
		}
		temp1 = ((uint64)hdinca_reg) * (1 << 20);
#ifdef T_LINUX
		do_div(temp1 , hdincb_reg);
		hdinc_reg = (uint32)temp1;
#else
		hdinc_reg = (uint32)((uint64)temp1  / hdincb_reg);
#endif
		

		PD_DEBUG ("#1 hdinc_reg = 0x%.8X\n", hdinc_reg);
		I2CWrite(pDevContext,0x3C, (hdinc_reg >> 16) & 0xFF);
		I2CWrite(pDevContext,0x3D, (hdinc_reg >>  8) & 0xFF);
		I2CWrite(pDevContext,0x3E, (hdinc_reg >>  0) & 0xFF);
	}
	if(hincb_reg == 0 || vincb_reg == 0)
	{
		g_nLastError = ERR_CAL_INC_DIV_ZERO;
		return ch_false;
	}
	
	if(hinca_reg > hincb_reg)
	{
		g_nLastError = ERR_CAL_INC_NO_MATH;
		return ch_false;
	}

	temp1 = (uint64)hinca_reg * (1 << 20);
#ifdef T_LINUX
	do_div(temp1 , hincb_reg);
	hinc_reg = (uint32)temp1;
#else
	hinc_reg = (uint32)((uint64)temp1 / hincb_reg);
#endif
	
PD_DEBUG ("#2 hinc_reg = 0x%.8X\n", hinc_reg);

	temp1 = (uint64)vinca_reg * (1 << 20);
#ifdef T_LINUX
	do_div( temp1 , vincb_reg);
	vinc_reg = (uint32)temp1;
#else
	vinc_reg = (uint32)((uint64)temp1 / vincb_reg);
#endif
	
	PD_DEBUG ("#1 vinc_reg = 0x%.8X\n", vinc_reg);
	
	I2CWrite(pDevContext,0x36, (hinc_reg >> 16) & 0xFF);
	I2CWrite(pDevContext,0x37, (hinc_reg >>  8) & 0xFF);
	I2CWrite(pDevContext,0x38, (hinc_reg >>  0) & 0xFF);

	I2CWrite(pDevContext,0x39, (vinc_reg >> 16) & 0xFF);
	I2CWrite(pDevContext,0x3A, (vinc_reg >>  8) & 0xFF);
	I2CWrite(pDevContext,0x3B, (vinc_reg >>  0) & 0xFF);

	return ch_true;
}






ch_bool cal_and_set_power(DEV_CONTEXT* pDevContext)
{
	
	uint8 hdmi_pd = 1;
	uint8 i2s_pd = 1;
	uint8 spdif_pd = 1;
	uint8 auddac=1;
	uint8 dri_pd = 1;
	uint8 dri_pd_ser = 1;
	uint8 dri_pd_pll = 1;
	uint8 dri_pddri = 7;
	uint8 cec_pd = 1; 
	uint8 pd_ddc = 1;

	
	uint8 lvds_pd = 1;
	uint8 rx_pd = 63;
	uint8 rxpll_pd = 1;
	uint8 txpll_pd = 1;
	uint8 txdrv_pd = 1;
	uint8 txser_pd = 1;

	
	


	uint8 icen0 =1;		
	

	
	uint8 vga_pd = 1;
	uint8 pddac = 7;

	uint8 pdpll1 = 1;
	uint8 pdpll0 = 1;


	uint8 pd_prom = 0; 
	uint8 pdmio = 1;
	uint8 pdio = 1;
	uint8 hpd_pd = 0; 

	
	uint8 sclpd = 1;
	uint8 sdpd = 1;
	uint8 mempd = 1;
	uint8 dacsence = 1;

	uint8 gckoff = 0; 

	
	uint8 dispon = 0;




	
	INPUT_INFO* pInput_Info = pDevContext->pInput_Info;
	OUTPUT_INFO* pOutput_Info = pDevContext->pOutput_Info;
	PREFER_INFO* pPrefer_Info = pDevContext->pPrefer_Info;

	
	iic_write_ex(CRYS_FREQ_SPP, 27000);
	iic_write_ex(I2SCK_SEC, 1);
	iic_write_ex(SP_EN, 0);
	iic_write_ex(HARD_SOFTB, 1);
	iic_write_ex(MULT_I2CEN, 0);

	
	
	if(pOutput_Info->channel & CHANNEL_LVDS)
	{
		lvds_pd = 0;
		rx_pd = 0;
		rxpll_pd = 0;
		txpll_pd = 0;
		txdrv_pd = 0;
		txser_pd = 0;
		pdio = 0; 
	

	}
	if(pOutput_Info->channel & CHANNEL_HDMI)
	{
		hdmi_pd = 0;
		if(pInput_Info->audio_type == 1)
			i2s_pd = 0;
		else {
			spdif_pd = 0;
			auddac=0;
		}
		dri_pd = 0;
		dri_pddri = 0;
		dri_pd_ser = 0;
		dri_pd_pll = 0;		
		pd_ddc = 0;
		
		vga_pd = 0;
		pdmio = 0;
		sclpd = 0;
		sdpd = 0;
		gckoff = 0;
		mempd = 0;

		rx_pd = 0; 
		rxpll_pd = 0;
		
		pdpll0 = 0;
		pdpll1 = 0;
		icen0 =0;		
		pdio = 0;

		hpd_pd = 0;
		
	}
	if(pOutput_Info->channel & CHANNEL_VGA)
	{
		vga_pd = 0;
		pdmio = 0;	
		sclpd = 0;
		sdpd = 0;
		gckoff = 0;
		mempd = 0;
		
		rx_pd = 0;
		rxpll_pd = 0;	
		
		pdpll0 = 0;
		pdpll1 = 0;
		icen0 =0;
		pdio = 0;

		dispon = 1;
		dri_pd_pll = 0;
				
		dacsence=1;		
		pddac = 0;
	}else{
		
		

		if(pPrefer_Info->vga_enable)
           dispon = 0;
	}

	
	iic_write_ex(HDMI_PD,		hdmi_pd);
	iic_write_ex(I2S_PD,		i2s_pd);
	iic_write_ex(SPDIF_PD,		spdif_pd);

	iic_write_ex(AUDDAC,        auddac);

	iic_write_ex(DRI_PD,		dri_pd);
	iic_write_ex(DRI_PD_SER,	dri_pd_ser);
	iic_write_ex(DRI_PD_PLL,	dri_pd_pll);
	iic_write_ex(DRI_PDDRI,		dri_pddri);
	iic_write_ex(CEC_PD,		cec_pd);
	iic_write_ex(PD_DDC,		pd_ddc);
	iic_write_ex(LVDS_PD,		lvds_pd);
	iic_write_ex(RX_PD,			rx_pd);
	iic_write_ex(RXPLL_PD,		rxpll_pd);
	iic_write_ex(TXPLL_PD,		txpll_pd);
	iic_write_ex(TXDRV_PD,		txdrv_pd);
	iic_write_ex(TXSER_PD,		txser_pd);
	iic_write_ex(VGA_PD,		vga_pd);
	iic_write_ex(PDDAC,			pddac);
	iic_write_ex(PDPLL1,		pdpll1);
	iic_write_ex(PDPLL0,		pdpll0);
	iic_write_ex(ICEN0,			icen0);
	iic_write_ex(PD_PROM,		pd_prom);
	iic_write_ex(PDMIO,			pdmio);
	iic_write_ex(PDIO,			pdio);
	iic_write_ex(HPD_PD,		hpd_pd);
	iic_write_ex(SCLPD,			sclpd);
	iic_write_ex(SDPD,			sdpd);
	iic_write_ex(MEMPD,			mempd);
	iic_write_ex(DISPON,		dispon);
	
	iic_write_ex(DACSENCE,			dacsence);	
	iic_write_ex(GCKOFF,			gckoff);
	iic_write_ex(ZRCTS, 0);

	return ch_true;
}

ch_bool convert_pll1n1_div(ch_bool pll1n1_to_div, uint8* pll1n1_addr, uint8* div_addr)
{
	uint8 val_t;

	if(pll1n1_to_div)	
	{
		if((*pll1n1_addr) <= 5)
		{
			(*div_addr)= 1<<(*pll1n1_addr);
		}
		else if((*pll1n1_addr) == 6 || (*pll1n1_addr) == 7) 
		{
			(*div_addr) = 64;
		}
		else
		{
			return ch_false;
		}
	}
	else 
	{
		for(val_t=1; val_t<=6; ++val_t)
		{
			if(*div_addr == (1 << val_t))
			{
				*pll1n1_addr = val_t;
				return ch_true;
			}
		}
		return ch_false;
	}
	return ch_true;
}

ch_bool convert_pll1n2_div(ch_bool pll1n2_to_div, uint8* pll1n2_addr, uint8* div_addr)
{
	uint8 val_t;

	if(pll1n2_to_div)	
	{
		if((*pll1n2_addr) <= 3)
		{
			*div_addr = 1 << (*pll1n2_addr);
		}
		else if((4 == (*pll1n2_addr)) || (6 == (*pll1n2_addr)))
		{
			(*div_addr) = 16;
		}
		else if((5 == (*pll1n2_addr)) || (7 == (*pll1n2_addr)))
		{
			(*div_addr) = 32;
		} 
		else 
		{
			return ch_false;
		}
	}
	else	
	{
		for(val_t=0; val_t<=5; ++val_t)
		{
			if(*div_addr == (1 << val_t))
			{
				*pll1n2_addr = val_t;
				return ch_true;
			}
		}
		return ch_false;
	}
	return ch_true;
}

ch_bool convert_pll1n3_div(ch_bool pll1n3_to_div, uint8* pll1n3_addr, uint8* div_addr)
{
	uint8 val_t;
	
	if(pll1n3_to_div)	
	{
		if((*pll1n3_addr) <= 3)
		{
			*div_addr = 1 << (*pll1n3_addr);
		}
		else if((4 == (*pll1n3_addr)) || (6 == (*pll1n3_addr)))
		{
			(*div_addr) = 16;
		}
		else if((5 == (*pll1n3_addr)) || (7 == (*pll1n3_addr)))
		{
			(*div_addr) = 32;
		} 
		else 
		{
			return ch_false;
		}
	}
	else	
	{
		for(val_t=0; val_t<=5; ++val_t)
		{
			if(*div_addr == (1 << val_t))
			{
				*pll1n3_addr = val_t;
				return ch_true;
			}
		}
		return ch_false;
	}
	return ch_true;
}

ch_bool convert_pll2n5_div(ch_bool pll2n5_to_div, uint8* pll2n5_addr, uint8* div_addr)
{
	if(pll2n5_to_div)	
	{
		switch(*pll2n5_addr)
		{
		case 0:		
			*div_addr = 1;	
			break;
		case 4:		
			*div_addr = 2;	
			break;
		case 1:    
			*div_addr = 3;
			break;
		case 8:    
			*div_addr = 4;
			break;
		case 2:    
			*div_addr = 5;
			break;
		case 5:    
			*div_addr = 6;
			break;
		case 3:    
			*div_addr = 7;
			break;
		case 12:
			*div_addr = 8;
			break;
		default :
			return ch_false;
		}
	}
	else	
	{
		switch(*div_addr)
		{
		case 1:    
			*pll2n5_addr = 0;
			break;
		case 2:    
			*pll2n5_addr = 4;
			break;
		case 3:    
			*pll2n5_addr = 1;
			break;
		case 4:    
			*pll2n5_addr = 8;
			break;
		case 5:    
			*pll2n5_addr = 2;
			break;
		case 6:    
			*pll2n5_addr = 5;
			break;
		case 7:    
			*pll2n5_addr = 3;
			break;
		case 8:    
			*pll2n5_addr = 0x0C;
			break;
		default : 
			return ch_false;
		}
	}
	return ch_true;
}

ch_bool convert_pll2n6_div(ch_bool pll2n6_to_div, uint8* pll2n6_addr, uint8* div_addr)
{
	if(pll2n6_to_div)
	{
		if(*pll2n6_addr <= 3)
		{
			(*div_addr)= (*pll2n6_addr) + 5;
			return ch_true;
		}
		return ch_false;
	}
	else
	{
		if(*div_addr >= 5 && *div_addr <= 8)
		{
			*pll2n6_addr = (*div_addr) - 5;
			return ch_true;
		}
		return ch_false;
	}
	return ch_true;
}

ch_bool convert_pll2n7_div(ch_bool pll2n7_to_div, uint8* pll2n7_addr, uint8* div_addr)
{
	if(pll2n7_to_div)
	{
		if(*pll2n7_addr <= 3)
		{
			(*div_addr)= (*pll2n7_addr) + 8;
			return ch_true;
		}
		return ch_false;
	}
	else
	{
		if(*div_addr >= 8 && *div_addr <= 11)
		{
			*pll2n7_addr = (*div_addr) - 8;
			return ch_true;
		}
		return ch_false;
	}
	return ch_true;
}

ch_bool convert_pll3n8_div(ch_bool pll3n8_to_div, uint8* pll3n8_addr, uint8* div_addr)
{
	uint8 val_t;

	if(pll3n8_to_div)	
	{
		if(*pll3n8_addr <= 3)
		{
			*div_addr= 1 << (*pll3n8_addr);
			return ch_true;
		}
		return ch_false;
	}
	else	
	{
		for(val_t=0; val_t<=3; ++val_t)
		{
			if(*div_addr == (1 << val_t))
			{
				*pll3n8_addr = val_t;
				return ch_true;
			}
		}
        return ch_false;
	}

	return ch_true;
}

ch_bool convert_dmxtal_div(ch_bool dmxtal_to_div, uint8* dmxtal_addr, uint8* div_addr)
{
	if(dmxtal_to_div)	
	{
		switch(*dmxtal_addr)
		{
		case 0x00:  
			*div_addr = 1;
			break;
		case 0x04: 
			*div_addr = 2;
			break;
		case 0x08: 
			*div_addr = 3;
			break;
		case 0x0C:  
			*div_addr = 4;
			break;
		case 0x10: 
			*div_addr = 5;
			break;
		case 0x11: 
			*div_addr = 6;
			break;
		case 0x12: 
			*div_addr = 7;
			break;
		case 0x13: 
			*div_addr = 8;
			break;
		case 0x01: 
			*div_addr = 9;
			break;
		case 0x14: 
			*div_addr = 10;
			break;
		case 0x02: 
			*div_addr = 11;
			break;
		case 0x15: 
			*div_addr = 12;
			break;
		case 0x03: 
			*div_addr = 13;
			break;
		case 0x16: 
			*div_addr = 14;
			break;
		case 0x18: 
			*div_addr = 15;
			break;
		case 0x17: 
			*div_addr = 16;
			break;
		default:  
			return ch_false;
		}
	}
	else	
	{
		switch(*div_addr)
		{
		case 1:  
			*dmxtal_addr = 0x00;
			break;
		case 2: 
			*dmxtal_addr = 0x04;
			break;
		case 3: 
			*dmxtal_addr = 0x08;
			break;
		case 4: 
			*dmxtal_addr = 0x0C;
			break;
		case 5:
			*dmxtal_addr = 0x10;
			break;
		case  6: 
			*dmxtal_addr = 0x11;
			break;
		case 7: 
			*dmxtal_addr = 0x12;
			break;
		case 8: 
			*dmxtal_addr = 0x13;
			break;
		case 9: 
			*dmxtal_addr = 0x01;
			break;
		case 10: 
			*dmxtal_addr = 0x14;
			break;
		case 11: 
			*dmxtal_addr = 0x02;
			break;
		case 12: 
			*dmxtal_addr = 0x15;
			break;
		case 13: 
			*dmxtal_addr = 0x03;
			break;
		case 14: 
			*dmxtal_addr = 0x16;
			break;
		case 15:
			*dmxtal_addr = 0x18;
			break;
		case 16: 
			*dmxtal_addr = 0x17;
			break;
		default: 
			return ch_false;
		}
	}
	return ch_true;
}



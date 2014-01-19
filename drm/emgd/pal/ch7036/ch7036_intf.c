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
* @file  ch7036_intf.c
* @version 1.2.5
*-----------------------------------------------------------------------------
*/


#include "ch7036_intf.h"
#include "ch7036_attr.h"


#ifdef T_LINUX
	#include "lvds/lvds.h"
#else
	#include "lvds.h"
#endif


ch7036_status_t ch7036_get_hdmi_hpd(ch7036_device_context_t* p_ctx, uint8 *hpd);

unsigned long ch7036_invis_6x4_modes_table_size[3] = {2,3,5}; 
unsigned long ch7036_invis_8x6_7x4_table_size[3] = {3,5,6};

OUT_FMT hdmi_timing_table[OUT_HDMI_END] = {
	
		                    
	{1,     25175,  1, {800,   640, 16,  96, 525,  480, 10,  2, 59, SCANTYPE_PROGRESSIVE} }, 
    {1,     25250,  1, {800,   640, 16,  96, 525,  480, 10,  2, 60, SCANTYPE_PROGRESSIVE} },
    {2,     27000,  1, {858,   720, 16,  62, 525,  480,  9,  6, 59, SCANTYPE_PROGRESSIVE} },
    {2,     27027,  1, {858,   720, 16,  62, 525,  480,  9,  6, 60, SCANTYPE_PROGRESSIVE} },
	{17,     27000,  1, {864,  720,  12,  64, 625,  576,  5,  5, 50, SCANTYPE_PROGRESSIVE} },

    {4,     74176,  2, {1650, 1280, 110, 40, 750,  720,  5,  5, 59, SCANTYPE_PROGRESSIVE} },
    {4,     74250,  2, {1650, 1280, 110, 40, 750,  720,  5,  5, 60, SCANTYPE_PROGRESSIVE} },
    {5,     74176,  2, {2200, 1920, 88,  44, 1125, 1080, 2,  5, 59, SCANTYPE_INTERLACED} },
    {5,     74250,  2, {2200, 1920, 88,  44, 1125, 1080, 2,  5, 60, SCANTYPE_INTERLACED} },
	{16,    148350,  2, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 59, SCANTYPE_PROGRESSIVE} },
//   {16,    148352,  2, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 59, SCANTYPE_PROGRESSIVE} }, 
   {16,    148500,  2, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 60, SCANTYPE_PROGRESSIVE} }, 
 
//   {17,     27000,  1, {864,  720,  12,  64, 625,  576,  5,  5, 50, SCANTYPE_PROGRESSIVE} }, 
   {19,     74250,  2, {1980, 1280, 440, 40, 750,  720,  5,  5, 50, SCANTYPE_PROGRESSIVE} }, 
   
   {20,     74250,  2, {2640, 1920, 528, 44, 1125, 1080, 2, 5, 50, SCANTYPE_INTERLACED} },


   {31,    148500,  2, {2640, 1920, 528, 44, 1125, 1080, 4,  5, 50, SCANTYPE_PROGRESSIVE} },
   {32,     74175,  2, {2750, 1920, 638, 44, 1125, 1080, 4,  5, 23, SCANTYPE_PROGRESSIVE} },
   {32,     74250,  2, {2750, 1920, 638, 44, 1125, 1080, 4,  5, 24, SCANTYPE_PROGRESSIVE} },
   {33,     74250,  2, {2640, 1920, 528, 44, 1125, 1080, 4,  5, 25, SCANTYPE_PROGRESSIVE} },
   {34,     74175,  2, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 29, SCANTYPE_PROGRESSIVE} },
   {34,     74250,  2, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 30, SCANTYPE_PROGRESSIVE} }, 

   {40,    148500,  2, {2640, 1920, 528, 44, 1124, 1080, 4, 10, 100, SCANTYPE_INTERLACED} },
   {41,    148500,  2, {1980, 1280, 440, 40, 750,  720,  5,  5, 100, SCANTYPE_PROGRESSIVE} },
   {42,     54000,  1, {864,   720, 12,  64, 625,  576,  5,  5, 100, SCANTYPE_PROGRESSIVE} },


   {46,    148352,  2, {2200, 1920, 88,  44, 1124, 1080, 4, 10, 119, SCANTYPE_INTERLACED} },
   {46,    148500,  2, {2200, 1920, 88,  44, 1124, 1080, 4, 10, 120, SCANTYPE_INTERLACED} },

   {47,    148352,  2, {1650, 1280, 110, 40, 750,  720,  5,  5, 119, SCANTYPE_PROGRESSIVE} },
   {47,    148500,  2, {1650, 1280, 110, 40, 750,  720,  5,  5, 120, SCANTYPE_PROGRESSIVE} },
   
   {48,     54000,  1, {858,   720, 16,  62, 525,  480,  9,  6, 119, SCANTYPE_PROGRESSIVE} },
   {48,     54054,  1, {858,   720, 16,  62, 525,  480,  9,  6, 120, SCANTYPE_PROGRESSIVE} }, 
  
   {52,    108000,  1, {864,   720, 12,  64, 625,  576,  5,  5, 200, SCANTYPE_PROGRESSIVE} },

   {56,    108000,  1, {858,   720, 16,  62, 525,  480,  9,  6, 239, SCANTYPE_PROGRESSIVE} },
   {56,    108108,  1, {858,   720, 16,  62, 525,  480,  9,  6, 240, SCANTYPE_PROGRESSIVE} }, 
  
};

OUT_FMT dvi_timing_table[OUT_DVI_END] = {
	


	{0,     25170,  0, { 800,  640, 16,  96, 525,  480, 10,  2,  60, SCANTYPE_PROGRESSIVE}	}, 
	{0,     31500,  0, { 832,  640, 24,  40, 520,  480,  9,  3,  72, SCANTYPE_PROGRESSIVE}	},  
    {0,     31500,  0, { 800,  640, 16,  96, 525,  480, 11,  2,  75, SCANTYPE_PROGRESSIVE}	},  

	{0,     28322,  0, { 900,  720, 15, 108, 449,  400, 11,  2,  70, SCANTYPE_PROGRESSIVE}	},  
	
	{0,		38100,	0, {1088,  800,	32, 128, 619,  600,  1,  4,  56, SCANTYPE_PROGRESSIVE}	},
	

	{0,		40000,	0, {1056,  800,	40, 128, 628,  600,  1,  4,  60, SCANTYPE_PROGRESSIVE}	},
	{0,		50000,	0, {1040,  800,	56, 120, 666,  600, 37,  6,  72, SCANTYPE_PROGRESSIVE}	},
	{0,		49500,	0, {1056,  800,	16,  80, 624,  600,  1,  2,  75, SCANTYPE_PROGRESSIVE}	},

	{0,		65000,	0, {1344, 1024,	24,	136, 806,  768,	 3,	 6,  60, SCANTYPE_PROGRESSIVE}	}, 
    {0,		75000,	0, {1328, 1024,	24,	136, 806,  768,	 3,	 6,  70, SCANTYPE_PROGRESSIVE}	},
	{0,		78750,	0, {1312, 1024,	16,	 96, 800,  768,	 1,	 3,  75, SCANTYPE_PROGRESSIVE}	},

	{0,		81517,	0, {1688, 1152,	48,112,1066,  864,	 1,	 3,  60, SCANTYPE_PROGRESSIVE}	},

	{0,		83460,	0, {1680, 1280,	64,	136, 828,  720,  1,	 3,  60, SCANTYPE_PROGRESSIVE}	}, 
	
	{0,		83865,	0, {1680, 1280,	64,	136, 828,  800,  1,	 3,  60, SCANTYPE_PROGRESSIVE}	},
	
	{0,		100638,	0, {1688, 1280,	48,	112, 1066, 960,  1,	 3,  60, SCANTYPE_PROGRESSIVE}	},

	{0,		108000,	0, {1688, 1280,	48,	112, 1066, 1024, 1,	 3,  60, SCANTYPE_PROGRESSIVE}	}, 
	{0,		135000,	0, {1688, 1280,	16,	144, 1066, 1024, 1,	 3,  75, SCANTYPE_PROGRESSIVE}	},

	{0,		85543,	0, {2256, 1360,104, 184, 1087, 768,  1,	 3,  60, SCANTYPE_PROGRESSIVE}	},
	{0,		85920,	0, {2256, 1366,104, 184, 1087, 768,  1,	 3,  60, SCANTYPE_PROGRESSIVE}	},

	{0,		121750,	0, {1864, 1400,	88,144,1089,  1050,	 3,	 4,  60, SCANTYPE_PROGRESSIVE}	}, 
    {0,		156000,	0, {1896, 1400,104,144,1099,  1050,	 3,	 4,  75, SCANTYPE_PROGRESSIVE}	},


	{0,		88750,	0, {1600, 1440,	48,	32,	 926,  900,	 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},
	{0,		119000,	0, {1840, 1440,	48,	32,	 1080, 1050, 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},
 	
	{0,		117936,	0, {2256, 1600,104,	184, 1250,  900, 1,	 3,  60, SCANTYPE_PROGRESSIVE}	},
	{0,		162000,	0, {2160, 1600,	64,	192, 1250, 1200, 1,	 3,  60, SCANTYPE_PROGRESSIVE}	},

	{0,		147130,	0, {2256, 1680,104,	184, 1087, 1050, 1,	 3,  60, SCANTYPE_PROGRESSIVE}	},
	{0,     148500, 0, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 60, SCANTYPE_PROGRESSIVE} },
	{0,		154000,	0, {2080, 1920,	48,	 32, 1235, 1200, 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},

};




OUT_FMT ch7036_crt_timing_table[OUT_CRT_END] = {
   
   
   {100,    31500,  1, {832,   640, 32,  64,  445,  400, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },
	 
   {100,    25175,  1, {800,   640,  8,  96,  525,  480, 2,  2,  60, SCANTYPE_PROGRESSIVE}  },
   {100,    31500,  1, {832,   640, 16,  40,  520,  480, 1,  3,  72, SCANTYPE_PROGRESSIVE}  },
   {100,    31500,  1, {840,   640, 16,  64,  500,  480, 1,  3,  75, SCANTYPE_PROGRESSIVE}  },
   {100,    36000,  1, {832,   640, 56,  56,  509,  480, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },
   
   {100,    35500,  1, {936,   720, 36,  72,  446,  400, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },
   

   
   {100,    36000,  1, {1024,  800, 24,  72,  625,  600, 1,  2,  56, SCANTYPE_PROGRESSIVE}  },
   

   {100,    38250,  1, {1024,  800, 32,  80,  624,  600, 3,  4,  60, SCANTYPE_PROGRESSIVE}  },
   {100,    50000,  1, {1040,  800, 56, 120,  666,  600, 37, 6,  72, SCANTYPE_PROGRESSIVE}  },
   {100,    49000,  1, {1040,  800, 40,  80,  629,  600, 3,  4,  75, SCANTYPE_PROGRESSIVE}  },
   {100,    56750,  1, {1056,  800, 48,  80,  633,  600, 3,  4,  85, SCANTYPE_PROGRESSIVE}  },

    
   {100,    65000,  1, {1344, 1024, 24, 136,  806,  768, 3,  6,  60, SCANTYPE_PROGRESSIVE}  },
   {100,    75000,  1, {1328, 1024, 24, 136,  806,  768, 3,  6,  70, SCANTYPE_PROGRESSIVE}  },
   {100,    78750,  1, {1312, 1024, 16,  96,  800,  768, 1,  3,  75, SCANTYPE_PROGRESSIVE}  }, 
   
   {100,    94500,  1, {1376, 1024, 48,  96,  808,  768, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },
   
    
   {100,   108000,  1, {1600, 1152, 64, 128,  900,  864, 1,  2,  75, SCANTYPE_PROGRESSIVE}  },

   
   {100,    79500,  1, {1664, 1280, 64, 128,  798,  768, 3,  7,  60, SCANTYPE_PROGRESSIVE}  },
   {100,   102250,  1, {1696, 1280, 80, 128,  805,  768, 3,  7,  75, SCANTYPE_PROGRESSIVE}  },
   {100,   117500,  1, {1712, 1280, 80, 136,  809,  768, 3,  7,  85, SCANTYPE_PROGRESSIVE}  },
  
   
   {100,   108000,  1, {1800, 1280, 96, 112, 1000,  960, 1,  3,  60, SCANTYPE_PROGRESSIVE}  },
   {100,   148500,  1, {1728, 1280, 64, 160, 1011,  960, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },

    
   {100,   108000,  1, {1688, 1280, 48, 112, 1066, 1024, 1,  3,  60, SCANTYPE_PROGRESSIVE}  },
   {100,   135000,  1, {1688, 1280, 16, 144, 1066, 1024, 1,  3,  75, SCANTYPE_PROGRESSIVE}  },
   {100,   157500,  1, {1728, 1280, 64, 160, 1072, 1024, 1,  3,  85, SCANTYPE_PROGRESSIVE}  },

   
   {100,    85500,	1, {1792, 1360,	64, 112,  795,  768, 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},

   
   {100,   121750,	1, {1864, 1400,	88,144,1089,  1050,	 3,	 4,  60, SCANTYPE_PROGRESSIVE}	},
   {100,   156000,	1, {1896, 1400,104,144,1099,  1050,	 3,	 4,  75, SCANTYPE_PROGRESSIVE}	},

   
   {100,	88750,	1, {1600, 1440,	48,	32,	 926,  900,	 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},

   
   {100,   119000,	1, {1840, 1440,	48,	32,	 1080, 1050, 3,	 6,  60, SCANTYPE_PROGRESSIVE}	},

   
   {100,   117936,	1, {2256, 1600,104,	184, 1250,  900, 1,	 3,  60, SCANTYPE_PROGRESSIVE}	},
   
   
   {100,   162000,  1, {2160, 1600, 64, 192, 1250, 1200, 1,  3,  60, SCANTYPE_PROGRESSIVE}  },
   
   
   
   {100,   148500, 1, {2200, 1920, 88,  44, 1125, 1080, 4,  5, 60, SCANTYPE_PROGRESSIVE} },
   
   
   
};





uint8 I2CRead(DEV_CONTEXT* pDevContext,uint8 index)
{	
	
	ch7036_device_context_t *p_ctx= pDevContext->pd_context;
	pd_reg_t reg_list[2];

	reg_list[0].reg = (i2c_reg_t)index;
	reg_list[1].reg = PD_REG_LIST_END;

	p_ctx->p_callback->read_regs(p_ctx->p_callback->callback_context, reg_list,PD_REG_DDC);

	return (uint8)(reg_list[0].value);
}

void I2CWrite(DEV_CONTEXT* pDevContext,uint8 index, uint8 value)
{
	ch7036_device_context_t *p_ctx= pDevContext->pd_context;
	pd_reg_t reg_list[2];

	

	reg_list[0].reg = (i2c_reg_t)index;
	reg_list[0].value = (i2c_reg_t)value;

	reg_list[1].reg = PD_REG_LIST_END;

	p_ctx->p_callback->write_regs(p_ctx->p_callback->callback_context, reg_list,PD_REG_DDC);

	return;
}


void I2CBlockWrite(DEV_CONTEXT* pDevContext,uint8 index, uint8* value, uint16 len)
{
	ch7036_device_context_t *p_ctx= pDevContext->pd_context;
	pd_reg_t reg_list[MAX_I2C_BLOCK_SIZE +1]; 
	uint16 i=0;
	

	for(i=0;i<len;i++) {
		reg_list[i].reg = (i2c_reg_t)index;
		reg_list[i].value = (i2c_reg_t)value[i];
	}

	reg_list[len].reg = PD_REG_LIST_END;

	p_ctx->p_callback->write_regs(p_ctx->p_callback->callback_context, reg_list,PD_REG_DDC);

	return;
}

ch7036_status_t ch7036_device_prepare(ch7036_device_context_t* p_ctx)
{
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;
	ch7036_status_t status = SS_SUCCESS;

	PD_DEBUG("ch7036_intf: ch7036_device_prepare()\n");


	if(!DevicePrepare(p_ch_ctx))
	{
		p_ctx->last_emsg = GetLastErrorMessage(); 
		status = SS_UNSUCCESSFUL; 
	}
	
	return status;
}

ch7036_status_t ch7036_device_config(ch7036_device_context_t* p_ctx)
{
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;
	ch7036_status_t status = SS_SUCCESS;

	PD_DEBUG("ch7036_intf: ch7036_device_config()\n");

	
	if(!DeviceConfig(p_ch_ctx))
	{
		p_ctx->last_emsg = GetLastErrorMessage();
		status = SS_UNSUCCESSFUL;
	}
	
	return status;
	
}

ch7036_status_t ch7036_device_start(ch7036_device_context_t* p_ctx)
{
	
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;
	ch7036_status_t status = SS_SUCCESS;

	PD_DEBUG("ch7036_intf: ch7036_device_start()\n");
	
	if(!DeviceRunning(p_ch_ctx))
	{
		p_ctx->last_emsg = GetLastErrorMessage();
		status = SS_UNSUCCESSFUL;
	}
	
	return status;
	
}

ch7036_status_t ch7036_device_set_power(ch7036_device_context_t* p_ctx, unsigned long channel)
{
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;
	ch7036_status_t status = SS_SUCCESS;

	PD_DEBUG("ch7036_intf: ch7036_device_set_power()- channel [%x]\n", channel);
	
	if(!DeviceSetPower(p_ch_ctx,channel))
	{
		p_ctx->last_emsg = GetLastErrorMessage();
		status = SS_UNSUCCESSFUL;
	}
	
	return status;
}



ch7036_status_t ch7036_load_firmware(ch7036_device_context_t* p_ctx)
{
	
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;

	ch7036_status_t status = SS_UNSUCCESSFUL;

	PD_DEBUG("ch7036: ch7036_load_firmware()\n");

	
	if(LHFM_load_firmware(p_ch_ctx) == -1) {
		PD_DEBUG("ch7036_load_firmware: LHFM_load_firmware()- firmware loading FAILED...\n");
		
	}
	else  {
		PD_DEBUG("ch7036_load_firmware: LHFM_load_firmware()- firmware loading is a SUCCESS\n");
		status = SS_SUCCESS;
	}
	

	return status;

}






ch7036_status_t ch7036_get_hdvi_display_modes_supported(ch7036_device_context_t* p_ctx) 
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	ch7036_status_t status;
	ch7036_edid_blk_t* p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;


	PD_DEBUG("ch7036_get_hdvi_display_modes_supported()- enter\n");

	status = LHFM_get_hdmi_modeinfo(p_ch7xxx_context,p_hedid->supported_modes);

	if (status == SS_SUCCESS) {

		PD_DEBUG("HDMI_Modes=%02X. Vesa_Modes=%02x\r\n", p_hedid->supported_modes[13], p_hedid->supported_modes[14]);

						
	}
	else {
		PD_DEBUG("ch7036_get_hdvi_display_modes_supported()-- failed!\r\n");
		PD_DEBUG("status: [%s]\n",status == SS_FIRMWARE_TIMEOUT?"timeout!":"firmware_error!");

		
	}
	
	return status;
}



ch7036_status_t ch7036_read_edid(ch7036_device_context_t* p_ctx, uint32 channel)
{
	
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	ch7036_status_t status = SS_UNSUCCESSFUL; 
	
#ifdef T_CH7036_EDID_DUMP
	uint8 ebn;
	int i;
#endif

	ch7036_edid_blk_t* p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;
	ch7036_edid_blk_t* p_cedid = (ch7036_edid_blk_t *)p_ctx->cedid;


	unsigned char* hedidblk = p_hedid->edidblk;
	unsigned char* cedidblk = p_cedid->edidblk;



	switch (channel) {

		case CHANNEL_LVDS_HDMI:
		case CHANNEL_HDMI:

			
			status = LHFM_get_edid(p_ch7xxx_context,hedidblk, &(p_hedid->ebn), CH7036_HDMI_DDC);

			if(status == SS_SUCCESS) {	
			
				PD_DEBUG("ch7036_read_edid()-hdmi-dvi hpd status- attached, hdmi-dvi edid read is a SUCCESS\n");
				PD_DEBUG("ch7036_read_edid()- number of blocks read [%x]\n", p_hedid->ebn);
			
				p_hedid->is_edid = 1;
				if(p_hedid->ebn == 1)  
					pOutput_Info->hdmi_fmt.is_dvi_mode =1;
							
				else 
					pOutput_Info->hdmi_fmt.is_dvi_mode =0;
				

		
#ifdef T_CH7036_EDID_DUMP
	
				if (p_hedid->ebn <= MAX_EDID_BLOCKS) { 
					for (i=0; i<p_hedid->ebn; i++) 
						ch7036_dump("HDMI-DVI EDID Data", 128, &hedidblk[i*128]);
								
				}

#endif

			} 

			else { 
				p_hedid->is_edid = 0;
				
				status = SS_UNSUCCESSFUL;

			}	

			break;

		case CHANNEL_LVDS_VGA:
		case CHANNEL_VGA:
			
			I2CWrite(p_ch7xxx_context,0x03, 0x01);			
			I2CWrite(p_ch7xxx_context,0x0F, I2CRead(p_ch7xxx_context,0x0F) & 0x7F);		
			pd_usleep(200);

			status = LHFM_get_edid(p_ch7xxx_context,cedidblk, &(p_cedid->ebn), CH7036_VGA_DDC);
			if (status== SS_SUCCESS) {
				p_cedid->is_edid =1;
				PD_DEBUG("ch7036_read_edid()- crt hpd status- attached, crt edid read is a SUCCESS\n");
				PD_DEBUG("ch7036_read_edid()- number of blocks read [%x]\n", p_cedid->ebn);

#ifdef T_CH7036_EDID_DUMP
				if (p_cedid->ebn <= MAX_EDID_BLOCKS) 
					for (i=0; i<p_cedid->ebn; i++) 
						ch7036_dump("VGA EDID Data", 128, &cedidblk[i*128]);


#endif

			}
			else {
				p_cedid->is_edid =0;
					
				status = SS_UNSUCCESSFUL;
			}

	
			break;

		default:
			break;


	} 


	return status;
}

ch7036_status_t ch7036_get_hdmi_hpd(ch7036_device_context_t* p_ctx, uint8 *hpd)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	ch7036_status_t status= SS_SUCCESS;
	unsigned char reg, reg_hpdpw;

	I2CWrite(p_ch7xxx_context,0x03, 0x03);
	reg = I2CRead(p_ch7xxx_context,0x25);

	I2CWrite(p_ch7xxx_context,0x03, 0x04);
	reg_hpdpw = I2CRead(p_ch7xxx_context,0x51);

#ifdef T_CONFIG_PLB	
	if(!(reg & 0x10) && (reg_hpdpw & 0x80) ){
		reg= 0x10;
	}
#endif

	switch (reg & 0x10) {
		case 0x10:
			if(p_ctx->hpd & CH7036HPD_HDVI_ATTACHED)
				*hpd = 0x01;
			else
				*hpd = 0x81;
			break;
		case 0x00:
			if(p_ctx->hpd & CH7036HPD_HDVI_ATTACHED)
				*hpd = 0x80;
			else
				*hpd = 0x00;
			break;
	}
	

	PD_DEBUG("ch7036: ch7036_get_hdmi_hpd- exit...*hpd [0x%x]\n",*hpd);

	return status;

}
ch7036_status_t ch7036_get_attached_device(ch7036_device_context_t* p_ctx)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	uint8 reg;
	ch7036_status_t status = SS_SUCCESS;
	ch7036_status_t status_hdmi, status_crt;

	ch7036_edid_blk_t * p_hedid = (ch7036_edid_blk_t *)(p_ctx->hedid);

	uint8 hpd = 0;


	
	I2CWrite(p_ch7xxx_context,0x03, 0x04);
	reg = I2CRead(p_ch7xxx_context,0x52);
	
	reg = reg & 0xEF; 
	I2CWrite(p_ch7xxx_context,0x52, reg);

	
	I2CWrite(p_ch7xxx_context,0x03, 0x0);
	reg = I2CRead(p_ch7xxx_context,0x07);
	
	reg = reg & 0x70; 
	I2CWrite(p_ch7xxx_context,0x07, reg);
	
	I2CWrite(p_ch7xxx_context,0x03, 0x01);
	reg = I2CRead(p_ch7xxx_context,0x0F);
	
	reg = reg & 0x7F; 	
	I2CWrite(p_ch7xxx_context,0x0F, reg); 

	I2CWrite(p_ch7xxx_context,0x0E, I2CRead(p_ch7xxx_context,0x0E) & 0x7F);
		
	PD_DEBUG("ch7036_get_attached_device()- enter- p_ctx->hpd= [0x%x]\n", p_ctx->hpd);

	status_hdmi = ch7036_get_hdmi_hpd(p_ctx,&hpd);
	LHFM_enable_crt_hpd(p_ch7xxx_context);  
	status_crt = LHFM_get_crt_hpd(p_ch7xxx_context);

	PD_DEBUG("ch7036_get_attached_device()- enter- pOutput_Info->channel = [0x%x]\n", pOutput_Info->channel);
	
	if(p_ctx->man_sel_out==1) { //manual selection

		
		if( (status_crt == SS_SUCCESS) || ( (hpd & 0x01)== 1 ) ) { 

			switch (pOutput_Info->channel) {				
				case CHANNEL_LVDS_DVI: 
				case CHANNEL_DVI: 
				case CHANNEL_LVDS_HDMI:
				case CHANNEL_HDMI: 

					 
					if( (status_crt == SS_SUCCESS) && ( (hpd & 0x01)== 0 ) ) { 

						p_ctx->hpd = 0x16; //bit 4=1 to indicate SS_DISPLAY_CHOICE_NOT_ALLOWED,  check edid and parse it												
						status = SS_DISPLAY_CHOICE_NOT_ALLOWED;

					}
					else {

						if (!p_ctx->init_done || ((p_ctx->prev_outchannel & pOutput_Info->channel) & 0x02) != 0x02 ) 
							p_ctx->hpd = 0x60;
						else 
							p_ctx->hpd = ( ((p_ctx->prev_outchannel & pOutput_Info->channel) & 0x02) == 0x02)?0xA0:0x60;							
											
						status = SS_SUCCESS;
					}
										
					break;

				case CHANNEL_LVDS_VGA: 
				case CHANNEL_VGA:
					
					
					if((status_crt != SS_SUCCESS) && ((hpd & 0x01)==1 ) ) { 
						
						if(!p_ctx->init_done) { 
							p_ctx->hpd = 0x06; 
							status = SS_SUCCESS;
							
						}else {					
							p_ctx->hpd = (hpd & 0x80)?0x70:0x30; 
							status = SS_DISPLAY_CHOICE_NOT_ALLOWED;
						}
						
					}
					else { 

						if ( (!p_ctx->init_done) || ((p_ctx->prev_outchannel & pOutput_Info->channel) & 0x04) != 0x04 )//at init, need to read edid, and parse it
							p_ctx->hpd = 0x06;
						else {							
							p_ctx->hpd = ( ((p_ctx->prev_outchannel & pOutput_Info->channel) & 0x04) == 0x04)?0x0A:0x06;
													
						}

						status = SS_SUCCESS;
					}
															
					break;
			} //switch

			if(status == SS_DISPLAY_CHOICE_NOT_ALLOWED)
				PD_DEBUG("ch7036_get_attached_device()- manual selection- display choice is not allowed...\n");

		}
		else {
			
			if (!p_ctx->init_done) { 
				p_ctx->hpd = 0x06; 
				status = SS_SUCCESS;
				
			}else {

				PD_DEBUG("ch7036_get_attached_device()- manual selection- none is connected- not allowed...\n");
				
				status = SS_DISPLAY_CHOICE_NOT_ALLOWED;
				p_ctx->hpd = 0x50;

			}


		}

		PD_DEBUG("ch7036_get_attached_device()- manual selection- exit- p_ctx->hpd= [0x%x]\n", p_ctx->hpd);
		PD_DEBUG("ch7036_get_attached_device()- exit- pOutput_Info->channel = [0x%x]\n", pOutput_Info->channel);

		I2CWrite(p_ch7xxx_context,0x03, 0x03);
		reg = I2CRead(p_ch7xxx_context,0x25);

		PD_DEBUG("ch7036_get_attached_device()- manual selection- exit...HPD_MCU [0x%x]\n",reg);

		return status;
	}

	//auto detection 

	if(status_hdmi == SS_SUCCESS) { 
			
			if( hpd == 0x81 ) { 
				if(p_ctx->hpd & CH7036HPD_HDVI_ATTACHED)  {

					p_ctx->hpd = (p_ctx->hpd  & 0x9F) & ((~CH7036HPD_HDVI_STATUS_CHANGED) | CH7036HPD_HDVI_ATTACHED); 
										
					ch7036_get_hdvi_display_modes_supported(p_ctx);
				
					if ( 
						((pOutput_Info->hdmi_fmt.is_dvi_mode== 0) && (p_hedid->supported_modes[13] == 0) )||
						((pOutput_Info->hdmi_fmt.is_dvi_mode== 1) && (p_hedid->supported_modes[13] > 0))
					) {

						p_ctx->hpd = (p_ctx->hpd  & 0x9F) | (CH7036HPD_HDVI_STATUS_CHANGED | CH7036HPD_HDVI_ATTACHED);
						PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status changed [HDMI<->DVI] since last query and it's HIGH\n");

					}
					else					
						PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status has not changed since last query and it's HIGH\n"); 
				}
				else {


					p_ctx->hpd = (p_ctx->hpd  & 0x9F) | (CH7036HPD_HDVI_STATUS_CHANGED | CH7036HPD_HDVI_ATTACHED);
					PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status changed since last query and it's HIGH\n");
				}
			}
			else if (hpd ==0x80) {	
				if( (p_ctx->hpd & CH7036HPD_HDVI_ATTACHED) ==0 )  {

					p_ctx->hpd = (p_ctx->hpd  & 0x9F) & ((~ CH7036HPD_HDVI_STATUS_CHANGED) & (~CH7036HPD_HDVI_ATTACHED)) ;
					PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status not changed since last query and it's LOW\n");
				}
				else {

					p_ctx->hpd = (p_ctx->hpd  & 0x9F) | (CH7036HPD_HDVI_STATUS_CHANGED & (~CH7036HPD_HDVI_ATTACHED)) ;
					PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status changed since last query and it's LOW\n");
				}
		
			}
			else if (hpd == 0x01) {

				p_ctx->hpd = (p_ctx->hpd  & 0x9F) & ((~CH7036HPD_HDVI_STATUS_CHANGED) | CH7036HPD_HDVI_ATTACHED);
				PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status has not changed since last query and it's HIGH\n");
			}
			else { 

				p_ctx->hpd &= 0x9F; 
				PD_DEBUG("ch7036_get_attached_device()- hdvi HPD status has not changed since last query and it's LOW\n");
			}
			

			
			PD_DEBUG("ch7036: ch7036_get_attached_device()- SUCCESS- hdmi hpd [0x%x]\n", hpd);
			
	}
	else {

		
		if ( (!p_ctx->init_done) &&  (hpd == 0x86) )  {

			p_ctx->hpd |= CH7036HPD_HDVI_ATTACHED;
							
		}

		else {
			p_ctx->hpd &= ~CH7036HPD_HDVI_ATTACHED;
			
		}
		
	}

	if(status_crt == SS_SUCCESS) {  
		
		if( (p_ctx->hpd & CH7036HPD_CRT_ATTACHED )== CH7036HPD_CRT_ATTACHED)  
			p_ctx->hpd = (p_ctx->hpd  & 0xF9) & ((~CH7036HPD_CRT_STATUS_CHANGED)  | CH7036HPD_CRT_ATTACHED);
					
		else 
			p_ctx->hpd = ((p_ctx->hpd  & 0xF9) | ( CH7036HPD_CRT_STATUS_CHANGED | CH7036HPD_CRT_ATTACHED));
									
	} 
	else  {  
		
		if( (p_ctx->hpd & CH7036HPD_CRT_ATTACHED ) == 0 ) { 
			p_ctx->hpd &= 0xF9; 

			if( (p_ctx->hpd & CH7036HPD_HDVI_ATTACHED)== 0  ) 
				p_ctx->hpd |= 0x06; 
										
		}
		else  {
			
			if ( (p_ctx->hpd & CH7036HPD_HDVI_ATTACHED) ==0 )
				p_ctx->hpd &= 0xFB; 
			else
				p_ctx->hpd = (p_ctx->hpd & 0xF9) |  CH7036HPD_CRT_STATUS_CHANGED;		
		}
			
	}

	PD_DEBUG("ch7036_get_attached_device()- auto detection - exit- p_ctx->hpd= [0x%x]\n", p_ctx->hpd);	
	return SS_SUCCESS;
}


void ch7036_reset(ch7036_device_context_t* p_ctx)
{
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;

	ch7036_reset_mcu(p_ch_ctx); 
	ch7036_reset_datapath(p_ch_ctx);

	return;
}

void ch7036_reset_datapath(DEV_CONTEXT* p_ch_ctx)
{

	PD_DEBUG("ch7036: ch7036_reset_datapath()-enter\n");
    I2CWrite(p_ch_ctx,0x03, 0x04); 
	I2CWrite(p_ch_ctx,0x52, 0x2E);
	
	pd_usleep(50); 
	I2CWrite(p_ch_ctx,0x52, 0x2F);
	
	return;
}

void ch7036_reset_mcu(DEV_CONTEXT* p_ch_ctx)
{

	PD_DEBUG("ch7036: ch7036_reset_mcu()-enter\n");

    I2CWrite(p_ch_ctx,0x03, 0x04);
	I2CWrite(p_ch_ctx,0x52, 0x2B);
	
	pd_usleep(50); 	
	I2CWrite(p_ch_ctx,0x52, 0x2F);
	
	
	PD_DEBUG("ch7036: ch7036_reset_mcu()-exit-\n");
	return;
}





void ch7036_set_input_timing_info(ch7036_device_context_t *p_ctx, INPUT_INFO* pInput_Info)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;

	pd_timing_t * p_current_mode = &(p_ctx->native_dtd);
	uint8 audio_id = AUDIO_SPDIF;
	PD_DEBUG("ch7036_intf: ch7036_set_input_timing_info()-\n");

	
	if (p_ctx->init_done) { 

	
		pInput_Info->timing.ht = p_current_mode->htotal+1;
	
		pInput_Info->timing.ha = p_current_mode->width;
		pInput_Info->timing.ho = p_current_mode->hsync_start - p_current_mode->hblank_start;
		pInput_Info->timing.hw = p_current_mode->hsync_end - p_current_mode->hsync_start;
		pInput_Info->timing.vt = p_current_mode->vtotal+1;
	
		pInput_Info->timing.va = p_current_mode->height;
		pInput_Info->timing.vo = p_current_mode->vsync_start - p_current_mode->vblank_start;
		pInput_Info->timing.vw = p_current_mode->vsync_end - p_current_mode->vsync_start;

	
		pInput_Info->rx_clk_khz = p_current_mode->dclk; 

		
		pInput_Info->hs_pol = ((uint8)(((p_current_mode)->mode_info_flags & PD_HSYNC_HIGH) >> 24 ))?1:0;  
		pInput_Info->vs_pol = ((uint8)(((p_current_mode)->mode_info_flags & PD_VSYNC_HIGH) >> 24))?1:0; 

	}


	
	
	
	pInput_Info->pixel_fmt = ((lvds_context_t *)p_ctx->internal_lvds)->panel_depth == 18?PIXEL_FMT_18BIT:3; 

	
	pInput_Info->data_ch_pol = POL_NO_INV;

	
	pInput_Info->data_ch_invert = POL_NO_INV;

	

	

	pInput_Info->de_pol = POL_HIGH;  
	
	
	
	
	
	if(pOutput_Info->channel & CHANNEL_HDMI)
		ch7036_set_audio_type(pInput_Info, audio_id);
	
}

void ch7036_set_output_timing_info(ch7036_device_context_t *p_ctx, OUTPUT_INFO* pOutput_Info)
{


	PD_DEBUG("ch7036: ch7036_set_output_timing_info()\n");
		
	
	PD_DEBUG("ch7036_set_output_timing_info()- output channel from pd context[%u]\n",pOutput_Info->channel);

	
	pOutput_Info->lvds_fmt.channel_swap = LVDS_CHANNEL_SWAP_DEF; 
	pOutput_Info->lvds_fmt.channel_pol = (POL_LOW << 4) | (POL_LOW << 3) | (POL_LOW << 2) | (POL_LOW << 1) | (POL_LOW << 0);
	pOutput_Info->lvds_fmt.pixel_fmt = p_ctx->dither_select;

	
	
	pOutput_Info->hdmi_fmt.channel_swap = 0;
	pOutput_Info->hdmi_fmt.data_pol_invert = POL_NO_INV;
	pOutput_Info->hdmi_fmt.protect_enable = 0;


	
	
	if (pOutput_Info->channel & CHANNEL_HDMI) 
	{
		
		if(!(pOutput_Info->hdmi_fmt.is_dvi_mode))
		{
			PD_DEBUG("ch7036_set_output_timing_info- hdmi mode index is [0x%x]\n",p_ctx->hdmi_mode_index);
			pOutput_Info->hdmi_fmt.format_index = (uint8)hdmi_timing_table[p_ctx->hdmi_mode_index].fmt_index; 
			pOutput_Info->hdmi_fmt.aspect_ratio = (uint8)hdmi_timing_table[p_ctx->hdmi_mode_index].aspect;

			pOutput_Info->timing.ht = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.ht;
			pOutput_Info->timing.ha = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.ha;
			pOutput_Info->timing.ho = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.ho;
			pOutput_Info->timing.hw = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.hw;
			pOutput_Info->timing.vt = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.vt;
			pOutput_Info->timing.va = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.va;
			pOutput_Info->timing.vo = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.vo;
			pOutput_Info->timing.vw = hdmi_timing_table[p_ctx->hdmi_mode_index].timing.vw;
			pOutput_Info->uclk_khz = hdmi_timing_table[p_ctx->hdmi_mode_index].clk_freq;
		}
		else 
		{
			pOutput_Info->hdmi_fmt.format_index = (uint8)dvi_timing_table[p_ctx->dvi_mode_index].fmt_index; 
			pOutput_Info->hdmi_fmt.aspect_ratio = (uint8)dvi_timing_table[p_ctx->dvi_mode_index].aspect;

			pOutput_Info->timing.ht = dvi_timing_table[p_ctx->dvi_mode_index].timing.ht;
			pOutput_Info->timing.ha = dvi_timing_table[p_ctx->dvi_mode_index].timing.ha;
			pOutput_Info->timing.ho = dvi_timing_table[p_ctx->dvi_mode_index].timing.ho;
			pOutput_Info->timing.hw = dvi_timing_table[p_ctx->dvi_mode_index].timing.hw;
			pOutput_Info->timing.vt = dvi_timing_table[p_ctx->dvi_mode_index].timing.vt;
			pOutput_Info->timing.va = dvi_timing_table[p_ctx->dvi_mode_index].timing.va;
			pOutput_Info->timing.vo = dvi_timing_table[p_ctx->dvi_mode_index].timing.vo;
			pOutput_Info->timing.vw = dvi_timing_table[p_ctx->dvi_mode_index].timing.vw;
			pOutput_Info->uclk_khz = dvi_timing_table[p_ctx->dvi_mode_index].clk_freq;

		}



	} else if((pOutput_Info->channel & CHANNEL_VGA) && ((pOutput_Info->channel & CHANNEL_HDMI)==0x00) ) 
	{
		
		
		
		PD_DEBUG("ch7036_set_output_timing_info- crt mode index is [0x%x]\n",p_ctx->crt_mode_index);
		pOutput_Info->timing.ht = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.ht;
		pOutput_Info->timing.ha = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.ha;
		pOutput_Info->timing.ho = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.ho;
		pOutput_Info->timing.hw = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.hw;
		pOutput_Info->timing.vt = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.vt;
		pOutput_Info->timing.va = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.va;
		pOutput_Info->timing.vo = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.vo;
		pOutput_Info->timing.vw = ch7036_crt_timing_table[p_ctx->crt_mode_index].timing.vw;
		pOutput_Info->uclk_khz = ch7036_crt_timing_table[p_ctx->crt_mode_index].clk_freq;

		
	} else
		; 

	
	if(pOutput_Info->channel & CHANNEL_HDMI)
		ch7036_set_hdmi_sync_polarity(pOutput_Info);

	if(pOutput_Info->channel & CHANNEL_HDMI || pOutput_Info->channel & CHANNEL_VGA) {
		
		ch7036_set_rotate (pOutput_Info);
		ch7036_set_hflip (pOutput_Info);
		ch7036_set_vflip (pOutput_Info);
	}
}

void ch7036_set_prefer_timing_info(ch7036_device_context_t *p_ctx, PREFER_INFO* pPrefer_Info)
{
	
	
	

	PD_DEBUG("ch7036_intf: ch7036_set_prefer_timing_info()\n");

	
	if (!p_ctx->init_done) {

	pPrefer_Info->mclk_khz = 166000; 
	pPrefer_Info->uclkod_sel = 1;
	pPrefer_Info->dat16_32b = 0;
	pPrefer_Info->true24 = 0;
	pPrefer_Info->true_com = 0;
	pPrefer_Info->lvds_out_hs_tolerance = HS_TOLERANCE_LEVEL0;
	pPrefer_Info->lvds_out_reset_bit_sel = RST_BIT_VSYNC;
	pPrefer_Info->dither_filter_enable = DITHER_ENABLE;  
		
	pPrefer_Info->hscale_ratio_gate = 130;	
	pPrefer_Info->reset=0; 
	pPrefer_Info->vga_enable=0;

	pPrefer_Info->text_enhancement = DEFAULT_TEXT_ENHANCE; 
	pPrefer_Info->pll_ref_dly = DEF_PLL_REF_DLY;
	pPrefer_Info->pll_ref_fbdly = DEF_PLL_REF_FBDLY;
	pPrefer_Info->lvds_txdrv_ctrl = DEF_LVDS_TXDRV_CTRL;

	pPrefer_Info->eye_bgtrim=0;
	pPrefer_Info->eye_dacg=0;
	pPrefer_Info->eye_dri_demp=0;
	pPrefer_Info->eye_dri_pll_cp=0;
	pPrefer_Info->eye_dri_damp=0;
	pPrefer_Info->eye_dri_pll_rlf=0;
	pPrefer_Info->eye_rdac=0;

	}

	pPrefer_Info->scale_line_adjust = 0;


	
}


ch7036_status_t ch7036_parse_standard_edid(ch7036_device_context_t* p_ctx, uint32 channel)
{
	uint8 i, index = 0;
	
	ch7036_edid_blk_t* p_edid = (ch7036_edid_blk_t *)p_ctx->cedid;
	unsigned char* p_edidblk = p_edid->edidblk;

	
	OUT_FMT* p_table = ch7036_crt_timing_table;
	TIMING* p_timing;

	unsigned char j=0;




	
	established_timings_t *p_etiming_I = p_edid->etiming_I;
	established_timings_t *p_etiming_II = p_edid->etiming_II;


	
	standard_timings_t	*stiming = p_edid->stiming; 
	ch7036_attr_table_index_t* p_modes =  p_edid->modes;

	unsigned long idx = 2; 


	PD_DEBUG("ch7036_parse_stardard_edid() channel [0x%x] - enter...\n", channel);


	if(channel == CHANNEL_LVDS_HDMI) {

		p_edid = (ch7036_edid_blk_t *)p_ctx->hedid;
		p_edidblk = p_edid->edidblk;
		p_etiming_I = p_edid->etiming_I;
		p_etiming_II = p_edid->etiming_II;

		stiming = p_edid->stiming; 
		p_modes =  p_edid->modes;

		
		if (p_edid->ebn > 1) { 
			idx = 1;
			p_table = hdmi_timing_table;
		}
		else { 
			idx = 0;
			p_table = dvi_timing_table;
			channel = 7; 
		}


	}

	
	while (index < MAX_ATTR_LIST_SIZE)
		p_modes[index++] = FALSE; 


	if (p_edidblk[EDID_EXTENSION_FLAG] == 0x00 ) {
			

		ch7036_parse_standard_timing(p_edid,0);

		
		ch7036_parse_established_timing(p_ctx, p_edid);
		

		
		ch7036_parse_detailed_descriptor_blocks(p_ctx, p_edid);
				

		
		for(i=0; i<8;i++) { 


			index=0;
			while (index < MAX_ATTR_LIST_SIZE ) {
				if(p_modes[index]== TRUE) {
					index++; 
					continue;
				}
				p_timing = &(p_table[index].timing);

				if ( (p_edid->dtblk[j]).data_tag & 0x00FFFFFF ) { 
					OUT_FMT* p_dtd = &((p_edid->dtblk[j]).dtiming);
					
					if( (p_dtd->timing.ha == p_timing->ha) && (p_dtd->timing.va == p_timing->va) && (p_dtd->timing.hz >= p_timing->hz) ) {
						p_modes[index] = TRUE;
						j= j> 3?3:j+1;

						PD_DEBUG("ch7036_parse_standard_edid()- detailed timing mode supported- index [%d] name [%s]...\n",index,ch7036_get_mode_name(channel,index) );

					}

				}
							
				
				if( (stiming[i].ha == p_timing->ha) && (stiming[i].va == p_timing->va) && (stiming[i].refresh_rate >= p_timing->hz) ) {
					p_modes[index] = TRUE;
					PD_DEBUG("ch7036_parse_standard_edid()- std_timing mode supported- index [%d] name [%s]...\n",index,ch7036_get_mode_name(channel,index) );
				}
				
				index++;

			}

		}

		
		for(i=0;i<4;i++) {
			if ( (p_edid->dtblk[i]).data_tag == 0xFA000000) {
				; 
				continue;
			}
		}

		

		for(i=0; i<8;i++) { 

			
			
			if( (p_etiming_I[i].is_supported == TRUE) && (p_etiming_I[i].index[idx] != OUT_CRT_END) ) {
				p_modes[p_etiming_I[i].index[idx]] = TRUE; 
				PD_DEBUG("ch7036_parse_standard_edid()- et1 mode supported- index [%d] name [%s]...\n",p_etiming_I[i].index[idx],p_etiming_I[i].mode_name);
				continue;

			}

			
			if( (i==7) && (channel ==7) && (p_etiming_I[i].is_supported == TRUE) ) {
				p_modes[p_etiming_I[i].index[idx]] = TRUE; 
				PD_DEBUG("ch7036_parse_standard_edid()- et1 mode supported- index [%d] name [%s]...\n",p_etiming_I[i].index[idx],p_etiming_I[i].mode_name);
			}

			
		}

		for(i=0; i<8;i++) { 
		
			if( (p_etiming_II[i].is_supported == TRUE) && (p_etiming_II[i].index[idx] != OUT_CRT_END) ) {
				p_modes[p_etiming_II[i].index[idx]] = TRUE; 
				PD_DEBUG("ch7036_parse_standard_edid()- et2 mode supported- index [%d] name [%s]...\n",p_etiming_II[i].index[idx],p_etiming_II[i].mode_name);
				continue;

			}

		}

		

	}
	else { 
		PD_DEBUG("ch7036_parse_standard_edid()- vga/dvi has more than one 128 byte block\n");
	}


	PD_DEBUG("ch7036_parse_stardard_edid()-channel [0x%x] - exit...\n", channel);

	return SS_SUCCESS;

}

void ch7036_parse_detailed_descriptor_blocks(ch7036_device_context_t* p_ctx, ch7036_edid_blk_t* p_edid)
{
	unsigned long *monitor_descriptor;
	unsigned char* p_ebuf, *p_st;
	unsigned char* p_edidblk = p_edid->edidblk;
	unsigned char i;
	OUT_FMT* p_dt;
	

	p_ebuf = &(p_edidblk[EDID_DETAILED_TIMING_DESCRIPTION_1]);

	PD_DEBUG("parse_detailed_descriptor_blocks()- enter...\n");

	for(i=0;i<4;i++) {

		monitor_descriptor = (unsigned long *) p_ebuf;

		if((*monitor_descriptor) & 0x00FFFFFF ) { 
			
			
			p_dt = &(p_edid->dtblk[i].dtiming);				
			ch7036_parse_detailed_timing(p_dt, p_ebuf);
			
		}
			
		else { 

			p_st = p_ebuf;
			p_st +=5;

			switch (*monitor_descriptor) {
				case 0xFA000000: 
					ch7036_parse_standard_timing(p_edid,p_st); 
					break;

				case 0xFD000000: 
					(p_edid->rtiming).vrate_min = *p_st; 
					(p_edid->rtiming).vrate_max = *(p_st+1);
					(p_edid->rtiming).hrate_min = *(p_st+2); 
					(p_edid->rtiming).hrate_max = *(p_st+3); 
					(p_edid->rtiming).pclk_max = (unsigned long)(*(p_st+4))*10000L;	
					
					
					break;

				case 0xFC000000: 
				case 0xFF000000: 
				default: 
					break;
			}
		}

		(p_edid->dtblk[i]).data_tag = *monitor_descriptor;

		p_ebuf += 18;
	}
	
	return;
}

void ch7036_parse_detailed_timing(OUT_FMT *p_dt, unsigned char* p_ebuf)
{

	unsigned short blanking;

	PD_DEBUG("ch7036_parse_detailed_descriptor_timing()- enter...\n");


	p_dt->clk_freq = ((uint32)(p_ebuf[1]<<8) | p_ebuf[0]) * 10; 

	p_dt->timing.ha = ((uint16)(p_ebuf[4] & 0xF0) << 4) | p_ebuf[2];

	PD_DEBUG("ch7036_parse_detailed_timing() byte 3 [%x] byte 5 [%x]\n",p_ebuf[2],p_ebuf[4]);

	blanking = ((uint16)(p_ebuf[4] & 0x0F) << 8) | p_ebuf[3]; 
	p_dt->timing.ht = p_dt->timing.ha + blanking;

	p_dt->timing.va = ((uint16)(p_ebuf[7] & 0xF0) << 4) | p_ebuf[5];

	PD_DEBUG("ch7036_parse_detailed_timing() byte 6 [%x] byte 8 [%x]\n",p_ebuf[5],p_ebuf[7]);

	blanking = ((uint16)(p_ebuf[7] & 0x0F) << 8) | p_ebuf[6]; 
	p_dt->timing.vt = p_dt->timing.va + blanking;

	PD_DEBUG("ch7036_parse_detailed_timing()- pclk [%d] Khz ha [%d] va [%d] ht [%d] vt [%d]\n",p_dt->clk_freq,p_dt->timing.ha,p_dt->timing.va,p_dt->timing.ht,p_dt->timing.vt);

	p_dt->timing.hz = 	(((p_dt->clk_freq / p_dt->timing.ht) + 1 ) * 1000) / p_dt->timing.vt; 

	p_dt->timing.stype = (p_ebuf[17] & 0x80)?0:1; 

	PD_DEBUG("ch7036_parse_detailed_timing()- refresh [%d] scantype [%d]\n",
		p_dt->timing.hz,p_dt->timing.stype);

	return;	
}



void ch7036_parse_standard_timing(ch7036_edid_blk_t* p_edid, unsigned char* p_addtional_st)
{
	standard_timings_t *stiming = p_edid->stiming;
	unsigned char i, max=8;

	unsigned char* p_edidblk = &(p_edid->edidblk[EDID_STANDARD_TIMINGS]);

	
	PD_DEBUG("ch7036_parse_standard_timing()- enter\n");
	
	if(p_addtional_st) {
		p_edidblk = p_addtional_st;
		stiming = p_edid->stiming_x;
		max = 6;
	}

	for(i = 0; i < max; stiming++,i++) {
			
	
		if( (*p_edidblk) == 0x01 &&  *(p_edidblk+1) == 0x01) 
			continue;
	
		stiming->ha = ((*p_edidblk) + 31) << 3 ;
		 
		stiming->refresh_rate = (*(p_edidblk + 1) & 0x3F) + 60;

		switch(*(p_edidblk +1) >> 6) {
	
		   case 0: 
			   stiming->va =  (stiming->ha *10) >> 4;
			   break;
		   case 1: 
			   stiming->va =  (stiming->ha *3) >> 2;
			   break;
		   case 2: 
			   stiming->va =  (stiming->ha << 2) / 5;			       
			   break;
		   case 3: 
			   stiming->va =  (stiming->ha* 9) >>4;			       
			   break;
		   default:
			   break;
		} 

		p_edidblk +=2;

		PD_DEBUG("ch7036_parse_standard_timing()- ha [%d] va [%d] refresh [%d]\n",stiming->ha,stiming->va,stiming->refresh_rate);

	} 
	
	return;
}




void ch7036_parse_established_timing(ch7036_device_context_t* p_ctx, ch7036_edid_blk_t* p_edid) 
{

	unsigned char* p_edidblk = p_edid->edidblk;

	

	established_timings_t *p_etiming_I = p_edid->etiming_I;
	established_timings_t *p_etiming_II = p_edid->etiming_II;
	established_timings_t *p_etiming_man = p_edid->etiming_man;

	unsigned char i=0;
	unsigned char et1, et2;

	PD_DEBUG("ch7036_parse_established_timing()- enter...\n");

	et1 = p_edidblk[EDID_ESTABLISHED_TIMINGS_1];
	et2 = p_edidblk[EDID_ESTABLISHED_TIMINGS_2];



	for(i=0; i<8;i++) {
	  p_etiming_I[i].is_supported = FALSE;
	  p_etiming_II[i].is_supported = FALSE;
	}



	p_etiming_man->is_supported = FALSE;
 

	for(i=0;i<8;i++) {


		
		if(et1 & 0x01) {   
			p_etiming_I[i].is_supported = TRUE;
	
		}
		
		
		if(et2 & 0x01) {   
			p_etiming_II[i].is_supported = TRUE;
	
		}

		et1 >>=  1;
		et2 >>=  1;

	}


	
	if (p_edidblk[EDID_MANUFACTURERS_RESERVED_TIMINGS] & 0x80) {

		p_etiming_man->is_supported = TRUE;
	 
	}


	return;

}


ch7036_status_t ch7036_parse_cea_edid(ch7036_device_context_t* p_ctx)
{
	uint8 tag, blk_size =0; 
	uint8 index = 0;
	
	uint16 count;

	uint8*  p_buff;


	ch7036_edid_blk_t* p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;

	unsigned char* p_edidblk = p_hedid->edidblk;
	ch7036_attr_table_index_t* p_modes =  p_hedid->modes;

	
	PD_DEBUG("ch7036_parse_cea_edid()- enter...\n");

	while (index < MAX_ATTR_LIST_SIZE )
		p_modes[index++] = FALSE; 
	
		
	if(p_edidblk[EDID_CEA_DETAILED_TIMING_DATA_OFFSET] == 0 ||  p_edidblk[EDID_CEA_DETAILED_TIMING_DATA_OFFSET] > 124) {
		PD_DEBUG("ch7036_parse_cea_edid()- invalid data block size [%d]\n", p_edidblk[EDID_CEA_DETAILED_TIMING_DATA_OFFSET]);
		return SS_UNSUCCESSFUL;
	}

	

	PD_DEBUG("ch7036_parse_cea_edid()- CEA revision [0x%x]...\n",p_edidblk[EDID_CEA_REVISION]);
	
	if(p_edidblk[EDID_CEA_TAG] == 0x02 && p_edidblk[EDID_CEA_REVISION] == 0x03)
	{
		
		p_buff = &(p_edidblk[EDID_CEA_DATA_BLOCK]);

		

		count=4 ; 
		for(; count < p_edidblk[EDID_CEA_DETAILED_TIMING_DATA_OFFSET]; p_buff += blk_size) {

			blk_size = (*p_buff) & 0x1F; 
			tag = (*p_buff) & 0xE0; 

			PD_DEBUG("ch7036_parse_cea_edid()- data type [0x%x] block_size [%d]\n", tag, blk_size);

			p_buff++;
			switch (tag) { 
			
				case 0x20: 
				
					break;

				case 0x40: 
					ch7036_parse_cea_video_data_block(blk_size, p_buff, p_hedid);
					break;

				case 0x60: 
					break;

				case 0x80: 
					break;

				default:
					break;

			}

			count += (blk_size + 1); 
					

		} 
	
	} 
	

	PD_DEBUG("ch7036_parse_cea_edid()- exit...\n");

	return SS_SUCCESS;

	
}


void ch7036_parse_cea_video_data_block(uint8 blk_size, uint8* p_buff, ch7036_edid_blk_t* p_edid)
{
	uint8 i, index;

	ch7036_attr_table_index_t* p_modes =  p_edid->modes;

	for(i=1; i <= blk_size; i++, p_buff++ ) {
		
			index=0;

			PD_DEBUG("ch7036_parse_cea_video_data_block()- HDMI display video code [0x%x]\n",(*p_buff) & 0x7F);
			
			while (index < OUT_HDMI_END) { 
			
				
				if( p_modes[index] == TRUE  ) {
					index++;
					continue;
				}
				

				if( ((*p_buff) & 0x7F) == hdmi_timing_table[index].fmt_index  ) {				
					
					p_modes[index] = TRUE;
					PD_DEBUG("ch7036_parse_cea_video_data_block()- mode supported: global table index [%d] name [%s]...\n",index, ch7036_get_mode_name(CHANNEL_LVDS_HDMI,index) );
					
					if( hdmi_timing_table[index-1].fmt_index == hdmi_timing_table[index].fmt_index) {						 
		
						break; 
					}

				}  
				
				index++;
								   
			} 
	} 

	return;
}


ch7036_status_t ch7036_parse_edid(ch7036_device_context_t* p_ctx)
{


	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;

	ch7036_edid_blk_t* p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;
	ch7036_edid_blk_t* p_cedid = (ch7036_edid_blk_t *)p_ctx->cedid;

	unsigned char* p_edidblk=0; 
	ch7036_attr_table_index_t* p_modes = 0;
	pd_attr_t  *p_attr = NULL ;
	int RESET =1;

	PD_DEBUG("ch7036_parse_edid()- enter...\n");

	
	//hpd checking schemes
	if(p_ctx->hpd == 0) 
		return SS_SUCCESS;


	if ( 
		( (pOutput_Info->channel & 0x02) &&  (p_ctx->hpd & 0x80) )||
		( (pOutput_Info->channel & 0x04) &&  (p_ctx->hpd & 0x08) )
		)
		//parsing is already done
		return SS_SUCCESS;

	
	switch (pOutput_Info->channel) {
		case CHANNEL_LVDS_HDMI:
		case CHANNEL_HDMI:
			if(p_hedid->is_edid) { 
				p_edidblk = p_hedid->edidblk;	

				p_hedid->is_preferred_timing = (p_edidblk[EDID_FEATURE_SUPPORT] >> 1) & 0x1;

				
				if(pOutput_Info->hdmi_fmt.is_dvi_mode == 0) {
					p_attr = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs, PD_ATTR_ID_HDMI_OUT_MODE, 0);
					ch7036_parse_cea_edid(p_ctx);
				}
				else {
					p_attr = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs, PD_ATTR_ID_DVI_OUT_MODE, 0);
					ch7036_parse_standard_edid(p_ctx, CHANNEL_LVDS_HDMI);
				}

								
				
				p_ctx->hpd |= 0x80; //don't parse again next time
							
			}
			else { 

				PD_DEBUG("ch7036_parse_edid()- hdmi edid read failed or never done...use default...\n");
				ch7036_set_edid_display_supported_attr( (void *)p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,p_ctx->downscaled,p_hedid->modes,RESET);
				p_ctx->hpd &= 0x7F; //allow to enter parsing block again

			}

			p_modes = p_hedid->modes;
			
			break;

		case CHANNEL_LVDS_VGA:
		case CHANNEL_VGA:
			if(p_cedid->is_edid) { 
				p_edidblk = p_cedid->edidblk;

				p_cedid->is_preferred_timing = (p_edidblk[EDID_FEATURE_SUPPORT] >> 1) & 0x1;
				p_attr = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs, PD_ATTR_ID_CRT_OUT_MODE, 0);
								
				ch7036_parse_standard_edid(p_ctx, CHANNEL_LVDS_VGA);
				p_ctx->hpd |= 0x08;
			}
			else {
				PD_DEBUG("ch7036_parse_edid()- vga edid read failed or never done...use default...\n");
				ch7036_set_edid_display_supported_attr( (void *)p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,p_ctx->downscaled,p_cedid->modes,RESET);
				p_ctx->hpd &= 0x7F; //allow to enter parsing block again
			}
			p_modes = p_cedid->modes;
			
			break;
		default:
			//there is no separate DVI display channel, or LVDS edid reading at this time
			//DVI display option should already be mapped to HDMI channel
			p_edidblk = 0;

	}

	if(p_edidblk) {

		ch7036_set_edid_display_supported_attr( (void *)p_attr,0,p_ctx->downscaled,p_modes,0);

	}

	PD_DEBUG("ch7036_parse_edid()- exit...\n");

	return SS_SUCCESS;
}


void ch7036_alter_display_table(int all, void *p_table,unsigned char* p_modes, void* val,unsigned long* p_invis,unsigned char is_invis,unsigned char is_6x4)
{
	pd_attr_t  *p_attr;
	unsigned long i,j;
	

	PD_DEBUG("ch7036_alter_display_table()-enter...\n");

	if(all) { 
		unsigned long* num_attrs = (unsigned long *)val;
		for(i=0;i<3;i++) { //all 3 tables: hdmi, dvi, vga
			
			p_attr = pd_get_attr((pd_attr_t *)p_table, *num_attrs, PD_ATTR_ID_HDMI_OUT_MODE+i, 0);
			if(is_6x4) 
				p_attr++;  		
			else 
				p_attr= p_attr + ch7036_invis_6x4_modes_table_size[i] + 1; //1 is to skip header

					
			for(j=0; j < *p_invis;j++) { 
				if(is_invis)
					p_attr->flags  |= PD_ATTR_FLAG_USER_INVISIBLE;  
				else 
					p_attr->flags  &= ~PD_ATTR_FLAG_USER_INVISIBLE; 


				p_attr++;
			}
			p_invis++; //now, grab dvi, then vga table size
			

		}

	}
	else { 
		unsigned long id= 0,k;
		p_attr = (pd_attr_t *)p_table;
		id = p_attr->id;

		if(is_6x4) {
			p_attr++;  
			j=0;
		}
		else {
			p_attr= p_attr + ch7036_invis_6x4_modes_table_size[id - PD_ATTR_ID_HDMI_OUT_MODE] + 1; //point to the first 7x4 entry			
			j= ch7036_invis_6x4_modes_table_size[id - PD_ATTR_ID_HDMI_OUT_MODE];
		}
		p_invis = p_invis + (id - PD_ATTR_ID_HDMI_OUT_MODE);
		k= *p_invis + j;

		for(; j < k;j++) { 
			if(is_invis) {
				p_attr->flags  |= PD_ATTR_FLAG_USER_INVISIBLE;  
				p_modes[j]=FALSE;
			} 
	
			p_attr++;
		}
			
	}
}


ch7036_status_t ch7036_set_edid_display_supported_attr(void *p_table, unsigned long num_attrs, unsigned char* p_downscaled, unsigned char* p_modes, int is_reset)
{

	pd_list_attr_t *p_hdr = (pd_list_attr_t *)p_table;
	pd_attr_t *p_entry = (pd_attr_t *)p_table; 
	
	uint8 i=0;

#ifdef T_SHOW_EDID_DISPLAY_ATTR
	pd_list_entry_attr_t *list_entry;
#endif
	
	PD_DEBUG("ch7036_set_edid_display_supported_attr()-enter... is_reset status = [%d]\n", is_reset);

	if(is_reset) {
		while (i < MAX_ATTR_LIST_SIZE ) {
			p_modes[i++] = TRUE; 
		}

		
		
		//special handling for 1080i/p @ 59Hz
		p_entry = pd_get_attr((pd_attr_t *)p_table, num_attrs, PD_ATTR_ID_HDMI_OUT_MODE, 0);
		p_entry++;
		p_entry = p_entry + OUT_HDMI_1920x1080I_59;
		p_entry->flags  |= PD_ATTR_FLAG_USER_INVISIBLE;  
		p_entry += 2;
		p_entry->flags  |= PD_ATTR_FLAG_USER_INVISIBLE;  

	}
	else { 

		PD_DEBUG("ch7036_set_edid_display_supported_attr()-start to build edid display mode list...\n");
	
		p_entry++; 

		for (i = 0; i < p_hdr->num_entries ; ++i,++p_entry) { 
			if( 
				(p_modes[i] == FALSE) ||
				//1080 i/p @ 59 Hz
				((p_hdr->id == PD_ATTR_ID_HDMI_OUT_MODE) && (i == OUT_HDMI_1920x1080I_59 || i== OUT_HDMI_1920x1080P_59) ) ){

				p_entry->flags  |= PD_ATTR_FLAG_USER_INVISIBLE;  
				continue;
			}
		

			p_entry->flags  &= ~PD_ATTR_FLAG_USER_INVISIBLE; 

		}


	}
	
	//special handling for 8x6,7x4,6x4- 
	ch7036_alter_display_table(is_reset,p_table,p_modes,(void *)&num_attrs,ch7036_invis_6x4_modes_table_size,*p_downscaled,1);			
	p_downscaled++;
	ch7036_alter_display_table(is_reset,p_table,p_modes,(void *)&num_attrs,ch7036_invis_8x6_7x4_table_size,*p_downscaled,0);

#ifdef T_SHOW_EDID_DISPLAY_ATTR

	list_entry = (pd_list_entry_attr_t *)(p_table);

	for (i = 0,++list_entry; i < p_hdr->num_entries; ++i, ++list_entry) 
		PD_DEBUG("ch7036 : ch7036_set_edid_display_supported_attr : \n"
				  "list entry[%hhu]=%s, id=%lu, "
					  "value=%lu, flags=0x%x \n",
					  i, list_entry->name, list_entry->id,
					  list_entry->value, (unsigned char)list_entry->flags);
#endif


	return SS_SUCCESS;
}


uint8 * ch7036_get_mode_name(uint32 channel, uint8 index)
{
	uint8* str = "Name String Is Not Yet Converted";

	switch (channel) {

		case CHANNEL_LVDS_HDMI:
			switch (index) {
				case OUT_HDMI_640x480P_59:
					return ("OUT_HDMI_640x480P_59");					
				case OUT_HDMI_640x480P_60:
					return ("OUT_HDMI_640x480P_60");
				case OUT_HDMI_720x480P_59:
					return ("OUT_HDMI_720x480P_59");
				case OUT_HDMI_720x480P_60:
					return ("OUT_HDMI_720x480P_60");
				case OUT_HDMI_1280x720P_59:
					return ("OUT_HDMI_1280x720P_59");					
				case OUT_HDMI_1280x720P_60:
					return ("OUT_HDMI_1280x720P_60");
				case OUT_HDMI_1920x1080I_59:
					return ("OUT_HDMI_1920x1080I_59");					
				case OUT_HDMI_1920x1080I_60:
					return ("OUT_HDMI_1920x1080I_60");
				case OUT_HDMI_1920x1080P_59:
					return ("OUT_HDMI_1920x1080P_59");
				case OUT_HDMI_1920x1080P_60:
					return ("OUT_HDMI_1920x1080P_60");
				

			}
			break;

		case CHANNEL_LVDS_VGA:

			switch (index) {

				case OUT_CRT_640x400_85:
					return ("OUT_CRT_640x400_85");

				case OUT_CRT_640x480_60:
					return ("OUT_CRT_640x480_60");
				case OUT_CRT_640x480_72:
					return ("OUT_CRT_640x480_72");
				case OUT_CRT_640x480_75:
					return ("OUT_CRT_640x480_75");
				case OUT_CRT_640x480_85:
					return ("OUT_CRT_640x480_85");

				case OUT_CRT_720x400_85:
					return ("OUT_CRT_720x400_85");

				case OUT_CRT_800x600_56:
					return ("OUT_CRT_800x600_56");
				case OUT_CRT_800x600_60:
					return ("OUT_CRT_800x600_60");
				case OUT_CRT_800x600_72:
					return ("OUT_CRT_800x600_72");
				case OUT_CRT_800x600_75:
					return ("OUT_CRT_800x600_75");
					
				case OUT_CRT_800x600_85:
					return ("OUT_CRT_800x600_85");

				case OUT_CRT_1024x768_60:
					return ("OUT_CRT_1024x768_60");
				case OUT_CRT_1024x768_70:
					return ("OUT_CRT_1024x768_70");
				case OUT_CRT_1024x768_75:
					return ("OUT_CRT_1024x768_75");
				case OUT_CRT_1024x768_85:
					return ("OUT_CRT_1024x768_85");

				case OUT_CRT_1152x864_75:
					return ("OUT_CRT_1152x864_75");

				case OUT_CRT_1280x768_60:
					return ("OUT_CRT_1280x768_60");
				case OUT_CRT_1280x768_75:
					return ("OUT_CRT_1280x768_75");
				case OUT_CRT_1280x768_85:
					return ("OUT_CRT_1280x768_85");

				case OUT_CRT_1280x960_60:
					return ("OUT_CRT_1280x960_60");
				case OUT_CRT_1280x960_85:
					return ("OUT_CRT_1280x960_85");	
					
				case OUT_CRT_1280x1024_60:
					return ("OUT_CRT_1280x1024_60");
				case OUT_CRT_1280x1024_75:
					return ("OUT_CRT_1280x1024_75");
				case OUT_CRT_1280x1024_85:
					return ("OUT_CRT_1280x1024_85");

				case OUT_CRT_1360x768_60:
					return ("OUT_CRT_1360x768_60");

				case OUT_CRT_1400x1050_60:
					return ("OUT_CRT_1400x1050_60");
				case OUT_CRT_1400x1050_75:
					return ("OUT_CRT_1400x1050_75");

				case OUT_CRT_1440x900_60:
					return ("OUT_CRT_1440x900_60");

				case OUT_CRT_1440x1050_60:
					return ("OUT_CRT_1440x1050_60");

				case OUT_CRT_1600x900_60:
					return ("OUT_CRT_1600x900_60");

				case OUT_CRT_1600x1200_60:
					return ("OUT_CRT_1600x1200_60");	
					
				case OUT_CRT_1920x1080_60:
					return ("OUT_CRT_1920x1080_60");

			}
			break;

		default: 
			switch (index) {
				case OUT_DVI_640x480_60:
					return ("OUT_DVI_640x480_60"); 
				case OUT_DVI_640x480_72:
					return ("OUT_DVI_640x480_72"); 

				case OUT_DVI_720x400_70:
					return ("OUT_DVI_720x400_70"); 

				case OUT_DVI_800x600_56:
					return ("OUT_DVI_800x600_56");
				case OUT_DVI_800x600_60:
					return ("OUT_DVI_800x600_60"); 
				case OUT_DVI_800x600_72:
					return ("OUT_DVI_800x600_72");
				case OUT_DVI_800x600_75:
					return ("OUT_DVI_800x600_75");

				case OUT_DVI_1024x768_60:
					return ("OUT_DVI_1024x768_60");
				case OUT_DVI_1024x768_70:
					return ("OUT_DVI_1024x768_70");
				case OUT_DVI_1024x768_75:
					return ("OUT_DVI_1024x768_75");

				case OUT_DVI_1152x864_60:
					return ("OUT_CRT_1152x864_60");

				case OUT_DVI_1280x720_60:
					return ("OUT_DVI_1280x720_60");

				case OUT_DVI_1280x800_60:
					return ("OUT_DVI_1280x800_60");

				case OUT_DVI_1280x960_60:
					return ("OUT_DVI_1280x960_60");

				case OUT_DVI_1280x1024_60:
					return ("OUT_DVI_1280x1024_60");
				case OUT_DVI_1280x1024_75:
					return ("OUT_DVI_1280x1024_75");

				case OUT_DVI_1360x768_60:
					return ("OUT_DVI_1360x768_60");

				case OUT_DVI_1366x768_60:
					return ("OUT_DVI_1366x768_60");

				case OUT_DVI_1400x1050_60:
					return ("OUT_DVI_1400x1050_60");
				case OUT_DVI_1400x1050_75:
					return ("OUT_DVI_1400x1050_75");

				case OUT_DVI_1440x900_60:
					return ("OUT_DVI_1440x900_60");

				case OUT_DVI_1440x1050_60:
					return ("OUT_DVI_1440x1050_60");

				case OUT_DVI_1600x900_60:
					return ("OUT_DVI_1600x900_60");

				case OUT_DVI_1600x1200_60:
					return ("OUT_DVI_1600x1200_60");
					
				case OUT_DVI_1680x1050_60:
					return ("OUT_DVI_1680x1050_60");
					
				case OUT_DVI_1920x1080_60:
					return ("OUT_DVI_1920x1080_60");
			}
			break;
			

	} 


	return str;

}



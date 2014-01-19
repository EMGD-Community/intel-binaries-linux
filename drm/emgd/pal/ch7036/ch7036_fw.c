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
* @file  ch7036_fw.c
* @version 1.2.5
*-----------------------------------------------------------------------------
*/



#include "ch7036_intf.h"

#include "edid7036.car"


#define EDID_RETRY_MAX_TIMES 5
#define HPD_LOOP_MAX 10


#define SLEEP_TIME 200 


#define lhfm_size  sizeof(lhfm_array)

#define LHFM_TIMEOUT	10  

static unsigned char edid_header[8]={0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00};

static unsigned char es_map[16] = {   
	0x26,0x27,0x42,0x43,0x44,0x45,0x46,0x47,
	0x6A,0x51,0x52,0x53,0x57,0x58,0x59,0x5A	
};





established_timings_t et_I[8] = {
	{0,"800x600_60", {OUT_DVI_800x600_60,OUT_HDMI_END, OUT_CRT_800x600_60} },
	{0,"800x600_56", {OUT_DVI_800x600_56,OUT_HDMI_END, OUT_CRT_800x600_56} },
	{0,"640x480_75", {OUT_DVI_640x480_75, OUT_HDMI_END, OUT_CRT_640x480_75} },
	{0,"640x480_72", {OUT_DVI_640x480_72, OUT_HDMI_END, OUT_CRT_640x480_72} },
	{0,"640x480_67", {OUT_DVI_END, OUT_HDMI_END, OUT_CRT_END } },
	{0,"640x480_60", {OUT_DVI_640x480_60, OUT_HDMI_640x480P_60, OUT_CRT_640x480_60} },
	{0,"720x400_88", {OUT_DVI_END, OUT_HDMI_END, OUT_CRT_END } },
	{0,"720x400_70", {OUT_DVI_720x400_70, OUT_HDMI_END, OUT_CRT_END} },
};

established_timings_t et_II[8] ={
	{0, "1280x1024_75", {OUT_DVI_1280x1024_75, OUT_HDMI_END, OUT_CRT_1280x1024_75} },
	{0, "1024x768_75", {OUT_DVI_1024x768_75, OUT_HDMI_END, OUT_CRT_1024x768_75} },
	{0, "1024x768_70", {OUT_DVI_1024x768_70, OUT_HDMI_END, OUT_CRT_1024x768_70} },
	{0, "1024x768_60", {OUT_DVI_1024x768_60, OUT_HDMI_END, OUT_CRT_1024x768_60} },
	{0, "1024x768_87", {OUT_DVI_END, OUT_HDMI_END, OUT_CRT_END} },
	{0, "832x624_75", {OUT_DVI_END, OUT_HDMI_END, OUT_CRT_END} },
	{0, "800x600_75", {OUT_DVI_800x600_75, OUT_HDMI_END, OUT_CRT_800x600_75} },
	{0, "800x600_72", {OUT_DVI_800x600_72, OUT_HDMI_END, OUT_CRT_800x600_72} }
};

established_timings_t et_man = {
	0, "1152x870_75", {OUT_DVI_END, OUT_HDMI_END, OUT_CRT_END} 
};


int LHFM_get_version(DEV_CONTEXT* p_ch7xxx_context,struct _FW7036_CFG* cfg)
{

	unsigned char reg;
	unsigned wj;

		 I2CWrite(p_ch7xxx_context,0x03, 0x00); 
		 reg = I2CRead(p_ch7xxx_context,0x4F);
		 

		 if (0==(LHFM_REQUEST & reg)) { 
			 I2CWrite(p_ch7xxx_context,0x4F, (LHFM_REQUEST+LHFM_GET_VERSION)); 
			 wj = 0;
			 while (wj++< LHFM_TIMEOUT) {
			 
			 				
				 I2CWrite(p_ch7xxx_context,0x03, 0x00); 
				 pd_usleep(SLEEP_TIME );
				 PD_DEBUG("LHFM_get_version [%x]\r\n", wj);

				 reg = I2CRead(p_ch7xxx_context,0x4F);
				 if(reg == LHFM_GET_VERSION) {
					 reg = I2CRead(p_ch7xxx_context,0x50);			
					 if (!(LHFM_RET_ERROR & reg)) {
						    I2CWrite(p_ch7xxx_context,0x03, 0x01); 
						    cfg->size = I2CRead(p_ch7xxx_context,es_map[0]);	 
							cfg->ver_major = I2CRead(p_ch7xxx_context,es_map[1]);
							cfg->ver_minor = I2CRead(p_ch7xxx_context,es_map[2]);
							cfg->did = I2CRead(p_ch7xxx_context,es_map[3]);
							cfg->rid = I2CRead(p_ch7xxx_context,es_map[4]);
							cfg->capbility = I2CRead(p_ch7xxx_context,es_map[5]);
							cfg->reserved = I2CRead(p_ch7xxx_context,es_map[6]);
						    I2CWrite(p_ch7xxx_context,0x03, 0x00); 
							return 0;
					 } else return -2;

				 } 
				 				 				
			 }  

		 } 

	return -1; 
}


void LHFM_enable_crt_hpd(DEV_CONTEXT* p_ch7xxx_context)
{
	uint8 reg;
	
	PD_DEBUG("ch7036: LHFM_enable_crt_hpd-enter\n");

	I2CWrite(p_ch7xxx_context,0x03, 0x04); 
	reg = I2CRead(p_ch7xxx_context,0x57);
	I2CWrite(p_ch7xxx_context,0x57, reg | 0x02); 
	   
	pd_usleep(SLEEP_TIME);

	I2CWrite(p_ch7xxx_context,0x03, 0x00); 
	I2CWrite(p_ch7xxx_context,0x08, I2CRead(p_ch7xxx_context,0x08) & 0xF1);
	
	pd_usleep(SLEEP_TIME);
}


ch7036_status_t LHFM_get_crt_hpd(DEV_CONTEXT* p_ch7xxx_context)
{

	unsigned char reg, count =0;
	ch7036_status_t status = SS_CRT_HPD_NOTCONNECTED;

	do {
		
		I2CWrite(p_ch7xxx_context,0x03, 0x00); 
		reg = I2CRead(p_ch7xxx_context,0x74); 
		I2CWrite(p_ch7xxx_context,0x74, reg & 0xFD); 
		pd_usleep(SLEEP_TIME);

		I2CWrite(p_ch7xxx_context,0x03, 0x00); 
		reg = I2CRead(p_ch7xxx_context,0x74);
		I2CWrite(p_ch7xxx_context,0x74, reg | 0x02); 		
		pd_usleep(SLEEP_TIME);

		I2CWrite(p_ch7xxx_context,0x03, 0x01); 
		reg = I2CRead(p_ch7xxx_context,0x7C); 		
		
		if( (reg & 0xF0) == 0x50  || (reg & 0xF4) == 0x14 || (reg & 0xF4) == 0x44) 
			status= SS_SUCCESS;
					      	   	    
	} while ( (status == SS_CRT_HPD_NOTCONNECTED) && ( (++count) < HPD_LOOP_MAX ) );

          
	I2CWrite(p_ch7xxx_context,0x03, 0x00); 	        
	reg = I2CRead(p_ch7xxx_context,0x74);	        
	I2CWrite(p_ch7xxx_context,0x74, reg & 0xFD);	 

	I2CWrite(p_ch7xxx_context,0x08, I2CRead(p_ch7xxx_context,0x08) | 0x0E);
		
	return status;

}


ch7036_status_t LHFM_get_hdmi_modeinfo(DEV_CONTEXT* p_ch7xxx_context,unsigned char *minfo)
{
	unsigned char reg;
	unsigned wj;
	unsigned i;

		 I2CWrite(p_ch7xxx_context,0x03, 0x00); 
		 reg = I2CRead(p_ch7xxx_context,0x4F);
		 
		 if (0==(LHFM_REQUEST & reg)) {
			 I2CWrite(p_ch7xxx_context,0x4F, (LHFM_REQUEST+ LHFM_GET_MODEINFO)); 
			 wj = 0;
			 while (wj++< LHFM_TIMEOUT) {
				 I2CWrite(p_ch7xxx_context,0x03, 0x00); 
				 pd_usleep(SLEEP_TIME);

				 reg = I2CRead(p_ch7xxx_context,0x4F);
				 if(reg == LHFM_GET_MODEINFO) {					 
					 reg = I2CRead(p_ch7xxx_context,0x50);			
					 if (!(LHFM_RET_ERROR & reg)) {
						 if (minfo) {
						    I2CWrite(p_ch7xxx_context,0x03, 0x01); 
							for (i=0; i<15; i++) minfo[i] = I2CRead(p_ch7xxx_context,es_map[i]);	 
						}
						return SS_SUCCESS;
					 } else return SS_FIRMWARE_ERR;

				 } 
				 			
			 } 

		 } 

	return SS_FIRMWARE_TIMEOUT;

}


ch7036_status_t LHFM_get_edid(DEV_CONTEXT* p_ch7xxx_context,unsigned char* edid, unsigned char* ebn, unsigned char flag)
{

	unsigned i,j, ie;
	unsigned k1, k2;
	unsigned char reg;
	unsigned wj;

	ch7036_status_t status;



	status = SS_SUCCESS;
	*ebn = 0;    

	pd_memset(edid, 0, EDID_SIZE); 

	I2CWrite(p_ch7xxx_context,0x03, 0x00); 
	reg = I2CRead(p_ch7xxx_context,0x4F);

	if( (reg & LHFM_REQUEST) != 0 )
		return SS_FIRMWARE_ERR; //mcu is busy
	else { //mcu is ready to serve host request
       
		for(i=0,j=0; i < 8; i++,j+=16) {

		    I2CWrite(p_ch7xxx_context,0x03, 0x00); 
			I2CWrite(p_ch7xxx_context,0x50, i | flag); 
			I2CWrite(p_ch7xxx_context,0x4F, (LHFM_REQUEST+LHFM_GET_EDID)); 
			wj = 0;

			status = SS_FIRMWARE_TIMEOUT;
			PD_DEBUG("ch7036: LHFM_get_edid- block [%d] - getting [%d]- 16 bytes - wj=[0x%x]\n",*ebn,i+1,wj);
			while (wj++< (2*LHFM_TIMEOUT) ) { 
				 I2CWrite(p_ch7xxx_context,0x03, 0x00); 
				 pd_usleep(SLEEP_TIME);
				 
				 reg = I2CRead(p_ch7xxx_context,0x4F);
				 
				 if(reg == LHFM_GET_EDID) { 
					 
					 reg = I2CRead(p_ch7xxx_context,0x50);	
					 
					 if (!(LHFM_RET_ERROR & reg)) { 
							I2CWrite(p_ch7xxx_context,0x03, 0x01); 
							for (ie=0; ie<16; ie++) edid[j+ie] = I2CRead(p_ch7xxx_context,es_map[ie] );								
							break; 
					 }
					 else 						
							return SS_FIRMWARE_ERR; 															 

				 } 
				 
				 				 
			} //while block
			
			status = SS_SUCCESS; 

		} //for

		for(i=0;i<8;i++)
			if (edid_header[i] != edid[i]) {
				return SS_FIRMWARE_ERR;
			}
		
		(*ebn)++;
		
		if(flag & 0x80 && (edid[0x7E] > 0) ) { 
			PD_DEBUG("ch7036: LHFM_get_edid- incorrect # of VGA EDID blocks read [%x]\n",edid[0x7E]); 
			return SS_FIRMWARE_ERR;
		}
		
		k2 = edid[0x7E];

		k2 = (k2 > 3)? 3 : k2;  
		
		if (k2>0) { //case: there are more than 1 blocks
			for (k1=1; k1<=k2; k1++) {

				for(i=k1*8; i < 8+k1*8; i++,j+=16){
					I2CWrite(p_ch7xxx_context,0x03, 0x00); 
					I2CWrite(p_ch7xxx_context,0x50, i + flag);  
					I2CWrite(p_ch7xxx_context,0x4F, (LHFM_REQUEST+LHFM_GET_EDID)); 
					wj = 0;
				
					status = SS_FIRMWARE_TIMEOUT;
					PD_DEBUG("ch7036: LHFM_get_edid- block [%d] - getting [%d]- 16 bytes - wj=[0x%x]\n",k1,i+1,wj);
					while (wj++< (2*LHFM_TIMEOUT)) {
						I2CWrite(p_ch7xxx_context,0x03, 0x00); 
						pd_usleep(SLEEP_TIME);
					 
						reg = I2CRead(p_ch7xxx_context,0x4F);
					 
						if(reg == LHFM_GET_EDID) {						 
							reg = I2CRead(p_ch7xxx_context,0x50);			
							if (!(LHFM_RET_ERROR & reg)) {
								I2CWrite(p_ch7xxx_context,0x03, 0x01); 
								for (ie=0; ie<16; ie++) edid[j+ie] = I2CRead(p_ch7xxx_context,es_map[ie] );			
								break;
							}
							else 
								return SS_FIRMWARE_ERR;
	

						} 
					 
					} //while

					status = SS_SUCCESS;

				}//nested for 

				(*ebn)++;

			} //outermost for


		} //if- case: there are more than 1 blocks

	} //else - case:  mcu is ready to serve host request
	
	return status; 
				
}


int LHFM_load_firmware(DEV_CONTEXT* p_ch7xxx_context)
{


	unsigned fs1;
	unsigned char ch;

	PD_DEBUG("ch7036: LHFM_load_firmware- firmware size [0x%.8X]\n",lhfm_size);
	
	I2CWrite(p_ch7xxx_context,0x03, 0x04);
	ch = 0x29 | I2CRead(p_ch7xxx_context,0x52);
	I2CWrite(p_ch7xxx_context,0x52, ch & 0xFB); 
   

	I2CWrite(p_ch7xxx_context,0x5B, 0x9E); 
	I2CWrite(p_ch7xxx_context,0x5B, 0xB3);

	I2CWrite(p_ch7xxx_context,0x03, 0x04);
	I2CWrite(p_ch7xxx_context,0x03, 0x07);

	for (fs1=0; fs1<lhfm_size; fs1++) 
		I2CWrite(p_ch7xxx_context, 0x07, lhfm_array[fs1]);  



	I2CWrite(p_ch7xxx_context, 0x03, 0x03); 
	ch = I2CRead(p_ch7xxx_context,0x74); 
	if (ch & 0x1) { 
		
		return -1;
	}
	else {
		
		I2CWrite( p_ch7xxx_context,0x03, 0x04); 
		ch = I2CRead(p_ch7xxx_context,0x52);
		I2CWrite(p_ch7xxx_context, 0x52, ch | 0x24);  
	}

	I2CWrite(p_ch7xxx_context, 0x03, 0x00);

	return 0;
}
#if 0
void ch7036_dump( char *s, int size, unsigned char *regdata)
{
	char temp[18];
    int ch;

    int i,j, size1;
    
    temp[16] = '?'; 
    temp[17] = '\n';

	PD_DEBUG("\n");
    PD_DEBUG("+--------------------------------------------------------------------+\n");
    PD_DEBUG("|%s |\n",s);
    PD_DEBUG("+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+-----------------+\n");
    PD_DEBUG("|  | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9| A| B| C| D| E| F|                 |\n");
    PD_DEBUG("++++--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+-----------------+\n");
    
    size1 = size % 16; 
    size = size/16; 

    for (i=0; i<size; i++) {

       
       PD_DEBUG("|%02x>", 0xFF & i*16);

       
	   for (j=0; j<16; j++) {
           ch = regdata[i*16+j];
           PD_DEBUG("%02X|", ch);

#if 0
		   
          
		   if( (ch >= 0x30 && ch <= 0x39) || (ch >= 0x61 && ch <= 0x7A) || (ch >= 0x41 && ch <= 0x5C) ) 
				temp[j]='.';
		   else
			   temp[j]=(char)ch; 
#endif
		   
        }
             
	   PD_DEBUG("\n");
	}

	if (size1) {
	
        PD_DEBUG( "|%02x>", 0xFF & size*16);

		for (j=0; j<size1; j++) {
           ch = regdata[size*16+j];
           PD_DEBUG("%02X|", ch);
#if 0
         
			if( (ch >= 0x30 && ch <= 0x39) || (ch >= 0x61 && ch <= 0x7A) || (ch >= 0x41 && ch <= 0x5C) ) 
				temp[j]='.';
		   else
			   temp[j]=(char)ch; 
#endif

        }
		for (j=size1; j<16; j++) {
     
           PD_DEBUG("   ");
      
        }
    
	}
   
	PD_DEBUG("\n");

    return;
}
#endif



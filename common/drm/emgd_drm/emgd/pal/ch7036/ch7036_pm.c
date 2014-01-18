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
* @file  ch7036_pm.c
* @version 1.2.4
*-----------------------------------------------------------------------------
*/



#include "ch7036_intf.h"



void ch7036_set_power_lvds(DEV_CONTEXT* pDevContext);
void ch7036_set_power_hdmi(DEV_CONTEXT* pDevContext);
void ch7036_set_power_crt(DEV_CONTEXT* pDevContext);

void ch7036_set_power_lvds(DEV_CONTEXT* p_ch7xxx_context)
{

	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	uint8 reg=0x00;

	PD_DEBUG("ch7036: ch7036_set_power_lvds()-enter...channel [%lu]\n", pOutput_Info->channel);
	if(pOutput_Info->channel & CHANNEL_LVDS)
	{
		

		I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		reg = I2CRead(p_ch7xxx_context,0x1A);
		reg = reg & 0xEF;                   
		I2CWrite(p_ch7xxx_context,0x1A, reg); 
		
		reg = I2CRead(p_ch7xxx_context,0x11);
		reg = reg & 0xEF;                   
	  if(((pOutput_Info->channel & CHANNEL_HDMI)==0x00)&&
		 ((pOutput_Info->channel & CHANNEL_VGA) == 0x00))
	    {
			
			reg = reg | 0x80;                   
	    }
		 I2CWrite(p_ch7xxx_context,0x11, reg);		

		I2CWrite(p_ch7xxx_context,0x03, 0x04);               
		reg = I2CRead(p_ch7xxx_context,0x66);
		reg = reg & 0xFD;                   
		I2CWrite(p_ch7xxx_context,0x66, reg);

        reg = I2CRead(p_ch7xxx_context,0x64);
		reg = reg & 0xFE;                   
		I2CWrite(p_ch7xxx_context,0x64, reg);

        reg = I2CRead(p_ch7xxx_context,0x63);
		reg = reg & 0x03;                   
		I2CWrite(p_ch7xxx_context,0x63, reg);

        I2CWrite(p_ch7xxx_context,0x03, 0x00);               
		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg & 0xF7;                   
        I2CWrite(p_ch7xxx_context,0x0A, reg);		

		/*Fix backlight blinking during mode switching in and out of lvds*/
		I2CWrite(p_ch7xxx_context,0x4E, I2CRead(p_ch7xxx_context,0x4E) | 0x80 );
		I2CWrite(p_ch7xxx_context,0x4D,0x80);/*need to use PWM to control backlight-6/4/12*/
		
		if((pOutput_Info->channel & CHANNEL_HDMI) == 0x00 && (pOutput_Info->channel & CHANNEL_VGA) == 0x00){

			
		 I2CWrite(p_ch7xxx_context,0x03, 0x00);
		 reg = I2CRead(p_ch7xxx_context,0x07);
		 reg = reg | 0x02;                  
		 I2CWrite(p_ch7xxx_context,0x07, reg);

		 I2CWrite(p_ch7xxx_context,0x03, 0x04);
		 reg = I2CRead(p_ch7xxx_context,0x54);
		 reg = reg | 0x10;                 
		 I2CWrite(p_ch7xxx_context,0x54, reg);

		 I2CWrite(p_ch7xxx_context,0x03, 0x02);             
		 reg = I2CRead(p_ch7xxx_context,0x16);
		 reg = reg | 0x08; 
		
	
		 I2CWrite(p_ch7xxx_context,0x16, reg);

		}
		
		
				
	}
	
	if((pOutput_Info->channel & CHANNEL_LVDS) == 0x00)
	{
		
		

        I2CWrite(p_ch7xxx_context,0x03, 0x00);   

		/*Fix backlight blinking during mode switching in and out of lvds*/
		I2CWrite(p_ch7xxx_context,0x4E, I2CRead(p_ch7xxx_context,0x4E) & 0x7F );
		I2CWrite(p_ch7xxx_context,0x4D,0x0);/*need to use PWM to control backlight-6/4/12*/

		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg | 0x08;                   
        I2CWrite(p_ch7xxx_context,0x0A, reg);

		if((pOutput_Info->channel & CHANNEL_HDMI) == 0x00 && (pOutput_Info->channel & CHANNEL_VGA)==0x00)
		{
			
			I2CWrite(p_ch7xxx_context,0x03, 0x04);               
			reg = I2CRead(p_ch7xxx_context,0x63);
			reg = reg | 0xFC;                   
			I2CWrite(p_ch7xxx_context,0x63, reg);

			reg = I2CRead(p_ch7xxx_context,0x64);
			reg = reg | 0x01;                   
			I2CWrite(p_ch7xxx_context,0x64, reg);
		}

		I2CWrite(p_ch7xxx_context,0x03, 0x04); 
		reg = I2CRead(p_ch7xxx_context,0x66);
		reg = reg | 0x02;                   
		I2CWrite(p_ch7xxx_context,0x66, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		reg = I2CRead(p_ch7xxx_context,0x11);
		reg = reg | 0x10;                   
		I2CWrite(p_ch7xxx_context,0x11, reg);

		reg = I2CRead(p_ch7xxx_context,0x1A);
		reg = reg | 0x10;                   
		I2CWrite(p_ch7xxx_context,0x1A, reg);

	}


}

void ch7036_set_power_hdmi(DEV_CONTEXT* p_ch7xxx_context)
{

	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	INPUT_INFO* pInput_Info = p_ch7xxx_context->pInput_Info;
	uint8 reg=0x00;

	PD_DEBUG("ch7036: ch7036_set_power_hdmi()-enter...channel [%lu]\n", pOutput_Info->channel);
   if(pOutput_Info->channel  & CHANNEL_HDMI)
   {
	   
		
        I2CWrite(p_ch7xxx_context,0x03, 0x04);  
		reg = I2CRead(p_ch7xxx_context,0x52);   
		reg = reg & 0xEF;      
		I2CWrite(p_ch7xxx_context,0x52, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg & 0xDF;      
		I2CWrite(p_ch7xxx_context,0x0A, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xBF;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xF7;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xEF;      
		I2CWrite(p_ch7xxx_context,0x09, reg);


		 
	   I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		reg = I2CRead(p_ch7xxx_context,0x11);
		reg = reg & 0x7F;                   
	    I2CWrite(p_ch7xxx_context,0x11, reg);

		 I2CWrite(p_ch7xxx_context,0x03, 0x00);
		 reg = I2CRead(p_ch7xxx_context,0x07);
		 reg = reg & 0xFD;                  
		 I2CWrite(p_ch7xxx_context,0x07, reg);
		  
		 I2CWrite(p_ch7xxx_context,0x03, 0x04);  
		reg = I2CRead(p_ch7xxx_context,0x54);
		reg = reg & 0xEF;
		I2CWrite(p_ch7xxx_context,0x54, reg);

	  

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg & 0xFB;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xFB;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x01);  
		reg = I2CRead(p_ch7xxx_context,0x0E);
		reg = reg & 0x7F;      
		I2CWrite(p_ch7xxx_context,0x0E, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x07);
		
		if(pInput_Info->audio_type == AUDIO_I2S){
          
		   reg = reg & 0xBF;   
		   reg = reg | 0x01;   
		}else{
		  
           reg = reg | 0x40;   
		   reg = reg & 0xFE;   
		}
		I2CWrite(p_ch7xxx_context,0x07, reg);


		I2CWrite(p_ch7xxx_context,0x03, 0x01);  
		reg = I2CRead(p_ch7xxx_context,0x16);
		reg = reg & 0xF7;      
		I2CWrite(p_ch7xxx_context,0x16, reg);

		reg = I2CRead(p_ch7xxx_context,0x16);
		reg = reg & 0xFE;      
		I2CWrite(p_ch7xxx_context,0x16, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x08);  
		reg = reg & 0x0F;      
		I2CWrite(p_ch7xxx_context,0x08, reg);

		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg & 0xF7;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x10;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xFE;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

				

   }else {

	   
	   if((pOutput_Info->channel  & CHANNEL_VGA) == 0x00){


        I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg | 0x20;      
		I2CWrite(p_ch7xxx_context,0x0A, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x40;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x08;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x10;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x04;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		 reg = I2CRead(p_ch7xxx_context,0x11);
		 reg = reg | 0x80;                   
		 I2CWrite(p_ch7xxx_context,0x11, reg);	

		
		 I2CWrite(p_ch7xxx_context,0x03, 0x00);
		 reg = I2CRead(p_ch7xxx_context,0x07);
		 reg = reg | 0x02;                  
		 I2CWrite(p_ch7xxx_context,0x07, reg);
		  

		 I2CWrite(p_ch7xxx_context,0x03, 0x04);
		 reg = I2CRead(p_ch7xxx_context,0x54);
		 reg = reg | 0x10;                 
		 I2CWrite(p_ch7xxx_context,0x54, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		 reg = I2CRead(p_ch7xxx_context,0x09);
		 reg = reg | 0x04;      
		 I2CWrite(p_ch7xxx_context,0x09, reg);
    
		}
		

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x40;   
		reg = reg | 0x01;   
		I2CWrite(p_ch7xxx_context,0x07, reg);

				

		if((pOutput_Info->channel  & CHANNEL_LVDS) == 0x00)
		{
		 I2CWrite(p_ch7xxx_context,0x03, 0x01);  
		 reg = I2CRead(p_ch7xxx_context,0x16);
		 reg = reg | 0x08;      
		 I2CWrite(p_ch7xxx_context,0x16, reg);
		}

		I2CWrite(p_ch7xxx_context,0x03, 0x01); 
		reg = I2CRead(p_ch7xxx_context,0x16);
		reg = reg | 0x01;      
		I2CWrite(p_ch7xxx_context,0x16, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x08);  
		reg = reg | 0xF0;      
		I2CWrite(p_ch7xxx_context,0x08, reg);

		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x08;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x10;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x01;      
		I2CWrite(p_ch7xxx_context,0x09, reg);
	

   }
		   
}

void ch7036_set_power_crt(DEV_CONTEXT* p_ch7xxx_context)
{
	

	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	uint8 reg=0x00; 

	PD_DEBUG("ch7036: ch7036_set_power_crt()-enter...channel [%lu]\n", pOutput_Info->channel);
	if(pOutput_Info->channel & CHANNEL_VGA)
	 {
		 
	     
	   I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		reg = I2CRead(p_ch7xxx_context,0x11);
		reg = reg & 0x7F;                   
	    I2CWrite(p_ch7xxx_context,0x11, reg);
  

		I2CWrite(p_ch7xxx_context,0x03, 0x00);
		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg & 0xDF;      
		I2CWrite(p_ch7xxx_context,0x0A, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xBF;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xF7;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);
		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xEF;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x00);
		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg & 0xF9;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		I2CWrite(p_ch7xxx_context,0x03, 0x04);  
		reg = I2CRead(p_ch7xxx_context,0x54);
		reg = reg & 0xEF;
		I2CWrite(p_ch7xxx_context,0x54, reg);

		
		I2CWrite(p_ch7xxx_context,0x03, 0x00); 
		reg = I2CRead(p_ch7xxx_context,0x08);
		reg = reg & 0xF1;      
		I2CWrite(p_ch7xxx_context,0x08, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg & 0xFB;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

	}else if((pOutput_Info->channel & CHANNEL_VGA) == 0x00){ 

		
       
		I2CWrite(p_ch7xxx_context,0x03, 0x00); 
	   
		 reg = I2CRead(p_ch7xxx_context,0x08);
		reg = reg | 0x0E;     
		I2CWrite(p_ch7xxx_context,0x08, reg);



		if((pOutput_Info->channel & CHANNEL_HDMI)==0x00){

			

	     I2CWrite(p_ch7xxx_context,0x03, 0x01);               
		 reg = I2CRead(p_ch7xxx_context,0x11);
		 reg = reg | 0x80;                   
	     I2CWrite(p_ch7xxx_context,0x11, reg);

		  I2CWrite(p_ch7xxx_context,0x03, 0x00);
		 reg = I2CRead(p_ch7xxx_context,0x07);
		 reg = reg | 0x02;                  
		 I2CWrite(p_ch7xxx_context,0x07, reg);
		  

		 I2CWrite(p_ch7xxx_context,0x03, 0x04);
		 reg = I2CRead(p_ch7xxx_context,0x54);
		 reg = reg | 0x10;                 
		 I2CWrite(p_ch7xxx_context,0x54, reg);

		 
        I2CWrite(p_ch7xxx_context,0x03, 0x00);  
		reg = I2CRead(p_ch7xxx_context,0x0A);
		reg = reg | 0x20;      
		I2CWrite(p_ch7xxx_context,0x0A, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x40;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x08;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		reg = I2CRead(p_ch7xxx_context,0x09);
		reg = reg | 0x10;      
		I2CWrite(p_ch7xxx_context,0x09, reg);

		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg | 0x04;      
		I2CWrite(p_ch7xxx_context,0x07, reg);

		
		 reg = I2CRead(p_ch7xxx_context,0x09);
		 reg = reg | 0x04;      
		 I2CWrite(p_ch7xxx_context,0x09, reg);
	   }
	} 

	return;
}



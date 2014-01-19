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
* @file  ch7036_attr.c
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



static pd_attr_t g_ch7036_attrs[] =
{
	

	
	PD_MAKE_ATTR (PD_ATTR_ID_HPOSITION,   PD_ATTR_TYPE_RANGE, "H Pos.",  0, DEFAULT_POSITION,	DEFAULT_POSITION,                    DEFAULT_POSITION - 40,  DEFAULT_POSITION + 40,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_VPOSITION,   PD_ATTR_TYPE_RANGE, "V Pos.",  0, DEFAULT_POSITION,	DEFAULT_POSITION,                    DEFAULT_POSITION - 30,  DEFAULT_POSITION + 30,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_HSCALE,   PD_ATTR_TYPE_RANGE, "H_Scale",  0, HDMI_DEFAULT_UNDERSCAN,   HDMI_DEFAULT_UNDERSCAN,  10,  20,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_VSCALE,   PD_ATTR_TYPE_RANGE, "V_Scale",  0, HDMI_DEFAULT_UNDERSCAN,     HDMI_DEFAULT_UNDERSCAN,   10,  20,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_HSCALE_CRT,   PD_ATTR_TYPE_RANGE, "H_Scale",  0, CRT_DEFAULT_UNDERSCAN,   CRT_DEFAULT_UNDERSCAN,  10,  20,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_VSCALE_CRT,   PD_ATTR_TYPE_RANGE, "V_Scale",  0, CRT_DEFAULT_UNDERSCAN,   CRT_DEFAULT_UNDERSCAN,  10,  20,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_DITHER,   PD_ATTR_TYPE_RANGE, "Dither", PD_ATTR_FLAG_USER_INVISIBLE, 0,   0,  0,  3,  1),
	
	PD_MAKE_ATTR (PD_ATTR_ID_TEXT_FILTER,   PD_ATTR_TYPE_RANGE, "Text Tuning", PD_ATTR_FLAG_USER_INVISIBLE, 4,   DEFAULT_TEXT_ENHANCE,  0,  7,  1),
	

	PD_MAKE_ATTR (PD_ATTR_ID_DITHER_BYPASS, PD_ATTR_TYPE_BOOL, "Quality Enhance",  0, 0,                 0,                 0, 0, 0),
	
	PD_MAKE_ATTR (PD_ATTR_ID_PLL_REF_DLY, PD_ATTR_TYPE_BOOL, "Pll Reference Delay",PD_ATTR_FLAG_USER_INVISIBLE, 0,0,0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_PLL_REF_FBDLY,PD_ATTR_TYPE_BOOL, "Pll Reference FBDelay",PD_ATTR_FLAG_USER_INVISIBLE, 1,   1, 0, 0, 0),
	
	PD_MAKE_ATTR (PD_ATTR_ID_LOAD_FIRMWARE,PD_ATTR_TYPE_BOOL, "Load Firmware",PD_ATTR_FLAG_USER_INVISIBLE, 0, 1, 0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_REFRESH,PD_ATTR_TYPE_BOOL, "Refresh",0, 0,   0, 0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_DWNSCAL_BYPASS,PD_ATTR_TYPE_BOOL, "Remove_DS",PD_ATTR_FLAG_USER_INVISIBLE, 0, 1, 0, 0, 0),
	
};


static ch7036_attr_list_entry_t g_list_entry_hdmi[] =
{
	{OUT_HDMI_640x480P_59,		"640x480p_59",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_HDMI_640x480P_60,		"640x480p_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_HDMI_720x480P_59,		"720x480p_59",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_720x480P_60,		"720x480p_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_HDMI_720x576P_50,     "720x576p_50",PD_ATTR_FLAG_DYNAMIC},

	{OUT_HDMI_1280x720P_59,    "1280x720p_59",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_1280x720P_60,    "1280x720p_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_HDMI_1920x1080I_59,   "1920x1080i_59",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_1920x1080I_60,   "1920x1080i_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_HDMI_1920x1080P_59,   "1920x1080p_59",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_1920x1080P_60,   "1920x1080p_60",PD_ATTR_FLAG_DYNAMIC}, 
	
	{OUT_HDMI_1280x720P_50,    "1280x720p_50",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_1920x1080I_50,   "1920x1080i_50",PD_ATTR_FLAG_DYNAMIC},
	{OUT_HDMI_1920x1080P_50,   "1920x1080p_50",PD_ATTR_FLAG_DYNAMIC},
	{0, NULL,0}
};



static ch7036_attr_list_entry_t g_list_entry_dvi[] =
{

	{OUT_DVI_640x480_60,	"640x480_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_640x480_72,	"640x480_72",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_640x480_75,	"640x480_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_720x400_70,	"720x400_70",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_800x600_56,	"800x600_56",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_800x600_60,	"800x600_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_800x600_72,	"800x600_72",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_800x600_75,	"800x600_75",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1024x768_60,	"1024x768_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_1024x768_70,	"1024x768_70",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1024x768_75,	"1024x768_75",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1152x864_60,	"1152x864_60",PD_ATTR_FLAG_DYNAMIC}, 

	{OUT_DVI_1280x720_60,   "1280x720_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_1280x800_60,   "1280x800_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1280x960_60,    "1280x960_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1280x1024_60,   "1280x1024_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_1280x1024_75,   "1280x1024_75",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1360x768_60,    "1360x768_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1366x768_60,    "1366x768_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1400x1050_60,   "1400x1050_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1400x1050_75,   "1400x1050_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1440x900_60,    "1440x900_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1440x1050_60,   "1440x1050_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1600x900_60,    "1600x900_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_DVI_1600x1200_60,   "1600x1200_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_DVI_1680x1050_60,   "1680x1050_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_DVI_1920x1080_60,   "1920x1080_60",PD_ATTR_FLAG_DYNAMIC}, 
	{0, NULL,0}
};



static ch7036_attr_list_entry_t g_list_entry_crt[] =
{
	{OUT_CRT_640x400_85, "640x400_85",PD_ATTR_FLAG_DYNAMIC}, 

	{OUT_CRT_640x480_60, "640x480_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_640x480_72, "640x480_72",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_640x480_75, "640x480_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_640x480_85, "640x480_85",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_720x400_85, "720x400_85",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_800x600_56, "800x600_56",PD_ATTR_FLAG_DYNAMIC},
	
	{OUT_CRT_800x600_60, "800x600_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_CRT_800x600_72, "800x600_72",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_800x600_75, "800x600_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_800x600_85, "800x600_85",PD_ATTR_FLAG_DYNAMIC}, 
	
	{OUT_CRT_1024x768_60,"1024x768_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_CRT_1024x768_70,"1024x768_70",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1024x768_75,"1024x768_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1024x768_85,"1024x768_85",PD_ATTR_FLAG_DYNAMIC},
	

	{OUT_CRT_1152x864_75,"1152x864_75",PD_ATTR_FLAG_DYNAMIC}, 

	{OUT_CRT_1280x768_60,"1280x768_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1280x768_75,"1280x768_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1280x768_85,"1280x768_85",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_1280x960_60,"1280x960_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1280x960_85,"1280x960_85",PD_ATTR_FLAG_DYNAMIC},


	{OUT_CRT_1280x1024_60,"1280x1024_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_CRT_1280x1024_75,"1280x1024_75",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1280x1024_85,"1280x1024_85",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_1360x768_60,"1360x768_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_1400x1050_60,"1400x1050_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1400x1050_75,"1400x1050_75",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_1440x900_60,"1440x900_60",PD_ATTR_FLAG_DYNAMIC},
	{OUT_CRT_1440x1050_60,"1440x1050_60",PD_ATTR_FLAG_DYNAMIC},

	{OUT_CRT_1600x900_60,"1600x900_60",PD_ATTR_FLAG_DYNAMIC},
	
	{OUT_CRT_1600x1200_60,"1600x1200_60",PD_ATTR_FLAG_DYNAMIC}, 
	{OUT_CRT_1920x1080_60,"1920x1080_60",PD_ATTR_FLAG_DYNAMIC}, 
	{0, NULL,0}
};


static ch7036_attr_list_entry_t g_list_entry_channel[] =
{
	{CHANNEL_AUTO_DETECT,"Auto Detect",PD_ATTR_FLAG_DYNAMIC},
	{CHANNEL_LVDS_HDMI, "LVDS_HDMI",PD_ATTR_FLAG_DYNAMIC}, 
	{CHANNEL_LVDS_DVI, "LVDS_DVI",PD_ATTR_FLAG_DYNAMIC},
	{CHANNEL_LVDS_VGA,  "LVDS_VGA",PD_ATTR_FLAG_DYNAMIC},
	{CHANNEL_HDMI, "HDMI",PD_ATTR_FLAG_DYNAMIC}, 
	{CHANNEL_DVI, "DVI",PD_ATTR_FLAG_DYNAMIC},
	{CHANNEL_VGA,  "VGA",PD_ATTR_FLAG_DYNAMIC},
	{0, NULL,0}	
};

static ch7036_attr_list_header_t g_list_header[] =
{
	{7, PD_ATTR_ID_DISPLAY,"Display Channel", 1,1, 
	g_list_entry_channel},

	{14, PD_ATTR_ID_HDMI_OUT_MODE, "HDMI",7,7,
	 g_list_entry_hdmi},

	{27, PD_ATTR_ID_DVI_OUT_MODE,"DVI", 9,9,
	 g_list_entry_dvi},

	{32, PD_ATTR_ID_CRT_OUT_MODE, "VGA", 12,12,
	 g_list_entry_crt},

	{ 0 }
};



ch7036_status_t ch7036_init_attribute_table(ch7036_device_context_t *p_ctx, ch7036_attr_list_header_t* p_list_header)
{
	unsigned long ch7036_num_attrs, num_attrs;
	unsigned char *p_table;
	

	
	ch7036_num_attrs = ch7036_enumerate_attr_table(p_ctx, NULL, p_list_header);
	PD_DEBUG("ch7036_init_attribute_table- ch7036 num_attr = [%lu]\n",ch7036_num_attrs);

	
	p_ctx->ch7036_num_attrs =  ch7036_num_attrs ;  


	if(p_ctx->p_ch7036_attr_table) {
		pd_free(p_ctx->p_ch7036_attr_table);
		p_ctx->p_ch7036_attr_table = NULL;
	}

	
	if(p_ctx->p_ch7036_attr_table == NULL) {
	
		p_ctx->p_ch7036_attr_table = pd_malloc((p_ctx->ch7036_num_attrs + 1) * sizeof(pd_attr_t)); 

		if (p_ctx->p_ch7036_attr_table == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_init_attribute_table: pd_malloc() failed allocating ch7036_attr_table");
		
			return SS_MEM_ALLOC_ERR;
		}

		pd_memset(p_ctx->p_ch7036_attr_table, 0, (p_ctx->ch7036_num_attrs + 1) *
			sizeof(pd_attr_t));

	}


	p_table = (unsigned char *)p_ctx->p_ch7036_attr_table;


	if (p_ctx->ch7036_num_attrs  > 0) {

		num_attrs = ch7036_enumerate_attr_table(p_ctx, (pd_attr_t *)p_table, p_list_header);


		PD_DEBUG("ch7036_init_attribute_table: ch7036_enumerate_attr_table()- returned %lu entries, expected %lu\n",
					  num_attrs, p_ctx->ch7036_num_attrs) ;

	}

	
	return SS_SUCCESS;

}



unsigned long ch7036_enumerate_attr_table (ch7036_device_context_t *p_ctx,
	pd_attr_t *p_attr, ch7036_attr_list_header_t* p_list_header)
{

	
	unsigned long num_attrs, i, j, num_attrs_static;
	ch7036_attr_list_entry_t *list_item;
	int ret;
	pd_attr_t *p_table;

	list_item = NULL;
	num_attrs = 0;
	i = j = 0;

	
	PD_DEBUG("ch7036_enumerate_attr_table- enter\n");


	
	if ( p_list_header == NULL )
			p_list_header = g_list_header; 
		
	
	if (p_attr == NULL) {

		if(p_ctx->p_ch7036_attr_table == NULL) { 
			ret = PD_INTERNAL_LVDS_MODULE_GET_ATTRIBUTES(ch7036_lvds_get_attrs,(p_ctx->internal_lvds,&(p_ctx->lvds_num_attrs),&(p_ctx->p_lvds_attr_table))); 

			if(ret != PD_SUCCESS)
				PD_DEBUG("ch7036_enumerate_attr_table: lvds_get_attributes() return ERROR! check this routine\n");
			else
				PD_DEBUG("ch7036_enumerate_attr_table: lvds_get_attributes()- lvds num_attr = [%lu]\n",p_ctx->lvds_num_attrs);
		}
		
				

		while (p_list_header[i].num_entries) {


			num_attrs += (p_list_header[i].num_entries + 1); 
			++i;	 
		}

		num_attrs += ((sizeof(g_ch7036_attrs)/sizeof(pd_attr_t)) + p_ctx->lvds_num_attrs) ;

		return num_attrs; 
	}

	
	if(p_ctx->lvds_num_attrs > 0) {

		pd_attr_t *p_lvds_des, *p_lvds_src ;

		
		p_lvds_src=  (pd_attr_t *)p_ctx->p_lvds_attr_table;
		p_lvds_des = (pd_attr_t *)p_attr;

		pd_memcpy(p_lvds_des, p_lvds_src, (p_ctx->lvds_num_attrs)*sizeof(pd_attr_t));
		num_attrs += p_ctx->lvds_num_attrs;

		for (i=0 ; i < p_ctx->lvds_num_attrs; i++, p_lvds_des++) {

			PD_DEBUG("ch7036 : ch7036_enumerate_attr_table : \n"
				  "adding lvds attr='%s', id=%lu, default=%lu, current=%lu, \n",
				  p_lvds_des->name, p_lvds_des->id, p_lvds_des->default_value, p_lvds_des->current_value);
		}

	}

	PD_DEBUG("ch7036 : ch7036_enumerate_attr_table adding lvds num attrs = %lu \n", num_attrs);

	

	i=0;


	while (p_list_header[i].num_entries) {

		pd_list_attr_t *p_hdr = (pd_list_attr_t *) &p_attr[num_attrs];

	
		p_hdr->type	= PD_ATTR_TYPE_LIST;
		p_hdr->id = p_list_header[i].id;
		p_hdr->num_entries = p_list_header[i].num_entries;
		p_hdr->flags |= PD_ATTR_FLAG_DYNAMIC;

		p_hdr->current_index = p_list_header[i].current_index;
		pd_strcpy(p_hdr->name, p_list_header[i].p_name);

		p_hdr->default_index = p_list_header[i].default_index;

		PD_DEBUG("ch7036 : ch7036_enumerate_attr_table : \n"
				  "adding attr - list header='%s', id=%lu, default=%lu, current=%lu, \n"
				  "num_entries=%lu\n",
				  p_hdr->name, p_hdr->id, p_hdr->default_index, p_hdr->current_index,
				  p_hdr->num_entries);

		++num_attrs;



		list_item = p_list_header[i].attr_list;
		for (j = 0; j < p_list_header[i].num_entries; ++j, ++num_attrs) {

			pd_list_entry_attr_t *p_entry =
				(pd_list_entry_attr_t *)&p_attr[num_attrs];

			p_entry->id		= p_list_header[i].id;  
			p_entry->type	= PD_ATTR_TYPE_LIST_ENTRY;
			p_entry->value	= list_item[j].id;     
			

			p_entry->flags  = list_item[j].flags; 

			
			
			
			

			

			pd_strcpy(p_entry->name, list_item[j].p_name);

			

			PD_DEBUG("ch7036 : ch7036_enumerate_attr_table : list entry[%lu]=%s, id=[%lu], "
					  "value=%lu, flags=0x%x \n",
					  j, p_entry->name, p_entry->id,
					  p_entry->value, (unsigned char)p_entry->flags);
					  
		}

		++i;   
	}

	
	p_table = (pd_attr_t *)&p_attr[num_attrs];
	num_attrs_static = sizeof(g_ch7036_attrs)/sizeof(pd_attr_t);
	pd_memcpy(p_table, g_ch7036_attrs,num_attrs_static*sizeof(pd_attr_t));

	for (i=0 ; i < num_attrs_static; i++, p_table++) {

		PD_DEBUG("ch7036 : ch7036_enumerate_attr_table : \n"
				  "adding ch7036 static attr='%s', id=%lu, default=%lu, current=%lu, \n",
				  p_table->name, p_table->id, p_table->default_value, p_table->current_value);

	}

	num_attrs += num_attrs_static;


	return num_attrs;

}

void ch7036_set_dither(ch7036_device_context_t* p_ctx)
{
	uint8 reg=0x00;
	DEV_CONTEXT* p_ch_ctx = p_ctx->p_ch7xxx_context;
	
	I2CWrite(p_ch_ctx,0x03, 0x04);
	reg = I2CRead(p_ch_ctx,0x57);
    reg = reg & 0x3F;


	switch(p_ctx->dither_select)
	{
		  case DITHER_18_TO_18: 
			  reg &= 0x3F;
			  break;
		  case DITHER_18_TO_24: 
			  reg = reg | 0x40;
			  break;
		  case DITHER_24_TO_18: 
			  reg = reg | 0x80;		    
			  break;
		  case DITHER_24_TO_24: 
			  reg = reg | 0xC0;
			  break;
		  default:
			 break;
    }
	I2CWrite(p_ch_ctx,0x57, reg);

	return;
}



void ch7036_set_audio_type(INPUT_INFO* pInput_Info, uint8 id)
{
	
	pInput_Info->audio_type = id; 
	
	if(pInput_Info->audio_type == AUDIO_I2S) {
		
		
		
		
		pInput_Info->i2s_pol = 0; 
		pInput_Info->i2s_len = 0; 
		pInput_Info->i2s_fmt = 0; 
		
	}
}



ch7036_status_t ch7036_set_output_channel(void* p_context, uint32 channel)
{
	ch7036_device_context_t * p_ctx= (ch7036_device_context_t *)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	pd_attr_t *p_attr_hdmi, *p_attr_crt, *p_attr_dvi;
	pd_attr_t *p_attr_hscale, *p_attr_vscale, *p_attr_hscale_crt, *p_attr_vscale_crt, *p_attr_hp, *p_attr_vp;
	pd_attr_t* p_attr_dither;


	
	PD_DEBUG("ch7036_set_output_channel- channel [%x]\n", channel);
	
	pOutput_Info->channel = channel;



	p_attr_dvi = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
				PD_ATTR_ID_DVI_OUT_MODE, PD_GET_ATTR_LIST);

	p_attr_hdmi = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
				PD_ATTR_ID_HDMI_OUT_MODE, PD_GET_ATTR_LIST);
	

	p_attr_crt = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_CRT_OUT_MODE, PD_GET_ATTR_LIST);

	
	p_attr_hscale_crt = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_HSCALE_CRT, 0);
	p_attr_vscale_crt = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_VSCALE_CRT, 0);
	
	p_attr_hscale = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_HSCALE, 0);
	p_attr_vscale = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_VSCALE, 0);

	p_attr_hp = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
				PD_ATTR_ID_HPOSITION, 0);
	p_attr_vp = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
				PD_ATTR_ID_VPOSITION, 0);

	p_attr_dither = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
				PD_ATTR_ID_DITHER_BYPASS, 0);
			
	

	if(pOutput_Info->channel & CHANNEL_HDMI) {

		if(pOutput_Info->hdmi_fmt.is_dvi_mode) {

			if(p_attr_hdmi) 
				p_attr_hdmi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;  

			p_attr_dvi->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE; 

			
			p_attr_dvi += p_attr_dvi->current_value; 
			p_ctx->dvi_mode_index = ((pd_list_entry_attr_t *)p_attr_dvi)->value;

			PD_DEBUG("ch7036_set_output_channel- dvi mode index [%hu]\n",p_ctx->dvi_mode_index);



		}
		else { 
			
			p_attr_hdmi->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE; 

			if(p_attr_dvi) 
				p_attr_dvi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;  

			
			p_attr_hdmi += p_attr_hdmi->current_value; 
			p_ctx->hdmi_mode_index = ((pd_list_entry_attr_t *)p_attr_hdmi)->value;

			PD_DEBUG("ch7036_set_output_channel- hdmi mode index [%hu]\n",p_ctx->hdmi_mode_index);

		}

		if(p_attr_crt)
			p_attr_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		
		//hdmi/dvi- hide it from user
		p_attr_hscale_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		//h/v pos scale- hide it
		p_attr_hp->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vp->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		
		//show it
		p_attr_hscale->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;


		p_attr_dither->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;


		PD_DEBUG("ch7036_set_output_channel-current hscale value [%d]\n",(uint8)p_attr_hscale->current_value);
		PD_DEBUG("ch7036_set_output_channel-current vscale value [%d]\n",(uint8)p_attr_vscale->current_value);
		
		ch7036_set_scaling(pOutput_Info,PD_ATTR_ID_HSCALE,(uint8)p_attr_hscale->current_value);
		ch7036_set_scaling(pOutput_Info,PD_ATTR_ID_VSCALE,(uint8)p_attr_vscale->current_value);


	} else if (pOutput_Info->channel & CHANNEL_VGA) {
		
		p_attr_crt->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;
		
		if(p_attr_hdmi)
			p_attr_hdmi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		if(p_attr_dvi)
			p_attr_dvi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		p_attr_crt += p_attr_crt->current_value; 
		p_ctx->crt_mode_index = ((pd_list_entry_attr_t *)p_attr_crt)->value;
	
		PD_DEBUG("ch7036_set_output_channel- crt mode index [%hu]\n",p_ctx->crt_mode_index);

		
		p_attr_hscale_crt->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale_crt->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;
		
		p_attr_hp->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vp->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;

		p_attr_dither->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE;


		p_attr_hscale->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		PD_DEBUG("ch7036_set_output_channel-current hscale_crt value [%d]\n",(uint8)p_attr_hscale_crt->current_value);
		PD_DEBUG("ch7036_set_output_channel-current vscale_crt value [%d]\n",(uint8)p_attr_vscale_crt->current_value);

		
		ch7036_set_scaling(pOutput_Info,PD_ATTR_ID_HSCALE_CRT,(uint8)p_attr_hscale_crt->current_value);
		ch7036_set_scaling(pOutput_Info,PD_ATTR_ID_VSCALE_CRT,(uint8)p_attr_vscale_crt->current_value);

		
	} else {

		if(p_attr_crt)
			p_attr_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		if(p_attr_hdmi)
			p_attr_hdmi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		if(p_attr_dvi)
			p_attr_dvi->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		
		p_attr_hscale->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		p_attr_hscale_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
		p_attr_vscale_crt->flags |= PD_ATTR_FLAG_USER_INVISIBLE;
	
		p_attr_hp->flags |= PD_ATTR_FLAG_USER_INVISIBLE;		
		p_attr_vp->flags |= PD_ATTR_FLAG_USER_INVISIBLE;

		p_attr_dither->flags |= PD_ATTR_FLAG_USER_INVISIBLE;


	}
	

	return SS_SUCCESS;
}
unsigned long ch7036_get_output_channel(void* p_context)
{
	ch7036_device_context_t * p_ctx= (ch7036_device_context_t *)p_context;
	pd_list_entry_attr_t *p_attr;
	uint32 channel = 0;

	PD_DEBUG("ch7036_get_output_channel- enter\n");

	p_attr = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,
			PD_ATTR_ID_DISPLAY, PD_GET_ATTR_LIST_ENTRY);

	if(p_attr->value & CHANNEL_DVI)
		channel = (p_attr->value & 0xF7) | CHANNEL_HDMI;

	return channel;
}

ch7036_status_t ch7036_set_position(ch7036_device_context_t *p_ctx, uint8 attr_id, uint16 pos)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;

	uint8 reg;

	PD_DEBUG("ch7036_set_position- enter\n");
	I2CWrite(p_ch7xxx_context,0x03, 0x00);
	reg = I2CRead(p_ch7xxx_context,0x39);
	
	if(attr_id == PD_ATTR_ID_HPOSITION) {
		reg = reg & 0xF0; 
		I2CWrite(p_ch7xxx_context,0x3A, (pos & 0x00FF));
		reg = ((pos >> 8) & 0x0F) | reg;	
		iic_write_ex(HP,pos);
	} 
	else { 
		reg = reg & 0x0F;
		I2CWrite(p_ch7xxx_context,0x3B, (pos & 0x00FF));
		reg = (((pos >> 8) & 0x0F) << 4) | reg;	
		iic_write_ex(VP,pos);
	}

	I2CWrite(p_ch7xxx_context,0x39, reg);
	
	return SS_SUCCESS;
}


void ch7036_set_hdmi_sync_polarity(OUTPUT_INFO* pOutput_Info)
{
	
	if((pOutput_Info->channel & CHANNEL_HDMI)&&(pOutput_Info->channel & CHANNEL_VGA))
	{
		if (pOutput_Info->timing.ha > 720) {
			pOutput_Info->hdmi_fmt.hs_pol = POL_HIGH;
			pOutput_Info->hdmi_fmt.vs_pol = POL_HIGH;
			if(pOutput_Info->timing.ha == 1440 )
			{
              pOutput_Info->hdmi_fmt.hs_pol = POL_LOW;
			  pOutput_Info->hdmi_fmt.vs_pol = POL_LOW;
			}
		}
		else if(pOutput_Info->timing.ha <= 720){
			
			pOutput_Info->hdmi_fmt.hs_pol = POL_LOW;
			pOutput_Info->hdmi_fmt.vs_pol = POL_LOW;
		}
	}
	
	
	if((pOutput_Info->channel & CHANNEL_HDMI) && ((pOutput_Info->channel & CHANNEL_VGA) == 0x00))
	{	
		pOutput_Info->hdmi_fmt.hs_pol = POL_LOW;
		pOutput_Info->hdmi_fmt.vs_pol = POL_LOW;
	}

}

void ch7036_set_quality_enhancement(ch7036_device_context_t *p_ctx, uint8 checked)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	PREFER_INFO* pPrefer_Info = p_ch7xxx_context->pPrefer_Info;
	uint8 reg;

	PD_DEBUG("ch7036_set_quality_enhancement- enter\n");
	I2CWrite(p_ch7xxx_context,0x03, 0x00);	
	reg = I2CRead(p_ch7xxx_context,0x19); 
	
	if(checked) 
		reg = reg & 0xBF; 
	else
		reg = reg | 0x40; 
	
	I2CWrite(p_ch7xxx_context,0x19, reg);

	
	pPrefer_Info->dither_filter_enable = checked?DITHER_ENABLE:DITHER_BYPASS;

	
	iic_write_ex(DBP, checked?DITHER_ENABLE:DITHER_BYPASS);
	
}

void ch7036_set_scaling (OUTPUT_INFO* pOutput_Info, unsigned long id, uint8 value)
{
	uint8* p_scale;

	PD_DEBUG("ch7036_set_scaling- enter- value [%d]\n", value);
		
	if(id == PD_ATTR_ID_HSCALE || id == PD_ATTR_ID_HSCALE_CRT)
		p_scale= &(pOutput_Info->ds_percent_h);
	else 
		p_scale= &(pOutput_Info->ds_percent_v);
		
	switch(value) {
		case 20: *p_scale = 0; break; 
		case 19: *p_scale = 1; break; 
		case 18: *p_scale = 2; break;
		case 17: *p_scale = 3; break;
		case 16: *p_scale = 4; break;
		case 15: *p_scale = 5; break;
		case 14: *p_scale = 6; break;
		case 13: *p_scale = 7; break;
		case 12: *p_scale = 8; break;
		case 11: *p_scale = 9; break;
		case 10: *p_scale = 10; break;
		
		case 9: *p_scale = 11; break;
		case 8: *p_scale = 12; break;
		case 7: *p_scale = 13; break;
		case 6: *p_scale = 14; break;
		case 5: *p_scale = 15; break;
		case 4: *p_scale = 16; break;
		case 3: *p_scale = 17; break;
		case 2: *p_scale = 18; break;
		case 1: *p_scale = 19; break;
		case 0: *p_scale = 20; break;
		default: break;
			
	}

	return;

}

void ch7036_set_rotate (OUTPUT_INFO* pOutput_Info)
{
	
	
	pOutput_Info->rotate = DEFAULT_ROTATE; 
}

void ch7036_set_hflip (OUTPUT_INFO* pOutput_Info)
{
	
	
	pOutput_Info->h_flip = DEFAULT_HFLIP; 
}
void ch7036_set_vflip (OUTPUT_INFO* pOutput_Info)
{
	
	
	pOutput_Info->v_flip = DEFAULT_VFLIP; 
}

void ch7036_set_text_enhancement (ch7036_device_context_t *p_ctx, uint8 val)
{
	
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	PREFER_INFO* pPrefer_Info = p_ch7xxx_context->pPrefer_Info;
	

	PD_DEBUG("ch7036_set_text_enhancement- enter\n");


	I2CWrite(p_ch7xxx_context,0x03, 0x00);
	I2CWrite(p_ch7xxx_context,0x2E, (I2CRead(p_ch7xxx_context,0x2E) & 0xF8) |  val);
	
	pPrefer_Info->text_enhancement = val; 
	iic_write_ex(TXTEN, val);
}


void ch7036_set_pll_refdly(PREFER_INFO* pPrefer_Info)
{
	pPrefer_Info->pll_ref_dly = DEF_PLL_REF_DLY;
}

void ch7036_set_pll_fbdly(PREFER_INFO* pPrefer_Info)
{
	pPrefer_Info->pll_ref_fbdly = DEF_PLL_REF_FBDLY;
}

void ch7036_set_lvds_txdrv_ctrl(PREFER_INFO* pPrefer_Info)
{
	pPrefer_Info->lvds_txdrv_ctrl = DEF_LVDS_TXDRV_CTRL;
}



ch7036_status_t ch7036_alter_display_channel (ch7036_device_context_t *p_ctx) 
{


	ch7036_status_t status;


	pd_list_entry_attr_t  *list_item;

	ch7036_edid_blk_t* p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;
	ch7036_edid_blk_t* p_cedid = (ch7036_edid_blk_t *)p_ctx->cedid;


	list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DISPLAY,
					PD_GET_ATTR_LIST_ENTRY); 
	
	PD_DEBUG("ch7036_alter_display_channel=> enter- list_item->value [0x%lu]\n",list_item->value);
	
	switch (p_ctx->hpd & 0x22) {
	
		case 0x22: 
		case 0x20:
			PD_DEBUG("ch7036_alter_display_channel=> hdmi-dvi hpd status: attached... \n");
			if(p_ctx->hpd & 0x41 ) { 

				status = ch7036_read_edid(p_ctx, CHANNEL_LVDS_HDMI);

		
				if (status == SS_SUCCESS) {

					if( p_ctx->man_sel_out==1 ) 
						p_ctx->hpd &= 0x3F; //need parsing but dont do edid again
					else													
						p_ctx->hpd &= 0x7F; 
					
					
					PD_DEBUG("ch7036_alter_display_channel=> hdmi-dvi, status changed,edid read is a success- proceed to alter attr table...\n");
					
				}
				else { 

					PD_DEBUG("ch7036_alter_display_channel=> hdmi-dvi, status changed,edid read failed- use default attr table- show all modes..\n");
					

				}

					

			}
			else {

				PD_DEBUG("ch7036_alter_display_channel=> hdmi/dvi, status unchanged,check if edid read needed...\n"); 
				ch7036_redo_edid_if_needed(p_ctx, (void *)p_hedid, CHANNEL_LVDS_HDMI);
			}

	


			if(list_item->value == CHANNEL_AUTO_DETECT) {
				if(p_ctx->prev_outchannel != CHANNEL_LVDS_HDMI)	
					ch7036_set_output_channel((void *)p_ctx, CHANNEL_LVDS_HDMI);
			}
			else { 

	

				
				if(list_item->value & CHANNEL_DVI) 
					//	map DVI to HDMI output channel;										
					ch7036_set_output_channel((void *)p_ctx, (list_item->value & 0x01) | CHANNEL_HDMI);
				
				else 
					
					ch7036_set_output_channel((void *)p_ctx, (uint32)list_item->value );
				

			}
	

			break;

		case 0x02: 
			
			PD_DEBUG("ch7036_alter_display_channel=> crt hpd status: attached... \n");
		
			if(p_ctx->hpd & 0x05) { 
				
				status = ch7036_read_edid(p_ctx, CHANNEL_LVDS_VGA);

			
				if (status == SS_SUCCESS) {

					PD_DEBUG("ch7036_alter_display_channel=> crt only, status changed, edid is a success, proceed ..\n");
					if( p_ctx->man_sel_out==1 ) 
						p_ctx->hpd &= 0xF3; //need parsing,hpd manually updated to 'unchanged',dont do edid again
					else
						p_ctx->hpd &= 0xF7; 
										
				
				}
				else {

					PD_DEBUG("ch7036_alter_display_channel=> crt only, status changed, edid failed- use default attr table- show all modes..\n");

				} 
							

			} 
			
			else  {
				PD_DEBUG("ch7036_alter_display_channel=> crt only, status unchange,check if edid read needed...\n"); 
				ch7036_redo_edid_if_needed(p_ctx, (void *)p_cedid, CHANNEL_LVDS_VGA);
			}
				
			if(list_item->value == CHANNEL_AUTO_DETECT)
					ch7036_set_output_channel((void *)p_ctx, CHANNEL_LVDS_VGA);
			else {

				//set output channel, correct display choice if needed 				
				ch7036_set_output_channel((void *)p_ctx, (uint32)list_item->value );
			}
			

			break;

		default: 

			if(list_item->value == CHANNEL_AUTO_DETECT) {
				if (!p_ctx->init_done) { 					
					ch7036_set_output_channel((void *)p_ctx, CHANNEL_LVDS_VGA); 
				}
				else
					ch7036_set_output_channel((void *)p_ctx, CHANNEL_LVDS);
			}
			else 		
				ch7036_set_output_channel((void *)p_ctx, (uint32)list_item->value );
			

		break;


	} 

	PD_DEBUG("ch7036_alter_display_channel=> exit- list_item->value [0x%x]\n",list_item->value);
	return SS_SUCCESS;

}


pd_list_entry_attr_t *ch7036_get_updated_display_ouput_entry(ch7036_device_context_t *p_ctx, uint32 channel)
{
		
	pd_list_entry_attr_t  *list_item;

	list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DISPLAY,
					PD_GET_ATTR_LIST_ENTRY); 

	list_item->value = (list_item->value & 0x01) | channel; 

	return list_item;			
}


void ch7036_redo_edid_if_needed(ch7036_device_context_t *p_ctx, void* p_edidblk, uint32 channel)
{
	ch7036_edid_blk_t* p_edid = (ch7036_edid_blk_t *)p_edidblk;
	ch7036_status_t status = SS_UNSUCCESSFUL;;
	
	
	
	if (!p_ctx->init_done) { 
					
			PD_DEBUG("ch7036_alter_list: ch7036_check_if_edid_read_needed=> attached, status unchanged at init, read edid NOW...\n"); 
			status = ch7036_read_edid(p_ctx, channel); 
			
	}
	else {
				
		if(!p_edid->is_edid) { 
			PD_DEBUG("ch7036_alter_list: ch7036_check_if_edid_read_needed=>attached, status unchanged, never read edid or edid failed, read it NOW..\n");
			status= ch7036_read_edid(p_ctx, channel);


		}
		else 
			PD_DEBUG("ch7036_alter_list: ch7036_check_if_edid_read_needed=> attached, status unchanged, edid read was done, edid read is NOT needed...\n");
	}

	if(status == SS_SUCCESS) { 

		if(channel & CHANNEL_HDMI) 
			if( p_ctx->man_sel_out==1 ) 
				p_ctx->hpd &= 0x3F; 
			else
				p_ctx->hpd &= 0x7F;
		
		else 
			if( p_ctx->man_sel_out==1 ) 
				p_ctx->hpd &= 0xF3; 
			else
				p_ctx->hpd &= 0xF7;
		
	} 



}


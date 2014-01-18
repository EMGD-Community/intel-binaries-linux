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
* @file  ch7036_port.c
* @version 1.2.5
*-----------------------------------------------------------------------------
*/


#include <linux/kernel.h>

#include "ch7036_port.h"


#ifdef T_LINUX
	#include "lvds/lvds.h"
#else
	#include "lvds.h"
#endif



static pd_version_t  g_ch7036_version = {1, 2, 5, 0};
static unsigned long g_ch7036_dab_list[] = {0xEC,PD_DAB_LIST_END};


void ch7036_update_position(ch7036_device_context_t *p_ctx, OUTPUT_INFO* pOutput_Info);
int ch7036_initialize_device(ch7036_device_context_t *p_ctx);


static pd_driver_t	 g_ch7036_drv = {
	PD_SDK_VERSION,
	"Chrontel CH7036 Port Driver",
	0,
	&g_ch7036_version,
	PD_DISPLAY_LVDS_INT,
	PD_FLAG_UP_SCALING,
	g_ch7036_dab_list,
	100,
	ch7036_validate,
	ch7036_open,
	ch7036_init_device,
	ch7036_close,
	ch7036_set_mode,
	ch7036_post_set_mode,
	ch7036_set_attributes,
	ch7036_get_attributes,
	ch7036_get_timing_list,
	ch7036_set_power,
	ch7036_get_power,
	ch7036_save,
	ch7036_restore,
	ch7036_get_port_status
};


extern established_timings_t et_I[8];
extern established_timings_t et_II[8];
extern established_timings_t et_man;


int PD_MODULE_INIT(ch7036_init, (void *handle))
{
	int status;

	PD_DEBUG("ch7036: ch7036_init()\n");

	status = pd_register(handle, &g_ch7036_drv);
	if (status != PD_SUCCESS) {
		PD_DEBUG("ch7036: Error ! ch7036_init: pd_register() failed with "
				  "status=%#x\n", status);  
	}
	return status;
}



int PD_MODULE_EXIT(ch7036_exit, (void))
{
	PD_DEBUG("ch7036: ch7036_exit()\n");

	return (PD_SUCCESS);
} 


unsigned long ch7036_validate(unsigned long cookie)
{
	PD_DEBUG("ch7036: ch7036_validate()\n");
	
	return cookie;
}


int ch7036_open(pd_callback_t *p_callback, void **pp_context)
{
	uint8 device_ID;
	ch7036_device_context_t* p_ctx;
	DEV_CONTEXT* p_ch7xxx_context;

	ch7036_edid_blk_t *p_edid1, *p_edid2;

		
	int ret;


	
	PD_DEBUG("ch7036: ch7036_open()- enter- ch7036 pd release- major [%x] minor [%x] patch [%x]\n",
		g_ch7036_version.major,g_ch7036_version.minor,g_ch7036_version.patch);
	
	ret = PD_INTERNAL_LVDS_MODULE_OPEN(ch7036_lvds_open,(p_callback, pp_context));
	if ( ret != PD_SUCCESS)
	{


		PD_ERROR("ch7036: ch7036_open: EXIT#1\n");
		return ret;	
	}
	
	p_ctx = pd_malloc(sizeof(ch7036_device_context_t));
	if (p_ctx == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed\n");
		goto exit6;
	}

	pd_memset(p_ctx, 0, sizeof(ch7036_device_context_t));
	
	/* per EMGD request */
	p_edid1 = (ch7036_edid_blk_t *)p_ctx->cedid;
	p_edid2 = (ch7036_edid_blk_t *)p_ctx->hedid;


	p_ctx->fw = (FW7036_CFG *)(pd_malloc(sizeof(FW7036_CFG)));
	
	if (p_ctx->fw == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating FW7036_CFG struct\n");
		goto exit5;
	}

#if 0	
	ret = PD_INTERNAL_LVDS_MODULE_OPEN(ch7036_lvds_open,(p_callback, pp_context));
	if ( ret != PD_SUCCESS)
	{
		pd_free(p_ctx->fw);
		pd_free(p_ctx);		
		return ret;	
	}
#endif
	
	p_ctx->internal_lvds = *pp_context; 

	p_ctx->p_callback = p_callback;

#ifdef LVDS_ONLY
	*pp_context = (void *)p_ctx;
	return (PD_SUCCESS);
#endif

	
	
	p_ctx->p_ch7xxx_context = pd_malloc(sizeof(DEV_CONTEXT));
	if (p_ctx->p_ch7xxx_context == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating DEV_CONTEXT struct");
		goto exit4;
	}
	
	p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	
	p_ch7xxx_context->pd_context = (void *)p_ctx;


	
	I2CWrite(p_ch7xxx_context,0x03, 0x04); 
	
	device_ID = I2CRead(p_ch7xxx_context,0x50); 	
  
	PD_DEBUG("ch7036: ch7036_open()- read device ID= 0x%.2X\n", device_ID);


	if(device_ID != 0x56)
	{
		PD_DEBUG("ch7036: ch7036_open()- device is NOT found...\n");

		if(p_ch7xxx_context->pd_context)
			p_ch7xxx_context->pd_context=NULL;
		pd_free(p_ch7xxx_context);
		if(p_ctx->internal_lvds) {
			p_ctx->internal_lvds=NULL;

		}
		pd_free(p_ctx->fw);
		pd_free(p_ctx);
		return PD_ERR_NODEV;
		
	}
	else
	{
		PD_DEBUG("ch7036: ch7036_open()- ch7036 device is found...\n");

		
	}

	p_ch7xxx_context->DeviceID = device_ID;

	ch7036_reset(p_ctx);
	pd_usleep(50); 	
		
	if (ch7036_load_firmware(p_ctx) != SS_SUCCESS)   { 
		p_ctx->use_firmware =0;
		p_ctx->cedid = NULL;
		p_ctx->hedid = NULL;
		
	}
	else {
		p_ctx->use_firmware =1;


		p_ctx->cedid = (ch7036_edid_blk_t *)pd_malloc(sizeof(ch7036_edid_blk_t));

		if (p_ctx->cedid == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating ch7036_edid_blk_t struct for crt");
			goto exit3;
		}

		p_edid1 = (ch7036_edid_blk_t *)p_ctx->cedid ;

		p_edid1->etiming_I = (established_timings_t *)pd_malloc(8 * sizeof(established_timings_t));
		if (p_edid1->etiming_I == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating established_timings_t struct I for crt");
			pd_free(p_ctx->cedid);
			goto exit3;
		}
		p_edid1->etiming_II = (established_timings_t *)pd_malloc(8 * sizeof(established_timings_t));
		if (p_edid1->etiming_II == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating established_timings_t struct II for crt");
			pd_free(p_edid1->etiming_I);
			pd_free(p_ctx->cedid);
			goto exit3;
		}

		p_ctx->hedid = (ch7036_edid_blk_t *)pd_malloc(sizeof(ch7036_edid_blk_t));

		if (p_ctx->hedid == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating ch7036_edid_blk_t struct for hdvi");
			pd_free(p_edid1->etiming_I);
			pd_free(p_edid1->etiming_II);
			pd_free(p_ctx->cedid);

			goto exit3;
		}

		p_edid2 = (ch7036_edid_blk_t *)p_ctx->hedid ;

		p_edid2->etiming_I = (established_timings_t *)pd_malloc(8 * sizeof(established_timings_t));
		if (p_edid2->etiming_I == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating established_timings_t struct I for crt");
			pd_free(p_edid1->etiming_I);
			pd_free(p_edid1->etiming_II);
			pd_free(p_ctx->cedid);
			pd_free(p_ctx->hedid);

			goto exit3;
		}

		p_edid2->etiming_II = (established_timings_t *)pd_malloc(8 * sizeof(established_timings_t));

		if (p_edid2->etiming_II == NULL) {
			PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating established_timings_t struct for hdvi");
			pd_free(p_edid1->etiming_I);
			pd_free(p_edid1->etiming_II);
			pd_free(p_ctx->cedid);
			pd_free(p_edid2->etiming_I);

			pd_free(p_ctx->hedid);

			goto exit3;
		}


	}

	
	pd_memset(p_edid1->etiming_I, 0, 8 *
			sizeof(established_timings_t));
	pd_memset(p_edid1->etiming_II, 0, 8 *
			sizeof(established_timings_t));

	pd_memset(p_edid2->etiming_I, 0, 8 *
			sizeof(established_timings_t));
	pd_memset(p_edid2->etiming_II, 0, 8 *
			sizeof(established_timings_t));






	
	p_ch7xxx_context->pInput_Info = pd_malloc(sizeof(INPUT_INFO));
	if (p_ch7xxx_context->pInput_Info == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating INPUT_INFO struct");

		goto exit23; 
	}

	p_ch7xxx_context->pOutput_Info = pd_malloc(sizeof(OUTPUT_INFO));
	if (p_ch7xxx_context->pOutput_Info == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating OUTPUT_INFO struct");

		goto exit2;
	}

	p_ch7xxx_context->pPrefer_Info = pd_malloc(sizeof(PREFER_INFO));
	if (p_ch7xxx_context->pPrefer_Info == NULL) {
		PD_ERROR("ch7036: Error ! ch7036_open: pd_malloc() failed allocating PREFER_INFO struct");
		
		goto exit1;
	}

	


	p_ctx->p_ch7036_attr_table = NULL;
	
	if( ch7036_init_attribute_table(p_ctx, NULL) == SS_MEM_ALLOC_ERR)
	{
		pd_free(p_ch7xxx_context->pPrefer_Info);
		goto exit1;
	}

	
	ch7036_initialize_device(p_ctx);
	
	g_ch7036_drv.type = PD_DISPLAY_LVDS_INT; 

	
	*pp_context = (void *)p_ctx;

	PD_DEBUG("ch7036: ch7036_open: EXIT w/ SUCCESS...ch7036 pd release- major [%x] minor [%x] patch [%x]\n",
		g_ch7036_version.major,g_ch7036_version.minor,g_ch7036_version.patch);
	return (PD_SUCCESS);

exit1:
	pd_free(p_ch7xxx_context->pOutput_Info);
exit2:
	pd_free(p_ch7xxx_context->pInput_Info);

exit23:
	pd_free(p_edid1->etiming_I);
	pd_free(p_edid1->etiming_II);
	pd_free(p_ctx->cedid);
	pd_free(p_edid2->etiming_I);
	pd_free(p_edid2->etiming_II);
	pd_free(p_ctx->hedid);

exit3:
	p_ch7xxx_context->pd_context=NULL; 
	pd_free(p_ch7xxx_context);
exit4:

	p_ctx->internal_lvds = NULL;
	pd_free(p_ctx->fw);
exit5:
	pd_free(p_ctx);
exit6:
	PD_ERROR("ch7036: ch7036_open: EXIT- PD_ERR_NOMEM\n");
	return PD_ERR_NOMEM;
}


int ch7036_init_device(void *p_context)
{
	ch7036_device_context_t* p_ctx  = (ch7036_device_context_t*)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	int ret;

	PD_DEBUG("ch7036: ch7036_init_device()-enter\n");

	p_ctx->init_done = 1;
	ret= PD_INTERNAL_LVDS_MODULE_INIT_DEVICE(ch7036_lvds_init_device, (p_ctx->internal_lvds));

	p_ctx->prev_outchannel = pOutput_Info->channel;
	PD_DEBUG("ch7036: ch7036_init_device()-p_ctx->prev_outchannel = pOutput_Info->channel = [0x%x]\n",pOutput_Info->channel);

	PD_DEBUG("ch7036: ch7036_init_device()-exit\n");
	return ret;
	
}



int ch7036_set_mode(void *p_context, pd_timing_t *p_mode, unsigned long flags)
{

	ch7036_device_context_t *p_ctx = (ch7036_device_context_t*)p_context;
	
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	INPUT_INFO* pInput_Info = p_ch7xxx_context->pInput_Info;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	PREFER_INFO* pPrefer_Info = p_ch7xxx_context->pPrefer_Info;
	int ret, channel_on=0;

	
	PD_DEBUG("ch7036: ch7036_set_mode()-enter\n");


	if (!p_ctx || !p_mode) {
		return (PD_ERR_NULL_PTR);
	}

	if (p_ch7xxx_context->DeviceID != 0x56)
	{
		return (PD_ERR_NULL_PTR);
	}

	PD_DEBUG("ch7036_set_mode: requested width = %u height = %u\n",
		p_mode->width, p_mode->height);

#ifndef LVDS_ONLY	
	if (
		(p_ctx->fp_width && (p_mode->width > p_ctx->fp_width)) ||
		(p_ctx->fp_height && (p_mode->height > p_ctx->fp_height))
		) {
		return PD_ERR_MODE_NOTSUPP;
	}
	
	if( (p_ctx->fp_width == 640) && (p_ctx->fp_height == 480) ) {
		p_ctx->downscaled[MODE_6x4_BYPASS] = 0;
	}
	else { 
		p_ctx->downscaled[MODE_6x4_BYPASS] = 1;
		if( (!p_ctx->dwnscal_bypass) || ((p_ctx->fp_width <= 800) && (p_ctx->fp_height <= 600) )  ) 
			p_ctx->downscaled[MODE_8x6_7x4_BYPASS] = 0;
		else
			p_ctx->downscaled[MODE_8x6_7x4_BYPASS] = 1;

	}

	if(pOutput_Info->channel  == CHANNEL_LVDS_HDMI_VGA_OFF) { 
		
		pOutput_Info->channel = p_ctx->prev_outchannel; //restore output channel before temp. power down
		channel_on =1;
	}
	
	
	if (pOutput_Info->channel == CHANNEL_LVDS) {  

		pOutput_Info->channel = CHANNEL_LVDS_HDMI;	//force both channel on before setting in/out timing

	}


	ch7036_set_input_timing_info(p_ctx,pInput_Info);
	ch7036_set_output_timing_info(p_ctx, pOutput_Info);
	ch7036_set_prefer_timing_info(p_ctx,pPrefer_Info);

	
	if(ch7036_device_prepare(p_ctx)== SS_UNSUCCESSFUL)
	{
		PD_DEBUG("ch7036_set_mode: ch7036_device_prepare()- NOT SUCCESS... ERROR CODE [%lu]\n", p_ctx->last_emsg);		
		return PD_ERR_UNSUCCESSFUL;  
	}

	
	ch7036_update_position(p_ctx, pOutput_Info);

	if(ch7036_device_config(p_ctx) == SS_UNSUCCESSFUL)
	{
		PD_DEBUG("ch7036_set_mode: ch7036_device_config()- NOT SUCCESS...ERROR CODE [%lu]\n", p_ctx->last_emsg);
		return PD_ERR_UNSUCCESSFUL; 
	}

	if(channel_on) {
		
		pOutput_Info->channel = CHANNEL_LVDS_HDMI_VGA_OFF; //now, power down if it's tmp. powered up
		
	}

#endif

	ret = PD_INTERNAL_LVDS_MODULE_SET_MODE(ch7036_lvds_set_mode,(p_ctx->internal_lvds,p_mode,flags));
	if(ret != PD_SUCCESS)
		return ret; 

	
	return PD_SUCCESS;
}
//thua- 10/24/11- when user switches back and forth from VGA to HDMI or vice versa
//need to set h/v pos. accordingly
void ch7036_update_position(ch7036_device_context_t *p_ctx, OUTPUT_INFO* pOutput_Info)
{
	
	PD_DEBUG("ch7036: ch7036_update_position()- enter\n");

	
	if(pOutput_Info->channel & CHANNEL_HDMI) {	 	
		pOutput_Info->h_position = DEFAULT_POSITION; 
		pOutput_Info->v_position = DEFAULT_POSITION;
				
	}

	else { //pOutput_Info->channel & CHANNEL_VGA; note that CHANNEL_LVDS would never come here
		
			pOutput_Info->h_position = (uint16)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_HPOSITION,
					PD_GET_ATTR_LIST)->current_value;

			pOutput_Info->v_position = (uint16)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_VPOSITION,
					PD_GET_ATTR_LIST)->current_value;
		
	}

	PD_DEBUG("ch7036: ch7036_update_position()- h pos [%d]\n",pOutput_Info->h_position);
	PD_DEBUG("ch7036: ch7036_update_position()- v pos [%d]\n",pOutput_Info->v_position);

	return;
}


int ch7036_post_set_mode(void *p_context, pd_timing_t *p_mode,
						  unsigned long flags)
{
	

	ch7036_device_context_t* p_ctx  = (ch7036_device_context_t*)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	int ret;
	
	
	
	

	if (!p_ctx || !p_mode ) {
		return (PD_ERR_NULL_PTR);
	}

	if (p_ch7xxx_context->DeviceID != 0x56)
	{
		return (PD_ERR_NULL_PTR);
	}

#ifndef LVDS_ONLY

	
		
	ch7036_set_output_channel(p_ctx, p_ctx->prev_outchannel);	
	PD_DEBUG("ch7036_post_set_mode- now, current pOutput_Info->channel is [%x]\n",pOutput_Info->channel);

	if(ch7036_device_start(p_ctx) == SS_UNSUCCESSFUL)
	{
		PD_DEBUG("ch7036_post_set_mode: ch7036_device_start()- NOT SUCCESS\n");
		return PD_ERR_UNSUCCESSFUL; 
	}

	
	


#endif

	/* Fix backlight blinking during mode switching 
	 * in and out of lvds.
	 */		
	I2CWrite(p_ch7xxx_context,0x03,0x00);
	I2CWrite(p_ch7xxx_context,0x4E, I2CRead(p_ch7xxx_context,0x4E) & 0x7F); 

	I2CWrite(p_ch7xxx_context,0x4D,0x0);/*need to use PWM to control backlight-6/4/12*/

	ret = PD_INTERNAL_LVDS_MODULE_POST_SET_MODE(ch7036_lvds_post_set_mode,(p_ctx->internal_lvds,p_mode,flags));

#ifndef LVDS_ONLY	
	if(ret != PD_SUCCESS)
		return ret; 
	else
	{
		
		ch7036_reset_datapath(p_ch7xxx_context);
		pd_usleep(50); 

			
		ch7036_device_set_power(p_ctx, pOutput_Info->channel);	
		
		p_ctx->prev_outchannel = pOutput_Info->channel; 
	}

#endif

	/* for CH7036: Code fix Meego hang after killall X */	
	return p_ctx->pwr_state = PD_POWER_MODE_D0;
}





int ch7036_close(void *p_context)
{
	
	ch7036_device_context_t* p_ctx  = (ch7036_device_context_t*)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	ch7036_edid_blk_t *p_edid1, *p_edid2;

	p_edid1 = (ch7036_edid_blk_t *)p_ctx->cedid;
	p_edid2 = (ch7036_edid_blk_t *)p_ctx->hedid;


	PD_DEBUG("ch7036: ch7036_close()\n");

#ifndef LVDS_ONLY
	
	ch7036_set_power(p_context, PD_POWER_MODE_D3);	
#endif

	PD_INTERNAL_LVDS_MODULE_CLOSE(ch7036_lvds_close, (p_ctx->internal_lvds));

	if (p_ctx!= NULL) 
	{
		
		if(p_ctx->p_ch7xxx_context) {

			pd_free(p_ch7xxx_context->pInput_Info);
			p_ch7xxx_context->pInput_Info = NULL;

			pd_free(p_ch7xxx_context->pOutput_Info);
			p_ch7xxx_context->pOutput_Info = NULL;

			pd_free(p_ch7xxx_context->pPrefer_Info);
			p_ch7xxx_context->pPrefer_Info = NULL;

			pd_free(p_ctx->p_ch7xxx_context);
			p_ch7xxx_context = NULL;
		}

		
		if(p_edid1) {
			pd_free(p_edid1->etiming_I);
			pd_free(p_edid1->etiming_II);
			pd_free(p_ctx->cedid);

			p_edid1->etiming_I=NULL;
			p_edid1->etiming_II=NULL;
			p_ctx->cedid = NULL;

		}
		if(p_edid2) {
			pd_free(p_edid2->etiming_I);
			pd_free(p_edid2->etiming_II);
			pd_free(p_ctx->hedid);

			p_edid2->etiming_I=NULL;
			p_edid2->etiming_II=NULL;
			p_ctx->hedid = NULL;

		}

		if ( p_ctx->p_ch7036_attr_table) {
			pd_free(p_ctx->p_ch7036_attr_table);
			p_ctx->p_ch7036_attr_table = NULL;
			p_ctx->ch7036_num_attrs = 0;
		}

		if(p_ctx->fw) {
			pd_free(p_ctx->fw);
			p_ctx->fw = NULL;
		}


		pd_free(p_ctx);
		p_ctx = NULL;
	}
	
	return PD_SUCCESS;
}


int ch7036_get_timing_list(void *p_context, pd_timing_t *p_in_list,
	pd_timing_t **pp_out_list)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
		
	int ret = 0;
	

#ifndef T_PANEL_NATIVE_DTD
	int i;
	pd_timing_t * p_table;
	lvds_context_t * p_lvds = (lvds_context_t *)(p_ctx->internal_lvds);

#endif

	pd_port_status_t port_status;



	PD_DEBUG("ch7036: ch7036_get_timing_list()-enter\n");

	if (p_ch7xxx_context->DeviceID != 0x56)
	{
		return (PD_ERR_NODEV);
	}

#ifdef LVDS_ONLY
	PD_DEBUG("NUHAIRI: p_ctx->internal_lvds = %lu\n", p_ctx->internal_lvds);
	if (p_ctx->internal_lvds != NULL){
		PD_DEBUG("NUHAIRI: p_ctx->internal_lvds = %lu\n", p_ctx->internal_lvds);
		return PD_INTERNAL_LVDS_MODULE_GET_TIMING_LIST(ch7036_get_timing_list, (p_ctx->internal_lvds,p_in_list,pp_out_list));
	}
	return PD_SUCCESS;
#endif


	
	if ( (g_ch7036_drv.type & PD_DISPLAY_LVDS_INT) || (g_ch7036_drv.type & PD_DISPLAY_LVDS_LHDV) )
	{
		ret = PD_INTERNAL_LVDS_MODULE_GET_TIMING_LIST(ch7036_lvds_get_timing_list, (p_ctx->internal_lvds,p_in_list,pp_out_list));

		if(((lvds_context_t *)(p_ctx->internal_lvds))->native_dtd == 0 )
			return PD_ERR_NO_TIMINGS;

		p_ctx->p_lvds_table = *pp_out_list; 

#ifdef T_PANEL_NATIVE_DTD	

		
		pd_memcpy(&(p_ctx->native_dtd),((lvds_context_t *)(p_ctx->internal_lvds))->native_dtd,sizeof(pd_timing_t));

		p_ctx->fp_width = ((lvds_context_t *)(p_ctx->internal_lvds))->fp_width;
		p_ctx->fp_height = ((lvds_context_t *)(p_ctx->internal_lvds))->fp_height;	
		
		if( (p_ctx->fp_width == 0) || (p_ctx->fp_height == 0) ) 
			return PD_ERR_NO_TIMINGS ;
			
#else
		
		for(i=0,p_table = *pp_out_list;i< 30;i++) 
			{
				
				if((p_table->width == 1024) && (p_table->height == 768 )&& 
					(p_table->refresh == 60) )
				{
				
				

					pd_memcpy(&(p_ctx->native_dtd),p_table,sizeof(pd_timing_t));
					p_ctx->fp_width = p_table->width;
					p_ctx->fp_height = p_table->height;
					
					break;
				}
				
				p_table= (pd_timing_t*)((uint8*)p_table + (sizeof(pd_timing_t)+4)); 
							
			}

	
			pd_memcpy(p_lvds->native_dtd,&(p_ctx->native_dtd),sizeof(pd_timing_t));
			p_lvds->fp_width = p_ctx->fp_width;
			p_lvds->fp_height = p_ctx->fp_height;

#endif
			
	} 
	


	if ( !p_ctx->init_done) {  
		PD_DEBUG("ch7036: ch7036_get_timing_list()-init is not done- inquire port status...\n");		
		ch7036_get_port_status((void *)p_ctx, &port_status);

	}

	
	ch7036_parse_edid(p_ctx);

	return ret;

}



int ch7036_get_attributes(void *p_context, unsigned long *p_num_attr,
	pd_attr_t **pp_list)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
		
	

	PD_DEBUG("ch7036: ch7036_get_attributes()-enter\n");

	
	
	if (!p_ctx || !p_num_attr || !pp_list) {
		return PD_ERR_NULL_PTR;
	}
	
	*pp_list = p_ctx->p_ch7036_attr_table;

	
	*p_num_attr  = p_ctx->ch7036_num_attrs;

	PD_DEBUG("ch7036: ch7036_get_attributes()- total num_attrs = [%u]\n",*p_num_attr);

	return PD_SUCCESS;
}


int ch7036_set_attributes(void *p_context, unsigned long num_attrs,
	pd_attr_t *p_list)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = p_ch7xxx_context->pOutput_Info;
	pd_list_entry_attr_t* list_item;

	pd_port_status_t port_status;

	pd_attr_t        *p_curr, *p_attr;
	int ret;
	unsigned long i,temp=0;
	uint32 temp_chan;
	ch7036_status_t status;

	
	PD_DEBUG("ch7036: ch7036_set_attributes()-enter: num_attrs=%u\n", num_attrs);
	
	
	ret = PD_INTERNAL_LVDS_MODULE_SET_ATTRIBUTES(ch7036_lvds_set_attrs, (p_ctx->internal_lvds,num_attrs,p_list));

	
	
	if(ret != PD_SUCCESS)
		return ret;

#ifdef LVDS_ONLY
		return PD_SUCCESS;
#endif	
		
	
	if (!p_ctx->init_done) {

		PD_DEBUG("ch7036: ch7036_set_attributes()- at bootup...\n");

		PD_DEBUG("ch7036: ch7036_set_attributes()- p_ctx->hpd [%x]\n", p_ctx->hpd);

		p_attr = pd_get_attr(p_list, num_attrs, PD_ATTR_ID_DISPLAY, 0);
		if (p_attr && (p_attr->flags & PD_ATTR_FLAG_VALUE_CHANGED) ) {
			
		
			pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DISPLAY,
				PD_GET_ATTR_LIST)->current_value
			= p_attr->current_value; 


		}

		
		p_attr = pd_get_attr(p_list, num_attrs, PD_ATTR_ID_HDMI_OUT_MODE, 0);
		p_curr = pd_get_attr(p_ctx->p_ch7036_attr_table, num_attrs, PD_ATTR_ID_HDMI_OUT_MODE, 0);

		if (p_attr && (p_attr->flags & PD_ATTR_FLAG_VALUE_CHANGED) ) {
					
			if(p_curr) {
				pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_HDMI_OUT_MODE,
					PD_GET_ATTR_LIST)->current_value
				= p_attr->current_value;
				p_curr->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE; 
			}


		}
		
						
		p_attr = pd_get_attr(p_list, num_attrs, PD_ATTR_ID_DVI_OUT_MODE, 0);
		p_curr = pd_get_attr(p_ctx->p_ch7036_attr_table, num_attrs, PD_ATTR_ID_DVI_OUT_MODE, 0);

		if (p_attr && (p_attr->flags & PD_ATTR_FLAG_VALUE_CHANGED) ) {
			

			
			if(p_curr) {
				pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DVI_OUT_MODE,
					PD_GET_ATTR_LIST)->current_value
				= p_attr->current_value;
				p_curr->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE; 
			}


		}
	
		
		
		
		p_attr = pd_get_attr(p_list, num_attrs, PD_ATTR_ID_CRT_OUT_MODE, 0);
		p_curr = pd_get_attr(p_ctx->p_ch7036_attr_table, num_attrs, PD_ATTR_ID_CRT_OUT_MODE, 0);

		if (p_attr && (p_attr->flags & PD_ATTR_FLAG_VALUE_CHANGED) ) {
			
			if(p_curr) {
				pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_CRT_OUT_MODE,
						PD_GET_ATTR_LIST)->current_value
					= p_attr->current_value;

				p_curr->flags &= ~PD_ATTR_FLAG_USER_INVISIBLE; 
			}


		}
		
	
		
	} 

	
	for (i = 0, p_attr = p_list; i < num_attrs; i++,p_attr++) 
	{
		
		
		if (!(p_attr->flags & PD_ATTR_FLAG_VALUE_CHANGED)) {

			continue;	
		}

		
		p_attr->flags &= ~PD_ATTR_FLAG_VALUE_CHANGED;
	
		if (p_attr->flags & PD_ATTR_FLAG_USER_INVISIBLE)
			continue;

		
		if( (p_attr->id == 0x1A) || (p_attr->id == 0x1B) ||
			(p_attr->id == 0x3C) || (p_attr->id == 0x46) ||
			(p_attr->id == 0x47) )
			continue;
		
#if 0		
		
		p_curr = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,p_attr->id,
				PD_GET_ATTR_LIST); 
#endif 
		
		
		p_curr = pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,p_attr->id,
				0); 
		PD_DEBUG("ch7036_set_attributes(): attribute changed is of type [%ld] name [%s] id [%ld]\n",p_attr->type, p_attr->name, p_attr->id);

		PD_DEBUG("ch7036_set_attributes():current value [%ld] requested value [%ld]\n",p_curr->current_value, p_attr->current_value); 	
			
		
		temp = p_curr->current_value; 
		p_curr->current_value = p_attr->current_value;
		switch (p_attr->id) {
			case PD_ATTR_ID_DISPLAY:
				
				list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DISPLAY,
							PD_GET_ATTR_LIST_ENTRY); 			
				
				p_attr->flags |= PD_ATTR_FLAG_SETMODE;  

				if(list_item->value == CHANNEL_AUTO_DETECT) { 
					p_ctx->man_sel_out = 0;

					if(p_ctx->init_done) { 
						p_ctx->hpd |= CH7036HPD_RESERVED1; //force port status inquiry
						ch7036_get_port_status((void *)p_ctx, &port_status);
	
					}
					break; 
				}
				//end of CHANNEL_AUTO_DETECT, manual selection of display output begins
				
				
				p_ctx->man_sel_out = 1; 

				//1- save a copy
				temp_chan = p_ctx->prev_outchannel;
				p_ctx->prev_outchannel = pOutput_Info->channel;

				PD_DEBUG("ch7036_set_attributes():current list item value [0x%x]\n",list_item->value);

				//2- get requested output channel- assume it's allowed		

				if(list_item->value & CHANNEL_DVI) { 					
					pOutput_Info->channel = (list_item->value & 0x01) | CHANNEL_HDMI;
				}
				else	 	
					pOutput_Info->channel = list_item->value;

				PD_DEBUG("ch7036_set_attributes():current output channel value [0x%x]\n",pOutput_Info->channel);

				//3- check requested selection vs what is available
				status = ch7036_get_attached_device(p_ctx); //manual mode- verify its selection and correct if needed
				
				//4- NOT allowed display choice- also include when none is connected
				if( status == SS_DISPLAY_CHOICE_NOT_ALLOWED) {  //restore previous states
					PD_DEBUG("ch7036_set_attributes(): display choice is not allowed- restore prev. states...\n");
							
					p_ctx->hpd &= 0xEF;	 //reset												
					
					if(p_ctx->init_done){ 																		
						pOutput_Info->channel = p_ctx->prev_outchannel;
						p_ctx->prev_outchannel = temp_chan; 
						p_curr->current_value = temp; 
						//when system is restarted right after this point, need to provide main driver w/ a valid 
						//restored choice
						p_attr->current_value = p_curr->current_value;
						if(p_curr->current_value==1)
							p_ctx->man_sel_out = 0; 

					}
					else { //at installation, when init is not done, if manual choice is not valid, revert to auto
					
						p_curr->current_value = 1; 
						p_ctx->man_sel_out = 0; 
						
						if (pOutput_Info->channel & CHANNEL_HDMI) //DVI mapped to CHANNEL_HDMI
						{
							pOutput_Info->channel = (pOutput_Info->channel & 0x01) | CHANNEL_VGA;
							p_ctx->prev_outchannel = pOutput_Info->channel;
						
						} 
						else { //VGA
							pOutput_Info->channel = (pOutput_Info->channel & 0x01) | CHANNEL_HDMI;
							p_ctx->prev_outchannel = pOutput_Info->channel;
						}
						
						
					}

					PD_DEBUG("ch7036_set_attributes(): p_curr->current_value is: [%x]\n",p_curr->current_value);
					if(p_ctx->hpd == 0x50) {//none is attached
						pOutput_Info->channel &= CHANNEL_LVDS; 
						p_ctx->prev_outchannel = pOutput_Info->channel;
					}

										
				}
				PD_DEBUG("ch7036_set_attributes(): now- output display channel is: value [%ld]\n",pOutput_Info->channel);


				//special case channel CHANNEL_xxx_HDMI: 
				//incoming dvi format => convert to hdmi format- or- incoming hdmi format => convert to dvi format 
				if( ( (pOutput_Info->hdmi_fmt.is_dvi_mode==1) && ((list_item->value & 0x02) == CHANNEL_HDMI) ) ||
					( (pOutput_Info->hdmi_fmt.is_dvi_mode==0) && ((list_item->value & 0x08) == CHANNEL_DVI) )
					)
					p_ctx->hpd |= 0x40; //request edid read again to update hdmi/dvi format accordingly


				//base on 'new' attached info, read edid and set proper display output channel, including DVI
				//case: manual selection w/ hpd change
				if(p_ctx->hpd & 0x44) 	{		
					ch7036_alter_display_channel(p_ctx); 
				}
				
				if( (pOutput_Info->channel & 0x04) == CHANNEL_VGA || p_curr->current_value==1  /* Coerced Auto Detect */ ) 
					break;

				//case: 
				//channel CHANNEL_xxx_HDMI- update list item xxx_DVI or xxx_HDMI accordingly
				if((pOutput_Info->channel & CHANNEL_HDMI) && (pOutput_Info->hdmi_fmt.is_dvi_mode==1) )
					p_curr->current_value = (pOutput_Info->channel & 0x01) == CHANNEL_LVDS?3:6;//lvds-dvi:dvi
				
				else 
					if((pOutput_Info->channel & CHANNEL_HDMI) && (pOutput_Info->hdmi_fmt.is_dvi_mode==0) )
						p_curr->current_value = (pOutput_Info->channel & 0x01) == CHANNEL_LVDS?2:5;//lvds-hdmi:hdmi

				PD_DEBUG("ch7036_set_attributes(): after alter channel- output display channel is: value [%ld]\n",pOutput_Info->channel);

				break;

			case PD_ATTR_ID_HDMI_OUT_MODE:
				list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_HDMI_OUT_MODE,
							PD_GET_ATTR_LIST_ENTRY); 
				p_ctx->hdmi_mode_index = list_item->value;
				PD_DEBUG("ch7036_set_attributes(): updated hdmi_mode_index is: value [%ld]\n",p_ctx->hdmi_mode_index);
				p_attr->flags |= PD_ATTR_FLAG_SETMODE;
				break;
				
			case PD_ATTR_ID_DVI_OUT_MODE:
				list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_DVI_OUT_MODE,
							PD_GET_ATTR_LIST_ENTRY); 
				p_ctx->dvi_mode_index = list_item->value;
				PD_DEBUG("ch7036_set_attributes(): updated dvi_mode_index is: value [%ld]\n",p_ctx->dvi_mode_index);
				p_attr->flags |= PD_ATTR_FLAG_SETMODE;
				break;

			case PD_ATTR_ID_CRT_OUT_MODE:
				list_item = (pd_list_entry_attr_t *)pd_get_attr(p_ctx->p_ch7036_attr_table, p_ctx->ch7036_num_attrs,PD_ATTR_ID_CRT_OUT_MODE,
							PD_GET_ATTR_LIST_ENTRY); 
				p_ctx->crt_mode_index = list_item->value;
				PD_DEBUG("ch7036_set_attributes(): updated crt_mode_index is: value [%ld]\n",p_ctx->crt_mode_index);
				p_attr->flags |= PD_ATTR_FLAG_SETMODE;
				break;

			case PD_ATTR_ID_HPOSITION: //these attributes are for vga only
			case PD_ATTR_ID_VPOSITION:
			
				PD_DEBUG("ch7036_set_attributes(): set vga h/v position...\n");
				 
				//save vga h/v attribute context
				if(p_curr->current_value > ((pd_range_attr_t *)p_curr)->max)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->max;
				else if (p_curr->current_value < ((pd_range_attr_t *)p_curr)->min)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->min;
	
				
				if( pOutput_Info->channel & CHANNEL_HDMI)
					break;

				if (p_attr->id == PD_ATTR_ID_HPOSITION) 
					pOutput_Info->h_position = (uint16)(p_curr->current_value);
				else
					pOutput_Info->v_position = (uint16)(p_curr->current_value);
					
				ch7036_set_position(p_ctx, (uint8)p_attr->id, (uint16)(p_curr->current_value));
				
				PD_DEBUG("ch7036_set_attributes(): updated and set vga position: value [%d]\n",p_curr->current_value);
		
				break;
			case PD_ATTR_ID_HSCALE:
			case PD_ATTR_ID_VSCALE:
			case PD_ATTR_ID_HSCALE_CRT:
			case PD_ATTR_ID_VSCALE_CRT:
				
				if(p_curr->current_value > ((pd_range_attr_t *)p_curr)->max)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->max;
				else if (p_curr->current_value < ((pd_range_attr_t *)p_curr)->min)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->min;
				
			
				if(p_attr->id== PD_ATTR_ID_HSCALE || p_attr->id== PD_ATTR_ID_HSCALE_CRT)
					ch7036_set_scaling (pOutput_Info,PD_ATTR_ID_HSCALE, (uint8)p_curr->current_value);
				else 
					ch7036_set_scaling (pOutput_Info,PD_ATTR_ID_VSCALE, (uint8)p_curr->current_value);				
				PD_DEBUG("ch7036_set_attributes(): updated scale value is: value [%lu]\n",p_curr->current_value);
			
				p_attr->flags |= PD_ATTR_FLAG_SETMODE; 

				break;
			case PD_ATTR_ID_DITHER_BYPASS: 
				
			
				PD_DEBUG("ch7036_set_attributes(): updated quality enhance value is: value [%lu]\n",p_curr->current_value);
				ch7036_set_quality_enhancement(p_ctx,(uint8)p_curr->current_value);
				break;
			
			case PD_ATTR_ID_DITHER: 
				p_ctx->dither_select = (uint8)p_curr->current_value;
				PD_DEBUG("ch7036_set_attributes(): updated dither select value is: value [%lu]\n",p_curr->current_value);
				ch7036_set_dither(p_ctx);

				break;

			case PD_ATTR_ID_TEXT_FILTER:
				
				if(p_curr->current_value > ((pd_range_attr_t *)p_curr)->max)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->max;
				else if (p_curr->current_value < ((pd_range_attr_t *)p_curr)->min)
					p_curr->current_value = ((pd_range_attr_t *)p_curr)->min;
				
				PD_DEBUG("ch7036_set_attributes(): update text tuning value...\n");
				ch7036_set_text_enhancement (p_ctx, (uint8) p_curr->current_value);

				break;

			case PD_ATTR_ID_LOAD_FIRMWARE:
				PD_DEBUG("ch7036_set_attributes(): updated [reload-firmware] value is: value [%lu]\n",p_curr->current_value);

				break;

			case PD_ATTR_ID_DWNSCAL_BYPASS:
				PD_DEBUG("ch7036_set_attributes(): updated [dwnscal_bypass] value is: value [%lu]\n",p_curr->current_value);
				if(p_curr->current_value ) { 
					p_ctx->dwnscal_bypass = 1; 
					p_ctx->downscaled[MODE_8x6_7x4_BYPASS]=1;
				}
				else {
					p_ctx->dwnscal_bypass = 0; 
					p_ctx->downscaled[MODE_8x6_7x4_BYPASS]=0;
				}

				break;

			case PD_ATTR_ID_REFRESH:
				PD_DEBUG("ch7036_set_attributes(): refresh value is: value [%lu]\n",p_curr->current_value);
				
				if(p_curr->current_value ) 
						p_curr->current_value= 0; 
				
				if(p_ctx->init_done && !p_ctx->man_sel_out ) {

						p_ctx->hpd |= CH7036HPD_RESERVED1; //force port status inquiry- edid read bit

						ch7036_get_port_status((void *)p_ctx, &port_status);

						p_attr->flags |= PD_ATTR_FLAG_SETMODE; 					
					
				}
				break;

			default:
				
				PD_DEBUG("ch7036_set_attr(): unhandled attr name[%s]id[%ld]curr_index[%lu]\n",p_attr->name, p_attr->id,p_attr->current_value);
				break;

		}
		
	}

	PD_DEBUG("ch7036: ch7036_set_attributes()-p_ctx->man_sel_out at exit [0x%x]\n",p_ctx->man_sel_out);
	
	PD_DEBUG("ch7036: ch7036_set_attributes()-p_ctx->hpd at exit [0x%x]\n",p_ctx->hpd);

	PD_DEBUG("ch7036: ch7036_set_attributes()-exit\n");

	return ret;
}

int ch7036_set_power(void *p_context, unsigned long state)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
	OUTPUT_INFO* pOutput_Info = (p_ctx->p_ch7xxx_context)->pOutput_Info;


	int ret;


	PD_DEBUG("ch7036: ch7036_set_power()-enter: requested state=%x\n", state);

#ifdef LVDS_ONLY
	return	PD_INTERNAL_LVDS_MODULE_SET_POWER(ch7036_set_power, (p_ctx->internal_lvds,state));
#endif
		
	if (!p_ctx) 
		return PD_ERR_NULL_PTR;
	
	if (state > PD_POWER_MODE_D3)
		return PD_ERR_INVALID_POWER;
	
	if (state != PD_POWER_MODE_D0) {
	
		if(pOutput_Info->channel != CHANNEL_LVDS_HDMI_VGA_OFF)  
			p_ctx->prev_outchannel = pOutput_Info->channel; //store current output channel before temporarily powered down
		pOutput_Info->channel = CHANNEL_LVDS_HDMI_VGA_OFF; 

		
		ch7036_device_set_power(p_ctx,pOutput_Info->channel);
				
		ret = PD_INTERNAL_LVDS_MODULE_SET_POWER(ch7036_lvds_set_power, (p_ctx->internal_lvds,state));
		
	
	}
	else {
		

		ch7036_set_output_channel(p_ctx,p_ctx->prev_outchannel); //restore previous output channel

		PD_DEBUG("ch7036: ch7036_set_power()- p->ctx-hpd [0x%x]\n",p_ctx->hpd);
		PD_DEBUG("ch7036: ch7036_set_power()- requested output channel- [%x]\n", pOutput_Info->channel);


		PD_INTERNAL_LVDS_MODULE_SET_POWER(ch7036_lvds_set_power, (p_ctx->internal_lvds,state));
		
		ch7036_device_set_power(p_ctx,pOutput_Info->channel);
						
	}

	
	p_ctx->pwr_state = state;

	return PD_SUCCESS;
}


int ch7036_get_power(void *p_context, unsigned long *p_state)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
	

	PD_DEBUG("ch7036: ch7036_get_power()\n");

#ifdef LVDS_ONLY
	return PD_INTERNAL_LVDS_MODULE_GET_POWER(ch7036_get_power, (p_ctx->internal_lvds,p_state));
#endif

	*p_state = p_ctx->pwr_state;

	return PD_SUCCESS;
}

int ch7036_save(void *p_context, void **state, unsigned long flags)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;
	OUTPUT_INFO* pOutput_Info = (p_ctx->p_ch7xxx_context)->pOutput_Info;

	PD_DEBUG("ch7036: ch7036_save()\n"); 

#ifdef LVDS_ONLY
	
	return PD_INTERNAL_LVDS_MODULE_SAVE(ch7036_save,(p_ctx->internal_lvds, state, flags));
#endif

	//in Linux, when being called @ init, it incorrectly assigned unintialized global attribute value to prev outchannel	
	//p_ctx->prev_outchannel = ch7036_get_output_channel(p_context); 
	//fixed
	p_ctx->prev_outchannel = pOutput_Info->channel; 
	

	*state = NULL;

	return PD_SUCCESS;
}



int ch7036_restore(void *p_context, void *state, unsigned long flags)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)p_context;

	unsigned long i;

	

	PD_DEBUG("ch7036: ch7036_restore()\n");

#ifdef LVDS_ONLY
	
	return PD_INTERNAL_LVDS_MODULE_RESTORE(ch7036_restore,(p_ctx->internal_lvds, state, flags));
#endif

	/* for CH7036: Code fix Meego hang after killall X 
	*/
#ifdef T_LINUX
	if(p_ctx->pwr_state != PD_POWER_MODE_D1)
		return PD_SUCCESS;
#endif

	
	if (ch7036_load_firmware(p_ctx) != SS_SUCCESS)   { 
		PD_DEBUG("ch7036: ch7036_restore()-  load fw is NOT a SUCCESS\n");
		return PD_ERR_UNSUCCESSFUL;
		
	}
	else {
		PD_DEBUG("ch7036: ch7036_restore()-  load fw is a SUCCESS\n");
	}



	if(p_ctx->prev_outchannel == CHANNEL_LVDS_HDMI) {

		ch7036_set_output_channel(p_context, p_ctx->prev_outchannel);
		ch7036_set_mode(p_context, &(p_ctx->native_dtd), 0);  
		ch7036_post_set_mode(p_context, &(p_ctx->native_dtd), 0);
	}

	
	for(i=0;i<p_ctx->ch7036_num_attrs;i++) {

		if( p_ctx->p_ch7036_attr_table[i].id == PD_ATTR_ID_REFRESH ) {

			
			p_ctx->p_ch7036_attr_table[i].flags |= PD_ATTR_FLAG_VALUE_CHANGED;
			p_ctx->p_ch7036_attr_table[i].current_value = 1;

			ch7036_set_attributes(p_context, 1, &p_ctx->p_ch7036_attr_table[i]);
			break;
		}

	}

	
	return PD_SUCCESS;

}


int ch7036_get_port_status(void *context, pd_port_status_t *port_status)
{
	ch7036_device_context_t *p_ctx = (ch7036_device_context_t *)context;
	OUTPUT_INFO* pOutput_Info = (p_ctx->p_ch7xxx_context)->pOutput_Info;

#if 0
	FW7036_CFG* fv = (FW7036_CFG*) p_ctx->fw;
	int ret;
#endif
	
	port_status->display_type = PD_DISPLAY_LVDS_INT;
	port_status->connected    = PD_DISP_STATUS_UNKNOWN;
	
	
	PD_DEBUG("ch7036: ch7036_get_port_status()-enter... p_ctx->hpd [%x]\n", p_ctx->hpd);

#if 0
	PD_DEBUG("Get LHFM Version Information.\r\n");
	ret = LHFM_get_version(p_ch7xxx_context, fv);
	if (0==ret) {
		PD_DEBUG("Ma_ver=%d, mi_ver=%d, did=%02X, rid= %02X, capability=%s\r\n",
			fv->ver_major, fv->ver_minor, fv->did, fv->rid, (fv->capbility & 0x2) ? "EDID+HDCP" : "EDID"); 
	}
	else {
		PD_DEBUG("--- failed!\r\n");
		PD_DEBUG("status: [%s]\n",ret ==-1?"timeout!":"firmware_error!");
	}
#endif

	
#ifdef LVDS_ONLY
	
	return PD_INTERNAL_LVDS_MODULE_GET_PORT_STATUS(ch7036_get_port_status,(p_ctx->internal_lvds, port_status));
#endif
	
	if(! (p_ctx->hpd & CH7036HPD_RESERVED1) ) {  
		//note: main driver check port status several times, this block is to speed things up a little 
		if ((p_ctx->init_done) && (pOutput_Info->channel == p_ctx->prev_outchannel ) ) {
			if(p_ctx ->hpd & 0x22)
				port_status->connected = PD_DISP_STATUS_ATTACHED;
			PD_DEBUG("ch7036: ch7036_get_port_status()-output channel UNCHANGED- exit. p_ctx->hpd [%x]\n", p_ctx->hpd);
			return PD_SUCCESS;
		}

	}

	if(p_ctx->use_firmware) {

		ch7036_get_attached_device(p_ctx);		
		if(p_ctx ->hpd & 0x22)
			port_status->connected = PD_DISP_STATUS_ATTACHED;

		ch7036_alter_display_channel(p_ctx); 
			
		}

	
	p_ctx->hpd &= 0xEE; 
	
	PD_DEBUG("ch7036: ch7036_get_port_status()-exit. p_ctx->hpd [%x]\n", p_ctx->hpd);
	return PD_SUCCESS;
}




int ch7036_initialize_device(ch7036_device_context_t *p_ctx)
{
	DEV_CONTEXT* p_ch7xxx_context = p_ctx->p_ch7xxx_context;
	OUTPUT_INFO* pOutput_Info = (p_ctx->p_ch7xxx_context)->pOutput_Info;
	PREFER_INFO* pPrefer_Info = p_ch7xxx_context->pPrefer_Info;
	uint8 reg; //,i=0;

	ch7036_edid_blk_t* p_hedid; 
	ch7036_edid_blk_t* p_cedid ;


	PD_DEBUG("ch7036: ch7036_initialize_device()- ENTER...\n");
	

	p_ctx->init_done = 0;
	p_ctx->hpd = 0;	

	p_ctx->downscaled[MODE_6x4_BYPASS] = 1; 
	p_ctx->downscaled[MODE_8x6_7x4_BYPASS] = 1;
	p_ctx->dwnscal_bypass = 1; 

	if(p_ctx->use_firmware) {

		p_hedid = (ch7036_edid_blk_t *)p_ctx->hedid;
		p_cedid = (ch7036_edid_blk_t *)p_ctx->cedid;

		
		
		pd_memcpy(p_cedid->etiming_I,et_I,8* sizeof(established_timings_t));  
		pd_memcpy(p_cedid->etiming_II,et_II,8* sizeof(established_timings_t)); 
		
		
		p_cedid->etiming_man = &et_man;


		
		pd_memcpy(p_hedid->etiming_I,et_I,8* sizeof(established_timings_t));  
		pd_memcpy(p_hedid->etiming_II,et_II,8* sizeof(established_timings_t)); 

		p_hedid->etiming_man = &et_man;

		
		p_cedid->is_edid = 0;
		p_hedid->is_edid = 0;
		p_cedid->ebn = 0;
		p_hedid->ebn = 0;

		
		I2CWrite(p_ch7xxx_context,0x03, 0x04);
		reg = I2CRead(p_ch7xxx_context,0x52);
		reg = reg & 0xEF; 
		I2CWrite(p_ch7xxx_context,0x52, reg);


		I2CWrite(p_ch7xxx_context,0x03, 0x0);
		reg = I2CRead(p_ch7xxx_context,0x07);
		reg = reg & 0x70; 
		I2CWrite(p_ch7xxx_context,0x07, reg);

		/*Fix backlight blinking during mode switching in and out of lvds*/
		I2CWrite(p_ch7xxx_context,0x4E, I2CRead(p_ch7xxx_context,0x4E) & 0x7F); 
		I2CWrite(p_ch7xxx_context,0x4D,0x0);/*need to use PWM to control backlight-6/4/12*/

		I2CWrite(p_ch7xxx_context,0x03, 0x01);
		reg = I2CRead(p_ch7xxx_context,0x0F); 
		reg = reg & 0x7F; 
		I2CWrite(p_ch7xxx_context,0x0F, reg);
	
		I2CWrite(p_ch7xxx_context,0x03, 0x03);		
		reg = I2CRead(p_ch7xxx_context,0x6E);
		reg = reg & 0xBF; 
		I2CWrite(p_ch7xxx_context,0x6E, reg | 0x40);


	}

	
	pOutput_Info->hdmi_fmt.is_dvi_mode = 0;  
	
	pOutput_Info->channel = CHANNEL_LVDS | CHANNEL_HDMI;



	PD_DEBUG("ch7036: ch7036_initialize_device()- default output channel is [%u]\n",pOutput_Info->channel);


	
	pOutput_Info->h_position = DEFAULT_POSITION;
	pOutput_Info->v_position = DEFAULT_POSITION;


	p_ctx->dither_select = DITHER_18_TO_18; 
	p_ctx->man_sel_out= 0; 

	ch7036_set_scaling (pOutput_Info,PD_ATTR_ID_HSCALE, HDMI_DEFAULT_UNDERSCAN);
	ch7036_set_scaling (pOutput_Info,PD_ATTR_ID_VSCALE, HDMI_DEFAULT_UNDERSCAN);

	ch7036_set_prefer_timing_info(p_ctx,pPrefer_Info); 
	
	return PD_SUCCESS;
}



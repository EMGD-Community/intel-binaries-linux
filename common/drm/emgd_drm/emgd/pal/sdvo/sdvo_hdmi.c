/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_hdmi.c
 * $Revision: 1.7 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2010, Intel Corporation.
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
 * Description:
 *  Port driver HDMI interface functions
 *-----------------------------------------------------------------------------
 */

#include "sdvo_hdmi.h"

#if 0
 /*	HDMI Attributes
	Currently if omitting out this attributes as this should NOT be avalable
	to user
	TODO: Integrate this attributes into HDMI  */
/* HDMI attributes */
#define PD_HDMI_COLORIMETRY_256_RGB					0
#define PD_HDMI_COLORIMETRY_220_RGB					1
#define PD_HDMI_COLORIMETRY_422_YCRCB				2
#define PD_HDMI_COLORIMETRY_444_YCRCB				3

#define PD_HDMI_PIXEL_REPLICATION_1X_REPITION				1
#define PD_HDMI_PIXEL_REPLICATION_2X_REPITION				2
#define PD_HDMI_PIXEL_REPLICATION_4X_REPITION				4
/-----------------------------------------------------------------------------/
 {PD_ATTR_ID_COLORIMETRY, MAKE_NAME("Colorimetry"), {4}, {0x0F},
  GET_CURRENT_COLORIMETRY, SET_CURRENT_COLORIMETRY},

  /* 256 RGB is passthrough, real value is 0x01 but 0x00 works as well
	and upon boot up the value is 0x00 */
 {PD_HDMI_COLORIMETRY_256_RGB,
  MAKE_NAME("RGB 256 level"),							{0}, {0x00}, 0, 0},
 {PD_HDMI_COLORIMETRY_220_RGB,
  MAKE_NAME("RGB 220 level"),							{0}, {0x02}, 0, 0},
 {PD_HDMI_COLORIMETRY_422_YCRCB,
  MAKE_NAME("4:2:2 YcrCb"),							{0}, {0x04}, 0, 0},
 {PD_HDMI_COLORIMETRY_444_YCRCB,
  MAKE_NAME("4:4:4 YcrCb"),							{0}, {0x08}, 0, 0},

 {PD_ATTR_ID_PIXEL_REPLICATION, MAKE_NAME("Pixel Replication"), {3}, {0x1F},
  GET_PIXEL_REPLICATION, SET_PIXEL_REPLICATION},

 {PD_HDMI_PIXEL_REPLICATION_1X_REPITION,
  MAKE_NAME("No Repetition"),							{0}, {0x00}, 0, 0},
 {PD_HDMI_PIXEL_REPLICATION_2X_REPITION,
  MAKE_NAME("1X Repetition"),							{0}, {0x01}, 0, 0},
 {PD_HDMI_PIXEL_REPLICATION_4X_REPITION,
  MAKE_NAME("3X Repetition"),							{0}, {0x03}, 0, 0},
#endif
#if defined(SDVO_HDMI)

/*	============================================================================
	Function	:	sdvo_hdmi_transmitter

	Parameters	:

	Remarks 	:	Detect if transmitter is a HDMI transmitter.

	Returns 	:
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_transmitter(sdvo_device_context_t *p_ctx)
{
	return sdvo_get_supported_encoding_modes(p_ctx,&(p_ctx->hdmi.version));
}

/*	============================================================================
	Function	:	sdvo_hdmi_support is called to see if the transmitter
					and the monitor device support HDMI.

	Parameters	:	p_context: Pointer to port driver allocated context
								structure

	Remarks 	:

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_support(void *p_context)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status = SS_SUCCESS;

	/* Monitor support is determined by valid CEA 861 extension and */
	/* vendor block with 0xC03 IEEE regstration ID */
	/*
	if((p_ctx->p_callback->eld) == NULL){
		return SS_NOT_SUPPORTED;
	}

	if((*p_ctx->p_callback->eld)->vendor_block_size) {
		return SS_NOT_SUPPORTED;
	}

	if((*p_ctx->p_callback->eld)->vendor_ieee_id != CEA_IEEE_HDMI_ID) {
		return SS_NOT_SUPPORTED;
	}
	*/
	/* Transmitter support is determined by SDVO GetSupportDigitalEncodingModes */
	status = sdvo_hdmi_transmitter(p_ctx);
	if (status != SS_SUCCESS) {
		return status;
	}

	return SS_SUCCESS;
}



/*	============================================================================
	Function	:	sdvo_hdmi_audio_characteristic

	Parameters	:

	Remarks 	:	Set audio characteristic to be used in ELD calcilation.
					TODO: Make sure this is recalled if hotplug is detected

	Returns 	:
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_audio_characteristic(sdvo_device_context_t *p_ctx)
{
	sdvo_status_t status;
	i2c_reg_t hdmi_audio_char[3];

	/* Skip since no CEA data available */
	if((!p_ctx->p_callback) || ((p_ctx->p_callback->eld) == NULL)){
		return SS_NOT_SUPPORTED;
	}
	if(*(p_ctx->p_callback->eld) == NULL){
		return SS_NOT_SUPPORTED;
	}

	status = sdvo_get_hdmi_audio_transmission_char(p_ctx, hdmi_audio_char);
	if(status == SS_SUCCESS){
		(*p_ctx->p_callback->eld)->audio_flag |= PD_AUDIO_CHAR_AVAIL;
		(*p_ctx->p_callback->eld)->NPL = hdmi_audio_char[0];
		(*p_ctx->p_callback->eld)->K0	= hdmi_audio_char[1];
		(*p_ctx->p_callback->eld)->K1	= hdmi_audio_char[2];
	}else{
		(*p_ctx->p_callback->eld)->audio_flag &= ~PD_AUDIO_CHAR_AVAIL;
	}

	return SS_SUCCESS;
}

/*	============================================================================
	Function	:	sdvo_hdmi_configure is called to program AVI infoframes, SPD
					infoframes, ELD data for audio, colorimetry and pixel replication.

	Parameters	:	p_context: Pointer to port driver allocated context
								structure
					p_mode: Point to current video output display mode

	Remarks 	:

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_configure(sdvo_device_context_t *p_ctx)
{
	i2c_reg_t data;
	sdvo_status_t status = SS_SUCCESS;
	i2c_reg_t encoding_mode = DVI_MODE;

	if((p_ctx->p_callback->eld) == NULL){
		sdvo_set_digital_encoding_mode(p_ctx, encoding_mode);
		return SS_NOT_SUPPORTED;
	}
	if(*(p_ctx->p_callback->eld) == NULL){
		sdvo_set_digital_encoding_mode(p_ctx, encoding_mode);
		return SS_NOT_SUPPORTED;
	}

	/* Prepare buffer for video information. Video only requires a total of 3
	 * buffers. (0)ELD (1)AVI (2)SPD. SO we will set it to 2 indexbased buffer */
	status = sdvo_execute_command(p_ctx, GET_HDMI_BUFFER_AUDIO_VIDEO_SPLIT,
				0, NULL,
				1, &data);
	data = 2;
	status = sdvo_execute_command(p_ctx, SET_HDMI_BUFFER_AUDIO_VIDEO_SPLIT,
				1, &data,
				0, NULL);

	/* Send pixel replication and colorimetry */
	if (sdvo_hdmi_pr_and_color(p_ctx) != SS_SUCCESS) {
		PD_ERROR("Fail toset pixel replication and colorimetry");
	}

	/* Build and send ELD Info Frames */
	if(sdvo_hdmi_send_eld(p_ctx) != SS_SUCCESS){
		PD_ERROR("Fail to write ELD to transmitter");
	}

	/* Build and send AVI Info Frames */
	if (sdvo_hdmi_avi_info_frame(p_ctx) != SS_SUCCESS) {
		PD_ERROR("Fail to write AVI infoframes to transmitter");
	}

	/* Build and send SPD Info Frames */
#ifndef CONFIG_MICRO
	if (sdvo_hdmi_spd_info_frame(p_ctx) != SS_SUCCESS) {
		PD_ERROR("Fail to write SPD Infoframes to transmitter");
	}
#endif
	/* Set to HDMI mode */
	if((*p_ctx->p_callback->eld)->audio_support){
		encoding_mode = HDMI_MODE;
	}
	status = sdvo_set_digital_encoding_mode(p_ctx, encoding_mode);
	if (status != SS_SUCCESS) {
		PD_DEBUG("sdvo: Fail to set HDMI mode ");
	}

	return SS_SUCCESS;
}
/*	============================================================================
	Function	:	sdvo_hdmi_pr_and_color is called to send pixel replication
					and colorimetry data to SDVO device.

	Parameters	:	p_context: Pointer to port driver allocated context
								structure

	Remarks 	:

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_pr_and_color(sdvo_device_context_t *p_context)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status = SS_SUCCESS;
	i2c_reg_t pixel_rep = (i2c_reg_t)(*p_ctx->p_callback->eld)->pixel_rep - 1;
	i2c_reg_t quantization = (i2c_reg_t)(*p_ctx->p_callback->eld)->quantization;

	/* Set pixel replication */
	status = sdvo_execute_command(p_ctx, SET_PIXEL_REPLICATION,
				1, &pixel_rep,
				0, NULL);
	if (status != SS_SUCCESS) {
		return status;
	}

	/* Set colorimetry */
	status = sdvo_execute_command(p_ctx, SET_CURRENT_COLORIMETRY,
				1, &quantization,
				0, NULL);
	if (status != SS_SUCCESS) {
		return status;
	}
	return status;
};
/*	============================================================================
	Function	:	sdvo_hdmi_send_eld

	Parameters	:

	Remarks 	:	Builds eld structure and write it into SDVO ELD buffers

	Returns 	:
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_send_eld(sdvo_device_context_t *p_ctx)
{
	sdvo_status_t status;
	sdvo_audio_state_t audio_state;

	if((p_ctx->p_callback->eld) == NULL){
		return SS_NOT_SUPPORTED;
	}
	if(*(p_ctx->p_callback->eld) == NULL){
		return SS_NOT_SUPPORTED;
	}
	if(!((*p_ctx->p_callback->eld)->audio_flag & ELD_AVAIL)){
		PD_DEBUG("Eld not available");
		return SS_NOT_SUPPORTED;
	}

	/* ELD 1.0, CEA version retrieve from callback */
	(*p_ctx->p_callback->eld)->cea_ver = 0;
	(*p_ctx->p_callback->eld)->eld_ver = 1;

	/* Capability Flags */
	(*p_ctx->p_callback->eld)->repeater = 0;
	(*p_ctx->p_callback->eld)->hdcp = 0;
#ifdef CONFIG_MICRO
	(*p_ctx->p_callback->eld)->_44ms = 0;
#endif

	/* We do not provide Monitor length */
	(*p_ctx->p_callback->eld)->mnl = 0;

#ifdef CONFIG_MICRO
	(*p_ctx->p_callback->eld)->sadc = 0;
#endif

	audio_state.value = 0;
	audio_state.eld_valid = FALSE;
	audio_state.presense_detect = TRUE;
	status = sdvo_execute_command(p_ctx, SET_AUDIO_STATE,
				1, (i2c_reg_t *)&audio_state,
				0, NULL);
	if (status != SS_SUCCESS) {
		PD_ERROR("Set Audio eld_valid  fail (FALSE)");
		return status;
	}

	/* Write ELD to SDVO buffers */
	status = sdvo_hdmi_write_buffer(p_ctx, SDVO_HDMI_ELD_BUFFER ,
		0,  (*p_ctx->p_callback->eld)->eld_ptr, SDVO_HDMI_ELD_BUFFER_SIZE);
	if (status != SS_SUCCESS) {
		PD_ERROR("Write ELD Failed !");
		return status;
	}

	/* Send audio state change by toggling eld_valid bit */
	audio_state.eld_valid = TRUE;
	status = sdvo_execute_command(p_ctx, SET_AUDIO_STATE,
				1, (i2c_reg_t *)&audio_state,
				0, NULL);
	if (status != SS_SUCCESS) {
		PD_ERROR("Set Audio eld_valid  fail (TRUE)");
		return status;
	}

	return SS_SUCCESS;
}
/*	============================================================================
	Function	:	sdvo_hdmi_avi_info_frame is called to program AVI infoframes

	Parameters	:	p_context: Pointer to port driver allocated context
								structure
					p_mode: Point to current video output display mode

	Remarks 	:

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_avi_info_frame(sdvo_device_context_t *p_context)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status = SS_SUCCESS;
	unsigned long sum = 0;
	int i;
	sdvo_avi_info_t		avi;

	/* Initialize AVI InfoFrame to no scan info, no bar info, no active format */
	/* mode rgb, active format same as picture format */
	/* no aspect ratio, no colorimetry, no scaling */
	pd_memset(&avi, 0, sizeof(sdvo_avi_info_t));
	avi.header.type = SDVO_HDMI_AVI_INFO_TYPE;
	avi.header.version = 2;
	avi.header.length = SDVO_HDMI_AVI_BUFFER_SIZE - sizeof(sdvo_info_header_t);

	/* Fill in VIC, colorimetry, aspect ratio */

	/* Set Video ID Code */
	avi.video_id_code = (unsigned char)(*p_ctx->p_callback->eld)->video_code;

	/* Set aspect ratio */
	avi.pic_aspect_ratio = (unsigned char)(*p_ctx->p_callback->eld)->aspect_ratio;

	/* Set Colorimetry */
	avi.colorimetry = (unsigned char)(*p_ctx->p_callback->eld)->colorimetry;

	/* Calc checksum */
	for (i= 0; i < SDVO_HDMI_AVI_BUFFER_SIZE; i++) {
		sum += avi.data[i];
	}
	avi.header.chksum = (unsigned char)(0x100 - (sum & 0xFF));

	/* Send data to SDVO device */
	status = sdvo_hdmi_write_buffer(p_ctx, SDVO_HDMI_AVI_BUFFER, 0, avi.data,
		SDVO_HDMI_AVI_BUFFER_SIZE);
	return status;
};
/*	============================================================================
	Function	:	sdvo_hdmi_spd_info_frame is called to program SPD infoframes

	Parameters	:	p_context: Pointer to port driver allocated context
								structure

	Remarks 	:

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_spd_info_frame(sdvo_device_context_t *p_context)
{
	sdvo_device_context_t *p_ctx = (sdvo_device_context_t *)p_context;
	sdvo_status_t status = SS_SUCCESS;
	sdvo_spd_info_t		spd;
	unsigned long sum = 0;
	int i;

	/* Initialize SPD InfoFrame to zero */
	pd_memset(&spd, 0, sizeof(sdvo_spd_info_t));
	spd.header.type = SDVO_HDMI_SPD_INFO_TYPE;
	spd.header.version = 1;
	spd.header.length = SDVO_HDMI_SPD_BUFFER_SIZE - sizeof(sdvo_info_header_t);

	pd_memcpy(spd.vendor_name, SDVO_HDMI_VENDOR_NAME, SDVO_HDMI_INTEL_VENDOR_NAME_SIZE);
	pd_memcpy(spd.product_desc,
		SDVO_HDMI_VENDOR_DESCRIPTION, SDVO_HDMI_IEGD_VENDOR_DESCRIPTION_SIZE);
	spd.source_device = SDVO_HDMI_SPD_SOURCE_PC;

	/* Calc checksum */
	for (i= 0; i < SDVO_HDMI_SPD_BUFFER_SIZE; i++) {
		sum += spd.data[i];
	}
	spd.header.chksum = (unsigned char)(0x100 - (sum & 0xFF));

	/* Send data to SDVO device */
	status = sdvo_hdmi_write_buffer
		(p_ctx, SDVO_HDMI_SPD_BUFFER, 0, spd.data, SDVO_HDMI_SPD_BUFFER_SIZE);
	return status;
};
/*	============================================================================
	Function	:	sdvo_hdmi_write_buffer

	Parameters	:

	Remarks 	:	Write into SDVO buffers

	Returns 	:
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_hdmi_write_buffer
	(sdvo_device_context_t *p_ctx,unsigned char index,
	unsigned char offset, unsigned char *input, int len)
{
	sdvo_status_t status;
	i2c_reg_t hdmi_index[2],data[8];
	int buffer_size = 0;
	unsigned char total_bytes, *p_sdvo_buffer, *ori_p_sdvo_buffer;

	/* Set Buffer index and offset */
	hdmi_index[0] = index;
	hdmi_index[1] = offset;
	status = sdvo_execute_command(p_ctx, SET_HDMI_BUFFER_INDEX,
				2, hdmi_index,
				0, NULL);
	if (status != SS_SUCCESS) {
		PD_ERROR("HDMI wirte buffer fail : Set index");
		return status;
	}

	/* Setup buffer transmit rate */
	if(index != SDVO_HDMI_ELD_BUFFER){
		data[0] = SDVO_HDMI_TRANSMIT_EVERY;
		status = sdvo_execute_command(p_ctx, SET_HDMI_BUFFER_TRANSMIT_RATE,
					1, data,
					0, NULL);
		if (status != SS_SUCCESS) {
			PD_ERROR("HDMI wirte buffer fail : Transmite rate");
			return status;
		}
	}
	else {
		status = sdvo_execute_command(p_ctx, GET_HDMI_BUFFER_TRANSMIT_RATE,
					0, NULL,
					1, data);
		if (status != SS_SUCCESS) {
			PD_ERROR("HDMI wirte buffer fail : Transmite rate");
			return status;
		}
	}

	/* Get buffer size info */
	status = sdvo_execute_command(p_ctx, GET_HDMI_BUFFER_INFO,
			0, NULL,
			1, (i2c_reg_t *)&data);
	buffer_size = data[0] + 1;
	/* Minimum buffer size is 3 then something is wrong */
	if (status != SS_SUCCESS || buffer_size <0x03) {
		PD_ERROR("HDMI wirte buffer fail : Buffer size");
		return status;
	}

	/* Make sure to query size of buffer from SDVO and use it, not size of source infoframe buffers */
	/* copy structs into cleared memory the size of SDVO buffer */
	p_sdvo_buffer = pd_malloc(buffer_size);
	ori_p_sdvo_buffer = p_sdvo_buffer;
	pd_memset(p_sdvo_buffer, 0, buffer_size);
	pd_memcpy(p_sdvo_buffer, input, (len < buffer_size) ? len : buffer_size);

	while(buffer_size) {
		//total_bytes = (buffer_size >= 8) ? 8:buffer_size;
		if(buffer_size >= 8)	total_bytes = 8;
		else					total_bytes = (unsigned char)buffer_size;

		status = sdvo_execute_command(p_ctx, SET_HDMI_BUFFER_DATA,
				total_bytes, (i2c_reg_t *)p_sdvo_buffer,
				0, NULL);
		if (status != SS_SUCCESS) {
			PD_ERROR("HDMI wirte buffer fail : Set buffer data");
			return status;
		}
		p_sdvo_buffer +=total_bytes;
		buffer_size -= total_bytes;
	};

	pd_free(ori_p_sdvo_buffer);
	return SS_SUCCESS;
}

#endif/* SDVO_HDMI */



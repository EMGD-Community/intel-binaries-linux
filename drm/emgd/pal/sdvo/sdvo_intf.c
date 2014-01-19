/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_intf.c
 * $Revision: 1.12 $
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
 *  Functions that provide interface to SDVO device
 *-----------------------------------------------------------------------------
 */

#include "sdvo_port.h"

/* .......................................................................... */
void sdvo_convert_parm_to_dtd(i2c_reg_t parm[16], sdvo_dtd_t *p_dtd);
void sdvo_convert_dtd_to_parm(
	sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd,
	i2c_reg_t parm[16]);


/* .......................................................................... */
/* .......................................................................... */
/*	============================================================================
	Function	:	sdvo_execute_command transfers a command and argument to
					the SDVO device and returns the status of execution

	Parameters	:	opcode		: One of the sdvo_opcode_t opcodes
					num_args 	: Number of arguments 0-8
					p_arg	    : List of arguments
					num_returns	: Expected number of returns 0-8
					p_ret_value	: List of return values

	Remarks 	:	sdvo_execute_command is core function that is called by other
					SDVO interface functions to execute a command.
					The arguments of the command are transferred to the argument
					I2C registers of the SDVO device
					The opcode is then transferred to the opcode I2C register
					It then waits for the command to complete by reading the
					status I2C register up to 3 times

	Returns 	:	sdvo_status_t : Status of command execution
	------------------------------------------------------------------------- */
sdvo_status_t sdvo_execute_command(sdvo_device_context_t *p_ctx,
	sdvo_opcode_t opcode,
	i2c_reg_t num_args,	   i2c_reg_t *p_arg,
	i2c_reg_t num_returns, i2c_reg_t *p_ret_value)
{
	i2c_reg_t status, i;

	/*	..................................................................... */
	/*	Error checking */
	if ((num_args > SDVO_MAX_ARGS) || (num_returns > SDVO_MAX_RETURNS)) {

		return SS_UNSUCCESSFUL;
	}

	if ((num_args > 0) && (p_arg == NULL)) {

		return SS_UNSUCCESSFUL;
	}

	if ((num_returns > 0) && (p_ret_value == NULL)) {

		return SS_UNSUCCESSFUL;
	}

	/*	..................................................................... */
	/*	Write the arguments and the opcode */
	for (i = 0; i < num_args; i++) {

		if (! sdvo_write_i2c_reg(p_ctx, (i2c_reg_t)(SDVO_REG_ARG_START - i),
				p_arg[i])) {

			return SS_WRITE_FAILED;
		}
	}

	if (! sdvo_write_i2c_reg(p_ctx, (i2c_reg_t)SDVO_REG_OPCODE,
			(i2c_reg_t)opcode)) {

		return SS_WRITE_FAILED;
	}

	/*	..................................................................... */
	/*	Wait for command to complete */
	for (i = 0; i < SDVO_MAX_RETRIES; i++) {

		if (! sdvo_read_i2c_reg(p_ctx, SDVO_REG_STATUS, &status)) {

			return SS_READ_FAILED;
		}

		if (status != SS_PENDING) {

			break;
		}
	}

	/*	..................................................................... */
	/*	Read the return parameters if command succeeded */
	if (status == SS_SUCCESS) {

		for (i = 0; i < num_returns; i++) {

			if (! sdvo_read_i2c_reg(p_ctx, SDVO_REG_RETURN_START + i,
					&p_ret_value[i])) {

				 return SS_READ_FAILED;
			}
		}
	}

	return status;
}

sdvo_status_t sdvo_execute_command_read (sdvo_device_context_t *p_ctx,
	i2c_reg_t num_returns, i2c_reg_t *p_ret_value)
{
	i2c_reg_t status, i;

	for (i = 0; i < SDVO_MAX_RETRIES; i++) {
		if (!sdvo_read_i2c_reg(p_ctx, SDVO_REG_STATUS, &status)) {
			return SS_READ_FAILED;
		}

		if (status != SS_PENDING) {
			break;
		}
	}

	/*	Read the return parameters if command succeeded */
	if (status == SS_SUCCESS) {
		for (i = 0; i < num_returns; i++) {
			if (! sdvo_read_i2c_reg(p_ctx, SDVO_REG_RETURN_START + i,
					&p_ret_value[i])) {
				 return SS_READ_FAILED;
			}
		}
	}
	return status;
}

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_reset(sdvo_device_context_t *p_ctx)
{
	return sdvo_execute_command(p_ctx, RESET,
							   0, NULL,
							   0, NULL);
}
#endif

sdvo_status_t sdvo_get_device_capabilities(sdvo_device_context_t *p_ctx,
	sdvo_device_capabilities_t *p_Dev_Cap)
{
	return sdvo_execute_command(p_ctx, GET_DEVICE_CAPABILITIES,
							    0, NULL,
							    8, (i2c_reg_t *)p_Dev_Cap);
}


sdvo_status_t sdvo_get_trained_inputs(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Out)
{
	return sdvo_execute_command(p_ctx, GET_TRAINED_INPUTS,
							   0, NULL,
							   1, p_Out);
}


sdvo_status_t sdvo_get_active_outputs(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t *p_Out_Flags)
{
	return sdvo_execute_command(p_ctx, GET_ACTIVE_OUTPUTS,
				0, NULL,
				2, (i2c_reg_t *)p_Out_Flags);
}


sdvo_status_t sdvo_set_active_outputs(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags)
{
	return sdvo_execute_command(p_ctx, SET_ACTIVE_OUTPUTS,
			   2, (i2c_reg_t *)&out_flags,
			   0, NULL);
}


#ifndef CONFIG_MICRO
sdvo_status_t sdvo_get_in_out_map(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags[2])
{
	return sdvo_execute_command(p_ctx, GET_IN_OUT_MAP,
				0, NULL,
				4, (i2c_reg_t *)out_flags);
}
#endif

sdvo_status_t sdvo_set_in_out_map(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t out_flags[2])
{
	return sdvo_execute_command(p_ctx, SET_IN_OUT_MAP,
				4, (i2c_reg_t *)out_flags,
				0, NULL);
}


sdvo_status_t sdvo_get_attached_displays(sdvo_device_context_t *p_ctx,
	sdvo_output_flags_t *p_Out_Flags)
{
	i2c_reg_t status, i;

	status = sdvo_execute_command(p_ctx, GET_ATTACHED_DISPLAYS,
				0, NULL,
				2, (i2c_reg_t *)p_Out_Flags);


	PD_DEBUG("sdvo: GET_ATTACHED_DISPLAYS status = %d]",  status);
	for (i=0; i<5; i++) {
		if (status == SS_SUCCESS)
			break;
		else {
			pd_ui_usleep(50000); /* wait for 50ms */
			status = sdvo_execute_command_read(p_ctx, 2, (i2c_reg_t *)p_Out_Flags);
		}
	}

	PD_DEBUG("sdvo: sdvo_execute_command_read- no. of tries = [%d]",  i);
	return status;
}


sdvo_status_t sdvo_set_target_input(sdvo_device_context_t *p_ctx,
	sdvo_target_input_t input)
{
	i2c_reg_t arg_value = input == SDVO0 ? 0 : 1;

	return sdvo_execute_command(p_ctx, SET_TARGET_INPUT,
				1, (i2c_reg_t *)&arg_value,
				0, NULL);
}


sdvo_status_t sdvo_set_target_output(sdvo_device_context_t *p_ctx,
		sdvo_output_flags_t out_flags)
{
	return sdvo_execute_command(p_ctx, SET_TARGET_OUTUT,
			   2, (i2c_reg_t *)&out_flags,
			   0, NULL);
}

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_get_input_timings(sdvo_device_context_t *p_ctx,
								 sdvo_dtd_t *p_dtd)
{
	sdvo_status_t status;
	i2c_reg_t ret[16];

	status = sdvo_execute_command(p_ctx, GET_INPUT_TIMINGS_PART1,
				0, NULL,
				8, ret);
	if (status != SS_SUCCESS) {

		return status;
	}

	status = sdvo_execute_command(p_ctx, GET_INPUT_TIMINGS_PART2,
				 0, NULL,
				 8, &ret[8]);
	if (status != SS_SUCCESS) {

		return status;
	}

	sdvo_convert_parm_to_dtd(ret, p_dtd);

	return status;
}
#endif

sdvo_status_t sdvo_set_input_timings(sdvo_device_context_t *p_ctx,
								 sdvo_dtd_t *p_dtd)
{
	sdvo_status_t status;
	i2c_reg_t parm[16];

	sdvo_convert_dtd_to_parm(p_ctx, p_dtd, parm);

	status = sdvo_execute_command(p_ctx, SET_INPUT_TIMINGS_PART1,
				 8, parm,
				 0, NULL);

	if ((status != SS_SUCCESS) && (status != SS_NOT_SUPPORTED)) {

		return status;
	}

	status = sdvo_execute_command(p_ctx, SET_INPUT_TIMINGS_PART2,
				 8, &parm[8],
				 0, NULL);

	return status;
}

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_get_output_timings(sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd)
{
	sdvo_status_t status;
	i2c_reg_t ret[16];

	status = sdvo_execute_command(p_ctx, GET_OUTPUT_TIMINGS_PART1,
				 0, NULL,
				 8, ret);
	if ((status != SS_SUCCESS) && (status != SS_NOT_SUPPORTED)) {

		return status;
	}

	status = sdvo_execute_command(p_ctx, GET_OUTPUT_TIMINGS_PART2,
				 0, NULL,
				 8, &ret[8]);
	if (status != SS_SUCCESS) {

		return status;
	}

	sdvo_convert_parm_to_dtd(ret, p_dtd);

	return status;
}
#endif

sdvo_status_t sdvo_set_output_timings(sdvo_device_context_t *p_ctx,
  sdvo_dtd_t *p_dtd)
{
	sdvo_status_t status;
	i2c_reg_t parm[16];

	sdvo_convert_dtd_to_parm(p_ctx, p_dtd, parm);

	status = sdvo_execute_command(p_ctx, SET_OUTPUT_TIMINGS_PART1,
				 8, parm,
				 0, NULL);

	if (status != SS_SUCCESS) {

		return status;
	}

	status = sdvo_execute_command(p_ctx, SET_OUTPUT_TIMINGS_PART2,
				 8, &parm[8],
				 0, NULL);

	return status;
}


sdvo_status_t sdvo_get_preferred_input_timings(sdvo_device_context_t *p_ctx,
				  sdvo_create_preferred_timings_t *p_Timings,
				  sdvo_dtd_t *p_dtd)
{
	sdvo_status_t status;
	i2c_reg_t ret[16];

	/*	..................................................................... */
	status = sdvo_execute_command(p_ctx, CREATE_PREFERRED_INPUT_TIMINGS,
				 7, (i2c_reg_t *)p_Timings,
				 0, NULL);
	if (status != SS_SUCCESS) {

		return status;
	}

	/*	..................................................................... */
	status =  sdvo_execute_command(p_ctx, GET_PREFERRED_INPUT_TIMINGS_PART1,
				  0, NULL,
				  8, ret);
	if (status != SS_SUCCESS) {

		return status;
	}

	status = sdvo_execute_command(p_ctx, GET_PREFERRED_INPUT_TIMINGS_PART2,
				 0, NULL,
				 8, &ret[8]);
	if (status != SS_SUCCESS) {

		return status;
	}

	/*	..................................................................... */
	sdvo_convert_parm_to_dtd(ret, p_dtd);

	return status;
}


/*	in_clock[0] = min, in_clock[1] = max */
sdvo_status_t sdvo_get_input_pixel_clock_range(sdvo_device_context_t *p_ctx,
	unsigned short in_clock[2])
{
	return sdvo_execute_command(p_ctx, GET_INPUT_PIXEL_CLOCK_RANGE,
			   0, NULL,
			   4, (i2c_reg_t *)in_clock);
}

#ifndef CONFIG_MICRO
/*	out_Clock[0] = min, out_Clock[1] = max */
sdvo_status_t sdvo_get_output_pixel_clock_range(sdvo_device_context_t *p_ctx,
				  unsigned short out_Clock[2])
{
	return sdvo_execute_command(p_ctx, GET_OUTPUT_PIXEL_CLOCK_RANGE,
			   0, NULL,
			   4, (i2c_reg_t *)out_Clock);
}

sdvo_status_t sdvo_get_clock_rate_multiplier(sdvo_device_context_t *p_ctx,
	sdvo_clock_rate_mult_t *p_mult)
{
	*p_mult = 0;

	return sdvo_execute_command(p_ctx, GET_SDVO_CLOCK_RATE_MULTIPLIER,
			   0, NULL,
			   1, (i2c_reg_t *)p_mult);
}
#endif


sdvo_status_t sdvo_set_clock_rate_multiplier(sdvo_device_context_t *p_ctx,
	sdvo_clock_rate_mult_t mult)
{
	return sdvo_execute_command(p_ctx, SET_SDVO_CLOCK_RATE_MULTIPLIER,
			   1, (i2c_reg_t *)&mult,
			   0, NULL);
}


sdvo_status_t sdvo_get_supported_power_states(sdvo_device_context_t *p_ctx,
	 i2c_reg_t *p_pwr_state)
{
	return sdvo_execute_command(p_ctx, GET_SUPPORTED_POWER_STATES,
			   0, NULL,
			   1, p_pwr_state);
}


sdvo_status_t sdvo_get_power_state(sdvo_device_context_t *p_ctx,
	   i2c_reg_t *p_pwr_state)
{
	return sdvo_execute_command(p_ctx, GET_POWER_STATE,
			   0, NULL,
			   1, p_pwr_state);
}


sdvo_status_t sdvo_set_power_state(sdvo_device_context_t *p_ctx,
   i2c_reg_t pwr_state)
{
	return sdvo_execute_command(p_ctx, SET_POWER_STATE,
			   1, &pwr_state,
			   0, NULL);
}


sdvo_status_t sdvo_set_display_power_state(sdvo_device_context_t *p_ctx,
  i2c_reg_t disp_pwr_state)
{
	return sdvo_execute_command(p_ctx, SET_DISPLAY_POWER_STATE,
			   1, &disp_pwr_state,
			   0, NULL);
}
#if !defined(CONFIG_MICRO)
sdvo_status_t sdvo_set_digital_encoding_mode(sdvo_device_context_t *p_ctx,
  i2c_reg_t digital_encoding_mode)
{
	return sdvo_execute_command(p_ctx, SET_DIGITAL_ENCODING_MODE,
			   1, &digital_encoding_mode,
			   0, NULL);
}
sdvo_status_t sdvo_get_hdmi_audio_transmission_char(sdvo_device_context_t *p_ctx,
  i2c_reg_t *hdmi_reg)
{
	return sdvo_execute_command(p_ctx, GET_HDMI_AUDIO_TRANSMISSION_CHARACTERISTICS,
			   0, NULL,
			   3, (i2c_reg_t*)hdmi_reg);
}
sdvo_status_t sdvo_get_supported_encoding_modes(sdvo_device_context_t *p_ctx,
  sdvo_digital_encode_modes_t *digital_encoding_modes)
{
	return sdvo_execute_command(p_ctx, GET_SUPPORTED_DIGITAL_ENCODING_MODES,
			   0, NULL,
			   8, (i2c_reg_t*)digital_encoding_modes);
}

sdvo_status_t sdvo_get_digital_encoding_mode(sdvo_device_context_t *p_ctx,
  i2c_reg_t *digital_encoding_mode)
{
	return sdvo_execute_command(p_ctx, GET_DIGITAL_ENCODING_MODE,
			   0, NULL,
			   1, (i2c_reg_t*)digital_encoding_mode);
}
/* ------------------------------------------------------------------------- */
#endif
sdvo_status_t sdvo_set_upscalar_coefficient(sdvo_device_context_t *p_ctx,
  i2c_reg_t upscalar_coefficient)
{
	return sdvo_execute_command(p_ctx, SET_UPSCALAR_COEFFICIENTS,
			   1, &upscalar_coefficient,
			   0, NULL);
}

sdvo_status_t sdvo_get_tv_output_format(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_TV_Output_Format_Flag)
{
	return sdvo_execute_command(p_ctx, GET_TV_OUTPUT_FORMAT,
				0, NULL,
				6, p_TV_Output_Format_Flag);
}

sdvo_status_t sdvo_get_sdtv_resolution_support(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Requested_SDTV_Format_Flag,
	i2c_reg_t *p_Resolution_Support_Flags)
{
	return sdvo_execute_command(p_ctx, GET_SDTV_RESOLUTION_SUPPORT,
				3, p_Requested_SDTV_Format_Flag,
				3, p_Resolution_Support_Flags);
}

sdvo_status_t sdvo_get_scaled_hdtv_resolution_support(sdvo_device_context_t *p_ctx,
	i2c_reg_t *p_Requested_HDTV_Format_Flag,
	i2c_reg_t *p_Resolution_Support_Flags)
{
	return sdvo_execute_command(p_ctx, GET_SCALED_HDTV_RESOLUTION_SUPPORT,
				4, p_Requested_HDTV_Format_Flag,
				8, p_Resolution_Support_Flags);
}

void sdvo_convert_parm_to_dtd(i2c_reg_t parm[16], sdvo_dtd_t *p_dtd)
{
	pd_memset(p_dtd, 0, sizeof(sdvo_dtd_t));

	p_dtd->pixel_clock			= (parm[1] << 8) | parm[0];
	p_dtd->horz_active			= ((parm[4] & 0xF0) << 4) | parm[2];
	p_dtd->horz_blanking		= ((parm[4] & 0x0F) << 8) | parm[3];
	p_dtd->vert_active			= ((parm[7] & 0xF0) << 4) | parm[5];
	p_dtd->vert_blanking		= ((parm[7] & 0x0F) << 8) | parm[6];
	p_dtd->horz_sync_offset		= ((parm[11] & 0xC0) << 2) | parm[8];
	p_dtd->horz_sync_pulse_width= ((parm[11] & 0x30) << 4) | parm[9];
	p_dtd->vert_sync_offset		= (parm[14] & 0xC0) | ((parm[11] & 0x0C) << 2) |
								  ((parm[10] & 0xF0) >> 4);
	p_dtd->vert_sync_pulse_width= ((parm[11] & 0x03) << 4) | (parm[10] & 0x0F);
	p_dtd->dtd_flags			= parm[12];
	p_dtd->sdvo_flags			= parm[13];
}


void sdvo_convert_dtd_to_parm(
	sdvo_device_context_t *p_ctx,
	sdvo_dtd_t *p_dtd,
	i2c_reg_t parm[16])
{
	unsigned int multp;

	parm[0]  = (p_dtd->pixel_clock & 0xFF);
	parm[1]  = ((p_dtd->pixel_clock & 0xFF00) >> 8);

	if (p_ctx->st_sdvo) {
		if (p_dtd->pixel_clock > 10000) {			/*	100-200 MHz */
			multp = 1;
		} else if (p_dtd->pixel_clock > 5000) {     /*	50-100 Mhz */
			multp = 2;
		} else {							        /*	25-50 Mhz */
			multp = 4;
		}

		parm[2]  = ((p_dtd->horz_active * multp) & 0xFF);
		parm[3]  = ((p_dtd->horz_blanking * multp) & 0xFF);
		parm[4]  = ((((p_dtd->horz_active * multp) & 0xF00) >> 4) |
					(((p_dtd->horz_blanking * multp) & 0xF00) >> 8));
	} else {
		parm[2]  = (p_dtd->horz_active & 0xFF);
		parm[3]  = (p_dtd->horz_blanking & 0xFF);
		parm[4]  = ((p_dtd->horz_active & 0xF00) >> 4) |
					((p_dtd->horz_blanking & 0xF00) >> 8);
	}
	parm[5]  = (p_dtd->vert_active & 0xFF);
	parm[6]  = (p_dtd->vert_blanking & 0xFF);
	parm[7]  = ((p_dtd->vert_active & 0xF00) >> 4) |
				((p_dtd->vert_blanking & 0xF00) >> 8);
	parm[8]  = (p_dtd->horz_sync_offset & 0xFF);
	parm[9]  = (p_dtd->horz_sync_pulse_width & 0xFF);
	parm[10] = ((p_dtd->vert_sync_offset & 0x0F) << 4) |
				(p_dtd->vert_sync_pulse_width & 0x0F);
	parm[11] = ((p_dtd->horz_sync_offset & 0x300) >> 2) |
			   ((p_dtd->horz_sync_pulse_width & 0x300) >> 4) |
			   ((p_dtd->vert_sync_offset & 0x30) >> 2) |
			   ((p_dtd->vert_sync_pulse_width & 0x30) >> 4);
	parm[12] = p_dtd->dtd_flags;
	parm[13] = p_dtd->sdvo_flags;
	parm[14] = p_dtd->vert_sync_offset & 0xC0;
}

#ifndef CONFIG_MICRO
sdvo_status_t sdvo_get_min_max_pixel_clock(sdvo_device_context_t *p_ctx,
	unsigned long *p_min_clock,
	unsigned long *p_max_clock)
{
	sdvo_status_t status;
	sdvo_clock_rate_mult_t clock_mult;
	unsigned short in_range[2], out_range[2];

	status = sdvo_get_clock_rate_multiplier(p_ctx, &clock_mult);
	if (status != SS_SUCCESS) {

		return status;
	}

	status = sdvo_get_input_pixel_clock_range(p_ctx, in_range);
	if (status != SS_SUCCESS) {

		return status;
	}

	status = sdvo_get_output_pixel_clock_range(p_ctx, out_range);
	if (status != SS_SUCCESS) {

		return status;
	}

	switch (clock_mult)
	{
		case CRM_1X :
			break;

		case CRM_2X :
			in_range[0] *= 2;
			break;

		case CRM_4X :
			in_range[0] *= 4;
			break;

		default :
			return SS_INVALID_RETURN;
	}

	/*	Minimum pixel clock supported is the maximum of min input/output */
	/*  pixel clock */
	*p_min_clock = in_range[0] > out_range[0] ? in_range[0] : out_range[0];

	/*	Maximum pixel clock supported is minimum of max pixel clock of */
	/*	encoder and SDVO bus clock rate */
	*p_max_clock = in_range[1] < out_range[1] ? in_range[1] : out_range[1];

	return SS_SUCCESS;
}
#endif

unsigned char sdvo_write_i2c_reg(sdvo_device_context_t *p_ctx, i2c_reg_t offset,
	i2c_reg_t value)
{
	pd_reg_t reg_list[2];

/*	PD_DEBUG("sdvo : W : 0x%02x, 0x%02x", offset, value); */

	reg_list[0].reg = offset;
	reg_list[0].value = value;

	reg_list[1].reg = PD_REG_LIST_END;

	if (p_ctx->p_callback->write_regs(p_ctx->p_callback->callback_context, reg_list,
			PD_REG_I2C)) {

		return FALSE;

	}else {

		return TRUE;
	}
}


unsigned char sdvo_read_i2c_reg(sdvo_device_context_t *p_ctx, unsigned char offset,
						i2c_reg_t *p_Value)
{
	pd_reg_t reg_list[2];

	reg_list[0].reg = offset;
	reg_list[1].reg = PD_REG_LIST_END;

	if (p_ctx->p_callback->read_regs(p_ctx->p_callback->callback_context, reg_list,
								   PD_REG_I2C)) {

		return FALSE;

	} else {

		*p_Value = (i2c_reg_t)reg_list[0].value;

/*		PD_DEBUG("sdvo : R : 0x%02x, 0x%02x", offset, *p_Value); */

		return TRUE;
	}
}

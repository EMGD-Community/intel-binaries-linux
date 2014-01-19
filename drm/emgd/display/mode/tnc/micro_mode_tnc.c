/*
 *-----------------------------------------------------------------------------
 * Filename: micro_mode_tnc.c
 * $Revision: 1.47 $
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
 *  1) Atom E6xx Core implementations for the mode dispatch functions.
 *  2) 0:2:0 = Device2 = LNC VGA
 *  0:3:0 = Device3 = Atom E6xx overlay
 *  3) For MMIO access:
 *  Dev2 only: Use EMGD_READ32(), EMGD_WRITE32(): less code
 *  Dev2/3: Use READ_MMIO_REG_TNC(), WRITE_MMIO_REG_TNC(): more code
 *  For code optimization sake use the right macro.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <io.h>

#include <igd.h>
#include <igd_pwr.h>

#include <context.h>
#include <mode.h>
#include <utils.h>
#include <dsp.h>
#include <vga.h>
#include <intelpci.h>
#include <math_fix.h>
#include <mode_access.h>
#include <rb.h>
#include <pi.h>
#include <dispatch.h>

#include <tnc/regs.h>
#include <tnc/context.h>
#include <tnc/mi.h>
#include <tnc/instr.h>
#include <tnc/igd_tnc_wa.h>
#include "drm_emgd_private.h"

#include "../cmn/match.h"
#include "../cmn/mode_dispatch.h"
#include "mode_tnc.h"

/*
	Turning on FIB part workaround for all IALs, for vBIOS this will limit
    the port drivers to one at a time (SDVO or LVDS)due to code size
 */

/*!
 * @addtogroup display_group
 * @{
 */

/*
 * Exports from the other components of this module.
 */

#ifdef CONFIG_TNC

extern int program_clock_tnc(igd_display_context_t *display,
	igd_clock_t *clock, unsigned long dclk);

int program_cdvo_tnc(void);

extern unsigned long get_port_type(int crtc_id);

extern mode_full_dispatch_t mode_full_dispatch_tnc;

int wait_for_vblank_tnc(unsigned long pipe_reg);
void wait_pipe(unsigned long pt, unsigned long pipe_reg, unsigned long check_on_off);

void shutdown_ST_bridge(igd_context_t *context);
void initialize_ST_bridge(igd_context_t *context, igd_display_port_t *port);

#if 0
static void cdvo_signal(void);
#endif

static unsigned long flag_clip_fix = IGD_CLIP_FIX_GLOBAL_ENABLE |
	IGD_CLIP_FIX_DISABLE_THROTTLE;

/* Extern defines for Device2, device3 and device31 iobases.
 * For Atom E6xx all devices are always io_mapped. */
extern unsigned char io_mapped;
extern unsigned short io_base;
extern unsigned char io_mapped_lvds;
extern unsigned short io_base_lvds;
extern unsigned char io_mapped_sdvo;
extern unsigned short io_base_sdvo;
extern unsigned char io_mapped_lpc;
extern unsigned short io_base_lpc;
extern unsigned char io_mapped_sdvo_st;
extern unsigned short io_base_sdvo_st;
extern unsigned char io_mapped_sdvo_st_gpio;
extern unsigned short io_base_sdvo_st_gpio;

#ifdef DEBUG_BUILD_TYPE
/* Debug configuration flag for B0 workaround. Workaround is turned on by default. */
int flag_basic_htotal_formula_wa = 1;    /* Enable formula workaround flag */
int flag_turn_off_port_wa = 1;           /* Turn off port when sweeping */
int flag_enable_tuning_wa = 1;           /* Disable sweeping flag */
#endif

/* Do not change the order */
const unsigned long ports_tnc[2] = {IGD_PORT_LVDS, IGD_PORT_SDVO};

tnc_wa_timing_t tune = {0, 0, 0, 0, 0, 0, 0,};

/* Temporary frame buffer */
static igd_framebuffer_info_t fb_info_tmp = {0,0,0,0,0,0,0};

/* VPhase variable */
int vphase = 0;	/* The current value to enable  vphase is 5 */

mode_data_tnc_t device_data_tnc[1] = {
	{
		0x000b0000, /* plane a preservation */
		0x00000000, /* plane b c preservation */
		0x60000000, /* pipe preservation */
		0x00003232, /* DSP FIFO Size A=50 B=50 C=28 May require fine tuning*/
		0x3f8f0404, /* FIFO watermark control1 */
		0x04040f04, /* FIFO watermark control2 */
		0x00000000, /* FIFO watermark control3 */
		0x04040404, /* FIFO watermark1 control4 */
		0x04040404, /* FIFO watermark1 control5 */
		0x00000078, /* FIFO watermark1 control6 */
	}
};

pixel_crc_t compute_pixel_crc( pixel_crc_t  pixel, pixel_crc_t  new_pixel)
{
	pixel_crc_t  pixel_crc_new;

    pixel_crc_new.pixel = 0x0;

    pixel_crc_new.bit.bit22 = pixel.bit.bit14;
    pixel_crc_new.bit.bit21 = pixel.bit.bit13;
    pixel_crc_new.bit.bit20 = pixel.bit.bit12;
    pixel_crc_new.bit.bit19 = pixel.bit.bit11;
    pixel_crc_new.bit.bit18 = pixel.bit.bit10;
    pixel_crc_new.bit.bit17 = pixel.bit.bit9;
    pixel_crc_new.bit.bit16 = pixel.bit.bit22 ^ pixel.bit.bit8;
    pixel_crc_new.bit.bit15 = pixel.bit.bit21 ^ pixel.bit.bit7;
    pixel_crc_new.bit.bit14 = pixel.bit.bit20 ^ pixel.bit.bit6;
    pixel_crc_new.bit.bit13 = pixel.bit.bit19 ^ pixel.bit.bit5;
    pixel_crc_new.bit.bit12 = pixel.bit.bit18 ^ pixel.bit.bit4;
    pixel_crc_new.bit.bit11 = pixel.bit.bit17 ^ pixel.bit.bit3;
    pixel_crc_new.bit.bit10 = pixel.bit.bit16 ^ pixel.bit.bit2;
    pixel_crc_new.bit.bit9  = pixel.bit.bit15 ^ pixel.bit.bit1;
    pixel_crc_new.bit.bit8  = pixel.bit.bit0;
    pixel_crc_new.bit.bit7  = pixel.bit.bit22 ^ new_pixel.bit.bit0;
    pixel_crc_new.bit.bit6  = pixel.bit.bit21 ^ new_pixel.bit.bit1;
    pixel_crc_new.bit.bit5  = pixel.bit.bit20 ^ new_pixel.bit.bit2;
    pixel_crc_new.bit.bit4  = pixel.bit.bit19 ^ new_pixel.bit.bit3;
    pixel_crc_new.bit.bit3  = pixel.bit.bit18 ^ new_pixel.bit.bit4;
    pixel_crc_new.bit.bit2  = pixel.bit.bit17 ^ new_pixel.bit.bit5;
    pixel_crc_new.bit.bit1  = pixel.bit.bit16 ^ new_pixel.bit.bit6;
    pixel_crc_new.bit.bit0  = pixel.bit.bit15 ^ new_pixel.bit.bit7;

	return (pixel_crc_new);
}

#ifndef CONFIG_MICRO
int check_display_tnc(igd_display_context_t *display,
					  unsigned short port_number,unsigned long status)
{
	pixel_crc_t red_val, green_val, blue_val;
	pixel_crc_t blue_crc, green_crc, red_crc;
	int i, j, bps = 4, ret_val = 0;
	unsigned long *curr_pixel, sync = 0, size = 0, dc;
	unsigned long tnc_crc_red, tnc_crc_green, tnc_crc_blue, temp;
	unsigned long temp_plane, temp_pitch, temp_control, plane_control;
	unsigned long pt = PORT_TYPE(display);
	igd_framebuffer_info_t *fb_info = PLANE(display)->fb_info;
	igd_timing_info_t pTimings_tmp, *pTimings_ext,*pTimings = PIPE(display)->timing;
	/* igd_display_port_t *port = PORT_OWNER(display); */
	platform_context_tnc_t *platform_context = display->context->platform_context;
	igd_rect_t dst_rect;
	igd_surface_t front_buffer;
	tnc_wa_timing_t *wa = WA_TUNE;
	os_alarm_t timeout;

	EMGD_TRACE_ENTER;

	dc = *(display->context->mod_dispatch.dsp_current_dc);
	pTimings_tmp = *pTimings;
	pTimings_ext = pTimings;

	/* Check to see if VGA plane was requested */
	while (pTimings_ext->extn_ptr) {
		pTimings_ext = (igd_timing_info_t *)pTimings_ext->extn_ptr;
	}
	if((pt != IGD_PORT_SDVO) || (display->context->device_context.rid != TNC_B0_RID)
		|| (platform_context->stbridgedev)
		|| (pTimings->reserved_dd & TNC_HTOTAL_TUNED) || (!FLAG(flag_enable_tuning_wa))
		|| (mode_context->tuning_wa == 0) /*if 0 don't tune*/
		|| wa->counter > LIMIT_TOTAL_CHECK_DISPLAY
		|| (MODE_IS_VGA(pTimings_ext))
		|| (!IGD_DC_CLONE(dc) && !IGD_DC_EXTENDED(dc))
		|| (platform_context->tnc_dev3_rid != TNC_B0_DEV3_RID)){
		/* Workaround is specific for SDVO display
		 * Dont tuned if it isnt B0 silicon
		 * Dont tuned if it is already tuned
		 * Dont tuned if bypass is turned on (debug)
		 * No tuning for VGA modes
		 * Stop Tuning if above limit LIMIT_TOTAL_CHECK_DISPLAY
		 * Dont tuned if B1 silicon (based on SDVO Device 3 RID)
		 */

		if(pt != IGD_PORT_SDVO ||
			(MODE_IS_VGA(pTimings_ext))){
			EMGD_DEBUG("Unsupported tuning");
		}else if((wa->counter > LIMIT_TOTAL_CHECK_DISPLAY) &&
			!(pTimings->reserved_dd & TNC_HTOTAL_TUNED)){
			pTimings->reserved_dd = 0;
			EMGD_ERROR("Unable to get tuned value!");
		}

		if(FLAG(flag_turn_off_port_wa)){
			/* Turn on SDVO source */
			temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG);
			temp |= BIT13;
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG, temp);
		}

		if(fb_info_tmp.width){
			/* Free up the temporary frame buffer */
			display->context->dispatch.gmm_free(fb_info_tmp.fb_base_offset);
			OS_MEMSET(&fb_info_tmp,0,sizeof (igd_framebuffer_info_t));
		}
		OS_MEMSET(wa, 0, sizeof(tnc_wa_timing_t));
		EMGD_TRACE_EXIT;
		return TRUE;
	}

	/* Increment checking counter */
	wa->counter++;

	/* CRC starting value */
	blue_crc.pixel = 0x007FFFFF;
	green_crc.pixel = 0x007FFFFF;
	red_crc.pixel = 0x007FFFFF;

	if(FLAG(flag_turn_off_port_wa)){
		/* Turn of port source.
		 * */
		temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG);
		temp &= ~BIT13;
		WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG, temp);
	}

	/* Allocate temporary 32 bpp frame buffer for CRC calculation.
	 * SV only provided codes for 32bpp CRC calculation so we would
	 * only calculate CRC for 32bpp. Once CRC is calculated EMGD
	 * would revert back to original plane. A temporary place is also
	 * neede so no corruption is seen on lvds for clone display
	 */
	if(fb_info_tmp.width != (unsigned short)pTimings_tmp.width &&
		fb_info_tmp.height != (unsigned short)pTimings_tmp.height){
		if(fb_info_tmp.width){
			EMGD_ERROR("Memory leak! Previous framebuffer is not freed!");
		}
		fb_info_tmp.width  = pTimings_tmp.width;
		fb_info_tmp.height = pTimings_tmp.height;
		fb_info_tmp.screen_pitch  = fb_info->screen_pitch;
		fb_info_tmp.fb_base_offset = 0;
		fb_info_tmp.pixel_format = IGD_PF_xRGB32;
		fb_info_tmp.flags = IGD_SURFACE_DISPLAY | IGD_SURFACE_RENDER;

		ret_val = display->context->dispatch.gmm_alloc_surface(
				&fb_info_tmp.fb_base_offset,
				fb_info_tmp.pixel_format,
				&fb_info_tmp.width,
				&fb_info_tmp.height,
				&fb_info_tmp.screen_pitch,
				&size,
				IGD_GMM_ALLOC_TYPE_NORMAL,
				&fb_info_tmp.flags);
	}
	/* Temp plane is always 32 bpp */
	temp_control = EMGD_READ32(MMIO(display)
		+ PLANE(display)->plane_reg);
	plane_control = temp_control;
	plane_control &= ~(BIT30 | BIT29 | BIT28 | BIT27);
	plane_control |= BIT28 | BIT27;
	EMGD_WRITE32(plane_control, MMIO(display)
		+ PLANE(display)->plane_reg);
	temp_pitch = EMGD_READ32(MMIO(display)
		+ PLANE(display)->plane_reg + DSP_STRIDE_OFFSET);
	EMGD_WRITE32(fb_info_tmp.screen_pitch, MMIO(display)
		+ PLANE(display)->plane_reg + DSP_STRIDE_OFFSET);
	temp_plane = EMGD_READ32(MMIO(display)
		+ PLANE(display)->plane_reg + DSP_START_OFFSET);
	EMGD_WRITE32(fb_info_tmp.fb_base_offset,
		MMIO(display) + PLANE(display)->plane_reg + DSP_START_OFFSET);
	/* Setup front buffer */
	OS_MEMSET(&front_buffer,0,sizeof (igd_surface_t));
	front_buffer.width  = fb_info_tmp.width;
	front_buffer.height = fb_info_tmp.height;
	front_buffer.pitch  = fb_info_tmp.screen_pitch;
	front_buffer.offset = fb_info_tmp.fb_base_offset;
	front_buffer.pixel_format = fb_info_tmp.pixel_format;
	front_buffer.flags = fb_info_tmp.flags;
	/* Draw 2 color bars for CRC calculation. This is simple and sufficient
	 * to detect clipping. Will consider complicated color bar if an issue
	 * is found for now go for quickest methodColor bar. Red and Blue */
	dst_rect.x1 = dst_rect.y1 = 0;
	dst_rect.x2 = fb_info_tmp.width/2;
	dst_rect.y2 = fb_info_tmp.height;

	/*
	// NOT supported in koheo, using software for now
	display->context->dispatch.color_blt((igd_display_h)display,
				IGD_PRIORITY_NORMAL,
				&front_buffer,
				&dst_rect,
				IGD_ALPHA_CHANNEL | IGD_RGB_CHANNEL,
				0x00FF0000,
				0xF0,
				(igd_appcontext_h)0, IGD_RENDER_BLOCK);
	*/

	{
		unsigned long row, col;
		unsigned char *buf = NULL;
		unsigned long height = fb_info_tmp.height;
		unsigned long width = fb_info_tmp.width/2;
		unsigned long pitch = fb_info_tmp.screen_pitch;

		buf = display->context->dispatch.gmm_map(fb_info_tmp.fb_base_offset);
		for (row = 0; row < (height); row++) {
			for (col = 0; col < (width); col++) {
				EMGD_WRITE32(0x00FF0000, (buf + (col*4)));
			}
			buf = buf + pitch;
		}
		display->context->dispatch.gmm_unmap(buf);
	}

	dst_rect.x1 = fb_info_tmp.width/2;
	dst_rect.y1 = 0;
	dst_rect.x2 = fb_info_tmp.width;
	dst_rect.y2 = fb_info_tmp.height;

	/*
	// NOT supported in koheo, using software for now
	display->context->dispatch.color_blt((igd_display_h)display,
				IGD_PRIORITY_NORMAL,
				&front_buffer,
				&dst_rect,
				IGD_ALPHA_CHANNEL | IGD_RGB_CHANNEL,
				0x000000FF,
				0xF0,
				(igd_appcontext_h)0, IGD_RENDER_BLOCK);
		wa->flag |= TNC_HTOTAL_TUNED;
		pTimings->reserved_dd = (wa->htotal | TNC_HTOTAL_TUNED);
		return FALSE;

	display->context->dispatch.sync((igd_display_h)display, IGD_PRIORITY_NORMAL,
		&sync,
		IGD_SYNC_NONBLOCK);
	*/

	{
		unsigned long row, col;
		unsigned char *buf = NULL;
		unsigned long height = fb_info_tmp.height;
		unsigned long width = fb_info_tmp.width;
		unsigned long pitch = fb_info_tmp.screen_pitch;

		buf = display->context->dispatch.gmm_map(fb_info_tmp.fb_base_offset);
		for (row = 0; row < (height); row++) {
			for (col = fb_info_tmp.width/2; col < (width); col++) {
				EMGD_WRITE32(0x000000FF, (buf + (col*4)));
			}
			buf = buf + pitch;
		}
		display->context->dispatch.gmm_unmap(buf);
	}


	ret_val = -IGD_ERROR_BUSY;
	timeout = OS_SET_ALARM(500);
	while (-IGD_ERROR_BUSY == ret_val && (!OS_TEST_ALARM(timeout))){
		ret_val = display->context->dispatch.sync(
			(igd_display_h)display,
			IGD_PRIORITY_NORMAL,
			&sync,
			IGD_SYNC_BLOCK);
		OS_SCHEDULE();
		if (0 == ret_val) {
			break;
		}
	}

	/* Turn on HW CRC */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_CRC_CTRL_REG, 0x80000000);
	/* Reset Status Bit */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, 0x71024);
	temp |= 0x00001000;
	WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, 0x71024, temp);
	temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x71024);
	temp |= 0x00001000;
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, 0x71024, temp);

	/* Calculate CRC pixel by reading every single pixel.
	 * Pixel that is more than the timing range is ignore which happens
	 * during panning. To save time CRC is only calculated once per timing
	 * change and kept inside a scratch global variable wa.
	 */
	if(wa->crc_red == 0 || wa->crc_green == 0 ||
		wa->crc_blue == 0 ){
		unsigned char *fb_adr;

		fb_adr = display->context->dispatch.gmm_map(fb_info_tmp.fb_base_offset);

		/* Calculate CRC for this timing */
		for(i=0; i<(int)pTimings_tmp.height; i++){
			for(j=0; j<(int)pTimings_tmp.width; j++){
				if(i < (int)fb_info_tmp.height && j < (int)fb_info_tmp.width){
					/* virt_fb_adr not available anymore	*/
					/*
					curr_pixel = (unsigned long*)(
						mode_context->context->device_context.virt_fb_adr +
						fb_info_tmp.fb_base_offset + j*bps + (i*fb_info_tmp.screen_pitch));
					*/

					curr_pixel = (unsigned long*)(fb_adr + (j*bps) + (i*fb_info_tmp.screen_pitch));

					if(curr_pixel == NULL){
						EMGD_DEBUG("FB not valid return true for now util FB is allocated");
						return TRUE;
					}



					/*
					blue_val.pixel =  (0x0000ff & *curr_pixel) >> 0;
					green_val.pixel = (0x00ff00 & *curr_pixel) >> 8;
					red_val.pixel = (0xff0000 & *curr_pixel) >> 16;
					*/

					blue_val.pixel =  (0x0000ff & EMGD_READ32(curr_pixel)) >> 0;
					green_val.pixel = (0x00ff00 & EMGD_READ32(curr_pixel)) >> 8;
					red_val.pixel = (0xff0000 & EMGD_READ32(curr_pixel)) >> 16;

				}else{
					red_val.pixel = green_val.pixel = blue_val.pixel = 0;
				}

				blue_crc = compute_pixel_crc(blue_crc, blue_val);
				green_crc = compute_pixel_crc(green_crc, green_val);
				red_crc = compute_pixel_crc(red_crc, red_val);
			}
		}
		display->context->dispatch.gmm_unmap(fb_adr);

		/* Save calculated value */
		wa->crc_red = red_crc.pixel;
		wa->crc_green = green_crc.pixel;
		wa->crc_blue = blue_crc.pixel;
	}else{
		/* Reuse CRC value calculated before */
		red_crc.pixel = wa->crc_red;
		green_crc.pixel = wa->crc_green;
		blue_crc.pixel = wa->crc_blue;
	}

	timeout = OS_SET_ALARM(70);
	do {
		temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x71024);
		if(temp & 0x00002000){
			break;
		}
		OS_SCHEDULE();
	} while ((!OS_TEST_ALARM(timeout)));
	/* Give some time for CRC to be stable */
	OS_SLEEP(500);
	/* Read CRC on LNC and OVL */
	tnc_crc_red = READ_MMIO_REG_TNC(IGD_PORT_SDVO,0x61060);
	tnc_crc_green = READ_MMIO_REG_TNC(IGD_PORT_SDVO,0x61064);
	tnc_crc_blue = READ_MMIO_REG_TNC(IGD_PORT_SDVO,0x61068);

	/* Revert back to original plane */
	EMGD_WRITE32(temp_control, MMIO(display)
		+ PLANE(display)->plane_reg);
	EMGD_WRITE32(temp_pitch, MMIO(display)
		+ PLANE(display)->plane_reg + DSP_STRIDE_OFFSET);
	EMGD_WRITE32(temp_plane, MMIO(display)
		+ PLANE(display)->plane_reg + DSP_START_OFFSET);

	/* Turn off CRC */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_CRC_CTRL_REG, 0x00000000);
	/* Compare CRC calculation with Atom E6xx CRC, return false if a mismatch and
	 * tuning continues. If it matches indicate the flag as tuned and return
	 * false anyway so pipe programming would program based on the correct
	 * timing spec.
	 */
	if((red_crc.pixel != (unsigned int)tnc_crc_red) ||
		(green_crc.pixel != (unsigned int)tnc_crc_green) ||
		(blue_crc.pixel != (unsigned int)tnc_crc_blue)){

			/* Step by step tuning */
		wa->delta = 1;

		/* Calculate the delta value */
		wa->htotal = (wa->htotal + wa->delta);

		EMGD_DEBUG("Counter %d",wa->counter);
		EMGD_DEBUG("Red CRC: %d Red TNC: %lu",red_crc.pixel,tnc_crc_red);
		EMGD_DEBUG("Grn CRC: %d Grn TNC: %lu",green_crc.pixel,tnc_crc_green);
		EMGD_DEBUG("Blu CRC: %d Blu TNC: %lu",blue_crc.pixel,tnc_crc_blue);
		EMGD_DEBUG("CRC does not match, tuning....");
		EMGD_TRACE_EXIT;
		return FALSE;
	}
	EMGD_ERROR("Tuned value found.");
	EMGD_ERROR("Total tuning required %d",wa->counter);
	EMGD_DEBUG("Red CRC: %d Red TNC: %lu",red_crc.pixel,tnc_crc_red);
	EMGD_DEBUG("Grn CRC: %d Grn TNC: %lu",green_crc.pixel,tnc_crc_green);
	EMGD_DEBUG("Blu CRC: %d Blu TNC: %lu",blue_crc.pixel,tnc_crc_blue);
	wa->flag |= TNC_HTOTAL_TUNED;
	pTimings->reserved_dd = (wa->htotal | TNC_HTOTAL_TUNED);

	EMGD_TRACE_EXIT;
	/* We are tuned but lets do another sequence since we may have changed
	 * the blanks to get a valid CRC. This happens on certain modes that has
	 * weird blanks value like VESA's 640x480@60
	 */
	return FALSE;
}
#endif

/*!
 *
 * @param mmio dev2 mmio
 *
 * @return void
 */
void disable_vga_tnc (unsigned char *mmio)
{
	unsigned long temp;
	unsigned char sr01;

	EMGD_TRACE_ENTER;

	/* Disable VGA plane if it is enabled. */
	temp = EMGD_READ32(EMGD_MMIO(mmio) + VGACNTRL);
	if ((temp & BIT31) == 0) {
		/* Read SR01 */
		READ_VGA(mmio, SR_PORT, SR01, sr01);

		/* Turn on SR01 bit 5 */
		WRITE_VGA(mmio, SR_PORT, SR01, sr01|BIT(5));

		/* Wait for 30us */
		OS_SLEEP(30);

		temp |= BIT31;     /* set bit 31 to disable */
		temp &= ~BIT30;    /* clear bit 30 to get VGA display in normal size */
		EMGD_WRITE32(temp, EMGD_MMIO(mmio) + VGACNTRL);

	}

	EMGD_TRACE_EXIT;
}

/*!
 *
 * @param display_handle
 * @param palette_entry
 * @param palette_color
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int igd_set_palette_entry_tnc(
	igd_display_h display_handle,
	unsigned long palette_entry,
	unsigned long palette_color)
{
	/* Return if Pipe is not on */
	if(!((1L<<31) & READ_MMIO_REG_TNC(PORT_TYPE_DH(display_handle),
			PIPE(display_handle)->pipe_reg))) {
		return -IGD_INVAL;
	}
	/* Palette can is only on 0:2:0 so use _TNC IGD_PORT_LVDS */
	WRITE_MMIO_REG_TNC(IGD_PORT_LVDS,
		PIPE(display_handle)->palette_reg + palette_entry * 4,
		palette_color);

	return 0;
}

/*!
 *
 * @param display_handle
 * @param palette_entry
 * @param palette_color
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int igd_get_palette_entry_tnc(
	igd_display_h display_handle,
	unsigned long palette_entry,
	unsigned long *palette_color)
{
	/* Return if Pipe is not on */
	if(!((1L<<31) & READ_MMIO_REG_TNC(
			PORT_TYPE_DH(display_handle),
			PIPE(display_handle)->pipe_reg))) {
		return -IGD_INVAL;
	}
	/* Palette can is only on 0:2:0 so use _TNC IGD_PORT_LVDS */
	*palette_color = 0xffffff & READ_MMIO_REG_TNC(
		IGD_PORT_LVDS,
		PIPE(display_handle)->palette_reg + palette_entry * 4);
	return 0;
}

/*!
 *
 * @param mmio
 * @param pipe_reg
 *
 * @return 0 on success
 * @return 1 on failure
 */
int wait_for_vblank_tnc(unsigned long pipe_reg)
{
	unsigned long pipe_status_reg = pipe_reg + PIPE_STATUS_OFFSET;
	unsigned long tmp;
	unsigned long port_type = IGD_PORT_LVDS;
	os_alarm_t timeout;
	int ret;
	unsigned long request_for;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Parameter: pipe_reg = %lx", pipe_reg);

	/* If pipe is off then just return */
	if(!((1L<<31) & READ_MMIO_REG_TNC(port_type, pipe_reg))) {
		EMGD_DEBUG("Pipe disabled/Off");
		EMGD_TRACE_EXIT;
		return 1;
	}

	/*
	 * When VGA plane is on the normal wait for vblank won't work
	 * so just skip it. VGA plane is on 0:2:0 so no need to use _TNC macros.
	 */
	if(!(EMGD_READ32(MMIO_TNC(port_type) + 0x71400) & 0x80000000)) {
		EMGD_DEBUG("VGA Plane On");
		EMGD_TRACE_EXIT;
		return 1;
	}

	/* 1. Request the interrupt handler to record the next VBlank: */
	request_for = VBINT_REQUEST(VBINT_WAIT,
		(pipe_status_reg == PIPEA_STAT) ? VBINT_PORT4 : VBINT_PORT2);
	mode_context->dispatch->full->request_vblanks(request_for,
		MMIO_TNC(port_type));

	/* 2. Wait (about 50 msec, 20Hz) & poll for the next VBlank: */
	timeout = OS_SET_ALARM(50);
	do {
		OS_SCHEDULE();
		tmp = mode_context->dispatch->full->vblank_occured(request_for);
	} while ((tmp == 0x00) && (!OS_TEST_ALARM(timeout)));


	if (tmp == 0) {
		EMGD_ERROR_EXIT("Timeout waiting for VBLANK");
		ret = 0;
	} else {
		ret = 1;
	}

	/* 3. End our request for the next VBlank: */
	mode_context->dispatch->full->end_request(request_for,
		MMIO_TNC(port_type));


	EMGD_TRACE_EXIT;
	return ret;
} /* wait_for_vblank_tnc */



/*!
 * This procedure waits for the next vertical blanking (vertical retrace)
 * period. If the display is already in a vertical blanking period, this
 * procedure exits.
 *
 * Note: A timeout is included to prevent an endless loop.
 *
 * @param display_handle
 *
 * @return FALSE if timed out
 */
int igd_wait_vblank_tnc(igd_display_h display_handle)
{

	return wait_for_vblank_tnc(PIPE(display_handle)->pipe_reg);

}


/*!
 * Get the stride and stereo values based on the display.  This is also used
 * by the MI instructions.
 *
 * @param display Pointer to hardware device instance data
 * @param flags Should the stereo be for the frontbuffer or backbuffer?
 *
 * @return stride - Stride of the display
 * @return stereo - Stereo address of the display
 */
int mode_get_stride_stereo_tnc(igd_display_context_t *display,
	unsigned long *stride,
	unsigned long *stereo,
	unsigned long flags)
{
	unsigned long pitch = PLANE(display)->fb_info->screen_pitch;
	igd_timing_info_t *timing = PIPE(display)->timing;
	unsigned long base_offset;

	EMGD_TRACE_ENTER;

	base_offset = PLANE(display)->fb_info->visible_offset;
	*stride = pitch;
	*stereo = 0;

	/* For field replication, valid for interlaced modes only
	 *     set stereo = fb_base
	 *         stride = pitch
	 */
	if (timing->mode_info_flags & IGD_SCAN_INTERLACE) {

		if(timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* Interlaced + Line double flags means field replication.
			 * same lines are sent for both fields. Program the
			 * second eye to be same as the first
			 */
			*stereo = base_offset;
		} else {
			/* Regular interlaced. Second eye starts on line 2.
			 * Skip every other line.
			 */
			*stereo = base_offset + pitch;
			*stride = pitch * 2;
		}
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*
 * Atom E6xx LVDS display: Mode switch sequence
 *
 * 1. Enable sequence
 *
 * Program power on delay, power off delay, power cycle delay registers
 * Program backlight control register to set appropriate backlight value
 * Pipe A must be completely off at this point
 * Write PIPEACONF bits[19:18] = 00
 * Write DSPACNTR bit[31] = 1
 * Write DSPASURF = 0x00000000
 * Write DSPACNTR bit[31] = 0
 * Write DSPASURF = 0x00000000
 * Restore PIPEACONF bits[19:18]  to original value
 * Program DPLL
 * Enable DPLL
 * Wait for DPLL warm up 10us and check for DPLL lock bit in Pipe A config reg
 * (Wait ensures clock is running smoothly before enabling pipe)
 * Program pipe timings (Can be done before DPLL programming)
 * Enable panel fitter as needed (Can be done before DPLL and/or
 *     pipe timing programming)
 * Enable pipe
 * Enable planes (VGA or HiRes)
 * Enable ports
 * Enable panel power (Can be done before DPLL programming)
 *
 * 2. Disable sequence
 *
 * Disable panel backlight
 * Disable panel power (for AOAC standby)
 * Disable ports
 * Disable planes (VGA or hires)
 * Disable pipe
 * Disable VGA display in 0x71400 bit 31
 * (Disable VGA display done after disable pipe to allow pipe to turn off
 *     when no vblank is available in native VGA mode)
 * Wait for pipe off status
 * (Wait ensures planes and pipe have completely turned off prior to
 *     disabling panelfitter then DPLL)
 * Disable panelfitter
 * Disable DPLL
 * Pipe timings change or change between VGA native or VGA center/upperleft
 *     or HiRes
 * Use complete disable sequence followed by complete enable sequence with
 *     new mode programmings.
 *
 *
 * Atom E6xx SDVO display: Display Pipe B Enable/Disable sequence:
 *
 * 1. Enable sequence
 *
 * Program LNC and LNW DPLL
 * Write all planes, port, power control registers on both LNC and LNW
 * 1st Enable LNC pipe
 * 2nd Enable LNW pipe
 *
 * 2. Disable Sequence
 *
 * Disable panel power
 * Disable ports
 * Disable planes
 * 1st Disable LNC pipe
 * 2nd Disable TNC_SDVO pipe
 * Disable panel fitter
 * Disable DPLL
 */


/*!
 *
 * @param display Pointer to hardware device instance data
 *
 * @return void
 */
void program_pipe_vga_tnc(
	igd_display_context_t *display)
{
	igd_timing_info_t *timing;
	unsigned long vga_control;
	unsigned long upscale = 0;
	int centering = 1;

	EMGD_TRACE_ENTER;

	/*
	 * VGA Plane can attach to only one pipe at a time. LVDS can
	 * only attach to pipe B. We need to use the display passed to
	 * determine the pipe number to use. (Napa is same as Alm).
	 */

	/*
	 * We can come here with following cases:
	 *   1. magic->vga    CRT, DVI type displays
	 *   2. native->vga   int-lvds, and up-scaling lvds displays
	 *   3. pipe->vga     TV and other unscaled-lvds displays
	 */
	vga_control = EMGD_READ32(MMIO(display) + 0x71400);
	vga_control &= 0x18e3ff00;
	vga_control |= 0x8e;

	timing = PIPE(display)->timing;
	if(!timing->extn_ptr) {
		EMGD_ERROR_EXIT("No Extension pointer in program_pipe_vga_tnc");
		return;
	}

	/* Find UPSCALING attr value*/
	pi_pd_find_attr_and_value(PORT_OWNER(display),
			PD_ATTR_ID_PANEL_FIT,
			0,/*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);
	/* this PI func will not modify value of upscale if attr does not exist */

	/* magic->vga || native->vga cases, centering isn't required */
	if ((timing->width == 720 && timing->height == 400) || upscale) {
		EMGD_DEBUG("Centering = 0");
		centering = 0;
	}

	/* Enable border */
	if((timing->width >= 800) && !upscale) {
		EMGD_DEBUG("Enable VGA Border");
		vga_control |= (1L<<26);
	}

	if(timing->width == 640) {
		EMGD_DEBUG("Enable Nine Dot Disable");
		vga_control |= (1L<<18);
	}

	if(centering) {
		EMGD_DEBUG("Enable VGA Center Centering");
		vga_control |= 1L<<24;

		if(timing->height >= 960) {
			if(timing->width >= 1280) {
				EMGD_DEBUG("Enable VGA 2x (Nine Dot Disable)");
				vga_control |= (1L<<30) | (1L<<18);
			}
		}
	} else {
		if(PORT_OWNER(display)->port_type == IGD_PORT_LVDS) {
			EMGD_DEBUG("Enable VGA Upper-Left Centering & Nine Dot Disable");
			vga_control |= (1L<<25 | (1L<<18));
		} else if (upscale) {
			EMGD_DEBUG("Enable VGA Center Upper-left for upscale ports");
			vga_control |= 1L<<25;
		}
	}

	if(PIPE(display)->pipe_num) {
		vga_control |= 1L<<29;
	}

	program_pipe_vga(display, (igd_timing_info_t *)timing->extn_ptr);
	EMGD_WRITE32(vga_control, MMIO(display) + 0x71400);

	EMGD_TRACE_EXIT;
	return;
}


#ifndef CONFIG_MICRO
extern os_pci_dev_t bridge_dev;
int read_bunit_tnc(unsigned long reg, unsigned long *val)
{
	reg <<= 8;
	reg &= 0x0000ff00;
	reg |= 0xD00300F0;
  	 
	EMGD_TRACE_ENTER;
  	 
	if(OS_PCI_WRITE_CONFIG_32(bridge_dev, 0xD0, reg)) {
		EMGD_ERROR_EXIT("Writing into the MCR Failed\n");
		return -IGD_ERROR_INVAL;
	}
  	 
	if(OS_PCI_READ_CONFIG_32(bridge_dev, 0xD4, val)) {
		EMGD_ERROR_EXIT("Reading from MDR Failed\n");
		return -IGD_ERROR_INVAL;
	}
  	 
	EMGD_TRACE_EXIT;
  	 
	return 0;
}
  	 
int write_bunit_tnc(unsigned long reg, unsigned long val)
{
	reg <<= 8;
	reg &= 0x0000ff00;
	reg |= 0xE00300F0;
  	 
	EMGD_TRACE_ENTER;
  	 
	if(OS_PCI_WRITE_CONFIG_32(bridge_dev, 0xD4, val)) {
			EMGD_ERROR_EXIT("Writing into the MDR Failed\n");
		return -IGD_ERROR_INVAL;
	}
  	 
	if(OS_PCI_WRITE_CONFIG_32(bridge_dev, 0xD0, reg)) {
		EMGD_ERROR_EXIT("Writing into the MCR Failed\n");
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return 0;
}
#endif

/*!
 * Program Display Plane Values.
 *
 * @param display Pointer to hardware device instance data
 * @param status
 *
 * @return void
 */
void program_plane_tnc(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long stride;
	unsigned long stereo;
	unsigned long plane_control;
	unsigned long other_plane_reg;
	igd_timing_info_t *timing;
	igd_framebuffer_info_t *fb_info = PLANE(display)->fb_info;
	unsigned long plane_reg = PLANE(display)->plane_reg;
	igd_timing_info_t *pipe_timing;
#ifndef  CONFIG_MICRO
	unsigned long ulreg = 0;
#endif
	unsigned long b_reg;
	/* tnc_wa_timing_t *wa = WA_TUNE; */

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program Plane: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	igd_wait_vblank_tnc((igd_display_h)display);

	plane_control = EMGD_READ32(MMIO(display) + plane_reg);
	if(PLANE(display)->plane_reg == DSPACNTR) {
		plane_control &= device_data_tnc->plane_a_preserve;
	} else { /* if it's plane b or plane c */
		plane_control &= device_data_tnc->plane_b_c_preserve;
	}

	/* TODO: Bspec: For EagleLake this Trickle Feed must always disable */

	if((status == FALSE) ||
		(GET_DEVICE_POWER_STATE(display) != IGD_POWERSTATE_D0)) {

		/*
		 * Note: The vga programming code does not have an "off". So
		 * when programming the plane to off we make sure VGA is off
		 * as well.
		 */
		disable_vga_tnc(MMIO(display));

		/*
		 * To turn off plane A or B, the program have to triger the plane A or B
		 * start register.  Or else, it will not work.
		 */
		EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
		EMGD_WRITE32(EMGD_READ32(MMIO(display) + plane_reg + DSP_START_OFFSET),
			MMIO(display) + plane_reg + DSP_START_OFFSET);

		igd_wait_vblank_tnc((igd_display_h)display);
		EMGD_TRACE_EXIT;
		return;
	}
	/*
	 * Note: The very first pass through this function will be with
	 * status false and timings == NULL. Don't use the timings before
	 * the check above.
	 */
	timing = PIPE(display)->timing;
	pipe_timing = timing;
	/* There is a special case code for legacy VGA modes */
	while (timing->extn_ptr) {
		timing = (igd_timing_info_t *)timing->extn_ptr;
	}
	if(MODE_IS_VGA(timing) && CHECK_VGA(pipe_timing)) {
		program_plane_vga(display, timing);
		EMGD_TRACE_EXIT;
		return;
	}

	disable_vga_tnc(MMIO(display));

	/* enable plane, select pipe, enable gamma correction logic */
	plane_control |= 0x80000000 | (PIPE(display)->pipe_num<<24);
	PIPE(display)->plane = PLANE(display);
#ifndef CONFIG_MICRO
	plane_control |= (1<<30);
#endif

	/* Here the settings:
	 *   If line + pixel dbling, set 21,20 to 01b, and set Horz Multiply
	 *   If line dbling only,    set 21,20 to 11b
	 *   If pixel dbling only,   set 21,20 to 00b, but set Horz Multiply
	 *   If no doubling,         set 21,20 to 00b (no Horz Multiply)
	 * For pixel doubling
	 *           --> both progressive/interlaced modes
	 * For Line doubling
	 *           --> progressive modes only
	 */

	if (!(timing->mode_info_flags & IGD_SCAN_INTERLACE)) {
		/* Line doubling in progressive mode requires special bits */
		if (timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* BIT 20 for line & pixel doubling*/
			plane_control |= BIT20;
			/* check later, if no pixel doubling, set bit 21 too*/
		}
	}
	if (timing->mode_info_flags & IGD_PIXEL_DOUBLE) {
		/* For line ONLY doubling, set bit 21 also '1' */
		plane_control |= BIT21;
	}

	mode_get_stride_stereo_tnc(display, &stride, &stereo, 0);

	/* set color depth */
	switch (IGD_PF_DEPTH(fb_info->pixel_format)) {
	case PF_DEPTH_8:
		plane_control |= BIT27 | BIT30;
		break;
	case PF_DEPTH_16:
		plane_control |= BIT28 | BIT26;
		break;
	default:
	case PF_DEPTH_32:
		plane_control |= BIT28 | BIT27;
		break;
	}

	if(fb_info->flags & IGD_ENABLE_DISPLAY_GAMMA) {
		plane_control |= (BIT30);
	}

	if(fb_info->flags & IGD_SURFACE_TILED) {
		plane_control |= (BIT10);
	}

	/* Set watermark for Atom E6xx */
#ifndef  CONFIG_MICRO
	if (!mode_context->en_reg_override) {
		if (plane_reg == DSPACNTR) {
			other_plane_reg = DSPBCNTR;
		} else {
			other_plane_reg = DSPACNTR;
		}

		if (EMGD_READ32(MMIO(display) + other_plane_reg) & 0x80000000) {
			EMGD_WRITE32(device_data_tnc->dsp_arb, MMIO(display) +
				PIPEA_DISP_ARB_CTRL);
		} else if (plane_reg == DSPACNTR) {
			EMGD_WRITE32(0x00003fff, MMIO(display) + PIPEA_DISP_ARB_CTRL);
		} else {
			EMGD_WRITE32(0x00003f80, MMIO(display) + PIPEA_DISP_ARB_CTRL);
		}

		/*
		 * Setting WM priority level to 11
		 * to workaround display bouncing issues
		 * TODO: Mode set from Clone->Single(Primary)
		 * PIPEA_DISP_ARB_CTRL does not get updated
		 */
		ulreg = EMGD_READ32(MMIO(display) + 0x00020f8);
		EMGD_WRITE32(ulreg | 0x000003f0, MMIO(display) + 0x00020f8);

		EMGD_WRITE32(device_data_tnc->fifo_watermark1, MMIO(display) + FW_1);
		EMGD_WRITE32(device_data_tnc->fifo_watermark2, MMIO(display) + FW_2);
		EMGD_WRITE32(device_data_tnc->fifo_watermark3, MMIO(display) + FW_3);
		EMGD_WRITE32(device_data_tnc->fifo_watermark4, MMIO(display) + FW_4);
		EMGD_WRITE32(device_data_tnc->fifo_watermark5, MMIO(display) + FW_5);
		EMGD_WRITE32(device_data_tnc->fifo_watermark6, MMIO(display) + FW_6);
	} else {
		/* en_reg_override=1 */
		/* Override display registers */
		EMGD_WRITE32(mode_context->gvd_hp_control, MMIO(display) + 0x00020f8);
		EMGD_WRITE32(mode_context->disp_arb, MMIO(display) + PIPEA_DISP_ARB_CTRL);
		EMGD_WRITE32(mode_context->fifo_watermark1, MMIO(display) + FW_1);
		EMGD_WRITE32(mode_context->fifo_watermark2, MMIO(display) + FW_2);
		EMGD_WRITE32(mode_context->fifo_watermark3, MMIO(display) + FW_3);
		EMGD_WRITE32(mode_context->fifo_watermark4, MMIO(display) + FW_4);
		EMGD_WRITE32(mode_context->fifo_watermark5, MMIO(display) + FW_5);
		EMGD_WRITE32(mode_context->fifo_watermark6, MMIO(display) + FW_6);
		EMGD_WRITE32(mode_context->disp_chicken_bits, MMIO(display) + 0x0070400);

		/* Override Bunit Chickenbits and BWFLUSH register */
		write_bunit_tnc(0x30, mode_context->bunit_chicken_bits);
		write_bunit_tnc(0x02, mode_context->bunit_write_flush);
	}

	EMGD_DEBUG(" GVD HP_CONTROL: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + PIPEA_DISP_ARB_CTRL);
	EMGD_DEBUG(" Display Arbitration register: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_1);
	EMGD_DEBUG(" FIFO Watermark Control Register 1: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_2);
	EMGD_DEBUG(" FIFO Watermark Control Register 2: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_3);
	EMGD_DEBUG(" FIFO Watermark Control Register 3: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_4);
	EMGD_DEBUG(" FIFO Watermark Control Register 4: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_5);
	EMGD_DEBUG(" FIFO Watermark Control Register 5: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + FW_6);
	EMGD_DEBUG(" FIFO Watermark Control Register 6: 0x%lx", ulreg);
	ulreg = EMGD_READ32(MMIO(display) + DSP_CHICKENBITS);
	EMGD_DEBUG(" Display Chicken bits: 0x%lx", ulreg);
	read_bunit_tnc(0x30, &b_reg);
	EMGD_DEBUG(" Bunit Chicken bits: 0x%lx", b_reg);
	read_bunit_tnc(0x02, &b_reg);
	EMGD_DEBUG(" Bunit Write Flush: 0x%lx", b_reg);
#else
	/* ITP Script is doing this and so go ahead */
	/* The PIPEA_DISP_ARB_CTRL set fixed the issue with 32bit vesa modes */
	EMGD_WRITE32(0x00001FBF, MMIO(display) + PIPEA_DISP_ARB_CTRL);
	EMGD_WRITE32(0x3F8F0F18, MMIO(display) + FW_1);
#endif
	/* FIXME: Not required for TNC.
	 * The B-Spec states that rendering will be slower if the fences are not
	 * a power of 2.  So for now, always use a power of 2. */
	/* EMGD_WRITE32(0x04000400, MMIO(display) + 0x209c); */

	EMGD_DEBUG(" Plane Control: 0x%lx", plane_control);
	EMGD_DEBUG(" Plane Base:    0x%lx", fb_info->visible_offset);
	EMGD_DEBUG(" Plane Pitch:   0x%lx", stride);

	EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
	EMGD_WRITE32(stride, MMIO(display) + plane_reg + DSP_STRIDE_OFFSET);
	/* Both of these registers are Reserved on Gen4 */
	/*EMGD_WRITE32(size, MMIO(display) + plane_reg + DSP_SIZE_OFFSET);*/
	/*EMGD_WRITE32(stereo, MMIO(display) + plane_reg + DSP_STEREO_OFFSET);*/
	EMGD_WRITE32(0, MMIO(display) + plane_reg + DSP_LINEAR_OFFSET);
	EMGD_WRITE32(0, MMIO(display) + plane_reg + 0x24);
	EMGD_WRITE32(fb_info->visible_offset,
		MMIO(display) + plane_reg + DSP_START_OFFSET);

	igd_wait_vblank_tnc((igd_display_h)display);

	EMGD_TRACE_EXIT;
}

/*!
 * PGen4 can check when the pipe is enabled or disabled.
 * This function waits for the pipe to be enabled or disabled.
 * check_on_off = 0 to wait for the pipe to disable.
 * check_on_off = 0x40000000 to wait for the pipe to enable.
 *
 * @param mmio
 * @param pipe_reg
 * @param check_on_off
 *
 * @return void
 */
void wait_pipe(unsigned long pt, unsigned long pipe_reg, unsigned long check_on_off)
{
	unsigned long temp;
	os_alarm_t timeout;

	EMGD_TRACE_ENTER;

	/* 0:3:0 doesn't wait pipe, only LNC device does. */
	if (pt == IGD_PORT_SDVO) {
		return;
	}

	/* Wait for Pipe enable/disable, about 50 msec (20Hz). */
	timeout = OS_SET_ALARM(50);
	do {
		OS_SCHEDULE();
		temp = EMGD_READ32(MMIO_TNC(IGD_PORT_LVDS) + pipe_reg) & 0x40000000;
		/* Check for timeout */
	} while ((temp != check_on_off) && (!OS_TEST_ALARM(timeout)));

	if (temp != check_on_off) {
		EMGD_ERROR_EXIT("Timeout waiting for pipe enable/disable");
	}

	EMGD_TRACE_EXIT;
	return;
}

unsigned long get_port_type(int crtc_id) {
	if (crtc_id == IGD_KMS_PIPEA) {
		return IGD_PORT_LVDS;
	}
	if (crtc_id == IGD_KMS_PIPEB) {
		return IGD_PORT_SDVO;
	}
	EMGD_ERROR("Unrecognized port type based on crtc_id of %d", crtc_id);
	return 0;
}

/*!
 * This function programs the Timing registers and clock registers and
 * other control registers for PIPE.
 *
 * @param display
 * @param status
 *
 * @return void
 */
void program_pipe_tnc(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long   timing_reg;
	unsigned long   pipe_conf;
	unsigned long   hactive, vactive;
	igd_timing_info_t  *pTimings;
	igd_timing_info_t  pTimings_tmp;
	igd_display_port_t *port;
	unsigned long temp;
	unsigned long pt = PORT_TYPE(display);
	unsigned long dc;
	unsigned long calc;
	short hactive_tmp, vactive_tmp;
	int i;
	tnc_wa_timing_t *wa;
	/* igd_framebuffer_info_t *fb_info = PLANE(display)->fb_info; */
	platform_context_tnc_t *platform_context;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program Pipe: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	platform_context = (platform_context_tnc_t *)display->context->platform_context;

	/* For Windows OS, flag_clip_fix will be overridden by configurable parameter in registry.
	* This section of code is excluded in VBIOS, VBIOS will always use the pre-assigned value.
	*/
	flag_clip_fix = mode_context->clip_hw_fix;

	pipe_conf = device_data_tnc->pipe_preserve &
		READ_MMIO_REG_TNC(pt, PIPE(display)->pipe_reg);

	if((status == FALSE) ||
		(GET_DEVICE_POWER_STATE(display) == IGD_POWERSTATE_D3)) {
		/* For SDVO disable both pipe Bs in 0:2:0 and 0:3:0 */
		if (pt == IGD_PORT_SDVO) {
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PIPE(display)->pipe_reg,
				pipe_conf & (~0x80000000L));
			/* After disable pipe B in 0:2:0, wait_pipe must be
 			 * called to ensure the pipe B to be turned off */
			wait_pipe(IGD_PORT_LVDS, PIPE(display)->pipe_reg, 0);
		}
		/* Disable pipe */
		WRITE_MMIO_REG_TNC(pt, PIPE(display)->pipe_reg,
			pipe_conf & (~0x80000000L));

		/* check when the pipe is disabled. */
		wait_pipe(pt, PIPE(display)->pipe_reg, 0);

		/* Disable DPLL */
		//WRITE_MMIO_REG_TNC(pt, PIPE(display)->clock_reg->dpll_control,
		//	READ_MMIO_REG_TNC(pt,
		//		PIPE(display)->clock_reg->dpll_control) & ~0x80000000L);

		EMGD_TRACE_EXIT;
		return;
	}

	/* For TNC B1, enable hardware cliping fix*/
	if((platform_context->tnc_dev3_rid == TNC_B1_DEV3_RID)&&
		(flag_clip_fix & IGD_CLIP_FIX_GLOBAL_ENABLE)) {
			/* Disable SDVO Pipe in Device 2 and Device 3 */
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PIPE(display)->pipe_reg,
				pipe_conf & (~0x80000000L));
  	 
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPE(display)->pipe_reg,
				pipe_conf & (~0x80000000L));
 
			/* check when the pipe is disabled. */
			wait_pipe(IGD_PORT_LVDS, PIPE(display)->pipe_reg, 0);
  	 
			/* Enable clipping hardware fix */
			temp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS);
			if(flag_clip_fix & IGD_CLIP_FIX_REPLACE_STALL) {
				temp |= BIT18;
			} else {
				temp &= ~BIT18;
			}
			if(flag_clip_fix & IGD_CLIP_FIX_DISABLE_THROTTLE) {
				temp |= BIT15;
			} else {
				temp &= ~BIT15; 
			}
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS, temp);
	}


	port     = PORT_OWNER(display);
	pTimings = PIPE(display)->timing;

	{
		/* Debug messages */
		pd_timing_t *vga_timing = (pd_timing_t *)pTimings->extn_ptr;
		EMGD_DEBUG("pTimings %ux%u mode_number = %u mode_info_flags = 0x%lx, dclk = %lu",
			pTimings->width,
			pTimings->height,
			pTimings->mode_number,
			pTimings->mode_info_flags,
			pTimings->dclk);
		if (vga_timing) {
			EMGD_DEBUG("ext_timing %ux%u mode_number = %u mode_info_flags= 0x%lx, dclk = %lu",
				vga_timing->width,
				vga_timing->height,
				vga_timing->mode_number,
				vga_timing->mode_info_flags,
				vga_timing->dclk);
		}
	}

	/*
	 * If the mode is VGA and the PD says it handles all VGA modes without
	 * reprogramming then just set the mode and leave centering off.
	 */
	if(pTimings->mode_info_flags & IGD_MODE_VESA) {
			EMGD_DEBUG("IGD_MODE_VESA");
		if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
			/* Pipe timings and clocks no longer need to be set since
			 * the VGA timings will be used.
			WRITE_MMIO_REG_TNC(pt, PIPE(display)->pipe_reg,
				pipe_conf | 0x80000000); */

			/* Gen4 can check when the pipe is enabled. No longer needed
			 * since pipe not enabled and VGA timings are used.
			wait_pipe(PIPE(display)->pipe_reg, 0x40000000);*/

			EMGD_DEBUG("pTimings->mode_number <= VGA_MODE_NUM_MAX");
			program_pipe_vga_tnc(display);
			EMGD_TRACE_EXIT;
			return;
		} else {
#ifdef CONFIG_MICRO
			set_256_palette(
				MMIO_TNC(PORT(display, display->port_number)->port_type));
#endif
		}
	}

	/* Program dot clock divisors. */
	program_clock_tnc(display, PIPE(display)->clock_reg, pTimings->dclk);

	/* Program timing registers for the pipe */
	timing_reg = PIPE(display)->timing_reg;
	if (pTimings->mode_info_flags & IGD_PIXEL_DOUBLE) {
		hactive = (unsigned long)pTimings->width*2 - 1;
	} else {
		hactive = (unsigned long)pTimings->width - 1;
	}

	if (pTimings->mode_info_flags & IGD_LINE_DOUBLE) {
		vactive = (unsigned long)pTimings->height*2 - 1;
	} else {
		/* For Atom E6xx Hardware will automatically divide by 2 to
		   get the number of line in each field */
		vactive = (unsigned long)pTimings->height - 1;
	}

#ifndef CONFIG_MICRO
	/* reset the palette */
	for (i = 0; i < 256; i++) {
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PIPE(display)->palette_reg,
			((i<<16) | (i<<8) | i));
	}


	/* apply color correction */
	for( i = 0; PD_ATTR_LIST_END != port->attributes[i].id; i++ ) {

		if ((PD_ATTR_ID_FB_GAMMA      == (port->attributes[i].id)) ||
			(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)) ||
			(PD_ATTR_ID_FB_BRIGHTNESS == (port->attributes[i].id)))  {

			mode_context->dispatch->full->set_color_correct(display);
		}
	}
#endif

	/*
	 * NOTE: For size reasons the timng table contains unsigned short
	 * values. Don't shift them past 16. Use a temp instead.
	 * All register offsets and bit shift are verified for Gen4
	 *
	 * For SDVO display:
	 * Write values into pipe B registers in both 0:2:0 and 0:3:0
	 */

	dc = *(display->context->mod_dispatch.dsp_current_dc);

	wa = WA_TUNE;

	for (i=0; i<2; i++) {
		/* Temp variable */
		pTimings_tmp = *pTimings;
		hactive_tmp = (short) hactive;
		vactive_tmp = (short) vactive;
#ifndef CONFIG_MICRO

		/* Htotal and tuning wa is not supported in VBIOS for TNC				
		 * TNC B0: Enable Htotal formula and CRC Tuning to fix clipping issue and
		 * async flip flikering on single display
		 * TNC B1: Enable Htotal formula without CRC Tuning to fix async flip flikering
		 * For B0, TVOUT is excluded for htotal workaround to avoid image shifting 
		 * issue happen. Image shifting will be fixed on B1 while ref_freq is lower down
		 * to 198Mhz
		 */


		if(	(pt == IGD_PORT_SDVO) && FLAG(flag_basic_htotal_formula_wa) &&
			(   ((platform_context->tnc_dev3_rid == TNC_B1_DEV3_RID) && 
					(mode_context->async_flip_wa)	)||
				(   (platform_context->tnc_dev3_rid == TNC_B0_DEV3_RID) &&
				    (!IGD_DC_SINGLE(dc) || (port->pd_driver->type != PD_DISPLAY_TVOUT))
				)
			)){


	 		if((!(pTimings->reserved_dd & TNC_HTOTAL_TUNED)) &&
	 				(wa->counter < LIMIT_TOTAL_CHECK_DISPLAY) &&
					(platform_context->tnc_dev3_rid == TNC_B0_DEV3_RID) &&
 					(FLAG(flag_enable_tuning_wa))) {

				/* Modify blanks so it always begin after active pixel and ends at the
				 * end. Do not change it if we are already tuned to maintain
				 * original timing specification
				 */
				pTimings_tmp.hblank_start = (short) (pTimings->width - 1);
				pTimings_tmp.vblank_start = (short) (pTimings->height - 1);
				pTimings_tmp.hblank_end = (short) (pTimings->htotal);
				pTimings_tmp.vblank_end = (short) (pTimings->vtotal);
			}

#ifndef CONFIG_MICRO
			/* Delay Frame start while Tuning*/
			/* Frame start delay is added as plane corruption occurs without delay. 
			 * Pipe would begin fetching data from random memory location causing the delay. 
			 * Silicon could not determine why this is happening but agree that adding a delay 
			 * would not cause any issue unless for a DTD with single vsync line which is 
			 * super unlikely used.
			 */

			/* B1 stepping does not need to set this bit because it does not need tuning*/
			if((platform_context->tnc_dev3_rid == TNC_B0_DEV3_RID) && 
				(IGD_DC_CLONE(dc) || IGD_DC_EXTENDED(dc))){
				pipe_conf |= BIT27;
			}
#endif


			if(i==0){
				if(pTimings->reserved_dd == 0 || (wa->counter > LIMIT_TOTAL_CHECK_DISPLAY)){
					/* First time tuning */
					if ((mode_context->ref_freq != 0) && 
							(mode_context->ref_freq >= 190000) && 
							(mode_context->ref_freq <= 210000))
					{
						if ((platform_context->tnc_dev3_rid == TNC_B1_DEV3_RID)){
							/* The optimum reference frequency used for 3DMark flickering workaround formula
							is the LNC clock minus the optimum margin found from experiment */
							calc = (pTimings->htotal * (mode_context->ref_freq - LNC_B1_OPTIMUM_MARGIN));
						} else {
						calc = (pTimings->htotal * mode_context->ref_freq);
						}
					} else {
						if((platform_context->tnc_dev3_rid == TNC_B1_DEV3_RID)){
							calc = (pTimings->htotal * (LNC_CLOCK - LNC_B1_OPTIMUM_MARGIN));
					} else {
						calc = (pTimings->htotal * LNC_CLOCK);
						}

					}

					calc = (calc / (PIPE(display)->clock_reg->actual_dclk));
					calc *= (pTimings->vtotal -1);
					calc /= pTimings->vtotal;
					pTimings_tmp.htotal = (short)calc;
					pTimings->reserved_dd = wa->htotal = pTimings_tmp.htotal;
					EMGD_DEBUG("Delta = %d", wa->htotal);
				}else if (pTimings->reserved_dd & TNC_HTOTAL_TUNED){
					pTimings_tmp.htotal = (short)(pTimings->reserved_dd & (~TNC_HTOTAL_TUNED));
				}else{
					if(wa->htotal == 0)
						wa->htotal = (short)pTimings->reserved_dd;
					pTimings_tmp.htotal = wa->htotal;
				}
				/* Use vphase formula if available */
				if(vphase){
					pTimings_tmp.vtotal -= (short)vphase;
					pTimings_tmp.vsync_start -= (short)vphase;
					pTimings_tmp.vsync_end -= (short)vphase;
					pTimings_tmp.vblank_end -= (short)vphase;
				}else{
					//pTimings_tmp.hblank_start += (short) (pTimings_tmp.htotal - pTimings->htotal);
					pTimings_tmp.hblank_end += (short) (pTimings_tmp.htotal - pTimings->htotal);
				}
			}
		}
#endif

		temp = (unsigned long)(pTimings_tmp.htotal) << 16 | hactive_tmp;
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg, temp);

		temp = ((unsigned long) pTimings_tmp.hblank_end << 16) |
			(unsigned long)(pTimings_tmp.hblank_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x04, temp);

		temp = ((unsigned long)(pTimings->hsync_end) << 16) |
			(unsigned long)(pTimings->hsync_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x08, temp);

		temp = ((unsigned long)(pTimings_tmp.vtotal) << 16) | vactive_tmp;
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x0C, temp);

		temp = ((unsigned long)(pTimings_tmp.vblank_end) << 16) |
			(unsigned long)(pTimings_tmp.vblank_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x10, temp);

		temp = ((unsigned long)(pTimings_tmp.vsync_end)<< 16) |
			(unsigned long)(pTimings_tmp.vsync_start);
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x14, temp);

		/*
		 * If there is a linked mode it is either the VGA or a scaled
		 * mode. If it is scaled then we need to use it as the source size.
		 */
		if(pTimings->extn_ptr) {
			igd_timing_info_t *scaled_timings =
				(igd_timing_info_t *)pTimings->extn_ptr;
			if((scaled_timings->mode_info_flags & IGD_MODE_VESA) &&
				(scaled_timings->mode_number <= VGA_MODE_NUM_MAX)) {
				temp = (hactive << 16) | vactive;
			} else {
				EMGD_DEBUG("scaled_timings->width [%d], scaled_timings->height [%d]\n", scaled_timings->width, scaled_timings->height);
				temp = (unsigned long)scaled_timings->width  - 1;
				temp = (temp << 16) |
					(unsigned long)(scaled_timings->height - 1);
			}
		} else {
			temp = (hactive_tmp << 16) | vactive_tmp;
		}
		WRITE_MMIO_REG_TNC(ports_tnc[i], timing_reg + 0x1C, temp);

		/* Enable pipe */
		pipe_conf |= PIPE_ENABLE;

		/* Put pipe in interlaced mode if requested:
		 *     should only happen for LVDS display if at all. */
		if (pTimings->mode_info_flags & IGD_SCAN_INTERLACE) {
			pipe_conf |= (INTERLACE_EN);
		} else {
			pipe_conf &= ~(INTERLACE_EN);
		}

#ifdef CONFIG_MICRO
		if (pt == IGD_PORT_SDVO) {
			/*
			   Enable the panel fitter as VGA controller in Lincroft
			   is a Panel Fitted VGA controller.
			   LNC only supports panel fitted VGA mode
			   (upper left VGA mode). You need to enable the panel fitter.
			   The timing control will be from the pipe timing generator
			   but not from the VGA timing generator CRTC registers
			   as in the centering mode.
			*/
			WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PFIT_CONTROL, 0xA2000000);
		}
#endif

		WRITE_MMIO_REG_TNC(ports_tnc[i], PIPE(display)->pipe_reg, pipe_conf);
		WRITE_MMIO_REG_TNC(ports_tnc[i], PIPE(display)->pipe_reg, pipe_conf);
		/* For LVDS port, don't touch dev3 registers stop after 1st iteration */
		if (pt == IGD_PORT_LVDS) {
			break;
		}
	}


	/* Gen4 can check when the pipe is enabled. */
	wait_pipe(IGD_PORT_LVDS, PIPE(display)->pipe_reg, 0x40000000);

	/*
	 * Set the VGA address range to 0xa0000 so that a normal (not VGA)
	 * mode can be accessed through 0xa0000 in a 16bit world.
	 */
	WRITE_AR(MMIO(display), 0x10, 0xb);
	WRITE_VGA(MMIO(display), GR_PORT, 0x06, 0x5);
	WRITE_VGA(MMIO(display), GR_PORT, 0x10, 0x1);

	if(pTimings->extn_ptr) {
		/* This means either internal scaling (LVDS) or centered VGA */
		pTimings = pTimings->extn_ptr;
		if(pTimings->extn_ptr) {
			/* This is both the scaled and centered VGA */
			pTimings = pTimings->extn_ptr;
		}
		if (pTimings->mode_info_flags & IGD_MODE_VESA) {
			if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
				program_pipe_vga_tnc(display);
			} else {
#ifdef CONFIG_MICRO
				/*
				 * FIXME: This is not appropriate. This assumes that
				 * CONFIG_MICRO means "This is vBIOS" and programs
				 * the palette. vBIOS IAL should probably just set the
				 * palette itself??
				 */
				set_256_palette(
					MMIO_TNC(PORT(display, display->port_number)->port_type));
#endif
			}
		}
	}

	if (pt == IGD_PORT_SDVO) {
		/*  Enable Chicken Bit */
		/*  Setting BIT6 enable Pipe B Palette Write
         *  to prevent hang during palette write */
		/*  Enable Chicken Bit */
		temp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS);
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, DSP_CHICKENBITS, temp | BIT6);
	}

	EMGD_TRACE_EXIT;
	return;
}

/*!
 *
 * @param context
 *
 * @return void
 */
void reset_plane_pipe_ports_tnc(igd_context_t *context)
{
	igd_plane_t *plane;
	igd_display_pipe_t *pipe;
	igd_display_port_t *port,*tv_port=NULL;
	unsigned long temp;
	unsigned long i, j;
	unsigned char *mmio;
	inter_module_dispatch_t *md;

	EMGD_TRACE_ENTER;

	/*
	 * Disable all plane, pipe and port registers because the
	 * bios may have been using a different set. Only unset the
	 * enable bit.
 */
	mmio = EMGD_MMIO(context->device_context.virt_mmadr);
	md = &context->mod_dispatch;
	/* Turn off LVDS and SDVO ports */
	port = NULL;
	while((port = md->dsp_get_next_port(context, port, 0)) != NULL) {
		/* if the port is TV, then don't set the power to S3 as this causes
		 * blank screen on analog port after killx or cosole mode,
		 * probably because the external clock needs to be on till the pipes and
		 * DPLLs are off
		 */
		if (port->pd_driver) {
			if(port->pd_type == PD_DISPLAY_TVOUT) {
				tv_port = port;
			}else {
				
				/* for CH7036: Code fix Meego hang after killall X 
				 * Basicall, when X enters ACPI D1 state, we want to pass that info to port device (Ch7036)
				 * from reset_plane_pipe_ports_xxx(). 
				 */
				if(context->device_context.power_state == IGD_POWERSTATE_D1) { 
				   /* D1 power state for graphics is requested 
					* Then the power_state status will pass to pd
					*/
					 port->pd_driver->set_power(port->pd_context,context->device_context.power_state);			
				}
				else {
					port->pd_driver->set_power(port->pd_context, IGD_POWERSTATE_D3); 
				}
			}
		}

		if (port->port_type == IGD_PORT_SDVO) {
			shutdown_ST_bridge(context);
		}

		if (port->pd_driver) {
			temp = READ_MMIO_REG_TNC(port->port_type, port->port_reg);
			WRITE_MMIO_REG_TNC(port->port_type, port->port_reg, (temp & ~BIT31));
		}
	}

	/* disable plane C */
	temp = EMGD_READ32(EMGD_MMIO(mmio) + DSPCCNTR);
	if(temp & BIT31) {
		EMGD_WRITE32(0x0, EMGD_MMIO(mmio) + DSPCCNTR);
		EMGD_WRITE32(0x0, EMGD_MMIO(mmio) + DSPCCNTR + DSP_START_OFFSET);
	}

	/*
	 * Gen4 appears to require that plane B be shut off prior to
	 * shutting off plane A.  The normal get_next_plane returns them
	 * in order.  We need to read the list backwards.
	 */
	plane = NULL;
	while ((plane = md->dsp_get_next_plane(context, plane, 1)) != NULL) {
		/*  This section only deals with display planes.
		 *  Leave cursor, VGA, overlay, sprite planes alone since they will
		 *  need a different disable bit/sequence.
		 */
		temp = EMGD_READ32(EMGD_MMIO(mmio) + plane->plane_reg);
		if ((plane->plane_features & IGD_PLANE_DISPLAY)) {
			i = 0x71008;  /* PIPE B */
			if (temp & BIT31) {
				if(plane->plane_reg == DSPACNTR) {
					temp = temp & device_data_tnc->plane_a_preserve;
					i = 0x70008;  /* use i as pipe_reg */
				}
				EMGD_WRITE32((temp & ~BIT31), EMGD_MMIO(mmio) + plane->plane_reg);

				/* The B-Spec is ambiguous on which register is the trigger.
				 * Testing has shown the the surface start address is the
				 * correct trigger to disable the plane.
				 */
				EMGD_WRITE32(0, EMGD_MMIO(mmio)+plane->plane_reg+DSP_START_OFFSET);

				/* Wait for VBLANK to ensure that the plane is really off */
				wait_for_vblank_tnc(i);

				EMGD_DEBUG("Plane disabled 0x%lx", plane->plane_reg);
			}
		} else if ((plane->plane_features & IGD_PLANE_CURSOR)) {
			EMGD_WRITE32((temp & 0xffffffe8),
				EMGD_MMIO(mmio) + plane->plane_reg);
			EMGD_WRITE32(0, EMGD_MMIO(mmio) + plane->plane_reg+4);
		}

	}

	/* Turn off pipes */
	pipe = NULL;
	while ((pipe = md->dsp_get_next_pipe(context, pipe, 0)) != NULL) {
		j = 0;

		/* Is this really required? Just waited for vblank above 2 times */
		wait_for_vblank_tnc(pipe->pipe_reg);

		for (i = 0; i < 2; i++) {
			temp = READ_MMIO_REG_TNC(ports_tnc[i], pipe->pipe_reg);

			if (temp & BIT31) {
				/* Do not turn off Pipe B when shutting down */
				if((context->device_context.power_state
						== IGD_POWERSTATE_UNDEFINED) &&
					(pipe->pipe_reg == PIPEB_CONF)){
					continue;
				}
				WRITE_MMIO_REG_TNC(ports_tnc[i], pipe->pipe_reg,
					(temp & device_data_tnc->pipe_preserve));

				/* Gen4 can check when the pipe is disabled. */
				wait_pipe(ports_tnc[i], pipe->pipe_reg, 0);

				/* Disable VGA display */
				disable_vga_tnc(EMGD_MMIO(mmio));

			}

			/* If current pipe is for sDVO, then iterate for PIPE B in
			 * both 0:2:0 LNC and 0:3:0 Atom E6xx devices */
			if (pipe->pipe_reg == 0x70008L) {
				break;
			}
		}
		/* Disable DPLL:
		 *  LVDS: LNC 0xF014
		 *  SDVO: Atom E6xx 0x6018 */
		temp = READ_MMIO_REG_TNC(ports_tnc[j], pipe->clock_reg->dpll_control);

		if (temp & BIT31) {
			WRITE_MMIO_REG_TNC(ports_tnc[j], pipe->clock_reg->dpll_control,
				temp & ~BIT31);
		}
		j++;
	}
	/* pipes and DPLLs are off, now set the power for TV */
	if(tv_port && tv_port->pd_driver) {
		tv_port->pd_driver->set_power(tv_port->pd_context, IGD_POWERSTATE_D3);
	}

	EMGD_TRACE_EXIT;
} /* end reset_plane_pipe_ports */


void shutdown_ST_bridge(igd_context_t *context) {
	platform_context_tnc_t *platform_context = context->platform_context;

	if (!platform_context->stbridgedev) {
		return;
	}

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18,   0x0106007E);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2000,   0x80);

	pd_usleep(20);
}

void initialize_ST_bridge(igd_context_t *context, igd_display_port_t *port) {
	platform_context_tnc_t *platform_context = context->platform_context;
	pd_port_status_t        port_status;
	unsigned long           tempreg;
	unsigned char           cmdreg;
	unsigned long           val;
	unsigned long           port_output = 0x04000000;
	int                     ret;

	if (!platform_context->stbridgedev) {
		return;
	}

	/* call pd_get_port_status() if exists */
	if (port && port->pd_driver && port->pd_driver->pd_get_port_status) {
		ret = port->pd_driver->pd_get_port_status(port->pd_context, &port_status);
		if (ret == PD_SUCCESS) {
			if (port_status.display_type == PD_DISPLAY_LVDS_EXT) {
				port_output = 0x02000000;
			}
		}
	}

	tempreg = READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x20);
	tempreg |= 0xFFFFFF00;
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x20, tempreg);

	tempreg = READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x1020);
	tempreg |= 0x7;
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x1020, tempreg);

	tempreg = READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x10);
	tempreg |= 0x6;
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x10, tempreg);

	tempreg = READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x4);
	tempreg |= 0x6;
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST_GPIO,0x4, tempreg);

	OS_PCI_READ_CONFIG_8((os_pci_dev_t)platform_context->stbridgedev, 0x4, &cmdreg);
	cmdreg |= 0x7;
	OS_PCI_WRITE_CONFIG_8((os_pci_dev_t)platform_context->stbridgedev, 0x4, cmdreg);

	OS_PCI_READ_CONFIG_8((os_pci_dev_t)platform_context->stgpiodev, 0x4, &cmdreg);
	cmdreg |= 0x7;
	OS_PCI_WRITE_CONFIG_8((os_pci_dev_t)platform_context->stgpiodev, 0x4, cmdreg);

	OS_PCI_READ_CONFIG_8((os_pci_dev_t)platform_context->pcidev1, 0x4, &cmdreg);
	cmdreg |= 0x7;
	OS_PCI_WRITE_CONFIG_8((os_pci_dev_t)platform_context->pcidev1, 0x4, cmdreg);

	/* Write DB control values */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18,   0x0106007E);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2000, 0x00000000);
	pd_usleep(20);

	/* Put Display Bridge in Reset */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18,   0x00F8003E | port_output);
	pd_usleep(20);

	/* SDVO PHY startup sequence */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2800, READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2800) | 0x00000080);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2900, READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2900) | 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2400, READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2400) | 0x00000080);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2500, READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2500) | 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2054, 0x00000063);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2454, 0x00000063);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2854, 0x00000063);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2044, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2444, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2844, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2040, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2440, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2840, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2080, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2480, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2880, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2148, 0x00000050);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2144, 0x00000002);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2000, 0x00000086);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2104, 0x00000020);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2108, 0x0000002B);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2100, 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x20C0, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x24C0, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x28C0, 0x00000000);
	pd_usleep(20);

	do {
		pd_usleep(20);
		val = READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2100);
	} while ((val & 0xC) != 0xC);

	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2000, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2140, 0x0000008D);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2400, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2800, 0x00000000);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x200C, 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2010, 0x0000007F);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x240C, 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2410, 0x0000007F);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x280C, 0x00000013);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2810, 0x0000007F);
	pd_usleep(20);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2080, 0x00000001);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2480, 0x00000001);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2880, 0x00000001);
	pd_usleep(20);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2084, 0x0000000F);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2484, 0x0000000F);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2884, 0x0000000F);
	pd_usleep(20);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2040, 0x00000004);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2440, 0x00000004);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2840, 0x00000004);
	pd_usleep(20);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2044, 0x00000014);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2444, 0x00000014);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2844, 0x00000014);
	pd_usleep(20);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2054, 0x0000002B);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2454, 0x0000002B);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x2854, 0x0000002B);
	pd_usleep(20);

	/*
	 * Enable interrupts. It is optional per STM. Include this line only
	 * when interrupt handler with functionality is implemented for Display
	 * Bridge.
	 */
	//WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x10, 0x00FC0000);
	//pd_usleep(20);

	/* Release Reset from Control register */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18, 0x00F80030 | port_output);
	pd_usleep(20);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18, 0x00F80000 | port_output);
	pd_usleep(20);

	/* Increase driver strength in case OLDI output is enabled */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18, READ_MMIO_REG_TNC(IGD_PORT_SDVO_ST, 0x18) | 0x00001800);
	pd_usleep(20);
}

/*!
 * Status is currently not used
 *
 * @param display
 * @param port_number
 * @param status
 *
 * @return 0 on success
 * @return 1 on failure
 */
int post_program_port_tnc(igd_display_context_t *display,
		unsigned short port_number,
		unsigned long status)
{
	int ret;
	igd_display_port_t *port;
	igd_timing_info_t  *timings;
	unsigned long portreg; /* temp; */
#ifndef CONFIG_MICRO
	unsigned long pt = PORT_TYPE(display);
#endif

	EMGD_TRACE_ENTER;

	port = PORT(display, port_number);
	timings = PIPE(display)->timing;

	/*
	 * The programming found in the common code for all chipsets
	 * has the device programming sequence as follows:
	 *  Port
	 *  Pipe
	 *  Post Port
	 *  Plane
	 * On Gen4, if the port is enabled before the pipe, there is a 10%
	 * chance that the port will not turn on properly.
	 * Due to compatability requires with other chipsets, this workaround
	 * fixes this issue
	 */
	portreg = READ_MMIO_REG_TNC(port->port_type, port->port_reg);
	WRITE_MMIO_REG_TNC(port->port_type, port->port_reg, portreg & ~BIT31);
	WRITE_MMIO_REG_TNC(port->port_type, port->port_reg, portreg);

	if (port->port_type == IGD_PORT_SDVO) {
		initialize_ST_bridge(display->context, port);
	}

	ret = 0;
	/* call post_set_mode() if exists */
	if (port->pd_driver->post_set_mode) {
		#ifndef CONFIG_MICRO
		if (pt == IGD_PORT_SDVO){
			ret = port->pd_driver->post_set_mode(port->pd_context, timings,
				status); /*Needed for LAPIS*/
		} else {
			ret = port->pd_driver->post_set_mode(port->pd_context, timings,
				1<<PIPE(display)->pipe_num);
		}
		#else
			ret = port->pd_driver->post_set_mode(port->pd_context, timings,
				1<<PIPE(display)->pipe_num);
		#endif
		if (ret) {
			EMGD_ERROR_EXIT("PD post_set_mode returned: 0x%x", ret);
		}
	}

	EMGD_TRACE_EXIT;
	return ret;
}

/*!
 *
 * @param display
 * @param port_number
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
int program_port_sdvo_tnc(igd_display_context_t *display,
		unsigned short port_number,
		unsigned long status)
{
	unsigned long port_control;
	unsigned long pd_powerstate = PD_POWER_MODE_D3;
	unsigned long preserve = 0;
	unsigned long upscale = 0;
	igd_timing_info_t  local_timing;
	igd_timing_info_t  *timing;
	unsigned long temp;
	int ret;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program Port: (%s)", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("pd_flags: 0x%lx", PORT(display, port_number)->pd_flags);

	timing = PIPE(display)->timing;

	port_control = preserve & READ_MMIO_REG_TNC(IGD_PORT_SDVO,
			PORT(display, port_number)->port_reg);

	if (status == TRUE) {
		if (!(PORT(display, port_number)->pt_info->flags &
			IGD_DISPLAY_ENABLE)) {
			EMGD_TRACE_EXIT;
			return 0;
		}

		/* Enable VGA syncs for native vga modes */
		if (PORT(display, port_number)->vga_sync == 1) {
			EMGD_DEBUG("VGA sync true, is width x height 720 x 400?");
			if((timing->width == 720) && (timing->height == 400)) {
				EMGD_DEBUG("Modify port control and multi_port_control");
				port_control |= (1L<<15);
			}
		}

		/* Fact that both IGD_ powerstates and PD_ powermodes have
		 * same definitions */
		pd_powerstate = GET_DISPLAY_POWER_STATE(display, port_number);

		if (pd_powerstate == IGD_POWERSTATE_D0) {
			EMGD_DEBUG("Power State: D0");
			/* Upscale */
			pi_pd_find_attr_and_value(PORT(display, port_number),
				PD_ATTR_ID_PANEL_FIT,
				0, /*no PD_FLAG for UPSCALING */
				NULL, /* dont need the attr ptr*/
				&upscale);

			/* Reach the end timing if upscaling is enabled */
			if (timing->extn_ptr && upscale) {
				timing = (pd_timing_t *)timing->extn_ptr;
			}

			local_timing = *timing;
			if (upscale) {
				/* For timings smaller than width 360 and height 200,
				 * double the size. This is because the active area of the mode
				 * is double the size of the resolution for these modes
				 *  - Very tricky huh */
				if (local_timing.width <= 360) {
					local_timing.width <<= 1;
				}
				if (local_timing.height <= 200) {
					local_timing.height <<= 1;
				}
			}

			/* BIT31 - Enable
			 * BIT30 - PIPE B
			 * BIT29 - Stall
			 * BIT7  - Border
			 */
			port_control |= BIT31|BIT30|BIT29|BIT7;

			/* Program cDVO registers:
			 * Keep default values for
			 *     7000h - cDVO control register
			 *     7004h - cDVO slew rate register
			 *     7008h - cDVO strength register
			 *     700Ch - cDVO RCOMP update register
			 *     6102Ch - cDVO stall register = 0xA.
			 * Note: Though EAS says 6102Ch default value is 6, it is a typo
			 *     in the spec, based on Si DE hw default value is 10 (0xA),
			 *     so no need to program explicitly. This saves few bytes for
			 *     micro.
			 */

			/* Enable Current Source */
			temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG);
			temp |= 0x2000;
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, SDVO_BUFF_CTRL_REG, temp);
		}
	}

	if (pd_powerstate == PD_POWER_MODE_D0) {
		ret = PORT(display, port_number)->pd_driver->set_mode(
			PORT(display, port_number)->pd_context, &local_timing, 0);
	} else {
		ret = PORT(display, port_number)->pd_driver->set_power(
			PORT(display, port_number)->pd_context, pd_powerstate);
	}

	if (ret) {
		EMGD_ERROR_EXIT("PD set_%s returned: 0x%x",
			(pd_powerstate == PD_POWER_MODE_D0)?"mode":"power", ret);
		return -IGD_ERROR_INVAL;
	}

	EMGD_DEBUG("Port_control: 0x%lx", port_control);

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO,
		PORT(display, port_number)->port_reg, port_control);

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param display
 * @param port_number
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
int program_port_lvds_tnc(igd_display_context_t *display,
		unsigned short port_number,
		unsigned long status)
{
	int ret = 0;
	unsigned long powerstate = PD_POWER_MODE_D3;
	pd_timing_t *timing;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program LVDS: (%s)", status?"ENABLE":"DISABLE");

	if (status == TRUE) {
		if(!(PORT(display, port_number)->pt_info->flags & IGD_DISPLAY_ENABLE)) {
			EMGD_TRACE_EXIT;
			return 0;
		}

		powerstate = GET_DISPLAY_POWER_STATE(display,port_number);
		if (powerstate == IGD_POWERSTATE_D0) {
			EMGD_DEBUG("Power State: D0");
			timing = (pd_timing_t *)PIPE(display)->timing;
			/* Reach end timing to get user resolution and pass it to pd */
			if(timing->extn_ptr) {
				timing = (pd_timing_t *)timing->extn_ptr;
			}
			/* set mode will take care of port control */
			ret = PORT(display, port_number)->pd_driver->set_mode(
					PORT(display, port_number)->pd_context,
					timing,
					1<<PIPE(display)->pipe_num);
		}
	}

	/* either status == FALSE, or status == TRUE, but powerstate is D1/D2/D3 */
	if (powerstate != IGD_POWERSTATE_D0) {
		ret = PORT(display, port_number)->pd_driver->set_power(
			PORT(display, port_number)->pd_context,
			PD_POWER_MODE_D3);
	}

	if (ret) {
		EMGD_ERROR_EXIT("PD set_%s returned: 0x%x",
			(powerstate == IGD_POWERSTATE_D0)?"mode":"power", ret);
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return ret;
}


/*!
 *
 * @param display
 * @param port_number
 * @param status
 *
 * @return program_port_lvds_gen4()
 * @return program_port_sdvo_gen4()
 * @return -IGD_ERROR_INVAL on failure
 */
int program_port_tnc(igd_display_context_t *display,
		unsigned short port_number,
		unsigned long status)
{
	EMGD_TRACE_ENTER;

	if (PORT(display, port_number)->port_type == IGD_PORT_LVDS) {
		EMGD_TRACE_EXIT;
		return program_port_lvds_tnc(display, port_number, status);
	} else {
		EMGD_TRACE_EXIT;
		return program_port_sdvo_tnc(display, port_number, status);
	}
}

/*!
 *
 * @param gpio
 *
 * @return size
 */
unsigned long get_gpio_sets_tnc(unsigned long **gpio)
{
	/* To small to trace */
	return 0;
}

/*!
 *
 * @param context
 * @param in_list
 *
 * @return void - To small to trace
 */
void filter_modes_tnc(igd_context_t *context, igd_display_port_t *port,
	pd_timing_t *in_list)
{
	while (in_list->width != IGD_TIMING_TABLE_END) {
		/* TC LVDS:
		 *    supports from 19.75MHz to 79.MHz
		 * TC SDVO:
		 *    supports from 25 MHz to 160 MHz and progressive only.
		 */
		if (port->port_type == IGD_PORT_SDVO) {
			if (in_list->mode_info_flags & IGD_SCAN_INTERLACE ||
				in_list->dclk < 25000 || in_list->dclk > 160000) {
				in_list->mode_info_flags &= ~IGD_MODE_SUPPORTED;
			}
		}
		/* No clock check is required for LVDS port as LVDS port driver
		 * already taken care of this.
		if (port->port_type == IGD_PORT_LVDS) {
			if (in_list->dclk < 19750 || in_list->dclk > 79500) {
				in_list->mode_info_flags &= ~IGD_MODE_SUPPORTED;
			}
		}*/
		in_list++;
		if (in_list->width == IGD_TIMING_TABLE_END && in_list->extn_ptr) {
			in_list = in_list->extn_ptr;
		}
	}
	return;
}
#ifndef CONFIG_MICRO
int get_timing_tnc(igd_display_context_t *display, pd_timing_t *in_list)
{
	//int ret = TRUE;
	igd_timing_info_t *pTimings_ori;
	pd_timing_t *timing = NULL;
	timing = in_list;

	EMGD_TRACE_ENTER;
	pTimings_ori = PIPE(display)->timing;

	if(display->port_number != 2){
		EMGD_DEBUG("Port does not need tuning");
		EMGD_TRACE_EXIT;
		return FALSE;
	}

	while(timing->width != PD_TIMING_LIST_END) {

		if((timing->width == pTimings_ori->width) &&
		   (timing->height == pTimings_ori->height) &&
		   (timing->refresh == pTimings_ori->refresh)){
			EMGD_DEBUG("Timing found");
			timing->reserved_dd = pTimings_ori->reserved_dd;
			timing->mode_info_flags = pTimings_ori->mode_info_flags;

           /* also return the hblank_end so that ioctl can calculate
            * the xblank_length.
            */
            timing->hblank_end = pTimings_ori->hblank_end;
            timing->vblank_end = pTimings_ori->vblank_end;


		}
		/* Go through the table list */
		timing++;
		if ((timing->width == PD_TIMING_LIST_END) && timing->extn_ptr) {
				timing = timing->extn_ptr;
		}
	}

	EMGD_TRACE_EXIT;
	return TRUE;
}

int check_port_supported(void *port_tmp)
{
	//igd_display_port_t *port = (igd_display_port_t *)port_tmp;
	/* Determine which port driver is supported by Atom E6xx */
#if 0 //Pendng confirmation from LNC architect
	if(pd_driver->type & PD_DISPLAY_TVOUT){
		return 1;
	}
#endif
	return 0;
}
#endif

/* Function checks if the incoming user DTD is one of the timings
 * in the crt_timing_table that has border. Meaning that the
 * h_blank_end is not the same as htotal.
 * We only see this issue when reading a user DTD generated for
 * harmonic which is derived from crt_timing_table. pi module
 * only handles user DTD which does not have borders.
 */
int get_refresh_in_border(pd_timing_t *in_list)
{
       /* return 1 if refresh is obtained */

       if(in_list->width == 640 &&
               in_list->height == 480 &&
               in_list->dclk == 25175 &&
               in_list->hsync_start == 655 &&
               in_list->hsync_end == 751 &&
               in_list->vsync_start == 489 &&
               in_list->vsync_end == 491){
                       in_list->refresh = 60;
                       in_list->htotal = 799;
                       in_list->vtotal = 524;
                       in_list->hblank_start = 646;
                       in_list->vblank_start = 486;
                       return 1;
       }

       if(in_list->width == 640 &&
               in_list->height == 480 &&
               in_list->dclk == 31500 &&
               in_list->hsync_start == 663 &&
               in_list->hsync_end == 703 &&
               in_list->vsync_start == 488 &&
               in_list->vsync_end == 491){

                       in_list->refresh = 72;
                       in_list->htotal = 831;
                       in_list->vtotal = 519;
                       in_list->hblank_start = 646;
                       in_list->vblank_start = 486;
                       return 1;
       }


       return 0;
}

/*
 * Function to check if dc is switching from
 * SDVO single to clone/extended mode
 * Tunnel Creek B0  needs tuning when there are 2 planes,
 * if true, it will force alter to run tuning
 */
bool dsp_is_force_alter_required_tnc(igd_display_context_t *display,
	unsigned long current_dc, unsigned long dc_to_set){

	if ((IGD_DC_SINGLE(current_dc) &&
			(IGD_DC_PRIMARY(current_dc) == IGD_PORT_TYPE_SDVOB )) &&
			(IGD_DC_CLONE(dc_to_set) || IGD_DC_EXTENDED(dc_to_set)) &&
			(display->context->device_context.rid == TNC_B0_RID)){

		return TRUE;
	}

	return FALSE;
}

mode_dispatch_t mode_dispatch_tnc = {
	igd_set_palette_entry_tnc,
	igd_get_palette_entry_tnc,
	igd_wait_vblank_tnc,
	program_plane_tnc,
	program_pipe_tnc,
	program_port_tnc,
	post_program_port_tnc,
	program_clock_tnc,
	program_cdvo_tnc,
	reset_plane_pipe_ports_tnc,
	get_gpio_sets_tnc,
	filter_modes_tnc,
	OPT_MICRO_VALUE(check_display_tnc, NULL),
	OPT_MICRO_VALUE(get_timing_tnc, NULL),
	OPT_MICRO_VALUE(check_port_supported, NULL),
	OPT_MICRO_VALUE(get_refresh_in_border, NULL),
	OPT_MICRO_VALUE(dsp_is_force_alter_required_tnc, NULL),
	OPT_MICRO_VALUE(&mode_full_dispatch_tnc, NULL),
};

/* VBIOS does not use the virtual mapped address
 * This function will return 0 for VBIOS anyway */
#ifndef CONFIG_MICRO
unsigned char *get_mmio_tnc(unsigned long port_type)
{
	unsigned char *virt_mmio;
	if (port_type == IGD_PORT_LVDS) {
		virt_mmio = mode_context->context->device_context.virt_mmadr;
	} else if (port_type == IGD_PORT_SDVO) {
		virt_mmio = mode_context->context->device_context.virt_mmadr_sdvo;
	} else if (port_type == IGD_PORT_SDVO_ST) {
		virt_mmio = mode_context->context->device_context.virt_mmadr_sdvo_st;
	} else if (port_type == IGD_PORT_SDVO_ST_GPIO) {
		virt_mmio = mode_context->context->device_context.virt_mmadr_sdvo_st_gpio;
	} else {
		/* i2c_bitbash will use the else condition by passing in port_type
		 * as '0' */
		virt_mmio = mode_context->context->device_context.virt_gpio_bar;
	}
	return virt_mmio;
}
#endif

/* Function to read Atom E6xx 0:2:0, 0:3:0 and 0:31:0 mmio registers */
unsigned long read_mmio_reg_tnc(unsigned long port_type, unsigned long reg)
{
	unsigned long value;
#ifndef CONFIG_MICRO
	unsigned char *mmio;
#endif

	/* to avoid updating or having another set of read/write macros for
	 * vbios, overwrite io_base to properly read from 0:2:0/0:3:0/0:31:0
	 * io_base and set it back after reading */
	if (port_type == IGD_PORT_LVDS) {
		io_base = io_base_lvds;
	} else if (port_type == IGD_PORT_SDVO) {
		io_base = io_base_sdvo;
	} else if (port_type == IGD_PORT_LPC) {
		io_base = io_base_lpc;
	} else if (port_type == IGD_PORT_SDVO_ST) {
		io_base = io_base_sdvo_st;
	}
	OPT_MICRO_CALL_RET(mmio, get_mmio_tnc(port_type));
	if (port_type == IGD_PORT_LPC) {
		value = EMGD_READ_PORT32(io_base_lpc + reg);
	} else {
		value = EMGD_READ32(EMGD_MMIO(mmio) + reg);
	}

	io_base = io_base_lvds;
	return value;
}

/* Function to write Atom E6xx 0:2:0 and 0:3:0 mmio registers */
void write_mmio_reg_tnc(unsigned long port_type, unsigned long reg,
	unsigned long value)
{
#ifndef CONFIG_MICRO
	unsigned char *mmio;
#endif

	/* to avoid updating or having another set of read/write macros for
	 * vbios, overwrite io_base to properly read from 0:2:0/0:3:0 io_base
	 * and set it back after writing */
	if (port_type == IGD_PORT_LVDS) {
		io_base = io_base_lvds;
	} else if (port_type == IGD_PORT_SDVO) {
		io_base = io_base_sdvo;
	} else if (port_type == IGD_PORT_LPC) {
		io_base = io_base_lpc;
	} else if (port_type == IGD_PORT_SDVO_ST) {
		io_base = io_base_sdvo_st;
	}

	OPT_MICRO_CALL_RET(mmio, get_mmio_tnc(port_type));
	if (port_type == IGD_PORT_LPC) {
		EMGD_WRITE_PORT32(io_base_lpc + reg, value);
	} else {
		EMGD_WRITE32(value, EMGD_MMIO(mmio) + reg);
	}
	io_base = io_base_lvds;
	return;
}

#endif

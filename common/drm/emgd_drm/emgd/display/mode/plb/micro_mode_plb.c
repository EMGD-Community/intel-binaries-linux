/*
 *-----------------------------------------------------------------------------
 * Filename: micro_mode_plb.c
 * $Revision: 1.22 $
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
 *  Napa Core implementations for the mode dispatch functions.
 *  NOTE: This file is designed to produce tiny code and is used in the
 *  vbios port. Make sure that all MMIO reads and writes do all mmio
 *  calculation within the OS_READ and OS_WRITE macros so that this code
 *  will be correctly removed.
 *  This is OK -> EMGD_READ32( MMIO(display) + foo_offset )
 *  This is NOT OK ->  mmio = MMIO(display)
 *  EMGD_READ32(mmio + foo_offset)
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <io.h>

#include <igd.h>
#include <igd_pwr.h>

#include <context.h>
#include <rb.h>
#include <mode.h>
#include <utils.h>
#include <dsp.h>
#include <vga.h>
#include <intelpci.h>
#include <mode_access.h>
#include <pi.h>
#include <dispatch.h>

#include <plb/regs.h>
#include <plb/context.h>
#include <plb/mi.h>
#include "drm_emgd_private.h"

#include "../cmn/match.h"
#include "../cmn/mode_dispatch.h"
#include "mode_plb.h"

/*!
 * @addtogroup display_group
 * @{
 */

#ifdef CONFIG_PLB

/*
 * Exports from the other components of this module.
 */

extern int program_clock_plb(igd_display_context_t *display,
	igd_clock_t *clock, unsigned long dclk);
extern mode_full_dispatch_t mode_full_dispatch_plb;

static unsigned long gpio_plb[] = {
	0x5010,
	0x5014,
	0x5018,
	0x501c,
	0x5020,
	0x5024,
	0x5028
};

mode_data_plb_t device_data_plb[1] = {
	{
		0x000b0000, /* plane a preservation */
		0x00000000, /* plane b/c preservation */
		0x01000000, /* pipe preservation */
		0, /* port preservation */
		0x490A010A, /* watermark/burst length 1 */
		0x14100D0A, /* watermark/burst length 2*/
		0x00007770, /* watermark/burst length 3 */
		0x0B0C9812, /* watermark/burst length self */
		0,  /* mem_mode */
		0x00001D9C, /* dsp arb */
	}
};

/*!
 *
 * @param mmio
 *
 * @return void
 */
void disable_vga_plb (unsigned char *mmio)
{
	unsigned long temp;
	unsigned char sr01;

	EMGD_TRACE_ENTER;

	/* Disable VGA plane if it is enabled. */
	temp = EMGD_READ32(EMGD_MMIO(mmio) + VGACNTRL);
	if ((temp & BIT31) == 0) {
		/* Read SR01 */
		READ_VGA(mmio, SR_PORT, 0x01, sr01);

		/* Turn on SR01 bit 5 */
		WRITE_VGA(mmio, SR_PORT, 0x01, sr01|BIT(5));
		/* Wait for 30us */
		OS_SLEEP(30);

		temp |= BIT31;     /* set bit 31 to disable */
		temp &= ~BIT30;    /* clear bit 30 to get VGA display in normal size */
		EMGD_WRITE32(temp, EMGD_MMIO(mmio) + VGACNTRL);
	}
	/*
	 * When turing off the VGA plane the palette sometimes gets stuck.
	 * if we do a couple reads to the palette it will unstuck.
	 */
	if((1L<<31) & EMGD_READ32( EMGD_MMIO(mmio) + PIPEA_CONF )) {
		EMGD_DEBUG("VGA Palette workaround");
		EMGD_READ32(EMGD_MMIO(mmio) + DPALETTE_A);
		EMGD_READ32(EMGD_MMIO(mmio) + DPALETTE_A);
	}
	if((1L<<31) & EMGD_READ32( EMGD_MMIO(mmio) + PIPEB_CONF )) {
		EMGD_DEBUG("VGA Palette workaround");
		EMGD_READ32(EMGD_MMIO(mmio) + DPALETTE_B);
		EMGD_READ32(EMGD_MMIO(mmio) + DPALETTE_B);
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
static int igd_set_palette_entry_plb(
	igd_display_h display_handle,
	unsigned long palette_entry,
	unsigned long palette_color)
{
	/* Too Slow For Tracing */

	/* Return if Pipe is not on */
	if(!((1L<<31) & EMGD_READ32(MMIO(display_handle) +
			PIPE(display_handle)->pipe_reg))) {
		return -IGD_INVAL;
	}
	/*
	EMGD_DEBUG("%x : %ld : %lx", display_handle,
		palette_entry, palette_color);
	*/
	EMGD_WRITE32(palette_color, MMIO(display_handle) +
		PIPE(display_handle)->palette_reg + palette_entry * 4);

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
static int igd_get_palette_entry_plb(
	igd_display_h display_handle,
	unsigned long palette_entry,
	unsigned long *palette_color)
{
	/* Too Slow For Tracing */

	/* Return if Pipe is not on */
	if(!((1L<<31) & EMGD_READ32(MMIO(display_handle) +
			PIPE(display_handle)->pipe_reg))) {
		return -IGD_INVAL;
	}

	*palette_color = 0xffffff & EMGD_READ32(MMIO(display_handle) +
		PIPE(display_handle)->palette_reg + palette_entry * 4);
	return 0;
}

/*!
 *
 * @param mmio
 * @param pipe_reg
 * @param time_interval
 *
 * @return 0 on success
 * @return 1 on failure
 */
int wait_for_vblank_timeout_plb(
	unsigned char *mmio,
	unsigned long pipe_reg,
	unsigned long time_interval)
{
	int ret;
	unsigned long pipe_status_reg = pipe_reg + PIPE_STATUS_OFFSET;
	unsigned long tmp;
	os_alarm_t timeout;
	unsigned long request_for;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameters: MMIO = %p, pipe_reg = %lx, time_interval = %lx",
		mmio, pipe_reg, time_interval);

	/* If pipe is off then just return */
	if(!((1L<<31) & EMGD_READ32(EMGD_MMIO(mmio) + pipe_reg))) {
		EMGD_DEBUG("Pipe disabled/Off");
		EMGD_TRACE_EXIT;
		return 1;
	}

	/*
	 * When VGA plane is on the normal wait for vblank won't work
	 * so just skip it.
	 */
	if(!(EMGD_READ32(EMGD_MMIO(mmio) + 0x71400) & 0x80000000)) {
		EMGD_DEBUG("VGA Plane On");
		EMGD_TRACE_EXIT;
		return 1;
	}

	/* 1. Request the interrupt handler to record the next VBlank: */
	request_for = VBINT_REQUEST(VBINT_WAIT,
		(pipe_status_reg == PIPEA_STAT) ? VBINT_PORT2 : VBINT_PORT4);
	mode_context->dispatch->full->request_vblanks(request_for, mmio);

	/* 2. Wait & poll for the next VBlank: */
	timeout = OS_SET_ALARM(time_interval);
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
	mode_context->dispatch->full->end_request(request_for, mmio);


	EMGD_TRACE_EXIT;
	return ret;
} /* wait_for_vblank_timeout_plb */

/*!
 *
 * @param mmio
 * @param pipe_reg
 *
 * @return 0 on success
 * @return 1 on failure
 */
int wait_for_vblank_plb(unsigned char *mmio,
	unsigned long pipe_reg)
{
	return wait_for_vblank_timeout_plb(mmio, pipe_reg, 100);
} /* wait_for_vblank_plb */


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
static int igd_wait_vblank_plb(igd_display_h display_handle)
{

	return wait_for_vblank_plb(MMIO(display_handle),
		PIPE(display_handle)->pipe_reg);

}  /* wait_vblank */


/*!
 * Get the stride and stereo values based on the display.  This is also used
 * by the MI instructions.
 *
 * @param display Pointer to hardware device instance data
 * @param stride
 * @param stereo
 * @param flags Should the stereo be for the frontbuffer or backbuffer?
 *
 * @return stride - Stride of the display
 * @return stereo - Stereo address of the display
 */
static int mode_get_stride_stereo_plb(igd_display_context_t *display,
	unsigned long *stride,
	unsigned long *stereo,
	unsigned long flags)
{
	unsigned long pitch = PLANE(display)->fb_info->screen_pitch;
	igd_timing_info_t *timing = PIPE(display)->timing;
	unsigned long base_offset;

	base_offset = PLANE(display)->fb_info->visible_offset;

	*stride = pitch;
	*stereo = 0;

	/* For field replication, valid for interlaced modes only
	 *     set stereo = fb_base,
	 *         stride = pitch
	 */
	if (timing->mode_info_flags & IGD_SCAN_INTERLACE) {

		if(timing->mode_info_flags & IGD_LINE_DOUBLE) {
			/* Interlaced + Line double flags means field replication.
			 * same lines are sent for both fields. Program the
			 * second eye to be same as the first.
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

	return 0;
}

/*!
 *
 * @param display Pointer to hardware device instance data
 *
 * @return void
 */
static void program_pipe_vga_plb(
	igd_display_context_t *display)
{
	igd_timing_info_t *timing;
	unsigned long vga_control;
	unsigned long upscale = 0;
	int centering = 1;

	EMGD_TRACE_ENTER;

#ifdef CONFIG_MICRO
	/*
	 * We cannot set a VGA mode unless the display planes are turned off.
     * This becomes evident during a Windows BSOD.  Since neither Windows
     * nor IEGD got a chance to turn off these registers, and the VGA mode was
     * set by the VBIOS, the screen gets corrupted.  In order to fix this
     * problem, we will turn the cursor and display planes here.
     *
     * Note: Removing previous partial-fix in favor of this complete one.
	 */
    /* Cursor A */
	WRITE_MMIO_REG(display, 0x70080, 0);
	WRITE_MMIO_REG(display, 0x70084, 0);
    /* Cursor B */
	WRITE_MMIO_REG(display, 0x700C0, 0);
	WRITE_MMIO_REG(display, 0x700C4, 0);
    /* Display A */
	WRITE_MMIO_REG(display, 0x70180, 0);
	WRITE_MMIO_REG(display, 0x70184, 0);
    /* Display B */
	WRITE_MMIO_REG(display, 0x71180, 0);
	WRITE_MMIO_REG(display, 0x71184, 0);
    /* Display C */
	WRITE_MMIO_REG(display, 0x72180, 0);
	WRITE_MMIO_REG(display, 0x72184, 0);
#endif

	/*
	 * VGA Plane can attach to only one pipe at a time. LVDS can
	 * only attach to pipe B. We need to use the display passed to
	 * determine the pipe number to use. (Plba is same as Alm).
	 */

	/*
	 * We can come here with following cases:
	 *   1. magic->vga    CRT, DVI type displays
	 *   2. native->vga   int-lvds, and up-scaling lvds displays
	 *   3. pipe->vga     TV and other unscaled-lvds displays
	 */
	vga_control = READ_MMIO_REG(display, 0x71400);
	vga_control &= 0x18e3ff00;
	vga_control |= 0x8e;

	timing = PIPE(display)->timing;
	if(!timing->extn_ptr) {
		EMGD_ERROR_EXIT("No Extension pointer in program_pipe_vga_plb");
		return;
	}

	/* Find UPSCALING attr value*/
	pi_pd_find_attr_and_value(PORT_OWNER(display),
			PD_ATTR_ID_PANEL_FIT,
			0,/*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);
	/* this PI func will not modify value of upscale if attr does not exist */

	/* magic->vga or native->vga cases */
	if ((timing->width == 720 && timing->height == 400) || upscale) {
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
	WRITE_MMIO_REG(display, 0x71400, vga_control);

	EMGD_TRACE_EXIT;
	return;
}

/*!
 * Program Display Plane Values.
 *
 * @param display Pointer to hardware device instance data
 * @param status
 *
 * @return void
 */
static void program_plane_plb(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long stereo;
	unsigned long stride;
	unsigned long size;
	unsigned long plane_control;
	igd_timing_info_t *timing;
	igd_framebuffer_info_t *fb_info = PLANE(display)->fb_info;
	unsigned long plane_reg = PLANE(display)->plane_reg;
	unsigned long start_addr_reg = DSPAADDR;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Program Plane: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	igd_wait_vblank_plb((igd_display_h)display);

	plane_control = EMGD_READ32(MMIO(display) + plane_reg);
	if(PLANE(display)->plane_reg == DSPACNTR) {
		plane_control &= device_data_plb->plane_a_preserve;
	}
	else { /* if it's plane b or plane c */
		plane_control &= device_data_plb->plane_b_c_preserve;
		start_addr_reg = 0x71184;
	}

	if((status == FALSE) ||
		(GET_DEVICE_POWER_STATE(display) != IGD_POWERSTATE_D0)) {

		/*
		 * Note: The vga programming code does not have an "off". So
		 * when programming the plane to off we make sure VGA is off
		 * as well.
		 */
		disable_vga_plb(MMIO(display));

		/*
		 * To turn off plane A or B, the program have to triger the plane A or B
		 * start register.  Or else, it will not work.
		 */
		EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
		EMGD_WRITE32(EMGD_READ32(MMIO(display) + start_addr_reg),
			MMIO(display) + start_addr_reg);

		igd_wait_vblank_plb((igd_display_h)display);
		return;
	}
	/*
	 * Note: The very first pass through this function will be with
	 * status false and timings == NULL. Don't use the timings before
	 * the check above.
	 */
	timing = PIPE(display)->timing;
	/* There is a special case code for legacy VGA modes */
	while (timing->extn_ptr) {
		timing = (igd_timing_info_t *)timing->extn_ptr;
	}
	if(MODE_IS_VGA(timing)) {
		program_plane_vga(display, timing);
		return;
	}

	disable_vga_plb(MMIO(display));

	size = (((unsigned long)timing->height - 1)<<16) |
		(unsigned long)(timing->width - 1);

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
		/* Horz pixel multiply must be set for double */
		plane_control |= BIT11;
		/* TODO -> Plba can more than double,
		It can 3X, 4X etc. These arent exposed now */
	}
	else if(plane_control & BIT20){
		/* For line ONLY doubling, set bit 21 also '1' */
		plane_control |= BIT21;
	}

	mode_get_stride_stereo_plb(display, &stride, &stereo, 0);

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

	EMGD_DEBUG(" Plane Control = 0x%lx", plane_control);
	EMGD_DEBUG(" Plane Base = 0x%lx", fb_info->visible_offset);
	EMGD_DEBUG(" Plane Pitch = 0x%lx", stride);
	EMGD_DEBUG(" Plane Size = 0x%lx", size);

	EMGD_WRITE32(stride, MMIO(display) + plane_reg + DSP_STRIDE_OFFSET);
	/*
	 * In reality this only exists for plane B. It doesn't seem to hurt
	 * plane A so just do it anyway and save us another case.
	 */
	EMGD_WRITE32(size, MMIO(display) + plane_reg + DSP_SIZE_OFFSET);

	/*EMGD_WRITE32(stereo, MMIO(display) + plane_reg + DSP_STEREO_OFFSET);
		- This register is Reserved ON plba */
	EMGD_WRITE32(fb_info->visible_offset,
		MMIO(display) + plane_reg + DSP_START_OFFSET);

	/* It seems we need push or trigger plane A/B to start to work
	 * on Poulsbo, especially for sDVO port. Let's write plane control
	 * register and start address register at last.
	 */
	EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
	EMGD_WRITE32(EMGD_READ32(MMIO(display) + start_addr_reg),
		MMIO(display) + start_addr_reg);

	igd_wait_vblank_plb((igd_display_h)display);

	EMGD_TRACE_EXIT;
	return;
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
static void program_pipe_plb(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long   timing_reg;
	unsigned long   pipe_conf;
	unsigned long   hactive, vactive;
	igd_timing_info_t  *pTimings;
	unsigned long temp;
#ifndef CONFIG_MICRO
	igd_display_port_t *port;
	int i;
#endif

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Program Pipe: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	pipe_conf = device_data_plb->pipe_preserve &
		EMGD_READ32(MMIO(display) + PIPE(display)->pipe_reg);

	/* Reset the plane of this pipe back to NULL, it will be set on the
	 * call to program_plane, which is ok, since program_pipe occurs
	 * before program_plane */
	PIPE(display)->plane = NULL;

	if((status == FALSE) ||
		(GET_DEVICE_POWER_STATE(display) == IGD_POWERSTATE_D3)) {
		/* Disable pipe */
		EMGD_WRITE32(pipe_conf & (~0x80000000),
			MMIO(display) + PIPE(display)->pipe_reg);

		return;
	}

	pTimings = PIPE(display)->timing;

	/*
	 * If the mode is VGA and the PD says it handles all VGA modes without
	 * reprogramming then just set the mode and leave centering off.
	 */
	if(pTimings->mode_info_flags & IGD_MODE_VESA) {
		if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
			/* Pipe timings and clocks are not used but it must be on anyway */
			EMGD_WRITE32(pipe_conf | 0x80000000,
				MMIO(display) + PIPE(display)->pipe_reg);
			program_pipe_vga_plb(display);
			return;
		} else {
#ifdef CONFIG_MICRO
			set_256_palette(MMIO(display));
#endif
		}
	}

	/* Program dot clock divisors. */
	program_clock_plb(display, PIPE(display)->clock_reg, pTimings->dclk);

	/* Program timing registers for the pipe */
	timing_reg = PIPE(display)->timing_reg;
	if (pTimings->mode_info_flags & IGD_PIXEL_DOUBLE) {
		hactive = (unsigned long)pTimings->width*2 - 1;
	} else {
		hactive = (unsigned long)pTimings->width - 1;
	}

	if (pTimings->mode_info_flags & IGD_LINE_DOUBLE) {
		if (pTimings->mode_info_flags & IGD_SCAN_INTERLACE) {
			vactive = (unsigned long)pTimings->height - 1;
		} else {
			vactive = (unsigned long)pTimings->height*2 - 1;
		}
	} else {
		if (pTimings->mode_info_flags & IGD_SCAN_INTERLACE) {
			vactive = (unsigned long)pTimings->height/2 - 1;
		} else {
			vactive = (unsigned long)pTimings->height - 1;
		}
	}

	/*
	 * DPLL should be on at this point which is required for touching
	 * the palette.
	 */
#ifndef CONFIG_MICRO
	/* reset the palette */
	for (i = 0; i < 256; i++) {
		EMGD_WRITE32(((i<<16) | (i<<8) | i),
			MMIO(display) + PIPE(display)->palette_reg + i*4);
	}

	/* apply color correction */
	port = PORT_OWNER(display);
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
	 * All register offsets and bit shift are verified for Napa
	 */
	temp = ((unsigned long)pTimings->htotal << 16) | hactive;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg);

	temp = ((unsigned long)pTimings->hblank_end << 16) |
		(unsigned long)pTimings->hblank_start;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x04);

	temp = ((unsigned long)pTimings->hsync_end << 16) |
		(unsigned long)pTimings->hsync_start;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x08);

	temp = ((unsigned long)pTimings->vtotal << 16) | vactive;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x0C);

	temp = ((unsigned long)pTimings->vblank_end << 16) |
		(unsigned long)pTimings->vblank_start;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x10);

	temp = ((unsigned long)pTimings->vsync_end << 16) |
		(unsigned long)pTimings->vsync_start;
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x14);

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
			temp = (unsigned long)scaled_timings->width  - 1;
			temp = (temp << 16) |
				(unsigned long)(scaled_timings->height - 1);
		}
	} else {
		temp = (hactive << 16) | vactive;
	}
	EMGD_WRITE32(temp, MMIO(display) + timing_reg + 0x1C);

	/* Set other registers */

	/*
	 * FIXME: max_dclk needs to be determined from core clock
	 * at init time. 915 etc has several skus with different
	 * clocks for the same device ID.
	 *
	 */

	/* These values are derived from the Poulsbo B-Spec as
	 * the suggested values */
	WRITE_MMIO_REG (display, FW_BLC1, device_data_plb->fw_blc1);
	WRITE_MMIO_REG (display, FW_BLC2, device_data_plb->fw_blc2);
	WRITE_MMIO_REG (display, FW_BLC3, device_data_plb->fw_blc3);
	WRITE_MMIO_REG (display, FW_BLC_SELF, device_data_plb->fw_self);
	WRITE_MMIO_REG (display, PIPEA_DISP_ARB_CTRL, device_data_plb->dsp_arb);

	/* The SGX 2D engine can saturate the memory bus and starve
	 * the display engine causing visible screen tearing.
	 * This reduces the priority of the SGX vs. display engine
	 */
	temp = READ_MMIO_REG (display, G_DEBUG);
	WRITE_MMIO_REG (display, G_DEBUG, (temp | (1 << 11)));

	pipe_conf |= PIPE_ENABLE;
	WRITE_MMIO_REG(display, PIPE(display)->pipe_reg, pipe_conf);

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
		if(pTimings->mode_info_flags & IGD_MODE_VESA) {
			if (pTimings->mode_number <= VGA_MODE_NUM_MAX) {
				program_pipe_vga_plb(display);
			} else {
#ifdef CONFIG_MICRO
				set_256_palette(MMIO(display));
#endif
			}
		}
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
static void reset_plane_pipe_ports_plb(igd_context_t *context)
{
	igd_plane_t        *plane;
	igd_display_pipe_t *pipe;
	igd_display_port_t *port,*tv_port=NULL;
	unsigned long temp;
	unsigned long i;
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

	disable_vga_plb(EMGD_MMIO(mmio));

	/* Turn off ports */
	port = NULL;
	while((port = md->dsp_get_next_port(context, port, 0)) != NULL) {
		/* if the port is TV, then don't set the power to S3 as this causes
		 * blank screen on analog port after killx or cosole mode,
		 * probably because the external clock needs to be on till the pipes and
		 * DPLLs are off
		 */
		if (port->pd_driver) {
#ifndef CONFIG_FASTBOOT
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
#endif
		}

		/* Disable WRITE protection on PIPE B for parts with Int-LVDS*/
		/* This should never happen as the panel power was set to D3 above */
		if (port->port_reg == LVDSCNTR) {
			if(EMGD_READ32(EMGD_MMIO(mmio) + LVDS_PNL_PWR_CTL) & 0x1) {
				EMGD_WRITE32(0xABCD0000, EMGD_MMIO(mmio) + LVDS_PNL_PWR_CTL);
				i=0;
				while(i++ < 0x10) {
					OS_SLEEP(10);
					if((EMGD_READ32(EMGD_MMIO(mmio)+LVDS_PNL_PWR_STS)&BIT(31))==0) {
						break;
					}
				}
			}
		}

		temp = EMGD_READ32(EMGD_MMIO(mmio) + port->port_reg);
		EMGD_WRITE32((temp & ~BIT31), EMGD_MMIO(mmio) + port->port_reg);
	}

	/* disable plane C */
	temp = EMGD_READ32(EMGD_MMIO(mmio) + DSPCCNTR);
	if(temp & BIT31) {
		EMGD_WRITE32(0x0, EMGD_MMIO(mmio) + DSPCCNTR);
		EMGD_WRITE32(0x0, EMGD_MMIO(mmio) + DSPCCNTR + DSP_START_OFFSET);
	}

	plane = NULL;
	while ((plane = md->dsp_get_next_plane(context, plane, 1)) != NULL) {
		/* Only display display planes.
		 *  Leave cursor, VGA, overlay, sprite planes alone since they will
		 *  need a different disable bit/sequence.
		 */
		temp = EMGD_READ32(EMGD_MMIO(mmio) + plane->plane_reg);
		if ((plane->plane_features & IGD_PLANE_DISPLAY)) {
			if ( temp & BIT31 ) {
				if(plane->plane_reg == DSPACNTR) {
					EMGD_WRITE32((temp & device_data_plb->plane_a_preserve),
						EMGD_MMIO(mmio) + plane->plane_reg);
				}
				else { /* if it's plane b or plane c */
					EMGD_WRITE32((temp & device_data_plb->plane_b_c_preserve),
						EMGD_MMIO(mmio) + plane->plane_reg);
				}
				EMGD_WRITE32(0, EMGD_MMIO(mmio) + plane->plane_reg+4);
			}
		} else if ((plane->plane_features & IGD_PLANE_CURSOR)) {
			EMGD_WRITE32((temp & 0xffffffe8),
				EMGD_MMIO(mmio) + plane->plane_reg);
			EMGD_WRITE32(0, EMGD_MMIO(mmio) + plane->plane_reg+4);
		}
	}

	/* Turn off pipes */
	pipe = NULL;
	while ((pipe = md->dsp_get_next_pipe(context, pipe, 0))) {
		wait_for_vblank_plb(EMGD_MMIO(mmio), pipe->pipe_reg);
		temp = EMGD_READ32(EMGD_MMIO(mmio) + pipe->pipe_reg);
		if ( temp & BIT31 ) {
			EMGD_WRITE32((temp & device_data_plb->pipe_preserve),
				EMGD_MMIO(mmio) + pipe->pipe_reg);
		}
	}
	/* pipes and DPLLs are off, now set the power for TV */
	if(tv_port && tv_port->pd_driver) {
		tv_port->pd_driver->set_power(tv_port->pd_context, IGD_POWERSTATE_D3);
	}
	EMGD_TRACE_EXIT;

} /* end reset_plane_pipe_ports */

/*!
 *
 * @param display
 * @param port_number
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int	program_port_plb(igd_display_context_t *display,
	unsigned short port_number,
	unsigned long status)
{
	unsigned long pipe_number;
	unsigned long port_control;
	unsigned long port_control_analog = 0;
	unsigned long mult_port_control;
	unsigned long pd_powerstate = 0;
	unsigned long upscale = 0;
	pd_timing_t *timing;
	pd_timing_t local_timing;
	unsigned long port_type;
	int ret;

	/* get the pipe	number */
	pipe_number = PIPE(display)->pipe_num;

	/* get the timings */
	timing = PIPE(display)->timing;

	/* keep the port type as local as we access it frequently */
	port_type = PORT(display, port_number)->port_type;

	/* Reading the preservation	bits */
	port_control = PORT(display, port_number)->preserve &
		READ_MMIO_REG(display, PORT(display, port_number)->port_reg);

	/* Reading the preservation bits for SDVO Gang Mode */
	mult_port_control = PORT(display, port_number)->mult_preserve &
		READ_MMIO_REG(display, PORT(display, port_number)->port_reg);

	/* If status is false, quickly disable the display */
	if(status == FALSE) {
		ret = PORT(display, port_number)->pd_driver->set_power(
				PORT(display, port_number)->pd_context, PD_POWER_MODE_D3);

		if (ret) {
			EMGD_ERROR_EXIT("PD set_power() returned: 0x%x", ret);
			return -IGD_ERROR_INVAL;
		}

		if(port_type == IGD_PORT_DIGITAL) {
			WRITE_MMIO_REG(display, PORT(display, port_number)->port_reg,
					port_control);

			if(PORT(display, port_number)->mult_port) {
				/* either gang mode or RGBA */
				WRITE_MMIO_REG(display,
						PORT(display, port_number)->mult_port->port_reg,
						mult_port_control);
			}
		} else if (port_type == IGD_PORT_ANALOG) {
			port_control |= (BIT11 | BIT10);	 /* put in D3 state */
			WRITE_MMIO_REG(display, PORT(display, port_number)->port_reg,
				port_control);

			return 0;
		}
		return 0;
	}

	EMGD_DEBUG("status isn't false, Check port enabled");

	if(! (PORT(display, port_number)->pt_info->flags & IGD_DISPLAY_ENABLE)) {
		return 0;
	}

	/*
	 * Is this is the magic mode then turn on VGA syncs
	 */
	EMGD_DEBUG("Check vga_sync");
	if(PORT(display, port_number)->vga_sync == 1) {
		/*
		 * Is this is the magic mode then turn on VGA syncs
		 */
		EMGD_DEBUG("VGA sync true, is width x height 720 x 400?");
		if((timing->width == 720) && (timing->height == 400)) {
			EMGD_DEBUG("Modify port control and multi_port_control");
			port_control |= (1L<<15);
			mult_port_control |= (1L<<15);
		}
	}

	EMGD_DEBUG("Check analog port");
	if(port_type == IGD_PORT_ANALOG) {
		port_control |= (0x80000000 | (pipe_number<<30));
		if(timing->mode_info_flags & IGD_VSYNC_HIGH) {
			port_control |= (1L<<4);
		}
		if(timing->mode_info_flags & IGD_HSYNC_HIGH) {
			port_control |= (1L<<3);
		}
		/* To differentiate	between	analog and other ports */
		port_control_analog = port_control;
	}

	EMGD_DEBUG("Get power state");
	EMGD_DEBUG("power state = %ld ", GET_DISPLAY_POWER_STATE(display, port_number));
	switch(GET_DISPLAY_POWER_STATE(display, port_number)) {
	case IGD_POWERSTATE_D0:
		EMGD_DEBUG("Power State is D0");
		pi_pd_find_attr_and_value(PORT(display, port_number),
			PD_ATTR_ID_PANEL_FIT,
			0, /*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);
		if(port_type == IGD_PORT_DIGITAL) {
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

			ret = PORT(display, port_number)->pd_driver->set_mode(
				PORT(display, port_number)->pd_context, &local_timing, 0);
			if (ret) {
				EMGD_ERROR_EXIT("PD set_mode returned: 0x%x", ret);
				return -IGD_ERROR_INVAL;
			}

			/* in Plba B-Speecs, there are no bits, *
			 * for the polarity of the H-sync/V-sync */

			/* in Plba B-Speecs, there are no bits,	 *
			 * for data ordering/format for DVO data */
			/* Gang-Mode and RGBA models are "exclusive-or" */
			if((PORT(display, port_number)->pd_driver->flags) &
					PD_FLAG_GANG_MODE) {
				mult_port_control |= (1L<<16);
			} else if(PORT(display, port_number)->pd_type == PD_DISPLAY_RGBA) {
				mult_port_control |= (1L<<2);
			}

			timing = PIPE(display)->timing;

			if(timing->dclk > 100000) {
				/* 100MPs < pixel rate < 200MPs */
				/* SDVO clock rate multiplier = 1x */
				/*
				port_control &=	~BIT23;
				mult_port_control |= ~BIT23;
					redundant code since BIT23 is
					already 0 at this point
				*/
			} else if(timing->dclk > 50000) {
				/* 50MPs < pixel rate < 100MPs */
				/* SDVO clock rate multiplier = 2x */
				port_control |= (1L<<23);
				mult_port_control |= (1L<<23);
			} else {
				/* 25MPs < pixel rate < 50MPs */
				/* SDVO clock rate multiplier = 4x */
				port_control |= (3L<<23);
				mult_port_control |= (3L<<23);
			}

			/*
			 * BIT7 = enable the border
			 *   Do we need to disable the SDVO border for native
			 *   VGA timings(i.e.,	use	DE)?
			 * BIT22->BIT19 = setup the clock phase-9
			 * BIT29 = enable the stall
			 *   Only set stall on DVO-B for gang mode
			 * BIT30 = pipe number
			 * BIT31 = port enable
			 */
			port_control |= ( ( pipe_number<<30 ) | (BIT31) | ((0x9l) << 19) |
					(BIT29) | (BIT7) );
			mult_port_control |= ( (BIT31) | ((0x9l) << 19) | (BIT7) );

			WRITE_MMIO_REG(display, PORT(display, port_number)->port_reg,
					port_control);

			if(PORT(display, port_number)->mult_port) { /* gang mode or rgba*/
				WRITE_MMIO_REG(display,
						PORT(display, port_number)->mult_port->port_reg,
						mult_port_control);
			}
			return 0;
		} else if(port_type == IGD_PORT_LVDS) {
			/*
			 * There is	a special case for LVDS	scaling. If	the	timing is
			 * the native one and the extension	points to another non-vga
			 * mode	then send the extension	pointer.
			 */
			/* Reach the end timing to get user requested mode */
			if(timing->extn_ptr) {
				timing = (pd_timing_t *)timing->extn_ptr;
			}
		}
		/* set mode	will take care of port control */
		ret = PORT(display, port_number)->pd_driver->set_mode(
			PORT(display, port_number)->pd_context, (pd_timing_t *)timing,
			1<<PIPE(display)->pipe_num);
		if (ret) {
			EMGD_ERROR_EXIT("PD set_mode returned: 0x%x",ret);
			return -IGD_INVAL;
		}
		break;
	case IGD_POWERSTATE_D1:
		port_control_analog &= ~0x80000000;
		port_control_analog |= 0x00000800 &
			~(PORT(display, port_number)->preserve);
		pd_powerstate =PD_POWER_MODE_D1;
		break;
	case IGD_POWERSTATE_D2:
		port_control_analog &= ~0x80000000;
		port_control_analog |= 0x00000400 &
			~(PORT(display, port_number)->preserve);
		pd_powerstate = PD_POWER_MODE_D2;
		break;
	case IGD_POWERSTATE_D3:
		port_control_analog &= ~0x80000000;
		port_control_analog |= 0x00003c00 &
			~(PORT(display, port_number)->preserve);
		pd_powerstate = PD_POWER_MODE_D3;
		break;
	default:
		EMGD_ERROR_EXIT("Invalid power state: 0x%lx",
			GET_DISPLAY_POWER_STATE(display, port_number));
		return -IGD_ERROR_INVAL;
	}

	ret = PORT(display, port_number)->pd_driver->set_power(
			PORT(display, port_number)->pd_context, pd_powerstate);
	if (ret) {
		EMGD_ERROR_EXIT("PD set_power returned: 0x%x", ret);
		return -IGD_ERROR_INVAL;
	}

	if(port_type == IGD_PORT_DIGITAL) {
		EMGD_DEBUG("Port_control = 0x%lx", port_control);

		WRITE_MMIO_REG(display, PORT(display, port_number)->port_reg,
				port_control);
		if(PORT(display, port_number)->mult_port) { /* gang mode or rgba*/
			WRITE_MMIO_REG(display,
					PORT(display, port_number)->mult_port->port_reg,
					mult_port_control);
		}
	}

	EMGD_TRACE_EXIT;
	return 0;
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
static int post_program_port_plb(igd_display_context_t *display,
	unsigned short port_number,
	unsigned long status)
{
	int ret;
	igd_display_port_t *port;
	igd_timing_info_t  *timings;
	unsigned long portreg;

	EMGD_TRACE_ENTER;

	port = PORT(display, port_number);
	timings = PIPE(display)->timing;

	/*
	 * Writing the sDVO control register here works around a problem
	 * where the sDVO port is not turning on when using a CH7308
	 * card in a 915 GM based system and the port order is 5200.
	 *
	 * In addition, post_set_mode() below, will report that the
	 * "inputs are not trained", however, this does not seem to
	 * have any negative effects.
	 */
	portreg = READ_MMIO_REG(display, port->port_reg);
	WRITE_MMIO_REG(display, port->port_reg, (portreg & ~BIT31));
	WRITE_MMIO_REG(display, port->port_reg, portreg);

	/* Reenable/Redisable other	port */
	if (port->port_reg == 0x61140) {
		WRITE_MMIO_REG(display, 0x61160, READ_MMIO_REG(display,
					0x61160));
	} else {
		WRITE_MMIO_REG(display, 0x61140,
				READ_MMIO_REG(display, 0x61140));
	}

	/*
	 * Added for Lakeport A0
	 * Port clock multiplier bits 4-7, needs to be rewritten
	 */
	WRITE_MMIO_REG(display, PIPE(display)->clock_reg->dpll_control,
		READ_MMIO_REG(display, PIPE(display)->clock_reg->dpll_control));

	/* We must wait for 150 us for the dpll clock to warm up */
	OS_SLEEP(150);

	ret = 0;
	/* call post_set_mode() if exists */
	if (port->pd_driver->post_set_mode) {
		ret = port->pd_driver->post_set_mode(port->pd_context, timings,
			1<<PIPE(display)->pipe_num);
		if (ret) {
			EMGD_ERROR("PD post_set_mode returned: 0x%x", ret);
		}
	}

	EMGD_TRACE_EXIT;
	return ret;
}

int program_cdvo_plb(void)
{
	return TRUE;
}

/*!
 *
 * @param gpio
 *
 * @return size
 */
static unsigned long get_gpio_sets_plb(unsigned long **gpio)
{
	*gpio = gpio_plb;
	return sizeof(gpio_plb)/sizeof(unsigned long);
}

/*!
 *
 * @param context
 * @param in_list
 *
 * @return void - To small to trace
 */
static void filter_modes_plb(igd_context_t *context, igd_display_port_t *port,
	pd_timing_t *in_list)
{

	return;
}

#ifndef CONFIG_MICRO
int check_display_plb(igd_display_context_t *display,
					  unsigned short port_number, unsigned long status){
	/* For poulsbo display should be working fine */
	return TRUE;
}
#endif

mode_dispatch_t mode_dispatch_plb = {
	igd_set_palette_entry_plb,
	igd_get_palette_entry_plb,
	igd_wait_vblank_plb,
	program_plane_plb,
	program_pipe_plb,
	program_port_plb,
	post_program_port_plb,
	program_clock_plb,
	program_cdvo_plb,
	reset_plane_pipe_ports_plb,
	get_gpio_sets_plb,
	filter_modes_plb,
	OPT_MICRO_VALUE(&check_display_plb, NULL),
	NULL,
	NULL,
	NULL,
	NULL,
	OPT_MICRO_VALUE(&mode_full_dispatch_plb, NULL),
};

#endif

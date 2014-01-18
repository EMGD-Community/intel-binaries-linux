/*
 *-----------------------------------------------------------------------------
 * Filename: mode_tnc.c
 * $Revision: 1.36 $
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
 *  Atom E6xx implementations for the mode dispatch functions.
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
#include <intelpci.h>
#include <mode_access.h>
#include <rb.h>
#include <pi.h>
#include <math_fix.h>

#include <tnc/regs.h>
#include <tnc/context.h>
#include <tnc/mi.h>
#include <tnc/instr.h>
#include <tnc/cmd.h>

#include "drm_emgd_private.h"
#include "../cmn/match.h"
#include "../cmn/mode_dispatch.h"

#include "emgd_shared.h"
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <drm/drmP.h>

#include <emgd_drm.h>

/* Get this table from clocks_tnc.c, use this in get_pipe_info */
extern unsigned long lvds_m_converts[];
extern unsigned long LVDS_M_CONVERTS_LEN;

/* This is a duplicate define of the same constant in clocks_tnc.c.
 * Redefined it here because there's no good TNC-specific common header
 * file to put this in  */
#define LVDS_M_MIN 10


/* Registered VBlank interrupt callbacks (one-per-pipe): */
static emgd_vblank_callback_t interrupt_callbacks_tnc[IGD_MAX_PORTS] =
	{{NULL,NULL,0},{NULL,NULL,1},{NULL,NULL,2},{NULL,NULL,3},{NULL,NULL,4}};

/* This variable contains a number of bits, which tell whether the interrupt
 * code has enabled interrupts, and if so, for what software and for what
 * port(s); and whether certain events have happened.
 */
static unsigned long vblank_interrupt_state = 0;

/* This variables keeps track of the number of clients currently using
 * the vblank interrupt
 */
static int vblank_interrupt_ref_cnt_port2 = 0;
static int vblank_interrupt_ref_cnt_port4 = 0;

/* Spin lock for synchronization of the vblank_interrupt_state variable,
 * between the VBlank interrupt handler and the non-interrupt handler code:
 */
DEFINE_SPINLOCK(vblank_lock_tnc);



int set_flip_pending_tnc(unsigned char *mmio, unsigned long pipe_status_reg);
int check_flip_pending_tnc(unsigned char *mmio, unsigned long pipe_status_reg);
unsigned long get_port_control_tnc(unsigned long port_num, unsigned long port_reg);

/* KMS callback from emgd_crtc.c */
int crtc_pageflip_handler(struct drm_device *dev, int port);
void notify_userspace_vblank(struct drm_device *dev, int port);

/*!
 * @addtogroup display_group
 * @{
 */

int mode_get_stride_stereo_tnc(igd_display_context_t *display,
	unsigned long *stride, unsigned long *stereo, unsigned long flags);



/*!
 *
 * @param display
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL if color attributes not found
 */
static int set_color_correct_tnc(igd_display_context_t *display)
{
	const int        MID_PIXEL_VAL    = 125;
	const int        MAX_PIXEL_VAL    = 255;
	const int        NUM_PALETTE_ENTRIES = 256;

	unsigned int     gamma_r_max_24i_8f, gamma_r_min_24i_8f;
	unsigned int     gamma_g_max_24i_8f, gamma_g_min_24i_8f;
	unsigned int     gamma_b_max_24i_8f, gamma_b_min_24i_8f;
	unsigned int     new_gamma_r_24i_8f, new_gamma_g_24i_8f;
	unsigned int     new_gamma_b_24i_8f;
	unsigned int     gamma_normal_r_24i_8f, gamma_normal_g_24i_8f;
	unsigned int     gamma_normal_b_24i_8f;
	int              brightness_factor_r, brightness_factor_g;
	int              brightness_factor_b;
	int              contrast_factor_r, contrast_factor_g;
	int              contrast_factor_b;

	unsigned int      *palette;
	unsigned int      i;

	igd_range_attr_t *gamma_attr      = NULL, *contrast_attr = NULL;
	igd_range_attr_t *brightness_attr = NULL;
	igd_attr_t       *hal_attr_list  = PORT_OWNER(display)->attributes;

	EMGD_TRACE_ENTER;

	/* Using OS_ALLOC to avoid using > 1024 on stack (frame size warning ) */
	palette = OS_ALLOC(sizeof (unsigned int) * NUM_PALETTE_ENTRIES);

	/* start with a fresh palette */
	for (i = 0; i < NUM_PALETTE_ENTRIES; i++) {
		palette[i] = (i << 16) | (i << 8) | i;
	}

	/* get a pointer to gamma, contrast, and brightness attr */
	i = 0;

	while (PD_ATTR_LIST_END != hal_attr_list[i].id) {
		switch (hal_attr_list[i].id) {
		case PD_ATTR_ID_FB_GAMMA:
			gamma_attr      = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		case PD_ATTR_ID_FB_BRIGHTNESS:
			brightness_attr = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		case PD_ATTR_ID_FB_CONTRAST:
			contrast_attr   = (igd_range_attr_t *) &hal_attr_list[i];
			break;

		default:
			break;
		}

		i++;
	}

	if(!gamma_attr || !brightness_attr || !contrast_attr) {
		EMGD_ERROR_EXIT("Color Correction Atrributes not found!");
		return -IGD_ERROR_INVAL;
	}

	/* Get the max and min */
	gamma_r_max_24i_8f = ((gamma_attr->max >> 16) & 0xFF) << 3;
	gamma_g_max_24i_8f = ((gamma_attr->max >>  8) & 0xFF) << 3;
	gamma_b_max_24i_8f =  (gamma_attr->max        & 0xFF) << 3;

	gamma_r_min_24i_8f = ((gamma_attr->min >> 16) & 0xFF) << 3;
	gamma_g_min_24i_8f = ((gamma_attr->min >>  8) & 0xFF) << 3;
	gamma_b_min_24i_8f =  (gamma_attr->min        & 0xFF) << 3;

	/* The new gamma values are in 3i.5f format, but we must convert it
	 * to 24i.8f format before passing it to OS_POW_FIX
	 */
	new_gamma_r_24i_8f = ((gamma_attr->current_value >> 16) & 0xFF) << 3;
	new_gamma_g_24i_8f = ((gamma_attr->current_value >> 8) & 0xFF) << 3;
	new_gamma_b_24i_8f = (gamma_attr->current_value & 0xFF) << 3;

	/* make sure the new gamma is within range */
	new_gamma_r_24i_8f = OS_MIN(gamma_r_max_24i_8f, new_gamma_r_24i_8f);
	new_gamma_r_24i_8f = OS_MAX(gamma_r_min_24i_8f, new_gamma_r_24i_8f);
	new_gamma_g_24i_8f = OS_MIN(gamma_g_max_24i_8f, new_gamma_g_24i_8f);
	new_gamma_g_24i_8f = OS_MAX(gamma_g_min_24i_8f, new_gamma_g_24i_8f);
	new_gamma_b_24i_8f = OS_MIN(gamma_b_max_24i_8f, new_gamma_b_24i_8f);
	new_gamma_b_24i_8f = OS_MAX(gamma_b_min_24i_8f, new_gamma_b_24i_8f);


	gamma_normal_r_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_r_24i_8f);

	gamma_normal_g_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_g_24i_8f);

	gamma_normal_b_24i_8f =
		OS_POW_FIX(MAX_PIXEL_VAL, (1<<16)/new_gamma_b_24i_8f);

	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		unsigned int new_gamma;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* Note that we do not try to calculate the gamma if it
		 * is 1.0, e.g. 0x100.  This is to avoid round-off errors
		 */

		/* red: calculate and make sure the result is within range */
		if (0x100 != new_gamma_r_24i_8f) {
			cur_color  = (cur_palette >> 16) & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_r_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_r_24i_8f;
			palette[i] &= 0x00FFFF;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF) << 16;
		}

		/* green: calculate and make sure the result is within range */
		if (0x100 != new_gamma_g_24i_8f) {
			cur_color  = (cur_palette >> 8) & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_g_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_g_24i_8f;
			palette[i] &= 0xFF00FF;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF) << 8;
		}

		/* blue: calculate and make sure the result is within range */
		if (0x100 != new_gamma_b_24i_8f) {
			cur_color  = cur_palette & 0xFF;
			new_gamma  = OS_POW_FIX(cur_color, (1<<16)/new_gamma_b_24i_8f);
			new_gamma  = (MAX_PIXEL_VAL * new_gamma)/gamma_normal_b_24i_8f;
			palette[i] &= 0xFFFF00;
			palette[i] |=
				(OS_MIN(new_gamma, (unsigned) MAX_PIXEL_VAL) & 0xFF);
		}
	}


	/* Brightness correction */
	brightness_factor_r = (brightness_attr->current_value >> 16) & 0xFF;
	brightness_factor_g = (brightness_attr->current_value >> 8) & 0xFF;
	brightness_factor_b = brightness_attr->current_value & 0xFF;

	/* The factors are offset by 0x80 because 0x80 is 0 correction */
	brightness_factor_r -= 0x80;
	brightness_factor_g -= 0x80;
	brightness_factor_b -= 0x80;

	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		int          new_pixel_val;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* red: calculate and make sure the result is within range */
		cur_color     =  (cur_palette >> 16) & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_r;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0x00FFFF;
		palette[i]    |= (OS_MAX(new_pixel_val, 0) & 0xFF) << 16;

		/* green: calculate and make sure the result is within range */
		cur_color     =  (cur_palette >> 8) & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_g;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0xFF00FF;
		palette[i]    |= (OS_MAX(new_pixel_val, 0) & 0xFF) << 8;

		/* blue: calculate and make sure the result is within range */
		cur_color     =  cur_palette & 0xFF;
		new_pixel_val =  cur_color + brightness_factor_b;
		new_pixel_val =  OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
		palette[i]    &= 0xFFFF00;
		palette[i]    |= OS_MAX(new_pixel_val, 0) & 0xFF;
	}


	/* contrast correction */
	contrast_factor_r = (contrast_attr->current_value >> 16) & 0xFF;
	contrast_factor_g = (contrast_attr->current_value >> 8) & 0xFF;
	contrast_factor_b = contrast_attr->current_value & 0xFF;

	/* make sure values are within range */
	contrast_factor_r -= 0x80;
	contrast_factor_g -= 0x80;
	contrast_factor_b -= 0x80;


	/* We're doing integer division in this loop using 16i.16f
	 * integers.  The result will then be converted back into a
	 * regular, 32-bit integer
	 */
	for( i = 0; i < NUM_PALETTE_ENTRIES; i++ ) {
		int new_pixel_val;
		unsigned int cur_color;
		unsigned int cur_palette = palette[i];

		/* red: calculate and make sure the result is within range */
		if (0 != contrast_factor_r ) {
			cur_color     = (cur_palette >> 16) & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_r);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0x00FFFF;  /* clear out the R color */
			palette[i]    |=  (OS_MAX(new_pixel_val, 0) & 0xFF) << 16;
		}

		/* green: calculate and make sure the result is within range */
		if (0 != contrast_factor_g ) {
			cur_color     = (cur_palette >> 8) & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_g);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0xFF00FF;  /* clear out the G color */
			palette[i]    |=  (OS_MAX(new_pixel_val, 0) & 0xFF) << 8;
		}

		/* blue: calculate and make sure the result is within range */
		if (0 != contrast_factor_b) {
			cur_color     = cur_palette & 0xFF;
			new_pixel_val =
				(MAX_PIXEL_VAL << 16) / (MAX_PIXEL_VAL - contrast_factor_b);
			new_pixel_val =   new_pixel_val * (cur_color - MID_PIXEL_VAL);
			new_pixel_val >>= 16;  /* convert back to 32i format */
			new_pixel_val +=  MID_PIXEL_VAL;
			new_pixel_val =   OS_MIN(new_pixel_val, MAX_PIXEL_VAL);
			palette[i]    &=  0xFFFF00;  /* clear out the B color */
			palette[i]    |=   OS_MAX(new_pixel_val, 0) & 0xFF;
		}
	}


	/* write the new values in the palette */
	for (i = 0; i < NUM_PALETTE_ENTRIES; i++) {
		/* SDVO palette register is not accesible */
		EMGD_WRITE32(palette[i],
			MMIO_TNC(IGD_PORT_LVDS) +
			PIPE(display)->palette_reg + i*4);
	}

	OS_FREE(palette);

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param display
 * @param fb
 * @param x
 * @param y
 *
 * @return 0
 */
static int set_display_base_tnc(igd_display_context_t *display,
	igd_framebuffer_info_t *fb, unsigned long *x, unsigned long *y)
{
	unsigned long reg;
	unsigned long temp;
	EMGD_TRACE_ENTER;

	EMGD_DEBUG ("Pan linear to (%lu,%lu)", *x, *y);

	/* Update framebuffer's visible offset */
	PLANE(display)->fb_info->visible_offset =
		((*y * fb->screen_pitch) + (*x * IGD_PF_BYPP(fb->pixel_format)));

	temp = PLANE(display)->fb_info->visible_offset;
	EMGD_DEBUG ("visible offset = %lx",temp );
	if(PLANE(display)->fb_info->lock) {

		EMGD_DEBUG ("Plane is locked");
		reg = (unsigned long)READ_MMIO_REG(display, PLANE(display)->plane_reg- 4);
		EMGD_DEBUG("Plane B start addr = %lx", reg);
		reg = (unsigned long)READ_MMIO_REG(display,PLANE(display)->plane_reg + 4);
		EMGD_DEBUG("Plane B start offset = %lx", reg);
	
	} else {
		/* Plane registers are always on 0:2:0 */
		WRITE_MMIO_REG(display, PLANE(display)->plane_reg + DSP_LINEAR_OFFSET,
				PLANE(display)->fb_info->visible_offset);
	}
	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function alters the position parameters associated with a cursor.
 *
 * @param display_handle
 * @param cursor_info
 *
 * @return 0
 */
static int igd_alter_cursor_pos_tnc(igd_display_h display_handle,
	igd_cursor_info_t *cursor_info)
{
	unsigned long cursor_reg;
	unsigned long new_pos;
	unsigned long cursor_base;

	igd_display_context_t *display = (igd_display_context_t *) display_handle;
	cursor_reg = PIPE(display)->cursor->cursor_reg;

	/* To Fast For Tracing */

	/* Plane registers are always on 0:2:0 device */
	if (0x27 & READ_MMIO_REG(display, cursor_reg)) {
		/* unlike almador, for Gen4, u must program the base offset
		to trigger the position update. However, this also means we
		accidentally enable an invalid cursor surface if the cursor
		was not enabled already. So do this check first */

		/*
		 * Encode the cursor position in the format required for the
		 * cursor position register.
		 */
		if(cursor_info->y_offset >= 0) {
			new_pos = (cursor_info->y_offset << 16);
		} else {
			new_pos = ((-(cursor_info->y_offset)) << 16) | 0x80000000;
		}
		if(cursor_info->x_offset >= 0) {
			new_pos |= (cursor_info->x_offset);
		} else {
			new_pos |= (-(cursor_info->x_offset)) | 0x00008000;
		}

		cursor_base = READ_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET);

		WRITE_MMIO_REG(display, cursor_reg + CUR_POS_OFFSET, new_pos);
		WRITE_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET, cursor_base);
	}

	return 0;
}

/*!
 * The assumption here is that palette_colors points to index 0 and
 * this function indexes into the palette_colors array by start_index
 *
 * @param display_handle
 * @param palette_colors
 * @param start_index
 * @param count
 *
 * @return 0
 */
static int igd_set_palette_entries_tnc(
		igd_display_h display_handle,
		unsigned long *palette_colors,
		unsigned int start_index,
		unsigned int count)
{
	unsigned int i;

	EMGD_TRACE_ENTER;

	for(i=start_index; i<start_index+count; i++) {
		/* Palette can is only on 0:2:0 so use _TNC IGD_PORT_LVDS */
		WRITE_MMIO_REG_TNC(IGD_PORT_LVDS,
			PIPE(display_handle)->palette_reg + i*4, palette_colors[i]);
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This procedure waits for the next vertical sync
 * period. If the display is already in a vertical sync period, this
 * procedure exits.
 *
 * Note: A timeout is included to prevent an endless loop.
 *
 * @param display_handle
 * @param palette_colors
 * @param start_index
 * @param count
 *
 * @return FALSE - if timed out
 */
static int igd_wait_vsync_tnc(igd_display_h display_handle)
{
	unsigned long tmp;
	unsigned long status_reg;
	os_alarm_t timeout;
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	int ret;

	EMGD_TRACE_ENTER;

	status_reg = PIPE(display)->pipe_reg + PIPE_STATUS_OFFSET;

	/* If pipe is off then just return */
	if(!((1<<31) & READ_MMIO_REG_TNC(PORT_TYPE(display),
		PIPE(display)->pipe_reg))) {
		EMGD_TRACE_EXIT;
		return 1;
	}

	/* 1. Disable VSync interrupt */
	tmp = READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	WRITE_MMIO_REG_TNC(PORT_TYPE(display), status_reg, tmp & ~(1<<25));

	/* 2. Clear interrupt status (by writing a 1) */
	tmp = READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	WRITE_MMIO_REG_TNC(PORT_TYPE(display), status_reg, tmp | (1<<9));

	/* 3. Enable VSync interrupt */
	tmp = READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	WRITE_MMIO_REG_TNC(PORT_TYPE(display), status_reg, tmp | (1<<25));
	READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);

	/* 4. Wait for VSync about 50 msec (20Hz) */
	timeout = OS_SET_ALARM(50);
	do {
		OS_SCHEDULE();
		/* Check for timeout */
	} while (((READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg) & (1<<9)) ==
			0x00) &&
		(!OS_TEST_ALARM(timeout)));

	if (((READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg) & (1<<9)) ==
			0x00)) {
		EMGD_ERROR_EXIT("Timeout waiting for VSYNC");
		ret = 0;
	} else {
		ret = 1;
	}

	/* 5. Disable VSync interrupt */
	tmp = READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	WRITE_MMIO_REG_TNC(PORT_TYPE(display), status_reg, tmp & ~(1<<25));

	/* 6. Clear interrupt status (by writing a 1) */
	tmp = READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	WRITE_MMIO_REG_TNC(PORT_TYPE(display), status_reg, tmp | (1<<9));
	READ_MMIO_REG_TNC(PORT_TYPE(display), status_reg);


	EMGD_TRACE_EXIT;
	return ret;
}  /* igd_wait_vsync*/

/*!
 *
 * @param display_handle
 * @param scanline
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int igd_get_scanline_tnc(igd_display_h display_handle, int *scanline)
{
	unsigned int tmp;
	unsigned long reg;
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	unsigned long fb_height = PLANE(display)->fb_info->height;
	unsigned long dp_height = PIPE(display)->timing->height;

	EMGD_TRACE_ENTER;

	/* Scanline reg is -8 from control reg */
	reg = PIPE(display)->pipe_reg - 0x8;

	tmp = READ_MMIO_REG_TNC(PORT_TYPE_DH(display_handle), reg);

	if(!(PORT_OWNER(display)->pt_info->flags & IGD_DISPLAY_ENABLE)) {
		EMGD_DEBUG("Port off, can not get scanline");
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}

	tmp = (tmp * fb_height) / dp_height;

	if(tmp >= fb_height) {
		*scanline = IGD_IN_VBLANK;
	} else {
		*scanline = (int)tmp;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end igd_get_scanline() */

/*!
 *
 * @param display_handle
 *
 * @return 1 if TRUE
 * @return 0 if FALSE
 */
static int igd_query_in_vblank_tnc(igd_display_h display_handle)
{
	int sl;

	/* To small for Tracing */

	igd_get_scanline_tnc(display_handle, &sl);
	if (sl == IGD_IN_VBLANK) {
		return 1; /*TRUE*/
	} else {
		return 0; /*FALSE*/
	}
}

/*!
 * This function programs the cursor registers for Grantsdale
 *
 * @param display
 * @param status
 *
 * @return void
 */
static void program_cursor_tnc(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long cursor_reg;
	unsigned long cursor_control = 0x00000000;
	unsigned long cursor_pos;
	unsigned long cursor_base;
	igd_cursor_info_t *cursor_info;
	int i;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Program Cursor: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	cursor_reg = PIPE(display)->cursor->cursor_reg;
	cursor_info = PIPE(display)->cursor->cursor_info;

	/* Turn off cursor before changing anything */
	/* planes are always on 0:2:0 device, so no need to use _TNC macros */
	cursor_base = READ_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET);

	WRITE_MMIO_REG(display, cursor_reg, cursor_control);
	WRITE_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET, cursor_base);

	if(cursor_info->flags & IGD_CURSOR_GAMMA) {
		cursor_control |= BIT26;
	}

	cursor_info->argb_pitch = 64*4;
	cursor_info->xor_pitch = 16;

	/* Setting the cursor format/pitch */
	switch(cursor_info->pixel_format) {
	case IGD_PF_ARGB32:
		cursor_control |= BIT5 | 0x7;
		break;
	case IGD_PF_RGB_XOR_2:
		cursor_control |= 0x5;
		break;
	case IGD_PF_RGB_T_2:
		cursor_control |= 0x4;
		break;
	case IGD_PF_RGB_2:
		cursor_control |= 0x6;
		break;
	default:
		EMGD_ERROR_EXIT("Invalid Pixel Format");
		return;
	}

	switch(cursor_info->pixel_format) {
	case IGD_PF_ARGB32:
		cursor_base = cursor_info->argb_offset;
		break;
	default:
		cursor_base = cursor_info->xor_offset;
		break;
	}

	/* If status is FALSE return with the cursor off */
	if((!status) || (GET_DEVICE_POWER_STATE(display) == IGD_POWERSTATE_D3)) {
		EMGD_TRACE_EXIT;
		return;
	}

	if(cursor_info->y_offset >= 0) {
		cursor_pos = cursor_info->y_offset << 16;
	} else {
		cursor_pos = ((-(cursor_info->y_offset)) << 16) | 0x80000000;
	}
	if(cursor_info->x_offset >= 0) {
		cursor_pos |= cursor_info->x_offset;
	} else {
		cursor_pos |= (-(cursor_info->x_offset)) | 0x00008000;
	}

	WRITE_MMIO_REG(display, cursor_reg + CUR_POS_OFFSET,
		cursor_pos);

	for(i=0; i<4; i++) {
		WRITE_MMIO_REG(display, cursor_reg + CUR_PAL0_OFFSET + i*4,
			cursor_info->palette[i]);
	}

	cursor_control = cursor_control | (PIPE(display)->pipe_num<<28);

	WRITE_MMIO_REG(display, cursor_reg, cursor_control);
	WRITE_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET, cursor_base);

	EMGD_TRACE_EXIT;
}

/*!
 *
 * @param display_handle
 * @param type
 * @param surface
 * @param appcontext
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int igd_get_surface_tnc(igd_display_h display_handle,
	igd_buffertype_t type,
	igd_surface_t *surface,
	igd_appcontext_h appcontext)
{
#if 0
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	state3d_tnc_t *state = STATE3D_TNC(appcontext);

	EMGD_TRACE_ENTER;

	if(!surface) {
		EMGD_ERROR_EXIT("Surface is NULL");
		return -IGD_ERROR_INVAL;
	}

	switch(type) {
	case IGD_BUFFER_DISPLAY:
		surface->offset = PLANE(display)->fb_info->visible_offset;
		surface->pitch = PLANE(display)->fb_info->screen_pitch;
		surface->width = PLANE(display)->fb_info->width;
		surface->height = PLANE(display)->fb_info->height;
		surface->u_offset = 0;
		surface->u_pitch = 0;
		surface->v_offset = 0;
		surface->v_pitch = 0;
		surface->pixel_format = PLANE(display)->fb_info->pixel_format;
		surface->palette_info = 0;
		surface->flags = PLANE(display)->fb_info->flags;
		surface->logic_ops = 0;
		surface->render_ops = 0;
		surface->alpha = 0;
		surface->diffuse = 0;
		surface->chroma_high = 0;
		surface->chroma_low = 0;
		EMGD_TRACE_EXIT;
		return 0;
	case IGD_BUFFER_COLOR:
		OS_MEMCPY(surface, &state->color_buffer,
			sizeof(igd_surface_t));
		EMGD_TRACE_EXIT;
		return 0;
	case IGD_BUFFER_DEPTH:
		OS_MEMCPY(surface, &state->depth_buffer,
			sizeof(igd_surface_t));
		EMGD_TRACE_EXIT;
		return 0;
	default:
		EMGD_ERROR("Invalid type in get_surface");
		break;
	}

	EMGD_TRACE_EXIT;
	return -IGD_ERROR_INVAL;
#else
	return 0;
#endif
}

/*!
 *
 * @param display_handle
 * @param type
 * @param surface
 * @param appcontext
 * @param flags
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int igd_set_surface_tnc(igd_display_h display_handle,
	int priority,
	igd_buffertype_t type,
	igd_surface_t *surface,
	igd_appcontext_h appcontext,
	unsigned long flags)
{
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	unsigned int dsp_current;
	unsigned long plane_reg;
	unsigned long plane_control;
	unsigned long visible_offset;

	EMGD_TRACE_ENTER;
	if(!surface) {
		return -IGD_ERROR_INVAL;
	}

	switch(type) {
	case IGD_BUFFER_DISPLAY:
		if(! (surface->flags & IGD_SURFACE_DISPLAY)) {
			EMGD_ERROR_EXIT("Surface is not a display surface");
			return -IGD_ERROR_INVAL;
		}

		if (PLANE(display)->plane_reg == DSPACNTR) {
			EMGD_DEBUG("About to flip a buffer for display/pipe A");
			dsp_current = 0;
		} else {
			EMGD_DEBUG("About to flip a buffer for display/pipe B");
			dsp_current = 1;
		}

		if(flags & IGD_BUFFER_WAIT) {
			/* If this is just a wait for flip, so return */
			return 0;
		}

		/*
		 * Async flips only work when the offset is on a 256kb boundary.
		 */
		if(surface->offset & 0x3ffff) {
			EMGD_ERROR("Display surface offset %lu is not 256kb aligned", surface->offset);
		}
		
		if (flags & IGD_BUFFER_NO_PAN) {
			/* Do not pan. Set visible_offset to zero */
			visible_offset = 0;
		} else {
			/* calculate the visible offset, taking panning into account */
			visible_offset =
				(PORT_OWNER(display)->pt_info->y_offset * surface->pitch) +
				(PORT_OWNER(display)->pt_info->x_offset *
					IGD_PF_BYPP(surface->pixel_format));
		}
		EMGD_DEBUG("visible surface_offset = 0x%08lx", visible_offset);

		/* Save new fb_info */
		PLANE(display)->fb_info->fb_base_offset = surface->offset;
		PLANE(display)->fb_info->visible_offset = visible_offset;
		PLANE(display)->fb_info->screen_pitch = surface->pitch;
		PLANE(display)->fb_info->width = surface->width;
		PLANE(display)->fb_info->height = surface->height;
		PLANE(display)->fb_info->pixel_format = surface->pixel_format;
		PLANE(display)->fb_info->flags = surface->flags;

		/* Get the correct stride and stereo */
		/* TODO - Does Atom E6xx flip need to handle stereo mode? */
		/*mode_get_stride_stereo_tnc(display, &stride, &stereo, 0);*/

		/* plane registers are always on 0:2:0, so no need to use _TNC macros */
		plane_reg = PLANE(display)->plane_reg;
		plane_control = EMGD_READ32(MMIO(display) + plane_reg);

		/* Perform the flip by doing the following:
		 *
		 *   Write the current plane_control value to the plane_reg
		 *   Write the surface stride to DSP_STRIDE_OFFSET
		 *   Write the visible from start of plane to DSP_LINEAR_OFFSET
		 *   Write the base surface offset to either:
		 *     1) the plane_reg - 4  if async
		 *     2) plane_reg + DSP_START_OFFSET (+0x1C) if not async
		 */
		if(!PLANE(display)->fb_info->lock){
			EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
			EMGD_WRITE32(surface->pitch,
					MMIO(display) + plane_reg + DSP_STRIDE_OFFSET);
			EMGD_WRITE32(visible_offset,
				MMIO(display) + plane_reg + DSP_LINEAR_OFFSET);
			EMGD_WRITE32(surface->offset,
				MMIO(display) + plane_reg + DSP_START_OFFSET);
		}

		EMGD_TRACE_EXIT;
		return 0;
	case IGD_BUFFER_COLOR:
		EMGD_TRACE_EXIT;
		return 0;
	case IGD_BUFFER_DEPTH:
		EMGD_TRACE_EXIT;
		return 0;
	case IGD_BUFFER_SAVE:
		PLANE(display)->fb_info->saved_offset = surface->offset;
		EMGD_DEBUG("saving surface_offset = 0x%08lx", surface->offset);
		EMGD_TRACE_EXIT;
		return 0;
	default:
		EMGD_ERROR("Invalid type in set_surface");
		break;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/* Atom E6xx does not support a flip pending, since there is no
 * Display Buffer Info instruction.  So this must be done with vBlank.
 * However, a wait_for_vblank can be given while a flip is also in
 * progress, so a semaphore is required when changing flip_pending or
 * when modifying the vBlank interrupt bits.
 *
 * This function should only be called with a flip_mutex around it */
int set_flip_pending_tnc(unsigned char *mmio, unsigned long pipe_status_reg)
{
	platform_context_tnc_t *tnc_context =
		(platform_context_tnc_t *)mode_context->context->platform_context;
	unsigned long request_for;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT((pipe_status_reg == PIPEB_STAT) ||
		(pipe_status_reg == PIPEA_STAT), "Invalid pipe_status_reg", 0);

	if (pipe_status_reg == PIPEB_STAT) {
		tnc_context->flip_pending |= PLB_FLIP_PIPE_B_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2);
	} else {
		tnc_context->flip_pending |= PLB_FLIP_PIPE_A_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4);
	}
	mode_context->dispatch->full->request_vblanks(request_for, mmio);

	EMGD_TRACE_EXIT;
	return 0;
}

/* This function should only be called with a flip_mutex around it */
int check_flip_pending_tnc(unsigned char *mmio,
	unsigned long pipe_status_reg)
{
	platform_context_tnc_t *tnc_context =
		(platform_context_tnc_t *)mode_context->context->platform_context;
	unsigned long request_for;
	unsigned int flip_pending;

	EMGD_TRACE_ENTER;
	EMGD_ASSERT((pipe_status_reg == PIPEB_STAT) ||
		(pipe_status_reg == PIPEA_STAT), "Invalid pipe_status_reg", 0);

	if (pipe_status_reg == PIPEB_STAT) {
		flip_pending = tnc_context->flip_pending & PLB_FLIP_PIPE_B_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2);
	} else {
		flip_pending = tnc_context->flip_pending & PLB_FLIP_PIPE_A_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4);
	}

	if (flip_pending) {
		if (mode_context->dispatch->full->vblank_occured(request_for)) {
			/* VBlank occured, flip complete */
			tnc_context->flip_pending &= ~flip_pending;
			mode_context->dispatch->full->end_request(request_for, mmio);
			EMGD_DEBUG("VBlank occured--returning 0");
			return 0;
		} else {
			/* VBlank not done, flip still in progress */
			EMGD_DEBUG("VBlank hasn't yet occured--returning 1");
			return 1;
		}
	} else {
		/* No flip pending, so it must have completed */
		EMGD_DEBUG("returning 0");
		return 0;
	}
}

/*!
 *
 * @param display_handle
 * @param event
 * @param status
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int igd_query_event_tnc(igd_display_h display_handle,
	igd_event_t event, unsigned long *status)
{
	platform_context_tnc_t *tnc_context =
		(platform_context_tnc_t *)mode_context->context->platform_context;
	unsigned char *mmio = MMIO(display_handle);
	unsigned long pipe_status_reg =
		(PLANE(display_handle)->plane_reg == DSPACNTR) ? 0x70024 : 0x71024;
	int ret;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("mmio=0x%p, pipe_status_reg=0x%08lx", mmio, pipe_status_reg);

	switch (event) {
	case IGD_EVENT_FLIP_PENDING:
		ret = OS_PTHREAD_MUTEX_LOCK(&tnc_context->flip_mutex);
		*status = check_flip_pending_tnc(mmio, pipe_status_reg);
		OS_PTHREAD_MUTEX_UNLOCK(&tnc_context->flip_mutex);
		break;
	default:
		return -IGD_ERROR_INVAL;
	}

	EMGD_DEBUG("Returning status=%lu", *status);
	return IGD_SUCCESS;
}

/*!
 *
 * @param void
 *
 * @return 0
 */
static int get_plane_info_tnc(void)
{
	igd_framebuffer_info_t *buffer_info;
	unsigned char* mmio = NULL;
	unsigned long plane_control = 0;
	unsigned long reg = 0;

	EMGD_TRACE_ENTER;

	mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	/* Check that plane A is active and process it */
	plane_control = EMGD_READ32(mmio + DSPACNTR);
	if(plane_control & PLANE_ENABLE){
		buffer_info = &mode_context->fw_info->fb_info[0];

		/* get the DSPASIZE register value */
		reg = (unsigned long)EMGD_READ32(mmio + DSPACNTR + DSP_SIZE_OFFSET);
		buffer_info[0].height = (reg >> 16) & 0xFFF;
		buffer_info[0].width =	reg & 0xFFF;

		/* get the DSPASTRIDE register value */
		buffer_info[0].screen_pitch =
			(unsigned int)EMGD_READ32(mmio + DSPACNTR + DSP_STRIDE_OFFSET);

		/* Following are NOT offset by 1 in fb info */
		buffer_info[0].width++;
		buffer_info[0].height++;
		if((plane_control & (BIT27 | BIT30)) == (BIT27 | BIT30)){
			buffer_info[0].pixel_format = PF_DEPTH_8;
		}
		if((plane_control & (BIT28 | BIT26)) == (BIT28 | BIT26)){
			buffer_info[0].pixel_format = PF_DEPTH_16;
		}
		if((plane_control & (BIT28 | BIT27)) == (BIT28 | BIT27)){
			buffer_info[0].pixel_format = PF_DEPTH_32;
		}
		
		EMGD_DEBUG("Plane A info height==%d, width=%d, pitch=%d", buffer_info[0].height, buffer_info[0].width, buffer_info[0].screen_pitch);
		reg = (unsigned long)EMGD_READ32(mmio + DSPACNTR - 4);
		EMGD_DEBUG("Plane A start addr = %lx", reg);
		reg = (unsigned long)EMGD_READ32(mmio + DSPACNTR + 4);
		EMGD_DEBUG("Plane A start offset = %lx", reg);
	}

	/* Check that plane B is active and process it */
	plane_control = EMGD_READ32(mmio + DSPBCNTR);
	if(plane_control & PLANE_ENABLE){
		buffer_info = &mode_context->fw_info->fb_info[0];

		/* get the DSPBSIZE register value */
		reg = (unsigned long)EMGD_READ32(mmio + DSPBCNTR + DSP_SIZE_OFFSET);
		buffer_info[1].height = (reg >> 16) & 0xFFF;
		buffer_info[1].width  =	reg & 0xFFF;

		/* get the DSPBSTRIDE register value */
		buffer_info[1].screen_pitch =
			(unsigned int)EMGD_READ32(mmio + DSPBCNTR + DSP_STRIDE_OFFSET);

		/* Following are NOT offset by 1 in fb info */
		buffer_info[1].width++;
		buffer_info[1].height++;
		if((plane_control & (BIT27 | BIT30)) == (BIT27 | BIT30)){
			buffer_info[1].pixel_format = PF_DEPTH_8;
		}
		if((plane_control & (BIT28 | BIT26)) == (BIT28 | BIT26)){
			buffer_info[1].pixel_format = PF_DEPTH_16;
		}
		if((plane_control & (BIT28 | BIT27)) == (BIT28 | BIT27)){
			buffer_info[1].pixel_format = PF_DEPTH_32;
		}
	
		EMGD_DEBUG("Plane B info height==%d, width=%d, pitch=%d", buffer_info[1].height, buffer_info[1].width, buffer_info[1].screen_pitch);
		reg = (unsigned long)EMGD_READ32(mmio + DSPBCNTR - 4);
		EMGD_DEBUG("Plane B start addr = %lx", reg);
		reg = (unsigned long)EMGD_READ32(mmio + DSPBCNTR + 4);
		EMGD_DEBUG("Plane B start offset = %lx", reg);
}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param void
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int get_pipe_info_tnc(igd_display_h *display)
{
	unsigned char *mmio = NULL;
	unsigned char *mmio_sdvo = NULL;
	unsigned long pipe_conf = 0;
	igd_display_info_t *timing;
	unsigned long reg = 0;

	EMGD_TRACE_ENTER;


	mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	mmio_sdvo =
		EMGD_MMIO(mode_context->context->device_context.virt_mmadr_sdvo);

	/*
	 * On Atom E6xx only
	 *    PIPE A + LVDS and
	 *    PIPE B + SDVO are valid.
	 *
	 * Read both VGA_TNC(0:2:0) and SDVO_TNC(0:3:0) mmio reg values in
	 * determining the current timing
	 *
	 * [FIXME]
	 * 1) Reading both device mmio regs in determining FW timings
	 * 2) Update dclk equation as it changed for TNC.
	 */
	pipe_conf = EMGD_READ32(mmio + PIPEA_CONF);
	if(pipe_conf & BIT(31)) { /* pipe A is active */
		timing = &mode_context->fw_info->timing_arr[0];

		reg = EMGD_READ32(mmio + HTOTAL_A);
		timing[0].htotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[0].width = (unsigned short)reg & 0xFFF;

		reg = EMGD_READ32(mmio + HBLANK_A);
		timing[0].hblank_start = (unsigned short)reg & 0x1FFF;
		timing[0].hblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = EMGD_READ32(mmio + HSYNC_A);
		timing[0].hsync_start = (unsigned short)reg & 0x1FFF;
		timing[0].hsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = EMGD_READ32(mmio + VTOTAL_A);
		timing[0].vtotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[0].height = (unsigned short)reg & 0xFFF;

		reg = EMGD_READ32(mmio + VBLANK_A);
		timing[0].vblank_start = (unsigned short)reg & 0x1FFF;
		timing[0].vblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = EMGD_READ32(mmio + VSYNC_A);
		timing[0].vsync_start = (unsigned short)reg & 0x1FFF;
		timing[0].vsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		/* Following are not offset by 1 in ptinfo */
		timing[0].width++;
		timing[0].height++;

		EMGD_DEBUG("Pipe A timing width = %d", timing[0].width);
		EMGD_DEBUG("Pipe A timing width = %d", timing[0].height);

		{
			/* Calculate the firmware programmed dot clock */
			unsigned long dplla, fpa0, fpa1;
			unsigned long m_select, m, p1_select, p1, p2;
			unsigned long ref_freq, dclk;
			int j;
			unsigned long lvds_port;

			dplla = EMGD_READ32(mmio + DPLLACNTR);
			fpa0  = EMGD_READ32(mmio + FPA0);
			fpa1  = EMGD_READ32(mmio + FPA1);
			/* Note: Only Pipe A supports LVDS */
			lvds_port = EMGD_READ32(mmio + LVDSCNTR);


			if(dplla & BIT(31)) {

				/*************************************************************
				 * Find M:  read m_select from the register, then do a reverse
				 *          lookup in the lvds_m_converts table
				 ************************************************************/

				/* Bits 0:8 determines where the divisor is coming from */
				if (dplla & 0xFF) {
					EMGD_DEBUG("Using fpa1");
					m_select = (fpa1 >> 8) & 0x3F; /* M is at bits 13:8 */
				} else {
					EMGD_DEBUG("Using fpa0");
					m_select = (fpa0 >> 8) & 0x3F; /* M is at bits 13:8 */
				}

				EMGD_DEBUG("m_select %ld", m_select);

				for( j=0; j < LVDS_M_CONVERTS_LEN; j++) {
					if (m_select == lvds_m_converts[j]) {
						break;
					}
				}

				if (j == LVDS_M_CONVERTS_LEN) {
					EMGD_ERROR_EXIT("Invalid M select value");
					return -IGD_ERROR_INVAL;
				} else {
					m = j + LVDS_M_MIN;
					EMGD_DEBUG("m value %ld", m);
				}


				/*************************************************************
				 * Find P1 and P2: read p1_select from the register, figure
				 *                 out its bit position, then add 2.
				 *                 P2 is always 14 for TNC.
				 ************************************************************/
				p2 = 14;  /* P2 is always 14 for TNC */

				/* P1 select is bits 23:17 */
				if (0 != (p1_select = (dplla >> 17) & 0x7F)) {
					EMGD_DEBUG("p1_select %ld", p1_select);

					/* p1 = 1 << (p1_select -2), so do the reverse to get p1 */
					for (j = 0; p1_select > 1; j++) {
						p1_select >>= 1;
					}

					/* According to the spec, only 7 bit positions are defined
					 * and so we should not have shifted more than 6 times */
					if (j <= 6) {
						p1 = j + 2;
						EMGD_DEBUG("p1 %ld", p1);
					} else {
						EMGD_ERROR_EXIT("Invalid P1 select value");
						return -IGD_ERROR_INVAL;
					}
				} else {
					EMGD_ERROR_EXIT("Invalid P1 select value (zero)");
					return -IGD_ERROR_INVAL;
				}


				/* Equation that calculates the dot clk:
				 * -------------------------------------
				 * dot clock = ref_freq * M / (P1 * P2)
				 */
				ref_freq = 200000000;  /* 200 MHz */

				dclk = ref_freq * m / (p1*p2);

				/*  FIXME:  This is a workaround to get dclk.  We are supposed
				 *  to be calculating this based on the formula, but DPLL
				 *  is somehow locked and does not return the programmed
				 *  p1 value.  Once this is fixed, we no longer need to have
				 *  igd_display_handle in the parameter of get_pipe_info
				 *
				 *  Update:
				 *  Now that we initialize the driver before X starts,
				 *  we want to do a seamless mode-set from firmware to
				 *  our kernel mode driver. At this point we do not have
				 *  the igd_display_context_t setup.
				 *
				 *	if (NULL != display) {
				 *		igd_display_context_t *display_context =
				 *		(igd_display_context_t *) display;
				 *
				 *		dclk = ref_freq * m / (p1 * p2);
				 *
				 *		PIPE(display)->dclk is in KHhz
				 *		dclk = PIPE(display)->dclk * 1000;
				 *	} else {
				 *		dclk = 0;
				 *	}
				 *
				 *	if( dclk == 0 ) {
				 *		EMGD_ERROR_EXIT(" Dot Clock/Ref Frequency is Zero!!!");
				 *		return -IGD_ERROR_INVAL;
				 *	}
				 */

				EMGD_DEBUG("Ref frequency = %lu", ref_freq);
				EMGD_DEBUG("Pipe A constructed Dot clock is = %lu", dclk);
				timing[0].dclk = dclk/1000; /* Make it to KHz */
				EMGD_DEBUG("Pipe A Dot clock in KHz = %lu", timing[0].dclk);

				timing[0].refresh = (unsigned short)(dclk/
					((timing[0].htotal)*(timing[0].vtotal)));

				EMGD_DEBUG("Pipe A refresh = %u", timing[0].refresh);
			} /* if  DPLL A active */
		} /* dot clock code block */
	} /* if Pipe A active */

	/* For 2nd display pipe, pipe b registers in both 0:2:0 and 0:3:0
	 * supposed to be programmed to same values. So these values can be
	 * read from either devices */
	pipe_conf = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_CONF);

	if(pipe_conf & BIT(31)) { /* pipe B is active */
		timing = &mode_context->fw_info->timing_arr[0];

		reg = READ_MMIO_REG_TNC(IGD_PORT_SDVO, HTOTAL_B);
		timing[1].htotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[1].width = (unsigned short)reg & 0xFFF;

		reg = READ_MMIO_REG_TNC(IGD_PORT_SDVO, HBLANK_B);
		timing[1].hblank_start = (unsigned short)reg & 0x1FFF;
		timing[1].hblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = READ_MMIO_REG_TNC(IGD_PORT_SDVO, HSYNC_B);
		timing[1].hsync_start = (unsigned short)reg & 0x1FFF;
		timing[1].hsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = READ_MMIO_REG_TNC(IGD_PORT_SDVO, VTOTAL_B);
		timing[1].vtotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[1].height = (unsigned short)reg & 0xFFF;

		reg = READ_MMIO_REG_TNC(IGD_PORT_SDVO, VBLANK_B);
		timing[1].vblank_start = (unsigned short)reg & 0x1FFF;
		timing[1].vblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		READ_MMIO_REG_TNC(IGD_PORT_SDVO, VSYNC_B);
		timing[1].vsync_start = (unsigned short)reg & 0x1FFF;
		timing[1].vsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		/* Following are not offset by 1 in ptinfo */
		timing[1].width++;
		timing[1].height++;

		{
			/* Calculate the firmware programmed dot clock */
			unsigned long dpllb, fpb0, fpb1;
			unsigned long mb1, mb2, nb, pb1, pb2, pllb_select;
			unsigned long ref_freq = 0, dclk;
			unsigned long port_mult = 1;
			unsigned long temp; /* To store intermediate values b4 dclk */
			int j;

			dpllb = READ_MMIO_REG_TNC(IGD_PORT_SDVO, DPLLBCNTR);
			fpb0  = READ_MMIO_REG_TNC(IGD_PORT_SDVO, FPB0);
			fpb1  = READ_MMIO_REG_TNC(IGD_PORT_SDVO, FPB1);

			if(dpllb & BIT(31)) {

				mb1 = (fpb0 >> 8) & 0x3F;    /* M1 is bits 13:8 */
				mb2 = (fpb0) & 0xFF;         /* M1 is bits 7:0 */
				nb = (fpb0 >> 16) & 0xFF;    /* N is bits 23:16 */
				pb1 = (dpllb >> 16) & 0xFF; /* P1 is bits 23:16 */

				port_mult = (((dpllb >> 4) & 0x0F) + 1);
				/* Check for illegal values of P1
				 * The bit representation MUST be power of 2
				 * All other values are illegal including zero.
				 */
				if( (pb1 == 0) ||
					( (pb1 & (pb1-1)) != 0 ) ) {
					EMGD_ERROR_EXIT("Invalid P1 bits set");
					return -IGD_ERROR_INVAL;
				}

				for(j = 0; j < 8; j++) {
					if(pb1 & BIT(j)) {  /* P1 is divide by 1 to 8 */
						pb1 = j+1;
						break;					}
				}

				for(j = 0; j < 8; j++) {
					if(nb & BIT(j)) {
						nb = j+1;
						break;
					}
				}

				pb2 = (dpllb >> 24) & 0x3; /* P2 is bits 25:24 */

				/* Since Pipe B does not support internal LVDS, we just
				 * follow the normal divisor values
				 */
				if(pb2 == 0) {
					pb2 = 10;
				} else if(pb2 == 1) {
					pb2 = 5;
				} else {
					EMGD_ERROR("Invalid P2 bits set = 0x%lx", pb2);
				}

				pllb_select = (dpllb >> 13) & 0x3; /* PLL Ref I/P Select */

				/* Equation that calculates the dot clk
				 * -------------------------------------
                 *
				 * pll_freq_factor = ((float)(5 * (mb1+2)+(mb2+2))/(nb+2))/
				 *	((pb1*pb2));
				 *
				 * fdclk = pll_freq_factor * ref_freq * 1000000;
				 *
				 * Support for FPU in Kernel Code is not straightforward
				 * we will just stick to int operations. We will just re-
                 * arrange the factors.
				 */
				if(pllb_select == 0) {
					ref_freq = 96; /* 96MHz */

				} else if( (pllb_select == 1) || (pllb_select == 3) ) {

					EMGD_ERROR_EXIT("PLL Reference Input Select Reserved");
					return -IGD_ERROR_INVAL;

				} else if(pllb_select  == 2) {
					EMGD_ERROR_EXIT("PLL ref is SDVO TV CLK");
					/* TODO: How to handle this value? */
					return -IGD_ERROR_INVAL;
				}

				/* First let's multiply by 1000 * 1000
				 * so that we don't end up in zero during
				 * integer division
				 */
				temp = 1000 * 1000;
				temp = temp * (mb2+2);
                temp = temp / nb;
				temp = temp/(pb1*pb2);
                dclk = temp * ref_freq;
                dclk = dclk / port_mult;

				if( (dclk == 0) || (ref_freq == 0) ) {
					EMGD_ERROR_EXIT("Dot Clock/Ref Frequency is Zero!!!");
					return -IGD_ERROR_INVAL;
				}

				EMGD_DEBUG("Ref frequency = %lu", ref_freq);
				EMGD_DEBUG("Pipe B constructed Dot clock is = %lu", dclk);
				timing[1].dclk = dclk/1000; /* Make it to KHz */
				EMGD_DEBUG("Pipe B Dot clock in KHz = %lu", timing[1].dclk);

				timing[1].refresh = (unsigned short) (dclk/
					((timing[1].htotal+1)*(timing[1].vtotal+1)));

				EMGD_DEBUG("Pipe B refresh = %u", timing[1].refresh);
			} /* if  DPLL B  active */
		} /* dot clock code block */
	} /* if Pipe B is active */
	EMGD_TRACE_EXIT;
	return 0;
}



/*!
 *
 * @param void
 *
 * @return 0
 */
static int get_port_info_tnc(void)
{
	/* TODO: Any port related info that needs to be populated ? */
	EMGD_TRACE_ENTER;

	EMGD_TRACE_EXIT;
	return 0;

}


/*******************************************************************************
 *
 * The following code provides VBlank interrupt support both within and without
 * the EMGD HAL.
 *
 ******************************************************************************/

/*!
 * Interrupt handler for VBlanks.
 *
 * @param irq (IN).  Unused.
 *
 * @param mmio (IN).  A pointer to the start of memory-mapped IO, to access
 * registers.
 *
 * @return A value to tell the kernel whether or not we handled the interrupt.
 */
static irqreturn_t interrupt_handler_tnc(int irq, void* mmio)
{
	unsigned long iir, iir_dev3;
	unsigned long lock_flags;
	unsigned long tmp;
	unsigned long port2_interrupt = 0;
	unsigned long port4_interrupt = 0;
	emgd_vblank_callback_t *cb;

	EMGD_TRACE_ENTER;

	iir = EMGD_READ32(EMGD_MMIO(mmio) + IIR);
	iir_dev3 = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IIR);


	/* Detect whether a vblank interrupt occured, and if so, what type of
	 * processing is needed (do the simple processing now):
	 */
	spin_lock_irqsave(&vblank_lock_tnc, lock_flags);
	if ((port2_interrupt = iir_dev3 & BIT5 /* Port 2/Pipe A/SDVO-B */) != 0) {
		if ((tmp = vblank_interrupt_state & VBLANK_INT4_PORT2) != 0) {
			/* Record "answers" for all requestors: */
			vblank_interrupt_state |= VBINT_ANSWER4_REQUEST(tmp);
		}
	}
	if ((port4_interrupt = iir & BIT7 /* Port 4/Pipe B/Int-LVDS */) != 0) {
		if ((tmp = vblank_interrupt_state & VBLANK_INT4_PORT4) != 0) {
			/* Record "answers" for all requestors: */
			vblank_interrupt_state |= VBINT_ANSWER4_REQUEST(tmp);
		}
	}
	spin_unlock_irqrestore(&vblank_lock_tnc, lock_flags);

	/*
	 * Call the KMS 'flip complete' handler if we're waiting for a flip to
	 * complete on this port.  Note that we should do this before the
	 * drm_handle_vblank() calls below since we need to clear the
	 * 'flip pending' bit in the CRTC before the vblank waitqueue gets
	 * woken up.
	 */
	if (port4_interrupt) {
		crtc_pageflip_handler(mode_context->context->drm_dev,
			IGD_PORT_TYPE_LVDS);

		if (mode_context->batch_blits[IGD_PORT_TYPE_LVDS - 1]) {
			notify_userspace_vblank(mode_context->context->drm_dev,
				IGD_PORT_TYPE_LVDS);
		}
	} else if (port2_interrupt) {
		crtc_pageflip_handler(mode_context->context->drm_dev,
			IGD_PORT_TYPE_SDVOB);

		if (mode_context->batch_blits[IGD_PORT_TYPE_SDVOB - 1]) {
			notify_userspace_vblank(mode_context->context->drm_dev,
				IGD_PORT_TYPE_SDVOB);
		}
	}


	/* Notify KMS Hander:  The assignment of CRTC=0 for PIPE A and
	 * CRTC=1 for PIPE B is not correct if somehow PIPE A is disabled.
	 */
	if (port2_interrupt) {
		drm_handle_vblank(mode_context->context->drm_dev, 1);
	}

	if (port4_interrupt) {
		drm_handle_vblank(mode_context->context->drm_dev ,0);
	}

	/* Call any registered/enabled callbacks for this interrupt: */
	cb = &interrupt_callbacks_tnc[2];
	if (port2_interrupt && cb->callback &&
		(vblank_interrupt_state & VBINT_ANSWER(VBINT_CB, VBINT_PORT2))) {
		/* Clear the state to indicate the vblank has occured prior to
		 * invoking the callback.
		 */
		vblank_interrupt_state &= ~VBINT_ANSWER(VBINT_CB, VBINT_PORT2);
		cb->callback(cb->priv);
	}
	cb = &interrupt_callbacks_tnc[4];
	if (port4_interrupt && cb->callback &&
		(vblank_interrupt_state & VBINT_ANSWER(VBINT_CB, VBINT_PORT4))) {
		/* Clear the state to indicate the vblank has occured prior to
		 * invoking the callback.
		 */
		vblank_interrupt_state &= ~VBINT_ANSWER(VBINT_CB, VBINT_PORT4);
		cb->callback(cb->priv);
	}

	/* Clear interrupt status registers: */
	if (port2_interrupt || port4_interrupt) {
		/* Clear the corresponding bits in the PIPE{A|B}_STAT register(s): */
		if (port2_interrupt) {
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS), EMGD_MMIO(mmio) + PIPEB_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);

			tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);
			/* Clear bits that are written by a 1, so we don't clear them: */
			tmp = tmp & (~PIPESTAT_STS_BITS);
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT,
					(tmp | VBLANK_STS));
			READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);
			tmp = READ_MMIO_REG_TNC(IGD_PORT_LVDS, PIPEB_STAT);
		}
		if (port4_interrupt) {
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS), EMGD_MMIO(mmio) + PIPEA_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
		}

		/* Clear the corresponding bits in the IIR register: */
		EMGD_WRITE32((port2_interrupt | port4_interrupt), EMGD_MMIO(mmio)+IIR);
		WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IIR, port2_interrupt);


		EMGD_DEBUG("EXIT--IRQ_HANDLED");
		return IRQ_HANDLED;
	} else {
		EMGD_DEBUG("EXIT--IRQ_NONE");
		return IRQ_NONE;
	}
}

/*!
 * Implementation of "protected" function (i.e. for use within the mode
 * module) to request VBlank interrupts for a particular purpose & port.
 * Based upon the outstanding requests, this function decides whether to
 * touch registers, register the interrupt handler, etc.
 *
 * @param request_for (IN).  A bit that identifies a who and what (e.g.
 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
 *
 * @param mmio (IN).  A pointer to the start of memory-mapped IO, to access
 * registers.
 *
 * @return Zero for success, non-zero for failure.
 */
int request_vblanks_tnc(unsigned long request_for, unsigned char *mmio)
{
	unsigned long lock_flags;
	unsigned long tmp;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameters: request_for=0x%lx, mmio=0x%p", request_for, mmio);

	/* Perform error checking--ensure a valid bit was set: */
	if (!(request_for & (VBLANK_INT4_PORT2 | VBLANK_INT4_PORT4))) {
		EMGD_ERROR_EXIT("Invalid parameter, request_for=0x%lx",
			request_for);
		return -1;
	}

	/* If we're just enabling interrupts, register the interrupt handler: */
	if (!VBLANK_INTERRUPTS_ENABLED) {
		struct drm_device *drm_device = mode_context->context->drm_dev;

		EMGD_DEBUG("Registering interrupt_handler_tnc()");
		if (request_irq(drm_device->pdev->irq, interrupt_handler_tnc,
			IRQF_SHARED, EMGD_DRIVER_NAME, mmio)) {
			EMGD_ERROR_EXIT("Failed to register interrupt_handler_tnc()");
			return -1;
		} else {
			EMGD_DEBUG("Successfully registered interrupt_handler_tnc()");
		}
	}

	/* Lock here to stop the interrupt handler until after changing bits: */
	spin_lock_irqsave(&vblank_lock_tnc, lock_flags);

	/* Enable interrupts for the requested purpose/port, actually touching the
	 * hardware registers if newly enabling interrupts for the given port/pipe:
	 */
	if (request_for & VBLANK_INT4_PORT2) {
		if (!VBLANK_INTERRUPTS_ENABLED4_PORT2) {
			/* 1. Change Pipe Display Status Register for this pipe: set the
			 *    Vertical Blank Interrupt Enable bit & clear (by setting) the
			 *    Vertical Blank Interrupt Status bit:
			 */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);
			/* Clear bits that are written by a 1, so we don't clear them: */
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS_EN | VBLANK_STS),
				EMGD_MMIO(mmio) + PIPEB_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);

			/* 2. Just in case, clear (by setting) the Interrupt Identity
			 *    Register bit for this pipe:
			 */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IIR);
			EMGD_WRITE32((tmp | BIT5), EMGD_MMIO(mmio) + IIR);

			/* 3. Clear the Interrupt Mask Register bit for this pipe: */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IMR);
			EMGD_WRITE32((tmp & (~BIT5)), EMGD_MMIO(mmio) + IMR);

			/* 4. Set the Interrupt Enable Register bit for this pipe: */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IER);
			EMGD_WRITE32((tmp | BIT5), EMGD_MMIO(mmio) + IER);

			/* NOW LET'S PROGRAM DEVICE 3 */

			tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);
			/* Clear bits that are written by a 1, so we don't clear them: */
			tmp = tmp & (~PIPESTAT_STS_BITS);
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT,
					(tmp | VBLANK_STS_EN | VBLANK_STS));
			READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);

			/* 2. Just in case, clear (by setting) the Interrupt Identity
			 *    Register bit for this pipe:
			 */
			tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IIR);
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IIR, (tmp | BIT5));

			/* 3. Clear the Interrupt Mask Register bit for this pipe: */
			tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IMR);
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IMR, (tmp & (~BIT5)));

			/* 4. Set the Interrupt Enable Register bit for this pipe: */
			tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IER);
			WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IER, (tmp | BIT5));

		}
		vblank_interrupt_state |= request_for;
		/* Since there is only  one vblank interrupt per request for now,
		   add counter have been remove for now and disable it when end request.
		   The counter may required after there is more than one request in future.
		*/
		vblank_interrupt_ref_cnt_port2 = 1;
	} else /* if (request_for & VBLANK_INT4_PORT4) */ {
		if (!VBLANK_INTERRUPTS_ENABLED4_PORT4) {
			/* 1. Change Pipe Display Status Register for this pipe: set the
			 *    Vertical Blank Interrupt Enable bit & clear (by setting) the
			 *    Vertical Blank Interrupt Status bit:
			 */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
			/* Clear bits that are written by a 1, so we don't clear them: */
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS_EN | VBLANK_STS),
				EMGD_MMIO(mmio) + PIPEA_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);

			/* 2. Just in case, clear (by setting) the Interrupt Identity
			 *    Register bit for this pipe:
			 */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IIR);
			EMGD_WRITE32((tmp | BIT7), EMGD_MMIO(mmio) + IIR);

			/* 3. Clear the Interrupt Mask Register bit for this pipe: */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IMR);
			EMGD_WRITE32((tmp & (~BIT7)), EMGD_MMIO(mmio) + IMR);

			/* 4. Set the Interrupt Enable Register bit for this pipe: */
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + IER);
			EMGD_WRITE32((tmp | BIT7), EMGD_MMIO(mmio) + IER);
		}
		vblank_interrupt_state |= request_for;
		vblank_interrupt_ref_cnt_port4 = 1;
	}


	/* Unlock to allow the interrupt handler to proceed: */
	spin_unlock_irqrestore(&vblank_lock_tnc, lock_flags);

	EMGD_TRACE_EXIT;
	return 0;
}



/*!
 * Implementation of "protected" function (i.e. for use within the mode
 * module) to end a previous request VBlank interrupts for a particular
 * purpose & port.  Based upon the outstanding requests, this function
 * decides whether to touch registers, unregister the interrupt handler,
 * etc.
 *
 * @param request_for (IN).  A bit that identifies a who and what (e.g.
 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
 *
 * @param mmio (IN).  A pointer to the start of memory-mapped IO, to access
 * registers.
 *
 * @return Zero for success, non-zero for failure.
 */
int end_request_tnc(unsigned long request_for, unsigned char *mmio)
{
	unsigned long lock_flags;
	unsigned long tmp;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameters: request_for=0x%lx, mmio=0x%p", request_for, mmio);

	/* Perform error checking--ensure a valid bit was set: */
	if (!(request_for & (VBLANK_INT4_PORT2 | VBLANK_INT4_PORT4)) ||
		!(vblank_interrupt_state & request_for)) {
		EMGD_ERROR_EXIT("Invalid parameter, request_for=0x%lx", request_for);
		return -1;
	}

	/* Lock here to stop the interrupt handler until after changing bits: */
	spin_lock_irqsave(&vblank_lock_tnc, lock_flags);

	/* Disable interrupts for the requested purpose/port, actually touching the
	 * hardware registers no software wants interrupts for the given port/pipe:
	 */
	if (request_for & VBLANK_INT4_PORT2) {
		/* Decrement reference count */
		vblank_interrupt_ref_cnt_port2 = 0;
		if (0 > vblank_interrupt_ref_cnt_port2) {
			EMGD_DEBUG("WARNING:  Disabled vblank INT too many times.");
			vblank_interrupt_ref_cnt_port2 = 0;
		}

		if (0 == vblank_interrupt_ref_cnt_port2) {
			/* Turn off both the request and the answer bits: */
			tmp = request_for & VBLANK_INT4_PORT2;
			vblank_interrupt_state &= ~(tmp | VBINT_ANSWER4_REQUEST(tmp));
			if (!VBLANK_INTERRUPTS_ENABLED4_PORT2) {
				/* 1. Change Pipe Display Status Register for this pipe: clear
				 *    the Vertical Blank Interrupt Enable bit & clear (by
				 *    setting) the Vertical Blank Interrupt Status bit:
				 */
				tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);
				/* Clear bits that are written by a 1, so don't clear them: */
				tmp = tmp & (~PIPESTAT_STS_BITS);
				EMGD_WRITE32(((tmp & (~VBLANK_STS_EN)) | VBLANK_STS),
					EMGD_MMIO(mmio) + PIPEB_STAT);
				EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);

				/* 2. Clear the Interrupt Enable Register bit for this pipe: */
				tmp = EMGD_READ32(EMGD_MMIO(mmio) + IER);
				EMGD_WRITE32((tmp & (~BIT5)), EMGD_MMIO(mmio) + IER);

				/* 3. Set the Interrupt Mask Register bit for this pipe: */
				tmp = EMGD_READ32(EMGD_MMIO(mmio) + IMR);
				EMGD_WRITE32((tmp | BIT5), EMGD_MMIO(mmio) + IMR);

				/* 4. Just in case, clear (by setting) the Interrupt Identity
				*    Register bit for this pipe:
				*/
				tmp = EMGD_READ32(EMGD_MMIO(mmio) + IIR);
				EMGD_WRITE32((tmp | BIT5), EMGD_MMIO(mmio) + IIR);

				/* NOW LET'S PROGRAM DEVICE 3 */

				tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);
					/* Clear bits that are written by a 1, so we don't clear them: */
				tmp = tmp & (~PIPESTAT_STS_BITS);
				WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT,
						((tmp & (~VBLANK_STS_EN)) | VBLANK_STS));
				READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_STAT);

				/* 2. Clear the Interrupt Enable Register bit for this pipe: */
				tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IER);
				WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IER, (tmp & (~BIT5)));

				/* 3. Set the Interrupt Mask Register bit for this pipe: */
				tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IMR);
				WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IMR, (tmp | BIT5));

				/* 4. Just in case, clear (by setting) the Interrupt Identity
				 *    Register bit for this pipe:
				 */
				tmp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, IIR);
				WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, IIR, (tmp | BIT5));

			}
		}
	}

	if (request_for & VBLANK_INT4_PORT4) {
		/* Decrement reference count */
		vblank_interrupt_ref_cnt_port4 = 0;
		if (0 > vblank_interrupt_ref_cnt_port4) {
			EMGD_DEBUG("WARNING:  Disabled vblank INT too many times.");
			vblank_interrupt_ref_cnt_port4 = 0;
		}

		if (0 == vblank_interrupt_ref_cnt_port4) {
			/* Turn off both the request and the answer bits: */
			tmp = request_for & VBLANK_INT4_PORT4;
			vblank_interrupt_state &= ~(tmp | VBINT_ANSWER4_REQUEST(tmp));
			if (!VBLANK_INTERRUPTS_ENABLED4_PORT4) {
				/* 1. Change Pipe Display Status Register for this pipe: clear
				 *    the Vertical Blank Interrupt Enable bit & clear (by
				 *    setting the Vertical Blank Interrupt Status bit:
				 */
				 tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
				 /* Clear bits that are written by a 1, so don't clear them: */
				 tmp = tmp & (~PIPESTAT_STS_BITS);
				 EMGD_WRITE32(((tmp & (~VBLANK_STS_EN)) | VBLANK_STS),
					 EMGD_MMIO(mmio) + PIPEA_STAT);
				 EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);

				 /* 2. Clear the Interrupt Enable Register bit for this pipe: */
				 tmp = EMGD_READ32(EMGD_MMIO(mmio) + IER);
				 EMGD_WRITE32((tmp & (~BIT7)), EMGD_MMIO(mmio) + IER);

				 /* 3. Set the Interrupt Mask Register bit for this pipe: */
				 tmp = EMGD_READ32(EMGD_MMIO(mmio) + IMR);
				 EMGD_WRITE32((tmp | BIT7), EMGD_MMIO(mmio) + IMR);

				 /* 4. Just in case, clear (by setting) the Interrupt Identity
				 *    Register bit for this pipe:
				 */
				 tmp = EMGD_READ32(EMGD_MMIO(mmio) + IIR);
				 EMGD_WRITE32((tmp | BIT7), EMGD_MMIO(mmio) + IIR);
			}
		}
	}

	/* Unlock to allow the interrupt handler to proceed: */
	spin_unlock_irqrestore(&vblank_lock_tnc, lock_flags);

	/* If we've completely disabled all causes for interrupts, unregister the
	 * interrupt handler:
	 */
	if (!VBLANK_INTERRUPTS_ENABLED) {
		struct drm_device *drm_device = mode_context->context->drm_dev;

		EMGD_DEBUG("Unregistering interrupt_handler_tnc()");
		free_irq(drm_device->pdev->irq, mmio);
		EMGD_DEBUG("Successfully unregistered interrupt_handler_tnc()");
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * Implementation of "protected" function (i.e. for use within the mode
 * module) to check whether a requested VBlank interrupt occured.
 *
 * @param request_for (IN).  A bit that identifies a who and what (e.g.
 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
 *
 * @return Non-zero if the requested VBlank occured, zero if not.
 */
int vblank_occured_tnc(unsigned long request_for)
{
	return (vblank_interrupt_state & VBINT_ANSWER4_REQUEST(request_for));
}

/*!
 *  Registers a VBlank interrupt callback function (and its parameter) to
 *  call when a VBlank interrupt occurs for a given port.
 *
 * @param callback (IN).  A callback (function pointer) to a non-HAL
 * function that processes a VBlank interrupt.
 *
 * @param priv (IN).  An opaque pointer to a non-HAL data structure.
 *  This pointer is passed as a parameter of the callback function.
 *
 * @param port_number (IN).  The EMGD port number to register a VBlank
 *  interrupt callback for.
 *
 * @return A handle that uniquely identifies this callback/port
 *  combination, or NULL if a failure.
 */
emgd_vblank_callback_h register_vblank_callback_tnc(
	emgd_process_vblank_interrupt_t callback,
	void *priv,
	unsigned long port_number)
{
	EMGD_TRACE_ENTER;

	if (!callback || !priv ||
		!((port_number == 2) || (port_number == 4))) {
		EMGD_ERROR_EXIT("Invalid parameter amongst the following:\n"
			"  process_interrupt=0x%p, priv=0x%p, port_number=%lu",
			callback, priv, port_number);
		return NULL;
	}

	interrupt_callbacks_tnc[port_number].callback = callback;
	interrupt_callbacks_tnc[port_number].priv = priv;

	EMGD_TRACE_EXIT;
	return &interrupt_callbacks_tnc[port_number];
}

/*!
 *  Unregisters a previously-registered VBlank interrupt callback function
 *  for a given port.
 *
 * @param callback_h (IN).  The handle that uniquely identifies the VBlank
 *  interrupt callback to unregister.
 */
void unregister_vblank_callback_tnc(emgd_vblank_callback_h callback_h)
{
	emgd_vblank_callback_t *cb = (emgd_vblank_callback_t *) callback_h;

	EMGD_TRACE_ENTER;

	cb->callback = NULL;
	cb->priv = NULL;

	EMGD_TRACE_EXIT;
}

/*!
 *  Enable delivering VBlank interrupts to the callback function for the
 *  registered callback/port combination.
 *
 * @param callback_h (IN).  The handle that uniquely identifies which
 *  VBlank interrupt callback/port combination to enable.
 *
 * @return Zero if successful, non-zero if a failure.
 */
int enable_vblank_callback_tnc(emgd_vblank_callback_h callback_h)
{
	emgd_vblank_callback_t *cb = (emgd_vblank_callback_t *) callback_h;
	unsigned long enable_for = VBINT_REQUEST(VBINT_CB,
		(cb->port_number == 2) ? VBINT_PORT2 : VBINT_PORT4);
	unsigned char* mmio =
		EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	int ret;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameter: callback_h=0x%p", callback_h);

	ret = request_vblanks_tnc(enable_for, mmio);

	EMGD_DEBUG("Return %d", ret);
	EMGD_TRACE_EXIT;
	return ret;
}

/*!
 *  Disable delivering VBlank interrupts to the callback function for the
 *  registered function/port combination.
 *
 * @param callback_h (IN).  The handle that uniquely identifies which
 *  VBlank interrupt callback/port combination to disable.
 */
void disable_vblank_callback_tnc(emgd_vblank_callback_h callback_h)
{
	unsigned long lock_flags;
	unsigned long enable_for;
	unsigned char* mmio =
		EMGD_MMIO(mode_context->context->device_context.virt_mmadr);

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameter: callback_h=0x%p", callback_h);

	if (callback_h == ALL_PORT_CALLBACKS) {
		/* Need to do some push-ups in order to get interrupts disabled: */
		spin_lock_irqsave(&vblank_lock_tnc, lock_flags);
		enable_for = (VBLANK_INT4_PORT2 | VBLANK_INT4_PORT4);
		if (!VBLANK_INTERRUPTS_ENABLED) {
			/* Nothing has enabled interrupts, so there's no interrupt handler
			 * to unregister.  Therefore, use a special value to prevent that:
			 */
			vblank_interrupt_state = (VBLANK_INT4_PORT2 |
				VBLANK_INT4_PORT4 | VBLANK_DISABLE_HW_ONLY);
		} else {
			vblank_interrupt_state = (VBLANK_INT4_PORT2 |
				VBLANK_INT4_PORT4);
		}
		spin_unlock_irqrestore(&vblank_lock_tnc, lock_flags);

		end_request_tnc(enable_for, mmio);

		spin_lock_irqsave(&vblank_lock_tnc, lock_flags);
		vblank_interrupt_state = 0;
		spin_unlock_irqrestore(&vblank_lock_tnc, lock_flags);
	} else {
		emgd_vblank_callback_t *cb =
			(emgd_vblank_callback_t *) callback_h;
		enable_for = VBINT_REQUEST(VBINT_CB,
			(cb->port_number == 2) ? VBINT_PORT2 : VBINT_PORT4);

		end_request_tnc(enable_for, mmio);
	}

	EMGD_TRACE_EXIT;
}
/*!
 *  checks if the port is enabled
 */
unsigned long get_port_control_tnc(unsigned long port_num, unsigned long port_reg)
{
	unsigned long port_value=0;

	EMGD_TRACE_ENTER;
	if( port_num == IGD_PORT_TYPE_SDVOB) {
		port_value = READ_MMIO_REG_TNC(IGD_PORT_SDVO,port_reg);
	} else {
		port_value = READ_MMIO_REG_TNC(IGD_PORT_LVDS,port_reg);
	}

	EMGD_TRACE_EXIT;
	return port_value;
}


/*!
 * locks the plane 
 * set_surface() and set_display_base_tnc() will not update the registers
 * when the planes are locked.
 */
void lock_planes(igd_display_h display_handle)
{
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	
	EMGD_TRACE_ENTER;
	if(!display){
		EMGD_ERROR_EXIT("Display is NULL");
		return;
	}
	PLANE(display)->fb_info->lock = TRUE;
	EMGD_TRACE_EXIT;
}


/*!
 * unlocks the plane 
 * resets the lock flag and writes to plane registers
 * 
 */
int unlock_planes(igd_display_h display_handle, unsigned int scrn_num)
{
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	unsigned long plane_reg;
	unsigned long plane_control;
	unsigned long visible_offset;

	EMGD_TRACE_ENTER;
	if(!display){
		EMGD_ERROR_EXIT("Display is NULL");
		return FALSE;
	}

	/* plane registers are always on 0:2:0, so no need to use _TNC macros */
	plane_reg = PLANE(display)->plane_reg;
	plane_control = EMGD_READ32(MMIO(display) + plane_reg);

	/*
	 *   Write the current plane_control value to the plane_reg
	 *   Write the surface stride to DSP_STRIDE_OFFSET
	 *   Write the visible from start of plane to DSP_LINEAR_OFFSET
	 *   Write the base surface offset to either:
	 */
	PLANE(display)->fb_info->lock = FALSE;

	if(scrn_num == PRIMARY_DISPLAY){
		visible_offset = 0;
	}else {
		visible_offset = PLANE(display)->fb_info->visible_offset;
	}

	EMGD_DEBUG("visible offset= %lx", visible_offset);

	EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
	EMGD_WRITE32(PLANE(display)->fb_info->screen_pitch,
			MMIO(display) + plane_reg + DSP_STRIDE_OFFSET);
	EMGD_WRITE32(visible_offset,
			MMIO(display) + plane_reg + DSP_LINEAR_OFFSET);
	EMGD_WRITE32(PLANE(display)->fb_info->fb_base_offset,
			MMIO(display) + plane_reg + DSP_START_OFFSET);
	EMGD_TRACE_EXIT;
	return TRUE;
}


mode_full_dispatch_t mode_full_dispatch_tnc = {
	igd_alter_cursor_pos_tnc,
	igd_set_palette_entries_tnc,
	igd_wait_vsync_tnc,
	igd_query_in_vblank_tnc,
	igd_get_scanline_tnc,
	set_display_base_tnc,
	program_cursor_tnc,
	set_color_correct_tnc,
	igd_get_surface_tnc,
	igd_set_surface_tnc,
	igd_query_event_tnc,
	set_flip_pending_tnc,
	check_flip_pending_tnc,
	get_plane_info_tnc,
	get_pipe_info_tnc,
	get_port_info_tnc,
	register_vblank_callback_tnc,
	unregister_vblank_callback_tnc,
	enable_vblank_callback_tnc,
	disable_vblank_callback_tnc,
	request_vblanks_tnc,
	end_request_tnc,
	vblank_occured_tnc,
	get_port_control_tnc,
	lock_planes,
	unlock_planes,
};


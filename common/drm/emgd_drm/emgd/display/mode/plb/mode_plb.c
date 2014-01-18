/*
 *-----------------------------------------------------------------------------
 * Filename: mode_plb.c
 * $Revision: 1.34 $
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
 *  Almador Core implementations for the mode dispatch functions.
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

#include <plb/regs.h>
#include <plb/context.h>
#include <plb/mi.h>
#include <plb/cmd.h>

#include "drm_emgd_private.h"
#include "../cmn/match.h"
#include "../cmn/mode_dispatch.h"

#include "emgd_shared.h"
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <drm/drmP.h>


/* Registered VBlank interrupt callbacks (one-per-pipe): */
static emgd_vblank_callback_t interrupt_callbacks_plb[IGD_MAX_PORTS] =
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
DEFINE_SPINLOCK(vblank_lock_plb);



int set_flip_pending_plb(unsigned char *mmio, unsigned long pipe_status_reg);
int check_flip_pending_plb(unsigned char *mmio, unsigned long pipe_status_reg);

unsigned long get_port_control_plb(unsigned long port_num, unsigned long port_reg);
/*!
 * @addtogroup display_group
 * @{
 */
int wait_for_vblank_plb(unsigned char *mmio, unsigned long pipe_reg);

void notify_userspace_vblank(struct drm_device *dev, int port);

/*!
 *
 * @param display
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL if color attributes not found
 */
static int set_color_correct_plb(igd_display_context_t *display)
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
		EMGD_ERROR("Color Correction Atrributes not found!");
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
		EMGD_WRITE32(palette[i], MMIO(display) +
			PIPE(display)->palette_reg + i*4);
	}
	OS_FREE(palette);

	return 0;
}

/*!
 * Note: When panning in clone mode, the clone must be panned seperatly
 * because it may (probably does) have a different x,y offset.
 *
 * @param display
 * @param fb
 * @param x
 * @param y
 *
 * @return 0
 */
static int set_display_base_plb(igd_display_context_t *display,
		igd_framebuffer_info_t *fb, unsigned long *x, unsigned long *y)
{
	unsigned long base;

	/* FIXME/TODO: Compare the difference between the plb/tnc versions of this
	 * function, as the plb code adds-in the offset of the frame buffer.
	 */
	base = fb->visible_offset;

	base += ((*y * fb->screen_pitch) + (*x * IGD_PF_BYPP(fb->pixel_format)));

	WRITE_MMIO_REG(display, PLANE(display)->plane_reg
		+ DSP_START_OFFSET, base);

#if 0
	/* If this plane is a mirrored plane, then update the other plane offset */
	if (PLANE(display)->mirror) {
		WRITE_MMIO_REG(display, PLANE(display)->mirror->plane_reg
				+ DSP_START_OFFSET, base);
	}
#endif

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
static int igd_alter_cursor_pos_plb(igd_display_h display_handle,
	igd_cursor_info_t *cursor_info)
{
	unsigned long cursor_reg;
	unsigned long new_pos;
	unsigned long cursor_base;

	igd_display_context_t *display = (igd_display_context_t *) display_handle;
	cursor_reg = PIPE(display)->cursor->cursor_reg;

	if (0x27 & READ_MMIO_REG(display, cursor_reg)) {
		/* unlike almador, for plba, u must program the base offset
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
static int igd_set_palette_entries_plb(
	igd_display_h display_handle,
	unsigned long *palette_colors,
	unsigned int start_index,
	unsigned int count)
{
	unsigned int i;

	for(i=start_index; i<start_index+count; i++) {
		EMGD_WRITE32(palette_colors[i],
			MMIO(display_handle) + PIPE(display_handle)->palette_reg + i*4);
	}

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
static int igd_wait_vsync_plb(igd_display_h display_handle)
{
	long i = 0;	/* General counter */
	unsigned long tmp;
	unsigned char *Status_Reg;
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	unsigned char *mmio = MMIO(display);

	Status_Reg = mmio + PIPE(display)->pipe_reg + PIPE_STATUS_OFFSET;

	/* If pipe is off then just return */
	if(!((1<<31) & EMGD_READ32(mmio + PIPE(display)->pipe_reg))) {
		return 1;
	}

	/* 1. Disable VSync interrupt */
	tmp = EMGD_READ32(Status_Reg);
	/* The sticky status bits are cleared by writing a 1, so zero them: */
	tmp = tmp & (~PIPESTAT_STS_BITS);
	EMGD_WRITE32 (tmp & ~(1<<25), Status_Reg);

	/* 2. Clear interrupt status (by writing a 1) */
	tmp = EMGD_READ32(Status_Reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	EMGD_WRITE32 (tmp | (1<<9), Status_Reg);

	/* 3. Enable VSync interrupt */
	tmp = EMGD_READ32(Status_Reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	EMGD_WRITE32 (tmp | (1<<25), Status_Reg);

	/* 4. Wait for VSync */
	while ((i++ < 0x1000000) &&  /* Check for timeout */
			((EMGD_READ32(Status_Reg) & (1<<9)) == 0x00)) {
		;
	}

	/* 5. Disable VSync interrupt */
	tmp = EMGD_READ32(Status_Reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	EMGD_WRITE32 (tmp & ~(1<<25), Status_Reg);

	/* 6. Clear interrupt status (by writing a 1) */
	tmp = EMGD_READ32(Status_Reg);
	tmp = tmp & (~PIPESTAT_STS_BITS);
	EMGD_WRITE32 (tmp | (1<<9), Status_Reg);

	if (i >= 0x10000000)
		return (0);

	return (1);
}  /* igd_wait_vsync*/


/*!
 *
 * @param display_handle
 * @param scanline
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */

static int igd_get_scanline_plb(igd_display_h display_handle, int *scanline)
{
	unsigned int tmp;
	unsigned char *reg;
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	unsigned long fb_height = PLANE(display)->fb_info->height;
	unsigned long dp_height = PIPE(display)->timing->height;

	/* Scanline reg is -8 from control reg */
	reg = MMIO(display) + PIPE(display)->pipe_reg - 0x8;

	tmp = EMGD_READ32(reg);

	if(!(PORT_OWNER(display)->pt_info->flags & IGD_DISPLAY_ENABLE)) {
		return -IGD_ERROR_INVAL;
	}

	tmp = (tmp * fb_height) / dp_height;

	if(tmp >= fb_height) {
		*scanline = IGD_IN_VBLANK;
	} else {
		*scanline = (int)tmp;
	}
	return 0;
} /* end igd_get_scanline() */

/*!
 *
 * @param display_handle
 *
 * @return 1 if TRUE
 * @return 0 if FALSE
 */
static int igd_query_in_vblank_plb(igd_display_h display_handle)
{
	int sl;

	igd_get_scanline_plb(display_handle, &sl);
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
static void program_cursor_plb(igd_display_context_t *display,
	unsigned long status)
{
	unsigned long cursor_reg;
	unsigned long cursor_control = 0x00000000;
	unsigned long cursor_pos;
	unsigned long cursor_base;
	igd_cursor_info_t *cursor_info;
	int i;

	EMGD_DEBUG("Enter program_cursor: %s", status?"ENABLE":"DISABLE");
	EMGD_DEBUG("Device power state: D%ld", GET_DEVICE_POWER_STATE(display));

	if (!(PIPE(display)->cursor)) {
		return;
	}

	cursor_reg = PIPE(display)->cursor->cursor_reg;
	cursor_info = PIPE(display)->cursor->cursor_info;

	/* Turn off cursor before changing anything */
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
		return;
	}

	switch(cursor_info->pixel_format) {
	case IGD_PF_ARGB32:
		if(display->context->dispatch.gmm_virt_to_phys(
				cursor_info->argb_offset, &cursor_base)) {
			EMGD_ERROR("No Phys pointer available for ARGB cursor");
			return;
		}
		break;
	default:
		if(display->context->dispatch.gmm_virt_to_phys(
				cursor_info->xor_offset, &cursor_base)) {
			EMGD_ERROR("No Phys pointer available for XOR cursor");
			return;
		}
		break;
	}

	/* If status is FALSE return with the cursor off */
	if((!status) ||
		(GET_DEVICE_POWER_STATE(display) == IGD_POWERSTATE_D3)) {
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

	WRITE_MMIO_REG(display, cursor_reg,
		cursor_control | (PIPE(display)->pipe_num<<28));
	WRITE_MMIO_REG(display, cursor_reg + CUR_BASE_OFFSET, cursor_base);
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
static int igd_get_surface_plb(igd_display_h display_handle,
	igd_buffertype_t type,
	igd_surface_t *surface,
	igd_appcontext_h appcontext)
{
#if 0
	igd_display_context_t *display = (igd_display_context_t *)display_handle;
	state3d_plb_t *state = STATE3D_PLB(appcontext);

	if(!surface) {
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
		return 0;
	case IGD_BUFFER_COLOR:
		OS_MEMCPY(surface, &state->color_buffer,
			sizeof(igd_surface_t));
		return 0;
	case IGD_BUFFER_DEPTH:
		OS_MEMCPY(surface, &state->depth_buffer,
			sizeof(igd_surface_t));
		return 0;
	default:
		EMGD_ERROR("Invalid type in get_surface");
		break;
	}

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
static int igd_set_surface_plb(igd_display_h display_handle,
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
	unsigned long surface_offset;

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
		if(PLANE(display)->fb_info->visible_offset & 0x3ffff) {
			EMGD_ERROR("FB offset must be 256kb aligned in Poulsbo");
		}

		/* Save new fb_info */
		PLANE(display)->fb_info->visible_offset = surface->offset;
		PLANE(display)->fb_info->screen_pitch = surface->pitch;
		PLANE(display)->fb_info->width = surface->width;
		PLANE(display)->fb_info->height = surface->height;
		PLANE(display)->fb_info->pixel_format = surface->pixel_format;
		PLANE(display)->fb_info->flags = surface->flags;

		/* Get the correct stride and stereo */
		/* TODO - Does Poulsbo flip need to handle stereo mode? */
		/*mode_get_stride_stereo_plb(display, &stride, &stereo, 0);*/

		/* calculate the real offset, taking panning into account */
		surface_offset = surface->offset;
		surface_offset +=
			(PORT_OWNER(display)->pt_info->y_offset * surface->pitch) +
			(PORT_OWNER(display)->pt_info->x_offset *
				IGD_PF_BYPP(surface->pixel_format));
		EMGD_DEBUG("surface_offset = 0x%08lx", surface_offset);

		plane_reg = PLANE(display)->plane_reg;
		plane_control = EMGD_READ32(MMIO(display) + plane_reg);

		/* Perform the flip by doing the following:
		 *
		 *   Write the current plane_control value to the plane_reg
		 *   Write the surface offset to either:
		 *     1) the plane_reg - 4  if async
		 *     2) plane_reg + DSP_START_OFFSET (+4) if not async
		 */
		EMGD_WRITE32(plane_control, MMIO(display) + plane_reg);
		EMGD_WRITE32(surface_offset,
			MMIO(display) + plane_reg + DSP_START_OFFSET);

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

/* Poulsbo does not support a flip pending, since there is no
 * Display Buffer Info instruction.  So this must be done with vBlank.
 * However, a wait_for_vblank can be given while a flip is also in
 * progress, so a semaphore is required when changing flip_pending or
 * when modifying the vBlank interrupt bits.
 *
 * This function should only be called with a flip_mutex around it */
int set_flip_pending_plb(unsigned char *mmio, unsigned long pipe_status_reg)
{
	platform_context_plb_t *plb_context =
		(platform_context_plb_t *)mode_context->context->platform_context;
	unsigned long request_for;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT((pipe_status_reg == PIPEA_STAT) ||
		(pipe_status_reg == PIPEB_STAT), "Invalid pipe_status_reg", 0);

	if (pipe_status_reg == PIPEA_STAT) {
		plb_context->flip_pending |= PLB_FLIP_PIPE_A_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2);
	} else {
		plb_context->flip_pending |= PLB_FLIP_PIPE_B_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4);
	}
	mode_context->dispatch->full->request_vblanks(request_for, mmio);

	EMGD_TRACE_EXIT;
	return 0;
}

/* This function should only be called with a flip_mutex around it */
int check_flip_pending_plb(unsigned char *mmio,
	unsigned long pipe_status_reg)
{
	platform_context_plb_t *plb_context =
		(platform_context_plb_t *)mode_context->context->platform_context;
	unsigned long request_for;
	unsigned int flip_pending;

	EMGD_TRACE_ENTER;
	EMGD_ASSERT((pipe_status_reg == PIPEA_STAT) ||
		(pipe_status_reg == PIPEB_STAT), "Invalid pipe_status_reg", 0);

	if (pipe_status_reg == PIPEA_STAT) {
		flip_pending = plb_context->flip_pending & PLB_FLIP_PIPE_A_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2);
	} else {
		flip_pending = plb_context->flip_pending & PLB_FLIP_PIPE_B_PENDING;
		request_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4);
	}

	if (flip_pending) {
		if (mode_context->dispatch->full->vblank_occured(request_for)) {
			/* VBlank occured, flip complete */
			plb_context->flip_pending &= ~flip_pending;
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
static int igd_query_event_plb(igd_display_h display_handle,
	igd_event_t event, unsigned long *status)
{
	platform_context_plb_t *plb_context =
		(platform_context_plb_t *)mode_context->context->platform_context;
	unsigned char *mmio = MMIO(display_handle);
	unsigned long pipe_status_reg =
		(PLANE(display_handle)->plane_reg == DSPACNTR) ? 0x70024 : 0x71024;
	int ret;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("mmio=0x%p, pipe_status_reg=0x%08lx", mmio, pipe_status_reg);

	switch (event) {
	case IGD_EVENT_FLIP_PENDING:
		ret = OS_PTHREAD_MUTEX_LOCK(&plb_context->flip_mutex);
		*status = check_flip_pending_plb(mmio, pipe_status_reg);
		OS_PTHREAD_MUTEX_UNLOCK(&plb_context->flip_mutex);
		break;
	default:
		return -IGD_ERROR_INVAL;
	}

	EMGD_DEBUG("Returning status=%lu", *status);
	return IGD_SUCCESS;
}

/*!
 * Function gets the width, height and pitch of the framebuffer.
 *
 * @param void
 *
 * @return 0
 */
static int get_plane_info_plb(void)
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
static int get_pipe_info_plb(igd_display_h *display)
{
	unsigned char *mmio = NULL;
	unsigned long pipe_conf = 0;
	igd_display_info_t *timing;
	unsigned long reg = 0;

	EMGD_TRACE_ENTER;

	mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
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
			unsigned long ma1, ma2, na, pa1, pa2, plla_select;
			unsigned long ref_freq = 0, dclk;
			unsigned long temp; /* To store intermediate values b4 dclk */
			int j;

			dplla = EMGD_READ32(mmio + DPLLACNTR);
			fpa0  = EMGD_READ32(mmio + FPA0);
			fpa1  = EMGD_READ32(mmio + FPA1);

			if(dplla & BIT(31)) {

				ma1 = (fpa0 >> 8) & 0x3F;    /* M1 is bits 13:8 */
				ma2 = (fpa0) & 0x1F;         /* M1 is bits 5:0 */
				na = (fpa0 >> 16) & 0x3F;    /* N is bits 21:16 */
				pa1 = (dplla >> 16) & 0xFF; /* P1 is bits 23:16 */

				/* Check for illegal values of P1
				 * The bit representation MUST be power of 2
				 * All other values are illegal including zero.
				 */
				if( (pa1 == 0) ||
					( (pa1 & (pa1-1)) != 0 ) ) {

					EMGD_ERROR("Invalid P1 bits set");
					return -IGD_ERROR_INVAL;
				}
				for(j = 0; j < 8; j++) {
					if(pa1 & BIT(j)) {  /* P1 is divide by 1 to 8 */
						pa1 = j+1;
						break;
					}
				}
				pa2 = (dplla >> 24) & 0x3; /* P2 is bits 25:24 */

				/* The post divisor values are different if the
				 * attached port is internal LVDS. Since Pipe A
				 * does not support internal LVDS, we just follow
				 * the normal divisor values
				 */
				if(pa2 == 0) {
					pa2 = 10;
				} else if(pa2 == 1) {
					pa2 = 5;
				} else {
					EMGD_ERROR("Invalid P2 bits set = 0x%lx", pa2);
				}
				plla_select = (dplla >> 13) & 0x3; /* PLL Ref I/P Select */

				/* Equation that calculates the dot clk
				 * -------------------------------------
                 *
				 * pll_freq_factor = ((float)(5 * (ma1+2)+(ma2+2))/(na+2))/
				 *	((pa1*pa2));
				 *
				 * fdclk = pll_freq_factor * ref_freq * 1000000;
				 *
				 * Support for FPU in Kernel Code is not straightforward
				 * we will just stick to int operations. We will just re-
                 * arrange the factors.
				 */
				if(plla_select == 0) {
					ref_freq = 96; /* 96MHz */
				} else if( (plla_select == 1) || (plla_select == 3) ) {

					EMGD_ERROR("Invalid PLL Reference Input Select Reserved");
					return -IGD_ERROR_INVAL;

				} else if(plla_select  == 2) {
					EMGD_DEBUG("PLL ref is SDVO TV CLK");
					/* TODO: How to handle this value? */
					ref_freq = 0;
				}

				/* First let's multiply by 1000 * 1000
				 * so that we don't get zero during integer
				 * division
				 */
				temp = 1000 * 1000;
				temp = temp * (5 * (ma1+2) + (ma2+2));
				temp = temp/(na+2);
				temp = temp/(pa1*pa2);

				/* dclk  = temp * ref_freq; */

				/*  FIXME:  This is a workaround to get dclk.  We are supposed
				 *  to be calculating this based on the formula, but DPLL
				 *  is somehow locked and does not return the programmed
				 *  p1 value.  Once this is fixed, we no longer need to have
				 *  igd_display_handle in the parameter of get_pipe_info
				 */
				if (NULL != display) {
					/*igd_display_context_t *display_context =
					  (igd_display_context_t *) display;*/

					/* dclk = ref_freq * m / (p1 * p2); */

					/* PIPE(display)->dclk is in KHhz */
					dclk = PIPE(display)->dclk * 1000;

				} else {
					dclk = 0;
				}

				if( (dclk == 0) || (ref_freq == 0) ) {
					EMGD_ERROR(" Dot Clock/Ref Frequency is Zero!!!");
					return -IGD_ERROR_INVAL;
				}
				EMGD_DEBUG("Ref frequency = %lu", ref_freq);
				EMGD_DEBUG("Pipe A constructed Dot clock is = %lu", dclk);
				timing[0].dclk = dclk/1000; /* Make it to KHz */
				EMGD_DEBUG("Pipe A Dot clock in KHz = %lu", timing[0].dclk);

				timing[0].refresh = (unsigned short)(dclk/
					((timing[0].htotal+1)*(timing[0].vtotal+1)));

				EMGD_DEBUG("Pipe A refresh = %u", timing[0].refresh);
			} /* if  DPLL A active */
		} /* dot clock code block */
	} /* if Pipe A active */

	pipe_conf = EMGD_READ32(mmio + PIPEB_CONF);

	if(pipe_conf & BIT(31)) { /* pipe B is active */
		timing = &mode_context->fw_info->timing_arr[0];

		reg = EMGD_READ32(mmio + HTOTAL_B);
		timing[1].htotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[1].width = (unsigned short)reg & 0xFFF;

		reg = EMGD_READ32(mmio + HBLANK_B);
		timing[1].hblank_start = (unsigned short)reg & 0x1FFF;
		timing[1].hblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = EMGD_READ32(mmio + HSYNC_B);
		timing[1].hsync_start = (unsigned short)reg & 0x1FFF;
		timing[1].hsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		reg = EMGD_READ32(mmio + VTOTAL_B);
		timing[1].vtotal = (unsigned short)(reg >> 16) & 0x1FFF;
		timing[1].height = (unsigned short)reg & 0xFFF;

		reg = EMGD_READ32(mmio + VBLANK_B);
		timing[1].vblank_start = (unsigned short)reg & 0x1FFF;
		timing[1].vblank_end = (unsigned short)(reg >> 16) & 0x1FFF;

		EMGD_READ32(mmio + VSYNC_B);
		timing[1].vsync_start = (unsigned short)reg & 0x1FFF;
		timing[1].vsync_end = (unsigned short)(reg >> 16) & 0x1FFF;

		/* Following are not offset by 1 in ptinfo */
		timing[1].width++;
		timing[1].height++;

		EMGD_DEBUG("Pipe B timing width = %d", timing[1].width);
		EMGD_DEBUG("Pipe B timing width = %d", timing[1].height);

		{
			/* Calculate the firmware programmed dot clock */
			unsigned long dpllb, fpb0, fpb1;
			unsigned long mb1, mb2, nb, pb1, pb2, pllb_select;
			unsigned long ref_freq = 0, dclk;
			unsigned long temp; /* To store intermediate values b4 dclk */
			int j;
			unsigned long lvds_port;

			dpllb = EMGD_READ32(mmio + DPLLBCNTR);
			fpb0  = EMGD_READ32(mmio + FPB0);
			fpb1  = EMGD_READ32(mmio + FPB1);
			/* Note: Only Pipe B supports LVDS */
			lvds_port = EMGD_READ32(mmio + LVDSCNTR);

			if(dpllb & BIT(31)) {

				mb1 = (fpb0 >> 8) & 0x3F;    /* M1 is bits 13:8 */
				mb2 = (fpb0) & 0x1F;         /* M1 is bits 5:0 */
				nb = (fpb0 >> 16) & 0x3F;    /* N is bits 21:16 */
				pb1 = (dpllb >> 16) & 0xFF; /* P1 is bits 23:16 */

				/* Check for illegal values of P1
				 * The bit representation MUST be power of 2
				 * All other values are illegal including zero.
				 */
				if( (pb1 == 0) ||
					( (pb1 & (pb1-1)) != 0 ) ) {
					EMGD_ERROR("Invalid P1 bits set");
					return -IGD_ERROR_INVAL;
				}

				for(j = 0; j < 8; j++) {
					if(pb1 & BIT(j)) {  /* P1 is divide by 1 to 8 */
						pb1 = j+1;
						break;
					}
				}

				pb2 = (dpllb >> 24) & 0x3; /* P2 is bits 25:24 */

				/* For LVDS port, the post divisor factors are different */
				if((lvds_port & 0xC0000000) == 0xC0000000) {
					/* Pipe is used for LVDS port */
					if(pb2 == 0) {
						pb2 = 14;
					} else if(pb2 == 1) {
						pb2 = 7;
					} else {
						EMGD_ERROR("Invalid P2 bits set = 0x%lx", pb2);
						return -IGD_ERROR_HWERROR;
					}

				} else {
					/* Pipe is used for Non-LVDS port */
					if(pb2 == 0) {
						pb2 = 10;
					} else if(pb2 == 1) {
						pb2 = 5;
					} else {
						EMGD_ERROR(":Invalid P2 bits set = 0x%lx", pb2);
						return -IGD_ERROR_HWERROR;
					}

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

					EMGD_ERROR("Invalid PLL Reference Input Select Reserved");
					return -IGD_ERROR_INVAL;

				} else if(pllb_select  == 2) {
					EMGD_DEBUG("PLL ref is SDVO TV CLK");
					/* TODO: How to handle this value? */
					return -IGD_ERROR_INVAL;
				}

				/* First let's multiply by 1000 * 1000
				 * so that we don't end up in zero during
				 * integer division
				 */
				temp = 1000 * 1000;
				temp = temp * (5 * (mb1+2) + (mb2+2));
				temp = temp /(nb+2);
				dclk = temp/(pb1*pb2);

				/* dclk = temp * ref_freq; */
				/*  FIXME:  This is a workaround to get dclk.  We are supposed
				 *  to be calculating this based on the formula, but DPLL
				 *  is somehow locked and does not return the programmed
				 *  p1 value.  Once this is fixed, we no longer need to have
				 *  igd_display_handle in the parameter of get_pipe_info
				 */
				if (NULL != display) {
					/*igd_display_context_t *display_context =
					  (igd_display_context_t *) display;*/

					/* dclk = ref_freq * m / (p1 * p2); */

					/* PIPE(display)->dclk is in KHhz */
					dclk = PIPE(display)->dclk * 1000;

				} else {
					dclk = 0;
				}


				if( (dclk == 0) || (ref_freq == 0) ) {
					EMGD_ERROR("Dot Clock/Ref Frequency is Zero!!!");
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
static int get_port_info_plb(void)
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
static irqreturn_t interrupt_handler_plb(int irq, void* mmio)
{
	unsigned long iir;
	unsigned long lock_flags;
	unsigned long tmp;
	unsigned long port2_interrupt = 0;
	unsigned long port4_interrupt = 0;
	emgd_vblank_callback_t *cb;

	EMGD_TRACE_ENTER;


	iir = EMGD_READ32(EMGD_MMIO(mmio) + IIR);

	/* Detect whether a vblank interrupt occured, and if so, what type of
	 * processing is needed (do the simple processing now):
	 */
	spin_lock_irqsave(&vblank_lock_plb, lock_flags);
	if ((port2_interrupt = iir & BIT7 /* Port 2/Pipe A/SDVO-B */) != 0) {
		if ((tmp = vblank_interrupt_state & VBLANK_INT4_PORT2) != 0) {
			/* Record "answers" for all requestors: */
			vblank_interrupt_state |= VBINT_ANSWER4_REQUEST(tmp);
		}
	}
	if ((port4_interrupt = iir & BIT5 /* Port 4/Pipe B/Int-LVDS */) != 0) {
		if ((tmp = vblank_interrupt_state & VBLANK_INT4_PORT4) != 0) {
			/* Record "answers" for all requestors: */
			vblank_interrupt_state |= VBINT_ANSWER4_REQUEST(tmp);
		}
	}
	spin_unlock_irqrestore(&vblank_lock_plb, lock_flags);


	if (port4_interrupt) {
		if (mode_context->batch_blits[IGD_PORT_TYPE_LVDS - 1]) {
			notify_userspace_vblank(mode_context->context->drm_dev,
				IGD_PORT_TYPE_LVDS);
		}
	}
	else if (port2_interrupt) {
		if (mode_context->batch_blits[IGD_PORT_TYPE_SDVOB - 1]) {
			notify_userspace_vblank(mode_context->context->drm_dev,
				IGD_PORT_TYPE_SDVOB);
		}
	}

	/* Call any registered/enabled callbacks for this interrupt: */
	cb = &interrupt_callbacks_plb[2];
	if (port2_interrupt && cb->callback &&
		(vblank_interrupt_state & VBINT_ANSWER(VBINT_CB, VBINT_PORT2))) {
		/* Clear the state to indicate the vblank has occured prior to
		 * invoking the callback.
		 */
		vblank_interrupt_state &= ~VBINT_ANSWER(VBINT_CB, VBINT_PORT2);
		cb->callback(cb->priv);
	}
	cb = &interrupt_callbacks_plb[4];
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
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS), EMGD_MMIO(mmio) + PIPEA_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEA_STAT);
		}
		if (port4_interrupt) {
			tmp = EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);
			tmp = tmp & (~PIPESTAT_STS_BITS);
			EMGD_WRITE32((tmp | VBLANK_STS), EMGD_MMIO(mmio) + PIPEB_STAT);
			EMGD_READ32(EMGD_MMIO(mmio) + PIPEB_STAT);
		}

		/* Clear the corresponding bits in the IIR register: */
		EMGD_WRITE32((port2_interrupt | port4_interrupt), EMGD_MMIO(mmio)+IIR);


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
int request_vblanks_plb(unsigned long request_for, unsigned char *mmio)
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

		EMGD_DEBUG("Registering interrupt_handler_plb()");
		if (request_irq(drm_device->pdev->irq, interrupt_handler_plb,
			IRQF_SHARED, EMGD_DRIVER_NAME, mmio)) {
			EMGD_ERROR_EXIT("Failed to register interrupt_handler_plb()");
			return -1;
		} else {
			EMGD_DEBUG("Successfully registered interrupt_handler_plb()");
		}
	}

	/* Lock here to stop the interrupt handler until after changing bits: */
	spin_lock_irqsave(&vblank_lock_plb, lock_flags);

	/* Enable interrupts for the requested purpose/port, actually touching the
	 * hardware registers if newly enabling interrupts for the given port/pipe:
	 */
	if (request_for & VBLANK_INT4_PORT2) {
		if (!VBLANK_INTERRUPTS_ENABLED4_PORT2) {
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
		vblank_interrupt_ref_cnt_port2++;
	} else /* if (request_for & VBLANK_INT4_PORT4) */ {
		if (!VBLANK_INTERRUPTS_ENABLED4_PORT4) {
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
		}
		vblank_interrupt_state |= request_for;
		vblank_interrupt_ref_cnt_port4++;
	}

	/* Unlock to allow the interrupt handler to proceed: */
	spin_unlock_irqrestore(&vblank_lock_plb, lock_flags);

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
int end_request_plb(unsigned long request_for, unsigned char *mmio)
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
	spin_lock_irqsave(&vblank_lock_plb, lock_flags);

	/* Disable interrupts for the requested purpose/port, actually touching the
	 * hardware registers no software wants interrupts for the given port/pipe:
	 */
	if (request_for & VBLANK_INT4_PORT2) {
		/* Decrement reference count */
		vblank_interrupt_ref_cnt_port2--;
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

	if (request_for & VBLANK_INT4_PORT4) {
		/* Decrement reference count */
		vblank_interrupt_ref_cnt_port4--;
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
			}
		}
	}

	/* Unlock to allow the interrupt handler to proceed: */
	spin_unlock_irqrestore(&vblank_lock_plb, lock_flags);

	/* If we've completely disabled all causes for interrupts, unregister the
	 * interrupt handler:
	 */
	if (!VBLANK_INTERRUPTS_ENABLED) {
		struct drm_device *drm_device = mode_context->context->drm_dev;

		EMGD_DEBUG("Unregistering interrupt_handler_plb()");
		free_irq(drm_device->pdev->irq, mmio);
		EMGD_DEBUG("Successfully unregistered interrupt_handler_plb()");
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
int vblank_occured_plb(unsigned long request_for)
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
emgd_vblank_callback_h register_vblank_callback_plb(
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

	interrupt_callbacks_plb[port_number].callback = callback;
	interrupt_callbacks_plb[port_number].priv = priv;

	EMGD_TRACE_EXIT;
	return &interrupt_callbacks_plb[port_number];
}

/*!
 *  Unregisters a previously-registered VBlank interrupt callback function
 *  for a given port.
 *
 * @param callback_h (IN).  The handle that uniquely identifies the VBlank
 *  interrupt callback to unregister.
 */
void unregister_vblank_callback_plb(emgd_vblank_callback_h callback_h)
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
int enable_vblank_callback_plb(emgd_vblank_callback_h callback_h)
{
	emgd_vblank_callback_t *cb = (emgd_vblank_callback_t *) callback_h;
	unsigned long enable_for = VBINT_REQUEST(VBINT_CB,
		(cb->port_number == 2) ? VBINT_PORT2 : VBINT_PORT4);
	unsigned char* mmio =
		EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	int ret;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameter: callback_h=0x%p", callback_h);

	ret = request_vblanks_plb(enable_for, mmio);

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
void disable_vblank_callback_plb(emgd_vblank_callback_h callback_h)
{
	unsigned long lock_flags;
	unsigned long enable_for;
	unsigned char* mmio =
		EMGD_MMIO(mode_context->context->device_context.virt_mmadr);

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Parameter: callback_h=0x%p", callback_h);

	if (callback_h == ALL_PORT_CALLBACKS) {
		/* Need to do some push-ups in order to get interrupts disabled: */
		spin_lock_irqsave(&vblank_lock_plb, lock_flags);
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
		spin_unlock_irqrestore(&vblank_lock_plb, lock_flags);

		end_request_plb(enable_for, mmio);

		spin_lock_irqsave(&vblank_lock_plb, lock_flags);
		vblank_interrupt_state = 0;
		spin_unlock_irqrestore(&vblank_lock_plb, lock_flags);
	} else {
		emgd_vblank_callback_t *cb =
			(emgd_vblank_callback_t *) callback_h;
		enable_for = VBINT_REQUEST(VBINT_CB,
			(cb->port_number == 2) ? VBINT_PORT2 : VBINT_PORT4);

		end_request_plb(enable_for, mmio);
	}

	EMGD_TRACE_EXIT;
}

/*!
 *  checks if the port is enabled
 */
unsigned long get_port_control_plb(unsigned long port_num, unsigned long port_reg)
{
	unsigned long port_value=0;
	unsigned char* mmio = NULL;

	EMGD_TRACE_ENTER;
	mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	port_value = EMGD_READ32(mmio+port_reg);

	EMGD_TRACE_EXIT;
	return port_value;
}

mode_full_dispatch_t mode_full_dispatch_plb = {
	igd_alter_cursor_pos_plb,
	igd_set_palette_entries_plb,
	igd_wait_vsync_plb,
	igd_query_in_vblank_plb,
	igd_get_scanline_plb,
	set_display_base_plb,
	program_cursor_plb,
	set_color_correct_plb,
	igd_get_surface_plb,
	igd_set_surface_plb,
	igd_query_event_plb,
	set_flip_pending_plb,
	check_flip_pending_plb,
	get_plane_info_plb,
	get_pipe_info_plb,
	get_port_info_plb,
	register_vblank_callback_plb,
	unregister_vblank_callback_plb,
	enable_vblank_callback_plb,
	disable_vblank_callback_plb,
	request_vblanks_plb,
	end_request_plb,
	vblank_occured_plb,
	get_port_control_plb,
};


/*
 *-----------------------------------------------------------------------------
 * Filename: igd_mode.c
 * $Revision: 1.35 $
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
 *  Contains client interface support functions for display allocations
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <config.h>
#include <io.h>
#include <memory.h>
#include <sched.h>
#include <gart.h>

#include <igd.h>
#include <igd_init.h>
#include <igd_mode.h>
#include <igd_render.h>
#include <igd_pwr.h>
#include <igd_errno.h>
#include <igd_gmm.h>
#include <igd_pd.h>

#include <debug.h>
#include <math_fix.h>
#include <context.h>
#include <rb.h>
#include <pd.h>
#include <intelpci.h>
#include <dsp.h>
#include <pi.h>
#include <dispatch.h>
#include <mode.h>
#include <mode_access.h>
#include <dsp.h>
#include <utils.h>
#include <general.h>

#include "emgd_drv.h"
#include "drm_emgd_private.h"
#include "match.h"
#include "mode_dispatch.h"

#include "ovl_dispatch.h"
#include "ovl_virt.h"

/*!
 * @addtogroup display_group
 * @{
 */

#define CURSOR_1_STATE 0x01
#define CURSOR_2_STATE 0x02
#define CURSOR_STATE(display) \
	((display == &display_list[0]) ? CURSOR_1_STATE : CURSOR_2_STATE)

/*!
 * This function is an exported utility function.
 * Its meant for calculating backbuffer to frontbuffer coordinates when in
 * rotation, render-scaling and / or flipping (in any combination).
 *
 * Eventually, this function will be only for HAL usage.
 *
 * @param rotation
 * @param do_flip
 * @param do_rscale
 * @param x_rnd_scale
 * @param y_rnd_scale
 * @param front_width
 * @param front_height
 * @param x
 * @param y
 * @param hotx
 * @param hoty
 *
 * @return void
 */
void igd_fb_to_screen(unsigned short rotation,
					unsigned char do_flip, unsigned char do_rscale,
					unsigned long x_rnd_scale, unsigned long y_rnd_scale,
					unsigned short front_width, unsigned short front_height,
					unsigned short *x, unsigned short *y,
					unsigned short hotx, unsigned short hoty);

int igd_kms_match_mode(void *emgd_encoder,
	void *fb_info, igd_timing_info_h **timing);

/* Do not malloc the context */
extern mode_context_t mode_context[];

extern emgd_drm_config_t config_drm;

/* This symbol has to be in this file as it is part of
 *  driver ONLY.
 */
static fw_info_t global_fw_info;

/*!
 * This function sets the per-port attribute to the values given in
 * the parameter.  If the requested port is the PIPE master, then
 * this function will proceed to program the palette.
 *
 * @param display used to program palette, if necessary
 * @param attr_to_set contains the new color attribute to set
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int set_color_correct(igd_display_context_t *display,
	const igd_range_attr_t *attr_to_set)
{
	igd_display_port_t    *port;
	igd_attr_t            *hal_attr_list;
	mode_dispatch_t       *dispatch = mode_context->dispatch;
	igd_range_attr_t      *attr     = NULL;
	unsigned int          i         = 0;
	int  				  changed_flag_set = 1;

	EMGD_TRACE_ENTER;

	port          = PORT_OWNER(display);
	hal_attr_list = port->attributes;


	/* update the HAL's own copy of attributes */
	while (PD_ATTR_LIST_END != hal_attr_list[i].id) {
		if (attr_to_set->id == hal_attr_list[i].id) {
			attr = (igd_range_attr_t *) &hal_attr_list[i];

			if (!(attr_to_set->flags & PD_ATTR_FLAG_VALUE_CHANGED)){
				changed_flag_set = 0;
			}
			/* make sure the value is within range */
			attr->current_value = OS_MAX(attr_to_set->current_value,
									attr->min);

			attr->current_value = OS_MIN(attr_to_set->current_value,
									attr->max);

			break;
		}

		i++;
	}

	/* if we didn't find anything, then quit with an error */
	if (PD_ATTR_LIST_END == hal_attr_list[i].id) {
		return -IGD_ERROR_INVAL;
	}

	/* If the current display is not the pipe master, then we're done */
	if (PIPE(display)->owner != display) {
		return 0;
	}

	/* Program palette */
	if(changed_flag_set){
		dispatch = mode_context->dispatch;
		dispatch->full->set_color_correct(display);
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function is used to put the mode module into the
 * requested powerstate.
 *
 * @param context SS level igd_context
 * @param powerstate IGD_POWERSTATE_D*
 *
 * @return 0 on success
 * @return >0 on failure
 */
int mode_pwr(igd_context_t *context,
	unsigned long powerstate)
{

#if 0
	igd_display_context_t *display_list[2];
	igd_display_context_t *display = NULL;
	int i,j;
#endif

	EMGD_TRACE_ENTER;

	switch(powerstate) {
	case IGD_POWERSTATE_D0:
		mode_context->dispatch->program_cdvo();
		toggle_vblank_interrupts(TRUE);
		break;

#if 0
		for(j = 0; j < 2; j++) {
			display = display_list[j];
			/* if there is no display or display not allocated, continue */
			if(!display || !display->allocated) {
				continue;
			}

			if (!PIPE(display)->timing) {
				/* if there is no pipe timing, cannot enable, continue */
				EMGD_DEBUG("No pipe timing for port = %lu",
							display->port_number);
				continue;
			}

			/* Set port power state */
			for (i = 0; i < IGD_MAX_PORTS; i++) {
				if (display->port[i] &&
					(((igd_display_port_t *)display->port[i])->pt_info->flags &
						IGD_DISPLAY_ENABLE)) {
					mode_context->dispatch->program_port(display, i+1, TRUE);
				}
			}

			mode_context->dispatch->program_pipe(display, TRUE);
			mode_context->dispatch->program_plane(display, TRUE);

			for (i = 0; i < IGD_MAX_PORTS; i++) {
				if (display->port[i] &&
					(((igd_display_port_t *)display->port[i])->pt_info->flags &
						IGD_DISPLAY_ENABLE)) {
					mode_context->dispatch->post_program_port(display, i+1, 0);
				}
			}

		}

		break;
#endif

	case IGD_POWERSTATE_D1:
	case IGD_POWERSTATE_D2:
	case IGD_POWERSTATE_D3:
		toggle_vblank_interrupts(FALSE);
		mode_context->dispatch->reset_plane_pipe_ports(mode_context->context);
		break;
	default:
		break;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end mode_pwr() */

/*!
 * This function is used to save mode module register state.
 *
 * @param context SS level igd_context
 * @param state pointer to module_state handle, where module_state_h is
 * pointer to actual state
 *@param flags should have IGD_REG_SAVE_MODE bit set for save
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
static int mode_save(igd_context_t *context, module_state_h *state,
	unsigned long *flags)
{
	mode_state_t       *mstate;
	int                i, ret;
	igd_display_port_t *port = NULL;
	inter_module_dispatch_t *md;

	EMGD_TRACE_ENTER;

	if (!state || !(*flags & IGD_REG_SAVE_MODE)) {
		EMGD_ERROR_EXIT("NULL pointer to save mode state or"
			" flags don't have IGD_REG_SAVE_MODE bit set.");
		return -IGD_ERROR_INVAL;
	}

	/* First allocate memory for mode state which includes pd states */
	mstate = OS_ALLOC(sizeof(mode_state_t));
	if (!mstate) {
		EMGD_ERROR_EXIT("memory allocation failed.");
		return -IGD_ERROR_NOMEM;
	}
	OS_MEMSET(mstate, 0, sizeof(mode_state_t));

	md = &context->mod_dispatch;

	/* Call pd_save */
	port = NULL;
	i = 0;
	while ((port = md->dsp_get_next_port(context, port, 0)) != NULL) {
		if (port->pd_driver) {
			EMGD_DEBUG("saving %s", port->pd_driver->name);
			ret = port->pd_driver->pd_save(port->pd_context,
				&(mstate->pd_state[i].state), 0);
			if (ret) {
				EMGD_DEBUG("pd_save failed for %s", port->pd_driver->name);
			}

			mstate->pd_state[i].port = port;
			i++;
		}
	}

	/* Update mode state */
	*state = (module_state_h) mstate;

	EMGD_DEBUG("mode_save: saved %d port driver states.", i);

	EMGD_TRACE_EXIT;
	return 0;
} /* end mode_save() */

/*!
 * This function is used to save mode module register state.
 *
 * @param context SS level igd_context
 * @param state pointer to module_state handle, where module_state_h is
 * pointer to actual state
 * *@param flags should have IGD_REG_SAVE_MODE bit set for restore
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int mode_restore(igd_context_t *context, module_state_h *state,
	unsigned long *flags)
{
	int                i, ret;
	igd_display_port_t *port = NULL;
	mode_state_t       *mstate;

	EMGD_TRACE_ENTER;

	if (!state || !(*flags & IGD_REG_SAVE_MODE)) {
		EMGD_ERROR_EXIT("Null mode state.or trying to restore without a save");
		return 0;
	}

	mstate = (mode_state_t *)(*state);

	if (!mstate) {
		EMGD_DEBUG("mode_restore: mstate = NULL");
		EMGD_TRACE_EXIT;
		return 0;
	}

	/* Restore all PD drivers */
	i = 0;
	while (mstate->pd_state[i].port) {
		port = mstate->pd_state[i].port;

		EMGD_DEBUG("restoring %s", port->pd_driver->name);
		ret = port->pd_driver->pd_restore(port->pd_context,
				mstate->pd_state[i].state, 0);
		if (ret) {
			/* What can be done if restore fails */
			EMGD_DEBUG("pd_restore failed for %s", port->pd_driver->name);
		}

		i++;
	}

	/* Free the memory allocated */
	OS_FREE(mstate);
	*state = NULL;

	EMGD_TRACE_EXIT;
	return 0;
} /* end mode_restore() */

/*!
 *
 * @param cursor
 * @param image
 * @param rotate
 * @param flip
 * @param width
 * @param height
 *
 * @return void
 */
static void load_argb_cursor_image(unsigned long *cursor,
		unsigned long *image,
		int rotate, int flip,
		int width, int height)
{
	int w, h, x, y;
	unsigned short nx, ny;
	unsigned long *i;

	w = width;
	h = height;

	/* make sure size is constrained to 64x64 */
	if (w > 64) {
		w = 64;
	}
	if (h > 64) {
		h = 64;
	}

	/* Copy image */
	for (y = 0; y < h; y++) {
		i = image;
		image += width;
		for (x = 0; x < w; x++) {
			/* rotate, flip  x,y here. No scaling!  */
			nx = (unsigned short) x;
			ny = (unsigned short) y;
			igd_fb_to_screen((unsigned short) rotate, (unsigned char) flip,
				0, 0, 0, 64, 64, &nx, &ny, 0, 0);
			cursor[nx + (64 * ny)] = *i++;
		}
	}
}

/*!
 *
 * @param cursor
 * @param image
 * @param rotate
 * @param flip
 * @param width
 * @param height
 *
 * @return void
 */
static void load_xor_cursor_image(unsigned char *cursor,
		unsigned char *image,
		int rotate, int flip,
		int width, int height)
{
	int j, x, y;
	int pixel_num, byte_num, line_num;
	int npixel_num, nbyte_num, nline_num, nbit_num;
	unsigned short nx, ny;
	unsigned char b_val, sbit, mask, pixel;

	for (j = 0; j < 2; j++) {
		cursor += (j * 8);
		image += (j * 8);
		for (y = 0; y < 64; y++) {
			for (x = 0; x < 64; x++) {
				pixel_num = x + (y * 64);
				line_num = pixel_num / 64;
				byte_num = (pixel_num & 63) / 8;
				b_val = *(image + (16 * line_num) + byte_num);
				pixel = (b_val >> ( 7 - (pixel_num & 7))) & 0x01;

				nx = (unsigned short) x;
				ny = (unsigned short) y;
				igd_fb_to_screen((unsigned short) rotate, (unsigned char) flip,
					0, 0, 0, (unsigned short) width, (unsigned short) height,
					&nx, &ny, 0 , 0);
				npixel_num = nx + (ny * 64);
				nline_num = npixel_num / 64;
				nbyte_num = (npixel_num & 63) / 8;
				nbit_num = 7 - (npixel_num & 7);
				b_val = *(cursor + (16 * nline_num) + nbyte_num);

				sbit = pixel << nbit_num;
				mask = 0x01 << nbit_num;
				b_val = (b_val & ~mask) | sbit;
				*(cursor + (16 * nline_num) + nbyte_num) = b_val;

				/*(cursor + (16 * line_num) + byte_num) = b_val; */
			}
		}
	}
}

/*!
 * This function caclulates the correct cursor position from IAL
 * provided coordinates.  It takes into account hotspot, rotation, flip,
 * and render_scale.
 *
 * This takes an x and y coordinate and sets the internal cursor
 * info structure with the updated values.  In addition, it
 * sets a flag if the coordiantes are outside the displays active
 * area.
 *
 * @param display
 * @param x
 * @param y
 * @param hotx
 * @param hoty
 *
 * @return void
 */
static void igd_set_cursor_pos(igd_display_context_t *display,
		unsigned short x, unsigned short y,
		unsigned short hotx, unsigned short hoty)
{
	igd_display_context_t *primary;
	igd_cursor_info_t *internal_ci;
	igd_display_info_t *timing;
	igd_display_plane_t *plane;
	unsigned char render_scale = 0;
	unsigned short rotation, flip;
	unsigned long cursor_state;

	if (!display || !PLANE(display) || !PIPE(display) ||
		!PIPE(display)->cursor || !PIPE(display)->cursor->cursor_info ||
		!PORT_OWNER(display) || !PORT_OWNER(display)->pt_info) {
		return;
	}

	internal_ci = PIPE(display)->cursor->cursor_info;
	timing = PORT_OWNER(display)->pt_info;
	plane = PLANE(display);

	rotation = (unsigned short) ((internal_ci->rotation &
			IGD_RENDER_OP_ROT_MASK) >> 8) * 90;
	flip = (unsigned short) (internal_ci->rotation &
			IGD_RENDER_OP_FLIP) >> 10;

	/*
	 * Handle rotation, flip and render scale
	 * Note that the x,y arguments are unsigned, but cursor cordinates
	 * are signed and we need to preserve the signness when saving back
	 * into internal_ci structure!
	 */
	if ((internal_ci->render_scale_x > 0) ||
			(internal_ci->render_scale_y > 0)) {
		render_scale = 1;
	} else {
		render_scale = 0;
	}

	igd_fb_to_screen((unsigned short) rotation, (unsigned char) flip,
			render_scale,
			internal_ci->render_scale_x, internal_ci->render_scale_y,
			(unsigned short) plane->fb_info->width,
			(unsigned short) plane->fb_info->height,
			&x, &y,
			hotx, hoty);

	/* Adjust the x and y values relative to the current display offset */
	internal_ci->x_offset = (long)((short)x - (short)timing->x_offset);
	internal_ci->y_offset = (long)((short)y - (short)timing->y_offset);


	/* Adjust the cursor offset for rotation and flip */

	switch (rotation) {
	case 0:
		if (flip) {
			internal_ci->x_offset -= 63;
		}
		break;
	case 90:
		internal_ci->y_offset += 1;
		if (!flip) {
			internal_ci->y_offset -= 63;
		}
		break;
	case 180:
		internal_ci->x_offset += 1;
		internal_ci->y_offset -= 63;
		if (!flip) {
			internal_ci->x_offset -= 63;
		}
		break;
	case 270:
		internal_ci->x_offset -= 63;
		if (flip) {
			internal_ci->y_offset -= 63;
		}
		break;
	default:
		break;
	}

	display->context->mod_dispatch.dsp_get_dc(NULL, &primary, NULL);
	if(display == primary) {
		cursor_state = CURSOR_1_STATE;
	} else {
		cursor_state = CURSOR_2_STATE;
	}

	/*
	 * When panning, the cursor can be positioned off screen. The hardware
	 * doesn't like it if this happens.  Thus, we set a flag to indicate
	 * that the cursor is currently off screen. That way it can be
	 * turned off when actually programmed.
	 *
	 * This also moves the position so that it is at the very edge of the
	 * screen, just in case it is turned on.
	 */

	internal_ci->off_screen &= ~cursor_state;

	/* Make sure the cursor is fully displayed */
	if (internal_ci->x_offset < -63) {
		internal_ci->x_offset = -63;
		internal_ci->off_screen |= cursor_state;
	}
	if (internal_ci->x_offset >= (long)timing->width) {
		internal_ci->x_offset = timing->width - 1;
		internal_ci->off_screen |= cursor_state;
	}
	if (internal_ci->y_offset < -63) {
		internal_ci->y_offset = -63;
		internal_ci->off_screen |= cursor_state;
	}
	if (internal_ci->y_offset >= (long)timing->height) {
		internal_ci->y_offset = timing->height - 1;
		internal_ci->off_screen |= cursor_state;
	}

	return;
}

/*!
 * This function sets the cursor_info obtained from igd_alloc_cursor
 * and programs the cursor
 *
 * @param display_handle
 * @param cursor_info
 * @param image
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int igd_alter_cursor(igd_display_h display_handle,
	igd_cursor_info_t *cursor_info,
	unsigned char *image)
{
	igd_display_context_t *display = (igd_display_context_t *) display_handle;
	igd_display_context_t *display2;
	igd_display_context_t *primary;
	igd_cursor_info_t *internal_ci;
	unsigned short rotation, flip;
	unsigned long cursor_state;
	unsigned long cursor_state2;
	unsigned long *cursora = NULL;
	unsigned char *cursorx = NULL;
//	unsigned long in_dihclone=0;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT(display, "Null Display Handle", -IGD_ERROR_INVAL);
	EMGD_ASSERT(cursor_info, "Null cursor info", -IGD_ERROR_INVAL);

//	in_dihclone = display->context->mod_dispatch.in_dih_clone_mode;
	if(validate_cursor(cursor_info, display)) {
		EMGD_ERROR_EXIT("pixel_format validation failed.");
		return -IGD_ERROR_INVAL;
	}

	internal_ci = PIPE(display)->cursor->cursor_info;

	cursor_info->argb_offset = internal_ci->argb_offset;
	cursor_info->xor_offset = internal_ci->xor_offset;

	rotation = (unsigned short) ((cursor_info->rotation &
		IGD_RENDER_OP_ROT_MASK) >> 8) * 90;
	flip = (unsigned short) (cursor_info->rotation & IGD_RENDER_OP_FLIP) >> 10;

	display->context->mod_dispatch.dsp_get_dc(NULL, &primary, NULL);

	/* If cursor plane is mirrored, then do the same for the other cursor */
//	if (PIPE(display)->cursor->mirror != NULL || in_dihclone) {
	if (PIPE(display)->cursor->mirror != NULL ) {
		if(display == primary) {
			display->context->mod_dispatch.dsp_get_dc(NULL, NULL,
				&display2);
		} else {
			display2 = primary;
		}
	}

	/*
	 * Loading new cursor (for both primary and clone):
	 * 1. Blank cursor image: This can be avoided if new hotspot and
	 *    new bitmap size are same as existing hotspot and image size.
	 * 2. Move cursor to new location accounting for new hotspot
	 * 3. Wait for vblank to load the new cursor to avoid flashing/tearing.
	 *    This impacts performace tests as cursor shape changes several times
	 *    while running/loading tests and this wait for vblank counts against
	 *    test times. So going without a wait for vblank. If flashing/tearing
	 *    becomes a must fix issue, then uncomment below wait_vblank().
	 * 4. Load new image
	 */

	if ((image != NULL) && (cursor_info->flags & IGD_CURSOR_LOAD_ARGB_IMAGE)) {
		unsigned long buffer_phys = 0;

		/* Calculate the cursor's address in kernel-space: */
		if (display->context->dispatch.gmm_virt_to_phys(internal_ci->argb_offset,
				&buffer_phys)) {
			EMGD_ERROR_EXIT("GMM Virtual to Physical Address translation failed");
			return -IGD_ERROR_INVAL;
		}

		/*
		 * TODO: Verify that phys_to_virt returns a valid address for
		 * agp memory
		 */
		cursora = phys_to_virt(buffer_phys);
		EMGD_DEBUG("ARGB cursor virtual address is 0x%p", cursora);
		if (cursora == NULL) {
			EMGD_ERROR_EXIT("Physical to Virtual Address translation failed");
			return -IGD_ERROR_INVAL;
		}

		/* Clear cursor plane */
		OS_MEMSET(cursora, 0, 64*64*4);

	} else if ((image != NULL) &&
		(cursor_info->flags & IGD_CURSOR_LOAD_XOR_IMAGE)) {

		unsigned long buffer_phys = 0;

		/* Calculate the cursor's address in kernel-space: */
		if (display->context->dispatch.gmm_virt_to_phys(internal_ci->xor_offset,
				&buffer_phys)) {
			EMGD_ERROR_EXIT("GMM Virtual to Physical Address translation failed");
			return -IGD_ERROR_INVAL;
		}

		/*
		 * TODO: Verify that phys_to_virt returns a valid address for
		 * agp memory
		 */
		cursorx = phys_to_virt(buffer_phys);
		EMGD_DEBUG("XOR cursor virtual address is 0x%p", cursorx);
		if (cursorx == NULL) {
			EMGD_ERROR_EXIT("Physical to Virtual Address translation failed");
			return -IGD_ERROR_INVAL;
		}
	}

	/* calculate the cursor position adjusting to new hotspot */
	igd_set_cursor_pos(display,
		(unsigned short)cursor_info->x_offset,
		(unsigned short)cursor_info->y_offset,
		(unsigned short)cursor_info->hot_x,
		(unsigned short)cursor_info->hot_y);

	/* Update internal structure with altered data */
	internal_ci->pixel_format = cursor_info->pixel_format;
	internal_ci->palette[0] = cursor_info->palette[0];
	internal_ci->palette[1] = cursor_info->palette[1];
	internal_ci->palette[2] = cursor_info->palette[2];
	internal_ci->palette[3] = cursor_info->palette[3];
	internal_ci->flags = cursor_info->flags;
	internal_ci->rotation = cursor_info->rotation;
	internal_ci->render_scale_x = cursor_info->render_scale_x;
	internal_ci->render_scale_y = cursor_info->render_scale_y;

	if(display == primary) {
		cursor_state = CURSOR_1_STATE;
		cursor_state2 = CURSOR_2_STATE;
	} else {
		cursor_state = CURSOR_2_STATE;
		cursor_state2 = CURSOR_1_STATE;
	}

	if ((cursor_info->flags & IGD_CURSOR_ON) &&
			!(internal_ci->off_screen & cursor_state)) {
		mode_context->dispatch->full->program_cursor(display, TRUE);
	} else {
		mode_context->dispatch->full->program_cursor(display, FALSE);
	}

//	if (PIPE(display)->cursor->mirror || (in_dihclone && display == primary) ) {
	if (PIPE(display)->cursor->mirror ) {
		igd_set_cursor_pos(display2,
			(unsigned short)cursor_info->x_offset,
			(unsigned short)cursor_info->y_offset,
			(unsigned short)cursor_info->hot_x,
			(unsigned short)cursor_info->hot_y);

		if ((cursor_info->flags & IGD_CURSOR_ON) &&
				!(internal_ci->off_screen & cursor_state2)) {
			mode_context->dispatch->full->program_cursor(display2, TRUE);
		} else {
			mode_context->dispatch->full->program_cursor(display2, FALSE);
		}
	}

	/* Pitch may have been altered by program_cursor */
	cursor_info->xor_pitch = internal_ci->xor_pitch;
	cursor_info->argb_pitch = internal_ci->argb_pitch;

	/* wait for VBLANK */
	/* mode_context->dispatch->wait_vblank(display); */

	if ((image != NULL) &&
			(cursor_info->flags & IGD_CURSOR_LOAD_ARGB_IMAGE)) {
		load_argb_cursor_image(cursora, (unsigned long *)image, rotation, flip,
				cursor_info->width, cursor_info->height);

	} else if ((image != NULL) &&
			(cursor_info->flags & IGD_CURSOR_LOAD_XOR_IMAGE)) {
		load_xor_cursor_image(cursorx, image, rotation, flip,
				cursor_info->width, cursor_info->height);
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function programs the cursor position.  It takes into account
 * rotation, flip, and render_scale.  It also knows about clone mode
 * and manages the clone cursor automaticlly.
 *
 * @param display_handle
 * @param cursor_info
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_alter_cursor_pos(igd_display_h display_handle,
	igd_cursor_info_t *cursor_info)
{
	igd_display_context_t *display = (igd_display_context_t *) display_handle;
	igd_display_context_t *display2 = NULL;
//	igd_display_context_t * primary= NULL;
	igd_cursor_info_t *internal_ci;
	unsigned long cursor_on_screen;
	unsigned long cursor_state=0;
//	unsigned long in_dihclone=0;

	/* If there is no cursor, return immediately */
	if (!display || !PIPE(display) || !PIPE(display)->cursor) {
		return -IGD_INVAL;
	}

//	in_dihclone = display->context->mod_dispatch.in_dih_clone_mode;

//	display->context->mod_dispatch.dsp_get_dc(NULL, &primary, NULL);


//	if (PIPE(display)->cursor->mirror != NULL || ( in_dihclone && display == primary )) {
	if (PIPE(display)->cursor->mirror != NULL) {
		display->context->mod_dispatch.dsp_get_dc(NULL, NULL, &display2);
		if (display == display2) {
			display->context->mod_dispatch.dsp_get_dc(NULL, &display2, NULL);
			cursor_state = CURSOR_2_STATE;
		} else {
			cursor_state = CURSOR_1_STATE;
		}
	}
	/* Reset display2 if cursor isn't setup */
	if (display2) {
		if (!PIPE(display2) || !PIPE(display2)->cursor) {
			display2 = NULL;
		}
	}

	while (display) {
		internal_ci = PIPE(display)->cursor->cursor_info;
		cursor_on_screen = (internal_ci->off_screen & cursor_state);

		igd_set_cursor_pos(display,
				(unsigned short)cursor_info->x_offset,
				(unsigned short)cursor_info->y_offset,
				(unsigned short)cursor_info->hot_x,
				(unsigned short)cursor_info->hot_y);

		if (cursor_on_screen !=
			(internal_ci->off_screen & cursor_state)) {
			/*
			 * Cursor has moved either on or off screen since the last
			 * call. If it has moved back on screen, turn the cursor
			 * back on.  If it moved off screen, turn it off.
			 */
			if (internal_ci->off_screen & cursor_state) {
				mode_context->dispatch->full->program_cursor(display, FALSE);

			} else if (cursor_info->flags & IGD_CURSOR_ON) {
			/*
			 * Only program the cursor back on if the IAL already has it
			 * ON.  This is to prevent the problem where the IAL turned
			 * of the cursor on purpose, e.g. a user app turns it off, and
			 * then have the HAL turn it back on when moving the cursor
			 * from one screen to another.
			 */
				mode_context->dispatch->full->program_cursor(display, TRUE);
			}
		}

		/* Program cursor position */
		if ((cursor_info->flags & IGD_CURSOR_ON) &&
			!(internal_ci->off_screen & cursor_state)) {
			mode_context->dispatch->full->alter_cursor_pos(
				(igd_display_h)display, internal_ci);
		}

		/* Switch to the second display if it is mirrored */
		display = display2;
		display2 = NULL;
		/* Switch the cursor states as well */
		if(cursor_state == CURSOR_1_STATE) {
			cursor_state = CURSOR_2_STATE;
		} else {
			cursor_state = CURSOR_1_STATE;
		}
	}

	return 0;
}

/*!
 * This function returns the current framebuffer and
 * display information.
 *
 * @param hDisplay required.  The hDisplay contains the display
 * 	information to return.  This parameter was returned from a
 * 	previous call to igd_alloc_display().
 * @param port_number
 * @param pFbInfo required and allocated by the caller.  The pFbInfo
 * 	struct is returned to the caller describing the current
 * 	frame buffer.
 * @param pPtInfo required and allocated by the caller.  The
 * 	pPtInfo struct is returned to caller describing the
 *	requested display parameters.
 * @param ulFlags Currently not used
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_get_display(
	igd_display_h hDisplay,
	unsigned short port_number,
	pigd_framebuffer_info_t pFbInfo,
	pigd_display_info_t pPtInfo,
	unsigned long ulFlags)
{
	igd_display_context_t *display = (igd_display_context_t *)hDisplay;

	EMGD_TRACE_ENTER;

	/* Check for NULL pointers */
	EMGD_ASSERT(pFbInfo, "Null FB Info", -IGD_ERROR_INVAL);
	EMGD_ASSERT(pPtInfo, "Null PT Info", -IGD_ERROR_INVAL);

	/* If the port->pt_info isn't set that means it is called just after
	 * igd_alloc_display and before igd_alter_display(). So, just fill
	 * port_type into pPtInfo */
	if (PORT(display, port_number)->pt_info == NULL) {
		OS_MEMSET(pPtInfo, 0, sizeof(igd_display_info_t));
		pPtInfo->flags = PORT(display, port_number)->port_type;
	} else {
		OS_MEMCPY(pPtInfo, PORT(display, port_number)->pt_info,
			sizeof(igd_display_info_t));
	}

	if (PLANE(display)->fb_info == NULL) {
		OS_MEMSET(pFbInfo, 0, sizeof(igd_framebuffer_info_t));
	} else {
		OS_MEMCPY(pFbInfo, PLANE(display)->fb_info,
			sizeof(igd_framebuffer_info_t));
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function pans the display on the display device.
 * It takes a x_offset, y_offset into the frame buffer and
 * sets the display from (x_offset, y_offset) to
 * (x_offset+width, y_offset+height).
 * If x_offset+width, y_offset+height crosses frame buffer
 * width and heigth, then it will return error.
 *
 * @param hDisplay pointer to an IGD_DISPLAY pointer returned
 * 	from a successful call to igd_allocate_display().
 * @param x_offset frame buffer offsets from (0, 0)
 * @param y_offset frame buffer offsets from (0, 0)
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
long igd_pan_display(igd_display_h hDisplay,
		unsigned long x_offset, unsigned long y_offset)
{
	igd_display_context_t   *display = (igd_display_context_t *)hDisplay;
	igd_framebuffer_info_t  *fb_info;
	igd_display_info_t      *pt_info;
	unsigned long           x_old;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT((display && PORT_OWNER(display) && PORT_OWNER(display)->pt_info),
		"Unvalid Display Handle", -IGD_ERROR_INVAL);

	fb_info = PLANE(display)->fb_info;
	pt_info = PORT_OWNER(display)->pt_info;

	/* Check paning can be done or not */
	if (!fb_info || !pt_info) {
		EMGD_ERROR_EXIT("Panning cannot be done. fb, pt infos weren't set.");
		return -IGD_ERROR_INVAL;
	}

	if (! (PORT_OWNER(display)->pt_info->flags & IGD_DISPLAY_ENABLE)) {
		EMGD_DEBUG("Currently this display is not enabled.");
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}

	if ((pt_info->width == fb_info->width) &&
		(pt_info->height == fb_info->height)) {
		EMGD_DEBUG("FB and display pt_info are same size, no paning.");
		EMGD_TRACE_EXIT;
		return 0;
	}

	/*
	 * handle case primary: 1280x720  seconday:1024x768
	 * TODO:
	 * "ideally the FB 720 height should center on LVDS 768 height and pan across horizontally that is 1280 FB width pans on LVDS 1024 width."
	 */
	if (fb_info->height >= pt_info->height) {
		if (pt_info->width + x_offset > fb_info->width ||
			pt_info->height + y_offset > fb_info->height) {
			EMGD_ERROR_EXIT("invalid offsets are passing frame buffer.");
			return -IGD_ERROR_INVAL;
		}
	} else {
		if (pt_info->width + x_offset > fb_info->width ) {
			EMGD_ERROR_EXIT("invalid offsets are passing frame buffer.");
			return -IGD_ERROR_INVAL;
		}
	}

	/* Now do the paning.
	 * Note: Never change the fb_base_offset (it always points to the
	 *       first pixel in the frame buffer), nor the visible_offset (it
	 *       always points to the first pixel of the visible buffer). */
	/* TODO: How to know which fb we are using, frontbuffer/backbuffer? */

	x_old = x_offset;
	mode_context->dispatch->full->set_display_base(display, fb_info,
			&x_offset, &y_offset);

	/* Save offsets into pt_info */
	pt_info->x_offset = (unsigned short)x_offset;
	pt_info->y_offset = (unsigned short)y_offset;

	/* igd_ovl_set_display also set this flag, this flag include:
	 * 1) ovl/sprite C swap, by igd_ovl_set_display
	 * 2) swap display, or only change 1 display,  by pan display
	*/
	ovl_context->ovl_display_swapped = 1;

	/* FIXME:
	 *   This could be negative, and thus conflict with the error codes
	 *   returned above.  However, this is needed for Whitney to compensate
	 *   for the framebuffer offset alignment requirements and should only
	 *   be positive.
	 */
	EMGD_TRACE_EXIT;
	return (x_offset - x_old);

} /* end igd_pan_display() */

/*!
 * This function get attributes for a port. SS will
 * allocate the memory required to return the *attr_list.
 * This is a live copy of attributes used by both IAL and SS.
 * Don't deallocate this memory. This will be freed by SS.
 *
 * The list returned will contain a list of HAL attributes
 * followed by a pointer to the PD attributes.
 *
 * @param driver_handle pointer to an IGD context pointer returned
 * 	from a successful call to igd_init().
 * @param port_number port number of port to get pd attributes.
 * @param num_attrs pointer to return the number of attributes
 * 	returned in attr_list.
 * @param attr_list - pointer to return the attributes.
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_get_attrs(
	igd_driver_h  driver_handle,
	unsigned short port_number,
	unsigned long *num_attrs,
	igd_attr_t **attr_list)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t *display;
	int                   ret;
	unsigned long         hal_attrib_num, pd_attr_num;
	igd_display_port_t    *port;
	igd_attr_t            *pd_attr_list = NULL;


	EMGD_TRACE_ENTER;

	/* basic parameter check */
	EMGD_ASSERT(driver_handle, "Null Driver Handle", -IGD_ERROR_INVAL);

	/* Get the display context that is currently using this port */
	context->mod_dispatch.dsp_get_display(port_number, &display, &port, 0);

	if(!display || !port) {
		/*
		 * No display is using this port,
		 * should we abort at this point?
		 */
		EMGD_TRACE_EXIT;
		printk(KERN_ALERT "[EGD] igd_get_attrs exiting: "
				"No display is using port %d.\n", port_number);
		return -IGD_ERROR_INVAL;
	}

	/* Get PD attributes */
	pd_attr_num = *num_attrs;
	ret = port->pd_driver->get_attrs( port->pd_context,
									&pd_attr_num,
									&pd_attr_list );

	if (ret) {
		pd_attr_num  = 0;
		pd_attr_list = NULL;
	}

	/* find the extension attribute and attach the pd_attr_list */
	for (hal_attrib_num = 0;
			PD_ATTR_LIST_END != port->attributes[hal_attrib_num].id;
			hal_attrib_num++ ) {

		if (PD_ATTR_ID_EXTENSION == port->attributes[hal_attrib_num].id ) {
			((igd_extension_attr_t *)(&port->attributes[hal_attrib_num]))->extension = pd_attr_list;
		}
	}

	/* If attr_list is NULL then the caller is only interested in
	 * the number of attributes
	 */
	if( NULL != attr_list ) {
		*attr_list = port->attributes;
	}

	if (0 == pd_attr_num ) {
		/* if there are no PD attributes, then subtract 1 from hal_attrib_num
		 * so that the "extension" attribute is not counted*/
		*num_attrs = hal_attrib_num - 1;
	} else {
		/* subtract 1 because we should not count the "extension" attribute */
		*num_attrs = hal_attrib_num + pd_attr_num - 1;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end igd_get_attrs() */

/*!
 * This function set attributes on a port.
 *
 * @param driver_handle pointer to an IGD context pointer returned
 * 	from a successful call to igd_init().
 * @param port_number port number of port to get pd attributes.
 * @param num_attrs pointer to return the number of attributes
 * 	returned in attr_list.
 * @param attr_list - pointer to return the attributes.
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_set_attrs(
	igd_driver_h  driver_handle,
	unsigned short port_number,
	unsigned long num_attrs,
	igd_attr_t *attr_list)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t *display;
	igd_display_port_t    *port;
	igd_attr_t            *attr;
	unsigned int          i;

	unsigned long         num_attrs_set = 0;
	int                   ret = 0, setmode = 0;
	igd_timing_info_t     *pd_timing_table = NULL;

	EMGD_TRACE_ENTER;

	/* basic parameter check */
	EMGD_ASSERT(driver_handle, "Null Driver Handle", -IGD_ERROR_INVAL);

	/* Get the display context that is currently using this port */
	context->mod_dispatch.dsp_get_display(port_number, &display, &port, 0);
	if(!display || !port) {
		/* No display is using this port, should we abort at this point? */
		return -IGD_ERROR_INVAL;
	}

	if(num_attrs == 0) {
		return 0;
	}


	/*
	 * Take care of HAL attributes.  Here we keep track of the number of
	 * attributes set.  If the number set is equal to num_attrs, then we
	 * don't need to call port driver's set_attr when this loop is done
	 */
	for( i = 0; i < num_attrs; i ++ ) {
		switch (attr_list[i].id) {
			case PD_ATTR_ID_FB_GAMMA:
			case PD_ATTR_ID_FB_BRIGHTNESS:
			case PD_ATTR_ID_FB_CONTRAST:
				set_color_correct(display, (igd_range_attr_t *) &attr_list[i]);
				num_attrs_set++;
				break;

			default:
				/* ignore all non HAL-related attributes */
				break;
		}
	}

	/* Pass the attribute list down to the port driver for futher processing
	 * if necessary */
	if (num_attrs > num_attrs_set) {
		ret = port->pd_driver->set_attrs(port->pd_context, num_attrs, attr_list);

		if (ret) {
			return -IGD_INVAL;
		}
	}

	attr = attr_list;
	i = 0;
	while (i++ < num_attrs) {
		if (attr->flags & PD_ATTR_FLAG_SETMODE) {
			setmode = 1;
			break;
		}
		attr++;
	}

	ret = 0;
	if (setmode) {
		/* Update internal timings */
		ret = port->pd_driver->get_timing_list(port->pd_context,
				(pd_timing_t *)port->timing_table,
				(pd_timing_t **)&pd_timing_table);

		if (ret) {
			EMGD_ERROR_EXIT("get_timing_list failed.");
			return -IGD_ERROR_INVAL;
		}

		port->timing_table = pd_timing_table;
		port->num_timing = get_native_dtd(pd_timing_table,
				PI_SUPPORTED_TIMINGS, &port->fp_native_dtd,
				PD_MODE_DTD_FP_NATIVE);
		ret = IGD_DO_QRY_SETMODE;
	}

	EMGD_TRACE_EXIT;
	return ret;
} /* end igd_set_attrs() */

/*!
 *
 * @param display_h
 * @param width
 * @param height
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int mode_getresolution(
	igd_display_h display_h,
	unsigned long *width,
	unsigned long *height)
{
	igd_display_context_t *display;
	int ret = 0;

	if (display_h) {
		display = (igd_display_context_t *) display_h;

		if (PORT_OWNER(display)->pt_info) {
			*width  = PORT_OWNER(display)->pt_info->width;
			*height = PORT_OWNER(display)->pt_info->height;
			ret = 0;
		}
		else {
			ret = -IGD_ERROR_INVAL;
		}

	} else {
		ret = -IGD_ERROR_INVAL;
	}

	return ret;
}

/*!
 *
 * @param driver_handle A igd_driver_h type returned from a previous
 * 	igd_init call.
 * @param port_number
 * @param port_info Returns the information about port
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_get_port_info(igd_driver_h driver_handle,
	unsigned short port_number,
	igd_port_info_t *port_info)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_port_t    *port;
	pd_port_status_t      port_status;
	int                   ret;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT(driver_handle, "Invalid Driver Handle", -IGD_ERROR_INVAL);

	if(port_number >= (IGD_MAX_PORTS + 1)) {
		EMGD_ERROR("Error, invalid port number. ");
		return -IGD_INVAL;
	}

	OS_MEMSET(port_info, 0, sizeof(igd_port_info_t));

	context->mod_dispatch.dsp_get_display(port_number, NULL, &port, 0);

	if (!port || !port->pd_driver || !port->pd_driver->pd_get_port_status) {
		EMGD_ERROR_EXIT("pd_get_port_status not implemented. ");
		return -IGD_ERROR_INVAL;
	}

	pd_strcpy(port_info->pd_name, port->pd_driver->name);
	pd_strcpy(port_info->port_name, port->port_name);

	port_info->port_num = port->port_number;
	OS_MEMCPY(&port_info->driver_version, port->pd_driver->version,
		sizeof(pd_version_t));

	ret = port->pd_driver->pd_get_port_status(port->pd_context, &port_status);
	if (ret == PD_SUCCESS) {
		if (port_status.connected == PD_DISP_STATUS_DETACHED) {
			/* 0 = Not Connected */
			port_info->connected = 0;
		} else {
			/* PD_DISP_STATUS_ATTACHED, PD_DISP_STATUS_UNKNOWN */
			port_info->connected = 1;
		}
	} else {
		port_info->connected = 1;
	}

	port_info->display_type = port_status.display_type;

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param driver_handle A igd_driver_h type returned from a previous
 * 	igd_init call.
 * @param i2c_reg
 * @param flags
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int igd_access_i2c(
	igd_driver_h  driver_handle,
	igd_i2c_reg_t *i2c_reg,
	unsigned long flags)
{
	igd_context_t  *context = (igd_context_t *)driver_handle;
	unsigned char  i;
	unsigned long  *gpio, num_gpio;
	int            ret = 0;

	EMGD_TRACE_ENTER;

	if (!driver_handle || !i2c_reg) {
		return -IGD_INVAL;
	}

	num_gpio = mode_context->dispatch->get_gpio_sets(&gpio);

	if (i2c_reg->bus_id >= num_gpio) {
		EMGD_DEBUG("Invalid bus number specified.");
		return -IGD_ERROR_INVAL;
	}

	if ((unsigned long)(i2c_reg->reg + i2c_reg->num_bytes) > 0xFF) {
		EMGD_DEBUG("Invalid number of %d bytes requested from reg 0x%x.",
			i2c_reg->num_bytes, i2c_reg->reg);
		return -IGD_ERROR_INVAL;
	}

	if (flags == IGD_I2C_WRITE) {
		pd_reg_t temp_reg[2];
		temp_reg[1].reg = PD_REG_LIST_END;

		/* I2C write operation */
		for (i=0; i<i2c_reg->num_bytes; i++) {
			temp_reg[0].reg = i2c_reg->reg + i;
			temp_reg[0].value = i2c_reg->buffer[i];
			ret = context->mod_dispatch.i2c_write_reg_list(
				context,
				gpio[i2c_reg->bus_id],
				i2c_reg->i2c_speed,
				i2c_reg->dab,
				temp_reg,
				0);
			if (ret) {
				EMGD_DEBUG("i2c write error.");
				break;
			}
		}
	} else if (flags == IGD_I2C_READ) {
		/* I2C read operation */
		ret = context->mod_dispatch.i2c_read_regs(
			context,
			gpio[i2c_reg->bus_id],
			i2c_reg->i2c_speed,
			i2c_reg->dab,
			i2c_reg->reg,
			i2c_reg->buffer,
			i2c_reg->num_bytes,
			0);
		if (ret) {
			EMGD_DEBUG("i2c read error.");
		}
	}

	EMGD_TRACE_EXIT;
	return ret;
} /* end igd_access_i2c() */

/*!
 *
 * @param driver_handle A igd_driver_h type returned from a previous
 * 	igd_init call.
 * @param port_number
 * @param edid_version
 * @param edid_revision
 * @param edid_size
 *
 * @return -IGD_ERROR_INVAL or -IGD_ERROR_EDID on failure
 * @return 0 on success
 */
/* FIXME: Move to PI */
static int igd_get_EDID_info(igd_driver_h driver_handle,
	unsigned short port_number,
	unsigned char *edid_version, unsigned char *edid_revision,
	unsigned long *edid_size)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_port_t *port;
	int                   ret;
	unsigned char         temp_buf[25];
	unsigned char         edid_1_3_header[] = {
		0x00, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0x00
	};

	EMGD_TRACE_ENTER;

	if (!driver_handle || !edid_version || !edid_revision || !edid_size) {
		return -IGD_ERROR_INVAL;
	}

	context->mod_dispatch.dsp_get_display(port_number, NULL, &port, 0);
	if(!port) {
		return -IGD_ERROR_INVAL;
	}

	/* Read EDID */
	ret = context->mod_dispatch.i2c_read_regs(context,
		port->ddc_reg,
		10,              /* DDC speed 10 KHz */
		port->ddc_dab,
		0x00,             /* DDC Address */
		temp_buf,         /* Read 20 bytes into temp_buf */
		20,
		0);
	if (ret) {
		return -IGD_ERROR_EDID;
	}

	/* Check for EDID version */
	if (!OS_MEMCMP((void *)temp_buf, (void *)edid_1_3_header, 8)) {
		*edid_version  = temp_buf[18];
		*edid_revision = temp_buf[19];
		*edid_size     = 128;

		/* Read EDID byte 0x7E which gives the number of (optional) 128-byte
		 * EDID extension blocks to follow. */
		temp_buf[0] = 0;
		ret = context->mod_dispatch.i2c_read_regs(context,
			port->ddc_reg,
			10,              /* DDC speed 10 KHz */
			port->ddc_dab,
			0x7E,             /* DDC Address */
			&temp_buf[0],     /* Read 1 byte into temp_buf */
			1,
			0);
		if (ret) {
			return -IGD_ERROR_EDID;
		}

		*edid_size += (temp_buf[0] * 128);

	} else {
		*edid_version  = temp_buf[0] >> 4;
		if (*edid_version != 2) {
			return -IGD_ERROR_EDID;
		}

		*edid_revision = temp_buf[0] & 0x0F;
		*edid_size     = 256;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end igd_get_EDID_info() */

/*!
 * This function sets enables or disables a specific port. Or, if
 * the port number passed in is zero, it applies the change to all
 * ports associated with the display handle.
 *
 * @param driver_handle display handle.
 * @param port_number port number to enable or disable (or zero).
 * @param flag IGD_ENABLE or IGD_DISABLE
 * @param test IGD_TEST or zero
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
static int igd_enable_port(igd_display_h display_handle,
		unsigned short port_number,
		unsigned long flag,
		unsigned long test)
{
	igd_display_context_t *display;
	igd_display_port_t *port;
	int i;
	int ret;

	EMGD_TRACE_ENTER;

	display = (igd_display_context_t *)display_handle;

	/*
	 * There are a number of things that will probably need to be checked.
	 *
	 * When enabling a port, make sure the display has pipe timings.
	 *
	 * When enabling a port and display_detect is on, make sure a display
	 * is detected before enabling the port.
	 *
	 * When enabling a port, check if it must be master, if so, the timings
	 * will need to be changed.
	 *
	 * When disabling a port, don't disable the only port allocated to the
	 * display.
	 *
	 * When disabling a port and it was the master, change timings to next
	 * allocated port.
	 */

	if (!PIPE(display)->timing && (flag & IGD_DISPLAY_ENABLE)) {
		EMGD_ERROR("port enable requested without pipe timings.");
		return -IGD_ERROR_INVAL;
	}

	if ((port_number > 0) && (port_number <= IGD_MAX_PORTS)) {
		port = (igd_display_port_t *)display->port[port_number - 1];
		if (port == NULL) {
			return -IGD_ERROR_INVAL;
		}

		if (test == IGD_TEST) {
			if ((port->pt_info->flags & IGD_DISPLAY_ENABLE) ==
					(flag & IGD_DISPLAY_ENABLE)) {
				return 0;
			} else {
				return -IGD_ERROR_INVAL;
			}
		}

		if (flag & IGD_DISPLAY_ENABLE) {
			/* Can we enable this port? */
			if (dsp_display_connected(display->context, port)) {
				port->pt_info->flags |= IGD_DISPLAY_ENABLE;
			} else {
				return -IGD_ERROR_INVAL;
			}
		} else {
			port->pt_info->flags &= ~IGD_DISPLAY_ENABLE;
		}
	} else if ((port_number == 0) && !(flag & IGD_DISPLAY_ENABLE)) {
		/* special case, disable all ports */
		for (i = 0; i < IGD_MAX_PORTS; i++) {
			if (display->port[i]) {
				port = (igd_display_port_t *)display->port[i];
				port->pt_info->flags &= ~IGD_DISPLAY_ENABLE;
			}
		}
	}

	/* Program ports to the proper enable/disable state */
	for(i = 0; i < IGD_MAX_PORTS; i++) {
		if ((port = (igd_display_port_t *)display->port[i]) != NULL) {
			if (!port_number || (port->port_number == port_number)) {
				if (port->pt_info->flags & IGD_DISPLAY_ENABLE) {
					ret = mode_context->dispatch->program_port(display, i+1,
						TRUE);
				} else {
					ret = mode_context->dispatch->program_port(display, i+1,
						FALSE);
				}
				if (ret != 0) {
					EMGD_ERROR("programming port %d failed", i+1);
				}
				if (port->pt_info->flags & IGD_DISPLAY_ENABLE) {
					/* Call post_program_port if port is getting enabled. */
					ret = mode_context->dispatch->post_program_port
						(display, i+1, 0);
				} else {
					ret = 0;
				}
				if (ret != 0) {
					EMGD_ERROR("post programming port %d failed", i+1);
				}
			}
		}
	}

	EMGD_TRACE_EXIT;

	return 0;
}

/*!
 * Free memory allocated to hold timing infomation. This must have been
 * allocated using the igd_query_mode_list.
 *
 * @param mode_list
 *
 * @return void
 */
void igd_free_mode_list(igd_display_info_t *mode_list)
{
	if (mode_list) {
		OS_FREE(mode_list);
	}
}

/*!
 * Generate a mode list that is correct for the given pipe master and
 * any twins.
 *
 * @param driver_handle
 * @param dc
 * @param mode_list
 * @param port
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int full_mode_query(igd_driver_h driver_handle,
	unsigned long dc,
	igd_display_info_t **mode_list,
	igd_display_port_t *port)
{
	igd_timing_info_t *tt;
	igd_timing_info_t *xt;
	int timings = 0;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Looking up timings for port: %ld", port->port_number);

	/* determine the size of the mode list including the extensions */
	tt = port->timing_table;
	if (!tt) {
		EMGD_ERROR("igd_query_mode_list:  No Timings");
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}
	while (tt->width != IGD_TIMING_TABLE_END) {
		/*
		 * Check here to see if this timing is valid on all other
		 * ports. If it is, then we'll add it to the list, otherwise
		 * it's skipped.
		 */
		if (tt->mode_info_flags & PD_MODE_SUPPORTED) {
			timings++;
			EMGD_DEBUG("Adding timing: (%dx%d)", tt->width, tt->height);
		}
		tt++;

		/*
		 * If reached the first table END, then check for timings
		 * added by the user or EDID.
		 */
		if ((tt->width == IGD_TIMING_TABLE_END) && tt->extn_ptr) {
			tt = tt->extn_ptr;
		}
	}
	timings++; /* add one for the terminating marker */

	*mode_list = OS_ALLOC(sizeof(igd_timing_info_t) * timings);
	if (*mode_list != NULL) {
		/* Now build the new mode list */
		tt = port->timing_table;
		xt = (igd_timing_info_t *)*mode_list;
		while (tt->width != IGD_TIMING_TABLE_END) {
			if (tt->mode_info_flags & PD_MODE_SUPPORTED) {
				OS_MEMCPY(xt, tt, sizeof(igd_timing_info_t));
				xt++;
			}

			tt++;
			/*
			 * If reached the first table END, then check for timings
			 * added by the user or EDID.
			 */
			if ((tt->width == IGD_TIMING_TABLE_END) && tt->extn_ptr) {
				tt = tt->extn_ptr;
			}
		}
		/* copy end of list marker */
		OS_MEMCPY(xt, tt, sizeof(igd_timing_info_t));
	} else {
		EMGD_ERROR("igd_query_mode_list: Memory allocation failure.");
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}
	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function is used to shutdown any module/dsp
 * module specific structures or tables etc.
 *
 * @param context SS level igd_context.
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
static void mode_shutdown(igd_context_t *context)
{
	inter_module_dispatch_t *md;
	module_state_h *mode_state = NULL;
	unsigned long *flags = NULL;
	reg_state_id_t id;

	EMGD_DEBUG("mode_shutdown Entry");

	/*
	 * Disable all the displays in decending pipe order
	 * Note: This isn't exactly device independent. It works for all
	 * multipipe platforms that we know of but is limited to 2 pipes
	 * and assumes that it is ok to disable pipe b before a.
	 */

	dsp_wait_rb(mode_context->context);

	toggle_vblank_interrupts(FALSE);

	/* Disable all VBlank interrupts: */
	context->dispatch.disable_vblank_callback(ALL_PORT_CALLBACKS);

	/* Reset all planes, pipe, ports to a known "off" state */
	mode_context->dispatch->reset_plane_pipe_ports(context);

	/* Shutdown dsp module */
	context->mod_dispatch.dsp_shutdown(context);

	/* Restore mode state */
	md = &context->mod_dispatch;
	if (config_drm.init) {
		id = REG_MODE_STATE_CON;
	} else {
		id = REG_MODE_STATE_REG;
	}
	md->reg_get_mod_state(id, &mode_state, &flags);
	mode_restore(context, mode_state, flags);

	/* Shutdown PI module */
	context->mod_dispatch.pi_shutdown(context);

	/*
	 * Do not clear mode_context pointer. It needs to stay around until
	 *  the very last thing.
	 *
	 *	context->mode_context = NULL;
	 */

	EMGD_DEBUG("Return");
	return;
} /* end mode_shutdown() */

/*!
 * This function is used to shutdown any module/dsp
 * module specific structures or tables etc.
 *
 * @param context SS level igd_context.
 * @param mode_context mode module initialization parameters
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int full_mode_init(igd_context_t *context,
	mode_context_t *mode_context)
{
	igd_dispatch_t     *dispatch = &context->dispatch;

	/* Hook up the IGD dispatch table entires for mode */
	dispatch->get_display = igd_get_display;
	dispatch->pan_display = igd_pan_display;
	dispatch->alter_cursor = igd_alter_cursor;
	dispatch->alter_cursor_pos = igd_alter_cursor_pos;
	dispatch->get_attrs = igd_get_attrs;
	dispatch->set_attrs = igd_set_attrs;
	dispatch->get_port_info = igd_get_port_info;
	dispatch->access_i2c = igd_access_i2c;
	dispatch->get_EDID_info = igd_get_EDID_info;
	dispatch->free_mode_list = igd_free_mode_list;
	dispatch->enable_port = igd_enable_port;

	/* Hook up optional inter-module functions */
	context->mod_dispatch.mode_save = mode_save;
	context->mod_dispatch.mode_restore = mode_restore;
	context->mod_dispatch.mode_pwr = mode_pwr;
	context->mod_dispatch.mode_shutdown = mode_shutdown;
	context->mod_dispatch.set_flip_pending =
		mode_context->dispatch->full->set_flip_pending;
	context->mod_dispatch.check_flip_pending =
		mode_context->dispatch->full->check_flip_pending;
	context->mod_dispatch.get_dd_timing =
		mode_context->dispatch->get_dd_timing;
	context->mod_dispatch.check_port_supported =
		mode_context->dispatch->check_port_supported;
	context->mod_dispatch.get_refresh_in_border =
		mode_context->dispatch->get_refresh_in_border;
	context->mod_dispatch.get_port_control =
		mode_context->dispatch->full->get_port_control;


	/* Hook up Core specific IGD dispatch table entries */
	dispatch->set_palette_entry = mode_context->dispatch->set_palette_entry;
	dispatch->get_scanline = mode_context->dispatch->full->get_scanline;
	dispatch->wait_vsync = mode_context->dispatch->full->wait_vsync;
	dispatch->query_in_vblank = mode_context->dispatch->full->query_in_vblank;
	dispatch->get_surface = mode_context->dispatch->full->get_surface;
	dispatch->set_surface = mode_context->dispatch->full->set_surface;
	dispatch->query_event = mode_context->dispatch->full->query_event;
	dispatch->register_vblank_callback =
		mode_context->dispatch->full->register_vblank_callback;
	dispatch->unregister_vblank_callback =
		mode_context->dispatch->full->unregister_vblank_callback;
	dispatch->enable_vblank_callback =
		mode_context->dispatch->full->enable_vblank_callback;
	dispatch->disable_vblank_callback =
		mode_context->dispatch->full->disable_vblank_callback;

	dispatch->unlock_planes = mode_context->dispatch->full->unlock_planes;
	/* Assign the fw_info structure and Zero-out the contents */
	mode_context->fw_info = &global_fw_info;
	OS_MEMSET(mode_context->fw_info, 0, sizeof(fw_info_t));

    /* Set the mode context quickboot options from the params */
    mode_context->quickboot = context->mod_dispatch.init_params->quickboot;
    mode_context->seamless = context->mod_dispatch.init_params->qb_seamless;
    mode_context->video_input = context->mod_dispatch.init_params->qb_video_input;
    mode_context->splash = context->mod_dispatch.init_params->qb_splash;

	/* Just in case, disable all VBlank interrupts: */
	dispatch->disable_vblank_callback(ALL_PORT_CALLBACKS);

	return 0;
} /* end full_mode_init() */

/*!
 * This function clears the framebutffer.
 *
 * @param mode_context mode module initialization parameters
 * @param fb_info
 * @param user_fb - the caller can provide an fb. If they do, then this
 * function will not mem map it. If they provide this parameter as NULL,
 * then we will go ahead and map one here.
 *
 * @return void
 */
void full_clear_fb(mode_context_t *mode_context,
		igd_framebuffer_info_t *fb_info,
		unsigned char *user_fb)
{
	unsigned short line, index;
	unsigned char *fb;
	volatile unsigned long *base_line_pitch;
	volatile unsigned long pitch_value;

	if(user_fb) {
		fb = user_fb;
	} else {
		fb = mode_context->context->dispatch.gmm_map(
				fb_info->fb_base_offset);
		EMGD_DEBUG("After gmm_map(), fb = 0x%p", fb);
	}
	base_line_pitch = (unsigned long *)fb;
	pitch_value = fb_info->screen_pitch / 4;

	for (line = 0; line < fb_info->height; line++) {
		for(index = 0; index < pitch_value; index++) {
			base_line_pitch[index] = mode_context->display_color;
		}
		base_line_pitch += pitch_value;
	}

	if(!user_fb) {
		mode_context->context->dispatch.gmm_unmap(fb);
	}
}

/*!
 * This is done in an attempt to re-use the plane and cursor allocated
 * to a port when that port is moved to a new display handle.  The basic
 * case is when the DC changes in this way:
 *
 *     0x00200058 -> 0x00000021
 *     0x00000021 -> 0x00200058
 *
 * In both these cases, it is better if the plane and cursor are sticky
 * to port 2.
 *
 * @param void
 *
 * @return void
 */
void swap_fb_cursor( void )
{
	igd_plane_t *display_plane1, *display_plane2;
	igd_display_pipe_t *pipe1, *pipe2;
	void *tmp;

	mode_context->context->mod_dispatch.dsp_get_planes_pipes(
		&display_plane1, &display_plane2,
		&pipe1, &pipe2);

	/* Swap the plane info data */
	if (display_plane1 && display_plane2) {

		tmp = display_plane1->plane_info;
		display_plane1->plane_info = display_plane2->plane_info;
		display_plane2->plane_info = tmp;
	}

	/* Swap the cursor info */
	if (pipe1 && pipe2 && pipe1->cursor && pipe2->cursor) {

		tmp = pipe1->cursor->cursor_info;
		pipe1->cursor->cursor_info = pipe2->cursor->cursor_info;
		pipe2->cursor->cursor_info = tmp;
	}
}

/*!
 * This function calculates target X and Y coordincates
 * for a provided front buffer dimension after including
 * corrections for render-scaling, rotation and flipping.
 *
 * @param rotation 0, 90, 180 or 270
 * @param do_flip 0 or 1
 * @param do_rscale 0 or 1
 * @param x_rnd_scale fixed point int = (native_width  << 16) / fb_width
 * @param y_rnd_scale fixed point int = (native_height << 16) / fb_height
 * @param front_width front buffer width for this pipe
 * @param front_height front buffer height for this pipe
 * @param x
 * @param y
 * @param hotx
 * @param hoty
 *
 * @return void
 */
void igd_fb_to_screen(unsigned short rotation,
					unsigned char do_flip, unsigned char do_rscale,
					unsigned long x_rnd_scale, unsigned long y_rnd_scale,
					unsigned short front_width, unsigned short front_height,
					unsigned short *x, unsigned short *y,
					unsigned short hotx, unsigned short hoty)
{
	unsigned short x_temp;
	unsigned short y_temp;

	/*
	 * we only need to up(or down)-scale the coordinates of
	 * of the cursor when we are moving the cursor... dont care
	 * about scaling the cursor image itself
	 */
	if(do_rscale){
		unsigned long phys_x_pos_scale = x_rnd_scale;
		unsigned long phys_y_pos_scale = y_rnd_scale;

		if(rotation == 90 || rotation == 270) {
			phys_x_pos_scale = y_rnd_scale;
			phys_y_pos_scale = x_rnd_scale;
		}

		/* Added code to fix rounding error */
		if(*x & 0x8000) {
			/* if its a negative x_offset, we need to put in additional
			 * type casting so the negative signage doesnt get scaled up
			 * and down and end up being a very large positive number
			 */
			*x = (unsigned short)(-(short)((((unsigned long )
				((short)-*x) * phys_x_pos_scale) + ((1<<15)-1)) >> 16));
		} else {
			*x = (unsigned short)((((unsigned long)*x * phys_x_pos_scale) + ((1<<15)-1)) >> 16);
		}

		if(*y & 0x8000) {
			/* if its a negative y_offset, we need to put in additional
			 * type casting so the negative signage doesnt get scaled up
			 * and down and end up being a very large positive number
			 */
			*y = (unsigned short)(-(short)((((unsigned long )
				((short)-*y) * phys_y_pos_scale) + ((1<<15)-1)) >> 16));
		} else  {
			*y = (unsigned short)((((unsigned long)*y * phys_y_pos_scale) + ((1<<15)-1)) >> 16);
		}
	}

	*x -= hotx;
	*y -= hoty;

	x_temp = *x;
	y_temp = *y;

	switch(rotation) {
	case 0:
	default:
		if(do_flip) {
			*x = front_width-1 - x_temp;
		}
		break;
	case 90:
		*x = y_temp;
		*y = (front_height - 1) - x_temp;
		if(do_flip) {
			*y = front_height-1 - *y ;
		}
		break;
	case 180:
		/* This is accurate for a 180 rotate */
		*x = (front_width -1) -  x_temp;
		*y = (front_height-1) - y_temp;
		if(do_flip) {
			*x = (front_width -1) - *x;
		}
		break;
	case 270:
		*x = (front_width - 1) - y_temp;
		*y=  x_temp;
		if(do_flip) {
			*y = (front_height -1) - *y;
		}
		break;
	}
}



/*!
 * Compare the incoming dc, timing, fb with the one that
 * the firmware has already programmed and see if seamless
 * is possible.
 *
 * @param dc
 * @param index 0 for primary and 1 for secondary
 * @param pt
 * @param pf
 * @param flags Not used right now
 *
 * @return TRUE if seamless is possible
 * @return FALSE if not possible
 */
int query_seamless(unsigned long dc,
		int index,
		igd_timing_info_t *pt,
		igd_framebuffer_info_t *pf,
		unsigned long flags)
{
	int ret = FALSE;
	igd_display_info_t *timing;
	igd_framebuffer_info_t *fb_info;
	unsigned long in_pitch;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Incoming dc = 0x%08lx", dc);

	/* Get the fw programmed DC from the inter-module data
	 * structure
     */
	mode_context->fw_info->fw_dc =
				 mode_context->context->mod_dispatch.dsp_fw_dc;

	EMGD_DEBUG("firmware dc = 0x%08lx",mode_context->fw_info->fw_dc );
	if(dc != mode_context->fw_info->fw_dc) {
		/*special case when seamless transition from fw clone to vext */
		if(!(IGD_DC_VEXT(dc) && IGD_DC_CLONE(mode_context->fw_info->fw_dc)))
		{
			/* DC doesn't match */
			return FALSE;
		}
		EMGD_DEBUG("past dc check");
	}

	/* Note: this test both overcomes a compiler warning, as well as a
	 * potential kernel Oops from chasing a NULL pointer:
	 */
	if ((pt == NULL) || (pf == NULL)) {
		return FALSE;
	}
	EMGD_DEBUG("Incoming Timing Width   = %hu", pt->width);
	EMGD_DEBUG("Incoming Timing height  = %hu", pt->height);
	EMGD_DEBUG("Incoming Timing Refresh = %hu", pt->refresh);

	/* Check pipe Timings */
	if(pt != NULL) {
		timing = &mode_context->fw_info->timing_arr[index];
		ret = FALSE;

		if( (timing->width  == pt->width) &&
			(timing->height == pt->height) )

			/* Have to build in some tolerance here because the fresh rate may
			 * not match exactly */
			if (abs(timing->refresh - pt->refresh) <= 2) {

			ret = TRUE;
		}
	}

	EMGD_DEBUG(" ");
	EMGD_DEBUG("Firmware Timing Width  = %hu", timing->width);
	EMGD_DEBUG("Fimrware Timing Height = %hu", timing->height);
	EMGD_DEBUG("Fimrware Timing Refesh = %hu", timing->refresh);
	EMGD_DEBUG("-------------------------");

	if(ret == FALSE) {
		EMGD_DEBUG("Incoming Timings and Firmware Timings Do NOT match!");
		EMGD_DEBUG("Seamless is NOT possible");
		EMGD_TRACE_EXIT;
		return ret;
	}


	EMGD_DEBUG("Incoming FB Width  = %u", pf->width);
	EMGD_DEBUG("Incoming FB Height = %u", pf->height);
	EMGD_DEBUG("Incoming FB Pitch  = %u", pf->screen_pitch);

	/* Check Plane information */
	if(pf != NULL) {
		fb_info = &mode_context->fw_info->fb_info[index];
		ret = TRUE;
		/* the incoming pitch=0, since it won't be filled until FB is allocated, calculating it now, so that
		 * it can be compared with the fw pitch */
		in_pitch = (IGD_PF_DEPTH(pf->pixel_format) * pf->width) >> 3;

		/* Pitch for both PLB and TNC requires 64-byte alignment */
		in_pitch = ALIGN(in_pitch, 64);

		if(fb_info->screen_pitch != in_pitch) {

			/* If width, height or pitch is different
			 * Don't have to turn-off pipe, just update
			 * the registers with the new values.
			 * Later we just call program_plane to update
			 * the registers.
			 */
			mode_context->fw_info->program_plane = 1;
		}

	}

	EMGD_DEBUG(" ");
	EMGD_DEBUG("Firmware FB Width  = %u", fb_info->width);
	EMGD_DEBUG("Firmware FB Height = %u", fb_info->height);
	EMGD_DEBUG("Firmware FB Pitch  = %u", fb_info->screen_pitch);
	EMGD_DEBUG("-----------------------");

	EMGD_DEBUG("value of ret = %d", ret);

	EMGD_TRACE_EXIT;
	return ret;
} /* end of query_seamless */


/*!
 * Takes inter-module function calls and calls mode-specific
 * kms_match_mode.
 *
 * @param emgd_encoder
 * @param fb_info
 * @param timing
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int igd_kms_match_mode(void *emgd_encoder,
    void *fb_info,
	igd_timing_info_h **timing)
{
	int ret;

	EMGD_TRACE_ENTER;

	ret = kms_match_mode((emgd_encoder_t *)emgd_encoder,
		(igd_framebuffer_info_t *)fb_info,
		(igd_timing_info_t **)timing);

	EMGD_TRACE_EXIT;
	return ret;
}

/*
 * Name: toggle_vblank_interrupts
 *
 * Description:
 * Requests/stops requesting for  vblank notification interrupts for ports 
 * where blit-batching has been enabled. Called from mode_init, mode_pwr and 
 * emgd_driver_pre_init to enable interrupts if needed, and from mode_pwr and
 * mode_shutdown to disable the interrupts when shutting down the driver. 
 *
 * @param status 0 to disable and 1 to enable interrupts
 *
 * @return None
 */
void toggle_vblank_interrupts(bool status) {

	unsigned long toggle_for;
	unsigned char * mmio;
	int i;

	mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
	for (i=0; i < IGD_MAX_PORTS; i++) {
		if(mode_context->batch_blits[i]) {
			toggle_for = 0;
			if (i == IGD_PORT_TYPE_SDVOB - 1) {
				toggle_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2);
			} else if (i == IGD_PORT_TYPE_LVDS - 1) {
				toggle_for = VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4);
			}

			if (toggle_for) {

				if (status) {
					mode_context->dispatch->full->request_vblanks(toggle_for, mmio);
				} else {
					mode_context->dispatch->full->end_request(toggle_for, mmio);
				}
			}
		}
	}

}

/* 
 * Notifies the user-space of a VBlank event by adding an event to the event
 * queue of the DRM master file-descriptor
 */
void notify_userspace_vblank(struct drm_device *dev, int port)
{

	drm_emgd_priv_t *devpriv = dev->dev_private;
	struct drm_pending_vblank_event *e = NULL;
	struct timeval now;
	unsigned long flags = 0;


	EMGD_TRACE_ENTER;

	if (!(devpriv->drm_master_fd)) {
		EMGD_DEBUG("DRM master file-descriptor not set - exiting");
		EMGD_TRACE_EXIT;
		return;
	}

	spin_lock_irqsave(&dev->event_lock,flags);

	/* Check if there is space for new event object */
	if (devpriv->drm_master_fd->event_space < sizeof(e->event)) {
		spin_unlock_irqrestore(&dev->event_lock, flags);
		EMGD_DEBUG("No space on file-descriptor event queue");
		EMGD_TRACE_EXIT;
		return;
	}
	devpriv->drm_master_fd->event_space -= sizeof(e->event);

	spin_unlock_irqrestore(&dev->event_lock, flags);

	e = OS_ALLOC(sizeof(struct drm_pending_vblank_event));
	if (e == NULL) {
		spin_lock_irqsave(&dev->event_lock, flags);
		devpriv->drm_master_fd->event_space += sizeof(e->event);
		spin_unlock_irqrestore(&dev->event_lock, flags);
		EMGD_TRACE_EXIT;
		return;
	}

	/* Setup event struct */
	OS_MEMSET(e, 0, sizeof(struct drm_pending_vblank_event));
	e->event.base.type = DRM_EVENT_VBLANK;
	e->event.base.length = sizeof(e->event);
	e->event.user_data = port;
	e->base.event = &e->event.base;
	e->base.file_priv = devpriv->drm_master_fd;
	e->base.destroy = (void (*)(struct drm_pending_event *))kfree;

	do_gettimeofday(&now);
	e->event.tv_sec = now.tv_sec;
	e->event.tv_usec = now.tv_usec;

	/* Add event to the event list */
	list_add_tail(&e->base.link, &e->base.file_priv->event_list);
	wake_up_interruptible(&e->base.file_priv->event_wait);

	EMGD_TRACE_EXIT;

}

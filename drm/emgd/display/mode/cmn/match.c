/*
 *-----------------------------------------------------------------------------
 * Filename: match.c
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
 *
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode


#define CURSOR_DEFAULT_WIDTH	64
#define CURSOR_DEFAULT_HEIGHT	64

#include <context.h>
#include <igd_init.h>
#include <io.h>
#include <memory.h>
#include <edid.h>
#include <pi.h>

#include <igd_mode.h>
#include <igd_errno.h>

#include <mode.h>
#include <config.h>

#include "match.h"


#define MATCH_MOD(x)  ((x>0)?x:-x)
#define MATCH_EXACT    0x01
#define MATCH_NATIVE   0x02
#define MATCH_CENTER   0x10
#define MATCH_FOR_VGA  0x20

extern igd_timing_info_t vga_timing_table[];
extern igd_timing_info_t crt_timing_table[];
static igd_timing_info_t scaled_timing[IGD_MAX_PIPES];


/*!
 * @addtogroup display_group
 * @{
 */

/* Local variables */
#ifndef CONFIG_MICRO
igd_cursor_info_t default_cursor = {
	CURSOR_DEFAULT_WIDTH,
	CURSOR_DEFAULT_HEIGHT,
	CONFIG_DEFAULT_PF,
	0, 0, 0, 0, 0, 0,
	0, 0, {0, 0, 0, 0}, IGD_CURSOR_ON, 0, 0, 0, 0
};

/*!
 *
 * @param cursor_info
 * @param display
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
int validate_cursor(igd_cursor_info_t *cursor_info,
	igd_display_context_t *display)
{
	unsigned long *list_pfs;
	igd_display_pipe_t *pipe = (igd_display_pipe_t *)(display->pipe);

	EMGD_TRACE_ENTER;
	if (pipe) {
		if (pipe->cursor) {
			list_pfs = pipe->cursor->pixel_formats;

			while (*list_pfs) {
				if (cursor_info->pixel_format == *list_pfs) {
					return 0;
				}
				list_pfs++;
			}
		}
	}

	EMGD_TRACE_EXIT;
	return -IGD_INVAL;
}
#endif

/*!
 *
 * @param timing
 * @param pt_info
 *
 * @return void
 */
static void fill_pt(
	igd_timing_info_t *timing,
	pigd_display_info_t pt_info)
{
	unsigned long flags;

	EMGD_DEBUG("fill_pt Entry");

	/* preserve existing pt_info flags */
	flags = pt_info->flags;

	/* Simply memcpy the structures and fix up the flags */
	OS_MEMCPY(pt_info, timing, sizeof(igd_timing_info_t));

	pt_info->flags |= flags;

	/* pt_info doesn't require a IGD_MODE_VESA flag, so clear IGD_MODE_VESA
	 * Setting this flag creates issues in match mode. */
	pt_info->flags &= ~IGD_MODE_VESA;
	return;
}



/*!
 *
 * @param emgd_encoder
 * @param timing_table
 * @param pt_info
 * @param type
 *
 * @return NULL on failure
 * @return timing on success
 */
igd_timing_info_t *kms_match_resolution(
		emgd_encoder_t *emgd_encoder,
		igd_timing_info_t *timing_table,
		igd_display_info_t *pt_info,
		int type)
{
	struct drm_device  *dev          = NULL;
	igd_timing_info_t  *timing       = NULL;
	igd_timing_info_t  *match        = NULL;
	igd_timing_info_t  *native_match = NULL;
	struct drm_encoder *encoder      = NULL;
	igd_display_port_t *port         = NULL;
	struct drm_crtc    *crtc         = NULL;
	emgd_crtc_t        *emgd_crtc    = NULL;
	igd_display_pipe_t *pipe         = NULL;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
		pt_info->width, pt_info->height, pt_info->refresh,
		pt_info->mode_number);

	encoder = &emgd_encoder->base;
	dev = encoder->dev;
	timing = timing_table;
	port = emgd_encoder->igd_port;
	match = NULL;

	/*
	 * Note on Native matching.
	 * The Ideal thing is for a fp_native_dtd to already be marked as such.
	 * If there is no native timing indicated then we must choose what is
	 * most likely correct.
	 * If the mode is not VGA then we should choose any DTD that closely
	 * matches the mode being set. Failing that we should choose any timing
	 * that closely matches the mode.
	 * If the mode is VGA then we should take the current mode as it is
	 * more likely correct.
	 */
	if(type == MATCH_NATIVE) {
		if(port->fp_native_dtd) {
			EMGD_DEBUG("Returning quick with a native match");

			EMGD_DEBUG("NATIVE Width=%d, height=%d, refresh=%d mode_num=0x%x",
				port->fp_native_dtd->width, port->fp_native_dtd->height,
				port->fp_native_dtd->refresh, port->fp_native_dtd->mode_number);

			return port->fp_native_dtd;
		}
		if((pt_info->flags & IGD_MODE_VESA) &&
			(pt_info->mode_number <= 0x13)) {

			list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
				if (crtc == encoder->crtc) {
					emgd_crtc = container_of(crtc, emgd_crtc_t, base);
					pipe = emgd_crtc->igd_pipe;
					if(pipe->timing) {
						native_match = pipe->timing;
					}
				}
			}
		}
	}

	while (timing->width != IGD_TIMING_TABLE_END) {
		if(!(timing->mode_info_flags & IGD_MODE_SUPPORTED)) {
			timing++;
			continue;
		}

		if(type == MATCH_NATIVE) {
			if(timing->mode_info_flags & IGD_MODE_DTD_FP_NATIVE) {
				port->fp_native_dtd = timing;
				return timing;
			}

			if(port->fp_info) {
				/*
				 * We may have only fp_width and fp_height which is really
				 * not enough information to be useful. If we find a
				 * matching width and height we'll keep the first one while
				 * still hoping to find an actual native mode later.
				 */
				if(!match &&
					(port->fp_info->fp_width ==
						(unsigned long)timing->width) &&
					(port->fp_info->fp_height ==
						(unsigned long)timing->height)) {
					match = timing;
				}
			} else {
				/*
				 * Keep a match because in the event that we never find a
				 * native DTD then we will just take the exact match.
				 */
				if(!match &&
					(timing->width == pt_info->width) &&
					(timing->height == pt_info->height) &&
					(timing->refresh == pt_info->refresh)) {
					match = timing;
				}
			}

			/*
			 * If it is a DTD then keep it only if it is better than any
			 * found before.
			 */
			if(timing->mode_info_flags & IGD_MODE_DTD_USER) {
				if(native_match) {
					if(MATCH_MOD((int)(pt_info->width*pt_info->height) -
							(native_match->width * native_match->height)) >
						MATCH_MOD((int)(pt_info->width*pt_info->height) -
							(timing->width*timing->height))) {
						native_match = timing;
					}
				} else {
					native_match = timing;
				}
			}
		} else if (type == MATCH_EXACT) {
			/*
			 * Looking for an exact match. For VGA/VESA it must match
			 * mode number. Otherwise it must match width, height, refresh
			 * etc.
			 */
			if(pt_info->flags & IGD_MODE_VESA) {
				/* ((timing->mode_info_flags & IGD_MODE_VESA)) */
				if((pt_info->mode_number == timing->mode_number) &&
					(!pt_info->refresh ||
						(pt_info->refresh == timing->refresh))) {
					match = timing;
					break;
				}
			} else {
				/* If exact match found, then break the loop */
				if((timing->width == pt_info->width) &&
					(timing->height == pt_info->height) &&
					(timing->refresh == pt_info->refresh) &&
					(
						(timing->mode_info_flags &
							(IGD_SCAN_INTERLACE|IGD_PIXEL_DOUBLE|
								IGD_LINE_DOUBLE)) ==
						(pt_info->flags &
							(IGD_SCAN_INTERLACE|IGD_PIXEL_DOUBLE|
								IGD_LINE_DOUBLE)))) {
					match = timing;

					/* If exact match found, then break the loop */
					if ((timing->mode_info_flags & PD_MODE_DTD_USER) ||
						(timing->mode_info_flags & PD_MODE_DTD)) {
						break;
					}
				}
			}
		}


		/* Center needs only to be bigger. Aspect ratio doesn't matter. */
		/*
		 * Note: The timings have to be big enough to fit the pt_info
		 * including any pixel double flags. VGA modes will sometimes be
		 * pixel doubled and need to be centered in a pipe that is double
		 * in size.
		 *
		 * Note2: 720x400 VGA modes can be centered in 640x480 with a
		 * special hardware config that drops every 9th pixel. Only do
		 * this when requested.
		 */
		else if(type & MATCH_CENTER) {
			unsigned short eff_width = pt_info->width;
			unsigned short eff_height = pt_info->height;

			if(type & MATCH_FOR_VGA) {
				/*
				 * 720x400 is a magic mode that means all VGA modes are supported
				 * always use that mode for centering if found.
				 */
				if((timing->width == 720) && (timing->height == 400)) {
					EMGD_DEBUG("Returning with a magic VGA mode");
					return timing;
				}
				if(pt_info->flags & IGD_PIXEL_DOUBLE) {
					eff_width *= 2;
				}
				if(pt_info->flags & IGD_LINE_DOUBLE) {
					eff_height *= 2;
				}
				if((eff_width == 720) &&
					(port->port_features & IGD_VGA_COMPRESS)) {
					eff_width = 640;
				}
			}

			if((timing->width >= eff_width) &&
				(timing->height >= eff_height) &&
				(timing->mode_info_flags & IGD_SCAN_INTERLACE) ==
				(pt_info->flags & IGD_SCAN_INTERLACE)) {
				if(match) {
					/* Check for tighter fit */
					if((match->width > timing->width) ||
						(match->height > timing->height)) {
						match = timing;
					}
					/* Try to match refreshrate as well */
					if((match->width == timing->width) &&
					   (match->height == timing->height) &&
					   (pt_info->refresh == timing->refresh)){
						match = timing;
					}
				} else {
					match = timing;
				}
			}
		}
		timing++;
	}

	if(native_match) {
		EMGD_DEBUG("Returning with a native match");
		EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
			native_match->width, native_match->height, native_match->refresh,
			native_match->mode_number);
		return native_match;
	}
	if (!match) {
		EMGD_DEBUG("Returning with NO match");
		return NULL;
	}

	EMGD_DEBUG("Returning with a match");
	EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
		match->width, match->height, match->refresh, match->mode_number);
	return match;
} /* end match_resolution */



/*!
 * Match the fb and pt structures to a Mode Structure from the table.
 * When a mode is found update the input structures to reflect the
 * values found.
 *
 * If the frambuffer is smaller than the timings requested, then we
 * modify the timings so that the framebuffer will be centered.  That is,
 * unless we are asked to upscale the framebuffer to fit the timings requested.
 *
 * In the case of LVDS both centering and scaling can happen. If the mode
 * is in the list it will be scaled to the Native Timings. If the mode
 * is not in the list (common or VGA) it will be centered in the next larger
 * supported mode and then scaled to the native timings.
 *
 * Centering is always indicated by returning the timings that should be
 * programmed to the pipe. The timings will then have their extension pointer
 * set to point to the centered timings. For centering with scaling the
 * first extension pointer will contain the scalable timings and the
 * second will contain the centering timings. The static "scaled_timings"
 * data structure will be used when the scaled timings need to be
 * created on the fly due to a framebuffer that is smaller than the
 * timings.
 *
 * FIXME: There is a lot of mentioning of VGA modes in this function from
 * an earlier implementation of this feature.  The VGA-related code may
 * not be relevant anymore if we are not building the VBIOS.
 *
 * @param emgd_encoder [IN]  Encoder expected to have a mode change
 * @param fb_info      [IN]  Dimension of the FB to be displayed
 * @param timing       [OUT] Best matched timing
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int kms_match_mode (
	emgd_encoder_t *emgd_encoder,
	igd_framebuffer_info_t *fb_info,
	igd_timing_info_t **timing)
{
	struct drm_device  *dev;
	struct drm_encoder *encoder;
	igd_display_port_t *port;
	igd_timing_info_t  *timing_table;
	igd_timing_info_t  *exact_timing  = NULL;
	igd_timing_info_t  *pipe_timing   = NULL;
	igd_timing_info_t  *user_timing   = NULL;
	igd_timing_info_t  *native_timing = NULL;
	igd_timing_info_t  *vga_timing    = NULL;
	igd_timing_info_t  *vesa_timing   = NULL;
	igd_display_info_t *pt_info       = NULL;
	struct drm_crtc    *crtc          = NULL;
	emgd_crtc_t        *emgd_crtc     = NULL;
	igd_display_pipe_t *pipe          = NULL;
	short               cntr_dff_w    = 0;
	short               cntr_dff_h    = 0;
	unsigned long       upscale       = 0;

	EMGD_TRACE_ENTER;


	encoder      = &emgd_encoder->base;
	dev          = encoder->dev;
	port         = emgd_encoder->igd_port;
	pt_info      = port->pt_info;
	timing_table = port->timing_table;

	if(!pt_info) {
		EMGD_ERROR("NULL Port info detected, returning");
		return -IGD_ERROR_INVAL;
	}

	/* Check for default case */
	if (!(pt_info->flags & IGD_MODE_VESA) &&
		(pt_info->width == 0) && (pt_info->height == 0)) {
		EMGD_DEBUG("Display Info width, height are zero, using default case");
		pt_info->width  = CONFIG_DEFAULT_WIDTH;
		pt_info->height = CONFIG_DEFAULT_HEIGHT;
	}

	EMGD_DEBUG("Checking for exact mode match");
	exact_timing = kms_match_resolution(emgd_encoder, timing_table, pt_info,
		MATCH_EXACT);

	/*
	 * At this point we have one of these cases:
	 *  1) Found an exact match, VGA, VESA or other.
	 *    -> Go check for FB centering and finish up.
	 *  2) Found nothing
	 *    -> Check for VGA/VESA mode to center.
	 *    -> Check common modes.
	 */
	if(exact_timing) {
		pipe_timing = exact_timing;
		user_timing = exact_timing;
		pipe_timing->extn_ptr = NULL;
	} else {
		/* No match found? Is it VGA? */
		if( (pt_info->flags & IGD_MODE_VESA) &&
			(pt_info->mode_number < 0x1D)    ){
			EMGD_DEBUG("Checking for exact match in VGA table");
			/* this only happens if it was a VGA mode number */
			pt_info->refresh = 0;
			vga_timing = kms_match_resolution(emgd_encoder, vga_timing_table,
				pt_info, MATCH_EXACT);

			if(!vga_timing) {
				return -IGD_ERROR_INVAL;
			}

			vga_timing->extn_ptr = NULL;
			/* We got something sane that needs to be centered */
			user_timing = vga_timing;
			fill_pt(vga_timing,pt_info);

			/* continue at the bottom where we have
			 * pipe_timing = NULL, so we will look
			 * for centered timings for pt_info and
			 * use cmn_vga_timings to tell kms_match_resolution
			 * to take into account special VGA mode
			 * centering regulations
			 */
		}
	}

	/* Find UPSCALING attr value
	 * this PI func will not modify value of upscale if attr does not exist */
	pi_pd_find_attr_and_value(port,
			PD_ATTR_ID_PANEL_FIT,
			0,/*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);


	if(!pipe_timing){
		/* At this point, one of 2 things has happenned:
		 *      - we have a mode request that we could not match exactly.
		 *        and it WASNT a VESA_MODE number request.
		 *      - we have a request based on VESA_MODE number (maybe from
		 *        VBIOS IAL) and we could not get a exact match from the
		 *        port_timing_table, but we did get a match from the vga-
		 *        timing_table.
		 * In this case, there is one thing to do - MATCH_CENTER. Match
		 * resolution will handle it this way:
		 *      - if its VESA MODE number based, we only need to get
		 *        the best (tightest) match if its VGA OR DONT match
		 *        if its one of those magic timings
		 *      - Else, we need to get the best (tightest) match, AND
		 *        we need to center requested timings in that tightest fitting
		 *        timing. But wait! This could mean if the requested pt_info
		 *        is bigger than anything in the port timing table, we have
		 *        no choice but to fail.
		 */
		unsigned char match_type = MATCH_CENTER;

		EMGD_DEBUG("Checking for a safe centered match");
		if(vga_timing) {
			match_type |= MATCH_FOR_VGA;
		} else if(pt_info->flags & IGD_MODE_VESA) {
			/* if a vesa mode number was requested...
			 * and we are centering that mode, we
			 * need to get the common mode fb size
			 * in case we need it later for VBIOS
			 * which doesnt populate the FBInfo
			 */
			vesa_timing = kms_match_resolution(emgd_encoder, crt_timing_table,
				pt_info, MATCH_EXACT);
		}

		if (upscale && vga_timing) {
			/* If port supports upscaling and match is called for VGA,
			 * then center vga mode resolution directly in the native mode
			 * instead of centering VGA in another resolution */
			pipe_timing = vga_timing;
		} else {
			pipe_timing = kms_match_resolution(emgd_encoder, timing_table,
							pt_info, match_type);
			/* This can happen if there is a spurious pt_info from IAL */
			if (!pipe_timing) {
				return -IGD_ERROR_INVAL;
			}
			pipe_timing->extn_ptr = vga_timing;
			/* for the case of non VGA mode call,
			 * at this point, vga_timing is NULL
			 */
		}

		if(!vga_timing) {
			user_timing = pipe_timing;
		}
	}

	/*
	 * At this point pipe_timing is what we are going to program the
	 * pipe to roughly speaking. If there is a common timing then we
	 * want it centered in the pipe_timing.
	 *
	 * If the framebuffer is smaller than the timings then we need to
	 * generate a centered set of timings by copying the pipe timings
	 * and shifting them a bit.
	 *
	 * If fb width and height are zero just assume that we want it to
	 * match the timings and make up a pixel format. This is mostly because
	 * VGA/VESA modes will just be set by number. We don't know their size
	 * until we look up the number.
	 */
	if(fb_info) {
		/*
		 * fb_info is sometimes NULL when just testing something.
		 */
		if(!fb_info->pixel_format) {
			/* Ugly VGA modes, it doesn't matter */
			fb_info->pixel_format = IGD_PF_ARGB8_INDEXED;
		}
		if(!fb_info->width) {
			if(vga_timing) {
				fb_info->width = vga_timing->width;
				fb_info->height = vga_timing->height;
			} else {
				if(!vesa_timing){
					vesa_timing = pipe_timing;
					/* in case vesa_timing is false set it to
					 * pipe_timing so we dont need to check for
					 * validity later, when increasing fb size for
					 * VBIOS in clone mode (see 18 lines below)
					 */
				}
				fb_info->width = vesa_timing->width;
				fb_info->height = vesa_timing->height;
			}
		}

		/*
		 * VGA common timings are centered in pipe timings by hardware.
		 * Otherwise we need to adjust the timings when centering is
		 * needed.
		 */
		if (!vga_timing) {
			/*
			 * For VBIOS clone modes the FB should be the biggest mode
			 * if this is the second match we may need to update the fb
			 * data structure.
			 */
			if(fb_info->flags & IGD_VBIOS_FB) {
				if ((fb_info->width < vesa_timing->width) ||
					(fb_info->height < vesa_timing->height)) {
					fb_info->width = vesa_timing->width;
					fb_info->height = vesa_timing->height;
				}
			}


			/* Do centering if fb is smaller than timing except on TV */
			if ((fb_info->width < pipe_timing->width) ||
				(fb_info->height < pipe_timing->height)) {
				unsigned short temp_width = pipe_timing->width;
				unsigned short temp_height = pipe_timing->height;
				/* Normally, we should NOT be in here. All IALs only
				 * are supposed to request for timings that ARE surely
				 * supported by the HAL,... i.e. query the list of
				 * supported timings by the port first!
				 *
				 * The exception would be if the IAL is purposely
				 * asking for CENTERING!!! (pt_info's that were not
				 * part of the supported mode list). This could indicate an
				 * error or an explicit request for VESA centering!.
				 */

				/* let's use these 2 variables as flags... and do the
				 * actual "centering" of the timings later since we do
				 * also need to acomodate native timings as well
				 */
				/* NOTE: we could never be in here in fb_info was NULL */
				cntr_dff_w = (pipe_timing->width  - fb_info->width)  / 2;
				cntr_dff_h = (pipe_timing->height - fb_info->height) / 2;

				/* Dont forget to use a different storage sice we dont
				 * want to change the original (and to be used later)
				 * ports mode list timings
				 */
				list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
					if (crtc == encoder->crtc) {
						emgd_crtc = container_of(crtc, emgd_crtc_t, base);
						pipe = emgd_crtc->igd_pipe;

						OS_MEMCPY(&scaled_timing[(pipe->pipe_num)],
							pipe_timing, sizeof(igd_timing_info_t));
						pipe_timing = &scaled_timing[(pipe->pipe_num)];
					}
				}

				if(port->pd_type != PD_DISPLAY_TVOUT ) {
					/* TV display don't like changed pipe actives,
					 * Updating syncs work for TV centering */
					if (fb_info->width < temp_width) {
						pipe_timing->width = (unsigned short)fb_info->width;
						pipe_timing->hblank_start -= cntr_dff_w;
						pipe_timing->hblank_end   -= cntr_dff_w;
					}

					if (fb_info->height < temp_height) {
						pipe_timing->height = (unsigned short)fb_info->height;
						pipe_timing->vblank_start -= cntr_dff_h;
						pipe_timing->vblank_end   -= cntr_dff_h;
					}
				}

				if (fb_info->width < temp_width) {
					pipe_timing->hsync_start -= cntr_dff_w;
					pipe_timing->hsync_end   -= cntr_dff_w;
				}

				if (fb_info->height < temp_height) {
					pipe_timing->vsync_start -= cntr_dff_h;
					pipe_timing->vsync_end   -= cntr_dff_h;
				}
			}
		}
	}

	if(upscale) {
		/* Get the native timings */
		EMGD_DEBUG("Checking for Native LVDS match for scaling");
		native_timing = kms_match_resolution(emgd_encoder, timing_table,
							pt_info, MATCH_NATIVE);
		if(native_timing && (native_timing != pipe_timing)) {
			native_timing->extn_ptr = pipe_timing;
			pipe_timing = native_timing;
		}
	}

	/*
	 * Match mode returns as follows:
	 * In case of VGA setmode:
	 * 1) We will end up with either:
	 *   magic->vga   ---   For displays supports native VGA
	 *      or
	 *   native->vga  ---   Upscaling displays
	 *      or
	 *   pipe->vga    ---   For other displays
	 *
	 * 2) In case of regular setmode:
	 *   pipe         ---   For regular displays
	 *      or
	 *   native->vesa ---   Upscaling displays
	 *
	 *   Note: 1) Here "pipe" can be munged if centering is required.
	 *         2) "vesa" is the requested mode, native is the native timing
	 *            of the display.
	 */

	/*
	 * Update Input Structures with values found
	 * Note: This might not be what is going to be programmed. It is what
	 * the user thinks they set. Scaling or centering could have altered
	 * that.
	 */
	fill_pt(user_timing, pt_info);
	*timing = pipe_timing;
	EMGD_TRACE_EXIT;

	return 0;
}


/*!
 *
 * @param display
 * @param timing_table
 * @param pt_info
 * @param type
 *
 * @return NULL on failure
 * @return timing on success
 */
static igd_timing_info_t *match_resolution(
		igd_display_context_t *display,
		igd_timing_info_t *timing_table,
		igd_display_info_t *pt_info,
		int type)
{
	igd_timing_info_t *timing;
	igd_timing_info_t *match;
	igd_timing_info_t *native_match = NULL;
	igd_display_port_t *port;

	EMGD_DEBUG("Enter match_resolution");

	EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
		pt_info->width, pt_info->height, pt_info->refresh,
		pt_info->mode_number);

	timing = timing_table;
	match = NULL;
	port = PORT_OWNER(display);

	/*
	 * Note on Native matching.
	 * The Ideal thing is for a fp_native_dtd to already be marked as such.
	 * If there is no native timing indicated then we must choose what is
	 * most likely correct.
	 * If the mode is not VGA then we should choose any DTD that closely
	 * matches the mode being set. Failing that we should choose any timing
	 * that closely matches the mode.
	 * If the mode is VGA then we should take the current mode as it is
	 * more likely correct.
	 */
	if(type == MATCH_NATIVE) {
		if(port->fp_native_dtd) {
			EMGD_DEBUG("Returning quick with a native match");

			EMGD_DEBUG("NATIVE Width=%d, height=%d, refresh=%d mode_num=0x%x",
				port->fp_native_dtd->width, port->fp_native_dtd->height,
				port->fp_native_dtd->refresh, port->fp_native_dtd->mode_number);

			return port->fp_native_dtd;
		}
		if((pt_info->flags & IGD_MODE_VESA) &&
			(pt_info->mode_number <= 0x13)) {
			if(PIPE(display)->timing) {
				native_match = PIPE(display)->timing;
			}
		}
	}

	while (timing->width != IGD_TIMING_TABLE_END) {
		if(!(timing->mode_info_flags & IGD_MODE_SUPPORTED)) {
			timing++;
			continue;
		}

		if(type == MATCH_NATIVE) {
			if(timing->mode_info_flags & IGD_MODE_DTD_FP_NATIVE) {
				port->fp_native_dtd = timing;
				return timing;
			}

			if(port->fp_info) {
				/*
				 * We may have only fp_width and fp_height which is really
				 * not enough information to be useful. If we find a
				 * matching width and height we'll keep the first one while
				 * still hoping to find an actual native mode later.
				 */
				if(!match &&
					(port->fp_info->fp_width ==
						(unsigned long)timing->width) &&
					(port->fp_info->fp_height ==
						(unsigned long)timing->height)) {
					match = timing;
				}
			} else {
				/*
				 * Keep a match because in the event that we never find a
				 * native DTD then we will just take the exact match.
				 */
				if(!match &&
					(timing->width == pt_info->width) &&
					(timing->height == pt_info->height) &&
					(timing->refresh == pt_info->refresh)) {
					match = timing;
				}
			}

			/*
			 * If it is a DTD then keep it only if it is better than any
			 * found before.
			 */
			if(timing->mode_info_flags & IGD_MODE_DTD_USER) {
				if(native_match) {
					if(MATCH_MOD((int)(pt_info->width*pt_info->height) -
							(native_match->width * native_match->height)) >
						MATCH_MOD((int)(pt_info->width*pt_info->height) -
							(timing->width*timing->height))) {
						native_match = timing;
					}
				} else {
					native_match = timing;
				}
			}
		} else if (type == MATCH_EXACT) {
			/*
			 * Looking for an exact match. For VGA/VESA it must match
			 * mode number. Otherwise it must match width, height, refresh
			 * etc.
			 */
			if(pt_info->flags & IGD_MODE_VESA) {
				/* ((timing->mode_info_flags & IGD_MODE_VESA)) */
				if((pt_info->mode_number == timing->mode_number) &&
					(!pt_info->refresh ||
						(pt_info->refresh == timing->refresh))) {
					match = timing;
					break;
				}
			} else {
				/* If exact match found, then break the loop */
				if((timing->width == pt_info->width) &&
					(timing->height == pt_info->height) &&
					(timing->refresh == pt_info->refresh) &&
					(
						(timing->mode_info_flags &
							(IGD_SCAN_INTERLACE|IGD_PIXEL_DOUBLE|
								IGD_LINE_DOUBLE)) ==
						(pt_info->flags &
							(IGD_SCAN_INTERLACE|IGD_PIXEL_DOUBLE|
								IGD_LINE_DOUBLE)))) {
					match = timing;

					/* If exact match found, then break the loop */
					if ((timing->mode_info_flags & PD_MODE_DTD_USER) ||
						(timing->mode_info_flags & PD_MODE_DTD)) {
						break;
					}
				}
			}
		}


		/* Center needs only to be bigger. Aspect ratio doesn't matter. */
		/*
		 * Note: The timings have to be big enough to fit the pt_info
		 * including any pixel double flags. VGA modes will sometimes be
		 * pixel doubled and need to be centered in a pipe that is double
		 * in size.
		 *
		 * Note2: 720x400 VGA modes can be centered in 640x480 with a
		 * special hardware config that drops every 9th pixel. Only do
		 * this when requested.
		 */
		else if(type & MATCH_CENTER) {
			unsigned short eff_width = pt_info->width;
			unsigned short eff_height = pt_info->height;

			if(type & MATCH_FOR_VGA) {
				/*
				 * 720x400 is a magic mode that means all VGA modes are supported
				 * always use that mode for centering if found.
				 */
				if((timing->width == 720) && (timing->height == 400)) {
					EMGD_DEBUG("Returning with a magic VGA mode");
					return timing;
				}
				if(pt_info->flags & IGD_PIXEL_DOUBLE) {
					eff_width *= 2;
				}
				if(pt_info->flags & IGD_LINE_DOUBLE) {
					eff_height *= 2;
				}
				if((eff_width == 720) &&
					(port->port_features & IGD_VGA_COMPRESS)) {
					eff_width = 640;
				}
			}

			if((timing->width >= eff_width) &&
				(timing->height >= eff_height) &&
				(timing->mode_info_flags & IGD_SCAN_INTERLACE) ==
				(pt_info->flags & IGD_SCAN_INTERLACE)) {
				if(match) {
					/* Check for tighter fit */
					if((match->width > timing->width) ||
						(match->height > timing->height)) {
						match = timing;
					}
					/* Try to match refreshrate as well */
					if((match->width == timing->width) &&
					   (match->height == timing->height) &&
					   (pt_info->refresh == timing->refresh)){
						match = timing;
					}
				} else {
					match = timing;
				}
			}
		}
		timing++;
	}

	if(native_match) {
		EMGD_DEBUG("Returning with a native match");
		EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
			native_match->width, native_match->height, native_match->refresh,
			native_match->mode_number);
		return native_match;
	}
	if (!match) {
		EMGD_DEBUG("Returning with NO match");
		return NULL;
	}

	EMGD_DEBUG("Returning with a match");
	EMGD_DEBUG("Width=%d, height=%d, refresh=%d mode_number=0x%x",
		match->width, match->height, match->refresh, match->mode_number);
	return match;
} /* end match_resolution */



/*!
 * Match the fb and pt structures to a Mode Structure from the table.
 * When a mode is found update the input structures to reflect the
 * values found.
 *
 * Notes:
 *  Match mode has several options for what it can do. Foremost it should
 * attempt to find a mode matching the requested one from the timing table
 * provided. If the mode requested is not in the list this means one of
 * two things.
 *   1) The IAL is calling without checking modes. It is just passing down
 *  something that a user asked for. This is ok but we need to be safe so
 *  we return the next smaller mode with the same aspect ratio.
 *
 *   2) The IAL is requesting a very common "required" mode even though the
 *  port doesn't support it. In this case it should be in the static common
 *  modes table and can be centered in the next larger timings in the
 *  mode table.
 *
 * If the Frambuffer is smaller than the timings requested a fake set of
 * centered timings is returned to program the pipe.
 *
 * In the case of VGA modes. If the mode is in the mode table everything is
 * fine and we just return that. If it is not in the table we find the next
 * larger suitable mode and prepare to center in that mode. Using the static
 * timings from the VGA table as the VGA mode. We do not need to generate
 * a fake set of timings because VGA will center itself automatically in
 * hardware.
 *
 * In the case of LVDS both centering and scaling can happen. If the mode
 * is in the list it will be scaled to the Native Timings. If the mode
 * is not in the list (common or VGA) it will be centered in the next larger
 * supported mode and then scaled to the native timings.
 *
 * Centering is always indicated by returning the timings that should be
 * programmed to the pipe. The timings will then have their extension pointer
 * set to point to the centered timings. For centering with scaling the
 * first extension pointer will contain the scalable timings and the
 * second will contain the centering timings. The static "scaled_timings"
 * data structure will be used when the scaled timings need to be
 * created on the fly due to a Framebuffer that is smaller than the
 * timings.
 *
 * @param display
 * @param timing_table
 * @param fb_info
 * @param pt_info
 * @param timing
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int match_mode (
	igd_display_context_t *display,
	igd_timing_info_t *timing_table,
	igd_framebuffer_info_t *fb_info,
	igd_display_info_t *pt_info,
	igd_timing_info_t **timing)
{
	igd_timing_info_t *exact_timing = NULL;
	igd_timing_info_t *pipe_timing = NULL;
	igd_timing_info_t *user_timing = NULL;
	igd_timing_info_t *native_timing = NULL;
	igd_timing_info_t *vga_timing = NULL;
	igd_timing_info_t *vesa_timing = NULL;
	short cntr_dff_w = 0;
	short cntr_dff_h = 0;
	unsigned long upscale = 0;

	EMGD_DEBUG("Enter Match Mode");

	if(!pt_info) {
		EMGD_ERROR("NULL Port info detected, returning");
		return -IGD_ERROR_INVAL;
	}

	/* Check for default case */
	if (!(pt_info->flags & IGD_MODE_VESA) &&
		(pt_info->width == 0) && (pt_info->height == 0)) {
		EMGD_DEBUG("Display Info width, height are zero, using default case");
		pt_info->width = CONFIG_DEFAULT_WIDTH;
		pt_info->height = CONFIG_DEFAULT_HEIGHT;
	}

	EMGD_DEBUG("Checking for exact mode match");
	exact_timing = match_resolution(display, timing_table, pt_info,
		MATCH_EXACT);
	/*
	 * At this point we have one of these cases:
	 *  1) Found an exact match, VGA, VESA or other.
	 *    -> Go check for FB centering and finish up.
	 *  2) Found nothing
	 *    -> Check for VGA/VESA mode to center.
	 *    -> Check common modes.
	 */
	if(exact_timing) {
		pipe_timing = exact_timing;
		user_timing = exact_timing;
		pipe_timing->extn_ptr = NULL;
	} else {
		/* No match found? Is it VGA? */
		if( (pt_info->flags & IGD_MODE_VESA) &&
			(pt_info->mode_number < 0x1D)    ){
			EMGD_DEBUG("Checking for exact match in VGA table");
			/* this only happens if it was a VGA mode number */
			pt_info->refresh = 0;
			vga_timing = match_resolution(display, vga_timing_table,
				pt_info, MATCH_EXACT);

			if(!vga_timing) {
				return -IGD_ERROR_INVAL;
			}

			vga_timing->extn_ptr = NULL;
			/* We got something sane that needs to be centered */
			user_timing = vga_timing;
			fill_pt(vga_timing,pt_info);

			/* continue at the bottom where we have
			 * pipe_timing = NULL, so we will look
			 * for centered timings for pt_info and
			 * use cmn_vga_timings to tell match_resolution
			 * to take into account special VGA mode
			 * centering regulations
			 */
		}
	}

	/* Find UPSCALING attr value*/
	pi_pd_find_attr_and_value(PORT_OWNER(display),
			PD_ATTR_ID_PANEL_FIT,
			0,/*no PD_FLAG for UPSCALING */
			NULL, /* dont need the attr ptr*/
			&upscale);
	/* this PI func will not modify value of upscale if attr does not exist */

	if(!pipe_timing){
		/* At this point, one of 2 things has happenned:
		 *      - we have a mode request that we could not match exactly.
		 *        and it WASNT a VESA_MODE number request.
		 *      - we have a request based on VESA_MODE number (maybe from
		 *        VBIOS IAL) and we could not get a exact match from the
		 *        port_timing_table, but we did get a match from the vga-
		 *        timing_table.
		 * In this case, there is one thing to do - MATCH_CENTER. Match
		 * resolution will handle it this way:
		 *      - if its VESA MODE number based, we only need to get
		 *        the best (tightest) match if its VGA OR DONT match
		 *        if its one of those magic timings
		 *      - Else, we need to get the best (tightest) match, AND
		 *        we need to center requested timings in that tightest fitting
		 *        timing. But wait! This could mean if the requested pt_info
		 *        is bigger than anything in the port timing table, we have
		 *        no choice but to fail.
		 */
		unsigned char match_type = MATCH_CENTER;

		EMGD_DEBUG("Checking for a safe centered match");
		if(vga_timing) {
			match_type |= MATCH_FOR_VGA;
		} else if(pt_info->flags & IGD_MODE_VESA) {
			/* if a vesa mode number was requested...
			 * and we are centering that mode, we
			 * need to get the common mode fb size
			 * in case we need it later for VBIOS
			 * which doesnt populate the FBInfo
			 */
			vesa_timing = match_resolution(display, crt_timing_table,
				pt_info, MATCH_EXACT);
		}

		if (upscale && vga_timing) {
			/* If port supports upscaling and match is called for VGA,
			 * then center vga mode resolution directly in the native mode
			 * instead of centering VGA in another resolution */
			pipe_timing = vga_timing;
		} else {
			pipe_timing = match_resolution(display, timing_table, pt_info,
				match_type);
			/* This can happen if there is a spurious pt_info from IAL */
			if (!pipe_timing) {
				return -IGD_ERROR_INVAL;
			}
			pipe_timing->extn_ptr = vga_timing;
			/* for the case of non VGA mode call,
			 * at this point, vga_timing is NULL
			 */
		}

		if(!vga_timing) {
			user_timing = pipe_timing;
		}
	}

	/*
	 * At this point pipe_timing is what we are going to program the
	 * pipe to roughly speaking. If there is a common timing then we
	 * want it centered in the pipe_timing.
	 *
	 * If the framebuffer is smaller than the timings then we need to
	 * generate a centered set of timings by copying the pipe timings
	 * and shifting them a bit.
	 *
	 * If fb width and height are zero just assume that we want it to
	 * match the timings and make up a pixel format. This is mostly because
	 * VGA/VESA modes will just be set by number. We don't know their size
	 * until we look up the number.
	 */
	if(fb_info) {
		/*
		 * fb_info is sometimes NULL when just testing something.
		 */
		if(!fb_info->pixel_format) {
			/* Ugly VGA modes, it doesn't matter */
			fb_info->pixel_format = IGD_PF_ARGB8_INDEXED;
		}
		if(!fb_info->width) {
			if(vga_timing) {
				fb_info->width = vga_timing->width;
				fb_info->height = vga_timing->height;
			} else {
				if(!vesa_timing){
					vesa_timing = pipe_timing;
					/* in case vesa_timing is false set it to
					 * pipe_timing so we dont need to check for
					 * validity later, when increasing fb size for
					 * VBIOS in clone mode (see 18 lines below)
					 */
				}
				fb_info->width = vesa_timing->width;
				fb_info->height = vesa_timing->height;
			}
		}

		/*
		 * VGA common timings are centered in pipe timings by hardware.
		 * Otherwise we need to adjust the timings when centering is
		 * needed.
		 */
		if (!vga_timing) {
			/*
			 * For VBIOS clone modes the FB should be the biggest mode
			 * if this is the second match we may need to update the fb
			 * data structure.
			 */
			if(fb_info->flags & IGD_VBIOS_FB) {
				if ((fb_info->width < vesa_timing->width) ||
					(fb_info->height < vesa_timing->height)) {
					fb_info->width = vesa_timing->width;
					fb_info->height = vesa_timing->height;
				}
			}


			/* Do centering if fb is smaller than timing except on TV */
			if ((fb_info->width < pipe_timing->width) ||
				(fb_info->height < pipe_timing->height)) {
				unsigned short temp_width = pipe_timing->width;
				unsigned short temp_height = pipe_timing->height;
				/* Normally, we should NOT be in here. All IALs only
				 * are supposed to request for timings that ARE surely
				 * supported by the HAL,... i.e. query the list of
				 * supported timings by the port first!
				 *
				 * The exception would be if the IAL is purposely
				 * asking for CENTERING!!! (pt_info's that were not
				 * part of the supported mode list). This could indicate an
				 * error or an explicit request for VESA centering!.
				 */

				/* let's use these 2 variables as flags... and do the
				 * actual "centering" of the timings later since we do
				 * also need to acomodate native timings as well
				 */
				/* NOTE: we could never be in here in fb_info was NULL */
				cntr_dff_w = (pipe_timing->width - fb_info->width) / 2;
				cntr_dff_h = (pipe_timing->height - fb_info->height) / 2;

				/* Dont forget to use a different storage sice we dont
				 * want to change the original (and to be used later)
				 * ports mode list timings
				 */
				OS_MEMCPY(&scaled_timing[(PIPE(display)->pipe_num)],
					pipe_timing,
					sizeof(igd_timing_info_t));

				pipe_timing = &scaled_timing[(PIPE(display)->pipe_num)];

				if(PORT_OWNER(display)->pd_type != PD_DISPLAY_TVOUT ) {
					/* TV display don't like changed pipe actives,
					 * Updating syncs work for TV centering */
					if (fb_info->width < temp_width) {
						pipe_timing->width = (unsigned short)fb_info->width;
						pipe_timing->hblank_start -= cntr_dff_w;
						pipe_timing->hblank_end -= cntr_dff_w;
					}

					if (fb_info->height < temp_height) {
						pipe_timing->height = (unsigned short)fb_info->height;
						pipe_timing->vblank_start -= cntr_dff_h;
						pipe_timing->vblank_end -= cntr_dff_h;
					}
				}

				if (fb_info->width < temp_width) {
					pipe_timing->hsync_start -= cntr_dff_w;
					pipe_timing->hsync_end -= cntr_dff_w;
				}

				if (fb_info->height < temp_height) {
					pipe_timing->vsync_start -= cntr_dff_h;
					pipe_timing->vsync_end -= cntr_dff_h;
				}
			}
		}
	}

	if(upscale) {
		/* Get the native timings */
		EMGD_DEBUG("Checking for Native LVDS match for scaling");
		native_timing = match_resolution(display, timing_table, pt_info,
			MATCH_NATIVE);
		if(native_timing && (native_timing != pipe_timing)) {
			native_timing->extn_ptr = pipe_timing;
			pipe_timing = native_timing;
		}
	}

	/*
	 * Match mode returns as follows:
	 * In case of VGA setmode:
	 * 1) We will end up with either:
	 *   magic->vga   ---   For displays supports native VGA
	 *      or
	 *   native->vga  ---   Upscaling displays
	 *      or
	 *   pipe->vga    ---   For other displays
	 *
	 * 2) In case of regular setmode:
	 *   pipe         ---   For regular displays
	 *      or
	 *   native->vesa ---   Upscaling displays
	 *
	 *   Note: 1) Here "pipe" can be munged if centering is required.
	 *         2) "vesa" is the requested mode, native is the native timing
	 *            of the display.
	 */

	/*
	 * Update Input Structures with values found
	 * Note: This might not be what is going to be programmed. It is what
	 * the user thinks they set. Scaling or centering could have altered
	 * that.
	 */
	fill_pt(user_timing, pt_info);
	*timing = pipe_timing;
	EMGD_DEBUG("Return");

	return 0;
}

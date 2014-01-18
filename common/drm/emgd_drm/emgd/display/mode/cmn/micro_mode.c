/*
 *-----------------------------------------------------------------------------
 * Filename: micro_mode.c
 * $Revision: 1.32 $
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

#include <igd.h>
#include <igd_init.h>
#include <igd_mode.h>
#include <igd_render.h>
#include <igd_pwr.h>
#include <igd_errno.h>
#include <igd_gmm.h>
#include <igd_pd.h>

#include <debug.h>
#include <context.h>
#include <rb.h>
#include <pd.h>
#include <intelpci.h>
#include <dsp.h>
#include <pi.h>
#include <pd_init.h>
#include <dispatch.h>
#include <mode.h>
#include <mode_access.h>
#include <dsp.h>
#include <utils.h>
#include <general.h>
#include <module_init.h>

#include "drm_emgd_private.h"
#include "match.h"
#include "mode_dispatch.h"

/*!
 * @addtogroup display_group
 * @{
 */

#ifndef CONFIG_MICRO
#define TIMING_CHANGED(a,b,c,d,e,f,g) timing_changed(a,b,c,d,e,f,g)
#define CALCULATE_ELD_INFOFRAMES
#else
#define TIMING_CHANGED(a,b,c,d,e,f,g) 1
#endif /* CONFIG_MICRO */

#define MODE_MIN(x, y) (x<y)?x:y

extern emgd_drm_config_t config_drm;

/*
 * NOTE: Do not add comma's to this dispatch table. The macro's
 * remove the entire entry.
 */
static dispatch_table_t mode_dispatch[] = {
	DISPATCH_PLB( &mode_dispatch_plb )
	DISPATCH_TNC( &mode_dispatch_tnc )
	DISPATCH_END
};



/*
 * Kernel Mode Setting (KMS) dispatch table
 */
static dispatch_table_t mode_kms_dispatch[] = {
	DISPATCH_PLB( &mode_kms_dispatch_plb )
	DISPATCH_TNC( &mode_kms_dispatch_tnc )
	DISPATCH_END
};



/*
 * Do not malloc the context for two reasons.
 *  1) vBIOS needs to minimize mallocs
 *  2) Mode context needs to stay around until after all modules are
 *    shut down for the register restore functionality.
*/
mode_context_t mode_context[1];


#ifndef CONFIG_MICRO

/*!
 * Check user supplied timing against what is currently
 * programmed to see if it needs to change.  If the timing
 * isn't going to change, then we don't want to turn off and
 * reprogram the port.
 *
 * NOTE: If this increases the size too much for vbios, then it
 * could be ifndef CONFIG_MICRO with an else that returns 1. That
 * would mean vbios wouldn't have these checks and always assume
 * the timing was changing.
 *
 * @param display
 * @param new_dc DC changes always require port/plane/pipe programming.
 * @param pt_info User supplied timing info to check.
 * @param fb_info User supplied framebuffer info.
 * @param port_number
 * @param display_mask Which display is being checked.
 * @param alter flags used to force the alter
 *
 * @return 1 timing has changed
 * @return 0 timing hasn't changed
 */
static int timing_changed(igd_display_context_t *display,
		unsigned long new_dc,
		unsigned long dc,
		igd_display_info_t *pt_info,
		igd_framebuffer_info_t *fb_info,
		unsigned long display_mask,
		unsigned long flags)
{
	igd_framebuffer_info_t *cfb;

	EMGD_TRACE_ENTER;

	/*
	 * will this cover every case?  Can a change on one display effect
	 * the timings of the other?  I don't think so.
	 */
	if ((new_dc & display_mask) != (dc & display_mask)) {
		return 1; /* Ports are changing on this display, must re-program. */
	}

	/*
	 * Make sure the owner port has a pt_info, if not then we'll assume
	 * that it hasn't been programmed yet and thus the timings are going
	 * to change.
	 */
	if (PORT_OWNER(display)->pt_info == NULL) {
		return 1;
	}

	/*
	 * Make sure we have valid timing info.  If not, then don't try
	 * and change the timings.
	 */
	if (!pt_info || !fb_info) {
		return 0;
	}

	/*
	 * If the caller really wants to re-program the planes/pipes/ports
	 * then do it
	 */

	if (flags & IGD_FORCE_ALTER) {
		return 1;
	}

	/*
	 * Check only width, height, refresh. If these don't match, then we
	 * know that something is changing.
	 */

	if ((pt_info->width == PORT_OWNER(display)->pt_info->width) &&
			(pt_info->height == PORT_OWNER(display)->pt_info->height) &&
			(pt_info->refresh == PORT_OWNER(display)->pt_info->refresh)) {

		/* Check framebuffer for changes, fb changes may change timing */
		if ((cfb = PLANE(display)->fb_info) != NULL) {
			if ((cfb->width != fb_info->width) ||
				(cfb->height != fb_info->height) ||
				(cfb->pixel_format != fb_info->pixel_format) ||
				((fb_info->flags^fb_info->flags) &~ IGD_SURFACE_RENDER)){
				/* Timing ok buf fb_info doesn't match */
				return 1;
			}
		}
	} else {
		/* Timing doesn't match */
		return 1;
	}

	/* Timing and fb have not changed */
	EMGD_TRACE_EXIT;
	return 0;
}
#endif

/*!
 * Update internal data structures for the plane, pipe, and port as
 * requested. Allocate a new framebuffer if the new parameters do not
 * match the existing framebuffer.
 *
 * @param display
 * @param port_number
 * @param timing
 * @param pt_info User supplied timing info to check.
 * @param fb_info User supplied framebuffer info.
 * @param flags
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int mode_update_plane_pipe_ports(
	igd_display_context_t *display,
	unsigned short port_number,
	igd_timing_info_t *timing,
	igd_framebuffer_info_t *fb_info,
	igd_display_info_t *pt_info,
	unsigned long flags)
{
	int ret;
	int alloc_fb;
	unsigned long size = 0;
	igd_framebuffer_info_t *plane_fb_info;
	igd_display_plane_t *mirror;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Port Number (%d)", port_number);

	EMGD_ASSERT( (fb_info || pt_info), "ERROR: fb_info & pt_info are NULL",
		-IGD_ERROR_INVAL);

	EMGD_ASSERT( PLANE(display)->fb_info, "ERROR: fb_info in plane is NULL",
		-IGD_ERROR_INVAL);

	plane_fb_info = PLANE(display)->fb_info;
	mirror = PLANE(display)->mirror;

	/*
	 * If there is a mirror plane (for Clone) and the mirror is populated
	 * then update our plane from the mirror. If the mirror is not populated
	 * then update the mirror from ours.
	 */
	if (mirror) {
		if(mirror->fb_info->flags) {
			OS_MEMCPY(plane_fb_info, mirror->fb_info,
				sizeof(igd_framebuffer_info_t));
		} else {
			OS_MEMCPY(mirror->fb_info, plane_fb_info,
				sizeof(igd_framebuffer_info_t));
		}
	}

	if (PORT(display, port_number)->pt_info == NULL) {
		if ((PORT(display, port_number)->pt_info = (igd_display_info_t *)
				OS_ALLOC(sizeof(igd_display_info_t))) == NULL) {
			EMGD_ERROR_EXIT("unable to alloc a pt_info struct in pipe.");
			return -IGD_ERROR_INVAL;
		}
	}

	/*
	 * If the fb_info was provided, and either we were asked to update
	 * the internal structures via the flags, or we are allocating a new
	 * framebuffer.
	 */
	if(fb_info && (flags & MODE_UPDATE_PLANE)) {

		/* Assume we will be allocating a FB */
		alloc_fb = 1;

		/* If the frambuffer parameters are unchanged then do not re-alloc */
		if((fb_info->width == plane_fb_info->width) &&
			(fb_info->height == plane_fb_info->height) &&
			(fb_info->pixel_format == plane_fb_info->pixel_format) &&
			((fb_info->flags^plane_fb_info->flags) & ~IGD_SURFACE_RENDER) == 0) {
			alloc_fb = 0;
		}

		/* Do not re-alloc a framebuffer if the re-use flag is set. */
		if(fb_info->flags & IGD_REUSE_FB) {
			alloc_fb = 0;
			/* May need to get the MIN_PITCH flags */
			plane_fb_info->flags = (fb_info->flags & IGD_FB_FLAGS_MASK) |
				(plane_fb_info->flags & ~IGD_FB_FLAGS_MASK);
		}

		/*
		 * If we don't have a framebuffer at all then we MUST allocate
		 * one.
		 */
		if(!plane_fb_info->allocated && !fb_info->allocated) {
			alloc_fb = 1;
		}

		EMGD_DEBUG("plane_fb_info->fb_base_offset = 0x%08lx",
			plane_fb_info->fb_base_offset);
		if(alloc_fb) {
			if(plane_fb_info->allocated) {
				/* Free frame buffer memory */
				display->context->dispatch.gmm_free(
					plane_fb_info->fb_base_offset);
				plane_fb_info->allocated = 0;
			}

			fb_info->fb_base_offset = plane_fb_info->fb_base_offset;
			/*
			 * Keep the FB flags, add in Displayable flag and blank out
			 * the rest. This insures that any tiled or usage flags from an
			 * earlier call do not get reused.
			 */
			fb_info->flags = (fb_info->flags & IGD_FB_FLAGS_MASK) |
				IGD_SURFACE_DISPLAY;

			/*
			 * Framebuffer allocations must always come from a reservation
			 * if the IAL changes the address the new address must also be
			 * from a reservation.
			 */
			GMM_SET_DEBUG_NAME("Framebuffer");
			ret = display->context->dispatch.gmm_alloc_surface(
				&fb_info->fb_base_offset,
				fb_info->pixel_format,
				&fb_info->width,
				&fb_info->height,
				&fb_info->screen_pitch,
				&size,
				IGD_GMM_ALLOC_TYPE_RESERVATION,
				&fb_info->flags);
			if(ret) {
				EMGD_ERROR_EXIT("Allocation of Front buffer failed: %d", ret);
				return ret;
			}
			fb_info->allocated = 1;
			/* Set the visible offset to the newly-allocated offset: */
			fb_info->visible_offset = fb_info->fb_base_offset;
		} else {
			/* If not reallocating, use back the offset in plane_fb_info */
			fb_info->fb_base_offset = plane_fb_info->fb_base_offset;
			 /* We must set the visible offset and screen pitch
			  * to proper value if we use back the plane.
			  */
                        fb_info->visible_offset = fb_info->fb_base_offset;
                        fb_info->screen_pitch = plane_fb_info->screen_pitch;
		}

		OS_MEMCPY(plane_fb_info, fb_info, sizeof(igd_framebuffer_info_t));
		plane_fb_info->allocated = 1;
		EMGD_DEBUG("plane_fb_info->fb_base_offset = 0x%08lx",
			plane_fb_info->fb_base_offset);

	}

	if(timing && (flags & MODE_UPDATE_PIPE)) {
		EMGD_DEBUG("Updating pipe timing.");
		PIPE(display)->timing = timing;
		PIPE(display)->owner = display;
	}

	if(pt_info && (flags & MODE_UPDATE_PORT)) {
		EMGD_DEBUG("OLD_PT========NEW PT ");
		IGD_PRINTK_PTINFO_2(PORT(display, port_number)->pt_info, pt_info);
		OS_MEMCPY(PORT(display, port_number)->pt_info, pt_info,
				sizeof(igd_display_info_t));
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end mode_update_plane_pipe_ports() */

#ifdef CALCULATE_ELD_INFOFRAMES

/*!
 * Calculates infoframes information top be used by HDMI port drivers
 *
 * @param port
 * @param timing_info
 * @param temp_cea
 *
 * @return 0
 */
static int calculate_infoframes(
	igd_display_port_t *port,
	igd_timing_info_t *timing_info,
	cea_extension_t *temp_cea)
{

	pd_timing_t     *cea_timings = NULL, *cea_timing_temp = NULL;

	EMGD_TRACE_ENTER;

	/* VBIOS has no access to CEA timing tables and this is not supported
	   there as well */
	if(timing_info->mode_info_flags & PD_MODE_CEA){
		if(timing_info->width != 640 && timing_info->height != 480){
			port->edid->cea->quantization	= HDMI_QUANTIZATION_RGB_220;
		}

		/* Based on DPG algorithm. If monitors support more than 2 channels
		   for 192Khz or/and 92Khz then set two pixel repeat one.
		   KIV: Add pruning for pixel PIX_REPLICATION_3 if required  */
		if(temp_cea->audio_cap[CAP_192_KHZ].max_channels>2 ||
			temp_cea->audio_cap[CAP_96_KHZ].max_channels>2){
			port->edid->cea->pixel_rep = PIX_REPLICATION_1;
		}


		/* Based on HDMI spec 6.7.1 & 6.7.2 */
		if ((timing_info->width == 720) && ((timing_info->height == 480) ||
			 (timing_info->height== 576))){
			port->edid->cea->colorimetry	= HDMI_COLORIMETRY_ITU601;
		} else if(((timing_info->width==1280) && (timing_info->height==720)) ||
			((timing_info->width == 1920) && (timing_info->height == 1080))){
			port->edid->cea->colorimetry	= HDMI_COLORIMETRY_ITU709;
		}

		cea_timings = (igd_timing_info_t *) OS_ALLOC(cea_timing_table_size);
		OS_MEMCPY(cea_timings, cea_timing_table, cea_timing_table_size);
		cea_timing_temp = cea_timings;

		while (cea_timings->width != IGD_TIMING_TABLE_END){
			if(cea_timings->width == timing_info->width &&
			   cea_timings->height == timing_info->height &&
			   cea_timings->refresh == timing_info->refresh &&
			   cea_timings->dclk == timing_info->dclk &&
			   (cea_timings->mode_info_flags &
			   (PD_ASPECT_16_9| IGD_SCAN_INTERLACE)) ==
			   (timing_info->mode_info_flags &
			   (PD_ASPECT_16_9| IGD_SCAN_INTERLACE))){
					port->edid->cea->video_code		= cea_timings->mode_number;
					break;
				}
			cea_timings++;
		}

		OS_FREE(cea_timing_temp);

	}

	EMGD_TRACE_EXIT;
	return 0;
}
#endif /* CALCULATE_ELD_INFOFRAMES */

/*!
 * Calculates the Edid like data (ELD) if port supports audio transmission
 *
 * @param port
 * @param timing_info
 *
 * @return 0
 */
int calculate_eld(
	igd_display_port_t *port,
	igd_timing_info_t *timing_info)
{
	/* Calculate for non-content protected & content protected(Array of 2) */
#ifdef CALCULATE_ELD_INFOFRAMES
	unsigned long cal_NPL[2]; /* Number of packer per line calculate*/
	unsigned long poss_NPL[2]; /* Number of packer per line possible*/
	unsigned long max_bitRate_2[2],max_bitRate_8[2];
	unsigned long h_refresh, audio_freq;
	unsigned char input;
	int i,j,pix_rep;
#endif
	cea_extension_t *temp_cea = NULL ;

	EMGD_TRACE_ENTER;
	/* Only calculate eld for HDMI port */
	if((port->pd_driver->type !=  PD_DISPLAY_HDMI_EXT &&
		port->pd_driver->type !=  PD_DISPLAY_HDMI_INT)){
		return 0;
	}
	if(port->firmware_type == PI_FIRMWARE_EDID){
		temp_cea = (cea_extension_t*)port->edid->cea;
	}
	/* Displayid unsupported for now. Uncomment this code when audio
	information is available for Display ID
	temp_cea = (&cea_extension_t)port->displayid->cea;*/

	if(temp_cea == NULL){
		/* CEA data unavailable, display does not have audio capability? */
		/* We would allocate dummy edid structure and here and we should ony
		   used canned ELD */
		if(port->edid == NULL) {
			port->edid = (edid_t *) OS_ALLOC(sizeof(edid_t));
			OS_MEMSET(port->edid, 0 , (sizeof(edid_t)));
		}

		port->edid->cea = (cea_extension_t *) OS_ALLOC(sizeof(cea_extension_t));
		OS_MEMSET(port->edid->cea, 0 , (sizeof(cea_extension_t)));
		port->edid->cea->canned_eld = 1;
		temp_cea = (cea_extension_t*)port->edid->cea;
		port->callback->eld = &(port->edid->cea);
	}

	/* Default to canned ELD data */
	temp_cea->LPCM_CAD[0] = 0x9;
	temp_cea->speaker_alloc_block[0] = 0x1;
	/* Default 0 Pixel replication */
	port->edid->cea->pixel_rep = PIX_REPLICATION_0;
	/* Default */
	port->edid->cea->colorimetry = HDMI_COLORIMETRY_NODATA;
	/* Default RGB 256 wuantization full range */
	port->edid->cea->quantization = HDMI_QUANTIZATION_RGB_256;
	/* Default Unknown video code */
	port->edid->cea->video_code = 0;
	port->edid->cea->aspect_ratio = (timing_info->mode_info_flags & PD_ASPECT_16_9)
	             ? PD_ASPECT_RATIO_16_9 : PD_ASPECT_RATIO_4_3;

#ifdef CALCULATE_ELD_INFOFRAMES
	calculate_infoframes(port,timing_info,temp_cea);
	/* If canned eld is not set and audio info from transmitter is available */
	if(temp_cea->canned_eld != 1 && (temp_cea->audio_flag & PD_AUDIO_CHAR_AVAIL)){
		pix_rep = port->edid->cea->pixel_rep;
		/*h_refresh = timing_info->dclk/timing_info->htotal;*/
		h_refresh = timing_info->refresh;
		cal_NPL[0] = (pix_rep*(timing_info->hsync_end - timing_info->hsync_start) -
					port->edid->cea->K0) /32;
		cal_NPL[1] = (pix_rep*(timing_info->hsync_end - timing_info->hsync_start) -
					port->edid->cea->K1) /32;

		poss_NPL[0] = MODE_MIN(cal_NPL[0],port->edid->cea->NPL);
		poss_NPL[1] = MODE_MIN(cal_NPL[1],port->edid->cea->NPL);

		max_bitRate_2[0] = h_refresh * poss_NPL[0] - 1500;
		max_bitRate_2[1] = h_refresh * poss_NPL[1] - 1500;

		max_bitRate_8[0] = h_refresh * poss_NPL[0] * 4 - 1500;
		max_bitRate_8[1] = h_refresh * poss_NPL[1] * 4 - 1500;

		/* Loop trough Content Protection disabled then enabled */
		for(i=0 ; i<2; i++){
			for(j=0 ; j<3; j++){
				input = 0;
				audio_freq = 48000 * (1<<j); /* 48Khz->96Khz->192Khz */
				if(max_bitRate_8[i] >= audio_freq){
					input = 7;
				}else if(max_bitRate_2[i] >= audio_freq){
					input = 1;
				}
				/* take the minimum value min(transmitter, receiver) */
				input = MODE_MIN(input,temp_cea->audio_cap[j].max_channels);
				temp_cea->LPCM_CAD[j] |= input<<((1-i)*3);
				if(temp_cea->audio_cap[j]._24bit){
					temp_cea->LPCM_CAD[j] |= BIT(7);
				}
				if(temp_cea->audio_cap[j]._20bit){
					temp_cea->LPCM_CAD[j] |= BIT(6);
				}
			}
		}

		/* TODO: Further construction of ELD from Monitor Name String begins here
		   for now we only support VSDB */
		/* By default we don send any vendor specific block unless latency value
		   use for audio sync feature is available */
		temp_cea->vsdbl = 0;
		/* This means the latecy field is available VSBD_LATENCY_FIELD = 8*/
		if(temp_cea->vendor_block.vendor_block_size > VSBD_LATENCY_FIELD){
			OS_MEMCPY(temp_cea->misc_data, temp_cea->vendor_data_block,
				temp_cea->vendor_block.vendor_block_size);
			temp_cea->vsdbl = temp_cea->vendor_block.vendor_block_size;
			/* If the VSBD has latency fields */
			if(*(temp_cea->vendor_data_block + VSBD_LATENCY_FIELD - 1) & 0x80){
				if(timing_info->mode_info_flags & IGD_SCAN_INTERLACE){
					if(*(temp_cea->vendor_data_block + VSBD_LATENCY_FIELD - 1) & 0x40){
						temp_cea->vendor_block.p_latency = 1;
						temp_cea->vendor_block.i_latency = 1;
					}else{
						/* No latency available: Since it is an interlace mode but no
						   vsbd_intlc_fld_present is available */
						temp_cea->vendor_block.p_latency = 0;
						temp_cea->vendor_block.i_latency = 0;
					}
				}else{
					temp_cea->vendor_block.p_latency = 1;
					temp_cea->vendor_block.i_latency = 0;
				}
			}
		}
	}
#endif /* CALCULATE_ELD_INFOFRAMES */
	temp_cea->audio_flag |= ELD_AVAIL;
	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * Configure either the primary or secondary display. This means all the
 * timings, the framebuffer, the ports, the plane, and the pipe.
 *
 * The port range could be calculated based on primary or secondary
 * display but it seems easier at this point to pass the port range
 * in since it is used for all the for loops.
 *
 * @param driver_handle
 * @param display
 * @param pt_info
 * @param fb_info
 * @param dc
 * @param p0
 * @param pn
 * @param flags
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
static int configure_display(
	igd_driver_h driver_handle,
	igd_display_context_t *display,
	igd_display_info_t *pt_info,
	igd_framebuffer_info_t *fb_info,
	unsigned long dc,
	int p0, int pn,
	unsigned long flags)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	int p;
	igd_display_port_t *port;
	igd_timing_info_t *timing_info;
	unsigned long update_flags;
	unsigned short port_number = 0;
	int ret;
	int seamless = FALSE;

	EMGD_TRACE_ENTER;

	/* FIXME: Should this be an assert? */
	if (display == NULL) {
		EMGD_DEBUG("Trying to configure a NULL display");
		return 0;
	}

	EMGD_DEBUG("Configure timings");
	for (p = pn; p > p0; p--) {
		EMGD_DEBUG("Configure port %d", DC_PORT_NUMBER(dc, p));
		if ((port_number = DC_PORT_NUMBER(dc, p))) {
			port = context->mod_dispatch.dsp_port_list[port_number];
			if (!port) {
				EMGD_DEBUG("Port %d not found", port_number);
			} else {

				/* Put a copy of the timings in the port's structure */
				if (pt_info) {
					if (port->pt_info == NULL) {
						port->pt_info = OS_ALLOC(sizeof(igd_display_info_t));
						if (!port->pt_info) {
							EMGD_ERROR_EXIT("unable to alloc a pt_info "
								"struct in port.");
							return -IGD_ERROR_INVAL;
						}
					}
					OS_MEMCPY(port->pt_info, pt_info,
						sizeof(igd_display_info_t));
				} else {
					EMGD_ERROR("No primary timing info!");
				}
			}
		}
	}

	if(!(pt_info->flags & IGD_DISPLAY_ENABLE)) {
		EMGD_ERROR_EXIT("Ptinfo has no IGD_DISPLAY_ENABLE!");
		return 0;
	}

	display->port_number = DC_PORT_NUMBER(dc, (p0 + 1));

	/* Set mode */
	EMGD_DEBUG("Set mode, using port %ld", display->port_number);
	port = PORT(display, display->port_number);

	EMGD_DEBUG("Calling matchmode on display");
	ret = match_mode(display, port->timing_table, fb_info, pt_info,
			&timing_info);
	if(ret) {
		EMGD_DEBUG("Match Mode for display failed");
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}

	/* Now b4, we program the timing_info, let's first see if seamless
     * option is requested, if it is then
	 * We need to make sure the incoming dc, timing, framebuffer
	 * info and etc match. We must respect the FORCE_ALTER flag.
	 *
	 * Seamless option buys you a one-time ticket for the seamless
	 * experience from the firmware to the driver. After the first mode set
	 * in driver, you don't get it the next time when you alter display.
	 *
	 */
#ifndef CONFIG_MICRO
	if(dc &&  !(flags & IGD_FORCE_ALTER) &&
			 (mode_context->seamless == TRUE) ) {

		/* User wants seamless */
		if(mode_context->fw_info != NULL) {

			OPT_MICRO_CALL_RET(seamless, query_seamless(dc,
				/*(p0/4),*/
				PIPE(display)->pipe_num,
			    timing_info,
				fb_info,
				0));

			EMGD_DEBUG(":Seamless = %s", seamless ?"ON" : "OFF");
			/* moved this to alter_displays to handle the case for CLONE */
			/*mode_context->seamless = FALSE;*/
			/* FIXME: For clone you get called twice. Need to
			 * Fix that corner case
			 */

		}
	}
#endif
	/* In case the seamless is FALSE, we do reset_plane_pipe_ports
	 * which is supposed to be called in alter_displays anyway.
	 * But we have to delay it since the user asked for seamless.
	 * And we don't want to switch-off the display during
	 * seamless.
	 * Now we know that even though the user asked for it, we cannot
	 * support seamless, so we call reset_plane_pipe_ports now.
	 */
	if(seamless == FALSE) {

		/* Reset planes/pipes/ports before doing first alter display */
		if (mode_context->first_alter) {
			mode_context->dispatch->reset_plane_pipe_ports(
									mode_context->context);
			mode_context->first_alter = FALSE;
		}

	}

	if(calculate_eld(port, timing_info)){
		EMGD_DEBUG("Fail to calculate ELD");
	}
	/* turn on all ports */
	EMGD_DEBUG("turn on displays plane_pipe_ports %d..%d", (p0 + 1), (pn-1));

	for (p = (p0 + 1); p <= pn; p++) {
		if (DC_PORT_NUMBER(dc, p)) {
			port = context->mod_dispatch.dsp_port_list[DC_PORT_NUMBER(dc, p)];

			display->allocated = 1;

			/* Update mode info for the port */
			if (p == (p0 + 1)) {
				update_flags = MODE_UPDATE_PLANE | MODE_UPDATE_PIPE |
						MODE_UPDATE_PORT;
			} else {
				update_flags = MODE_UPDATE_PORT;
			}
			ret = mode_update_plane_pipe_ports(display, DC_PORT_NUMBER(dc, p),
					timing_info, fb_info, pt_info, update_flags);
			if (ret) {
				/*
				 * This could happen if there was no memory for the
				 * framebuffer or the FB was an invalid format. The
				 * first is a too bad failure. The second should have
				 * been checked by the IAL.
				 */
				EMGD_ERROR_EXIT("mode_update_plane_pipe_ports returned error "
					"%d", ret);
				port->pt_info->flags &= ~IGD_DISPLAY_ENABLE;
				return ret;
			}

			/* Program the port registers */
			if(seamless == TRUE) {
				/* Don't have to program the registers, Since it's
				 * all updated. Just return 0
				 */
				ret = 0;
			} else {
				ret = mode_context->dispatch->program_port(display,
					DC_PORT_NUMBER(dc, p), TRUE);
			}
			if (ret == 0) {
				port->inuse = 1;
			} else {
				port->pt_info->flags &= ~IGD_DISPLAY_ENABLE;
			}
		}
	}
	EMGD_DEBUG("done - turn on displays plane_pipe_ports %d", p);

	/* Clear the Framebuffer after the planes, pipes and ports are
	 * disabled and before they are enabled. */
	if (flags & IGD_CLEAR_FB) {
		OPT_MICRO_VOID_CALL(full_clear_fb(mode_context, fb_info, NULL));
	}

	/* program the pipe/plane/port if seamless is FALSE */
	if(seamless == FALSE) {

		EMGD_DEBUG("Seamless is FALSE");

		ret = TRUE;

		do{
		/* turn on pipe */
		mode_context->dispatch->program_pipe(display, TRUE);

		/* turn on plane */
		mode_context->dispatch->program_plane(display, TRUE);

		/* turn on port */
		for (p = pn; p > p0; p--) {
			if (DC_PORT_NUMBER(dc, p)) {
					mode_context->dispatch->post_program_port(display,
				DC_PORT_NUMBER(dc, p), TRUE);
			}
		}

			/* Check is display working fine */
			OPT_MICRO_CALL_RET(ret, mode_context->dispatch->
					check_display(display, DC_PORT_NUMBER(dc, p),TRUE));

			if(!ret){
				/* turn off plane and pipe */
				mode_context->dispatch->program_plane(display, FALSE);
				mode_context->dispatch->program_pipe(display, FALSE);
			}
		}while(!ret);

	}
#ifndef CONFIG_MICRO
	else  { /* Seamless is TRUE */

		/* Updating the plane registers, does not require us to
		 * turn-off the pipe and we can still have seamless
		 * because the display is not turned-off
		 */

		EMGD_DEBUG(" Seamless is TRUE");
		/* special handling for fw clone to vext seamless */
		if((IGD_DC_VEXT(config_drm.dc) && IGD_DC_CLONE(mode_context->fw_info->fw_dc))){
			mode_context->dispatch->full->lock_planes(display);
		}
		if(mode_context->fw_info->program_plane == 1) {

			/* This means we have to update the plane registers
			 * with the new values.eg. Change in pitch size between
			 * firmware values and driver values. But we MUST also
			 * update the palette registers for this to work and
			 * palette registers are programmed when pipe is programmed.
			 * This means we program the pipe , followed by the plane.
			 */

			/* By doing this, we update the palette */
			mode_context->dispatch->program_pipe(display, TRUE);

			/* update the plane registers */
			mode_context->dispatch->program_plane(display, TRUE);
		}
	}
#endif

	EMGD_TRACE_EXIT;
	return 0;
}

int igd_configure_display(
	igd_driver_h driver_handle,
	igd_display_h *display,
	igd_display_info_t *pt_info,
	igd_framebuffer_info_t *fb_info,
	unsigned long dc,
	int fb_index,
	unsigned long flags)
{
	int p0, pn;
	int ret;

	EMGD_TRACE_ENTER;

    /* Which ports do we loop through in the dc */
    if (fb_index == 0) {
        p0 = 0;
        pn = 4;
    } else {
        p0 = 4;
        pn = 7;
    }

	ret = configure_display(driver_handle, (igd_display_context_t *)display,
		pt_info, fb_info, dc, p0, pn, flags);

	EMGD_TRACE_EXIT;
	return ret;
}


#ifndef CONFIG_MICRO

/*!
 * This function checks whether all the ports on the primary pipe of a dc are
 * moving to a secondary pipe on a new dc.
 *
 * @param current_dc current_dc the system is in.
 * @param dc     new requested dc.
 *
 * @return 0 if not all the ports are moving
 * @return 1 if all the ports are moving
 */
int all_ports_moving(unsigned long current_dc, unsigned long dc)
{
	unsigned long index, index2, port;
	int found_port;
	int result = 1;

	/*
	 * We need to look for each of the primary ports on the current dc,
	 * and see if they exist on the secondary port of the new dc.
	 */
	for (index = IGD_DC_IDX_PRIMARY_MASTER;
		index <= IGD_DC_IDX_PRIMARY_TWIN3; index++) {
		found_port = 0;
		port = IGD_DC_PORT_NUMBER(current_dc, index);
		/* If there is a port, let's look for it. */
		if (port) {
			/* Try to find this port on the secondary pipe */
			for (index2 = IGD_DC_IDX_SECONDARY_MASTER;
				index2 <= IGD_DC_IDX_SECONDARY_TWIN2; index2++) {
				/*
				 * We found the port, let's stop looking for this port and
				 * move on to the next port
				 */
				if (port == IGD_DC_PORT_NUMBER(dc, index2)) {
					found_port = 1;
					break;
				}
			}
			/* As soon as there is a port that we don't find, we can exit */
			if (!found_port) {
				result = 0;
				break;
			}
		}
	}

	return result;
}

/*!
 * This function checks whether a specified port exists in a dc, and if it does
 * it returns the pipe master index for the pipe which contained the port.
 *
 * @param dc     the dc to check for a port.
 * @param port   the port to check for.
 *
 * @return 0 if the specified does not exist
 * @return IGD_DC_IDX_PRIMARY_MASTER if the port was found on the primary pipe
 * @return IGD_DC_IDX_SECONDARY_MASTER if the port was found on the secondary
 *   pipe
 */
int dc_contains_port_type(unsigned long dc, unsigned long port)
{
	unsigned long index;
	int result = 0;

	for (index = IGD_DC_IDX_PRIMARY_MASTER;
		index <= IGD_DC_IDX_PRIMARY_TWIN3; index++) {

		if (IGD_DC_PORT_NUMBER(dc, index) == port) {
			result = IGD_DC_IDX_PRIMARY_MASTER;
			break;
		}
	}

	for (index = IGD_DC_IDX_SECONDARY_MASTER;
		index <= IGD_DC_IDX_SECONDARY_TWIN2; index++) {

		if (IGD_DC_PORT_NUMBER(dc, index) == port) {
			result = IGD_DC_IDX_SECONDARY_MASTER;
			break;
		}
	}

	return result;
}

/*!
 * This function checks whether a frame buffer swap and cursor
 * swap is required based on given current_dc and new_dc.
 *
 * @param current_dc current_dc the system is in.
 * @param dc     new requested dc.
 * @param dsp    pointer to a display context pointer
 *
 * @return 0 if no swap is required
 * @return 1 if swap is required
 */
int swap_required(
	unsigned long current_dc,
	unsigned long dc,
	igd_display_context_t **dsp)
{

	/*
	 * Note: Preserve the order of conditions as is. Changing the order
	 * of below conditions require relook into the whole function to
	 * make sure sematics match to return right value.
	 */

	/* Do not swap if no dc or current_dc */
	if (!dc || !current_dc) {
		return 0;
	}

	/*
	 * Do NOT swap the frame buffer and cursor if dc is going
	 * from twin to extended and not all the ports are moving to the other
	 * pipe. If all the ports are moving to the other pipe, then DO swap
	 * the frame buffer and cursor.
	 * We also need to make sure we are not in the special case where our
	 * extended mode contains LVDS and we are on a platform which does not
	 * support LVDS on PIPE A. In this case we do not want to swap here because
	 * we will not be swapping later since the twin configuration will stay on
	 * PIPE A so that the LVDS can move to PIPE B.
	 */
	if (IGD_DC_TWIN(current_dc) && IGD_DC_EXTENDED(dc)) {
		if (dsp && *dsp && all_ports_moving(current_dc, dc) &&
			!(dc_contains_port_type(dc, IGD_PORT_TYPE_LVDS) &&
			!(PIPE(*dsp)->pipe_features & IGD_PORT_SHARE_LVDS))) {
			return 1;
		} else {
			return 0;
		}
	}

	/*
	 * Do NOT swap if dc is changing from Single to clone or
	 * vice versa. In this case there is only 1 fb and no swap is required
	 * and it stays with primary display.
	 */
	if ((IGD_DC_SINGLE(current_dc) && IGD_DC_CLONE(dc)) ||
		(IGD_DC_CLONE(current_dc) && IGD_DC_SINGLE(dc))) {
		return 0;
	}

	/*
	 * If secondary master port is moving to primary master or
	 * primary master is moving to secondary master, try
	 * and keep the framebuffer address and cursor the same.
	 */
	if (DC_PORT_NUMBER(current_dc, 5) == DC_PORT_NUMBER(dc, 1) ||
		(DC_PORT_NUMBER(current_dc, 1) == DC_PORT_NUMBER(dc, 5))) {
		/*
		Note: This was previosuly return 1 to indicate swap required, which will cause
		a frame buffer swap when changing display from twin to clone. If the display is
		in extended mode before changing from twin to clone, then both Plane A and Plane B
		will point to the secondary frame buffer offset adress in clone mode and cause
		blank screen (.
		*/
		return 0;
	}
	return 0;
}
#endif

/*!
 * This function sets up planes, pipes, and ports
 * with the configuration passed in and returnes either one
 * or two display handle lists.
 *
 * @param driver_handle from igd_init_driver().
 * @param primary on return, this points to a list of displays.
 * @param primary_ptinfo incoming timing info for the primary.
 * @param primary_fbinfo incoming framebuffer info.
 * @param secondary on return, this points to a list of displays.
 * @param secondary_fbinfo incoming framebuffer info.
 * @param dc display configuration
 * @param flags modify function behavior
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int igd_alter_displays(
	igd_driver_h driver_handle,
	igd_display_h *_primary,
	igd_display_info_t *primary_pt_info,
	igd_framebuffer_info_t *primary_fb_info,
	igd_display_h *_secondary,
	igd_display_info_t *secondary_pt_info,
	igd_framebuffer_info_t *secondary_fb_info,
	unsigned long dc,
	unsigned long flags)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t **primary = (igd_display_context_t **)_primary;
	igd_display_context_t **secondary = (igd_display_context_t **)_secondary;
	igd_framebuffer_info_t *fb_info = NULL;
	igd_display_context_t *display = NULL,*tv_display=NULL;
	drm_emgd_priv_t *priv = ((struct drm_device *)context->drm_dev)->dev_private;
	int p;
	int ret;
	unsigned short tv_port_num=0;
	int p_chng = 1, s_chng = 1;
	unsigned char disable_plane_pipe = 0;
	unsigned long current_dc;


	EMGD_TRACE_ENTER;

	/*
	 * Make sure the DC is valid
	 *
	 * vBIOS won't be able to do this every time, for now only have
	 * the drivers's do the check.
	 */
#ifndef CONFIG_MICRO
	if (dc && !dsp_valid_dc(dc, 0)) {
		EMGD_ERROR_EXIT("Invalid display configuration: 0x%08lx", dc);
		return -IGD_ERROR_INVAL;
	}
#endif


	/*
	 * Can all display_info's and fb_info's be NULL?  I.E. make this
	 * function do an alloc of display handles only?  If so, then
	 * check for that condition and return without error. Otherwise
	 * return an error.
	 */
	if (dc && (!primary_pt_info && !primary_fb_info) &&
			(!secondary_pt_info && !secondary_fb_info)) {
		EMGD_ERROR_EXIT("Invalid timing and framebuffer info");
		return -IGD_ERROR_INVAL;
	}

#ifndef CONFIG_MICRO
	/* FIXME: GDK Change this to dispatch->idle() */
	if (dsp_wait_rb(mode_context->context) != 0) {
		return -IGD_ERROR_INVAL;
	}
#endif

	/* If seamless request is NOT set , then do reset_plane_pipe_ports
	 * else delay it until we cannot support it.
	 * If seamless is requested by the user and we CAN support it
	 * then we need to make sure reset_plane_pipe_ports is NOT
	 * called. That's the whole point anyway. Not to reset anything
	 * during seamless transition
	 */
	if(mode_context->seamless != TRUE) {

		/* Reset planes/pipes/ports before doing first alter display */
		if (mode_context->first_alter) {
			mode_context->dispatch->reset_plane_pipe_ports(
									mode_context->context);
			mode_context->first_alter = FALSE;
		}
	}

	current_dc = *(context->mod_dispatch.dsp_current_dc);

#ifndef CONFIG_MICRO
	/* Check if platform needs force alter
	* 	to make sure we run tuning code. This
	* 	is for TNC-B0 workaround.*/
	if (mode_context->dispatch->dsp_is_force_alter_required){
		if (mode_context->dispatch-> dsp_is_force_alter_required(context->
						mod_dispatch.dsp_display_list[IGD_DC_PRIMARY(current_dc)],
						current_dc, dc)){
				flags |= IGD_FORCE_ALTER;
		}
	}
#endif

	/*
	 * Turn off the planes, pipes, and ports associated with the current
	 * DC. However, limit the change to the primary if the secondary
	 * display handle is NULL or limit the change to the secondary if the
	 * the pimary display handle is NULL.
	 */
	for (p = 7; p > 0; p--) {
		if (p > 4) {
			display = NULL;
			if (DC_PORT_NUMBER(current_dc, p)) {
				display = context->mod_dispatch.
					dsp_display_list[IGD_DC_SECONDARY(current_dc)];
				s_chng = TIMING_CHANGED(display, dc, current_dc,
						secondary_pt_info, secondary_fb_info,
						(unsigned long)0xfff00000, flags);
			}

			if (s_chng && display && secondary) {
		/* if the port is TV, then don't set the power to S3 as this causes
		 * blank screen and system hang on LVDS on FSDOS, probably because the
		 * external clock needs to be on till the pipes and
		 * DPLLs are off
		 */
				/* Invalidate flip-chains to avoid race conditions during the
				 * mode-set */
				priv->invalidate_flip_chains(IGD_DISPLAY_SECONDARY);

				if(PORT(display,DC_PORT_NUMBER(current_dc, p))->pd_type ==
					PD_DISPLAY_TVOUT) {
					tv_display = display;
					tv_port_num = DC_PORT_NUMBER(current_dc, p);
				} else {
					ret = mode_context->dispatch->program_port(display,
					      DC_PORT_NUMBER(current_dc, p), FALSE);
				}
				/* The secondary pipe master */
				if (p == 5) {
					disable_plane_pipe = 1;
				}
			}
		} else {
			display = NULL;
			if (DC_PORT_NUMBER(current_dc, p)) {
				display = context->mod_dispatch.
					dsp_display_list[IGD_DC_PRIMARY(current_dc)];
				p_chng = TIMING_CHANGED(display, dc, current_dc,
						primary_pt_info, primary_fb_info,
						(unsigned long)0x000ffff0, flags);
			}

			if (p_chng && display && primary) {
		/* if the port is TV, then don't set the power to S3 as this causes
		 * blank screen and system hang on LVDS on FSDOS, probably because the
		 * external clock needs to be on till the pipes and
		 * DPLLs are off
		 */
				/* Invalidate flip-chains to avoid race conditions during the
				 * mode-set */
				priv->invalidate_flip_chains(IGD_DISPLAY_PRIMARY);

				if(PORT(display,DC_PORT_NUMBER(current_dc, p))->pd_type ==
					PD_DISPLAY_TVOUT) {
					tv_display = display;
					tv_port_num = DC_PORT_NUMBER(current_dc, p);
				} else {
					ret = mode_context->dispatch->program_port(display,
				      DC_PORT_NUMBER(current_dc, p), FALSE);
				}
				/* The primary pipe master */
				if (p == 1) {
					disable_plane_pipe = 1;
				}
			}
		}

		/* Disable plane and pipe after disabling the ports */
		if (disable_plane_pipe) {
			if(mode_context->dispatch->full) {
				mode_context->dispatch->full->program_cursor(display, FALSE);
			}
			mode_context->dispatch->program_plane(display, FALSE);
			mode_context->dispatch->program_pipe(display, FALSE);
			/*pipes and dplls are off, now turn off tv port */
			if(tv_display) {
				ret = mode_context->dispatch->program_port(tv_display,
					tv_port_num, FALSE);
				tv_display = NULL;
			}
			disable_plane_pipe = 0;
		}
	}

#ifndef CONFIG_MICRO
	/* If DC is zero, then return here. A zero dc turns everything off */
	/* This never happens for VBIOS since it only always calls *
	 * alter_displays at the same point with the same valid DC */
	if (!dc) {
		int i;

		priv->invalidate_flip_chains(IGD_DISPLAY_ALL);

		mode_context->dispatch->reset_plane_pipe_ports(mode_context->context);
		/* Should de-allocate everything here */
		dsp_alloc(driver_handle, dc, flags);
		/*
		 * FIXME: This should be done inside dsp alloc, mode module does
		 * not own this information.
		 * When dc = 0, set all displays allocated to 0.
		 */
		for (i=0; i<IGD_MAX_PORTS+1; i++) {
			if (context->mod_dispatch.dsp_display_list[i]) {
				context->mod_dispatch.dsp_display_list[i]->allocated = 0;
			}
			context->mod_dispatch.dsp_display_list[i] = NULL;
		}

		return 0;
	}
#endif

	/*
	 * Check the DC (display configuration). If it is the same as the
	 * current configuration, then don't change any allocations, only
	 * modify the framebuffers and timings.
	 */
	if (dc != current_dc) {
		EMGD_DEBUG("Allocate display handles based on DC");

#ifndef CONFIG_MICRO
		if (swap_required(current_dc, dc, primary)) {
			swap_fb_cursor();
		}
#endif
		/*
		 * This function should never be called after VBIOS initialization *
		 * The dsp_alloc is discarded after VBIOS init and is over-  *
		 * written by font tables. Thus in VBIOS IAL, alter_displays *
		 * is never get called with a different DC from the 1st time *
		 */
		dsp_alloc(driver_handle, dc, flags);

	}

	/* Attach the displays to the caller's pointers */
	if (primary) {
		*primary = context->mod_dispatch.dsp_display_list[IGD_DC_PRIMARY(dc)];
	}
	if (secondary) {
		EMGD_DEBUG("Attaching display 1 to secondary pointer");
		*secondary = context->mod_dispatch.
			dsp_display_list[IGD_DC_SECONDARY(dc)];
	}

	/*
	 * Configure the primary display. This configures the timings and the
	 * framebuffer. Once configured, it turns everythying on.
	 */
	if(primary && *primary && (primary_pt_info || primary_fb_info) && p_chng) {
		EMGD_DEBUG("Configure primary timings");
		/* make framebuffer changes */
		if (primary_fb_info) {
			/* set up new frame buffer info */
			fb_info = primary_fb_info;
		} else {
			fb_info = PLANE(*primary)->fb_info;
		}

		ret = configure_display(driver_handle,
				(igd_display_context_t *)(*primary), primary_pt_info,
				fb_info, dc, 0, 4, flags);
		if (ret) {
			EMGD_DEBUG("Primary display disabled.");
		}
	}

	/*
	 * Configure the secondary display. This configures the timings and the
	 * framebuffer. Once configured, it turns everythying on.
	 *
	 * How close is this code to the code for the primary?  Could this
	 * be moved to a separate function?
	 */
	if (secondary != NULL) {

#ifndef CONFIG_MICRO
		/*
		 * In the case where we are in extended or clone and our pipe is not
		 * turned on, we need to turn the pipes on.
		 * We can run into this situation on pre-Cantiga Gen platforms on Linux
		 * where LVDS was the primary display and was assigned PIPE B. Then we
		 * are switching from LVDS to another display and that other display
		 * wants to take PIPE A. In this case PIPE B will be turned on, the
		 * display's new port will take PIPE A and turn on PIPE A.  The second
		 * display thinks it is still PIPE A and nothing has changed for it.
		 * In this case where our pipe is not turned on, we need to let the
		 * system know that something has changed.
		 */
		if ((IGD_DC_CLONE(dc) || IGD_DC_EXTENDED(dc))
			&& !(EMGD_READ32(MMIO(*secondary) + PIPE(*secondary)->pipe_reg)
			& 0x80000000)) {
			s_chng = 1;
		}
#endif

		EMGD_DEBUG("Starting secondary pipe programming");
		if ((*secondary != NULL) && (secondary_pt_info || secondary_fb_info) &&
				s_chng){
			/*
			 * Configure the framebuffer.  For clone, it is the same
			 * as the primary. For DIH, it is a unique fb.
			 */
			EMGD_DEBUG("configure secondary framebuffer");
			if (dc & IGD_DISPLAY_CONFIG_CLONE) {
				fb_info = PLANE(*primary)->fb_info;
			} else {
				if (secondary_fb_info) {
					fb_info = secondary_fb_info;
				} else {
					fb_info = PLANE(*secondary)->fb_info;
				}
			}

			ret = configure_display(driver_handle,
					(igd_display_context_t *)(*secondary), secondary_pt_info,
					fb_info, dc, 4, 7, flags);
			if (ret) {
				EMGD_DEBUG("Secondary display disabled.");
				EMGD_ERROR("Secondary display disabled.");
			}
		}
	} else {
		EMGD_DEBUG("Skipped secondary programming, NULL handle");
	}

	/*
	 * Workaround: wait for Vblank to avoid people accessing display
	 * plane registers before the register is updated properly.
	 */
	if (primary && *primary) {
		EMGD_DEBUG("Wait for vblank on primary display (%p)", primary);
		EMGD_DEBUG("Wait for vblank on primary display (%p)", *primary);
		mode_context->dispatch->wait_vblank(*primary);
	} else if (secondary && *secondary) {
		EMGD_DEBUG("Wait for vblank on secondary display");
		mode_context->dispatch->wait_vblank(*secondary);
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function sets the power state for the passed
 * display handle.  This only updates the power state for the
 * display/port.  The pipe, plane, and ringbuffer are left in
 * the same power state.  There is also no need to
 * alter_display, since only the port is modified.
 *
 * @param driver_handle from igd_init().
 * @param port_number
 * @param power_state
 *
 * @return 0 on success
 * @return -IGD_INVAL on failure
 */
int igd_power_display(igd_driver_h driver_handle,
	unsigned short port_number,
	unsigned int power_state)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t *display;

	EMGD_TRACE_ENTER;
	EMGD_DEBUG("Requested power state = %d", power_state);

	/* Get the display context that is currently using this port.  */
	display = context->mod_dispatch.dsp_display_list[port_number];
	if(!display) {
		return -IGD_ERROR_INVAL;
	}

	/* If this display is allocated, but has not been altered, return
	 * an error. */
	if (!PORT(display, port_number)->pt_info || !PIPE(display)->timing) {
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}

	/* Set the desired power state to the display handle and let program_port
	 * take care of the rest
	 */
	PORT(display, port_number)->power_state = (unsigned long)power_state;

	switch(power_state) {
	case IGD_POWERSTATE_D0:
		mode_context->dispatch->program_port(display, port_number, TRUE);
		mode_context->dispatch->post_program_port(display, port_number, 0);
		break;
	case IGD_POWERSTATE_D1:
	case IGD_POWERSTATE_D2:
	case IGD_POWERSTATE_D3:
		mode_context->dispatch->program_port(display, port_number, TRUE);
		break;
	default:
		break;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param driver_handle from igd_init().
 * @param port_number
 * @param edid_ptr
 * @param block_number
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL or -IGD_ERROR_EDID on failure
 */
/* FIXME: Move this to PI */
static int igd_get_EDID_block(igd_driver_h driver_handle,
		unsigned short port_number,
		unsigned char FAR *edid_ptr,
		unsigned char block_number)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_port_t *port;
	int                   ret;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT(driver_handle, "Null driver_handle", -IGD_ERROR_INVAL);
	EMGD_ASSERT(edid_ptr, "Null edid_ptr", -IGD_ERROR_INVAL);

	port = context->mod_dispatch.dsp_port_list[port_number];
	if(!port) {
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_INVAL;
	}
	/* Read EDID */
	ret = context->mod_dispatch.i2c_read_regs(
		context,
		port->ddc_reg,
		10,              /* DDC speed 10 KHz */
		port->ddc_dab,
		128*block_number,
		edid_ptr,
		128,
		0);

	if (ret) {
		EMGD_TRACE_EXIT;
		return -IGD_ERROR_EDID;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end igd_get_EDID_block() */

/*!
 * Return either a pointer to the live mode list or a copy of the mode list
 * for the requested display. This will be the mode list for the master port
 * on the pipe.
 *
 * @note Currently (AS of 3.3 development) the mode list is
 * described as a igd_display_info_t. However IT IS NOT, this
 * pointer must be cast to a igd_timing_info_t to be used. After
 * 3.3 the igd_display_info_t will be altered to match the
 * igd_timing_info_t with the exception of the private pointers.
 *
 * @param driver_handle handle returned from a successful call to
 * 	igd_driver_init().
 * @param dc Display configuration that will determine which port
 * 	controlls the pipe timings and thus, which set of timings to return.
 * @param mode_list The returned mode list. This data may be LIVE. If
 * 	a live list is returned, care should be taken to not free or alter
 * 	the data.
 * @param flags The flags will determine which display to query (primary
 * 	or secondary) and if the mode list returned should be the live list.
 *
 * @return 0 on success.
 * @return -IGD_INVAL if an error occured (memory allocation failed)
 */
int igd_query_mode_list(igd_driver_h driver_handle,
		unsigned long dc,
		igd_display_info_t **mode_list,
		unsigned long flags)
{
	igd_context_t *context;
	unsigned short port_number;
	igd_display_port_t *port;

	EMGD_TRACE_ENTER;

	context = (igd_context_t *)driver_handle;
	*mode_list = NULL;

	/* given the DC and flags, which port number to check? */
	port_number = (flags & IGD_QUERY_SECONDARY_MODES) ? DC_PORT_NUMBER(dc, 5) :
		DC_PORT_NUMBER(dc, 1);

	port = context->mod_dispatch.dsp_port_list[port_number];
	if (port) {
		if (flags & IGD_QUERY_LIVE_MODES) {
			/*
			 * FIXME:
			 * timing_table is not an igd_dislay_info_t structure but
			 * eventually it will be?
			 */
			*mode_list = (igd_display_info_t *)port->timing_table;
		} else {
			OPT_MICRO_CALL(full_mode_query(driver_handle, dc, mode_list,
					port));
		}
	}

	if (*mode_list == NULL) {
		EMGD_DEBUG("No port on requested pipe");
		return -IGD_ERROR_INVAL;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * This function is used to initialize any module/dsp
 * module specific structures or tables etc.
 *
 * @param context SS level igd_context.
 *
 * @return 0 on success.
 * @return -IGD_INVAL or -IGD_ERROR_NODEV on failure
 */
int mode_init(igd_context_t *context)
{
	igd_dispatch_t     *dispatch = &context->dispatch;
	inter_module_dispatch_t *md;
	int port_num;
	int i;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("Allocating a mode context...");

	/* Clear the allocated memory for mode context */
	OS_MEMSET((void *)mode_context, 0, sizeof(mode_context_t));

	/* Set the pointer to igd level context */
	mode_context->context = context;
	mode_context->first_alter = TRUE;
	mode_context->display_color =
		context->mod_dispatch.init_params->display_color;
#ifndef CONFIG_MICRO
	mode_context->ref_freq =
		context->mod_dispatch.init_params->ref_freq;
	mode_context->tuning_wa =
		context->mod_dispatch.init_params->tuning_wa;
	/*To give option for validation*/
	mode_context->clip_hw_fix =
		context->mod_dispatch.init_params->clip_hw_fix;
	mode_context->async_flip_wa =
		context->mod_dispatch.init_params->async_flip_wa;
	mode_context->en_reg_override =
		context->mod_dispatch.init_params->en_reg_override;
	mode_context->disp_arb =
		context->mod_dispatch.init_params->disp_arb;
	mode_context->fifo_watermark1 =
		context->mod_dispatch.init_params->fifo_watermark1;
	mode_context->fifo_watermark2 =
		context->mod_dispatch.init_params->fifo_watermark2;
	mode_context->fifo_watermark3 =
		context->mod_dispatch.init_params->fifo_watermark3;
	mode_context->fifo_watermark4 =
		context->mod_dispatch.init_params->fifo_watermark4;
	mode_context->fifo_watermark5 =
		context->mod_dispatch.init_params->fifo_watermark5;
	mode_context->fifo_watermark6 =
		context->mod_dispatch.init_params->fifo_watermark6;
	mode_context->gvd_hp_control =
		context->mod_dispatch.init_params->gvd_hp_control;
	mode_context->bunit_chicken_bits =
		context->mod_dispatch.init_params->bunit_chicken_bits;
	mode_context->bunit_write_flush =
		context->mod_dispatch.init_params->bunit_write_flush;
	mode_context->disp_chicken_bits =
		context->mod_dispatch.init_params->disp_chicken_bits;
#endif

	for (i=0; i < IGD_MAX_PORTS; i++) {
		port_num = context->mod_dispatch.init_params->display_params[i].port_number;
		mode_context->batch_blits[port_num - 1] =
			(context->mod_dispatch.init_params->display_params[i].flags
			& IGD_DISPLAY_BATCH_BLITS);
	}

	/* Get mode's dispatch table */
	mode_context->dispatch = (mode_dispatch_t *)
		dispatch_acquire(context, mode_dispatch);
	if(!mode_context->dispatch) {
		EMGD_ERROR_EXIT("Unsupported Device");
		return -IGD_ERROR_NODEV;
	}

	/* Hook up KMS dispatch table */
	mode_context->kms_dispatch = (mode_kms_dispatch_t *)
		dispatch_acquire(context, mode_kms_dispatch);
	if(!mode_context->kms_dispatch) {
		EMGD_ERROR_EXIT("Unsupported Device");
		return -IGD_ERROR_NODEV;
	}


	md = &context->mod_dispatch;

	/* Set the fw_info to 0 */
	mode_context->fw_info = NULL;

	/* Hook up the IGD dispatch table entires for mode */
	dispatch->get_EDID_block = igd_get_EDID_block;
	dispatch->power_display = igd_power_display;
	dispatch->query_mode_list = igd_query_mode_list;
	dispatch->alter_displays = igd_alter_displays;
	dispatch->igd_configure_display = igd_configure_display;

	OPT_MICRO_CALL(full_mode_init(context, mode_context));

	/* Hook up inter-module dispatch functions */
	md->mode_get_gpio_sets = mode_context->dispatch->get_gpio_sets;
	md->mode_reset_plane_pipe_ports =
		mode_context->dispatch->reset_plane_pipe_ports;
	md->filter_modes = mode_context->dispatch->filter_modes;

	/* Hook up Core specific IGD dispatch table entries */
	dispatch->set_palette_entries =
		mode_context->dispatch->full->set_palette_entries;
	dispatch->set_palette_entry = mode_context->dispatch->set_palette_entry;
	dispatch->get_palette_entry = mode_context->dispatch->get_palette_entry;
	dispatch->wait_vblank = mode_context->dispatch->wait_vblank;

	/* Initialize dsp module */
	if (dsp_init(context)) {
		EMGD_ERROR("dsp_init() failed.");
		return -IGD_INVAL;
	}

	/* Initialze port interface (pi) module */
	if (pi_init(context)) {
		EMGD_ERROR_EXIT("pi_init() failed.");
		if(md->dsp_shutdown) {
			md->dsp_shutdown(context);
		}
		return -IGD_ERROR_INVAL;
	}

	if (mode_context->dispatch->full && md->reg_get_mod_state) {
		module_state_h *state = NULL;

		/* Save mode state for the regular case*/
		unsigned long *flags = NULL;
		md->reg_get_mod_state(REG_MODE_STATE_REG, &state, &flags);
		md->mode_save(context, state, flags);

		/*Save mode state for the console case */
		if (config_drm.init) {
			state = NULL;
			md->reg_get_mod_state(REG_MODE_STATE_CON, &state, &flags);
			md->mode_save(context, state, flags);
		}
	}

	toggle_vblank_interrupts(TRUE);
	/* Initialize the Display Configuration List */
	/* FIXME: This should be done in dsp init */
	dsp_dc_init(context);

	EMGD_TRACE_EXIT;
	return 0;
}

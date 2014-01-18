/*
 *-----------------------------------------------------------------------------
 * Filename: igd_ovl.c
 * $Revision: 1.28 $
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

#define MODULE_NAME hal.overlay

#include <context.h>
#include <memory.h>
#include <dsp.h>
#include <io.h>

#include <igd_mode.h>
#include <igd_gmm.h>
#include <igd_ovl.h>
#include <igd_errno.h>
#include <igd_pwr.h>
#include <igd_render.h>
#include <igd_blend.h>

#include <general.h>
#include <mode_access.h>
#include <dispatch.h>
#include <intelpci.h>

#include "ovl_dispatch.h"
#include "ovl_virt.h"

void _overlay_shutdown(igd_context_t *context);


extern ovl_dispatch_t ovl_dispatch_plb[];
extern ovl_dispatch_t ovl_dispatch_tnc[];

static dispatch_table_t ovl_dispatch_list[] = {

#ifdef CONFIG_PLB
	{PCI_DEVICE_ID_VGA_PLB, &ovl_dispatch_plb},
#endif
#ifdef CONFIG_TNC
	{PCI_DEVICE_ID_VGA_TNC, &ovl_dispatch_tnc},
#endif

	{0, NULL}
};

ovl_context_t ovl_context[1];

/* Description: Turns off video plane that was turned by fw/EFI. To be called by
 * alter_ovl if needed.
 *
 * Notes:Upon video splash being turned on, in order to reset it, need to call
 * secondary ovl dispatch function to turn off the right registers
 */
void igd_reset_fw_ovl(igd_display_context_t *display)
{
	ovl_dispatch_t	*ovl_dispatch = (ovl_dispatch_t *)ovl_context->dispatch;
	if(NULL == ovl_dispatch){
		EMGD_DEBUG("ovl_dispatch_t	*ovl_dispatch == NULL");
		return ;
	}
	ovl_dispatch[OVL_SECONDARY].alter_ovl(display,
								NULL, NULL, NULL, NULL, IGD_OVL_ALTER_OFF);
}

static int igd_ovl_set_display(igd_display_h ovl_display[])
{
	ovl_context->ovl_display_km[OVL_PRIMARY] = (igd_display_context_t *)ovl_display[OVL_PRIMARY];
	ovl_context->ovl_display_km[OVL_SECONDARY] = (igd_display_context_t *)ovl_display[OVL_SECONDARY];

	ovl_context->ovl_display_swapped = 1;
	return 0;
}

static int ovl_get_display(igd_display_context_t *display,
	igd_display_context_t *(ovl_displays[]),
	unsigned long flags)
{
	igd_display_context_t *primary, *secondary;
	unsigned long dc;

	display->context->mod_dispatch.dsp_get_dc(&dc, &primary, &secondary);

	/* This is a special case  to enable Dual Overlay */
	if(flags & IGD_OVL_FORCE_USE_DISP) {

		/* Use the passed in Display Handle */
		if(display == primary) {

			ovl_displays[OVL_PRIMARY]   = display;
			ovl_displays[OVL_SECONDARY] = NULL;

		} else if(display == secondary) {

			ovl_displays[OVL_PRIMARY]   = NULL;
			ovl_displays[OVL_SECONDARY] = display;
		}

		return 0;
	}

	if (dc & IGD_DISPLAY_CONFIG_CLONE || display->context->mod_dispatch.in_dih_clone_mode) {
		/* CLONE or Vertical Extended
		 * Primary Overlay uses the display from the primary pipe.
		 * Secondary Overlay uses the display from the secondary pipe. */
		/* The above is TRUE on init, but subsequent 
		 * video plane override calls will change the overlay plane assignment.
		 * Use ovl_um_context->ovl_display_km[OVL_PRIMARY] & 
		 * ovl_um_context->ovl_display_km[OVL_SECONDARY] to retrieve
		 * HW overlay(PRIMARY) & SpriteC(SECONDARY) plane to display 
		 * handle assignment respectively */
		ovl_displays[OVL_PRIMARY]   = ovl_context->ovl_display_km[OVL_PRIMARY];
		ovl_displays[OVL_SECONDARY] = ovl_context->ovl_display_km[OVL_SECONDARY];
	} else {
		/* Single, Twin, Extended */
		if (ovl_context->ovl_display_km[OVL_PRIMARY] == display) {
			ovl_displays[OVL_PRIMARY]   = display;
			ovl_displays[OVL_SECONDARY] = NULL;
		} else {
			ovl_displays[OVL_PRIMARY]   = NULL;
			ovl_displays[OVL_SECONDARY] = display;
		}
	}

	return 0;
}

static int igd_alter_ovl2(igd_display_h display_h,
	igd_surface_t       *src_surf,
	igd_rect_t          *src_rect,
	igd_rect_t          *dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	igd_display_context_t *display =
		(igd_display_context_t *)display_h;
	ovl_dispatch_t    *ovl_dispatch =
		(ovl_dispatch_t *)ovl_context->dispatch;
	int cur_ovl, ret = 0;
	igd_display_context_t *primary, *secondary;
	unsigned long dc;
	igd_display_context_t *ovl_displays[OVL_MAX_HW];

	EMGD_TRACE_ENTER;

	if(NULL == ovl_dispatch) {
		EMGD_DEBUG("ovl_dispatch == NULL");
		return IGD_SUCCESS;
	}

	if(flags == IGD_FW_VIDEO_OFF)
	{
		igd_reset_fw_ovl(display);
		return IGD_SUCCESS;
	}

	/* Determine which display this overlay belongs to */
	if(display->context == NULL){
		EMGD_DEBUG("display->context == NULL");
		return -IGD_ERROR_INVAL;
	}
	display->context->mod_dispatch.dsp_get_dc(&dc, &primary, &secondary);

	if ((ovl_context->ovl_display_km[OVL_PRIMARY] == NULL) &&
		(ovl_context->ovl_display_km[OVL_SECONDARY] == NULL)) {
		/* One time initialization of ovl_display_km */
		ovl_displays[OVL_PRIMARY] = primary;
		ovl_displays[OVL_SECONDARY] = secondary;
		igd_ovl_set_display((igd_display_h)ovl_displays);
	}

	/* Determine which overlays belong to which displays */
	ovl_get_display(display, ovl_displays, flags);

	/* Determine which display this overlay belongs to */
	if(display == ovl_displays[OVL_PRIMARY]) {
		cur_ovl = 0;
	} else if (display == ovl_displays[OVL_SECONDARY]) {
		cur_ovl = 1;
	} else {
		/* shouldn't get here. */
		EMGD_TRACE_EXIT;
		return  -IGD_ERROR_INVAL;
	}


	/* Is the overlay is being turned off? */
	if ((flags & IGD_OVL_ALTER_ON) == IGD_OVL_ALTER_OFF) {

		/* Call family dependent overlay
		 * function to alter the overlay. */
		ret = ovl_dispatch[cur_ovl].alter_ovl(display,
						  src_surf,
						  src_rect,
						  dest_rect,
						  ovl_info,
						  flags);

		EMGD_TRACE_EXIT;
		return ret;
	}

	/* Call family dependent overlay function
	 * to alter the overlay. */
	ret = ovl_dispatch[cur_ovl].alter_ovl(display,
                                                      src_surf,
                                                      src_rect,
                                                      dest_rect,
                                                      ovl_info,
					      flags);

	if (ret != IGD_SUCCESS) {
		/* Turn the overlay off when there is an error */
		ovl_dispatch[cur_ovl].alter_ovl(display,
						NULL, NULL, NULL, NULL,
						IGD_OVL_ALTER_OFF);
	}

	EMGD_TRACE_EXIT;
	return ret;
}



static int igd_query_ovl(igd_display_h display_h,
	unsigned int flags)
{
	igd_display_context_t *display =
		(igd_display_context_t *)display_h;
	ovl_dispatch_t    *ovl_dispatch =
		(ovl_dispatch_t *)ovl_context->dispatch;
	int           cur_ovl;
	igd_display_context_t *(ovl_displays[OVL_MAX_HW]); /* Array of pointers */
	int ret = FALSE;

	ovl_get_display(display, ovl_displays, flags);

	/* This is a bit of a short circuit (since the hardware dependent functions
	 * are not being called), but to determine if the hardware overlay is on,
	 * just check the state of the ovl_context. */
	if (flags == IGD_OVL_QUERY_IS_ON) {
		if (ovl_context->state == OVL_STATE_ON) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	/* NOTE: As with alter_ovl2, user mode driver should decide which ovl
	 * to flip. Instead of having a duplicate logic here to check for clone,
	 * just flip the ovl bound to the display handle. This prevents a condition
	 * in clone mode, where the user mode driver does 2 query_ovl calls for 
	 * each ovl plane, that translates into 4 query_ovl calls in kernel.
	 */

	/* Determine which display this overlay belongs to */
	if(display == ovl_displays[OVL_PRIMARY]) {
		cur_ovl = 0;
	} else if (display == ovl_displays[OVL_SECONDARY]) {
		cur_ovl = 1;
	} else {
		/* shouldn't get here. */
		EMGD_TRACE_EXIT;
		return  -IGD_ERROR_INVAL;
	}

	ret = ovl_dispatch[cur_ovl].query_ovl(
		(igd_display_h)(ovl_displays[cur_ovl]),
		(flags & IGD_OVL_QUERY_MASK));
	if (ret == FALSE) {
		/* Can only return TRUE (event has occured and capability
		 * is available) if it is TRUE for all displays */
		return FALSE;
	}

	return ret;
}
static int igd_query_max_size_ovl(igd_display_h display_h,
	unsigned long pf,
	unsigned int *max_width,
	unsigned int *max_height)
{
	igd_display_context_t *display =
		(igd_display_context_t *)display_h;
	ovl_dispatch_t    *ovl_dispatch =
		(ovl_dispatch_t *)ovl_context->dispatch;
	int           cur_ovl;
	igd_display_context_t *(ovl_displays[OVL_MAX_HW]); /* Array of pointers */
	unsigned int tmp_max_width, tmp_max_height;
	int ret = -IGD_ERROR_INVAL;

	*max_width =  0x10000000;
	*max_height = 0x10000000;

	/* pass a dummy flag */
	ovl_get_display(display, ovl_displays, 0);

	for (cur_ovl = 0; cur_ovl < OVL_MAX_HW; cur_ovl++) {
		if (ovl_displays[cur_ovl] != NULL) {
			ret = ovl_dispatch[cur_ovl].query_max_size_ovl(
				(igd_display_h)(ovl_displays[cur_ovl]), pf,
				&tmp_max_width, &tmp_max_height);
			if (ret != IGD_SUCCESS) {
				/* Can only return IGD_SUCCESS (no error)
				 * if there is no error for all displays */
				return ret;
			}
			if (tmp_max_width < *max_width) {
				*max_width = tmp_max_width;
			}
			if (tmp_max_height < *max_height) {
				*max_height = tmp_max_height;
			}
		}
	}

	return ret;
}



/* This is a wrapper function that will call the device specific alter_ovl2_osd 
 * function. Currently this function is used by video driver to map the subpicture 
 * surface to second overlay so that it blends with the video surface on first 
 * overlay. Furthermore, this is only enabled for PLB device only. 
 */
static int igd_alter_ovl2_osd(igd_display_h display_h,
	igd_surface_t *sub_surface,
	igd_rect_t *sub_src_rect,
	igd_rect_t *sub_dest_rect,
	igd_ovl_info_t      *ovl_info,
	unsigned int         flags)
{
	ovl_dispatch_t  *ovl_dispatch =
		(ovl_dispatch_t *)ovl_context->dispatch;
	igd_display_context_t *display =
		(igd_display_context_t *)display_h;

	int ret = 0;

	ret = ovl_dispatch[1].alter_ovl(display,
			sub_surface, sub_src_rect, sub_dest_rect,
			ovl_info, flags);

	return ret;
}



/*----------------------------------------------------------------------
 * Function: igd_get_ovl_init_params(ovl_um_context_t *ovl_um_context)
 * Description:
 *
 * Notes in Usage:
 * the user mode caller fills in the primary and secondary display handles.
 *
 *----------------------------------------------------------------------*/
int igd_get_ovl_init_params(igd_driver_h driver_handle,
			    ovl_um_context_t *ovl_um_context)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t *primary, *secondary;
	unsigned long dc;

	context->mod_dispatch.dsp_get_dc(&dc, &primary, &secondary);
	ovl_um_context->dc = dc;
	ovl_um_context->primary = primary;
	ovl_um_context->secondary = secondary;
    ovl_um_context->chiptype = 0;

    return IGD_SUCCESS;
}



int ovl_full_init(igd_context_t *context,
			igd_param_t *params,
			ovl_context_t *ovl_context)
{
	unsigned long tmp_reg_size;
	unsigned long region_type = IGD_GMM_REGION_TYPE_OVLREG;
	void *virt;

	if(context->device_context.did == PCI_DEVICE_ID_VGA_TNC){
		/* Atom E6xx needs the overlay update register offset
		 * to be a 64K aligned address. */
		region_type = IGD_GMM_REGION_TYPE_OVLREG64;
	}

	/* Allocate a 4K page aligned region from
	 * memory manager for overlay register update memory */

	tmp_reg_size = 4096;
	GMM_SET_DEBUG_NAME("Overlay Register Buffer");
	if(context->dispatch.gmm_alloc_region(
		   &(ovl_context->reg_update_offset),
		   &tmp_reg_size,
		   IGD_GMM_REGION_TYPE_OVLREG,
		   0)){

		EMGD_ERROR_EXIT("Memory allocation for "
			"Overlay Register Update failed");

		ovl_context->reg_allocated = 0;

		return -IGD_ERROR_NOMEM;
	}
	ovl_context->reg_allocated = 1;
	ovl_context->ovl_display_swapped = 0;

	/* Get the register update physical address in RAM */
	if(context->dispatch.gmm_virt_to_phys(
		   ovl_context->reg_update_offset,
		   &ovl_context->reg_update_phys)) {

		EMGD_ERROR_EXIT("Virtual to Physical Address translation failed");
		return -IGD_ERROR_NOMEM;
	}

	/*
	 * TODO: Verify that phys_to_virt returns a valid address for
	 * agp memory
	 */
	virt = phys_to_virt(ovl_context->reg_update_phys);

	/* Clear the register update page */
	OS_MEMSET(virt, 0, tmp_reg_size);


	/* Get overlay's dispatch table */
	ovl_context->dispatch = (ovl_dispatch_t (*)[])dispatch_acquire(context,
		ovl_dispatch_list);
	if(!ovl_context->dispatch) {
		EMGD_ERROR_EXIT("Unsupported Device");
		return -IGD_ERROR_NODEV;
	}


	/* Hook up the IGD dispatch table entries for overlay
	 * Alter has a common function, query can call the family function
	 * directly */
	context->dispatch.get_ovl_init_params = igd_get_ovl_init_params;
	context->dispatch.alter_ovl = NULL;
	context->dispatch.alter_ovl2 = igd_alter_ovl2;
//	context->dispatch.alter_ovl2_dihclone = igd_alter_ovl2_dihclone;
	context->dispatch.query_ovl = igd_query_ovl;
	context->dispatch.query_max_size_ovl = igd_query_max_size_ovl;
	context->dispatch.alter_ovl2_osd = igd_alter_ovl2_osd;
	context->dispatch.set_ovl_display = igd_ovl_set_display;

	/* Hook up optional inter-module functions */
	context->mod_dispatch.overlay_shutdown = _overlay_shutdown;

	/* Initialize any OS / Chipst general HAL params for overlay */
	if(params->display_flags & IGD_DISPLAY_FB_BLEND_OVL){
		ovl_context->fb_blend_ovl = 1;
	}
	ovl_context->saved_src_surf = NULL;
	ovl_context->saved_src_rect = NULL;
	ovl_context->saved_dest_rect = NULL;
	ovl_context->saved_ovl_info = NULL;
	ovl_context->saved_flags	=  0;
	ovl_context->ovl_display_km[OVL_PRIMARY] = NULL;
	ovl_context->ovl_display_km[OVL_SECONDARY] = NULL;

	
	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

void _overlay_shutdown(igd_context_t *context)
{

	EMGD_TRACE_ENTER;

	if (ovl_context->reg_allocated) {
		context->dispatch.gmm_free(ovl_context->reg_update_offset);
		ovl_context->reg_update_offset = 0;
		ovl_context->reg_allocated = 0;
	}

	EMGD_TRACE_EXIT;

	return;
}

/*----------------------------------------------------------------------
 * Function: igd_overlay_pwr()
 * Description:
 *  igd_overlay_pwr will only be called from power module .
 *  It shuts down / powers up overlay output based on the power
 *  state transition requested.
 *
 * Notes in Usage:
 *
 *----------------------------------------------------------------------*/
int igd_overlay_pwr(
	igd_driver_h driver_handle,
	int power_state)
{
	igd_context_t *context = (igd_context_t *)driver_handle;
	igd_display_context_t *primary, *secondary;

	/* NOTE: The overlay will not be turned on by this method, but will be
	 * turned on with the next call to alter_ovl */

	/* Turn off the overlay for every display.  Most of the displays will
	 * likely not have the overlay off, but there should be no harm it turning
	 * it off for a display which does not have the overlay. */

	EMGD_TRACE_ENTER;
	if(power_state != IGD_POWERSTATE_D0){
		context->mod_dispatch.dsp_get_dc(NULL, &primary, &secondary);

		if(primary) {
			/* Turn the overlay off. */
			igd_alter_ovl2((igd_display_h)primary,
				 NULL, NULL, NULL, NULL, IGD_OVL_ALTER_OFF);
		}
		if(secondary) {
			igd_alter_ovl2((igd_display_h)secondary,
				 NULL, NULL, NULL, NULL, IGD_OVL_ALTER_OFF);
		}
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

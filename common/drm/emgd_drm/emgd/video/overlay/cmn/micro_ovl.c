/*
 *-----------------------------------------------------------------------------
 * Filename: micro_ovl.c
 * $Revision: 1.13 $
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
//#include <igd_ovl.h>
#include <igd_pwr.h>
#include <igd_render.h>
#include <igd_blend.h>

#include <general.h>
#include <mode_access.h>
#include <dispatch.h>
#include <intelpci.h>

#include "ovl_dispatch.h"
#include "ovl_virt.h"

/*
 * TODO: If there is micro overlay for device dependent layer
 * modify extern below to the correct micro device dispatch
 * Only two micro overlay existing is for Plb and Ctg
 */

extern ovl_dispatch_t ovl_micro_dispatch_plb[];

static dispatch_table_t ovl_micro_dispatch_list[] = {
/*
 * TODO: Fix this.
 *  * This table would point the micro dispatch tables.
 *  For now, other than Plb ,refer the micro dispatch
 *  to the original table
 * */

#ifdef CONFIG_PLB
	{PCI_DEVICE_ID_VGA_PLB, &ovl_micro_dispatch_plb},
#endif
#ifdef CONFIG_TNC
	{PCI_DEVICE_ID_VGA_TNC, &ovl_micro_dispatch_plb},
#endif
	{0, NULL}
};
extern int ovl_full_init(igd_context_t *context, igd_param_t *params,ovl_context_t *ovl_context);

#ifndef CONFIG_MICRO_OVERLAY
#define OVL_FULL_INIT(a, b, c) ovl_full_init(a, b, c)
#else
#define OVL_FULL_INIT(a, b, c) 1
#endif

extern ovl_context_t ovl_context[1];
static int igd_micro_alter_ovl(igd_display_h display_h,
	igd_appcontext_h     appcontext_h,
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

	igd_display_context_t *ovl_display;

	int ret = 0;

	EMGD_TRACE_ENTER;

	ovl_display = display;

	/* Call family dependent overlay function to alter the overlay. */
	ret = ovl_dispatch->alter_ovl(
				ovl_display, src_surf, src_rect,
				dest_rect, ovl_info, flags);

	if (ret != IGD_SUCCESS) {
		EMGD_ERROR("Error micro alter overlay\n");
		/* Turn the overlay off when there is an error */
		ovl_dispatch->alter_ovl(ovl_display,
					NULL, NULL, NULL, NULL, IGD_OVL_ALTER_OFF);
	}

	EMGD_TRACE_EXIT;
	return ret;
}

int _overlay_init(igd_context_t *context, igd_param_t *params)
{

	EMGD_TRACE_ENTER;

	/* Ensure the device context pointer is valid */
	if(!context){
		EMGD_ERROR_EXIT("Error: Null context");
		return -IGD_ERROR_INVAL;
	}

	/* Clear the allocated memory for overlay context */
	OS_MEMSET((void *)ovl_context, 0, sizeof(ovl_context_t));

	/* Get overlay's dispatch table */
	ovl_context->dispatch = (ovl_dispatch_t (*)[])dispatch_acquire(context,
		ovl_micro_dispatch_list);
	if(!ovl_context->dispatch) {
		EMGD_ERROR_EXIT("Error: Unsupported Device");
		return -IGD_ERROR_NODEV;
	}


	/* Hook up the IGD dispatch table entries for overlay
	 * Alter has a common function, query can call the family function
	 * directly */
	context->dispatch.alter_ovl = igd_micro_alter_ovl;
	context->dispatch.query_ovl = NULL;
	context->dispatch.query_max_size_ovl = NULL;
	context->mod_dispatch.overlay_shutdown = NULL;

	if (OVL_FULL_INIT(context, params, ovl_context)) {
		/*
		 * Even if full init failed we can still use the micro-gmm
		 */
		EMGD_DEBUG("Full overlay did not initialize, using micro-overlay");

	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

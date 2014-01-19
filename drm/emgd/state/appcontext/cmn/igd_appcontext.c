/*
 *-----------------------------------------------------------------------------
 * Filename: igd_appcontext.c
 * $Revision: 1.10 $
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
 *  This file inplements the IGD interface for the context module. This
 *  includes functions that handle hardware context swapping and state
 *  variable management.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.state

#include <config.h>

#include <igd_appcontext.h>
#include <igd_gmm.h>
#include <igd_mode.h>
#include <igd_errno.h>

#include <intelpci.h>
#include <context.h>
#include <mode.h>
#include <dispatch.h>

#include <io.h>

#include "appcontext_dispatch.h"

/*!
 * @addtogroup state_group
 * @{
 */

static dispatch_table_t appcontext_dispatch_list[] = {
#ifdef CONFIG_PLB
	{PCI_DEVICE_ID_VGA_PLB, &appcontext_dispatch_plb},
#endif
#ifdef CONFIG_TNC
	{PCI_DEVICE_ID_VGA_TNC, &appcontext_dispatch_plb},
#endif
	{0, NULL}
};

/* Not static: cores may need it */
appcontext_dispatch_t *appcontext_dispatch = NULL;

/*!
 *
 * @param display_handle
 * @param priority
 * @param flags
 *
 * @return appcontext_h
 */
igd_appcontext_h igd_appcontext_alloc(
	igd_display_h display_handle,
	int priority,
	unsigned int flags)
{

	return appcontext_dispatch->appcontext_alloc(
		display_handle, priority, flags);
}

/*!
 *
 * @param display_handle
 * @param priority
 * @param appcontext_handle
 *
 * @return void
 */
void igd_appcontext_free(
	igd_display_h display_handle,
	int priority,
	igd_appcontext_h appcontext_handle)
{
	appcontext_dispatch->appcontext_free(
		display_handle, priority, appcontext_handle);
}

/*!
 *
 * @param context
 *
 * @return void
 */
void appcontext_shutdown(igd_context_t *context)
{

	EMGD_DEBUG("appcontext_shutdown Entry");

	if (appcontext_dispatch != NULL)
	{
		appcontext_dispatch->appcontext_shutdown(context);
	}
}

/*!
 *
 * @param context
 *
 * @return void
 */
int appcontext_init(igd_context_t *context)
{

	EMGD_DEBUG("appcontext_init Entry");

	appcontext_dispatch = (appcontext_dispatch_t *)dispatch_acquire(context,
		appcontext_dispatch_list);
	if(!appcontext_dispatch) {
		return -IGD_ERROR_NODEV;
	}

	/* Hook up top level dispatch table */
	context->dispatch.appcontext_alloc = appcontext_dispatch->appcontext_alloc;
	context->dispatch.appcontext_free = appcontext_dispatch->appcontext_free;

	/* Hook up optional inter-module functions */
	context->mod_dispatch.appcontext_shutdown = appcontext_shutdown;

	return 0;
}


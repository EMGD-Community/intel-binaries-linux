/*
 *-----------------------------------------------------------------------------
 * Filename: appcontext.h
 * $Revision: 1.6 $
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
 *  This is the inter-module header file for the client context module.
 *  It contains data structures needed for modules to use and manipulate
 *  the device state including hardware context switching.
 *-----------------------------------------------------------------------------
 */

#ifndef _APPCONTEXT_PLB_H
#define _APPCONTEXT_PLB_H

#include <context.h>
#include <igd_mode.h>

/*
 * This data structure contains a copy of all needed state variables and
 * the logical context used by hardware for context switching. An IGD
 * client driver can allocate a client context for each client or allocate
 * just one to be shared by all clients.
 */
/*
 * TODO:
 * Add or modify the struct and other definitions for Poulsbo
 */
typedef struct _appcontext {
	void *state3d;
	void *state2d;
	unsigned char *hw_context_virt;
	unsigned long hw_context_phys;
	unsigned long hw_context_offset;
}appcontext_t, appcontext_plb_t;

int appcontext_set_plb(igd_display_h display,
	int priority,
	appcontext_t *context,
	int extstate_save_enable,
	int extstate_restore_enable,
	int force_restore,
	int restore_inhibit);


#endif

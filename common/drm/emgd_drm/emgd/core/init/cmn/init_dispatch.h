/*
 *-----------------------------------------------------------------------------
 * Filename: init_dispatch.h
 * $Revision: 1.8 $
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

#ifndef _INIT_DISPATCH_H
#define _INIT_DISPATCH_H

#include <pci.h>

#include <dispatch.h>

/*
 * Note: Platforms extend this data structure so the pointer can be used
 * as either this DI dispatch or cast to the DD dipatch.
 */
typedef struct _init_dispatch {
	char *name;
	char *chipset;
	char *default_pd_list;
	int (*query)(igd_context_t *context, struct _init_dispatch *dispatch,
		os_pci_dev_t vga_dev, unsigned int *bus, unsigned int *slot,
		unsigned int *func);
	int (*config)(igd_context_t *context, struct _init_dispatch *dispatch);
	int (*set_param)(igd_context_t *context, unsigned long id,
		unsigned long value);
	int (*get_param)(igd_context_t *context, unsigned long id,
		unsigned long *value);
	void (*shutdown)(igd_context_t *context);
	int (*query_2d_caps_hwint) (igd_context_t *context,
		unsigned long caps_val, unsigned long *status);
} init_dispatch_t;

extern init_dispatch_t init_dispatch_plb;
extern init_dispatch_t init_dispatch_tnc;
extern init_dispatch_t init_dispatch_tnc_a0;
extern init_dispatch_t init_dispatch_lnc;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: appcontext_dispatch.h
 * $Revision: 1.7 $
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


#ifndef _APPCONTEXT_DISPATCH_H
#define _APPCONTEXT_DISPATCH_H

typedef struct _appcontext_table_t {
	unsigned long appcontext_id;
	unsigned long logical_context_offset;
} appcontext_table_t;

typedef struct _appcontext_dispatch_t {
	int (*appcontext_init)(igd_context_t *context);
	void (*appcontext_shutdown)(igd_context_t *context);
	igd_appcontext_h (*appcontext_alloc)(igd_display_h display_handle,
		int priority, unsigned int flags);
	void (*appcontext_free)(igd_display_h display_handle,
		int priority, igd_appcontext_h context_handle);
	appcontext_table_t *table;
} appcontext_dispatch_t;


extern appcontext_dispatch_t appcontext_dispatch_plb;

extern appcontext_dispatch_t *appcontext_dispatch;

#endif

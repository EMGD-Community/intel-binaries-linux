/*
 *-----------------------------------------------------------------------------
 * Filename: dispatch.h
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
 *  This file contains helper functions for creating a dispatch table
 *  for IGD modules.
 *-----------------------------------------------------------------------------
 */

#ifndef _HAL_DISPATCH_H
#define _HAL_DISPATCH_H

#include <context.h>

typedef struct _dispatch_table {
	unsigned long did;
	void *table;
}dispatch_table_t;

static __inline void *dispatch_acquire(
	igd_context_t *context,
	dispatch_table_t *list)
{
	dispatch_table_t *curr = list;
	unsigned long did = context->device_context.did;

	while(curr->did != 0) {
		if(curr->did == did) {
			return curr->table;
		}
		curr++;
	}
	return curr->table;
}

#endif

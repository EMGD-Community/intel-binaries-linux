/*
 *-----------------------------------------------------------------------------
 * Filename: dispatch_utils.h
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
 *  This file implements common macros that are used throughout the HAL
 *  for Device Dependent dispatching. This file is used in both the HAL and
 *  HAL helper libraries, as such it cannot include any HAL data structures
 *  that are not available to the helper libraries.
 *-----------------------------------------------------------------------------
 */

#ifndef _DISPATCH_UTILS_H_
#define _DISPATCH_UTILS_H_

#include <config.h>
#include <intelpci.h>

typedef struct _dispatch_table {
	unsigned long did;
	void *table;
}dispatch_table_t;

static __inline void *dispatch_acquire(
	unsigned long did,
	dispatch_table_t *list)
{
	dispatch_table_t *curr = list;

	while(curr->did != 0) {
		if(curr->did == did) {
			return curr->table;
		}
		curr++;
	}
	return curr->table;
}


#ifdef CONFIG_PLB
#define DISPATCH_PLB(p) {PCI_DEVICE_ID_VGA_PLB, p},
#else
#define DISPATCH_PLB(p)
#endif
#ifdef CONFIG_TNC
#define DISPATCH_TNC(p) {PCI_DEVICE_ID_VGA_TNC, p},
#define DISPATCH_TNC_A0(p) {PCI_DEVICE_ID_VGA_TNC_A0, p},
#define DISPATCH_LNC(p) {PCI_DEVICE_ID_VGA_LNC, p},
#else
#define DISPATCH_TNC(p)
#define DISPATCH_TNC_A0(p)
#define DISPATCH_LNC(p)
#endif
#define DISPATCH_END {0, NULL}

#endif

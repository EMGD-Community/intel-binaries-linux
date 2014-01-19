/*
 *-----------------------------------------------------------------------------
 * Filename: reg_dispatch.h
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
 *  This is the implementation file for the register module's main API's called
 *  by external devices.
 *-----------------------------------------------------------------------------
 */

#ifndef _REG_DISPATCH_H
#define _REG_DISPATCH_H

#include <context.h>
#include <drm_emgd_private.h>

typedef struct _reg_buffer {
	unsigned long flags;
	module_state_h mode_buffer;
	void *platform_buffer;
}reg_buffer_t;

/*
 * This needs to be available for inter-module use so that the
 * mode module can save state during it's init which happens after
 * the reg module init.
 */
typedef struct _reg_dispatch {
	reg_buffer_t *(*reg_alloc)(igd_context_t *context, unsigned long flags,
		void *_platform_context);
	void (*reg_free)(igd_context_t *context, reg_buffer_t *reg_set,
		void *_platform_context);
	int (*reg_save)(igd_context_t *context, reg_buffer_t *reg_set,
		void *_platform_context);
	int (*reg_restore)(igd_context_t *context, reg_buffer_t *reg_set,
		void *_platform_context);
	void (*reg_crtc_lut_get)(igd_context_t *context, emgd_crtc_t *emgd_crtc);
	void (*reg_crtc_lut_set)(igd_context_t *context, emgd_crtc_t *emgd_crtc);
	void *platform_context;
} reg_dispatch_t;

/*
 * Reg dispatch data structure is in reg.h because it and the reg context
 * needs to be available for initial reg save during mode module init.
 */


extern reg_dispatch_t reg_dispatch_plb;
extern reg_dispatch_t reg_dispatch_tnc;

#endif

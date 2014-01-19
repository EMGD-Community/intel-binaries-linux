/*
 *-----------------------------------------------------------------------------
 * Filename: dsp_dispatch.h
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

#ifndef _DSP_DISPATCH_H
#define _DSP_DISPATCH_H

#include <igd.h>
#include <igd_init.h>
#include <igd_mode.h>

#include <mode.h>

/*
 * FIXME: This belong to PI, remove the defaults and let PI set
 * them as it does for i2c.
 */
#define DDC_DEFAULT_SPEED  10  /* Default DDC bus speed in KHz */

typedef struct _dsp_dispatch {
	igd_plane_t **planes;
	igd_display_pipe_t **pipes;
	igd_display_port_t **ports;
	igd_fb_caps_t *caps;
	unsigned long *overlay_pfs;
	unsigned long *render_pfs;
	unsigned long *texture_pfs;
	int (*dsp_init)(igd_context_t *context);
	void (*dsp_control_plane_format)(igd_context_t *context,
			int enable, int display_plane, 
			igd_plane_t *plane_override);
} dsp_dispatch_t;

extern dsp_dispatch_t dsp_dispatch_plb;
extern dsp_dispatch_t dsp_dispatch_tnc;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: igd_debug.h
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
 *  This file contains data structures and variables needed to control
 *  debugging within the IEGD driver.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_DEBUG_H
#define _IGD_DEBUG_H

typedef struct _igd_debug {
	struct {
		/* Per-Module Debug Bits */
		short cmd      ;
		short dsp      ;
		short mode     ;
		short init     ;
		short overlay  ;
		short power    ;
		short _2d      ;
		short blend    ;
		short state    ;
		short gmm      ;
		short gart     ;
		short oal      ;
		short intr     ;
		short dpd      ;
		short video    ;
		short pvr3dd   ;
		short buf_class;
		/* Global Debug Bits */
		short trace    ;
		short instr    ;
		short debug    ;
		/* Verbose debug bits */
		short blend_stats        ;
		short dump_overlay_regs  ;
		short dump_command_queue ;
		short dump_gmm_on_fail   ;
		short dump_shaders       ;
	} hal;
} igd_debug_t;

extern igd_debug_t *emgd_debug;


#endif

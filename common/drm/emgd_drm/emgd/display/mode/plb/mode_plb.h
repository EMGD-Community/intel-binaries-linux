/*
 *-----------------------------------------------------------------------------
 * Filename: mode_plb.h
 * $Revision: 1.2 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2011, Intel Corporation.
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
 */

#define MODULE_NAME hal.mode


#ifdef CONFIG_PLB

typedef struct _mode_data_plb {
	unsigned long plane_a_preserve;
	unsigned long plane_b_c_preserve;
	unsigned long pipe_preserve;
	unsigned long port_preserve;
	unsigned long fw_blc1;
	unsigned long fw_blc2;
	unsigned long fw_blc3;
	unsigned long fw_self;
	unsigned long mem_mode;
	unsigned long dsp_arb;
}mode_data_plb_t;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: mode_tnc.h
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

typedef struct _mode_data_tnc {
	unsigned long plane_a_preserve;
	unsigned long plane_b_c_preserve;
	unsigned long pipe_preserve;
	unsigned long dsp_arb;
	unsigned long fifo_watermark1;
	unsigned long fifo_watermark2;
	unsigned long fifo_watermark3;
	unsigned long fifo_watermark4;
	unsigned long fifo_watermark5;
	unsigned long fifo_watermark6;
} mode_data_tnc_t;

#define CHECK_VGA(a) MODE_IS_VGA(a)

#ifdef DEBUG_BUILD_TYPE
#define FLAG(a) a
#else
/* Turn all workaround for release driver */
#define FLAG(a) 1
#endif


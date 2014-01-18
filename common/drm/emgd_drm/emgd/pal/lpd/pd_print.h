/*
 *-----------------------------------------------------------------------------
 * Filename: pd_print.h
 * $Revision: 1.9 $
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
 *  Header file to get printing macros for port drivers
 *-----------------------------------------------------------------------------
 */

#ifndef _PD_PRINT_H
#define _PD_PRINT_H

#include <config.h>

extern int pd_print(const char *funcname, const int error,
	const int error_exit, const char *format, ...);
extern void pd_trace_enter(const char *funcname);
extern void pd_trace_exit(const char *funcname);

#ifdef DEBUG_BUILD_TYPE
#define PD_DEBUG(arg...) pd_print(__FUNCTION__, 0, 0, arg)
#define PD_ERROR(arg...) pd_print(__FUNCTION__, 1, 0, arg)
#define PD_TRACE_ENTER   pd_trace_enter(__FUNCTION__)
#define PD_TRACE_EXIT    pd_trace_exit(__FUNCTION__)
#define PD_ERROR_EXIT(arg...) pd_print(__FUNCTION__, 1, 1, arg)
#else
#define PD_DEBUG(arg...)
#define PD_ERROR(arg...)
#define PD_TRACE_ENTER
#define PD_TRACE_EXIT
#define PD_ERROR_EXIT(arg...)
#endif

#define PD_OFFSETOF(t,m) ((unsigned long)&(((t *)0)->m))
#if 0
#define PD_OFFSETOF(t,m) offsetof(t,m)
#endif

#define PD_MODULE_INIT(func, params) func params
#define PD_MODULE_EXIT(func, params) func params

#endif


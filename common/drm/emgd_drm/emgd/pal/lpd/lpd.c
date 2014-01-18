/*
 *-----------------------------------------------------------------------------
 * Filename: lpd.c
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
 *  Functions necessary to allow debug printing from the port drivers.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <igd_pd.h>
#include <igd_pi.h>
#include "pd_print.h"
#include <memory.h>


/*
 * This file implements the pd debug printing functions using the OAL.
 * In order to keep the OAL debug printing macro definitions in the OAL
 * the DEBUG_BUILD_TYPE option must be set, even in a production build. The
 * port drivers themselves must not call the pd debug printing functions
 * in a release build. This enables the PD SDK to have debug printing from
 * port drivers, even when using a release driver.
 */
#ifndef DEBUG_BUILD_TYPE
#define DEBUG_BUILD_TYPE
#endif
#define MODULE_NAME hal.dpd

#include <io.h>
#include <igd_debug.h>

unsigned long *dropped_debug_messages;

int pd_set_funcname( const char *name )
{
	os_set_funcname(name);
	return 1;
}

int pd_print(const char *funcname, const int error,
	const int error_exit, const char *format, ...)
{
#ifdef DEBUG_BUILD_TYPE
	va_list ap;
	unsigned int *blah;
	char *priority = error ? KERN_ERR : EMGD_DEBUG_MSG_PRIORITY;
	char *fmt = NULL;

	/* Can't directly use the EMGD_DEBUG_S macro (because "format" is a string
	 * variable), so duplicate some of it here:
	 */
	if (!(emgd_debug && emgd_debug-> MODULE_NAME)) {
		return 0;
	}

	va_start(ap, format);
	blah = (unsigned int *)ap;

	if (error_exit) {
		EMGD_DEBUG("EXIT With Error...");
	}

	/* Create a new format string, with all of the correct parts: */
	fmt = OS_ALLOC(strlen(priority) + strlen(funcname) +
		strlen(format) + 2);
	if (fmt == NULL) {
		printk(format, blah[0], blah[1], blah[2], blah[3], blah[4], blah[5],
			blah[6], blah[7], blah[8], blah[9]);
	} else {
		sprintf(fmt, "%s%s %s", priority, funcname, format);
		printk(fmt, blah[0], blah[1], blah[2], blah[3], blah[4], blah[5],
			blah[6], blah[7], blah[8], blah[9]);
		OS_FREE(fmt);
	}
	printk("\n");
	va_end(ap);

	return 0;
#endif
}

void pd_trace_enter(const char *funcname)
{
	EMGD_DEBUG_S("%s ENTER", funcname);
}

void pd_trace_exit(const char *funcname)
{
	EMGD_DEBUG_S("%s EXIT", funcname);
}

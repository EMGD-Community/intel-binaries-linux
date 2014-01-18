/*
 *-----------------------------------------------------------------------------
 * Filename: mode_access.h
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
 *  Contains functional support to set video mode
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_MODEACCESS_H
#define _IGD_MODEACCESS_H

#include <mode.h>
#include <context.h>

extern int mode_getresolution(igd_display_h display_h,
	unsigned long *width,
	unsigned long *height);

/*
 * The power state for the display is the lower(power) of the display
 * state and the device state. A larger power_state number means a
 * lower power state.
 */
#define GET_DISPLAY_POWER_STATE(d_h, pn) \
(GET_DEVICE_POWER_STATE(d_h) > ((PORT(d_h, pn))->power_state) ? \
GET_DEVICE_POWER_STATE(d_h) : ((PORT(d_h, pn))->power_state))

#endif /* _IGD_MODEACCESS_H */


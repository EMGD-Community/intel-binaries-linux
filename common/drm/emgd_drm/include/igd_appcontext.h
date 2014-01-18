/*
 *-----------------------------------------------------------------------------
 * Filename: igd_appcontext.h
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
 *  This is an IGD exported header file for the client context Module.
 *  This module includes tools for maintaining device state including
 *  hardware context allocation.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_APPCONTEXT_H
#define _IGD_APPCONTEXT_H

typedef void *igd_appcontext_h;

#define IGD_CONTEXT_STATE_2D     0x00000001
#define IGD_CONTEXT_STATE_3D     0x00000002
#define IGD_CONTEXT_STATE_VD     0x00000004

/*----------------------------------------------------------------------
 * Function:
 *  igd_appcontext_h dispatch->appcontext_alloc(igd_display_h display_handle,
 *    int priority,
 *    unsigned int flags);
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */

/*----------------------------------------------------------------------
 * Function:
 *  void dispatch->appcontext_free(igd_display_h display_handle,
 *    int priority,
 *    igd_appcontext_h context_handle);
 *
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *----------------------------------------------------------------------
 */


#endif

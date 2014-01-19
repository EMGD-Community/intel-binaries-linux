/*
 *-----------------------------------------------------------------------------
 * Filename: igd_errno.h
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
 *  This file contains igd error definitions.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_ERRNO_H
#define _IGD_ERRNO_H

/* Error values, must be <64kb for vBIOS */
#define IGD_ERROR_NOMEM           0x0001
#define IGD_ERROR_INVAL           0x0002
#define IGD_ERROR_BUSY            0x0003
#define IGD_ERROR_PWRDOWN         0x0004
#define IGD_ERROR_NODEV           0x0005
#define IGD_ERROR_HWERROR         0x0006
#define IGD_ERROR_EDID            0x0007
#define IGD_ERROR_INVAL_CONTEXT   0x0008
#define IGD_ERROR_INVAL_PARAM     0x0009
#define IGD_ERROR_IOCTL           0x000A
#define IGD_ERROR_GENERIC         0x000B
#define IGD_ERROR_BLT_NOTCOMPLETE 0x000B
#define IGD_ERROR_NOT_IMPLEMENTED 0x000C
#define IGD_ERROR_MAPPING_FAILED  0x000D

#define IGD_NO_MEM           IGD_ERROR_NOMEM
#define IGD_INVAL            IGD_ERROR_INVAL

/* Success value */
#define IGD_SUCCESS               0
#define IGD_SUCCESS_DO_QRY_SETMODE    0x1001

#define IGD_DO_QRY_SETMODE        IGD_SUCCESS_DO_QRY_SETMODE

#endif

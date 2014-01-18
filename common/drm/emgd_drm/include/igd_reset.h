/*
 *-----------------------------------------------------------------------------
 * Filename: igd_reset.h
 * $Revision: 1.6 $
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
 *  This is the external header file for the reset module. It should be included
 *  by above IAL for reset functionality on the ssigd HAL layers.
 *-----------------------------------------------------------------------------
 */


#ifndef _IGD_RESET_H
#define _IGD_RESET_H


/*----------------------------------------------------------------------------
 * Function:
 *   int dispatch->reset_alter(igd_driver_h driver_handle);
 *
 * Description:
 *		.
 * Parameters:
 *		driver_handle - pointer to main driver context structure.
 *
 * Return:
 *  !0: Pointer to a ring buffer structure upon successful allocation
 *  0: A ring buffer of specified type can not be allocated, or an
 *    error occurred during memory allocation for the buffer.
 *----------------------------------------------------------------------------
 */


#endif /*_IGD_RESET_H*/

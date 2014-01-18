/*
 *-----------------------------------------------------------------------------
 * Filename: math_fix.h
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
 *  This file contains OS abstracted interfaces to fixed point math functions
 *-----------------------------------------------------------------------------
 */

#ifndef _OAL_MATH_FIX_H
#define _OAL_MATH_FIX_H


#define OS_POW_FIX(b, p)         os_pow_fix(b, p)


#define OS_MAX(a,b)              (((a) > (b)) ? (a) : (b))
#define OS_MIN(a,b)              (((a) < (b)) ? (a) : (b))

/*****************************************************************************
 * Function: os_pow_fix
 *
 * Parameters:
 *    [IN] base:  the base, should be between 0 and 255.
 *    [IN] power:  this must be in 24i.8f format.
 *
 * Description:
 *    This function uses the Taylor Series to approximate the power function
 *    using fixed-point math.
 *
 ****************************************************************************/

unsigned int os_pow_fix( const int base, const int power );



#endif

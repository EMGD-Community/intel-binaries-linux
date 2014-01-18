/*
 *-----------------------------------------------------------------------------
 * Filename: igd_pwr.h
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
 *
 *-----------------------------------------------------------------------------
 */



#ifndef _IGD_PWR_H
#define _IGD_PWR_H

/*
 * Device power state defines
 */

#define IGD_POWERSTATE_D0			0x0
#define IGD_POWERSTATE_D1			0x1
#define IGD_POWERSTATE_D2			0x2
#define IGD_POWERSTATE_D3			0x3
#define IGD_POWERSTATE_UNDEFINED    0xFFFFFFFF

/*
 * Graphics Adaptor Power State Defines
 */
#define IGD_ADAPTERPOWERSTATE_ON        0x10
#define IGD_ADAPTERPOWERSTATE_STANDBY   0x11
#define IGD_ADAPTERPOWERSTATE_SUSPEND   0x12
#define IGD_ADAPTERPOWERSTATE_OFF       0x13
#define IGD_ADAPTERPOWERSTATE_HIBERNATE 0x14


/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->pwr_alter(igd_driver_h driver_handle,
 *    unsigned int power_state);
 *
 * *----------------------------------------------------------------------
 */

#endif /*_IGD_PWR_H*/

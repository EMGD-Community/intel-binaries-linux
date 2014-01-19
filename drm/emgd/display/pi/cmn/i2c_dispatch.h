/*
 *-----------------------------------------------------------------------------
 * Filename: i2c_dispatch.h
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _I2C_DISPATCH_H
#define _I2C_DISPATCH_H

/*
 * IO.h is needed to resolve the FAR define
 * context.h is needed for the igd_display_context_t
 */

#include <io.h>
#include <context.h>
#include <mode.h>

#include <pd.h>

#define I2C_DEFAULT_SPEED 100  /* Default I2C bus speed in KHz */
#define DDC_DEFAULT_SPEED  10  /* Default DDC bus speed in KHz */

/*
 * Flags for Write Reg List
 *
 * Serial Write: Write a Reg Value, Data Value repeatedly within one
 *  write cycle.
 */
#define IGD_I2C_SERIAL_WRITE 0x1
#define IGD_I2C_WRITE_FW 0x2

typedef struct _i2c_dispatch {
	int (*i2c_read_regs)(
		igd_context_t *context,
		unsigned long i2c_bus,
		unsigned long i2c_speed,
		unsigned long dab,
		unsigned char reg,
		unsigned char FAR *buffer,
		unsigned long num_bytes,
		unsigned long flags);
	int (*i2c_write_reg_list)(
		igd_context_t *context,
		unsigned long i2c_bus,
		unsigned long i2c_speed,
		unsigned long dab,
		pd_reg_t *reg_list,
		unsigned long flags);
} i2c_dispatch_t;

#endif


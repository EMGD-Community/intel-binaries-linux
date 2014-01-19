/*-----------------------------------------------------------------------------
* Copyright (c) Chrontel Inc.
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
* @file  ch7036_iic.h
* @version 1.2.2
*-----------------------------------------------------------------------------
*/

#ifndef _CH7036_IIC_H
#define _CH7036_IIC_H

#include "ch7036_typedef.h"
#include "ch7036_reg_table.h"




uint8 I2CRead(DEV_CONTEXT* pDevContext, uint8 index);
void I2CWrite(DEV_CONTEXT* pDevContext, uint8 index, uint8 value);

void I2CBlockWrite(DEV_CONTEXT* pDevContext,uint8 index, uint8* value, uint16 len);
void DeviceReset(DEV_CONTEXT* pDevContext);
void DeviceSetup(DEV_CONTEXT* pDevContext);

uint8 iic_read(uint8 index);
void iic_write(uint8 index, uint8 value);
void iic_reset(void);
uint32 iic_read_ex(MULTI_REG_ID reg_id);
void iic_write_ex(MULTI_REG_ID reg_id, uint32 reg_val);

extern int g_verbose;

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: sdvo_port.h
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
 *  Main include file for SDVO port driver
 *-----------------------------------------------------------------------------
 */

#ifndef _SDVO_PORT_H_
#define _SDVO_PORT_H_

#include <config.h>
#include <igd_pd.h>
#include <pd.h>
#include <pd_print.h>

/* #define DEBUG 0 */

/*  ............................................................................ */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(p) (sizeof(p)/sizeof((p)[0]))
#endif

#include "sdvo_intf.h"
#include "sdvo_attr.h"
#include "sdvo_hdmi.h"

/*  ............................................................................ */
#ifndef CONFIG_MICRO
#define	SDVO_GET_ATTR_NAME(p_attr)	p_attr->name
#else
#define SDVO_GET_ATTR_NAME(p_attr)	""
#endif

/*  ............................................................................ */
unsigned char sdvo_write_i2c_reg(sdvo_device_context_t *p_Ctx, i2c_reg_t offset,
	i2c_reg_t value);
unsigned char sdvo_read_i2c_reg(sdvo_device_context_t *p_Ctx, unsigned char offset,
	i2c_reg_t *p_Value);


int sdvo_is_multi_display_device(sdvo_device_context_t *p_ctx);

/*LAPIS Workaround*/
#define	VENDOR_ID_LAPIS 0x81


#endif  /*  _SDVO_PORT_H_ */

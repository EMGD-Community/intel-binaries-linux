/*
 *-----------------------------------------------------------------------------
 * Filename: ovl2_regs_tnc.h
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
 *  This is the internal header file for napa overlay engine. It should be not be
 *  by any other module besides the overlay module itself. It contains the
 *  neccessary hardware virtualized structures and functions internal to
 *  the napa core's overlay engine
 *-----------------------------------------------------------------------------
 */
#ifndef _OVL2_REGS_TNC_H
#define _OVL2_REGS_TNC_H
/* Second Overlay Structure.
 * These registers are the trigger registers and should be written
 * last. */
typedef struct _ovl2_reg_tnc{
	unsigned int control;
	unsigned int start;
} ovl2_reg_tnc_t;

/* Color Correction */
#define OVL2_RGB_COLOR_DEF_CONT_BRGHT    0x1000000
#define OVL2_RGB_COLOR_DEF_SATN_HUE      0x0000080

/* Overlay Command Definitions */
/* 422 Swap */
#define OVL2_CMD_YVYU				0x00020000
#define OVL2_CMD_UYVY				0x00010000
#define OVL2_CMD_VYUY				0x00030000

/* Source Format */
#define OVL2_CMD_ARGB_8888			0x1C000000
#define OVL2_CMD_RGB_8888			0x18000000
#define OVL2_CMD_RGB_565			0x14000000
#define OVL2_CMD_RGB_555			0x10000000
#define OVL2_CMD_RGB_8				0x08000000
#define OVL2_CMD_YUV_422			0x00000000 /*packed YUV422*/
#define OVL2_CMD_SRC_FMT_MASK		0x3C000000 /*mask for above*/

#define OVL2_REG_ADDR_GAMMA5						0x721E0
#define OVL2_REG_ADDR_GAMMA4						0x721E4
#define OVL2_REG_ADDR_GAMMA3						0x721E8
#define OVL2_REG_ADDR_GAMMA2						0x721EC
#define OVL2_REG_ADDR_GAMMA1						0x721F0
#define OVL2_REG_ADDR_GAMMA0						0x721F4
#define OVL2_TOTAL_GAMMA_REG						6

/* following value are needed because hardware seems to display yuv slightly dimmer
   than RGB when color data is calculated out to be equal */
#define MID_CONTRAST_YUV					0x43//4a
#define MID_SATURATION_YUV					0x91//92
#define MID_BRIGHTNESS_YUV					-5
#endif /* _OVL2_REGS_NAP_H */

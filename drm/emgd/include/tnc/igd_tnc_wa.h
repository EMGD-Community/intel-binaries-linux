/*
 *-----------------------------------------------------------------------------
 * Filename: igd_tnc_wa.h
 * $Revision: 1.11 $
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
 *  This header file contains the Device Dependent information used with
 *  the IEGD HAL for the Atom E6xx family of supported chips.
 *  Note: This file should be included into a C file as <tnc/igd_tnc_wa.h> and
 *  not <igd_tnc_wa.h> this will insure that multiple device dependent
 *  igd_cmd.h files can be included into a single C source.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_TNC_WA_H
#define _IGD_TNC_WA_H

#define LNC_CLOCK 199500
#define LNC_B1_OPTIMUM_MARGIN 2000
#define LIMIT_TOTAL_CHECK_DISPLAY 10
#define T0 0
#define T1 1
#define WA_TUNE \
	((tnc_wa_timing_t *)&tune)
/* Graphic core Revision ID for Atom E6xx stepping */
#define TNC_A0_RID			0x2
#define TNC_B0_RID			0x3
#define TNC_B0_DEV3_RID		0x1
#define TNC_B1_DEV3_RID		0x2
#define SDVO_CRC_CTRL_REG	0x61050
#define SDVO_BUFF_CTRL_REG	0x61170
#define TNC_HTOTAL_TUNED	0x8000

/* flag to enable clipping hardware fix in TNC B1 */
#define IGD_CLIP_FIX_DISABLE            0x0000
 /* Completely disable throttling */
#define IGD_CLIP_FIX_DISABLE_THROTTLE   0x0001          
/* Replaces the original stall_pixel from FSM with external stall from cDVO */
#define IGD_CLIP_FIX_REPLACE_STALL      0x0002    
#define IGD_CLIP_FIX_GLOBAL_ENABLE      0x8000
#define CLIP_HW_FIX_DEFAULT             0x8001

/*
 * These are the register default settings for TC B1
 * They are used for the undocumented register override feature
 * when en_reg_override=1
 */
#define DISP_ARB                0x00003232
#define FIFO_WATERMARK_1        0x3f8f0404
#define FIFO_WATERMARK_2        0x04040f04
#define FIFO_WATERMARK_3        0x00000000
#define FIFO_WATERMARK_4        0x04040404
#define FIFO_WATERMARK_5        0x04040404
#define FIFO_WATERMARK_6        0x00000078
#define GVD_HP_CONTROL          0x102003f2
#define DISP_CHICKENBITS        0x0000c040
#define BCHICKENBITS            0xffe32004
#define BWFLUSH                 0x00000408

typedef struct _tnc_wa_timing_t{
	short htotal;
	short delta;
	short flag;
	short counter;
	unsigned int crc_red;
	unsigned int crc_green;
	unsigned int crc_blue;
}tnc_wa_timing_t;

typedef union
{
       unsigned int pixel;
       struct
	   {
			   unsigned int bit0             :1;
               unsigned int bit1             :1;
               unsigned int bit2             :1;
               unsigned int bit3             :1;
               unsigned int bit4             :1;
               unsigned int bit5             :1;
               unsigned int bit6             :1;
               unsigned int bit7             :1;
               unsigned int bit8             :1;
               unsigned int bit9             :1;
               unsigned int bit10             :1;
               unsigned int bit11             :1;
               unsigned int bit12             :1;
               unsigned int bit13             :1;
               unsigned int bit14             :1;
               unsigned int bit15             :1;
               unsigned int bit16             :1;
               unsigned int bit17             :1;
               unsigned int bit18             :1;
               unsigned int bit19             :1;
               unsigned int bit20             :1;
               unsigned int bit21             :1;
               unsigned int bit22             :1;
               unsigned int bitExtra          :1;
               unsigned int bitRest           :8;
       } bit;
} pixel_crc_t;


/*
* CDVO regs structure. This is mainly used to move the CDVO reset sequence
* to the data segment for VBIOS.
*/
typedef struct _cdvo_regs_t{
	unsigned long reg;
	unsigned long value;
}cdvo_regs_t;
#endif

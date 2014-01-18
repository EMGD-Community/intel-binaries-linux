/*
 *-----------------------------------------------------------------------------
 * Filename: igd_vga.h
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
 *  Header file for common VGA registers and macros.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_VGA_H
#define _IGD_VGA_H

#ifdef CONFIG_MICRO
#define VB_CODE_SEG __based( __segname("_CODE"))
#else
#define VB_CODE_SEG
#endif

/* The Maximum VGA Mode Number as specified in the IGD Mode Table. */
#define VGA_MODE_NUM_MAX 0x1C

typedef struct vga_mode_data {
	unsigned char  cols;
	unsigned char  rows;                 /* #rows-1 */
	unsigned char  char_height;
	unsigned short video_buff_size;
	unsigned char  sr_regs[4];
	unsigned char  misc_reg;
	unsigned char  crtc_regs[25];
	unsigned char  ar_regs[20];
	unsigned char  gr_regs[9];
} vga_mode_data_t;

/* This contains the VGA Mode Table.  A ptr to this exists in the
 * Environment. */
extern vga_mode_data_t VB_CODE_SEG vga_mode_data[];

/* This contains a pointer to the VGA Mode_Table.  This is necessary, so
 * vga_mode.c can use this to program the mode, incase the user has
 * changed the Environment Table. */
extern vga_mode_data_t FAR (*vga_mode_data_ptr)[];

#endif

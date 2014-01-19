/*
 *-----------------------------------------------------------------------------
 * Filename: instr.h
 * $Revision: 1.8 $
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
/*
 * TODO:
 * Add or modify the struct and other definitions for Poulsbo
 */
#ifndef _INSTR_PLB_H
#define _INSTR_PLB_H

#include <instr_common.h>

typedef struct _igd_vertex {
	unsigned long x;
	unsigned long x_ieee_fp;
	unsigned long y;
	unsigned long y_ieee_fp;
	unsigned long z;
	unsigned long z_ieee_fp;
	unsigned long argb;
} igd_vertex_t, *pigd_vertex_t;

/* Common FP numbers */
#define IEEE_FP_0 0x00000000
#define IEEE_FP_1 0x3f800000

/* For EMGD_WRITE32 */
#include <io.h>


/* All instruction sizes must be Double DWORD aligned for the current
 * platform.  The exception to this is the NOOP, which may be useful
 * not being Double DWORD aligned. */

/* color depth */
#define PLB_PF_1BIT_PALETTE      0x0
#define PLB_PF_2BIT_PALETTE      0x1
#define PLB_PF_4BIT_PALETTE      0x2
#define PLB_PF_8BIT_PALETTE      0x3
#define PLB_PF_8BIT_ALPHA        0x4
#define PLB_PF_4BIT_ALPHA        0x5
#define PLB_PF_8BPP              0x6
#define PLB_PF_16BPP4444         0x7
#define PLB_PF_16BPP555          0x8
#define PLB_PF_16BPP1555         0x9
#define PLB_PF_16BPP565          0xA
#define PLB_PF_24BPP             0xB
#define PLB_PF_32BPP             0xC
#define PLB_PF_32BPP_UYVY        0xD
#define PLB_PF_32BPP_AYUV        0xE

/*-----------------*/
/* 2D Instructions */
/*-----------------*/
/* Size of 2D Instructions */
#define PLB_2D_PAT_CONTROL_SIZE    1
#define PLB_2D_CONTROL_SIZE        3
#define PLB_2D_SRC_SURF_SIZE       2
#define PLB_2D_DEST_SURF_SIZE      2
#define PLB_2D_PAT_SURF_SIZE       2
#define PLB_2D_MASK_SURF_SIZE      2
#define PLB_2D_SRC_OFFSET_SIZE     1
#define PLB_2D_MASK_OFFSET_SIZE    1
#define PLB_2D_SRC_PAL_SIZE        1
#define PLB_2D_PAT_PAL_SIZE        1
/* The 2D_BLT_SIZE is different when doing a Fill */
#define PLB_2D_BLT_SIZE            3
#define PLB_2D_BLT_FILL_SIZE       4
#define PLB_2D_CLIP_SIZE           2
#define PLB_2D_FENCE_SIZE          1
#define PLB_2D_FLUSH_SIZE          1
#define PLB_2D_BLT_SRC_COPY_SIZE   3

/* 2D instruction (BR0) */
#define _PLB_2D_CLIP        (0x0 << 28)
#define _PLB_2D_PAT_CONTROL (0x1 << 28)
#define _PLB_2D_CONTROL     (0x2 << 28)
#define _PLB_2D_SRC_OFFSET  (0x3 << 28)
#define _PLB_2D_MASK_OFFSET (0x4 << 28)
#define _PLB_2D_FENCE       (0x7 << 28)
#define _PLB_2D_BLT         (0x8 << 28)
#define _PLB_2D_SRC_SURF    (0x9 << 28)
#define _PLB_2D_DEST_SURF   (0xa << 28)
#define _PLB_2D_PAT_SURF    (0xb << 28)
#define _PLB_2D_SRC_PAL     (0xc << 28)
#define _PLB_2D_PAT_PAL     (0xd << 28)
#define _PLB_2D_MASK_SURF   (0xe << 28)
#define _PLB_2D_FLUSH       (0xf << 28)

/* In 2D Control */
#define PSB_2D_SRCCK_CTRL     1
#define PSB_2D_DSTCK_CTRL     2
#define PSB_2D_ALPHA_CTRL     4

#define PLB_2D_BLT_CTRL_ROT_0      0
#define PLB_2D_BLT_CTRL_ROT_90     (1<<25)
#define PLB_2D_BLT_CTRL_ROT_180    (2<<25)
#define PLB_2D_BLT_CTRL_ROT_270    (3<<25)

/* Top Left or Bottom Right */
#define PLB_2D_BLT_CTRL_ORDER_TL2BR   0
#define PLB_2D_BLT_CTRL_ORDER_BR2TL   (1<<23)
#define PLB_2D_BLT_CTRL_ORDER_TR2BL   (2<<23)
#define PLB_2D_BLT_CTRL_ORDER_BL2TR   (3<<23)

#define PLB_2D_BLT_CTRL_DEST_CK_DISABLE     0
#define PLB_2D_BLT_CTRL_DEST_CK_PASS_MATCH  (1<<21)
#define PLB_2D_BLT_CTRL_DEST_CK_KILL_MATCH  (2<<21)

#define PLB_2D_BLT_CTRL_SRC_CK_DISABLE     0
#define PLB_2D_BLT_CTRL_SRC_CK_PASS_MATCH  (1<<19)
#define PLB_2D_BLT_CTRL_SRC_CK_KILL_MATCH  (2<<19)

#define PLB_2D_BLT_CTRL_CLIP_DISABLE  0
#define PLB_2D_BLT_CTRL_CLIP_ENABLE   (1<<18)

#define PLB_2D_BLT_CTRL_ALPHA_BLEND_DISABLE  0
#define PLB_2D_BLT_CTRL_ALPHA_BLEND_ENABLE   (1<<17)

#define PLB_2D_BLT_CTRL_FILL      0
#define PLB_2D_BLT_CTRL_PATTERN   (1<<16)

/*----------------*/
/* 2D Abstraction */
/*----------------*/
#define PLB_2D_CLIP(queue, xmin, xmax, ymin, ymax)               \
    CMD_2D_WRITE_PLB(queue, _PLB_2D_CLIP | (xmax << 12) | xmin); \
    CMD_2D_WRITE_PLB(queue, (ymax << 12) | ymin);

#define PLB_2D_PAT_CONTROL(queue, pt, width, height)                       \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_PAT_CONTROL | (pt.x << 15) |           \
                     (pt.y << 10) | (width << 5) | height);

#define PLB_2D_CONTROL(queue, ctrl_flag, ck_color, ck_mask)                \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_CONTROL | ctrl_flag);                  \
	CMD_2D_WRITE_PLB(queue, ck_color);                                     \
	CMD_2D_WRITE_PLB(queue, ck_mask);

#define PLB_2D_FENCE(queue)                                                \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_FENCE);

#define PLB_2D_FLUSH(queue)                                                \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_FLUSH);

#define PLB_2D_DEST_SURF(queue, pf, pitch, offset)                         \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_DEST_SURF | (pf<<15) | pitch);         \
	CMD_2D_WRITE_PLB(queue, offset);

#define PLB_2D_SRC_SURF(queue, pf, pitch, offset)                          \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_SRC_SURF | (pf<<15) | pitch);          \
	CMD_2D_WRITE_PLB(queue, offset);

#define PLB_2D_PAT_SURF(queue, pf, pitch, offset)                          \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_PAT_SURF | (pf<<15) | pitch);          \
	CMD_2D_WRITE_PLB(queue, offset);

#define PLB_2D_MASK_SURF(queue, pitch, offset)                             \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_MASK_SURF | pitch);                    \
	CMD_2D_WRITE_PLB(queue, offset);

#define PLB_2D_SRC_OFFSET(queue, pt)                                       \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_SRC_OFFSET | (pt.x<<12) | pt.y);

#define PLB_2D_MASK_OFFSET(queue, pt)                                      \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_MASK_OFFSET | (pt.x<<12) | pt.y);

#define PLB_2D_SRC_PAL(queue, offset)                                      \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_SRC_PAL | offset);

#define PLB_2D_PAT_PAL(queue, offset)                                      \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_PAT_PAL | offset);

/* Color fill from Top Left to Bottom Right */
#define PLB_2D_BLT_FILL_TL2BR(queue, control, rop, fill, rect)             \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_BLT | control | (rop<<8) | rop);       \
	CMD_2D_WRITE_PLB(queue, fill);                                         \
	CMD_2D_WRITE_PLB(queue, (rect->x1<<12) | rect->y1);                    \
	CMD_2D_WRITE_PLB(queue, ((rect->x2-rect->x1)<<12) |                    \
                     (rect->y2-rect->y1));

/* Color fill from Bottom Right to Top Left */
#define PLB_2D_BLT_FILL_BR2TL(queue, control, rop, fill, rect)             \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_BLT | control | (rop<<8) | rop);       \
	CMD_2D_WRITE_PLB(queue, fill);                                         \
	CMD_2D_WRITE_PLB(queue, ((rect->x2-1)<<12) | (rect->y2-1));            \
	CMD_2D_WRITE_PLB(queue, ((rect->x2-rect->x1)<<12) |                    \
                     (rect->y2-rect->y1));

#define PLB_2D_BLT_SRC_COPY(queue, control, rop, pt, w, h)                 \
	CMD_2D_WRITE_PLB(queue, _PLB_2D_BLT | control | (rop<<8) | rop);       \
	CMD_2D_WRITE_PLB(queue, (pt.x<<12) | pt.y);                            \
	CMD_2D_WRITE_PLB(queue, (w<<12) | h);

#define PLB_2D_BLT_CHROMA(queue, chroma_color)                             \
	CMD_2D_WRITE_PLB(queue, chroma_color);

#endif

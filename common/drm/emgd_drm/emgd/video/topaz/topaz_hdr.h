/*
 *-----------------------------------------------------------------------------
 * Filename: topaz_hdr.h
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TOPAZ_HDR_H
#define TOPAZ_HDR_H

#define MTX_CCBCTRL_ROFF		0
#define MTX_CCBCTRL_COMPLETE		4
#define MTX_CCBCTRL_CCBSIZE		8
#define MTX_CCBCTRL_QP			12
#define MTX_CCBCTRL_FRAMESKIP		20
#define MTX_CCBCTRL_INITQP		24

#define TOPAZ_CTX(ctx) \
	((platform_context_tnc_t *)(ctx)->platform_context)->tpz_private_data


#define TOPAZ_BEGIN_CCB(ctx) \
	write_mtx_mem_multiple_setup((ctx), \
	TOPAZ_CTX((ctx)).topaz_ccb_buffer_addr + \
	TOPAZ_CTX((ctx)).topaz_cmd_windex * 4)

#define TOPAZ_OUT_CCB(ctx, cmd) \
	write_mtx_mem_multiple((ctx), (cmd)); \
	TOPAZ_CTX((ctx)).topaz_cmd_windex++;

#define TOPAZ_END_CCB(ctx, kick_cnt) \
	EMGD_WRITE32(1,  (ctx)->device_context.virt_mmadr + TNC_TOPAZ_MTX_KICK);

/* macros to get/set CCB control data */
#define WB_CCB_CTRL_RINDEX(ctx) \
*((unsigned long *)TOPAZ_CTX(ctx).topaz_ccb_wb)

#define WB_CCB_CTRL_SEQ(ctx) \
*((unsigned long *)(TOPAZ_CTX(ctx).topaz_ccb_wb + 1))

/* HACK: temporary turn off MMU bypass to POLL, then turn on
#define POLL_WB_RINDEX(ctx, value)				\
do {								\
	int i;							\
	unsigned char *mmio = ctx->device_context.virt_mmadr;   \
	EMGD_WRITE32(0x00000000, mmio + TNC_TOPAZ_MMU_CONTROL0);\
	for (i = 0; i < 10000; i++) {				\
		if (CCB_CTRL_RINDEX(ctx) == value)	\
			break;					\
		else						\
			OS_SLEEP(100);			\
	}							\
	if (CCB_CTRL_RINDEX(ctx) != value) {		\
		EMGD_ERROR("TOPAZ: poll rindex timeout\n");	\
		ret = -IGD_ERROR_HWERROR;					\
	}							\
	EMGD_WRITE32(0x00000800, mmio + TNC_TOPAZ_MMU_CONTROL0);\
} while (0)*/

#define POLL_WB_RINDEX(ctx, value)				\
do {								\
	int i;							\
	for (i = 0; i < 10000; i++) {				\
		if (CCB_CTRL_RINDEX(ctx) == value)	\
			break;					\
		else						\
			OS_SLEEP(100);			\
	}							\
	if (CCB_CTRL_RINDEX(ctx) != value) {		\
		EMGD_ERROR("TOPAZ: poll rindex timeout\n");	\
		ret = -IGD_ERROR_HWERROR;					\
	}							\
} while (0)

#define CCB_CTRL_RINDEX(ctx)			\
	read_mtx_mem(ctx, 			\
	TOPAZ_CTX(ctx).topaz_ccb_ctrl_addr + MTX_CCBCTRL_ROFF)

#define CCB_CTRL_SET_QP(ctx, qp)				\
	write_mtx_mem(ctx,					\
		TOPAZ_CTX(ctx).topaz_ccb_ctrl_addr + MTX_CCBCTRL_QP, qp)

#define CCB_CTRL_FRAMESKIP(ctx)				   \
	read_mtx_mem(ctx,				   \
	    TOPAZ_CTX(ctx).topaz_ccb_ctrl_addr + MTX_CCBCTRL_FRAMESKIP)

#define CCB_CTRL_SEQ(ctx)				\
	read_mtx_mem(ctx,				\
	    TOPAZ_CTX(ctx).topaz_ccb_ctrl_addr + MTX_CCBCTRL_COMPLETE)


void write_mtx_mem_multiple_setup(igd_context_t *context,
		unsigned long addr);

void write_mtx_mem_multiple(igd_context_t *context,
		unsigned long cmd);

unsigned long read_mtx_mem(igd_context_t *, unsigned long );

void write_mtx_mem(igd_context_t *,
		unsigned long ,
		unsigned long );

#endif /* TOPAZ_HDR_H */

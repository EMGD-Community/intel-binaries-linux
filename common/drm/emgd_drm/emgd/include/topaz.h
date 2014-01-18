/*
 *-----------------------------------------------------------------------------
 * Filename: topaz.h
 * $Revision: 1.16 $
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
 *  These are the defines specific to the Topaz engine code.
 *-----------------------------------------------------------------------------
 */
extern unsigned long _topaz_base;
#define TOPAZ_BASE _topaz_base

#ifndef _TOPAZ_H
#define _TOPAZ_H

/* MTX registers */
#define TNC_TOPAZ_MTX_ENABLE				(TOPAZ_BASE + 0x0000)
#define TNC_TOPAZ_MTX_STATUS				(TOPAZ_BASE + 0x0008)
#define TNC_TOPAZ_MTX_KICK				(TOPAZ_BASE + 0x0080)
#define TNC_TOPAZ_MTX_KICKI				(TOPAZ_BASE + 0x0088)
#define TNC_TOPAZ_MTX_FAULT0				(TOPAZ_BASE + 0x0090)
#define TNC_TOPAZ_MTX_REGISTER_READ_WRITE_DATA		(TOPAZ_BASE + 0x00f8)
#define TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST	(TOPAZ_BASE + 0x00fc)
#define TNC_TOPAZ_MTX_RAM_ACCESS_DATA_EXCHANGE		(TOPAZ_BASE + 0x0100)
#define TNC_TOPAZ_MTX_RAM_ACCESS_DATA_TRANSFER		(TOPAZ_BASE + 0x0104)
#define TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL		(TOPAZ_BASE + 0x0108)
#define TNC_TOPAZ_MTX_RAM_ACCESS_STATUS			(TOPAZ_BASE + 0x010c)
#define TNC_TOPAZ_MTX_SOFT_RESET			(TOPAZ_BASE + 0x0200)
#define TNC_TOPAZ_MTX_SYSC_CDMAC			(TOPAZ_BASE + 0x0340)
#define TNC_TOPAZ_MTX_SYSC_CDMAA			(TOPAZ_BASE + 0x0344)
#define TNC_TOPAZ_MTX_SYSC_CDMAS0			(TOPAZ_BASE + 0x0348)
#define TNC_TOPAZ_MTX_SYSC_CDMAS1			(TOPAZ_BASE + 0x034c)
#define TNC_TOPAZ_MTX_SYSC_CDMAT			(TOPAZ_BASE + 0x0350)
/* Topaz Registers */
#define TNC_START_OFFSET			(0x02000)
#define TNC_IMG_TOPAZ_REG_BASE			(TOPAZ_BASE + TNC_START_OFFSET)
#define TNC_TOPAZ_IMG_TOPAZ_SRST		(TNC_IMG_TOPAZ_REG_BASE + 0x0000)
#define TNC_TOPAZ_IMG_TOPAZ_INTSTAT		(TNC_IMG_TOPAZ_REG_BASE + 0x0004)
#define TNC_TOPAZ_IMG_TOPAZ_INTENAB		(TNC_IMG_TOPAZ_REG_BASE + 0x0008)
#define TNC_TOPAZ_IMG_TOPAZ_INTCLEAR		(TNC_IMG_TOPAZ_REG_BASE + 0x000c)
#define TNC_TOPAZ_IMG_TOPAZ_MAN_CLK_GATE	(TNC_IMG_TOPAZ_REG_BASE + 0x0010)
#define TNC_TOPAZ_IMG_TOPAZ_AUTO_CLK_GATE	(TNC_IMG_TOPAZ_REG_BASE + 0x0014)
#define TNC_TOPAZ_IMG_TOPAZ_RSVD0		(TNC_IMG_TOPAZ_REG_BASE + 0x03b0)
#define TNC_TOPAZ_TOPAZ_MTX_C_RATIO		(TNC_IMG_TOPAZ_REG_BASE + 0x0018)
#define TNC_TOPAZ_IMG_TOPAZ_CORE_ID		(TNC_IMG_TOPAZ_REG_BASE + 0x03c0)
#define TNC_TOPAZ_IMG_TOPAZ_CORE_REV		(TNC_IMG_TOPAZ_REG_BASE + 0x03d0)
#define TNC_TOPAZ_IMG_TOPAZ_CORE_DES1		(TNC_IMG_TOPAZ_REG_BASE + 0x03e0)
#define TNC_TOPAZ_IMG_TOPAZ_CORE_DES2		(TNC_IMG_TOPAZ_REG_BASE + 0x03f0)
#define TNC_TOPAZ_MMU_STATUS			(TNC_IMG_TOPAZ_REG_BASE + 0x001c)
#define TNC_TOPAZ_MMU_MEM_REQ			(TNC_IMG_TOPAZ_REG_BASE + 0x0020)
#define TNC_TOPAZ_MMU_CONTROL0			(TNC_IMG_TOPAZ_REG_BASE + 0x0024)
#define TNC_TOPAZ_MMU_CONTROL1			(TNC_IMG_TOPAZ_REG_BASE + 0x0028)
#define TNC_TOPAZ_MMU_DIR_LIST_BASE0		(TNC_IMG_TOPAZ_REG_BASE + 0x0030)
#define TNC_TOPAZ_MMU_TILE0			(TNC_IMG_TOPAZ_REG_BASE + 0x0034)
#define TNC_TOPAZ_MMU_BANK_INDEX		(TNC_IMG_TOPAZ_REG_BASE + 0x0038)
#define TNC_TOPAZ_MTX_DEBUG			(TNC_IMG_TOPAZ_REG_BASE + 0x003c)
#define TNC_TOPAZ_IMG_TOPAZ_DMAC_MODE		(TNC_IMG_TOPAZ_REG_BASE + 0x0040)
#define TNC_TOPAZ_RTM				(TNC_IMG_TOPAZ_REG_BASE + 0x0044)
#define TNC_TOPAZ_RTM_VALUE			(TNC_IMG_TOPAZ_REG_BASE + 0x0048)
/* MVEA Registers */
#define TNC_MVEA_START_OFFSET			(0x03000)
#define TNC_IMG_MVEA_REG_BASE			(TOPAZ_BASE + TNC_MVEA_START_OFFSET)
#define TNC_TOPAZ_IMG_MVEA_SRST			(TNC_IMG_MVEA_REG_BASE + 0x0000)
#define TNC_TOPAZ_IMG_MVEA_INTSTAT		(TNC_IMG_MVEA_REG_BASE + 0x0004)
#define TNC_TOPAZ_IMG_MVEA_INTENAB		(TNC_IMG_MVEA_REG_BASE + 0x0008)
#define TNC_TOPAZ_IMG_MVEA_INTCLEAR		(TNC_IMG_MVEA_REG_BASE + 0x000c)
#define TNC_TOPAZ_IMG_MVEA_INT_COMB_SEL		(TNC_IMG_MVEA_REG_BASE + 0x0010)
#define TNC_TOPAZ_IMG_MVEA_RSVD0		(TNC_IMG_MVEA_REG_BASE + 0x03b0)
#define TNC_TOPAZ_IMG_MVEA_CORE_ID		(TNC_IMG_MVEA_REG_BASE + 0x03c0)
#define TNC_TOPAZ_IMG_MVEA_CORE_REV		(TNC_IMG_MVEA_REG_BASE + 0x03d0)
#define TNC_TOPAZ_MVEA_START			(TNC_IMG_MVEA_REG_BASE + 0x0014)
#define TNC_TOPAZ_MVEA_BUSY			(TNC_IMG_MVEA_REG_BASE + 0x0018)
#define TNC_TOPAZ_MVEA_DMACMDFIFO_WAIT		(TNC_IMG_MVEA_REG_BASE + 0x001c)
#define TNC_TOPAZ_MVEA_DMACMDFIFO_STATUS	(TNC_IMG_MVEA_REG_BASE + 0x0020)
#define TNC_TOPAZ_MVEA_AUTO_CLOCK_GATING	(TNC_IMG_MVEA_REG_BASE + 0x0024)
#define TNC_TOPAZ_MVEA_MAN_CLOCK_GATING		(TNC_IMG_MVEA_REG_BASE + 0x0028)
#define TNC_TOPAZ_MB_PERFORMANCE_RESULT		(TNC_IMG_MVEA_REG_BASE + 0x002c)
#define TNC_TOPAZ_MB_PERFORMANCE_MB_NUMBER	(TNC_IMG_MVEA_REG_BASE + 0x0030)
#define TNC_TOPAZ_HW_MB_PERFORMANCE_RESULT	(TNC_IMG_MVEA_REG_BASE + 0x0034)
#define TNC_TOPAZ_HW_MB_PERFORMANCE_MB_NUMBER	(TNC_IMG_MVEA_REG_BASE + 0x0038)
/* VLC Registers */
#define TNC_VLC_START_OFFSET			(0x05000)
#define TNC_IMG_VLC_REG_BASE			(TOPAZ_BASE + TNC_VLC_START_OFFSET)
#define TNC_TOPAZ_VLC_CONTROL			(TNC_IMG_VLC_REG_BASE + 0x0000)
#define TNC_TOPAZ_VLC_STATUS			(TNC_IMG_VLC_REG_BASE + 0x0004)
#define TNC_TOPAZ_VLC_INFO_0			(TNC_IMG_VLC_REG_BASE + 0x0008)
#define TNC_TOPAZ_VLC_INFO_1			(TNC_IMG_VLC_REG_BASE + 0x000c)
#define TNC_TOPAZ_VLC_INFO_2			(TNC_IMG_VLC_REG_BASE + 0x0010)
#define TNC_TOPAZ_VLC_STUFF_HEAD_CTRL		(TNC_IMG_VLC_REG_BASE + 0x0014)
#define TNC_TOPAZ_VLC_HEADER_FIFO		(TNC_IMG_VLC_REG_BASE + 0x0018)
#define TNC_TOPAZ_VLC_HEADER_CTRL		(TNC_IMG_VLC_REG_BASE + 0x001c)
#define TNC_TOPAZ_VLC_HEADER_STATUS		(TNC_IMG_VLC_REG_BASE + 0x0020)
#define TNC_TOPAZ_VLC_RATE_CTRL_0		(TNC_IMG_VLC_REG_BASE + 0x0024)
#define TNC_TOPAZ_VLC_RATE_CTRL_1		(TNC_IMG_VLC_REG_BASE + 0x002c)
#define TNC_TOPAZ_VLC_BUFFER_SIZE		(TNC_IMG_VLC_REG_BASE + 0x0034)
#define TNC_TOPAZ_VLC_SIGNATURE_0		(TNC_IMG_VLC_REG_BASE + 0x0038)
#define TNC_TOPAZ_VLC_SIGNATURE_1		(TNC_IMG_VLC_REG_BASE + 0x003c)
#define TNC_TOPAZ_VLC_SIGNATURE_2		(TNC_IMG_VLC_REG_BASE + 0x0040)
#define TNC_TOPAZ_VLC_SIGNATURE_3		(TNC_IMG_VLC_REG_BASE + 0x0044)
#define TNC_TOPAZ_VLC_SIGNATURE_4		(TNC_IMG_VLC_REG_BASE + 0x0048)
#define TNC_TOPAZ_VLC_JPEG_CFG			(TNC_IMG_VLC_REG_BASE + 0x004c)
#define TNC_TOPAZ_VLC_PERFORMANCE_0		(TNC_IMG_VLC_REG_BASE + 0x0050)
#define TNC_TOPAZ_VLC_PERFORMANCE_1		(TNC_IMG_VLC_REG_BASE + 0x0054)
#define TNC_TOPAZ_VLC_PERFORMANCE_2		(TNC_IMG_VLC_REG_BASE + 0x0058)
#define TNC_TOPAZ_VLC_IPCM_0			(TNC_IMG_VLC_REG_BASE + 0x005c)
#define TNC_TOPAZ_VLC_IPCM_1			(TNC_IMG_VLC_REG_BASE + 0x0060)
#define TNC_TOPAZ_VLC_MPEG4_CFG			(TNC_IMG_VLC_REG_BASE + 0x0064)
#define TNC_TOPAZ_VLC_MB_PARAMS			(TNC_IMG_VLC_REG_BASE + 0x0068)
#define TNC_TOPAZ_VLC_RESET			(TNC_IMG_VLC_REG_BASE + 0x006c)

#define WRITEBACK_MEM_SIZE			(4 * 1024)

#define TOPAZ_MTX_PC				(0x00000005)
#define PC_START_ADDRESS			(0x80900000)
#define MTX_CODE_BASE				(0x80900000)
#define MTX_DATA_BASE				(0x82880000)
#define MTX_CORE_CODE_MEM			(0x10)
#define MTX_CORE_DATA_MEM			(0x18)

#define MTX_CCB_CTRL_CCB_SIZE			(8)
#define MTX_CCB_CTRL_INIT_QP			(24)

#define MVEA_BASE				0xA3000
#define VLC_BASE				0xA5000

#define TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_RNW_MASK 0x00010000
#define TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK 0x80000000

enum tnc_topaz_encode_fw {
	FW_JPEG = 0,
	FW_H264_NO_RC,
	FW_H264_VBR,
	FW_H264_CBR,
	FW_H263_NO_RC,
	FW_H263_VBR,
	FW_H263_CBR,
	FW_MPEG4_NO_RC,
	FW_MPEG4_VBR,
	FW_MPEG4_CBR,
	FW_NUM
};

typedef struct _enc_fw_info {
	enum tnc_topaz_encode_fw idx;
	unsigned long *text_size;
	unsigned long *data_size;
	unsigned long *data_offset;
	unsigned long *text;
	unsigned long *data;
	char *fw_version;
}enc_fw_info_t;

struct topaz_cmd_header {
	union {
		struct {
			unsigned long enable_interrupt:1;
			unsigned long id:7;
			unsigned long size:8;
			unsigned long seq:16;
		};
		unsigned int val;
	};
};

/* commands for topaz,shared with user space driver */
enum drm_lnc_topaz_cmd {
	MTX_CMDID_NULL = 0,
	MTX_CMDID_DO_HEADER = 1,
	MTX_CMDID_ENCODE_SLICE = 2,
	MTX_CMDID_WRITEREG = 3,
	MTX_CMDID_START_PIC = 4,
	MTX_CMDID_END_PIC = 5,
	MTX_CMDID_SYNC = 6,
	MTX_CMDID_ENCODE_ONE_ROW = 7,
	MTX_CMDID_FLUSH = 8,
	MTX_CMDID_SW_LEAVE_LOWPOWER = 0x7c,
	MTX_CMDID_SW_ENTER_LOWPOWER = 0x7e,
	MTX_CMDID_SW_NEW_CODEC = 0x7f
};

extern int topaz_setup_fw(igd_context_t *context, enum tnc_topaz_encode_fw codec);
int process_video_encode_tnc(igd_context_t *context, unsigned long offset,
		void *mem_handle, unsigned long *fence_id);
int topaz_init_tnc(unsigned long wb_offset, void* mem_handle_writeback,
		void *mem_handle_enc_fw);
int topaz_get_fence_id(igd_context_t *context, unsigned long *fence_id);
int topaz_flush_tnc(igd_context_t *context);
int topaz_get_frame_skip(igd_context_t *context, unsigned long *frame_skip);
int topaz_shutdown_tnc(igd_context_t *context);

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: msvdx.h
 * $Revision: 1.21 $
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
 *  These are the defines specific to the MSDVX engine code.
 *-----------------------------------------------------------------------------
 */
#include <linux/list.h>
#include <context.h>


extern unsigned long _msvdx_base;
#define MSVDX_BASE _msvdx_base

#ifndef _MSVDX_H
#define _MSVDX_H

/* MTX registers */
#define PSB_MSVDX_MTX_ENABLE                      (MSVDX_BASE + 0x0000)
#define PSB_MSVDX_MTX_STATUS                      (MSVDX_BASE + 0x0008)
#define PSB_MSVDX_MTX_KICK                        (MSVDX_BASE + 0x0080)
#define PSB_MSVDX_MTX_KICKI                       (MSVDX_BASE + 0x0088)
#define PSB_MSVDX_MTX_FAULT0                      (MSVDX_BASE + 0x0090)
#define PSB_MSVDX_MTX_REGISTER_READ_WRITE_DATA    (MSVDX_BASE + 0x00f8)
#define PSB_MSVDX_MTX_REGISTER_READ_WRITE_REQUEST (MSVDX_BASE + 0x00fc)
#define PSB_MSVDX_MTX_RAM_ACCESS_DATA_EXCHANGE    (MSVDX_BASE + 0x0100)
#define PSB_MSVDX_MTX_RAM_ACCESS_DATA_TRANSFER    (MSVDX_BASE + 0x0104)
#define PSB_MSVDX_MTX_RAM_ACCESS_CONTROL          (MSVDX_BASE + 0x0108)
#define PSB_MSVDX_MTX_RAM_ACCESS_STATUS           (MSVDX_BASE + 0x010c)
#define PSB_MSVDX_MTX_SOFT_RESET                  (MSVDX_BASE + 0x0200)
#define PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_TIMERDIV_OFFSET (MSVDX_BASE + 0x0208)

#define PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAC      (MSVDX_BASE + 0x0340)
#define PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAA      (MSVDX_BASE + 0x0344)
#define PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAS0     (MSVDX_BASE + 0x0348)
#define PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAT      (MSVDX_BASE + 0x0350)

#define PSB_MSVDX_DMAC_SETUP                      (MSVDX_BASE + 0x0500)
#define PSB_MSVDX_DMAC_COUNT                      (MSVDX_BASE + 0x0504)
#define PSB_MSVDX_DMAC_PERIPH                     (MSVDX_BASE + 0x0508)
#define PSB_MSVDX_DMAC_IRQ_STAT                   (MSVDX_BASE + 0x050c)
#define PSB_MSVDX_DMAC_PERIPHERAL_ADDR            (MSVDX_BASE + 0x0514)
/* MSVDX registers */
#define PSB_MSVDX_CONTROL                         (MSVDX_BASE + 0x0600)
#define PSB_MSVDX_INTERRUPT_STATUS                (MSVDX_BASE + 0x0608)
#define PSB_MSVDX_INTERRUPT_CLEAR                 (MSVDX_BASE + 0x060c)
#define PSB_MSVDX_HOST_INTERRUPT_ENABLE           (MSVDX_BASE + 0x0610)
#define PSB_MSVDX_MAN_CLK_ENABLE                  (MSVDX_BASE + 0x0620)
#define PSB_MSVDX_CORE_REV                        (MSVDX_BASE + 0x0640)
#define PSB_MSVDX_MMU_CONTROL0                    (MSVDX_BASE + 0x0680)
#define PSB_MSVDX_MMU_CONTROL1                    (MSVDX_BASE + 0x0684)
#define PSB_MSVDX_MMU_BANK_INDEX                  (MSVDX_BASE + 0x0688)
#define PSB_MSVDX_MMU_STATUS                      (MSVDX_BASE + 0x068c)
#define PSB_MSVDX_MMU_DIR_LIST_BASE0              (MSVDX_BASE + 0x0694)
#define PSB_MSVDX_MMU_DIR_LIST_BASE1              (MSVDX_BASE + 0x0698)
#define PSB_MSVDX_MMU_DIR_LIST_BASE2              (MSVDX_BASE + 0x069c)
#define PSB_MSVDX_MMU_DIR_LIST_BASE3              (MSVDX_BASE + 0x06a0)
#define PSB_MSVDX_MMU_MEM_REQ                     (MSVDX_BASE + 0x06d0)
#define PSB_MSVDX_MTX_RAM_BANK                    (MSVDX_BASE + 0x06f0)
/* RENDEC registers */
#define PSB_MSVDX_RENDEC_CONTROL0                 (MSVDX_BASE + 0x0868)
#define PSB_MSVDX_RENDEC_CONTROL1                 (MSVDX_BASE + 0x086C)
#define PSB_MSVDX_RENDEC_BUFFER_SIZE              (MSVDX_BASE + 0x0870)
#define PSB_MSVDX_RENDEC_BASE_ADDR0               (MSVDX_BASE + 0x0874)
#define PSB_MSVDX_RENDEC_BASE_ADDR1               (MSVDX_BASE + 0x0878)
#define PSB_MSVDX_RENDEC_READ_DATA                (MSVDX_BASE + 0x0898)
#define PSB_MSVDX_RENDEC_CONTEXT0                 (MSVDX_BASE + 0x0950)
#define PSB_MSVDX_RENDEC_CONTEXT1                 (MSVDX_BASE + 0x0954)
#define PSB_MSVDX_RENDEC_CONTEXT2                 (MSVDX_BASE + 0x0958)
#define PSB_MSVDX_RENDEC_CONTEXT3                 (MSVDX_BASE + 0x095C)
#define PSB_MSVDX_RENDEC_CONTEXT4                 (MSVDX_BASE + 0x0960)
#define PSB_MSVDX_RENDEC_CONTEXT5                 (MSVDX_BASE + 0x0964)

#define MSVDX_COMMS_AREA_ADDR                     (MSVDX_BASE + 0x02fd0)
#define PSB_MSVDX_COMMS_FW_STATUS                 (MSVDX_COMMS_AREA_ADDR + 0x00)
#define PSB_MSVDX_COMMS_VLR_RES                   (MSVDX_COMMS_AREA_ADDR + 0x04)
#define PSB_MSVDX_COMMS_SCRATCH                   (MSVDX_COMMS_AREA_ADDR + 0x08)
#define PSB_MSVDX_COMMS_MSG_COUNTER               (MSVDX_COMMS_AREA_ADDR + 0x0c)
#define PSB_MSVDX_COMMS_SIGNATURE                 (MSVDX_COMMS_AREA_ADDR + 0x10)
#define PSB_MSVDX_COMMS_TO_HOST_BUF_SIZE          (MSVDX_COMMS_AREA_ADDR + 0x14)
#define PSB_MSVDX_COMMS_TO_HOST_RD_INDEX          (MSVDX_COMMS_AREA_ADDR + 0x18)
#define PSB_MSVDX_COMMS_TO_HOST_WRT_INDEX         (MSVDX_COMMS_AREA_ADDR + 0x1c)
#define PSB_MSVDX_COMMS_TO_MTX_BUF_SIZE           (MSVDX_COMMS_AREA_ADDR + 0x20)
#define PSB_MSVDX_COMMS_TO_MTX_RD_INDEX           (MSVDX_COMMS_AREA_ADDR + 0x24)
#define PSB_MSVDX_COMMS_OFFSET_FLAGS              (MSVDX_COMMS_AREA_ADDR + 0x28)
#define PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX          (MSVDX_COMMS_AREA_ADDR + 0x2c)

#define MTX_CORE_CODE_MEM                         (0x10)
#define MTX_CORE_DATA_MEM                         (0x18)
#define MTX_CODE_BASE                             (0x80900000)
#define MTX_DATA_BASE                             (0x82880000)
#define PC_START_ADDRESS                          (0x80900000)
#define MSVDX_MTX_ENABLE_MTX_ENABLE_MASK          (0x00000001)
#define MTX_PC                                    (5)
#define RENDEC_A_SIZE                             (1024 * 1024 * 2)
#define RENDEC_B_SIZE                             (RENDEC_A_SIZE / 4)
#define FWRK_PADMSG_SIZE                          (2)
#define FWRK_MSGID_PADDING                        (0)
#define FWRK_MSGID_START_PSR_HOSTMTX_MSG          (0x80)
#define FWRK_MSGID_START_PSR_MTXHOST_MSG          (0xc0)
#define MSVDX_CLK_ENABLE_CR_CORE_MASK             (0x00000001)
#define MSVDX_CLK_ENABLE_CR_VDEB_PROCESS_MASK     (0x00000002)
#define MSVDX_CLK_ENABLE_CR_VDEB_ACCESS_MASK      (0x00000004)
#define MSVDX_CLK_ENABLE_CR_VDMC_MASK             (0x00000008)
#define MSVDX_CLK_ENABLE_CR_VEC_ENTDEC_MASK       (0x00000010)
#define MSVDX_CLK_ENABLE_CR_VEC_ITRANS_MASK       (0x00000020)
#define MSVDX_CLK_ENABLE_CR_MTX_MASK              (0x00000040)
#define MSVDX_CLK_ENABLE_CR_VDEB_ACCESS_AUTO_MASK (0x00040000)
#define MSVDX_CLK_ENABLE_CR_VDMC_AUTO_MASK        (0x00080000)
#define MSVDX_CLK_ENABLE_CR_VEC_ENTDEC_AUTO_MASK  (0x00100000)
#define MSVDX_CLK_ENABLE_CR_VEC_ITRANS_AUTO_MASK  (0x00200000)


#define PSB_CLK_ENABLE_ALL \
	MSVDX_CLK_ENABLE_CR_CORE_MASK |\
	MSVDX_CLK_ENABLE_CR_VDEB_PROCESS_MASK |\
	MSVDX_CLK_ENABLE_CR_VDEB_ACCESS_MASK |\
	MSVDX_CLK_ENABLE_CR_VDMC_MASK |\
	MSVDX_CLK_ENABLE_CR_VEC_ENTDEC_MASK |\
	MSVDX_CLK_ENABLE_CR_VEC_ITRANS_MASK |\
	MSVDX_CLK_ENABLE_CR_MTX_MASK

#define PSB_CLK_ENABLE_MIN    MSVDX_CLK_ENABLE_CR_CORE_MASK
#define PSB_MSVDX_FW_STATUS_HW_IDLE                 (0x00000001)

#define MSVDX_DEVICE_NODE_FLAGS_MMU_NONOPT_INV      (0x00000002)
#define MSVDX_DEVICE_NODE_FLAGS_MMU_HW_INVALIDATION (0x00000020)
#define MSVDX_DEVICE_NODE_FLAG_BRN23154_BLOCK_ON_FE (0x00000200)

#define MSVDX_DEVICE_NODE_FLAGS_DEFAULT \
	MSVDX_DEVICE_NODE_FLAGS_MMU_HW_INVALIDATION


#define FW_VA_RENDER_HOST_INT		0x00004000

#ifndef list_first_entry
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)
#endif

enum {
	/*! Sent by the video driver on the host to the mtx firmware. */
	IGD_MSGID_INIT               = FWRK_MSGID_START_PSR_HOSTMTX_MSG,
	IGD_MSGID_RENDER,
	IGD_MSGID_DEBLOCK,
	IGD_MSGID_BUBBLE,

	/* Test Messages */
	IGD_MSGID_TEST1,
	IGD_MSGID_TEST2,

	/*! Sent by the mtx firmware to itself. */
	IGD_MSGID_RENDER_MC_INTERRUPT,

	/*! Sent by the DXVA firmware on the MTX to the host. */
	IGD_MSGID_CMD_COMPLETED  = FWRK_MSGID_START_PSR_MTXHOST_MSG,
	IGD_MSGID_CMD_COMPLETED_BATCH,
	IGD_MSGID_DEBLOCK_REQUIRED,
	IGD_MSGID_TEST_RESPONCE,
	IGD_MSGID_ACK,

	IGD_MSGID_CMD_FAILED,
	IGD_MSGID_CMD_UNSUPPORTED,
	IGD_MSGID_CMD_HW_PANIC,
};

struct msvdx_cmd_queue {
	struct list_head head;
	void *cmd;
	unsigned long cmd_size;
	unsigned long context_id;
};

/* TODO:  From UMG, temporary put here first, may need to use this
 * MSVDX private structure
 */

struct msvdx_private {
	int msvdx_needs_reset;

	unsigned int pmstate;

	struct sysfs_dirent *sysfs_pmstate;

	uint32_t msvdx_current_sequence;
	uint32_t msvdx_last_sequence;

	/*
	 * MSVDX Rendec Memory
	 */
	uint32_t base_addr0;
	uint32_t base_addr1;

	/*
	 * msvdx command queue
	 */
	/* spinlock_t msvdx_lock; */
	/* struct mutex msvdx_mutex; */
	struct list_head msvdx_queue;
	int msvdx_busy;
	int msvdx_fw_loaded;
	void *msvdx_fw;
	int msvdx_fw_size;

	struct list_head deblock_queue; /* deblock parameter list */

	uint32_t msvdx_hw_busy;
};

typedef struct msvdx_fw_ {
	unsigned long fw_text_size;
	unsigned long *fw_text;
	unsigned long fw_data_location;
	unsigned long fw_data_size;
	unsigned long *fw_data;
	unsigned long fw_version_size;
	char *fw_version;
} msvdx_fw_t;


int process_video_decode_plb(igd_context_t *context, unsigned long offset,
		void *mem_handle, unsigned long *fence_id);
int msvdx_query_plb(igd_context_t *context, unsigned long *status);
int msvdx_preinit_mmu(unsigned long hmemcxt);
int msvdx_init_plb(unsigned long base0, unsigned long base1,
					void* mem_handle_fw,int reset_flag);
int msvdx_init_compositor_mmu(unsigned long mmu_base);
int msvdx_uninit_plb(igd_context_t *context);
int msvdx_close_context(igd_context_t *context, unsigned long context_id);
int msvdx_create_context(igd_context_t *context, void *drm_file_priv, unsigned long ctx_id);
int msvdx_shutdown_plb(igd_context_t *context);
int msvdx_get_fence_id(igd_context_t *context, unsigned long *fence_id);
int msvdx_flush_tlb(igd_context_t *context);
void msvdx_postclose_check(igd_context_t *context, void *drm_file_priv);

#endif

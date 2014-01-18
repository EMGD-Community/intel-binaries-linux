/*
 *-----------------------------------------------------------------------------
 * Filename: context.h
 * $Revision: 1.17 $
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

#ifndef _HAL_PLB_CONTEXT_H
#define _HAL_PLB_CONTEXT_H

#include <sched.h>

#include <pci.h>
#include <igd_render.h>
#include <plb/sgx.h>
#include <servicesint.h>
/*
 * FIXME: Promote io_mapped/io_base to DI layer
 *
 * Note: This define is for the vBIOS OAL only. Do not use
 * it anywhere else, use the actual type name.
 */
#define PLATFORM_CONTEXT_T platform_context_plb_t

typedef struct psb_use_reg {
	unsigned long reg_seq;
	unsigned long base;
	unsigned long size;
	unsigned long data_master;
	unsigned char * virt;
} psb_use_reg_t;

typedef struct drmBO {
	unsigned long offset;
} drmBO_t;

typedef struct psb_closed_dpm {
	drmBO_t *page_table_bo;
	drmBO_t *parameter_bo;
	unsigned int num_pages;
	unsigned int context_id;
	unsigned int ta_global_list;
	unsigned int ta_threshold;
	unsigned int zls_threshold;
} psb_closed_dpm_t;

typedef struct _psb_sgx_priv {

	/* HW workaround table */
	/* ***********************************************/
    struct pclosed_vopt   vopt;

	/* stuff required to be setup by sgx_init in cmd */
	/* ***********************************************/
	struct psb_use_reg    use_code[SGX_MAX_USSE_THRDS];
	igd_dma_t             drm_bo[DRM_BO_MEM_TYPES];

	igd_dma_t             commBO;
	igd_dma_t             codeBO;
	igd_dma_t             sProg;
	igd_dma_t             geom;
	igd_dma_t             local;

	unsigned long         usse_reg_dm;
	unsigned long         num_use_attribute_registers;

	psb_closed_dpm_t      dpms[2];
	/* What is an igd_command variable doing here?!
	 * should we move this into an appcontext_plb
	 * structure and let psb_sgx_priv_t have a ptr
	 * to the active appcontext_plb pointer? i.e. an
	 * appcontext created for 3d context?
	 */
	igd_command_t         context_select;

	/* stuff required to be setup by sgx_init in gart */
	/* ***********************************************/
	unsigned int          cache_ctrl;

	/* state required to be setup by sgx_init in pwr */
	/* ***********************************************/
} psb_sgx_priv_t;

/* Values used in platform_context_plb_t->flip_pending
 * This corresponds to the pipe, which is a bit strange,
 * but since the flip must wait for a vBlank, it is
 * based off the PIPE */
#define PLB_FLIP_PIPE_A_PENDING 1
#define PLB_FLIP_PIPE_B_PENDING 2

typedef struct _tnc_topaz_priv {

	/* current video task */
	unsigned long topaz_cur_codec;
	unsigned long cur_mtx_data_size;
	int topaz_needs_reset;
	int topaz_start_idle;
	unsigned long topaz_idle_start_jiffies;
	/* used by topaz_lockup */
	unsigned long topaz_current_sequence;
	unsigned long topaz_last_sequence;
	unsigned long topaz_finished_sequence;

	/*
	 * topaz command queueu
	 */
	int topaz_busy;		/* 0 means topaz is free */
	int topaz_fw_loaded;

	/* topaz ccb data */
	unsigned long topaz_ccb_buffer_addr;
	unsigned long topaz_ccb_ctrl_addr;
	unsigned long topaz_ccb_size;
	unsigned long topaz_cmd_windex;
	unsigned short topaz_cmd_seq;

	unsigned long stored_initial_qp;
	unsigned long topaz_frame_skip;
	unsigned long topaz_dash_access_ctrl;

	unsigned char *topaz_ccb_wb;
	unsigned long topaz_wb_offset;
	unsigned long *topaz_sync_addr;
	unsigned long topaz_sync_offset;
	unsigned long topaz_sync_cmd_seq;
	unsigned long topaz_sync_id;
	/**
	 * Virtual address to writeback memory in the aperture space.
	 */
	unsigned char *virt_wb;
	/**
	 * Offset in gmm space for write back memory.
	 */
	unsigned long wb_offset;
	int selected_codec;
} tnc_topaz_priv_t;

struct msvdx_pvr_info;

typedef struct _platform_context_plb {
	int irq;
	unsigned short did;
	os_pci_dev_t pcidev0;
	os_pci_dev_t pcidev1;
	os_pci_dev_t lpc_dev;
	os_pci_dev_t bridgedev;
	unsigned char tnc_dev3_rid;             /* TNC Device 3 RID*/
	os_pci_dev_t stbridgedev;
	os_pci_dev_t stgpiodev;
	unsigned long rendec_base0;
	unsigned long rendec_base1;
	/*
	 * Cached value of the SGX's PSB_CR_BIF_DIR_LIST_BASE1, which is
	 * used to configure MSVDX MMU base 0.
	 */
	unsigned long psb_cr_bif_dir_list_base1;
	int msvdx_needs_reset;
    spinlock_t msvdx_lock;
    spinlock_t msvdx_init_plb;
	spinlock_t topaz_init_tnc;
    unsigned long msvdx_status;
    int msvdx_busy;
    struct list_head msvdx_queue;
	unsigned long msvdx_dash_access_ctrl;
	struct msvdx_pvr_info *msvdx_pvr;
	psb_sgx_priv_t sgx_priv_data;
	tnc_topaz_priv_t tpz_private_data;
    unsigned long msvdx_fence;
	int topaz_busy;
	unsigned long src_pat_data_offset;
	unsigned long glyph_data_offset;
	unsigned long sequence;
	unsigned long mtx_submitted;
	unsigned long mtx_completed;
	unsigned long mtx_buf_size;
	unsigned long host_buf_size;
	unsigned long mtx_buf_offset;
	unsigned long host_buf_offset;
	/* Flip pending. This is used in the mode
	 * module, but it is intialized in the cmd
	 * module along with the other mutex-es */
	unsigned int flip_pending;
        os_pthread_mutex_t flip_mutex;
	int force_polling;
	int irq_enabled;
} platform_context_plb_t, platform_context_tnc_t;

#endif

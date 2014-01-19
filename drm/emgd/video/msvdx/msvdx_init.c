/*
 *-----------------------------------------------------------------------------
 * Filename: msvdx_init.c
 * $Revision: 1.32 $
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
 *  Initialize the MSVDX video engine.  This loads the MTX firmware and
 *  starts a MTX thread running the firmware.
 *  The host communicates with the firmware via messages. The following
 *  messages are supported:
 *  INIT               -> MTX
 *  RENDER             -> MTX
 *  DEBLOCK            -> MTX
 *  BUBBLE             -> MTX
 *  TEST1              -> MTX
 *  TEST2              -> MTX
 *  CMD_COMPLETED      <- MTX
 *  CMD_COMPLTED_BATCH <- MTX
 *  DEBLOCK_REQUIRED   <- MTX
 *  TEST_RESPONSE      <- MTX
 *  ACK                <- MTX
 *  CMD_FAILED         <- MTX
 *-----------------------------------------------------------------------------
 */

#include <io.h>
#include <pci.h>
#include <memmap.h>
#include <sched.h>

#include <igd.h>
#include <igd_errno.h>
#include <igd_init.h>

#include <context.h>
#include <intelpci.h>
#include <general.h>
#include <utils.h>
#include <msvdx.h>

#include <plb/regs.h>
#include <plb/context.h>
#include <drm/drm.h>
#include <drm_emgd_private.h>
#include <emgd_drm.h>
#include <osfunc.h>  /* for OSFlushCPUCacheKM() */
#include "msvdx_pvr.h"

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#include <linux/module.h>
#include <linux/export.h>
#endif
struct drm_device *gpDrmDevice = NULL;
static int init_msvdx_first_time = 1;
static unsigned long msvdx_compositor_mmu_base = 0;
extern void send_to_mtx(igd_context_t *context, unsigned long *init_msg);
extern int process_mtx_messages(igd_context_t *context,
		unsigned long *mtx_msgs, unsigned long mtx_msg_cnt,
		unsigned long fence);

extern unsigned long populate_fence_id(igd_context_t *context, unsigned long *mtx_msgs,
		unsigned long mtx_msg_cnt);
extern int msvdx_dequeue_send(igd_context_t *context);
extern int alloc_ramdec_region(unsigned long *base_addr0, unsigned long *base_addr1,
				unsigned long size0, unsigned long size1);

static int poll_mtx_irq(igd_context_t *context);
static int reg_ready_psb(igd_context_t *context, unsigned long reg,
        unsigned long mask, unsigned long value);
int context_count = 0;

void msvdx_reset_plb(igd_context_t *context);

static msvdx_fw_t *priv_fw = NULL;

extern unsigned long jiffies_at_last_dequeue;
static int msvdx_fw_dma_upload = 1;

MODULE_PARM_DESC(msvdx_dma_upload, "MSVDX: upload firmware using DMA");
module_param_named(msvdx_dma_upload, msvdx_fw_dma_upload, int, 0600);
unsigned long LastClockState;

///// HHP TODO: Move to separate .h file
#define MSVDX_MTX_DEBUG      PSB_MSVDX_MTX_RAM_BANK

#define MSVDX_MTX_DEBUG_MTX_DBG_IS_SLAVE_MASK            (0x00000004)
#define MSVDX_MTX_DEBUG_MTX_DBG_IS_SLAVE_LSBMASK         (0x00000001)
#define MSVDX_MTX_DEBUG_MTX_DBG_IS_SLAVE_SHIFT           (2)

#define MSVDX_MTX_DEBUG_MTX_DBG_GPIO_IN_MASK             (0x00000003)
#define MSVDX_MTX_DEBUG_MTX_DBG_GPIO_IN_LSBMASK          (0x00000003)
#define MSVDX_MTX_DEBUG_MTX_DBG_GPIO_IN_SHIFT            (0)


#define MTX_CORE_CR_MTX_SYSC_CDMAC_BURSTSIZE_MASK        (0x07000000)
#define MTX_CORE_CR_MTX_SYSC_CDMAC_BURSTSIZE_SHIFT       (24)

#define MTX_CORE_CR_MTX_SYSC_CDMAC_RNW_MASK              (0x00020000)
#define MTX_CORE_CR_MTX_SYSC_CDMAC_RNW_SHIFT             (17)

#define MTX_CORE_CR_MTX_SYSC_CDMAC_ENABLE_MASK           (0x00010000)
#define MTX_CORE_CR_MTX_SYSC_CDMAC_ENABLE_SHIFT          (16)

#define MTX_CORE_CR_MTX_SYSC_CDMAC_LENGTH_MASK           (0x0000FFFF)
#define MTX_CORE_CR_MTX_SYSC_CDMAC_LENGTH_SHIFT          (0)

//#define MSVDX_CORE_CR_MSVDX_CONTROL_DMAC_CH0_SELECT_MASK   (0x00001000)
//#define MSVDX_CORE_CR_MSVDX_CONTROL_DMAC_CH0_SELECT_SHIFT  (12)

#define PSB_MSVDX_CONTROL_DMAC_CH0_SELECT_MASK           (0x00001000)
#define PSB_MSVDX_CONTROL_DMAC_CH0_SELECT_SHIFT          (12)


#define DMAC_DMAC_COUNT_BSWAP_LSBMASK                    (0x00000001)
#define DMAC_DMAC_COUNT_BSWAP_SHIFT                      (30)

#define DMAC_DMAC_COUNT_PW_LSBMASK                       (0x00000003)
#define DMAC_DMAC_COUNT_PW_SHIFT                         (27)

#define DMAC_DMAC_COUNT_DIR_LSBMASK                      (0x00000001)
#define DMAC_DMAC_COUNT_DIR_SHIFT                        (26)

#define DMAC_DMAC_COUNT_PI_LSBMASK                       (0x00000003)
#define DMAC_DMAC_COUNT_PI_SHIFT                         (24)

#define DMAC_DMAC_COUNT_EN_MASK                          (0x00010000)
#define DMAC_DMAC_COUNT_EN_SHIFT                         (16)

#define DMAC_DMAC_COUNT_CNT_LSBMASK                      (0x0000FFFF)
#define DMAC_DMAC_COUNT_CNT_SHIFT                        (0)


#define DMAC_DMAC_PERIPH_ACC_DEL_LSBMASK                 (0x00000007)
#define DMAC_DMAC_PERIPH_ACC_DEL_SHIFT                   (29)

#define DMAC_DMAC_PERIPH_INCR_LSBMASK                    (0x00000001)
#define DMAC_DMAC_PERIPH_INCR_SHIFT                      (27)

#define DMAC_DMAC_PERIPH_BURST_LSBMASK                   (0x00000007)
#define DMAC_DMAC_PERIPH_BURST_SHIFT                     (24)

#define DMAC_DMAC_PERIPHERAL_ADDR_ADDR_MASK              (0x007FFFFF)
#define DMAC_DMAC_PERIPHERAL_ADDR_ADDR_LSBMASK           (0x007FFFFF)
#define DMAC_DMAC_PERIPHERAL_ADDR_ADDR_SHIFT             (0)


#define DMAC_DMAC_IRQ_STAT_TRANSFER_FIN_MASK             (0x00020000)

/*watch dog for FE and BE*/
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_OFFSET		(MSVDX_BASE + 0x0064)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CNT_CTRL
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CNT_CTRL_MASK		(0x00060000)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CNT_CTRL_LSBMASK		(0x00000003)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CNT_CTRL_SHIFT		(17)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_ENABLE
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ENABLE_MASK		(0x00010000)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ENABLE_LSBMASK		(0x00000001)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ENABLE_SHIFT		(16)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_ACTION1
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION1_MASK		(0x00003000)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION1_LSBMASK		(0x00000003)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION1_SHIFT		(12)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_ACTION0
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION0_MASK		(0x00000100)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION0_LSBMASK		(0x00000001)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_ACTION0_SHIFT		(8)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CLEAR_SELECT
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLEAR_SELECT_MASK		(0x00000030)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLEAR_SELECT_LSBMASK		(0x00000003)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLEAR_SELECT_SHIFT		(4)

// MSVDX_CORE, CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CLKDIV_SELECT
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLKDIV_SELECT_MASK		(0x00000007)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLKDIV_SELECT_LSBMASK		(0x00000007)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_FE_WDT_CLKDIV_SELECT_SHIFT		(0)

#define MSVDX_CORE_CR_FE_MSVDX_WDTIMER_OFFSET		(MSVDX_BASE + 0x0068)

// MSVDX_CORE, CR_FE_MSVDX_WDTIMER, FE_WDT_COUNTER
#define MSVDX_CORE_CR_FE_MSVDX_WDTIMER_FE_WDT_COUNTER_MASK		(0x0000FFFF)
#define MSVDX_CORE_CR_FE_MSVDX_WDTIMER_FE_WDT_COUNTER_LSBMASK		(0x0000FFFF)
#define MSVDX_CORE_CR_FE_MSVDX_WDTIMER_FE_WDT_COUNTER_SHIFT		(0)

#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_OFFSET		(MSVDX_BASE + 0x006C)

// MSVDX_CORE, CR_FE_MSVDX_WDT_COMPAREMATCH, FE_WDT_CM1
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM1_MASK		(0xFFFF0000)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM1_LSBMASK		(0x0000FFFF)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM1_SHIFT		(16)

// MSVDX_CORE, CR_FE_MSVDX_WDT_COMPAREMATCH, FE_WDT_CM0
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM0_MASK		(0x0000FFFF)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM0_LSBMASK		(0x0000FFFF)
#define MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_FE_WDT_CM0_SHIFT		(0)

#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_OFFSET		(MSVDX_BASE + 0x0070)

// MSVDX_CORE, CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CNT_CTRL
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CNT_CTRL_MASK		(0x001E0000)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CNT_CTRL_LSBMASK		(0x0000000F)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CNT_CTRL_SHIFT		(17)

// MSVDX_CORE, CR_BE_MSVDX_WDT_CONTROL, BE_WDT_ENABLE
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ENABLE_MASK		(0x00010000)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ENABLE_LSBMASK		(0x00000001)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ENABLE_SHIFT		(16)

// MSVDX_CORE, CR_BE_MSVDX_WDT_CONTROL, BE_WDT_ACTION0
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ACTION0_MASK		(0x00000100)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ACTION0_LSBMASK		(0x00000001)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_ACTION0_SHIFT		(8)

// MSVDX_CORE, CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CLEAR_SELECT
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLEAR_SELECT_MASK		(0x000000F0)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLEAR_SELECT_LSBMASK		(0x0000000F)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLEAR_SELECT_SHIFT		(4)

// MSVDX_CORE, CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CLKDIV_SELECT
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLKDIV_SELECT_MASK		(0x00000007)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLKDIV_SELECT_LSBMASK		(0x00000007)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_BE_WDT_CLKDIV_SELECT_SHIFT		(0)

#define MSVDX_CORE_CR_BE_MSVDX_WDTIMER_OFFSET		(MSVDX_BASE + 0x0074)

// MSVDX_CORE, CR_BE_MSVDX_WDTIMER, BE_WDT_COUNTER
#define MSVDX_CORE_CR_BE_MSVDX_WDTIMER_BE_WDT_COUNTER_MASK		(0x0000FFFF)
#define MSVDX_CORE_CR_BE_MSVDX_WDTIMER_BE_WDT_COUNTER_LSBMASK		(0x0000FFFF)
#define MSVDX_CORE_CR_BE_MSVDX_WDTIMER_BE_WDT_COUNTER_SHIFT		(0)

#define MSVDX_CORE_CR_BE_MSVDX_WDT_COMPAREMATCH_OFFSET		(MSVDX_BASE + 0x0078)

// MSVDX_CORE, CR_BE_MSVDX_WDT_COMPAREMATCH, BE_WDT_CM0
#define MSVDX_CORE_CR_BE_MSVDX_WDT_COMPAREMATCH_BE_WDT_CM0_MASK		(0x0000FFFF)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_COMPAREMATCH_BE_WDT_CM0_LSBMASK		(0x0000FFFF)
#define MSVDX_CORE_CR_BE_MSVDX_WDT_COMPAREMATCH_BE_WDT_CM0_SHIFT		(0)
/*watch dog end*/

#define FW_SIZE 16128
#define MAX_FW_SIZE 16 * 1024

enum {
	MSVDX_DMAC_BSWAP_NO_SWAP = 0x0, /* No byte swapping will be performed */
	MSVDX_DMAC_BSWAP_REVERSE = 0x1, /* Byte order will be reversed */
};

enum {
	MSVDX_DMAC_DIR_MEM_TO_PERIPH = 0x0, /* Data from memory to peripheral */
	MSVDX_DMAC_DIR_PERIPH_TO_MEM = 0x1, /* Data from peripheral to memory */
};

enum {
	MSVDX_DMAC_ACC_DEL_0    = 0x0, /* Access delay zero clock cycles */
	MSVDX_DMAC_ACC_DEL_256  = 0x1, /* Access delay 256 clock cycles */
	MSVDX_DMAC_ACC_DEL_512  = 0x2, /* Access delay 512 clock cycles */
	MSVDX_DMAC_ACC_DEL_768  = 0x3, /* Access delay 768 clock cycles */
	MSVDX_DMAC_ACC_DEL_1024 = 0x4, /* Access delay 1024 clock cycles */
	MSVDX_DMAC_ACC_DEL_1280 = 0x5, /* Access delay 1280 clock cycles */
	MSVDX_DMAC_ACC_DEL_1536 = 0x6, /* Access delay 1536 clock cycles */
	MSVDX_DMAC_ACC_DEL_1792 = 0x7, /* Access delay 1792 clock cycles */
};

enum {
	MSVDX_DMAC_INCR_OFF  = 0x0, /* Static peripheral address */
	MSVDX_DMAC_INCR_ON   = 0x1, /* Incrementing peripheral address */
};

enum {
	MSVDX_DMAC_BURST_0   = 0x0, /* burst size of 0 */
	MSVDX_DMAC_BURST_1   = 0x1, /* burst size of 1 */
	MSVDX_DMAC_BURST_2   = 0x2, /* burst size of 2 */
	MSVDX_DMAC_BURST_3   = 0x3, /* burst size of 3 */
	MSVDX_DMAC_BURST_4   = 0x4, /* burst size of 4 */
	MSVDX_DMAC_BURST_5   = 0x5, /* burst size of 5 */
	MSVDX_DMAC_BURST_6   = 0x6, /* burst size of 6 */
	MSVDX_DMAC_BURST_7   = 0x7, /* burst size of 7 */
};

/* DMAC control */
#define MSVDX_DMAC_VALUE_COUNT(BSWAP,PW,DIR,PERIPH_INCR,COUNT) \
	(((BSWAP) & DMAC_DMAC_COUNT_BSWAP_LSBMASK) << DMAC_DMAC_COUNT_BSWAP_SHIFT) | \
	(((PW)    & DMAC_DMAC_COUNT_PW_LSBMASK)    << DMAC_DMAC_COUNT_PW_SHIFT)    | \
	(((DIR)   & DMAC_DMAC_COUNT_DIR_LSBMASK)   << DMAC_DMAC_COUNT_DIR_SHIFT)   | \
	(((PERIPH_INCR) & DMAC_DMAC_COUNT_PI_LSBMASK) << DMAC_DMAC_COUNT_PI_SHIFT) | \
	(((COUNT) & DMAC_DMAC_COUNT_CNT_LSBMASK)   << DMAC_DMAC_COUNT_CNT_SHIFT)

#define MSVDX_DMAC_VALUE_PERIPH_PARAM(ACC_DEL,INCR,BURST) \
	(((ACC_DEL) & DMAC_DMAC_PERIPH_ACC_DEL_LSBMASK) << DMAC_DMAC_PERIPH_ACC_DEL_SHIFT) | \
	(((INCR)    & DMAC_DMAC_PERIPH_INCR_LSBMASK)    << DMAC_DMAC_PERIPH_INCR_SHIFT)    | \
	(((BURST)   & DMAC_DMAC_PERIPH_BURST_LSBMASK)   << DMAC_DMAC_PERIPH_BURST_SHIFT)



#define REGIO_READ_FIELD(reg_val, reg, field)                           \
        ((reg_val & reg##_##field##_MASK) >> reg##_##field##_SHIFT)

#define REGIO_WRITE_FIELD(reg_val, reg, field, value)                   \
        (reg_val) =                                                     \
                ((reg_val) & ~(reg##_##field##_MASK)) |                 \
               (((value) << (reg##_##field##_SHIFT)) & (reg##_##field##_MASK));

#define REGIO_WRITE_FIELD_LITE(reg_val, reg, field, value)              \
        (reg_val) =                                                     \
                ((reg_val) | ((value) << (reg##_##field##_SHIFT)));

#define STACKGUARDWORD (0x10101010)

#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_CORE_MAN_CLK_ENABLE_MASK	\
	(0x00000001)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_VDEB_PROCESS_MAN_CLK_ENABLE_MASK \
	(0x00000002)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_VDEB_ACCESS_MAN_CLK_ENABLE_MASK \
	(0x00000004)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_VDMC_MAN_CLK_ENABLE_MASK	\
	(0x00000008)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_VEC_ENTDEC_MAN_CLK_ENABLE_MASK \
	(0x00000010)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_VEC_ITRANS_MAN_CLK_ENABLE_MASK \
	(0x00000020)
#define MSVDX_CORE_CR_MSVDX_MAN_CLK_ENABLE_CR_MTX_MAN_CLK_ENABLE_MASK	\
	(0x00000040)

typedef struct _msvdx_context {
	unsigned long context_id;
	void *drm_file_priv;
} msvdx_context_t;

#define MSVDX_MAXIMUM_CONTEXT     8

static msvdx_context_t msvdx_contexts[MSVDX_MAXIMUM_CONTEXT];


int msvdx_init_compositor_mmu(unsigned long mmu_base) {
	msvdx_compositor_mmu_base = mmu_base;
	return 0;
}

/*
 * Map and copy the firmware image to the shared SGX heap.
 */
static int msvdx_map_fw(uint32_t size)
{
	drm_emgd_priv_t       *priv;
	igd_context_t          *context;
	platform_context_plb_t *platform;
	PVRSRV_KERNEL_MEM_INFO *mem_info;
	unsigned long alloc_size;
	uint32_t *last_word;
	void     *mapped_fw_addr;

	priv     = gpDrmDevice->dev_private;
	context  = priv->context;
    platform = (platform_context_plb_t *)context->platform_context;

	/* Round up as DMA's can overrun a page */
	alloc_size = (size + 8192) & ~0x0fff;

	/* Verify there is enough memory for the firmware text */
	if( ((priv_fw->fw_text_size * 4) <= 0) ||
		((priv_fw->fw_text_size * 4) > size) ) {
		return -EINVAL;
	}

	if( ((priv_fw->fw_data_location - MTX_DATA_BASE) <=0) ||
		((priv_fw->fw_data_location - MTX_DATA_BASE) > size) ) {
		return -EINVAL;
	}

	/* Verify there is enough memory for the firmware data */
	if( ((priv_fw->fw_data_size * 4) <= 0) ||
		((priv_fw->fw_data_size * 4) > (size -
				(priv_fw->fw_data_location - MTX_DATA_BASE))) ) {
		return -EINVAL;
	}

	mem_info = platform->msvdx_pvr->fw_mem_info;
	if (!mem_info) {
		mem_info = msvdx_pvr_alloc_devmem(alloc_size, "MSVDX firmware");
		if (!mem_info) {
			printk(KERN_ERR "[EMGD] MSVDX: Failed to allocate %u "
				"bytes from SGX heap\n",
				(unsigned int)alloc_size);
			return -ENOMEM;
		}
		platform->msvdx_pvr->fw_mem_info = mem_info;
	}

	mapped_fw_addr = (unsigned long *)mem_info->pvLinAddrKM;

	memset(mapped_fw_addr, 0x00, size);

	memcpy(mapped_fw_addr, priv_fw->fw_text, priv_fw->fw_text_size * 4);

	memcpy(mapped_fw_addr + (priv_fw->fw_data_location - MTX_DATA_BASE),
		priv_fw->fw_data, priv_fw->fw_data_size * 4);

	/*
	 * Write a known value to the last word in MTX memory. Useful for
	 * detection of stack overruns.
	 */
	last_word = (uint32_t *)(mapped_fw_addr + size - sizeof(uint32_t));
	*last_word = STACKGUARDWORD;

	OSFlushCPUCacheRangeKM(mapped_fw_addr, mapped_fw_addr + size);

	return 0;
}

static void msvdx_get_mtx_control_from_dash(igd_context_t *context)
{
	platform_context_plb_t *platform;
	unsigned char *mmio;
	int count = 0;
	uint32_t reg_val = 0;

	mmio = context->device_context.virt_mmadr;
        platform = (platform_context_plb_t *)context->platform_context;

	REGIO_WRITE_FIELD(reg_val, MSVDX_MTX_DEBUG, MTX_DBG_IS_SLAVE, 1);
	REGIO_WRITE_FIELD(reg_val, MSVDX_MTX_DEBUG, MTX_DBG_GPIO_IN, 0x02);
	EMGD_WRITE32(reg_val, mmio + MSVDX_MTX_DEBUG);

	do {		reg_val = EMGD_READ32(mmio + MSVDX_MTX_DEBUG);
		count++;
	} while (((reg_val & 0x18) != 0) && count < 50000);

	if (count >= 50000)
		printk(KERN_ERR "[EMGD] MSVDX: timeout in %s\n", __FUNCTION__);

	/* Save the RAM access control register */
	platform->msvdx_dash_access_ctrl =
		EMGD_READ32(mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);
}

static void msvdx_release_mtx_control_from_dash(igd_context_t *context)
{
	platform_context_plb_t *platform;
	unsigned char *mmio;

	mmio = context->device_context.virt_mmadr;
        platform = (platform_context_plb_t *)context->platform_context;

	/* Restore access control */
	EMGD_WRITE32(platform->msvdx_dash_access_ctrl,
		mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);

	/* Release the bus */
	EMGD_WRITE32(0x04, mmio + MSVDX_MTX_DEBUG);
}

static int msvdx_upload_fw_dma(uint32_t address)
{
	drm_emgd_priv_t       *priv;
	igd_context_t          *context;
	platform_context_plb_t *platform;
	unsigned long addr;
	unsigned char *mmio;
	uint32_t core_rev;
	uint32_t size;
	uint32_t cmd;
	uint32_t count_reg;
	uint32_t dma_channel;
	uint32_t reg_val;
	int ret;

    printk(KERN_INFO "MSVDX: Upload firmware by DMA\n");

	priv     = gpDrmDevice->dev_private;
	context  = priv->context;
	mmio     = context->device_context.virt_mmadr;
       platform = (platform_context_plb_t *)context->platform_context;

	dma_channel = 0; /* Use DMA channel 0 */

	core_rev = EMGD_READ32(mmio + PSB_MSVDX_CORE_REV);
	if ((core_rev & 0xffffff) < 0x020000)
		size = 16 * 1024; /* mtx_mem_size */
	else
		size = 40 * 1024;

	if (platform->msvdx_pvr && !platform->msvdx_pvr->fw_mem_info) {
		ret = msvdx_map_fw(size);
		if (ret)
			return ret;
	}

	msvdx_get_mtx_control_from_dash(context);

	/*
	 * dma transfers to/from the mtx have to be 32-bit aligned and in
	 * multiples of 32 bits
	 */
	EMGD_WRITE32(address, mmio + PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAA);

	reg_val = 0;
	/* Burst size in multiples of 64 bytes (allowed values are 2 or 4) */
	REGIO_WRITE_FIELD_LITE(reg_val, MTX_CORE_CR_MTX_SYSC_CDMAC, BURSTSIZE, 4);

	/* False means write to MTX mem, true means read from MTX mem */
	REGIO_WRITE_FIELD_LITE(reg_val, MTX_CORE_CR_MTX_SYSC_CDMAC, RNW, 0);

	/* Begin transfer */
	REGIO_WRITE_FIELD_LITE(reg_val, MTX_CORE_CR_MTX_SYSC_CDMAC, ENABLE, 1);

	/* DMA transfer is in size of 32-bit words */
	REGIO_WRITE_FIELD_LITE(reg_val, MTX_CORE_CR_MTX_SYSC_CDMAC, LENGTH, (size/4));

	EMGD_WRITE32(reg_val, mmio + PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAC);

	/* Toggle channel 0 usage between MTX and other MSVDX peripherals */
	reg_val = EMGD_READ32(mmio + PSB_MSVDX_CONTROL);
	REGIO_WRITE_FIELD(reg_val, PSB_MSVDX_CONTROL, DMAC_CH0_SELECT, 0);
	EMGD_WRITE32(reg_val, mmio + PSB_MSVDX_CONTROL);

	/* Clear the DMAC Stats */
	EMGD_WRITE32(0, mmio + PSB_MSVDX_DMAC_IRQ_STAT + (dma_channel * 0x20));

	addr = platform->msvdx_pvr->fw_mem_info->sDevVAddr.uiAddr;

	/* Use bank 0 */
	EMGD_WRITE32(0, mmio + PSB_MSVDX_MMU_BANK_INDEX);

	/* Use the same MMU PTD as the SGX for MMU base 0 */
	EMGD_WRITE32(platform->psb_cr_bif_dir_list_base1,
		mmio + PSB_MSVDX_MMU_DIR_LIST_BASE0);

	/* Invalidate and flush TLB */
	msvdx_flush_tlb(context);

	EMGD_WRITE32(addr,
		mmio + PSB_MSVDX_DMAC_SETUP + (dma_channel * 0x20));

	/* Only use single DMA - assert that this is valid */
	if ((size / 4) >= (1 << 15)) {
		printk(KERN_ERR "[EMGD] MSVDX: DMA size beyond limit. "
			"Firmware uploading aborted.\n");
        msvdx_release_mtx_control_from_dash(context);
		return -ENODEV;
	}

        count_reg = MSVDX_DMAC_VALUE_COUNT(MSVDX_DMAC_BSWAP_NO_SWAP,
					0,  /* 32 bits */
					MSVDX_DMAC_DIR_MEM_TO_PERIPH,
					0,
					(size / 4));

	/* Set the number of bytes to DMA */
	EMGD_WRITE32(count_reg,
		mmio + PSB_MSVDX_DMAC_COUNT + (dma_channel * 0x20));

	cmd = MSVDX_DMAC_VALUE_PERIPH_PARAM(MSVDX_DMAC_ACC_DEL_0,
					MSVDX_DMAC_INCR_OFF,
					MSVDX_DMAC_BURST_2);
	EMGD_WRITE32(cmd, mmio + PSB_MSVDX_DMAC_PERIPH + (dma_channel * 0x20));

	/* Set the destination port for DMA */
	cmd = 0;
	REGIO_WRITE_FIELD(cmd, DMAC_DMAC_PERIPHERAL_ADDR,
		ADDR, (PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAT - MSVDX_BASE));
	EMGD_WRITE32(cmd,
		mmio + PSB_MSVDX_DMAC_PERIPHERAL_ADDR + (dma_channel * 0x20));

	/* Finally, rewrite the count register with the enable bit set */
	EMGD_WRITE32(count_reg | DMAC_DMAC_COUNT_EN_MASK,
		mmio + PSB_MSVDX_DMAC_COUNT + (dma_channel * 0x20));

	/* Wait for DMA to complete */
	if (reg_ready_psb(context,
			PSB_MSVDX_DMAC_IRQ_STAT + (dma_channel * 0x20),
			DMAC_DMAC_IRQ_STAT_TRANSFER_FIN_MASK,
			DMAC_DMAC_IRQ_STAT_TRANSFER_FIN_MASK)) {
		msvdx_release_mtx_control_from_dash(context);

		printk(KERN_ERR "[EMGD] MSVDX: DMA firmware upload timed "
			"out\n");

		return -ENODEV;
	}

	/* Assert that the MTX DMA port is done */
	if (reg_ready_psb(context,
			PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_CDMAS0, 1, 1)) {
		msvdx_release_mtx_control_from_dash(context);
		printk(KERN_ERR "[EMGD] MSVDX: MTX DMA port done timed out\n");
		return -ENODEV;
	}

	msvdx_release_mtx_control_from_dash(context);
    printk(KERN_INFO "[EMGD] MSVDX: firmware DMA upload done!\n");
	return 0;
}

/*
 * Upload firmware using PIO
*/
static int msvdx_upload_fw(void)
{
	drm_emgd_priv_t *priv;
	igd_context_t *context;
	unsigned char *mmio;
	unsigned long ram_bank;
	unsigned long bank_size;
	unsigned long current_bank;
	unsigned long acc_control;
	unsigned long address;
	unsigned long fw_size;
	unsigned long *fw_data;
	unsigned long ram_id;
	unsigned long ctrl;
	unsigned long i;

	priv    = gpDrmDevice->dev_private;
	context = priv->context;
	mmio    = context->device_context.virt_mmadr;

	/*
	 * Get the ram bank size
	 * The banks size seems to be a 4 bit value in the MTX debug register.
	 */
	ram_bank = EMGD_READ32(mmio + PSB_MSVDX_MTX_RAM_BANK);
	bank_size = (ram_bank & 0x000f0000) >> 16;
	bank_size = (1 << (bank_size + 2));

	/* Save RAM access control register */
	acc_control = EMGD_READ32(mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);

	/* Loop writing text/code to core memory */
	current_bank = ~0L;
	address = PC_START_ADDRESS - MTX_CODE_BASE;

	fw_data = priv_fw->fw_text;
	fw_size = priv_fw->fw_text_size;

	for (i = 0; i < fw_size; i++) {
		/* Wait for MCMSTAT to become be idle 1 */
		if (reg_ready_psb(context, PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
					0xffffffff, 0x00000001) == 0) {
			ram_id = MTX_CORE_CODE_MEM + (address / bank_size);
			if (ram_id != current_bank) {
				/*
				 * bits 20:27 - ram bank (CODE_BASE | DATA_BASE)
				 * bits  2:19 - address
				 * bit   1    - enable auto increment
				 *              addressing mode
				 */
				ctrl = (ram_id << 20) | (((address >> 2) & 0x000ffffc) << 2) | 0x02;
				EMGD_WRITE32(ctrl, mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);

				current_bank = ram_id;
				/* Wait for MCMSTAT to become be idle 1 */
				reg_ready_psb(context,
					PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
					0xffffffff, 0x00000001);
			}

			address +=  4;
			EMGD_WRITE32(fw_data[i],
				mmio + PSB_MSVDX_MTX_RAM_ACCESS_DATA_TRANSFER);
		} else {
			printk(KERN_ERR
				"[EMGD] MSVDX: Timeout waiting for MCMSTAT "
				"to be idle\n");
		}
	}

	/* verify firmware upload. */
	current_bank = ~0L;
	address = PC_START_ADDRESS - MTX_CODE_BASE;

	for (i = 0; i < fw_size; i++) {
		if (reg_ready_psb(context, PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
				0xffffffff, 0x00000001) == 0) {
			ram_id = MTX_CORE_CODE_MEM + (address / bank_size);
			if (ram_id != current_bank) {
				/*
				 * bits 20:27 - ram bank (CODE_BASE | DATA_BASE)
				 * bits  2:19 - address
				 * bit   1    - enable auto increment
				 *              addressing mode
				 */
				ctrl = (ram_id << 20) | (((address >> 2) & 0x000ffffc) << 2) | 0x03;
				EMGD_WRITE32(ctrl, mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);
				current_bank = ram_id;
				reg_ready_psb(context,
					PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
					0xffffffff, 0x00000001);
			}

			address +=  4;
			if (EMGD_READ32(mmio + PSB_MSVDX_MTX_RAM_ACCESS_DATA_TRANSFER) !=
				fw_data[i]) {
				printk(KERN_ERR "Verify Error at index %ld\n", i);
			}
		} else {
			printk(KERN_ERR "Timeout waiting for MCMSTAT to be idle while verifying\n");
		}
	}

	fw_data = priv_fw->fw_data;
	fw_size = priv_fw->fw_data_size;

	/* Loop writing data to core memory */
	current_bank = ~0L;
	address = priv_fw->fw_data_location - MTX_DATA_BASE;

	for (i = 0; i < fw_size; i++) {
		if (reg_ready_psb(context, PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
				0xffffffff, 0x00000001) == 0) {
			ram_id = MTX_CORE_DATA_MEM + (address / bank_size);
			if (ram_id != current_bank) {
				/*
				 * bits 20:27 - ram bank (CODE_BASE | DATA_BASE)
				 * bits  2:19 - address
				 * bit   1    - enable auto increment
				 *              addressing mode
				 */
				ctrl = (ram_id << 20) | (((address >> 2) & 0x000ffffc) << 2) | 0x02;
				EMGD_WRITE32(ctrl, mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);
				current_bank = ram_id;
				reg_ready_psb(context, PSB_MSVDX_MTX_RAM_ACCESS_STATUS,
					0xffffffff, 0x00000001);
			}

			address +=  4;
			EMGD_WRITE32(fw_data[i],
				mmio + PSB_MSVDX_MTX_RAM_ACCESS_DATA_TRANSFER);
		} else {
			printk(KERN_ERR
				"[EMGD] MSVDX: Timeout waiting for MCMSTAT "
				"to be idle - data segment\n");
		}
	}

	/* Restore the RAM access control register */
	EMGD_WRITE32(acc_control, mmio + PSB_MSVDX_MTX_RAM_ACCESS_CONTROL);
	return 0;
}

int msvdx_query_plb(igd_context_t *context,
					unsigned long *status)
{
	platform_context_plb_t *platform;
	EMGD_TRACE_ENTER;

	platform = (platform_context_plb_t *)context->platform_context;
	*status = 0;

	if (priv_fw) {
		*status |= VIDEO_STATE_FW_LOADED;
	}

	if(!platform->rendec_base0 || !platform->rendec_base1) {
		*status |= VIDEO_STATE_RENDEC_FREED;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

int msvdx_pwr_plb(
	igd_context_t *context,
	unsigned long power_state)
{
	platform_context_plb_t *platform = (platform_context_plb_t *)context->platform_context;

	/* NOTE: The MSVDX need to reset after resume */
	EMGD_TRACE_ENTER;
	if(power_state != IGD_POWERSTATE_D0){
		platform->msvdx_needs_reset = 1;
	}

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

int msvdx_pre_init_plb(struct drm_device *dev)
{
    drm_emgd_priv_t *priv;
    igd_context_t *context;

	EMGD_TRACE_ENTER;

    gpDrmDevice = dev;
	priv = gpDrmDevice->dev_private;
	context = priv->context;

	context->mod_dispatch.msvdx_pwr = msvdx_pwr_plb;

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}

int msvdx_init_plb(unsigned long base0, unsigned long base1,
           void *mem_handle_fw, int reset_flag)
{
    drm_emgd_priv_t *priv;
    igd_context_t *context;
    unsigned char *mmio;
    unsigned long mmu_base_address;
    unsigned long base_addr0, base_addr1, size0, size1;
    unsigned long ctrl;
	int tmp;
	unsigned long fw_size;
	unsigned long reg_val;
	unsigned long irq_flags;
	msvdx_fw_t *fw = NULL;
	platform_context_plb_t *platform = NULL;
	int ret =0;
	int msvdx_status;
	PVRSRV_ERROR err;
	PVRSRV_PER_PROCESS_DATA *ps_data = NULL;
	IMG_UINT32 pid = 0;
	PVRSRV_KERNEL_MEM_INFO *mem_info_fw = NULL;

	if(mem_handle_fw) {

		pid = OSGetCurrentProcessIDKM();
		ps_data = PVRSRVPerProcessData(pid);
		if (!ps_data) {
			printk(KERN_ERR "MSVDX: Cannot get process data information");
			return -1;
		}

		err = PVRSRVLookupHandle(ps_data->psHandleBase, (void **)&mem_info_fw,
				(IMG_HANDLE)mem_handle_fw, PVRSRV_HANDLE_TYPE_MEM_INFO);

		if(err != PVRSRV_OK) {
			printk(KERN_ERR "MSVDX: Cannot get memory context from process data");
			return -1;
		}

	}

    priv = gpDrmDevice->dev_private;
	context = priv->context;
	mmio = context->device_context.virt_mmadr;
    platform = (platform_context_plb_t *)context->platform_context;

    // return back if firmware is already loaded
    if (init_msvdx_first_time) {
		spin_lock_init(&platform->msvdx_init_plb);
    } else if(!reset_flag){
		if (context_count == 0) {
			spin_lock_irqsave(&platform->msvdx_lock, irq_flags);
			INIT_LIST_HEAD(&platform->msvdx_queue);  // empty the list.
			spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);
		}

		return ret;
    }

    // Set the status for firmware loading
    spin_lock(&platform->msvdx_init_plb);
    platform->msvdx_status = platform->msvdx_status | 1;
    spin_unlock(&platform->msvdx_init_plb);

    // now wait for message processing to finish
    do
    {
        spin_lock(&platform->msvdx_init_plb);
        msvdx_status = platform->msvdx_status ;
        spin_unlock(&platform->msvdx_init_plb);
        OS_SLEEP(100);
    }
    while((msvdx_status & 2));

	if (!priv_fw && mem_handle_fw) {
		if(!mem_info_fw) {
			ret = -1;
			goto cleanup;
		}

		if(mem_info_fw->ui32AllocSize < FW_SIZE) {
			ret = -1;
			goto cleanup;
		}

		fw = (msvdx_fw_t *)mem_info_fw->pvLinAddrKM;

		if((fw->fw_version_size <= 0) || (fw->fw_version_size > 64 )) {
			ret = -1;
			goto cleanup;
		}

		fw_size = sizeof(unsigned long) * fw->fw_text_size;
		if((fw_size == 0) || (fw_size > MAX_FW_SIZE)) {
			ret = -1;
			goto cleanup;
		}

		fw_size = sizeof(unsigned long) * fw->fw_data_size;
		if((fw_size == 0) || (fw_size > MAX_FW_SIZE)) {
			ret = -1;
			goto cleanup;
		}

		priv_fw = kzalloc(sizeof(msvdx_fw_t), GFP_KERNEL);
		if (priv_fw == NULL) {
			printk(KERN_ERR "MSVDX: Out of memory\n");
			ret = -ENOMEM;
			goto cleanup;
		}

		priv_fw->fw_text_size = fw->fw_text_size;
		priv_fw->fw_data_size = fw->fw_data_size;
		priv_fw->fw_version_size = fw->fw_version_size;
		priv_fw->fw_data_location = fw->fw_data_location;

		fw_size = sizeof(unsigned long) * fw->fw_text_size;
		priv_fw->fw_text = kmalloc(fw_size, GFP_KERNEL);
		if (priv_fw->fw_text == NULL) {
			kfree (priv_fw);
			priv_fw = NULL;
			printk(KERN_ERR "MSVDX: Out of memory\n");
			ret = -ENOMEM;
			goto cleanup;
		}
		memcpy(priv_fw->fw_text, (void *) ((unsigned long)mem_info_fw->pvLinAddrKM) +
				((unsigned long) fw->fw_text), fw_size);

		fw_size = sizeof(unsigned long) * fw->fw_data_size;
		priv_fw->fw_data = kmalloc(fw_size, GFP_KERNEL);
		if (priv_fw->fw_data == NULL) {
			kfree (priv_fw->fw_text);
			priv_fw->fw_text = NULL;
			kfree (priv_fw);
			priv_fw = NULL;
			printk(KERN_ERR "MSVDX: Out of memory\n");
			ret = -ENOMEM;
			goto cleanup;
		}
		memcpy(priv_fw->fw_data, (void *) ((unsigned long) mem_info_fw->pvLinAddrKM) +
			((unsigned long) fw->fw_data), fw_size);

		priv_fw->fw_version = kzalloc(priv_fw->fw_version_size, GFP_KERNEL);
		if (priv_fw->fw_version == NULL) {
			kfree (priv_fw->fw_text);
			kfree (priv_fw->fw_data);
			priv_fw->fw_text = NULL;
			priv_fw->fw_data = NULL;
			kfree(priv_fw);
			priv_fw = NULL;
			printk(KERN_ERR "MSVDX: Out of memory\n");
			ret = -ENOMEM;
			goto cleanup;
		}

		strncpy(priv_fw->fw_version, (char *) (((unsigned long) mem_info_fw->pvLinAddrKM) +
			((unsigned long) fw->fw_version)), priv_fw->fw_version_size);

	} else if (!priv_fw) {
		printk(KERN_INFO "Kernel firmware is not loaded");
		if(init_msvdx_first_time) {
			printk(KERN_ERR "!priv_fw at msvdx init 1st");
		}
		ret = -1;
		goto cleanup;
	}

	if(!context_count || reset_flag) {

    //init_msvdx_first_time = 1;
    /* Reset MSVDX engine */
    EMGD_WRITE32(0x00000100, mmio + PSB_MSVDX_CONTROL);
    reg_ready_psb(context, PSB_MSVDX_CONTROL, 0x00000100, 0);

    /*
    * Make sure the clock is on.
    *
    * Clock enable bits are 0 - 6, with each bit controlling one of the
    * clocks.  For this, make sure all the clocks are enabled.
    */
    EMGD_WRITE32(PSB_CLK_ENABLE_ALL, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

    /* Set default MMU PTD to the same value used by the SGX */
    if (!msvdx_compositor_mmu_base) {
		printk(KERN_ERR "XSERVER never sent compositor MMU base!!!");
		mmu_base_address = EMGD_READ32(mmio + PSB_CR_BIF_DIR_LIST_BASE1);
		tmp = 0;
		while(!mmu_base_address){
			mmu_base_address = EMGD_READ32(mmio + PSB_CR_BIF_DIR_LIST_BASE0);
			++tmp;
			if(!tmp%100){
				printk(KERN_ERR "Cant read SGX Base0 count = %d", tmp);
			}
			if(tmp > 10000){
				printk(KERN_ERR "Giving up reading SGX Base0 from register - expect hang!");
				break;
			}
		}
		platform->psb_cr_bif_dir_list_base1 = mmu_base_address;
	} else {
			platform->psb_cr_bif_dir_list_base1 = msvdx_compositor_mmu_base;
    }

    EMGD_WRITE32(platform->psb_cr_bif_dir_list_base1,
	mmio + PSB_MSVDX_MMU_DIR_LIST_BASE0);

    /*
    * MMU Page size = 12
    * MMU best count = 7
    * MMU ADT TTE = 0
    * MMU TTE threshold = 12
    */
    EMGD_WRITE32(0xc070000c, mmio + PSB_MSVDX_MMU_CONTROL1);


    /* Flush the directory cache */
    ctrl = EMGD_READ32(mmio + PSB_MSVDX_MMU_CONTROL0) | 0x0C; /* Flush */
    EMGD_WRITE32(ctrl, mmio + PSB_MSVDX_MMU_CONTROL0);

#if 1 //disable watchdog
    reg_val = 0;
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CNT_CTRL, 0x3);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL, FE_WDT_ENABLE, 0);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL, FE_WDT_ACTION0, 1);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CLEAR_SELECT, 1);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL, FE_WDT_CLKDIV_SELECT, 7);
    printk(KERN_INFO "CTL_MSG: WDT Control value = 0x%x", reg_val);
    EMGD_WRITE32(0, mmio + MSVDX_CORE_CR_FE_MSVDX_WDT_COMPAREMATCH_OFFSET);
    EMGD_WRITE32(reg_val, mmio + MSVDX_CORE_CR_FE_MSVDX_WDT_CONTROL_OFFSET);

    reg_val = 0;
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CNT_CTRL, 0x7);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL, BE_WDT_ENABLE, 0);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL, BE_WDT_ACTION0, 1);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CLEAR_SELECT, 0xd);
    REGIO_WRITE_FIELD(reg_val, MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL, BE_WDT_CLKDIV_SELECT, 7);
    printk(KERN_INFO "CTL_MSG: WDT Control value = 0x%x", reg_val);
    EMGD_WRITE32(0, mmio + MSVDX_CORE_CR_BE_MSVDX_WDT_COMPAREMATCH_OFFSET);
    EMGD_WRITE32(reg_val, mmio + MSVDX_CORE_CR_BE_MSVDX_WDT_CONTROL_OFFSET);


#endif

    /* Enable MMU by removing all bypass bits */
    EMGD_WRITE32(0, mmio + PSB_MSVDX_MMU_CONTROL0);
    }

    /* Set up the RENDEC.
    *   The RENDEC requires two blocks of virtual address space so those
    *   must be allocated and then the RENDEC is initialized using those
    *   address ranges.
    *
    *   RENDEC control0:
    *     bit 3     1 - search MTX_to_MTX header
    *     bit 2     1 - skip next slice
    *     bit 1     1 - flush remaining bit stream
    *     bit 0     1 - initialize RENDEC
    *
    *   RENDEC control1:
    *     bit 24:   1 - enables data to be transferred through ext. memory
    *     bit 19:18 WR burst size (0 = 32 bytes, 1 = 64 bytes, 2 = 128 bytes)
    *     bit 17:16 RD burst size (0 = 32 bytes, 1 = 64 bytes, 2 = 128 bytes)
    *     bit  7: 0 start size (zero)
    */

    size0 = RENDEC_A_SIZE;
    size1 = RENDEC_B_SIZE;

    /*
    * These allocations need to be undone when shutting down.  Where
    * should they be saved?
    */
    if (init_msvdx_first_time) {
		base_addr0 = base0;
		base_addr1 = base1;

		//printk(KERN_INFO "get the base_addr=%lx, base_addr1=%lx\n", base_addr0,base_addr1);

        /* Save the offsets so it can be freed and restored later */
        platform = (platform_context_plb_t *)context->platform_context;
        platform->rendec_base0 = base_addr0;
        platform->rendec_base1 = base_addr1;

		init_msvdx_first_time = 0;
        INIT_LIST_HEAD(&platform->msvdx_queue);
		spin_lock_init(&platform->msvdx_lock);

		memset(msvdx_contexts, 0x00, sizeof(msvdx_context_t) * MSVDX_MAXIMUM_CONTEXT);
    } else {
        /* restore offsets. */
        platform = (platform_context_plb_t *)context->platform_context;
        base_addr0 = platform->rendec_base0;
        base_addr1 = platform->rendec_base1;

        /* Init link list */
        if(!context_count) {
		INIT_LIST_HEAD(&platform->msvdx_queue);
	} else {
		if(!reset_flag){
			EMGD_TRACE_EXIT;
			ret = 0;
			goto cleanup;
		}
	}
    }


	platform->msvdx_busy = 0;
    EMGD_WRITE32(base_addr0, mmio + PSB_MSVDX_RENDEC_BASE_ADDR0);
    EMGD_WRITE32(base_addr1, mmio + PSB_MSVDX_RENDEC_BASE_ADDR1);

    EMGD_WRITE32((((size1 / 4096) << 16) | (size0 / 4096)),
            mmio + PSB_MSVDX_RENDEC_BUFFER_SIZE);

    /* Rendec setup:
    *   Start size = 0
    *   Burst size R = 4 words
    *   Burst size W = 4 words
    *   External memory enabled
    *   Stream End = 0
    *   Slice mode = 0
    *   DEC disable = 0
    */
    EMGD_WRITE32(0x01050000, mmio + PSB_MSVDX_RENDEC_CONTROL1);

    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT0);
    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT1);
    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT2);
    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT3);
    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT4);
    EMGD_WRITE32(0x00101010, mmio + PSB_MSVDX_RENDEC_CONTEXT5);

    EMGD_WRITE32(0x00000001, mmio + PSB_MSVDX_RENDEC_CONTROL0);


    /* Start Firmware Load process */

    /* Reset the MTX */
    EMGD_WRITE32(0x00000001, mmio + PSB_MSVDX_MTX_SOFT_RESET);

	/* Reset the counter that looks for MSVDX getting into a bad state */
	jiffies_at_last_dequeue = 0;

    /*
    * Should this check the core revision and only do this if it is
    * a specific version or range of versions?
    *
    * Stepping prior to D0, need to set COMMS_OFFSET_FLAGS to 0
    * Stepping D0 should set MSVDX_DEVICE_NODE_FLAGS_DEFAULT_D0 (0x222)
    * Stepping D1 should set MSVDX_DEVICE_NODE_FLAGS_DEFAULT_D1 (0x220)
    */
#if 0
    /* If POULSBO_D1 or later use MSVDX_DEVICE_NODE_FLAGS_MMU_HW_INVALIDATION */
    EMGD_WRITE32(MSVDX_DEVICE_NODE_FLAGS_DEFAULT,
            mmio + PSB_MSVDX_COMMS_OFFSET_FLAGS);
    /* Else EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_OFFSET_FLAGS); */
#endif
#if 1
{
#define DISABLE_FW_WDT                          0x0008
#define ABORT_ON_ERRORS_IMMEDIATE               0x0010
#define ABORT_FAULTED_SLICE_IMMEDIATE           0x0020
#define RETURN_VDEB_DATA_IN_COMPLETION          0x0800
#define DISABLE_Auto_CLOCK_GATING               0x1000
#define DISABLE_IDLE_GPIO_SIG                   0x2000

    unsigned long msvdx_fw_flag;

    // msvdx_fw_flag = DISABLE_Auto_CLOCK_GATING | RETURN_VDEB_DATA_IN_COMPLETION | DISABLE_FW_WDT;
    msvdx_fw_flag = DISABLE_FW_WDT; /* Per ImgTec ticket 16892 */

    EMGD_WRITE32(msvdx_fw_flag, mmio + PSB_MSVDX_COMMS_OFFSET_FLAGS);

    /*
     * Following two setting can't find reg definition in spec, just copy
     * from IMG DDK 187
     */
    /* 1/200th of the clock frequency */
    EMGD_WRITE32(200 - 1,
      mmio + PSB_MSVDX_MTX_CORE_CR_MTX_SYSC_TIMERDIV_OFFSET);
    EMGD_WRITE32(0, mmio + 0x2884); /* EXT_FW_ERROR_STATE */
}
#endif
    /* Initialize communication control */
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_MSG_COUNTER);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_SIGNATURE);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_TO_HOST_RD_INDEX);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_TO_HOST_WRT_INDEX);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_TO_MTX_RD_INDEX);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);
    EMGD_WRITE32(0x00, mmio + PSB_MSVDX_COMMS_FW_STATUS);

    printk(KERN_INFO "MSVDX: Firmware version is %s\n", priv_fw->fw_version);
    if (msvdx_fw_dma_upload)
        msvdx_upload_fw_dma(0 /* Offset of firmware's .text section */);
    else
        msvdx_upload_fw();

    /* Start the firmware thread running */
    EMGD_WRITE32(PC_START_ADDRESS, mmio + PSB_MSVDX_MTX_REGISTER_READ_WRITE_DATA);
    EMGD_WRITE32(MTX_PC, mmio + PSB_MSVDX_MTX_REGISTER_READ_WRITE_REQUEST);
    reg_ready_psb(context, PSB_MSVDX_MTX_REGISTER_READ_WRITE_REQUEST,
            0x80000000, 0x80000000);

    /* Enable the MTX */
    printk(KERN_INFO "Enabling MTX 0x%x\n", EMGD_READ32(mmio + PSB_MSVDX_MTX_ENABLE));
    EMGD_WRITE32(MSVDX_MTX_ENABLE_MTX_ENABLE_MASK, mmio + PSB_MSVDX_MTX_ENABLE);
    printk(KERN_INFO "Enabled MTX 0x%x\n", EMGD_READ32(mmio + PSB_MSVDX_MTX_ENABLE));

    /*
    * Wait for signature value to be written.
    *
    * This is how the firmware thread notifies us that it is running.
    */
    if (reg_ready_psb(context, PSB_MSVDX_COMMS_SIGNATURE, 0xffffffff,
                0xA5A5A5A5)){
        /* Error initializing firmware.... */
        EMGD_DEBUG("Error, no MSVDX COMMS Signature");
	    ret = -1; /* FIXME: return an error code */
        goto cleanup;
    }
    printk(KERN_INFO "MSVDX COMMS Signature OK\n");

    /* Locate message buffers */
    platform->mtx_buf_size = EMGD_READ32(mmio+PSB_MSVDX_COMMS_TO_MTX_BUF_SIZE) & 0xFFFF;
    platform->host_buf_size = EMGD_READ32(mmio+PSB_MSVDX_COMMS_TO_HOST_BUF_SIZE) & 0xFFFF;
    platform->mtx_buf_offset = MSVDX_BASE + (EMGD_READ32(mmio+PSB_MSVDX_COMMS_TO_MTX_BUF_SIZE) >> 16) + 0x2000;
    platform->host_buf_offset = MSVDX_BASE + (EMGD_READ32(mmio+PSB_MSVDX_COMMS_TO_HOST_BUF_SIZE) >> 16) + 0x2000;

    platform->sequence = 1;
    platform->mtx_submitted = 0;

    /* Send initialization message to firmware, newer versions don't */
    if (0) {
        unsigned long init_msg[2];

        init_msg[0] = 8 | (0x80 << 8);

        /* physical address of the PD shared by SGX/MSVDX */
        init_msg[1] = EMGD_READ32(mmio + 0x40c84);

        send_to_mtx(context, init_msg);

        /* Check response from MTX firmware */
        poll_mtx_irq(context);
    }

    /* Clear the firmware buffer, this is mostly to make debugging easier */
    if (1) {
        unsigned long i;

        for (i = 0; i < platform->mtx_buf_size; i++) {
            EMGD_WRITE32(0, mmio + platform->mtx_buf_offset + (i <<2));
        }
        for (i = 0; i < platform->host_buf_size; i++) {
            EMGD_WRITE32(0, mmio + platform->host_buf_offset + (i <<2));
        }
    }


    /* Enable minimal clocks */
    EMGD_WRITE32(PSB_CLK_ENABLE_MIN, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

    /* Enable MTX interrupts to host */
    EMGD_WRITE32(1<<14, mmio + PSB_MSVDX_HOST_INTERRUPT_ENABLE);

cleanup:
    // unset fw loading flag
    spin_lock(&platform->msvdx_init_plb);
    platform->msvdx_status = platform->msvdx_status & ~1;
    spin_unlock(&platform->msvdx_init_plb);
    /* Are we done? */
    EMGD_TRACE_EXIT;
    return ret; /* Successfully initialized the MTX firmware */
}


int msvdx_uninit_plb(igd_context_t *context)
{
	EMGD_TRACE_ENTER;

	if(!context_count) {
		//msvdx_reset_plb(context);
		//msvdx_pvr_deinit();
 	}
	EMGD_TRACE_EXIT;
	return 0;
}


int msvdx_close_context(igd_context_t *context, unsigned long context_id)
{
	unsigned long irq_flags;
	struct list_head *entry = NULL, *cur = NULL;
    struct msvdx_cmd_queue *msvdx_cmd = NULL;
    platform_context_plb_t *platform;
	int i;

    EMGD_TRACE_ENTER;

    platform = (platform_context_plb_t *)context->platform_context;

	spin_lock_irqsave(&platform->msvdx_lock, irq_flags);

	list_for_each(entry, &platform->msvdx_queue) {
		msvdx_cmd = (struct msvdx_cmd_queue *) entry;
		if (msvdx_cmd->context_id == context_id) {
			cur = entry;
			entry = entry->prev;
			list_del(cur);
			msvdx_cmd->cmd = NULL;
			kfree(msvdx_cmd);
		}
	}

	for (i = 0; i <  MSVDX_MAXIMUM_CONTEXT; ++i) {
		if (msvdx_contexts[i].context_id == context_id) {
			msvdx_contexts[i].drm_file_priv = NULL;
			msvdx_contexts[i].context_id = 0;
		}
	}

	spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);

	if(context_count) {
		context_count -= 1;

		if (context_count == 0 && !list_empty(&platform->msvdx_queue)) {
            printk(KERN_ERR "MSVDX!!!  Closing final context but the list is still not empty");
			spin_lock_irqsave(&platform->msvdx_lock, irq_flags);
			INIT_LIST_HEAD(&platform->msvdx_queue);  // empty the list.
			spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);
		}
	} else {
		EMGD_TRACE_EXIT;
		return 1;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

int msvdx_create_context(igd_context_t *context, void * drm_file_priv, unsigned long ctx_id)
{
	int i = 0, ret = 0;
	EMGD_TRACE_ENTER;

	if(!drm_file_priv) {
		return -EINVAL;
	}

	for (i = 0; i <  MSVDX_MAXIMUM_CONTEXT; ++i) {
		if (msvdx_contexts[i].drm_file_priv == NULL) {
			msvdx_contexts[i].drm_file_priv = drm_file_priv;
			msvdx_contexts[i].context_id = ctx_id;
			break;
		}
	}

	if (i < MSVDX_MAXIMUM_CONTEXT) {
		context_count += 1;
	} else {
		ret = -1;
	}

	EMGD_TRACE_EXIT;
	return ret;
}

void msvdx_postclose_check(igd_context_t *context, void *drm_file_priv)
{
	int i;
	EMGD_TRACE_ENTER;

	for (i = 0; i <  MSVDX_MAXIMUM_CONTEXT; ++i) {
		if (msvdx_contexts[i].drm_file_priv == drm_file_priv) {
			printk(KERN_ERR "MSVDX!!! User mode does not call video closing ioctl.");
			msvdx_close_context(context, msvdx_contexts[i].context_id);
			msvdx_contexts[i].drm_file_priv = NULL;
			msvdx_contexts[i].context_id = 0;
		}
	}

	EMGD_TRACE_EXIT;
}

int process_video_decode_plb(igd_context_t *context, unsigned long offset,
		void* mem_handle, unsigned long *fence_id)
{
	unsigned long *mtx_buf;
    unsigned long *mtx_msgs;
    unsigned long mtx_offset;
    unsigned long mtx_msg_cnt;
    unsigned long irq_flags;
	int ret = 0;
    platform_context_plb_t *platform;
	PVRSRV_ERROR err;
	PVRSRV_PER_PROCESS_DATA *ps_data = NULL;
	IMG_UINT32 pid = 0;
	PVRSRV_KERNEL_MEM_INFO *mem_info_mtx_buf = NULL;

    EMGD_TRACE_ENTER;

	if(!mem_handle || !fence_id) {
		printk(KERN_ERR "Invalid message");
		return -EINVAL;
	}

    platform = (platform_context_plb_t *)context->platform_context;

	pid = OSGetCurrentProcessIDKM();
	ps_data = PVRSRVPerProcessData(pid);
	if (!ps_data) {
		printk(KERN_ERR "MSVDX: Cannot get process data information");
		return -1;
	}

	err = PVRSRVLookupHandle(ps_data->psHandleBase, (void **)&mem_info_mtx_buf,
			(IMG_HANDLE)mem_handle, PVRSRV_HANDLE_TYPE_MEM_INFO);

	if(err != PVRSRV_OK) {
		printk(KERN_ERR "MSVDX: Cannot get mtx buf memory context from process data");
		return -1;
	}

	if(!mem_info_mtx_buf) {
		printk(KERN_ERR "MSVDX: invalid mtx buf memory context from process data");
		return -1;
	}

	mtx_buf = (unsigned long *) mem_info_mtx_buf->pvLinAddrKM;
    mtx_offset = mtx_buf[0];
    mtx_msg_cnt = mtx_buf[1];

	if (mtx_msg_cnt > 0x20) {
		printk(KERN_ERR "Message count too big at %ld\n", mtx_msg_cnt);
		return -EINVAL;
	}

	mtx_msgs = mtx_buf + (mtx_offset / sizeof (unsigned long));
	if(!mtx_msgs) {
		printk(KERN_ERR "Invalid message");
		return -EINVAL;
	}

	if (mtx_msg_cnt > 0) {
	//if ((mtx_buf[0] != 0x8) || (mtx_buf[2] != 0x8504)) {

		spin_lock_irqsave(&platform->msvdx_lock, irq_flags);

		if (!platform->msvdx_busy) {

			platform->msvdx_busy = 1;
			spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);


			if (platform->msvdx_needs_reset) {
				msvdx_reset_plb(context);
				msvdx_init_plb(0, 0, NULL, 1);
				jiffies_at_last_dequeue = 0;
			}
			// Send message buffer to MSVDX Firmware

			populate_fence_id(context, mtx_msgs, mtx_msg_cnt);
			ret = process_mtx_messages(context, mtx_msgs, mtx_msg_cnt, platform->msvdx_fence);

			if (ret) {
				ret = -EINVAL;

			}
		} else {
			struct msvdx_cmd_queue *msvdx_cmd;

			spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);

			msvdx_cmd = kzalloc(sizeof(struct msvdx_cmd_queue), GFP_KERNEL);
			if (msvdx_cmd == NULL) {
				printk(KERN_ERR "MSVDXQUE: Out of memory\n");
				return -ENOMEM;
			}

			msvdx_cmd->context_id = populate_fence_id(context, mtx_msgs, mtx_msg_cnt);
			msvdx_cmd->cmd = mtx_msgs;
			msvdx_cmd->cmd_size = mtx_msg_cnt;
			/* If more than 1000 msec (1 second or 1000 jiffies) passes since
			 * the last time a video cmd has been decoded, MSVDX may be hung
			 * and needing to be reset.
			 */
			if ((jiffies_at_last_dequeue != 0) &&
				((jiffies - jiffies_at_last_dequeue) > 1000)) {
				printk(KERN_ERR "Video decode hardware appears to be hung; "
					"resetting\n");
				platform->msvdx_needs_reset = 1;
			}
			if (platform->msvdx_needs_reset) {
				msvdx_reset_plb(context);
				msvdx_init_plb(0, 0, NULL, 1);
				platform->msvdx_busy = 0;
				jiffies_at_last_dequeue = 0;
			}

			spin_lock_irqsave(&platform->msvdx_lock, irq_flags);
			list_add_tail(&msvdx_cmd->head, &platform->msvdx_queue);
			if (!platform->msvdx_busy) {
				platform->msvdx_busy = 1;
				msvdx_dequeue_send(context);
			}

			spin_unlock_irqrestore(&platform->msvdx_lock, irq_flags);

		}
		*fence_id = platform->msvdx_fence;
	} else {
		/* return the fence id even there is no messages to process.
		 * Used this for context id.
		 */
		*fence_id = platform->msvdx_fence;
	}

	return ret;
}

int msvdx_get_fence_id(igd_context_t *context, unsigned long *fence_id)
{
	int ret = 0;
    platform_context_plb_t *platform;

    if(!fence_id) {
    	return -EINVAL;
    }

    platform = (platform_context_plb_t *)context->platform_context;

	*fence_id = platform->mtx_completed;

	return ret;
}

int msvdx_flush_tlb(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long msvdx_mmu;
	msvdx_mmu = EMGD_READ32(mmio + PSB_MSVDX_MMU_CONTROL0);
	msvdx_mmu &= 0xFFFFFFF0;
	msvdx_mmu |= 0x0C; 	/* MMU_INVALDC + MMU_FLUSH */
	EMGD_WRITE32(msvdx_mmu, mmio + PSB_MSVDX_MMU_CONTROL0);

	msvdx_mmu = EMGD_READ32(mmio + PSB_MSVDX_MMU_CONTROL0);
	msvdx_mmu &= 0xFFFFFF00;
	EMGD_WRITE32(msvdx_mmu, mmio + PSB_MSVDX_MMU_CONTROL0);
	EMGD_READ32(mmio + PSB_MSVDX_MMU_CONTROL0);

	return 0;

}


/*
 * Resets the MSVDX engine via the soft reset control.
 *
 * This function is exported.
 */
void msvdx_reset_plb(igd_context_t *context)
{
    unsigned char *mmio = context->device_context.virt_mmadr;
    platform_context_plb_t *platform;
    EMGD_TRACE_ENTER;

    platform = (platform_context_plb_t *)context->platform_context;

    /* Reset MSVDX engine */
    EMGD_WRITE32(0x11111100, mmio + PSB_MSVDX_CONTROL);
    reg_ready_psb(context, PSB_MSVDX_CONTROL, 0x00000100, 0);

    /* Clear interrupt and clear pending interrupts */
    EMGD_WRITE32(0, mmio + PSB_MSVDX_HOST_INTERRUPT_ENABLE);
    EMGD_WRITE32(0xffffffff, mmio + PSB_MSVDX_INTERRUPT_CLEAR);

    /* Mark the engine as being reset */
    platform->msvdx_needs_reset = 0;
}

#if 0
void MSVDXSetClocksEnable(int ClockState)
{
	unsigned long reg_val = 0;

	if(ClockState == 0)
	{
		// Turn off clocks procedure

		if(LastClockState)
		{
			// Turn off all the clocks except core
			EMGD_WRITE32(0x00000001, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

			// Make sure all the clocks are off except core
			reg_ready_psb(context, PSB_MSVDX_MAN_CLK_ENABLE, 0x00000001, 0);

			// Turn off core clock
			EMGD_WRITE32(0, mmio + PSB_MSVDX_MAN_CLK_ENABLE);
		}

		LastClockState = 0;
	}
	else
	{
		// ui32ClockState
		unsigned long ClocksEn = ClockState;

		//Make sure that core clock is not accidentally turned off
		ClocksEn |= 0x00000001;

		//If all clocks were disable do the bring up procedure
		if(LastClockState == 0 )
		{
			// turn on core clock
			EMGD_WRITE32(0x00000001, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

			// Make sure it is on
			reg_ready_psb(context, PSB_MSVDX_MAN_CLK_ENABLE, 0x00000001, 0);

			// turn on the other clocks as well
			EMGD_WRITE32(ClocksEn, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

			// Make sure that they are on
			reg_ready_psb(context, PSB_MSVDX_MAN_CLK_ENABLE, ClocksEn, 0);
		}
		else
		{
			EMGD_WRITE32(ClocksEn, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

			// Make sure that they are on
			reg_ready_psb(context, PSB_MSVDX_MAN_CLK_ENABLE, ClocksEn, 0);
		}

		LastClockState = ClocksEn;
	}
}

void msvdx_reset_plb_workaround(igd_context_t *context)
{
    unsigned char *mmio = context->device_context.virt_mmadr;
    platform_context_plb_t *platform;

    EMGD_TRACE_ENTER;

    platform = (platform_context_plb_t *)context->platform_context;

    /*
    * Make sure the clock is on.
    *
    * Clock enable bits are 0 - 6, with each bit controlling one of the
    * clocks.  For this, make sure all the clocks are enabled.
    */

    EMGD_WRITE32(PSB_CLK_ENABLE_ALL, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

    /* Reset MSVDX engine */
    EMGD_WRITE32(0x11111100, mmio + PSB_MSVDX_CONTROL);
    reg_ready_psb(context, PSB_MSVDX_CONTROL, 0x00000100, 0);

    /* Clear interrupt and clear pending interrupts */
    EMGD_WRITE32(0, mmio + PSB_MSVDX_HOST_INTERRUPT_ENABLE);
    EMGD_WRITE32(0xffffffff, mmio + PSB_MSVDX_INTERRUPT_CLEAR);

    /* Mark the engine as being reset */
    platform->msvdx_needs_reset = 0;
}
#endif
/*
 * When shuting down, need to reset the MSVDX engine too.
 */
int msvdx_shutdown_plb(igd_context_t *context)
{
	platform_context_plb_t *platform;
	EMGD_TRACE_ENTER;
	platform = (platform_context_plb_t *)context->platform_context;

	/* Reset MSVDX engine */
	msvdx_reset_plb(context);
	msvdx_pvr_deinit();
	/* Free RENDEC memory allocations */
	platform->rendec_base0 = 0;
	platform->rendec_base1 = 0;
	init_msvdx_first_time = 1;
	EMGD_TRACE_EXIT;
	return 0;
}

static int reg_ready_psb(igd_context_t *context,
        unsigned long reg,
        unsigned long mask,
        unsigned long value)
{
    unsigned char *mmio = context->device_context.virt_mmadr;
    unsigned long status;
    int poll_cnt = 1000;

    while (poll_cnt) {
        status = EMGD_READ32(mmio + reg);
        if ((status & mask) == value) {
            return 0;
        }
        poll_cnt--;
        OS_SLEEP(100);
    }

    /* Timeout waiting for RAM ACCESS ready */
    EMGD_DEBUG("TIMEOUT: Got 0x%08lx while waiting for 0x%08lx", status, value);
    return 1;
}


static int poll_mtx_irq(igd_context_t *context)
{
    unsigned char *mmio = context->device_context.virt_mmadr;
    int ret;
    unsigned long mtx_int;

    EMGD_TRACE_ENTER;
    mtx_int = (1 << 14);

    ret = reg_ready_psb(context, PSB_MSVDX_INTERRUPT_STATUS, mtx_int, mtx_int);
    if (ret) {
        /* Timeout waiting on interrupt status */
        return ret;
    }

    /* Clear the interrupt */
    EMGD_WRITE32(mtx_int, mmio + PSB_MSVDX_INTERRUPT_CLEAR);

    return ret;
}


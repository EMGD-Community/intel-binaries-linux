/*
 *-----------------------------------------------------------------------------
 * Filename: sgx.h
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
 *  These are the defines specific to the SGX engine code.
 *-----------------------------------------------------------------------------
 */
#ifndef _SGX_H
#define _SGX_H


/* ---------------------------------------------------------*/
/* MACROS */
/* ---------------------------------------------------------*/
#ifndef BUG_ON
#define BUG_ON(_cond)
#endif

#ifndef BUG
#define BUG()
#endif


/* ---------------------------------------------------------*/
/* DEFINES*/
/* ---------------------------------------------------------*/
/* PSB SGX Memory buffer types */
/* FIXME! - rename 'DRM' prefix to 'SGX'
 * if we start using this */
#define DRM_BO_MEM_LOCAL                        0
#define DRM_BO_MEM_TT                           1
#define DRM_BO_MEM_VRAM                         2
#define DRM_BO_MEM_PRIV0                        3
#define DRM_BO_MEM_PRIV1                        4
#define DRM_BO_MEM_PRIV2                        5
#define DRM_BO_MEM_PRIV3                        6
#define DRM_BO_MEM_PRIV4                        7
#define DRM_BO_MEM_TYPES                        8
		/* For now. */
#define DRM_BO_LOCK_UNLOCK_BM                   (1 << 0)
#define DRM_BO_LOCK_IGNORE_NO_EVICT             (1 << 1)

/* GMM APERTURE FUNCTIONAL SEGMENTATION
 * - BEWARE!!!! we are not using this kind
 *   of pre-allocated slots in IEGD arch
 *   so this should remain commented!!??
 *
#define PSB_VDC_OFFSET                          0x00000000
#define PSB_VDC_SIZE                            0x000080000
#define PSB_SGX_SIZE                            0x8000
#define PSB_SGX_OFFSET                          0x00040000
#define PSB_MMIO_RESOURCE                       0
#define PSB_GATT_RESOURCE                       2
#define PSB_GTT_RESOURCE                        3
#define PSB_GMCH_CTRL                           0x52
#define PSB_BSM                                 0x5C
#define _PSB_GMCH_ENABLED                       0x4
#define PSB_PGETBL_CTL                          0x2020
#define _PSB_PGETBL_ENABLED                     0x00000001
#define PSB_SGX_2D_SLAVE_PORT                   0x4000
#define PSB_TT_PRIV0_LIMIT                      (256*1024*1024)
#define PSB_TT_PRIV0_PLIMIT                     (PSB_TT_PRIV0_LIMIT >> PAGE_SHIFT)
#define PSB_NUM_VALIDATE_BUFFERS                512
#define PSB_MEM_KERNEL_START                    0x10000000
#define PSB_MEM_PDS_START                       0x20000000
#define PSB_MEM_RASTGEOM_START                  0x30000000
#define PSB_MEM_MMU_START                       0x40000000
*/

#define XPSB_LOCAL_SIZE                         4096
#define XPSB_SPROG_SIZE                         1024
#define XPSB_GEOM_SIZE                          8192

#define PSB_HW_COOKIE_SIZE                      16
#define PSB_HW_FEEDBACK_SIZE                    8

#define PAGE_SHIFT                              12
#define PSB_DPM_BUFFER_PAGES                    ((10*1024*1024) >> PAGE_SHIFT)
#define PSB_DPM_TABLE_SIZE                      0x40000
#define PSB_PARAM_PAGE_SHIFT                    PAGE_SHIFT
#define PSB_PARAM_PAGE_SIZE                     PAGE_SIZE
#define PSB_MIN_NONGLOBAL_PAGES                 1024
#define PSB_PLM(_val, _base)			\
		(((_val) << (_base ## _SHIFT)) & (_base ## _MASK))

/*
 * Number of registers in a PDS attribute chunk.
 */
#define PSB_PDS_CHUNK_SIZE		                (32)
#define PSB_PDS_CHUNK_SIZE_SHIFT	            (5)

/*
 * Number of registers reserved for output registers.
 */
#define PSB_USE_NUM_OUTPUT_REGISTERS            (384)

/*
 * Total number of USE registers.
 */

#define PSB_USE_NUM_UNIFIED_REGISTERS	        (2048)
/*
 *
 */

#define PSB_USE_DEFAULT_TEMP_REG_COUNT	        (384)
#define PSB_USE_DEFAULT_TEMP_GRAN	            (4)
#define PSB_USE_DEFAULT_TEMP_REG_INIT	        (24)
#define PSB_USE_DEFAULT_ATTRIB_REG_COUNT        (2048 - 384 * 2) /* 1280 */


/* ---------------------------------------------------------*/
/* STRUCTURES */
/* ---------------------------------------------------------*/
struct drm_psb_xhw_arg {
	unsigned long op;
	int ret;
	unsigned long irq_op;
	unsigned long issue_irq;
	unsigned long cookie[PSB_HW_COOKIE_SIZE];
	union {
		struct {
			unsigned long w;
			unsigned long h;
			unsigned long size;
			unsigned long clear_p_start;
			unsigned long clear_num_pages;
		} si;
		struct {
			unsigned long fire_flags;
			unsigned long hw_context;
			unsigned long offset;
			unsigned long engine;
			unsigned long flags;
			unsigned long feedback[PSB_HW_FEEDBACK_SIZE];
		} sb;
		struct {
			unsigned long pages;
			unsigned long size;
		} bi;
		struct {
			unsigned long bca;
			unsigned long rca;
			unsigned long flags;
		} oom;
	} arg;
};
struct pclosed_vopt
{
    int fix_hw_brn_20267;
    int fix_hw_brn_20696;
    int fix_hw_brn_20852;
    int fix_hw_brn_21024;
    int fix_hw_brn_21049;
    int fix_hw_brn_21117;
    int fix_hw_brn_21158;
    int fix_hw_brn_21183;
    int fix_hw_brn_21226;
    int fix_hw_brn_21227;
    int fix_hw_brn_21301;
    int fix_hw_brn_21329;
    int fix_hw_brn_21351;
    int fix_hw_brn_21369;
    int fix_hw_brn_21387;
    int fix_hw_brn_21246;
    int fix_hw_brn_21500;
    int fix_hw_brn_21551;
    int fix_hw_brn_21592;
    int fix_hw_brn_21652;
    int fix_hw_brn_21788;
    int fix_hw_brn_21826;
    int fix_hw_brn_21878;
    int fix_hw_brn_21893;
    int fix_hw_brn_21934;
    int fix_hw_brn_21986;
    int fix_hw_brn_22048;
    int fix_hw_brn_22107;
    int fix_hw_brn_22111;
    int fix_hw_brn_22136;
    int fix_hw_brn_22162;
    int fix_hw_brn_22329;
    int fix_hw_brn_22336;
    int fix_hw_brn_22364;
    int fix_hw_brn_22117;
    int fix_hw_brn_22380;
    int fix_hw_brn_22391;
    int fix_hw_brn_22393;
    int fix_hw_brn_22462;
    int fix_hw_brn_22563;
    int fix_hw_brn_22666;
    int fix_hw_brn_23281;
};

#endif

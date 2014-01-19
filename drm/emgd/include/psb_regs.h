/*
 *-----------------------------------------------------------------------------
 * Filename: psb_regs.h
 * $Revision: 1.9 $
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
 *  This is the register definition file for the SGX and MSVDX cores.
 *  These registers were formerly part of the plb/regs.h. They are taken out
 *  because Atom E6xx shares the same core as PLB. This file is included in tnc/regs.h
 *-----------------------------------------------------------------------------
 */

#ifndef _PSB_REGS_H_
#define _PSB_REGS_H_

extern unsigned long _sgx_base;
/*-----------------------------------------------------------------------------
 * SGX and MSVDX registers
 ----------------------------------------------------------------------------*/
#define SGX_BASE _sgx_base

/* bits in PSB_EUR_CR_EVENT_STATUS */
#define PSB_DPM_3D_MEM_FREE    (1<<0)
#define PSB_OUT_OF_MEM_MT      (1<<1)
#define PSB_OUT_OF_MEM_GBL     (1<<2)
#define PSB_REACHED_MEM_THRESH (1<<3)
#define PSB_TA_TERMINATE       (1<<12)
#define PSB_TA_FINISHED        (1<<13)
#define PSB_PIXELBE_END_RENDER (1<<18)
#define PSB_DPM_TA_MEM_FREE    (1<<24)
#define PSB_DPM_OUT_OF_MEM_ZLS (1<<25)
#define PSB_TWOD_COMPLETE      (1<<27)
#define PSB_TA_DPM_FAULT       (1<<28)

/* PSB Registers */
#define PSB_SGX_2D_CMD_SLAVE_PORT               (SGX_BASE + 0x4000)

/* 0x000-0x0ff */
/* CORE CLOCK GATING register definitions */
#define PSB_CR_CLKGATECTL                       (SGX_BASE + 0x0000)
#define _PSB_C_CLKGATECTL_CLKG_ENABLED          0x0
#define _PSB_C_CLKGATECTL_CLKG_DISABLED         0x1
#define _PSB_C_CLKGATECTL_CLKG_AUTO             0x2

#define _PSB_C_CLKGATECTL_USE_CLKG_SHIFT        (20)
#define _PSB_C_CLKGATECTL_DPM_CLKG_SHIFT        (16)
#define _PSB_C_CLKGATECTL_TA_CLKG_SHIFT         (12)
#define _PSB_C_CLKGATECTL_TSP_CLKG_SHIFT        (8)
#define _PSB_C_CLKGATECTL_ISP_CLKG_SHIFT        (4)
#define _PSB_C_CLKGATECTL_2D_CLKG_SHIFT         (0)

#define _PSB_C_CLKGATECTL_USE_CLKG_MASK         (0x3 << 20)
#define _PSB_C_CLKGATECTL_DPM_CLKG_MASK         (0x3 << 16)
#define _PSB_C_CLKGATECTL_TA_CLKG_MASK          (0x3 << 12)
#define _PSB_C_CLKGATECTL_TSP_CLKG_MASK         (0x3 << 8)
#define _PSB_C_CLKGATECTL_ISP_CLKG_MASK         (0x3 << 4)
#define _PSB_C_CLKGATECTL_2D_CLKG_MASK          (0x3 << 0)

#define PSB_CR_CORE_ID                          (SGX_BASE + 0x0010)
#define _PSB_CC_ID_ID_SHIFT                     (16)
#define _PSB_CC_ID_ID_MASK                      (0xFFFF << 16)
#define _PSB_CC_ID_CONFIG_SHIFT                 (0)
#define _PSB_CC_ID_CONFIG_MASK                  (0xFFFF << 0)

#define PSB_CR_CORE_REVISION                    (SGX_BASE + 0x0014)
#define _PSB_CC_REVISION_DESIGNER_SHIFT         (24)
#define _PSB_CC_REVISION_DESIGNER_MASK          (0xFF << 24)
#define _PSB_CC_REVISION_MAJOR_SHIFT            (16)
#define _PSB_CC_REVISION_MAJOR_MASK             (0xFF << 16)
#define _PSB_CC_REVISION_MINOR_SHIFT            (8)
#define _PSB_CC_REVISION_MINOR_MASK             (0xFF << 8)
#define _PSB_CC_REVISION_MAINTENANCE_SHIFT      (0)
#define _PSB_CC_REVISION_MAINTENANCE_MASK       (0xFF << 0)

#define PSB_CR_SOFT_RESET						(SGX_BASE + 0x0080)
#define _PSB_CS_RESET_TSP_RESET					(1 << 6)
#define _PSB_CS_RESET_ISP_RESET					(1 << 5)
#define _PSB_CS_RESET_USE_RESET					(1 << 4)
#define _PSB_CS_RESET_TA_RESET					(1 << 3)
#define _PSB_CS_RESET_DPM_RESET					(1 << 2)
#define _PSB_CS_RESET_TWOD_RESET				(1 << 1)
#define _PSB_CS_RESET_BIF_RESET					(1 << 0)

/* 0x100-0x200 */

#define PSB_CR_EVENT_HOST_ENABLE2               (SGX_BASE + 0x0110)
#define PSB_CR_EVENT_HOST_CLEAR2                (SGX_BASE + 0x0114)
#define PSB_CR_EVENT_STATUS2                    (SGX_BASE + 0x0118)
#define PSB_TRIG_TA                             (1 << 7)
#define PSB_TRIG_3D                             (1 << 6)
#define PSB_TRIG_DL                             (1 << 5)
#define PSB_BIF_REQ_FAULT                       (1 << 4)
#define _PSB_CE_BIF_REQUESTER_FAULT             (1 << 4)
#define _PSB_CE_DPM_DHOST_FREE_LOAD             (1 << 3)
#define _PSB_CE_DPM_HOST_FREE_LOAD              (1 << 2)
#define _PSB_CE_DPM_3D_FREE_LOAD                (1 << 1)
#define _PSB_CE_DPM_TA_FREE_LOAD                (1 << 0)

#define PSB_EUR_CR_EVENT_STATUS                 (SGX_BASE + 0x012c)

#define PSB_CR_EVENT_STATUS                     (SGX_BASE + 0x012C)

#define PSB_EUR_CR_EVENT_HOST_ENABLE            (SGX_BASE + 0x0130)

#define PSB_CR_EVENT_HOST_CLEAR                 (SGX_BASE + 0x0134)
#define _PSB_CE_MASTER_INTERRUPT                (1 << 31)
#define _PSB_CE_TIMER                           (1 << 29)
#define _PSB_CE_TA_DPM_FAULT                    (1 << 28)
#define _PSB_CE_TWOD_COMPLETE                   (1 << 27)
#define _PSB_CE_MADD_CACHE_INVALCOMPLETE        (1 << 26)
#define _PSB_CE_DPM_OUT_OF_MEMORY_ZLS           (1 << 25)
#define _PSB_CE_DPM_TA_MEM_FREE                 (1 << 24)
#define _PSB_CE_ISP_END_TILE                    (1 << 23)
#define _PSB_CE_DPM_INITEND                     (1 << 22)
#define _PSB_CE_OTPM_LOADED                     (1 << 21)
#define _PSB_CE_OTPM_INV                        (1 << 20)
#define _PSB_CE_OTPM_FLUSHED                    (1 << 19)
#define _PSB_CE_PIXELBE_END_RENDER              (1 << 18)
#define _PSB_CE_ISP_HALT                        (1 << 17)
#define _PSB_CE_ISP_VISIBILITY_FAIL             (1 << 16)
#define _PSB_CE_BREAKPOINT                      (1 << 15)
#define _PSB_CE_SW_EVENT                        (1 << 14)
#define _PSB_CE_TA_FINISHED                     (1 << 13)
#define _PSB_CE_TA_TERMINATE                    (1 << 12)
#define _PSB_CE_TPC_CLEAR                       (1 << 11)
#define _PSB_CE_TPC_FLUSH                       (1 << 10)
#define _PSB_CE_DPM_CONTROL_CLEAR               (1 << 9)
#define _PSB_CE_DPM_CONTROL_LOAD                (1 << 8)
#define _PSB_CE_DPM_CONTROL_STORE               (1 << 7)
#define _PSB_CE_DPM_STATE_CLEAR                 (1 << 6)
#define _PSB_CE_DPM_STATE_LOAD                  (1 << 5)
#define _PSB_CE_DPM_STATE_STORE                 (1 << 4)
#define _PSB_CE_DPM_REACHED_MEM_THRESH          (1 << 3)
#define _PSB_CE_DPM_OUT_OF_MEMORY_GBL           (1 << 2)
#define _PSB_CE_DPM_OUT_OF_MEMORY_MT            (1 << 1)
#define _PSB_CE_DPM_3D_MEM_FREE                 (1 << 0)

#define PSB_CR_PDS_CACHE_STATUS                 (SGX_BASE + 0x0138)

#define PSB_CR_PDS_CACHE_HOST_ENABLE            (SGX_BASE + 0x013C)

#define PSB_CR_PDS_CACHE_HOST_CLEAR             (SGX_BASE + 0x0140)
#define _PSB_CPC_DSC1_INV3                      (1 << 8)
#define _PSB_CPC_DSC1_INV2                      (1 << 7)
#define _PSB_CPC_DSC1_INV1                      (1 << 6)
#define _PSB_CPC_DSC1_INV0                      (1 << 5)
#define _PSB_CPC_DSC0_INV3                      (1 << 4)
#define _PSB_CPC_DSC0_INV2                      (1 << 3)
#define _PSB_CPC_DSC0_INV1                      (1 << 2)
#define _PSB_CPC_DSC0_INV0                      (1 << 1)
#define _PSB_CPC_CSC_INV                        (1 << 0)

/* 0x200 - 0x300 */
#define PSB_CR_VDM_START                        (SGX_BASE + 0x0200)
#define _PSB_CV_START_PULSE                     (1 << 0)

#define PSB_CR_TE_TPCCONTROL                    (SGX_BASE + 0x0224)
#define _PSB_CT_TPCCONTROL_CLEAR                (1 << 31)
#define _PSB_CT_TPCCONTROL_FLUSH                (1 << 30)

#define PSB_CR_TE_RGNBBOX_X                     (SGX_BASE + 0x0228)
#define _PSB_CT_RGNBOX_X_MAX_SHIFT              (16)
#define _PSB_CT_RGNBOX_X_MAX_MASK               (0x1FF << 16)
#define _PSB_CT_RGNBOX_X_MIN_SHIFT              (0)
#define _PSB_CT_RGNBOX_X_MIN_MASK               (0x1FF << 0)

#define PSB_CR_TE_RGNBBOX_Y                     (SGX_BASE + 0x022C)
#define _PSB_CT_RGNBOX_Y_MAX_SHIFT              (16)
#define _PSB_CT_RGNBOX_Y_MAX_MASK               (0x1FF << 16)
#define _PSB_CT_RGNBOX_Y_MIN_SHIFT              (0)
#define _PSB_CT_RGNBOX_Y_MIN_MASK               (0x1FF << 0)

#define PSB_CR_MTE_OTPM_OP                      (SGX_BASE + 0x024C)
#define _PSB_CMO_OP_CSM_FLUSH                   (1 << 2)
#define _PSB_CMO_OP_CSM_LOAD                    (1 << 1)
#define _PSB_CMO_OP_CSM_INV                     (1 << 0)

#define PSB_CR_TE_DIAG1                         (SGX_BASE + 0x0280)
#define _PSB_CT_DIAG1_PRERGNPRITILE_COUNT_MASK  (0xFFFFFFFF)

#define PSB_CR_TE_DIAG2                         (SGX_BASE + 0x0284)
#define _PSB_CT_DIAG2_POSTRGNPRITILE_COUNT_MASK (0xFFFFFFFF)

#define PSB_CR_TE_DIAG3                         (SGX_BASE + 0x0288)
#define _PSB_CT_DIAG3_PREPTEPRITILE_COUNT_MASK  (0xFFFFFFFF)

#define PSB_CR_TE_DIAG4                         (SGX_BASE + 0x028C)
#define _PSB_CT_DIAG4_POSTPTEPRITILE_COUNT_MASK (0xFFFFFFFF)

#define PSB_CR_TE_DIAG5                         (SGX_BASE + 0x0290)
#define _PSB_CT_DIAG5_PSGCTLDWORD_COUNT_MASK    (0xFFFFFFFF)

#define PSB_CR_TE_DIAG6                         (SGX_BASE + 0x0294)
#define _PSB_CT_DIAG6_PSGRGNHDR_COUNT_MASK      (0xFFFF << 0)

#define PSB_CR_TE_DIAG7                         (SGX_BASE + 0x0298)
#define _PSB_CT_DIAG7_TPCREADREQ_COUNT_SHIFT    (16)
#define _PSB_CT_DIAG7_TPCREADREQ_COUNT_MASK     (0xFFFF << 16)
#define _PSB_CT_DIAG7_TPCREADHIT_COUNT_SHIFT    (0)
#define _PSB_CT_DIAG7_TPCREADHIT_COUNT_MASK     (0xFFFF << 0)

#define PSB_CR_TE_DIAG8                         (SGX_BASE + 0x029C)
#define _PSB_CT_DIAG8_TPCSTATEINV_COUNT_SHIFT   (16)
#define _PSB_CT_DIAG8_TPCSTATEINV_COUNT_MASK    (0xFFFF << 16)
#define _PSB_CT_DIAG8_TPCWRITEHIT_COUNT_SHIFT   (0)
#define _PSB_CT_DIAG8_TPCWRITEHIT_COUNT_MASK    (0xFFFF << 0)

/* 0x400 - 0x500 */
#define PSB_CR_ISP_START_RENDER                 (SGX_BASE + 0x0428)
#define _PSB_CI_START_RENDER_PULSE              (1 << 0)

#define PSB_CR_TSP_PARAMETER_CACHE              (SGX_BASE + 0x043C)
#define _PSB_CTP_CACHE_INVALIDATE               (1 << 0)

/* 0x600 - 0x700 */

#define PSB_CR_DPM_3D_PAGE_TABLE_BASE           (SGX_BASE + 0x0600)
#define _PSB_CD3PT_BASE_ADDR_SHIFT              (4)//128-bit aligned address

#define PSB_CR_DPM_3D_FREE_LIST                 (SGX_BASE + 0x0604)
#define _PSB_CD3F_LIST_TAIL_SHIFT               (16)
#define _PSB_CD3F_LIST_TAIL_MASK                (0xFFFF << 16)
#define _PSB_CD3F_LIST_HEAD_SHIFT               (0)
#define _PSB_CD3F_LIST_HEAD_MASK                (0xFFFF << 0)

#define PSB_CR_DPM_HOST_DALLOC_PAGE_TABLE_BASE  (SGX_BASE + 0x0608)
#define _PSB_CDHDPT_BASE_ADDR_SHIFT             (4)//128-bit aligned address

#define PSB_CR_DPM_HOST_DALLOC_FREE_LIST        (SGX_BASE + 0x060C)
#define _PSB_CDHDF_LIST_TAIL_SHIFT              (16)
#define _PSB_CDHDF_LIST_TAIL_MASK               (0xFFFF << 16)
#define _PSB_CDHDF_LIST_HEAD_SHIFT              (0)
#define _PSB_CDHDF_LIST_HEAD_MASK               (0xFFFF << 0)

#define PSB_CR_DPM_HOST_ALLOC_PAGE_TABLE_BASE   (SGX_BASE + 0x0610)
#define _PSB_CDHAPT_BASE_ADDR_SHIFT             (4)//128-bit aligned address

#define PSB_CR_DPM_HOST_ALLOC_FREE_LIST         (SGX_BASE + 0x0614)
#define _PSB_CDHAF_LIST_TAIL_SHIFT              (16)
#define _PSB_CDHAF_LIST_TAIL_MASK               (0xFFFF << 16)
#define _PSB_CDHAF_LIST_HEAD_SHIFT              (0)
#define _PSB_CDHAF_LIST_HEAD_MASK               (0xFFFF << 0)

#define PSB_CR_DPM_TA_ALLOC_PAGE_TABLE_BASE     (SGX_BASE + 0x0618)
#define _PSB_CDTAPT_BASE_ADDR_SHIFT             (4)//128-bit aligned address

#define PSB_CR_DPM_TA_ALLOC_FREE_LIST           (SGX_BASE + 0x061C)
#define _PSB_CDTAF_LIST_TAIL_SHIFT              (16)
#define _PSB_CDTAF_LIST_TAIL_MASK               (0xFFFF << 16)
#define _PSB_CDTAF_LIST_HEAD_SHIFT              (0)
#define _PSB_CDTAF_LIST_HEAD_MASK               (0xFFFF << 0)


#define PSB_CR_DPM_TA_PAGE_THRESHOLD            (SGX_BASE + 0x0620)
#define _PSB_CDTP_THRESHOLD_VALUE_SHIFT         (0)
#define _PSB_CDTP_THRESHOLD_VALUE_MASK          (0xFFFF << 0)

#define PSB_CR_DPM_ZLS_PAGE_THRESHOLD           (SGX_BASE + 0x0624)
#define _PSB_CDZP_THRESHOLD_VALUE_SHIFT         (0)
#define _PSB_CDZP_THRESHOLD_VALUE_MASK          (0xFFFF << 0)

#define PSB_CR_DPM_TA_GLOBAL_LIST               (SGX_BASE + 0x0628)
#define _PSB_CDTG_LIST_POLICY                   (1 << 16)
#define _PSB_CDTG_LIST_SIZE_SHIFT               (0)
#define _PSB_CDTG_LIST_SIZE_MASK                (0xFFFF << 0)

#define PSB_CR_DPM_STATE_CONTEXT_ID             (SGX_BASE + 0x0630)
#define _PSB_CDSC_ID_ALLOC                      (1 << 2)
#define _PSB_CDSC_ID_DALLOC                     (1 << 1)
#define _PSB_CDSC_ID_LS                         (1 << 0)

#define PSB_CR_DPM_3D_DEALLOCATE                (SGX_BASE + 0x063C)
#define _PSB_CD3_DEALLOCATE_ENABLE              (1 << 1)
#define _PSB_CD3_DEALLOCATE_GLOBAL              (1 << 0)

#define PSB_CR_DPM_ALLOC                        (SGX_BASE + 0x0640)
#define _PSB_CD_ALLOC_PAGE_OUTOFMEMORY          (1 << 17)
#define _PSB_CD_ALLOC_PAGE_VALID                (1 << 16)
#define _PSB_CD_ALLOC_PAGE_SHIFT                (0)
#define _PSB_CD_ALLOC_PAGE_MASK                 (0xFFFF << 0)

#define PSB_CR_DPM_DALLOC                       (SGX_BASE + 0x0644)
#define _PSB_CD_DALLOC_PAGE_FREE                (1 << 16)
#define _PSB_CD_DALLOC_PAGE_SHIFT               (0)
#define _PSB_CD_DALLOC_PAGE_MASK                (0xFFFF << 0)

#define PSB_CR_DPM_TA_ALLOC                     (SGX_BASE + 0x0648)
#define _PSB_CDT_ALLOC_FREE_LIST_PREVIOUS_SHIFT (16)
#define _PSB_CDT_ALLOC_FREE_LIST_PREVIOUS_MASK  (0xFFFF << 16)

#define PSB_CR_DPM_3D                           (SGX_BASE + 0x064C)
#define _PSB_CD_3D_FREE_LIST_PREVIOUS_SHIFT     (16)
#define _PSB_CD_3D_FREE_LIST_PREVIOUS_MASK      (0xFFFF << 16)

#define PSB_CR_DPM_HOST_DALLOC                  (SGX_BASE + 0x0650)
#define _PSB_CDH_DALLOC_FREE_LIST_PREVIOUS_SHIFT (16)
#define _PSB_CDH_DALLOC_FREE_LIST_PREVIOUS_MASK (0xFFFF << 16)

#define PSB_CR_DPM_HOST_ALLOC                   (SGX_BASE + 0x0654)
#define _PSB_CDH_ALLOC_FREE_LIST_PREVIOUS_SHIFT (16)
#define _PSB_CDH_ALLOC_FREE_LIST_PREVIOUS_MASK  (0xFFFF << 16)

#define PSB_CR_DPM_TASK_3D_FREE                 (SGX_BASE + 0x0680)
#define _PSB_CDT3_FREE_LOAD                     (1 << 0)

#define PSB_CR_DPM_TASK_TA_FREE                 (SGX_BASE + 0x0684)
#define _PSB_CDTT_FREE_LOAD                     (1 << 0)

#define PSB_CR_DPM_TASK_HOST_FREE               (SGX_BASE + 0x0688)
#define _PSB_CDTH_FREE_LOAD                     (1 << 0)

#define PSB_CR_DPM_TASK_DHOST_FREE              (SGX_BASE + 0x0690)
#define _PSB_CDTD_FREE_LOAD                     (1 << 0)

#define PSB_CR_DPM_TASK_STATE                   (SGX_BASE + 0x0694)
#define _PSB_CDT_STATE_LOAD                     (1 << 2)
#define _PSB_CDT_STATE_CLEAR                    (1 << 1)
#define _PSB_CDT_STATE_STORE                    (1 << 0)

#define PSB_CR_DPM_TASK_CONTROL                 (SGX_BASE + 0x0698)
#define _PSB_CDT_CONTROL_LOAD                   (1 << 2)
#define _PSB_CDT_CONTROL_CLEAR                  (1 << 1)
#define _PSB_CDT_CONTROL_STORE                  (1 << 0)

#define PSB_CR_DPM_OUTOFMEM                     (SGX_BASE + 0x069C)
#define _PSB_CD_OUTOFMEM_ABORTALL               (1 << 2)
#define _PSB_CD_OUTOFMEM_ABORT                  (1 << 1)
#define _PSB_CD_OUTOFMEM_RESTART                (1 << 0)

#define PSB_CR_DPM_FREE_CONTEXT                 (SGX_BASE + 0x06A0)
#define _PSB_CDF_CONTEXT_NOW                    (1 << 0)

#define PSB_CR_DPM_3D_TIMEOUT                   (SGX_BASE + 0x06A4)
#define _PSB_CD3_TIMEOUT_NOW                    (1 << 0)

#define PSB_CR_DPM_TA_EVM                       (SGX_BASE + 0x06A8)
#define _PSB_CDT_EVM_INIT                       (1 << 0)

/* 0x700 - 0x800 */

#define PSB_CR_DPM_HOST_DALLOC_FREE_LIST_STATUS1 (SGX_BASE + 0x0708)
#define _PSB_CDHDFL_STATUS1_TAIL_SHIFT          (16)
#define _PSB_CDHDFL_STATUS1_TAIL_MASK           (0xFFFF << 16)
#define _PSB_CDHDFL_STATUS1_HEAD_SHIFT          (0)
#define _PSB_CDHDFL_STATUS1_HEAD_MASK           (0xFFFF << 0)

#define PSB_CR_DPM_PAGE_STATUS                  (SGX_BASE + 0x0724)
#define _PSB_CDP_STATUS_TA_SHIFT                (16)
#define _PSB_CDP_STATUS_TA_MASK                 (0xFFFF << 16)
#define _PSB_CDP_STATUS_TOTAL_SHIFT             (0)
#define _PSB_CDP_STATUS_TOTAL_MASK              (0xFFFF << 0)

#define PSB_CR_DPM_GLOBAL_PAGE_STATUS           (SGX_BASE + 0x072C)
#define _PSB_CDGP_STATUS_TA_SHIFT               (16)
#define _PSB_CDGP_STATUS_TA_MASK                (0xFFFF << 16)
#define _PSB_CDGP_STATUS_TOTAL_SHIFT            (0)
#define _PSB_CDGP_STATUS_TOTAL_MASK             (0xFFFF << 0)

#define PSB_CR_CACHE_CTRL                       (SGX_BASE + 0x0804)
#define _PSB_CC_CTRL_L0P1OFF                    (1 << 30)
#define _PSB_CC_CTRL_L0P0OFF                    (1 << 29)
#define _PSB_CC_CTRL_INVALIDATE                 (1 << 28)
#define _PSB_CC_CTRL_L1P1OFF                    (1 << 26)
#define _PSB_CC_CTRL_L1P0OFF                    (1 << 25)
#define _PSB_CC_CTRL_L2OFF                      (1 << 24)
#define _PSB_CC_CTRL_PARTDM3_SHIFT              (12)
#define _PSB_CC_CTRL_PARTDM3_MASK               (0xF << 12)
#define _PSB_CC_CTRL_PARTDM2_SHIFT              (8)
#define _PSB_CC_CTRL_PARTDM2_MASK               (0xF << 8)
#define _PSB_CC_CTRL_PARTDM1_SHIFT              (4)
#define _PSB_CC_CTRL_PARTDM1_MASK               (0xF << 4)
#define _PSB_CC_CTRL_PARTDM0_SHIFT              (0)
#define _PSB_CC_CTRL_PARTDM0_MASK               (0xF << 0)

/* 0xa00 - 0xb00 */
#define PSB_CR_USE_CACHE                        (SGX_BASE + 0x0A08)
#define _PSB_CU_CACHE_INVALIDATE                (1 << 0)

/* USSE register definitions */

/* USSE/PDS - 0xA00-0xBFF */

#define PSB_CR_USE_CTRL                         (SGX_BASE + 0x0A00)
#define _PSB_CU_CTRL_REGBOUND_ZERO              (1 << 19)
#define _PSB_CU_CTRL_REGBOUND_ZERO_R1           (0 << 19)
#define _PSB_CU_CTRL_INSTLIMIT_SHIFT            (14)
#define _PSB_CU_CTRL_INSTLIMIT_MASK             (0x1F << 14)
#define _PSB_CU_CTRL_ST_TTE_SCALE_SHIFT         (12)
#define _PSB_CU_CTRL_ST_TTE_SCALE_MASK          (0x3 << 12)
#define _PSB_CU_CTRL_CACHE_TTE_SHIFT            (4)
#define _PSB_CU_CTRL_CACHE_TTE_MASK             (0xFF << 4)
#define _PSB_CU_CTRL_KILL_MODE_SHIFT            (2)
#define _PSB_CU_CTRL_KILL_MODE_MASK             (0x3 << 2)
#define _PSB_CU_CTRL_KILL_MODE_OFF              (0 << 2)
#define _PSB_CU_CTRL_KILL_MODE_FLUSH            (1 << 2)
#define _PSB_CU_CTRL_KILL_MODE_KILL             (2 << 2)
#define _PSB_CU_CTRL_KILL_DM_SHIFT              (0)
#define _PSB_CU_CTRL_KILL_DM_MASK               (0x3 << 0)


#define PSB_CR_USE_CODE_BASE(i)                 (SGX_BASE + (0x0A0C + ((i) << 2)))
#define _PSB_CUC_BASE_DM_SHIFT                  (25)
#define _PSB_CUC_BASE_DM_MASK                   (0x3 << 25)
#define _PSB_CUC_BASE_ADDR_SHIFT                (0) // 1024-bit aligned address?
#define _PSB_CUC_BASE_ADDR_ALIGNSHIFT           (7)
#define _PSB_CUC_BASE_ADDR_MASK                 (0x1FFFFFF << 0)
#define _PSB_CUC_DM_VERTEX                      (0)
#define _PSB_CUC_DM_PIXEL                       (1)
#define _PSB_CUC_DM_RESERVED                    (2)
#define _PSB_CUC_DM_EDM                         (3)

#define SGX_MAX_USSE_THRDS                      16
#define PSB_USE_OFFSET_MASK                     0x0007FFFF
#define PSB_USE_OFFSET_SIZE                     (SGX_USE_OFFSET_MASK + 1)
#define PSB_CR_USE_CODE_BASE0                   (SGX_BASE + 0x0A0C)
#define PSB_CR_USE_CODE_BASE1                   (SGX_BASE + 0x0A10)
#define PSB_CR_USE_CODE_BASE2                   (SGX_BASE + 0x0A14)
#define PSB_CR_USE_CODE_BASE3                   (SGX_BASE + 0x0A18)
#define PSB_CR_USE_CODE_BASE4                   (SGX_BASE + 0x0A1C)
#define PSB_CR_USE_CODE_BASE5                   (SGX_BASE + 0x0A20)
#define PSB_CR_USE_CODE_BASE6                   (SGX_BASE + 0x0A24)
#define PSB_CR_USE_CODE_BASE7                   (SGX_BASE + 0x0A28)
#define PSB_CR_USE_CODE_BASE8                   (SGX_BASE + 0x0A2C)
#define PSB_CR_USE_CODE_BASE9                   (SGX_BASE + 0x0A30)
#define PSB_CR_USE_CODE_BASE10                  (SGX_BASE + 0x0A34)
#define PSB_CR_USE_CODE_BASE11                  (SGX_BASE + 0x0A38)
#define PSB_CR_USE_CODE_BASE12                  (SGX_BASE + 0x0A3C)
#define PSB_CR_USE_CODE_BASE13                  (SGX_BASE + 0x0A40)
#define PSB_CR_USE_CODE_BASE14                  (SGX_BASE + 0x0A44)
#define PSB_CR_USE_CODE_BASE15                  (SGX_BASE + 0x0A48)

#define PSB_CR_EVENT_PDS_ENABLE                 (SGX_BASE + 0x0A58)
#define _PSB_CEP_ENABLE_TIMER                   (1 << 29)
#define _PSB_CEP_ENABLE_TA_DPM_FAULT            (1 << 28)
#define _PSB_CEP_ENABLE_TWOD_COMPLETE           (1 << 27)
#define _PSB_CEP_ENABLE_MADD_CACHE_INVALCOMPLETE (1 << 26)
#define _PSB_CEP_ENABLE_DPM_OUT_OF_MEMORY_ZLS   (1 << 25)
#define _PSB_CEP_ENABLE_DPM_TA_MEM_FREE         (1 << 24)
#define _PSB_CEP_ENABLE_ISP_END_PASS            (1 << 23)
#define _PSB_CEP_ENABLE_DPM_INITEND             (1 << 22)
#define _PSB_CEP_ENABLE_OTPM_LOADED             (1 << 21)
#define _PSB_CEP_ENABLE_OTPM_INV                (1 << 20)
#define _PSB_CEP_ENABLE_OTPM_FLUSHED            (1 << 19)
#define _PSB_CEP_ENABLE_PIXELBE_END_RENDER      (1 << 18)
#define _PSB_CEP_ENABLE_ISP_HALT                (1 << 17)
#define _PSB_CEP_ENABLE_ISP_VISIBILITY_FAIL     (1 << 16)
#define _PSB_CEP_ENABLE_BREAKPOINT              (1 << 15)
#define _PSB_CEP_ENABLE_SW_EVENT                (1 << 14)
#define _PSB_CEP_ENABLE_TA_FINISHED             (1 << 13)
#define _PSB_CEP_ENABLE_TA_TERMINATE            (1 << 12)
#define _PSB_CEP_ENABLE_TPC_CLEAR               (1 << 11)
#define _PSB_CEP_ENABLE_TPC_FLUSH               (1 << 10)
#define _PSB_CEP_ENABLE_DPM_CONTROL_CLEAR       (1 << 9)
#define _PSB_CEP_ENABLE_DPM_CONTROL_LOAD        (1 << 8)
#define _PSB_CEP_ENABLE_DPM_CONTROL_STORE       (1 << 7)
#define _PSB_CEP_ENABLE_DPM_STATE_CLEAR         (1 << 6)
#define _PSB_CEP_ENABLE_DPM_STATE_LOAD          (1 << 5)
#define _PSB_CEP_ENABLE_DPM_STATE_STORE         (1 << 4)
#define _PSB_CEP_ENABLE_DPM_REACHED_MEM_THRESH  (1 << 3)
#define _PSB_CEP_ENABLE_DPM_OUT_OF_MEMORY_GBL   (1 << 2)
#define _PSB_CEP_ENABLE_DPM_OUT_OF_MEMORY_MT    (1 << 1)
#define _PSB_CEP_ENABLE_DPM_3D_MEM_FREE         (1 << 0)

#define PSB_CR_DMS_CTRL                         (SGX_BASE + 0x0A74)
#define _PSB_CD_CTRL_MAX_NUM_VERTEX_PARTITIONS_SHIFT (25)
#define _PSB_CD_CTRL_MAX_NUM_VERTEX_PARTITIONS_MASK  (0x7 << 25)
#define _PSB_CD_CTRL_MAX_NUM_PIXEL_PARTITIONS_SHIFT  (22)
#define _PSB_CD_CTRL_MAX_NUM_PIXEL_PARTITIONS_MASK   (0x7 << 22)
#define _PSB_CD_CTRL_MAX_NUM_EDM_TASKS_SHIFT    (16)
#define _PSB_CD_CTRL_MAX_NUM_EDM_TASKS_MASK     (0x3F << 16)
#define _PSB_CD_CTRL_MAX_NUM_VERTEX_TASKS_SHIFT (10)
#define _PSB_CD_CTRL_MAX_NUM_VERTEX_TASKS_MASK  (0x3F << 10)
#define _PSB_CD_CTRL_MAX_NUM_PIXEL_TASKS_SHIFT  (4)
#define _PSB_CD_CTRL_MAX_NUM_PIXEL_TASKS_MASK   (0x3F << 4)
#define _PSB_CD_CTRL_DISABLE_DM_SHIFT           (0)
#define _PSB_CD_CTRL_DISABLE_DM_MASK            (0xF << 0)
#define _PSB_CD_CTRL_DISABLE_DM_VERTEX          (1 << 0)
#define _PSB_CD_CTRL_DISABLE_DM_PIXEL           (1 << 1)
#define _PSB_CD_CTRL_DISABLE_DM_EVENT           (1 << 2)
#define _PSB_CD_CTRL_DISABLE_DM_LOOPBACK        (1 << 3)

#define PSB_CR_DMS_AGE                          (SGX_BASE + 0x0A78)
#define _PSB_CD_AGE_ENABLE                      (1 << 16)
#define _PSB_CD_AGE_VALUE_SHIFT                 (0)
#define _PSB_CD_AGE_VALUE_MASK                  (0xFFFF << 0)

#define PSB_CR_USE_G0                           (SGX_BASE + 0x0A7C)
#define _PSB_CU_G0_VALUE_SHIFT                  (0)
#define _PSB_CU_G0_VALUE_MASK                   (0xFF << 0)

#define PSB_CR_USE_G1                           (SGX_BASE + 0x0A80)
#define _PSB_CU_G1_VALUE_SHIFT                  (0)
#define _PSB_CU_G1_VALUE_MASK                   (0xFF << 0)

#define PSB_CR_PDS_EXEC_BASE                    (SGX_BASE + 0x0AB8)
#define _PSB_CR_PDS_EXEC_BASE_ADDR_SHIFT        (20)/* 1MB aligned address */
#define _PSB_CR_PDS_EXEC_BASE_ADDR_ALIGNSHIFT   (20)

#define PSB_CR_USE0_DM_SLOT                     (SGX_BASE + 0x0AA4)

#define PSB_CR_USE1_DM_SLOT                     (SGX_BASE + 0x0AA8)

#define PSB_CR_USE_TMPREG                       (SGX_BASE + 0x0AAC)
#define _PSB_CU_TMPREG_INIT_SHIFT               (3)
#define _PSB_CU_TMPREG_INIT_MASK                (0x1F << 3)
#define _PSB_CU_TMPREG_SIZE_SHIFT               (0)
#define _PSB_CU_TMPREG_SIZE_MASK                (0x7 << 0)

#define PSB_CR_EVENT_TIMER                      (SGX_BASE + 0x0ACC)
#define _PSB_CE_TIMER_ENABLE                    (1 << 24)
#define _PSB_CE_TIMER_VALUE_SHIFT               (0)
#define _PSB_CE_TIMER_VALUE_MASK                (0xFFFFFF << 0)

#define PSB_CR_PDS                              (SGX_BASE + 0x0ABC)
#define _PSB_C_PDS_DOUT_TIMEOUT_DISABLE         (1 << 6)
#define _PSB_C_PDS_ATTRIBUTE_CHUNK_START_SHIFT  (0)
#define _PSB_C_PDS_ATTRIBUTE_CHUNK_START_MASK   (0x3F << 0)

#define PSB_CR_PDS_INV0                         (SGX_BASE + 0x0AD0)

#define PSB_CR_PDS_INV1                         (SGX_BASE + 0x0AD4)

#define PSB_CR_PDS_INV2                         (SGX_BASE + 0x0AD8)

#define PSB_CR_PDS_INV3                         (SGX_BASE + 0x0ADC)
#define _PSB_CP_INV_DSC                         (1 << 0)

#define PSB_CR_PDS_INV_CSC                      (SGX_BASE + 0x0AE0)
#define _PSB_CPI_CSC_KICK                       (1 << 0)

/* 0xb00 - 0xc00 */
#define PSB_CR_USE0_SERV_PIXEL                  (SGX_BASE + 0x0B08)
#define _PSB_CUS_PIXEL_EMPTY                    (1 << 16)
#define _PSB_CUS_PIXEL_COUNT_SHIFT              (0)
#define _PSB_CUS_PIXEL_COUNT_MASK               (0xFFFF << 0)

#define PSB_CR_USE0_SERV_VERTEX                 (SGX_BASE + 0x0B0C)
#define _PSB_CUS_VERTEX_EMPTY                   (1 << 16)
#define _PSB_CUS_VERTEX_COUNT_SHIFT             (0)
#define _PSB_CUS_VERTEX_COUNT_MASK              (0xFFFF << 0)

#define PSB_CR_USE0_SERV_EVENT                  (SGX_BASE + 0x0B10)
#define _PSB_CUS_EVENT_EMPTY                    (1 << 16)
#define _PSB_CUS_EVENT_COUNT_SHIFT              (0)
#define _PSB_CUS_EVENT_COUNT_MASK               (0xFFFF << 0)

#define PSB_CR_USE1_SERV_PIXEL                  (SGX_BASE + 0x0B14)

#define PSB_CR_USE1_SERV_VERTEX                 (SGX_BASE + 0x0B18)

#define PSB_CR_USE1_SERV_EVENT                  (SGX_BASE + 0x0B1C)

/* 0xc00 - 0xd00 */
/* BIF register definitions */
#define PSB_CR_BIF_CTRL                         (SGX_BASE + 0x0C00)
#define _PSB_CB_CTRL_CLEAR_FAULT                (1 << 4)
#define _PSB_CB_CTRL_INVALDC                    (1 << 3)
#define _PSB_CB_CTRL_FLUSH                      (1 << 2)
#define _PSB_MMU_ER_MASK                        0x0001FF00
#define _PSB_MMU_ER_HOST                        (1 << 16)

#define PSB_CR_BIF_INT_STAT                     (SGX_BASE + 0x0C04)

#define PSB_CR_BIF_FAULT                        (SGX_BASE + 0x0C08)
#define _PSB_CBI_STAT_PF_N_RW                   (1 << 14)
#define _PSB_CBI_STAT_FAULT_SHIFT               (0)
#define _PSB_CBI_STAT_FAULT_MASK                (0x3FFF << 0)
#define _PSB_CBI_STAT_FAULT_CACHE               (1 << 1)
#define _PSB_CBI_STAT_FAULT_TA                  (1 << 2)
#define _PSB_CBI_STAT_FAULT_VDM                 (1 << 3)
#define _PSB_CBI_STAT_FAULT_2D                  (1 << 4)
#define _PSB_CBI_STAT_FAULT_PBE                 (1 << 5)
#define _PSB_CBI_STAT_FAULT_TSP                 (1 << 6)
#define _PSB_CBI_STAT_FAULT_ISP                 (1 << 7)
#define _PSB_CBI_STAT_FAULT_USSEPDS             (1 << 8)
#define _PSB_CBI_STAT_FAULT_HOST                (1 << 9)

#define PSB_CR_BIF_BANK_SET                     (SGX_BASE + 0x0C74)
#define _PSB_CBB_SET_SELECT_SHIFT               (0)
#define _PSB_CBB_SET_SELECT_MASK                (0x3FF << 0)
#define _PSB_CBB_SET_SELECT_DPM_TA              (0 << 9)
#define _PSB_CBB_SET_SELECT_DPM_3D              (1 << 9)
#define _PSB_CBB_SET_SELECT_EDM_B0              (0 << 8)
#define _PSB_CBB_SET_SELECT_EDM_B1              (1 << 8)
#define _PSB_CBB_SET_SELECT_TA_B0               (0 << 6)
#define _PSB_CBB_SET_SELECT_TA_B0B1             (1 << 6)
#define _PSB_CBB_SET_SELECT_TA_B1B0             (2 << 6)
#define _PSB_CBB_SET_SELECT_TA_B1               (3 << 6)
#define _PSB_CBB_SET_SELECT_HOST_B0             (0 << 4)
#define _PSB_CBB_SET_SELECT_HOST_B1             (3 << 4)
#define _PSB_CBB_SET_SELECT_3D_B0               (0 << 2)
#define _PSB_CBB_SET_SELECT_3D_B0B1             (1 << 2)
#define _PSB_CBB_SET_SELECT_3D_B1B0             (2 << 2)
#define _PSB_CBB_SET_SELECT_3D_B1               (3 << 2)
#define _PSB_CBB_SET_SELECT_2D_B0               (0 << 0)
#define _PSB_CBB_SET_SELECT_2D_B1               (3 << 0)

#define PSB_CR_BIF_BANK0                        (SGX_BASE + 0x0C78)
#define PSB_CR_BIF_BANK1                        (SGX_BASE + 0x0C7C)

#define _PSB_CB_BANK_INDEX_2D_SHIFT             (16)
#define _PSB_CB_BANK_INDEX_2D_MASK              (0xF << 16)
#define _PSB_CB_BANK_INDEX_3D_SHIFT             (12)
#define _PSB_CB_BANK_INDEX_3D_MASK              (0xF << 12)
#define _PSB_CB_BANK_INDEX_HOST_SHIFT           (8)
#define _PSB_CB_BANK_INDEX_HOST_MASK            (0xF << 8)
#define _PSB_CB_BANK_INDEX_TA_SHIFT             (4)
#define _PSB_CB_BANK_INDEX_TA_MASK              (0xF << 4)
#define _PSB_CB_BANK_INDEX_EDM_SHIFT            (0)
#define _PSB_CB_BANK_INDEX_EDM_MASK             (0xF << 0)

#define PSB_CR_BIF_DIR_LIST_BASE0               (SGX_BASE + 0x0C84)
#define PSB_CR_BIF_DIR_LIST_BASE1               (SGX_BASE + 0x0C38)
#define PSB_CR_BIF_TWOD_REQ_BASE                (SGX_BASE + 0x0C88)

#define PSB_CR_BIF_TA_REQ_BASE                  (SGX_BASE + 0x0C90)
#define _PSB_CBTR_BASE_ADDR_SHIFT               (20) // 1MB aligned address

#define PSB_CR_BIF_MEM_ARB_CONFIG               (SGX_BASE + 0x0CA0)
#define _PSB_CBMA_CONFIG_TTE_THRESH_SHIFT       (12)
#define _PSB_CBMA_CONFIG_TTE_THRESH_MASK        (0xFFF << 12)
#define _PSB_CBMA_CONFIG_BEST_CNT_SHIFT         (4)
#define _PSB_CBMA_CONFIG_BEST_CNT_MASK          (0xFF << 4)
#define _PSB_CBMA_CONFIG_PAGE_SIZE_SHIFT        (0)
#define _PSB_CBMA_CONFIG_PAGE_SIZE_MASK         (0xF << 0)

#define PSB_CR_BIF_3D_REQ_BASE                  (SGX_BASE + 0x0CAC)
#define _PSB_CB3R_BASE_ADDR_ALIGNSHIFT          (20)
#define _PSB_CB3R_BASE_ADDR_SHIFT               (20) // 1MB aligned address

/* 0xe00 - 0xf00 */
#define PSB_EUR_CR_2D_SOCIF                     (SGX_BASE + 0x0e18)

#define PSB_EUR_CR_2D_BLIT_STATUS               (SGX_BASE + 0x0e04)

#endif /* _REGS_H_ */


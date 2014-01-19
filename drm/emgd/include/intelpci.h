/*
 *-----------------------------------------------------------------------------
 * Filename: intelpci.h
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
 *  Contains PCI bus transaction definitions
 *-----------------------------------------------------------------------------
 */

/* PCI */
#define PCI_VENDOR_ID_INTEL             0x8086
#ifndef PCI_VENDOR_ID_STMICRO
#define PCI_VENDOR_ID_STMICRO           0x104A
#endif

/* PLB Family Chips */
#define PCI_DEVICE_ID_BRIDGE_PLB        0x8100
#define PCI_DEVICE_ID_VGA_PLB           0x8108

/* Atom E6xx */
#define PCI_DEVICE_ID_BRIDGE_TNC        0x4114
#define PCI_DEVICE_ID_VGA_TNC           0x4108

/* Atom E6xx ULP */
#define PCI_DEVICE_ID_BRIDGE_TNC_ULP    0x4115

/* Atom E6xx Device 3 */
#define PCI_DEVICE_ID_SDVO_TNC 	        0x8182

/* Atom E6xx Device 31 (LPC) */
#define PCI_DEVICE_ID_LPC_TNC 	        0x8186

/* Atom E6xx ST Micro SDVO PCI device */
#define PCI_DEVICE_ID_SDVO_TNC_ST       0xcc13

/* Atom E6xx ST Micro GPIO SDVO PCI device */
#define PCI_DEVICE_ID_SDVO_TNC_ST_GPIO  0xcc0c

/* Support for MSRT and Pre-Release PCI ID for Atom E6xx
 * Can be removed in future */
#if 0
/* Atom E6xx A0 Stepping */
#define PCI_DEVICE_ID_BRIDGE_TNC_A0     0x4110
#define PCI_DEVICE_ID_VGA_TNC_A0        0x4100

/* Moorestown */
#define PCI_DEVICE_ID_BRIDGE_LNC        0x4110
#define PCI_DEVICE_ID_VGA_LNC           0x4102
#endif

/* Start: Southbridge specific */
#define PCI_DEVICE_ID_LPC_82801AA       0x2410
#define PCI_DEVICE_ID_LPC_82801AB       0x2420
#define PCI_DEVICE_ID_LPC_82801BA       0x2440
#define PCI_DEVICE_ID_LPC_82801BAM      0x244c
#define PCI_DEVICE_ID_LPC_82801E        0x2450
#define PCI_DEVICE_ID_LPC_82801CA       0x2480
#define PCI_DEVICE_ID_LPC_82801DB       0x24c0
#define PCI_DEVICE_ID_LPC_82801DBM      0x24cc
#define PCI_DEVICE_ID_LPC_82801EB       0x24d0
#define PCI_DEVICE_ID_LPC_82801EBM      0x24dc
#define PCI_DEVICE_ID_LPC_80001ESB      0x25a1  /* LPC on HanceRapids ICH */
#define PCI_DEVICE_ID_LPC_82801FB       0x2640  /* ICH6/ICH6R */
#define PCI_DEVICE_ID_LPC_82801FBM      0x2641  /* ICH6M/ICH6MR */
#define PCI_DEVICE_ID_LPC_82801FW       0x2642  /* ICH6W/ICH6WR */
#define PCI_DEVICE_ID_LPC_82801FWM      0x2643  /* ICH6MW/ICH6MWR */
#define PCI_DEVICE_ID_LPC_Q35DES        0x2910  /* ICH9 */
#define PCI_DEVICE_ID_LPC_Q35DHES       0x2912  /* ICH9 */
#define PCI_DEVICE_ID_LPC_Q35DOES       0x2914  /* ICH9 */
#define PCI_DEVICE_ID_LPC_Q35RES        0x2916  /* ICH9 */
#define PCI_DEVICE_ID_LPC_Q35BES        0x2918  /* ICH9 */



#define INTEL_PTE_ALLIGNMENT                0xFFFFF000


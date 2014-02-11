/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: oslinux.h
 * $Revision: 1.3 $
 *-----------------------------------------------------------------------------
 * INTEL CONFIDENTIAL
 * Copyright (2002-2008) Intel Corporation All Rights Reserved.
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intel's prior express written permission.
 * 
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 * 
 * 
 *-----------------------------------------------------------------------------
 * Description:
 *  This file contains macros and functions required to communicate with the
 *  Linux OS and EMGD driver.
 *-----------------------------------------------------------------------------
 */

#ifndef _OSLINUX_H
#define _OSLINUX_H

#include <config.h>
#include <pciaccess.h>

#if 0
typedef unsigned long pciaddr_t;
#endif

struct emgd_pci_device {
	unsigned short vendor_id, device_id;	/* Identity of the device */
	unsigned short device_class;			/* PCI device class */
	unsigned long long irq;		/* IRQ number */
	pciaddr_t base_addr[6];		/* Base addresses */
	pciaddr_t size[6];			/* Region sizes */
	pciaddr_t rom_base_addr;	/* Expansion ROM base address */
	pciaddr_t rom_size;			/* Expansion ROM size */

};


extern int g_fd;
extern void *g_mem, *g_mmio;
extern emgd_pci_device g_device;
extern unsigned char g_raw_device[256];
extern bool g_device_present;
extern size_t g_mem_size;

bool detect_gmch(OUT void **mmio, OUT void **mem);

#define DEVICE_PRESENT_CHECK() if(!g_device_present) { \
		WRITE_MSG(0, (MSG_ERROR, \
			"ERROR: Device not mapped, First map device by calling" \
			" os_initialize")); \
		ret_val = false; \
	}

#if !SOURCE_CODE_ONLY

#define DETECT_GMCH(a, b) detect_gmch(a, b)

#else

#define DETECT_GMCH(a, b) true

#endif

#endif

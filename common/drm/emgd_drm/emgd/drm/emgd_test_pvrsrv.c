/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_test_pvrsrv.c
 * $Revision: 1.23 $
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
 *  This file implements an ioctl devoted to the "ult/drmtest_surface.c" test.
 *  It tests the 3rd-Party Display Driver (and as such, it tests some of the
 *  PVR services) within kernel-space.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.oal

#include "drmP.h"
#include "drm.h"

#include "drm_emgd_private.h"
#include "emgd_drm.h"
#include "emgd_drv.h"
#include "memory.h"

#include "module_init.h"
#include "mode_dispatch.h"
#include "ovl_dispatch.h"
#include "ovl_virt.h"
#include "msvdx.h"

#include "memmap.h"
#include "sched.h"

#include "services.h"
#include "perproc.h"
#include "pvr_bridge_km.h"
#include "syscommon.h"
#include "pvr_drm.h"
/* Pull in whether HAL & 3rd-party display driver (3DD) should use interrupts */
#include "emgd_shared.h"



/* Note: The following "TEST_ERROR" macro is only for this function.  It prints
 * the message to the kernel log, and it formats and inserts the same message
 * into the drm_data->rtn_msg, so that the user-space code can print it.
 */
#define TEST_ERROR(format, arg...) \
	printk(KERN_ERR format, ##arg);\
	snprintf(drm_data->rtn_msg, 1024, format, ##arg);


#define NUM_FRONT_BUFFERS 1
#define NUM_BACK_BUFFERS 2
#define NUM_BUFFERS (NUM_FRONT_BUFFERS + NUM_BACK_BUFFERS)


#define DEVICE1 0
#define DEVICE2 1


/* Common colors: */
#define BLACK   0xff000000
#define WHITE   0xffffffff
#define RED     0xffff0000
#define GREEN   0xff00ff00
#define BLUE    0xff0000ff
#define CYAN    0xff00ffff
#define YELLOW  0xffffff00
#define MAGENTA 0xffff00ff


/*
 * Note: Am using the following global variables as a sort-of "poor man's OO"
 * class members:
 */
static emgd_drm_test_pvrsrv_t *drm_data;
static drm_emgd_priv_t *priv;

static PVRSRV_PER_PROCESS_DATA *pp_data = NULL;
static void *sgx_cookie = NULL;

static void *dev_mem_context = NULL;
static unsigned long heap_count = 0;
static IMG_BOOL mem_created, dummy, mem_destroyed;

static unsigned long dev_ids[2] = {0, 0};
static void *dev_handle[2] = {NULL, NULL};
static DISPLAY_INFO display_info;
static DISPLAY_FORMAT *pf = NULL, *pixel_formats = NULL;
static unsigned long num_dims = 0;
static DISPLAY_DIMS *dim = NULL, *dimensions = NULL;

static void *sys_buffer = NULL;
static PVRSRV_KERNEL_MEM_INFO *sysbuf_mem_info = NULL;
static void *sysbuf_map_info = NULL;

#if 0
static unsigned long fb_offset = 0;
#endif
static unsigned long mapped_fb_addr = 0;

static DISPLAY_SURF_ATTRIBUTES	dst_surf_attrib;
static DISPLAY_SURF_ATTRIBUTES	src_surf_attrib;
static void *swap_chain = NULL;
static unsigned long swap_chain_id = 0;

static void *buffers[NUM_BUFFERS] = {NULL, NULL, NULL};
static PVRSRV_KERNEL_MEM_INFO *buffers_mem_info[NUM_BUFFERS] = {NULL, NULL, NULL};
static void *buffers_map_info[NUM_BUFFERS] = {NULL, NULL, NULL};





/*!
 * Initialize PVR services (including the 3DD), and get general info:
 */
static void init_pvrsrv(void)
{
	int i;
	unsigned long num = 10;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	if (!drm_data->rtn) {
		/* This is similar to PVRSRVConnect(): */
		EMGD_DEBUG("Calling PVRSRVPerProcessDataConnect()");
		err = PVRSRVPerProcessDataConnect(/*Use a dummy PID*/ 200);
		EMGD_DEBUG("Calling PVRSRVPerProcessData()");
		pp_data = PVRSRVPerProcessData(/*Use a dummy PID*/ 200);
		EMGD_DEBUG("  TEST: pp_data = 0x%p", pp_data);
		if ((err != PVRSRV_OK) || (NULL == pp_data)) {
			TEST_ERROR("[EMGD] can not find per-process data\n");
			drm_data->rtn = -1;
		}
	}
	if (!drm_data->rtn) {
		/* Enumerate the devices and acquire the SGX device: */
		PVRSRV_DEVICE_IDENTIFIER dev_id_list[10];

		EMGD_DEBUG("Calling PVRSRVEnumerateDevicesKM()");
		err = PVRSRVEnumerateDevicesKM(&num, dev_id_list);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumerateDevicesKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			EMGD_DEBUG("  PVRSRVEnumerateDevicesKM() found %ld devices", num);
			for (i = 0 ; i < num ; i++) {
				PVRSRV_DEVICE_IDENTIFIER *id = dev_id_list + i;
				unsigned long cookie = 0;
				EMGD_DEBUG("    Device %d has type %d, class %d & index %ld", i,
					id->eDeviceType, id->eDeviceClass, id->ui32DeviceIndex);
				if (PVRSRV_DEVICE_TYPE_EXT != id->eDeviceType) {
					/* Call PVRSRVAcquireDeviceDataKM(): */
					EMGD_DEBUG("Calling PVRSRVAcquireDeviceDataKM()");
					err = PVRSRVAcquireDeviceDataKM(id->ui32DeviceIndex,
						PVRSRV_DEVICE_TYPE_UNKNOWN, (void *) &cookie);
					if (err != PVRSRV_OK) {
						TEST_ERROR("[EMGD] PVRSRVAcquireDeviceDataKM() "
							"returned %d\n", err);
						drm_data->rtn = -1;
						break;
					}
					if (PVRSRV_DEVICE_TYPE_SGX == id->eDeviceType) {
						EMGD_DEBUG("  Found cookie = 0x%lx", cookie);
						/* Save this away for later: */
						sgx_cookie = (void *) cookie;
					}
				}
			}
		}
	}
	if (!drm_data->rtn) {
		/* Enumerate the display class devices to be able to find the 3DD: */
		EMGD_DEBUG("Calling PVRSRVEnumerateDCKM()");
EMGD_DEBUG("dev_ids=0x%p", dev_ids);
		err = PVRSRVEnumerateDCKM(PVRSRV_DEVICE_CLASS_DISPLAY,
			&num, dev_ids);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumerateDCKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			/* Find the 3DD: */
			EMGD_DEBUG("  PVRSRVEnumerateDCKM() found %ld devices", num);
			for (i = 0 ; i < num ; i++) {
				EMGD_DEBUG("    device %d has ID %ld", i, dev_ids[i]);
			}
			if (0 == dev_ids[0]) {
				TEST_ERROR("[EMGD] Did not find 3rd-party display driver ID\n");
				drm_data->rtn = -1;
			}
		}
	}
	if (!drm_data->rtn) {
		/* Call PVRSRVCreateDeviceMemContextKM(): */
		PVRSRV_HEAP_INFO heap_info[PVRSRV_MAX_CLIENT_HEAPS];

		EMGD_DEBUG("Calling PVRSRVCreateDeviceMemContextKM()");
		err = PVRSRVCreateDeviceMemContextKM(sgx_cookie, pp_data,
			&dev_mem_context, &heap_count, heap_info, &mem_created, &dummy);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVCreateDeviceMemContextKM() "
				"returned %d\n", err);
			drm_data->rtn = -1;
		}
	}


	EMGD_TRACE_EXIT;

} /* init_pvrsrv() */





/*!
 * IOCTL to allow a ult test ("drmtest_surface.c") to test our 3rd-party
 * display driver (3DD) without the need for the user-mode IMG service code.
 */
static void deinit_pvrsrv(void)
{
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	if (NULL != dev_mem_context) {
		EMGD_DEBUG("Calling PVRSRVDestroyDeviceMemContextKM()");
		err = PVRSRVDestroyDeviceMemContextKM(sgx_cookie, dev_mem_context,
			&mem_destroyed);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVDestroyDeviceMemContext() "
				"returned %d\n", err);
			drm_data->rtn = -1;
		}
	}

	if (NULL != pp_data) {
		EMGD_DEBUG("Calling PVRSRVPerProcessDataDisconnect()");
		PVRSRVPerProcessDataDisconnect(/*Use a dummy PID*/ 200);
	}


	EMGD_TRACE_EXIT;

} /* deinit_pvrsrv() */





/*!
 * Initialize PVR services (including the 3DD), and get general info:
 */
static void open_device(int which_device)
{
	int i;
	unsigned long num = 10;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	if (!drm_data->rtn) {
		/* Finally, open a connection to the 3DD: */
		EMGD_DEBUG("Calling PVRSRVOpenDCDeviceKM()");
		err = PVRSRVOpenDCDeviceKM(pp_data, dev_ids[which_device], sgx_cookie,
			&dev_handle[which_device]);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVOpenDCDeviceKM() returned %d\n", err);
			drm_data->rtn = -1;
		}
		if (NULL == dev_handle[which_device]) {
			TEST_ERROR("[EMGD] PVRSRVOpenDCDeviceKM() returned NULL handle\n");
			drm_data->rtn = -1;
		}
	}
	if (!drm_data->rtn) {
		/* Get information about the display: */
		EMGD_DEBUG("Calling PVRSRVGetDCInfoKM()");
		err = PVRSRVGetDCInfoKM(dev_handle[which_device], &display_info);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVGetDCInfoKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			EMGD_DEBUG("PVRSRVGetDCInfoKM() returned the following info:");
			EMGD_DEBUG("  name = \"%s\"", display_info.szDisplayName);
			EMGD_DEBUG("  MaxSwapChains = %lu", display_info.ui32MaxSwapChains);
			EMGD_DEBUG("  MaxSwapChainBuffers = %lu",
				display_info.ui32MaxSwapChainBuffers);
			EMGD_DEBUG("  MinSwapInterval = %lu",
				display_info.ui32MinSwapInterval);
			EMGD_DEBUG("  MaxSwapInterval = %lu",
				display_info.ui32MaxSwapInterval);
		}
	}
	if (!drm_data->rtn) {
		/* Get the the number of pixel formats: */
		EMGD_DEBUG("Calling PVRSRVEnumDCFormatsKM()");
		err = PVRSRVEnumDCFormatsKM(dev_handle[which_device], &num, NULL);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCFormatsKM() returned %d\n",err);
			drm_data->rtn = -1;
		}
		if (0 == num) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCFormatsKM() says that there "
				"are %lu pixel formats\n", num);
			drm_data->rtn = -1;
		}
	}
	if (!drm_data->rtn) {
		/* Get the pixel formats: */
		EMGD_DEBUG("Allocating space for %lu pixel formats", num);
		pixel_formats = OS_ALLOC(sizeof(DISPLAY_FORMAT) * num);
		EMGD_DEBUG("Calling PVRSRVEnumDCFormatsKM()");
		err = PVRSRVEnumDCFormatsKM(dev_handle[which_device], &num,
			pixel_formats);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCFormatsKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			pf = pixel_formats;
			for (i = 0 ; i < num ; i++) {
				EMGD_DEBUG("  pixel format %d is %d", i, pf->pixelformat);
				pf++;
			}
			/* Save the first pixel format (the one being used): */
			pf = pixel_formats;
		}
	}


	if (!drm_data->rtn) {
		/* Get the number of display dimensions: */
		EMGD_DEBUG("Calling PVRSRVEnumDCDimsKM()");
		err = PVRSRVEnumDCDimsKM(dev_handle[which_device], pf, &num_dims, NULL);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCDimsKM() returned %d\n", err);
			drm_data->rtn = -1;
		}
		if (0 == num_dims) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCDimsKM() says that there "
				"are %lu dimensions\n", num_dims);
			drm_data->rtn = -1;
		}
	}
	if (!drm_data->rtn) {
		/* Get the display dimensions: */
		EMGD_DEBUG("Allocating space for %lu dimensions", num_dims);
		dimensions = OS_ALLOC(sizeof(DISPLAY_DIMS) * num_dims);
		EMGD_DEBUG("Calling PVRSRVEnumDCDimsKM()");
		err = PVRSRVEnumDCDimsKM(dev_handle[which_device], pf, &num_dims,
			dimensions);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVEnumDCDimsKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			dim = dimensions;
			for (i = 0 ; i < num_dims ; i++) {
				EMGD_DEBUG("  dimension %d is width = %lu, height = %lu, "
					"stride = %lu", i, dim->ui32Width, dim->ui32Height,
					dim->ui32ByteStride);
				dim++;
			}
			/* Save the first dimension (the one being used): */
			dim = dimensions;
		}
	}


	EMGD_TRACE_EXIT;

} /* open_device() */





/*!
 * IOCTL to allow a ult test ("drmtest_surface.c") to test our 3rd-party
 * display driver (3DD) without the need for the user-mode IMG service code.
 */
static void close_device(int which_device)
{
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	if (NULL != sysbuf_mem_info) {
		EMGD_DEBUG("Calling PVRSRVUnmapDeviceClassMemoryKM()");
		err = PVRSRVUnmapDeviceClassMemoryKM(sysbuf_mem_info);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVUnmapDeviceClassMemoryKM() "
				"returned %d\n", err);
			drm_data->rtn = -1;
		}
	}
	if (NULL != dimensions) {
		OS_FREE(dimensions);
	}
	if (NULL != pixel_formats) {
		OS_FREE(pixel_formats);
	}
	if (NULL != dev_handle[which_device]) {
		EMGD_DEBUG("Calling PVRSRVCloseDCDeviceKM()");
		err = PVRSRVCloseDCDeviceKM(dev_handle[which_device], dummy);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVCloseDCDeviceKM() returned %d\n", err);
			drm_data->rtn = -1;
		}
	}

	if (NULL != dev_mem_context) {
		EMGD_DEBUG("Calling PVRSRVDestroyDeviceMemContextKM()");
		err = PVRSRVDestroyDeviceMemContextKM(sgx_cookie, dev_mem_context,
			&mem_destroyed);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVDestroyDeviceMemContext() "
				"returned %d\n", err);
			drm_data->rtn = -1;
		}
	}

	if (NULL != pp_data) {
		EMGD_DEBUG("Calling PVRSRVPerProcessDataDisconnect()");
		PVRSRVPerProcessDataDisconnect(/*Use a dummy PID*/ 200);
	}


	EMGD_TRACE_EXIT;

} /* close_device() */




/*!
 * Maps the "system buffer" and paints it a solid color.
 */
static void map_and_paint_fb(int which_device, unsigned long color)
{
	int i;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	/* Get a handle to the primary surface (a.k.a. "system buffer"): */
	EMGD_DEBUG("Calling PVRSRVGetDCSystemBufferKM()");
	err = PVRSRVGetDCSystemBufferKM(dev_handle[which_device], &sys_buffer);
	if (err != PVRSRV_OK) {
		TEST_ERROR("[EMGD] PVRSRVGetDCSystemBufferKM() returned %d\n", err);
		drm_data->rtn = -1;
	} else if (NULL == sys_buffer) {
		TEST_ERROR("[EMGD] PVRSRVGetDCSystemBufferKM() returned a "
			"NULL handle to the system buffer\n");
		drm_data->rtn = -1;
	}

	if (!drm_data->rtn) {
		/* Map the memory of the "system buffer": */
		EMGD_DEBUG("Calling PVRSRVMapDeviceClassMemoryKM()");
		err = PVRSRVMapDeviceClassMemoryKM(pp_data, dev_mem_context,
			sys_buffer, &sysbuf_mem_info, &sysbuf_map_info);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVMapDeviceClassMemoryKM() "
				"returned %d\n", err);
			drm_data->rtn = -1;
		} else if (NULL == sysbuf_mem_info) {
			TEST_ERROR("[EMGD] PVRSRVMapDeviceClassMemoryKM() returned a "
				"NULL pointer to the system buffer's PVRSRV_KERNEL_MEM_INFO\n");
			drm_data->rtn = -1;
		} else {
			EMGD_DEBUG("  pvLinAddrKM = 0x%p", sysbuf_mem_info->pvLinAddrKM);
			EMGD_DEBUG("  sDevVAddr = 0x%lx",sysbuf_mem_info->sDevVAddr.uiAddr);
			EMGD_DEBUG("  ui32Flags = 0x%lx", sysbuf_mem_info->ui32Flags);
			EMGD_DEBUG("  ui32AllocSize = %lu = 0x%lx",
				sysbuf_mem_info->ui32AllocSize, sysbuf_mem_info->ui32AllocSize);
			EMGD_DEBUG("  sMemBlk.sDevVirtAddr = %lu = 0x%lx",
				sysbuf_mem_info->sMemBlk.sDevVirtAddr.uiAddr,
				sysbuf_mem_info->sMemBlk.sDevVirtAddr.uiAddr);
			EMGD_DEBUG("  sMemBlk.psIntSysPAddr = 0x%p",
				sysbuf_mem_info->sMemBlk.psIntSysPAddr);
			EMGD_DEBUG("  pvSysBackupBuffer = 0x%p",
				sysbuf_mem_info->pvSysBackupBuffer);
			EMGD_DEBUG("  ui32RefCount = %lu", sysbuf_mem_info->ui32RefCount);
			EMGD_DEBUG("  bPendingFree = 0x%d", sysbuf_mem_info->bPendingFree);
			EMGD_DEBUG("  psKernelSyncInfo = 0x%p",
				sysbuf_mem_info->psKernelSyncInfo);
		}
	}
	if (!drm_data->rtn) {
		/* Draw the "system buffer" the desired color: */
		mapped_fb_addr = (unsigned long) (sysbuf_mem_info->pvLinAddrKM);
		if (0 == mapped_fb_addr) {
			TEST_ERROR("[EMGD] Failed to map the framebuffer\n");
			drm_data->rtn = -1;
		} else {
			unsigned long *fb = (unsigned long *) (mapped_fb_addr);
			unsigned int pixels = sysbuf_mem_info->ui32AllocSize / 4;
			EMGD_DEBUG("  Mapped %luMB @ virtual addr= 0x%lx",
				sysbuf_mem_info->ui32AllocSize/(1024*1024), mapped_fb_addr);
			EMGD_DEBUG("     About to write to addr = 0x%p", fb);
			EMGD_DEBUG("     About to write %d (0x%x) pixels",
				pixels, pixels);
			for (i = 0 ; i < pixels ; i++) {
#if 0
				if (0 == (i % 1024)) {
					printk("row %3d: fb=0x%p\n", (i / 1024), fb);
				}
#endif
				*fb++ = color;
			}
		}
	}


	EMGD_TRACE_EXIT;

} /* map_and_paint_fb() */




/*!
 * Creates a swap chain, maps the buffers, and paints each a solid color.
 */
static void create_swap_chain(int which_device, int which_dimension,
	unsigned long fb_color,
	unsigned long bb1_color,
	unsigned long bb2_color)
{
	int i, j;
	unsigned long num = 10;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	/**************************************************************************
	 *
	 * Create a swap chain, draw green to one back buffer and red to the
	 * other; and then flip between the frame buffer and the 2 back buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		/* Create a swap chain: */
		dst_surf_attrib.pixelformat = pf->pixelformat;
		dst_surf_attrib.sDims = dimensions[which_dimension];
		src_surf_attrib.pixelformat = pf->pixelformat;
		src_surf_attrib.sDims = dimensions[which_dimension];
		EMGD_DEBUG("Calling PVRSRVCreateDCSwapChainKM()");
		err = PVRSRVCreateDCSwapChainKM(pp_data, dev_handle[which_device], 0,
			&dst_surf_attrib, &src_surf_attrib, NUM_BUFFERS, 0, &swap_chain,
			&swap_chain_id);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVCreateDCSwapChainKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else {
			EMGD_DEBUG("PVRSRVCreateDCSwapChainKM() returned swap_chain=0x%p, "
				"ID=%lu", swap_chain, swap_chain_id);
		}
	}
	if (!drm_data->rtn) {
		/* Get the DC Buffers: */
		num = NUM_BUFFERS;
		EMGD_DEBUG("Calling PVRSRVGetDCBuffersKM()");
		err = PVRSRVGetDCBuffersKM(dev_handle[which_device], swap_chain, &num,
			buffers);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVGetDCBuffersKM() returned %d\n", err);
			drm_data->rtn = -1;
		} else if (NUM_BUFFERS != num) {
			TEST_ERROR("[EMGD] PVRSRVGetDCBuffersKM() changed number of "
				"buffers to %lu\n", num);
			drm_data->rtn = -1;
		} else {
			EMGD_DEBUG("PVRSRVGetDCBuffersKM() returned buffers: 0x%p, 0x%p and "
				"0x%p", buffers[0], buffers[1], buffers[2]);
		}
	}
	if (!drm_data->rtn) {
		/* Map the memory of swap chain buffers: */
		for (j = 0 ; j < NUM_BUFFERS ; j++) {
			EMGD_DEBUG("Calling PVRSRVMapDeviceClassMemoryKM(buffers[%d])", j);
			err = PVRSRVMapDeviceClassMemoryKM(pp_data, dev_mem_context,
				buffers[j], &buffers_mem_info[j], &buffers_map_info[j]);
			if (err != PVRSRV_OK) {
				TEST_ERROR("[EMGD] PVRSRVMapDeviceClassMemoryKM() "
					"returned %d\n", err);
				drm_data->rtn = -1;
			} else if (NULL == buffers_mem_info[j]) {
				TEST_ERROR("[EMGD] PVRSRVMapDeviceClassMemoryKM() "
					"returned a NULL pointer for buffer %d's "
					"PVRSRV_KERNEL_MEM_INFO\n", j);
				drm_data->rtn = -1;
			} else {
				EMGD_DEBUG("  pvLinAddrKM = 0x%p",
					buffers_mem_info[j]->pvLinAddrKM);
				EMGD_DEBUG("  sDevVAddr = 0x%lx",
					buffers_mem_info[j]->sDevVAddr.uiAddr);
				EMGD_DEBUG("  ui32Flags = 0x%lx",
					buffers_mem_info[j]->ui32Flags);
				EMGD_DEBUG("  ui32AllocSize = %lu = 0x%lx",
					buffers_mem_info[j]->ui32AllocSize,
					buffers_mem_info[j]->ui32AllocSize);
				EMGD_DEBUG("  sMemBlk.sDevVirtAddr = %lu = 0x%lx",
					buffers_mem_info[j]->sMemBlk.sDevVirtAddr.uiAddr,
					buffers_mem_info[j]->sMemBlk.sDevVirtAddr.uiAddr);
				EMGD_DEBUG("  sMemBlk.psIntSysPAddr = 0x%p",
					buffers_mem_info[j]->sMemBlk.psIntSysPAddr);
				EMGD_DEBUG("  pvSysBackupBuffer = 0x%p",
					buffers_mem_info[j]->pvSysBackupBuffer);
				EMGD_DEBUG("  ui32RefCount = %lu",
					buffers_mem_info[j]->ui32RefCount);
				EMGD_DEBUG("  bPendingFree = 0x%d",
					buffers_mem_info[j]->bPendingFree);
				EMGD_DEBUG("  psKernelSyncInfo = 0x%p",
					buffers_mem_info[j]->psKernelSyncInfo);
			}
		}
	}

	if (!drm_data->rtn) {
		/* Draw green in buffer 1 and red in buffer 2: */
		for (j = 0 ; j < NUM_BUFFERS ; j++) {
			unsigned long color;
			unsigned long *fb =
				(unsigned long *)buffers_mem_info[j]->pvLinAddrKM;
			unsigned int pixels = buffers_mem_info[j]->ui32AllocSize / 4;

			switch(j) {
			case 0:
				color = fb_color;
				break;
			case 1:
				color = bb1_color;
				break;
			case 2:
				color = bb2_color;
				break;
			default:
				color = WHITE;
				break;
			}

			EMGD_DEBUG("   Drawing the color 0x%lx to buffer %u", color, j);
			EMGD_DEBUG("     About to write to addr = 0x%p", fb);
			EMGD_DEBUG("     About to write %d (0x%x) pixels",
				pixels, pixels);
			for (i = 0 ; i < pixels ; i++) {
#if 0
				if (0 == (i % 1024)) {
					printk("row %3d: fb=0x%p\n", (i / 1024), fb);
				}
#endif
				*fb++ = color;
			}
		}
	}


	EMGD_TRACE_EXIT;

} /* create_swap_chain() */




/*!
 * Flips each of the swap chain buffers, pausing a specified amount of time in
 * between, so that the color can be seen.
 */
static void flip_swap_chain_buffers(int which_device, int which_dimension,
	unsigned long sleep_time)
{
	int j;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	if (!drm_data->rtn) {
		/* Display each of the buffers: */
#ifndef TURN_ON_INTERRUPTS
		igd_context_t *context = priv->context;
		igd_display_h primary = priv->primary;
		int ret;
		unsigned long status;
#endif /* TURN_ON_INTERRUPTS */
		IMG_RECT clip_rect = {0, 0, dimensions[which_dimension].ui32Width,
			dimensions[which_dimension].ui32Height};

		/* Sleep for half a second, to give a chance to see the original system
		 * buffer:
		 */
		OS_SLEEP(sleep_time);

		/* Show each of the swap-chain buffers twice: */
		for (j = 1 ; j < (NUM_BUFFERS+2) ; j++) {
#ifndef TURN_ON_INTERRUPTS
			int bailout;
#endif /* TURN_ON_INTERRUPTS */
			int buf = j % NUM_BUFFERS;

			EMGD_DEBUG("Calling PVRSRVSwapToDCBufferKM(%d)", buf);
			err = PVRSRVSwapToDCBufferKM(dev_handle[which_device], buffers[buf],
#ifdef TURN_ON_INTERRUPTS
				/* Note: it is interesting to test with swap interval values of
				 * 0, 63, and 130, which cause (respectively) no queueing,
				 * immediate flipping but queueing to count-down the swap
				 * interval, and the 3DD not getting some flips from PVR
				 * services before the swap chain is destoryed:
				 */
				1, NULL, 1, &clip_rect);
#else /* TURN_ON_INTERRUPTS */
				0, NULL, 1, &clip_rect);
#endif /* TURN_ON_INTERRUPTS */
			if (err != PVRSRV_OK) {
				TEST_ERROR("[EMGD] PVRSRVSwapToDCBufferKM() returned %d\n",err);
				drm_data->rtn = -1;
			} else {
				TEST_ERROR("[EMGD] PVRSRVSwapToDCBufferKM() swapped to "
					"buffer %d\n", buf);
			}

#ifndef TURN_ON_INTERRUPTS
			/* Test that we can query whether the flip occured: */
			for (bailout = 18 ; bailout > 0 ; bailout--) {
				ret = context->dispatch.query_event(primary,
					IGD_EVENT_FLIP_PENDING, &status);
				if (0 == status) {
					break;
				} else {
					OS_SLEEP(1000);
				}
			}
			if (0 == bailout) {
				TEST_ERROR("Buffer flip didn't complete after 17msec\n");
				drm_data->rtn = -1;
			}
#endif /* TURN_ON_INTERRUPTS */

			/* Sleep for half a second, to give a chance to verify the flip
			 * occured:
			 */
			OS_SLEEP(sleep_time);
		}
	}


	EMGD_TRACE_EXIT;

} /* flip_swap_chain_buffers() */




/*!
 * Unmaps each swap chain buffer, and then destroys the swap chain.
 */
static void destroy_swap_chain(void)
{
	int j;
	PVRSRV_ERROR err;

	EMGD_TRACE_ENTER;


	for (j = 0 ; j < NUM_BUFFERS ; j++) {
		if (NULL != buffers_mem_info[j]) {
			EMGD_DEBUG("Calling PVRSRVUnmapDeviceClassMemoryKM(buffers[%d])", j);
			err = PVRSRVUnmapDeviceClassMemoryKM(buffers_mem_info[j]);
			if (err != PVRSRV_OK) {
				TEST_ERROR("[EMGD] PVRSRVUnmapDeviceClassMemoryKM(%d) "
					"returned %d\n", j, err);
				drm_data->rtn = -1;
			}
		}
	}
	if (NULL != swap_chain) {
		EMGD_DEBUG("Calling PVRSRVDestroyDCSwapChainKM()");
		err = PVRSRVDestroyDCSwapChainKM(swap_chain);
		if (err != PVRSRV_OK) {
			TEST_ERROR("[EMGD] PVRSRVDestroyDCSwapChainKM() returned %d\n",err);
			drm_data->rtn = -1;
		}
	}


	EMGD_TRACE_EXIT;

} /* destroy_swap_chain() */




/*!
 * Change the mode, via EMGD HAL functions.
 */
static void change_mode(struct drm_device *dev, int which_dim)
{
	EMGD_TRACE_ENTER;


	/**************************************************************************
	 *
	 * Do a mode change (with a new background color of white), external to the
	 * 3DD (which will invalidate the swap chain) and see that the flips are no
	 * longer visible:
	 *
	 **************************************************************************/
	if ((!drm_data->rtn) && (num_dims != 0)) {
		igd_context_t *context = priv->context;
		unsigned long emgd_pf = IGD_PF_ARGB32;
		igd_display_info_t *mode_list = NULL;
		igd_display_info_t *mode = NULL;
		igd_display_info_t *desired_mode = NULL;
		int mode_flags = IGD_QUERY_LIVE_MODES;
		unsigned long byte_stride;
		igd_framebuffer_info_t primary_fb_info;
		igd_framebuffer_info_t secondary_fb_info;
		igd_display_h primary;
		igd_display_h secondary;
		int err = 0;

		EMGD_DEBUG("Will attempt to do a mode change...");
		EMGD_DEBUG("  Target width=%lu, height=%lu",
			dimensions[which_dim].ui32Width,
			dimensions[which_dim].ui32Height);

		EMGD_DEBUG("  Calling query_mode_list()");
		err = context->dispatch.query_mode_list(context, priv->dc,
			&mode_list, mode_flags);
		if (err) {
			TEST_ERROR("The query_mode_list() function returned %d.", err);
			drm_data->rtn = -1;
		}
		if (!drm_data->rtn) {
			EMGD_DEBUG("  Comparing mode list with target width and height");
			mode = mode_list;
			while (mode && (mode->width != IGD_TIMING_TABLE_END)) {
				byte_stride =  IGD_PF_PIXEL_BYTES(emgd_pf, mode->width);
				EMGD_DEBUG("    Found a mode w/ width=%d, height=%d, refresh=%d;",
					mode->width, mode->height, mode->refresh);
				if ((mode->width == dimensions[which_dim].ui32Width) &&
					(mode->height == dimensions[which_dim].ui32Height)) {
					EMGD_DEBUG("      This mode is a match!");
					desired_mode = mode;
					break;
				}
				mode++;
			}
			if (NULL == desired_mode) {
				TEST_ERROR("  No mode matching the desired width (%lu), "
					"height (%lu) was found.", dimensions[which_dim].ui32Width,
					dimensions[which_dim].ui32Height);
				drm_data->rtn = -1;
			} else {
				/* Must set this in order to get the timings setup: */
				desired_mode->flags |= IGD_DISPLAY_ENABLE;
			}
		}
		if (!drm_data->rtn) {
			/* Make the mode change by calling alter_displays(): */
			primary_fb_info.width = desired_mode->width;
			primary_fb_info.height = desired_mode->height;
			primary_fb_info.pixel_format = emgd_pf;
			primary_fb_info.flags = 0;
			primary_fb_info.allocated = 0;
			memcpy(&secondary_fb_info, &primary_fb_info,
				sizeof(igd_framebuffer_info_t));

			EMGD_DEBUG("Calling alter_displays()");
			err = context->dispatch.alter_displays(context,
				&primary, desired_mode, &primary_fb_info,
				&secondary, desired_mode, &secondary_fb_info, priv->dc, 0);
			if (err) {
				TEST_ERROR("The alter_displays() function returned %d.", err);
				drm_data->rtn = -1;
			}
		}
		if (!drm_data->rtn) {
			/* Update the private copy, like emgd_alter_displays() would do: */
			priv->primary = primary;
			priv->secondary = secondary;
			priv->primary_port_number = (priv->dc & 0xf0) >> 4;
			priv->secondary_port_number = (priv->dc & 0xf00000) >> 20;

			/* Re-initialize the display values: */
			err = priv->reinit_3dd(dev);
			if (err != 0) {
				TEST_ERROR("The reinit_3dd() function returned %d.", err);
				drm_data->rtn = -1;
			} else {
				EMGD_DEBUG("The MODE CHANGE was SUCCESSFUL");
			}
		}
	}



	EMGD_TRACE_EXIT;

} /* change_mode() */









/*!
 * Test a single/clone DC with lots of buffer flipping and mode changes.
 */
static void emgd_test_pvrsrv_single_dc(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	/**************************************************************************
	 *
	 * Initialize PVR services (including the 3DD):
	 *
	 **************************************************************************/
	init_pvrsrv();
	if (!drm_data->rtn) {
		open_device(DEVICE1);
	}


	/**************************************************************************
	 *
	 * Map the frame buffer and paint it blue:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		map_and_paint_fb(DEVICE1, BLUE);
	}


	/**************************************************************************
	 *
	 * Create a swap chain, draw red to one back buffer and green to the
	 * other; and then flip between the frame buffer and the 2 back buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		create_swap_chain(DEVICE1, 0, BLUE, RED, GREEN);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 0, 500*1000);
	}


	/**************************************************************************
	 *
	 * Do a mode change, external to the 3DD (which will invalidate the swap
	 * chain) and see that the flips are no longer visible:
	 *
	 **************************************************************************/
	if ((!drm_data->rtn) && (num_dims != 0)) {
		change_mode(dev, 1);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 0, 500*1000);
	}


	/**************************************************************************
	 *
	 * Destroy the old swap chain, and create a new one that has the new
	 * dimensions; then draw cyan to the frame buffer, yellow to one back
	 * buffer and magenta to the other; and then flip between the frame buffer
	 * and the 2 back buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}
	if (!drm_data->rtn) {
		create_swap_chain(DEVICE1, 1, CYAN, YELLOW, MAGENTA);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 1, 500*1000);
	}


	/**************************************************************************
	 *
	 * Destroy the old swap chain, and create a new one that has different
	 * dimensions (the original dimensions), which will cause the 3DD to do a
	 * mode change; then draw blue to the frame buffer, red to one back
	 * buffer and green to the other; and then flip between the frame buffer
	 * and the 2 back buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}
	if (!drm_data->rtn) {
		create_swap_chain(DEVICE1, 0, BLUE, RED, GREEN);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 0, 500*1000);
	}


	/**************************************************************************
	 *
	 * Clean up to avoid an Oops and/or memory leak:
	 *
	 **************************************************************************/
	destroy_swap_chain();

	close_device(DEVICE1);
	deinit_pvrsrv();


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_single_dc() */




/*!
 * Test a DIH/extended DC by opening/mapping the first device and drawing blue
 * on it, and then by creating a swap chain, and flipping between all buffers.
 */
static void emgd_test_pvrsrv_dih_open_dev1(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	/**************************************************************************
	 *
	 * Initialize PVR services (including the 3DD):
	 *
	 **************************************************************************/
	init_pvrsrv();
	if (!drm_data->rtn) {
		open_device(DEVICE1);
	}


	/**************************************************************************
	 *
	 * Map the frame buffer and paint it blue:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		map_and_paint_fb(DEVICE1, BLUE);
	}
	OS_SLEEP(3*1000*1000);


	/**************************************************************************
	 *
	 * Create a swap chain and flip between all buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		create_swap_chain(DEVICE1, 0, BLUE, RED, GREEN);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 0, 1000*1000);
	}
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_open_dev1() */




/*!
 * Continue testing a DIH/extended DC by having EMGD do a mode change on the
 * first device.
 */
static void emgd_test_pvrsrv_dih_change1_dev1(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	if ((!drm_data->rtn) && (num_dims != 0)) {
		change_mode(dev, 1);
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_change1_dev1() */




/*!
 * Continue testing a DIH/extended DC by having the 3DD (via PVRSRV) do a mode
 * change, via the creation of a swap chain, on the first device, and then flip
 * between all buffers.
 */
static void emgd_test_pvrsrv_dih_change2_dev1(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	if (!drm_data->rtn) {
		create_swap_chain(DEVICE1, 0, BLUE, RED, GREEN);
	}
	OS_SLEEP(3*1000*1000);
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE1, 0, 500*1000);
	}
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_change2_dev1() */




/*!
 * Close the first device and close PVR services to avoid an Oops and/or
 * memory leak:
 */
static void emgd_test_pvrsrv_dih_close_dev1(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	close_device(DEVICE1);
	deinit_pvrsrv();


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_close_dev1() */




/*!
 * Continue testing a DIH/extended DC by opening/mapping the second device and
 * drawing red on it.
 */
static void emgd_test_pvrsrv_dih_open_dev2(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	/**************************************************************************
	 *
	 * Initialize PVR services (including the 3DD):
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		init_pvrsrv();
	}
	if (!drm_data->rtn) {
		open_device(DEVICE2);
	}


	/**************************************************************************
	 *
	 * Map the frame buffer and paint it cyan:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		map_and_paint_fb(DEVICE2, CYAN);
	}
	OS_SLEEP(3*1000*1000);


	/**************************************************************************
	 *
	 * Create a swap chain and flip between all buffers:
	 *
	 **************************************************************************/
	if (!drm_data->rtn) {
		create_swap_chain(DEVICE2, 0, CYAN, YELLOW, MAGENTA);
	}
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE2, 0, 1000*1000);
	}
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_open_dev2() */




/*!
 * Continue testing a DIH/extended DC by having EMGD do a mode change on the
 * second device.
 */
static void emgd_test_pvrsrv_dih_change1_dev2(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	if ((!drm_data->rtn) && (num_dims != 0)) {
		change_mode(dev, 1);
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_change1_dev2() */




/*!
 * Continue testing a DIH/extended DC by having the 3DD (via PVRSRV) do a mode
 * change, via the creation of a swap chain, on the second device, and then flip
 * between all buffers.
 */
static void emgd_test_pvrsrv_dih_change2_dev2(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	if (!drm_data->rtn) {
		create_swap_chain(DEVICE2, 0, CYAN, YELLOW, MAGENTA);
	}
	OS_SLEEP(3*1000*1000);
	if (!drm_data->rtn) {
		flip_swap_chain_buffers(DEVICE2, 0, 500*1000);
	}
	if (!drm_data->rtn) {
		destroy_swap_chain();
	}


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_change2_dev2() */




/*!
 * Close the second device and close PVR services to avoid an Oops and/or
 * memory leak:
 */
static void emgd_test_pvrsrv_dih_close_dev2(struct drm_device *dev)
{
	EMGD_TRACE_ENTER;


	close_device(DEVICE2);
	deinit_pvrsrv();


	EMGD_TRACE_EXIT;

} /* emgd_test_pvrsrv_dih_close_dev2() */










/*!
 * IOCTL to allow a ult test ("drmtest_surface.c") to test our 3rd-party
 * display driver (3DD) without the need for the user-mode IMG service code.
 */
int emgd_test_pvrsrv(struct drm_device *dev, void *arg,
	struct drm_file *file_priv)
{
	EMGD_TRACE_ENTER;


	drm_data = arg;
	priv = dev->dev_private;
	drm_data->rtn = 0;


	/**************************************************************************
	 *
	 * Depending on the opcode, call the appropriate function that does the
	 * test:
	 *
	 **************************************************************************/
	switch (drm_data->opcode) {
	case SINGLE_DC_SWAPING:
		emgd_test_pvrsrv_single_dc(dev);
		break;
	case DIH_DC_DRAW_OPEN_DEV1:
		emgd_test_pvrsrv_dih_open_dev1(dev);
		break;
	case DIH_DC_MODE_CHANGE1_DEV1:
		emgd_test_pvrsrv_dih_change1_dev1(dev);
		break;
	case DIH_DC_MODE_CHANGE2_DEV1:
		emgd_test_pvrsrv_dih_change2_dev1(dev);
		break;
	case DIH_DC_CLOSE_DEV1:
		emgd_test_pvrsrv_dih_close_dev1(dev);
		break;
	case DIH_DC_DRAW_OPEN_DEV2:
		emgd_test_pvrsrv_dih_open_dev2(dev);
		break;
	case DIH_DC_MODE_CHANGE1_DEV2:
		emgd_test_pvrsrv_dih_change1_dev2(dev);
		break;
	case DIH_DC_MODE_CHANGE2_DEV2:
		emgd_test_pvrsrv_dih_change2_dev2(dev);
		break;
	case DIH_DC_CLOSE_DEV2:
		emgd_test_pvrsrv_dih_close_dev2(dev);
		break;
	}


	EMGD_DEBUG("drm_data->rtn = %d", drm_data->rtn);
	EMGD_DEBUG("Returning 0");
	EMGD_TRACE_EXIT;
	return 0;
} /* emgd_test_pvrsrv() */

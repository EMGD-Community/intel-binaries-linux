/*
 *-----------------------------------------------------------------------------
 * Filename: msvdx_pvr.c
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

#include "msvdx.h"
#include "msvdx_pvr.h"
#include "sgxinfokm.h"
#include "services_headers.h"
//#include "sgxinfokm.h"
#include "sgxapi_km.h"
#include "services.h"
#include "perproc.h"
#include "pvr_bridge_km.h"
#include "syscommon.h"
#include "memory.h"
#include "memmap.h"
#include <drm_emgd_private.h>
#include "emgd_drm.h"
#include <linux/kthread.h>
#include <plb/regs.h>
#include <plb/context.h>

extern IMG_BOOL msvdx_mtx_isr(IMG_VOID *pvData);
#if 0
extern int emgd_test_pvrsrv(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);

static PVRSRV_PER_PROCESS_DATA *psPerProc = NULL;
static unsigned long dev_ids[2] = {0, 0};
static void *dev_mem_context = NULL;
static IMG_BOOL mem_created, dummy;
static PVRSRV_DEVICE_IDENTIFIER dev_id_list[10];
static PVRSRV_HEAP_INFO heap_info[PVRSRV_MAX_CLIENT_HEAPS];

int alloc_ramdec_region(unsigned long *base_addr0, unsigned long *base_addr1,
				unsigned long size0, unsigned long size1)
{
	unsigned long pid = OSGetCurrentProcessIDKM();
    unsigned long num = 10;
	unsigned long heap_count = 0;
    void *sgx_cookie = NULL;
    unsigned long heapIndex = 0, generalHeapIndex = 0;
	PVRSRV_ERROR err;
	PVRSRV_KERNEL_MEM_INFO *rendec0MemInfo;
	PVRSRV_KERNEL_MEM_INFO *rendec1MemInfo;
	int i;

	printk(KERN_INFO "Calling PVRSRVPerProcessData()\n");
	if (PVRSRVPerProcessDataConnect(200) != PVRSRV_OK) {
		printk(KERN_ERR "msvdx_init: connect to PVR failed\n");
	}

	psPerProc = PVRSRVPerProcessData(200);
	if (psPerProc == IMG_NULL)
	{
		printk(KERN_ERR "msvdx_init:  Couldn't find per process data for pid=%lx\n", pid);
	}

	printk(KERN_INFO "  TEST: pp_data = 0x%p\n", psPerProc);


	PVRSRVEnumerateDevicesKM(&num, dev_id_list);
	printk(KERN_INFO "Calling PVRSRVEnumerateDevicesKM()\n");
	if (PVRSRVEnumerateDevicesKM(&num, dev_id_list) != PVRSRV_OK) {
		printk(KERN_ERR "msvdx_init: PVRSRVEnumerateDevice failed\n");
	} else {
		printk(KERN_INFO "  PVRSRVEnumerateDevicesKM() found %ld devices\n", num);
		for (i = 0 ; i < num ; i++) {
			PVRSRV_DEVICE_IDENTIFIER *id = dev_id_list + i;
			unsigned long cookie = 0;
			printk(KERN_INFO "    Device %d has type %d, class %d & index %ld\n", i,
					id->eDeviceType, id->eDeviceClass, id->ui32DeviceIndex);
			if (PVRSRV_DEVICE_TYPE_EXT != id->eDeviceType) {
				// Call PVRSRVAcquireDeviceDataKM():
				printk(KERN_INFO "Calling PVRSRVAcquireDeviceDataKM()\n");
				err = PVRSRVAcquireDeviceDataKM(id->ui32DeviceIndex,
						PVRSRV_DEVICE_TYPE_UNKNOWN, (void *) &cookie);
				if (err != PVRSRV_OK) {
					printk(KERN_ERR "[EMGD] PVRSRVAcquireDeviceDataKM() "
							"returned %d\n", err);
					break;
				}
				if (PVRSRV_DEVICE_TYPE_SGX == id->eDeviceType) {
					printk(KERN_INFO "  Found cookie = 0x%lx\n", cookie);
					// Save this away for later:
					sgx_cookie = (void *) cookie;
				}
			}
		}
	}

	// Enumerate the display class devices to be able to find the 3DD:
	printk(KERN_INFO "Calling PVRSRVEnumerateDCKM()\n");
	err = PVRSRVEnumerateDCKM(PVRSRV_DEVICE_CLASS_DISPLAY,
			&num, dev_ids);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] PVRSRVEnumerateDCKM() returned %d\n", err);
	} else {
		// Find the 3DD:
		printk(KERN_INFO "  PVRSRVEnumerateDCKM() found %ld devices\n", num);
		for (i = 0 ; i < num ; i++) {
			printk(KERN_INFO "    device %d has ID %ld\n", i, dev_ids[i]);
		}
		if (0 == dev_ids[0]) {
			printk(KERN_ERR "[EMGD] Did not find 3rd-party display driver ID\n");
		}
	}

	// Call PVRSRVCreateDeviceMemContextKM():

	printk(KERN_INFO "Calling PVRSRVCreateDeviceMemContextKM()\n");
	err = PVRSRVCreateDeviceMemContextKM(sgx_cookie, psPerProc,
			&dev_mem_context, &heap_count, heap_info, &mem_created, &dummy);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] PVRSRVCreateDeviceMemContextKM() "
				"returned %d\n", err);
	}


	for (heapIndex=0; heapIndex<heap_count; heapIndex++) {
		if (HEAP_IDX(heap_info[heapIndex].ui32HeapID) == SGX_GENERAL_HEAP_ID)
		{
			generalHeapIndex = heapIndex;
			break;
		}
	}

	if (PVRSRVAllocDeviceMemKM(sgx_cookie, psPerProc, heap_info[generalHeapIndex].hDevMemHeap,
                        PVRSRV_MEM_READ | PVRSRV_MEM_WRITE,
                        size0, 0, &rendec0MemInfo, "") != PVRSRV_OK) {
		printk(KERN_ERR "msvdx: PVRSRVAllocDeviceMemKM failed\n");
	}

	if (PVRSRVAllocDeviceMemKM(sgx_cookie, psPerProc, heap_info[generalHeapIndex].hDevMemHeap,
                        PVRSRV_MEM_READ | PVRSRV_MEM_WRITE,
                        size1, 0, &rendec1MemInfo, "") != PVRSRV_OK) {
		printk(KERN_ERR "msvdx: PVRSRVAllocDeviceMemKM failed\n");
	}

	*base_addr0 = rendec0MemInfo->sDevVAddr.uiAddr;
	*base_addr1 = rendec1MemInfo->sDevVAddr.uiAddr;

	//printk(KERN_INFO "pvr size0=%lx, size1=%lx, heap=%ld\n", size0, size1, generalHeapIndex);

//	return PVRSRV_OK;
	return 0;

}
#endif

/*
 * Dummy kernel thread.  Needed for a persistent per process data for
 * PVR services.
 */
static int msvdx_pvr_kthread(void *data)
{
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return 0;
}

int msvdx_preinit_mmu(unsigned long hmemcxt)
{
	PVRSRV_ERROR err;
	PVRSRV_PER_PROCESS_DATA *ps_data = NULL;
	IMG_HANDLE hmem = (IMG_HANDLE) hmemcxt;
	IMG_HANDLE hmemkm;
	IMG_DEV_PHYADDR addr;
	IMG_UINT32 pid = OSGetCurrentProcessIDKM();
	int ret;
	drm_emgd_priv_t *priv = gpDrmDevice->dev_private;
	igd_context_t *context = priv->context;
	platform_context_plb_t *platform =
		(platform_context_plb_t *) context->platform_context;

    ps_data = PVRSRVPerProcessData(pid);
	if (!ps_data) {
		printk(KERN_ERR "MSVDX: Cannot get process data information");

		return -1;
	}


	err = PVRSRVLookupHandle(ps_data->psHandleBase, &hmemkm, hmem, PVRSRV_HANDLE_TYPE_DEV_MEM_CONTEXT);

	if(err != PVRSRV_OK)
	{
		printk(KERN_ERR "MSVDX: Cannot get memory context from process data");

		return -1;
	}

	addr = BM_GetDeviceNode(hmemkm)->pfnMMUGetPDDevPAddr(BM_GetMMUContextFromMemContext(hmemkm));
	msvdx_init_compositor_mmu(addr.uiAddr);

	if (!platform->msvdx_pvr) {
		ret = msvdx_pvr_init();
		if (ret) {
			printk(KERN_INFO "Failed in msvdx_pvr_init()");
		} else {

		   	printk(KERN_INFO "Succeed for msvdx_pvr_init()");

		}
	}


	return 0;
}

/*
 * Can't call from module initialization, since PVR services are started when
 * the Xorg server starts.
 */
int msvdx_pvr_init(void)
{
	drm_emgd_priv_t        *priv;
	igd_context_t           *context;
	platform_context_plb_t  *platform;
	struct msvdx_pvr_info   *pvr;
	PVRSRV_DEVICE_IDENTIFIER dev_id_list[PVRSRV_MAX_DEVICES];
	IMG_UINT32   pid;
	IMG_UINT32   num_devices;
	IMG_UINT32   i;
	IMG_BOOL     mem_created;
	PVRSRV_ERROR err;
	int ret;

	priv     = gpDrmDevice->dev_private;
	context  = priv->context;
	platform = (platform_context_plb_t *)context->platform_context;

	if (platform->msvdx_pvr) {
		printk(KERN_INFO "[EMGD] MSVDX: PVR services already "
			"initialized\n");
		return 0;
	}

    pvr = kzalloc(sizeof(*pvr), GFP_KERNEL);
	if (!pvr)
		return -ENOMEM;

    /*
	 * Create a dummy kernel thread so that a persistent PVR per process
	 * data could be created.
	 */
	pvr->kthread = kthread_run(msvdx_pvr_kthread, NULL, "msvdx-pvr");
	if (IS_ERR(pvr->kthread)) {
		ret = PTR_ERR(pvr->kthread);
		printk(KERN_ERR "[EMGD] MSVDX: failed to create MSVDX PVR "
			"kernel tread, error=%i\n", ret);
		pvr->kthread = NULL;
		goto out_free;
	}

	ret = -ENODEV;
	pid = OSGetCurrentProcessIDKM(); //(IMG_UINT32)pvr->kthread->pid;
    err = PVRSRVPerProcessDataConnect(pid);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] MSVDX: connect to PVR failed (pid=%u), "
			"error=%i\n", (unsigned int)err, err);
		goto out_stop_kthread;
	}
	pvr->per_proc = PVRSRVPerProcessData(pid);
	if (pvr->per_proc == IMG_NULL) {
		printk(KERN_ERR "[EMGD] MSVDX: Couldn't find per process "
			"data for pid=%u\n", (unsigned int)pid);
		goto out_stop_kthread;
	}

	err = PVRSRVEnumerateDevicesKM(&num_devices, dev_id_list);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] MSVDX: PVRSRVEnumerateDevice() failed, "
			"error=%u\n", (unsigned int)err);
		goto out_stop_kthread;
	}

	for (i = 0 ; i < num_devices ; i++) {
		PVRSRV_DEVICE_IDENTIFIER *id;
		IMG_HANDLE cookie;

		cookie = IMG_NULL;
		id = &dev_id_list[i];

		if (id->eDeviceType != PVRSRV_DEVICE_TYPE_EXT) {
			err = PVRSRVAcquireDeviceDataKM(id->ui32DeviceIndex,
				PVRSRV_DEVICE_TYPE_UNKNOWN, &cookie);
			if (err != PVRSRV_OK) {
				printk(KERN_ERR "[EMGD] MSVDX: "
					"PVRSRVAcquireDeviceDataKM() failed, "
					"error=%u\n", err);
				break;
			}
			if (PVRSRV_DEVICE_TYPE_SGX == id->eDeviceType) {
				pvr->sgx_cookie = cookie;
				break;
			}
		}
	}

	if (pvr->sgx_cookie == IMG_NULL)
		goto out_stop_kthread;

	err = PVRSRVCreateDeviceMemContextKM(pvr->sgx_cookie, pvr->per_proc,
			&pvr->dev_mem_context, &pvr->heap_count,
			pvr->heap_info, &mem_created, pvr->heap_shared);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] MSVDX: PVRSRVCreateDeviceMemContextKM()"
			" failed, error=%u\n", (unsigned int)err);
		goto out_stop_kthread;
	}

	for (i = 0; i < pvr->heap_count; i++) {
		if (HEAP_IDX(pvr->heap_info[i].ui32HeapID) ==
			SGX_VIDEO_HEAP_ID) {
			pvr->mapping_heap_index = i;
			break;
		}
	}

	platform->msvdx_pvr = pvr;
	pvr->pid = pid;


	return 0;

out_stop_kthread:
	kthread_stop(pvr->kthread);
out_free:
	kfree(pvr);
	return ret;
}

/*
 * HHP: FIXME:
 * This should also be called when GMM/GTT is re-initialized, i.e. when
 * the Xorg server stops.  The PVR backed memory is invalid when this happens,
 * so if we call this function from msvdx_driver_unload() when the module is
 * removed, the "fw_mem_info" and "dev_mem_context" are invalid.
 */
void msvdx_pvr_deinit(void)
{
	drm_emgd_priv_t       *priv;
	igd_context_t          *context;
	platform_context_plb_t *platform;
	struct msvdx_pvr_info  *pvr;
	IMG_BOOL mem_destroyed;
	int pid=0;
	unsigned char *mmio;

	priv     = gpDrmDevice->dev_private;
	context  = priv->context;
	platform = (platform_context_plb_t *)context->platform_context;
    mmio = context->device_context.virt_mmadr;
	pvr = platform->msvdx_pvr;

    //Reset MTX before unloading firmware
    EMGD_WRITE32(0x00000001, mmio + PSB_MSVDX_MTX_SOFT_RESET);


	if (pvr) {
        kthread_stop(pvr->kthread);
		if (pvr->fw_mem_info)
			PVRSRVFreeDeviceMemKM(pvr->sgx_cookie,
						pvr->fw_mem_info);

		PVRSRVDestroyDeviceMemContextKM(pvr->sgx_cookie,
						pvr->dev_mem_context,
						&mem_destroyed);

	//	PVRSRVPerProcessDataDisconnect((IMG_UINT32)pvr->kthread->pid);
        PVRSRVPerProcessDataDisconnect((IMG_UINT32)pvr->pid);
        pid = pvr->pid;
		kfree(pvr);
		platform->msvdx_pvr = NULL;
	}

}

PVRSRV_KERNEL_MEM_INFO *msvdx_pvr_alloc_devmem(unsigned long alloc_size,
	const char *mame)
{
	drm_emgd_priv_t        *priv;
	igd_context_t           *context;
	platform_context_plb_t  *platform;
	struct msvdx_pvr_info   *pvr;
	PVRSRV_KERNEL_MEM_INFO  *mem_info;
	PVRSRV_ERROR err;

	priv     = gpDrmDevice->dev_private;
	context  = priv->context;
	platform = (platform_context_plb_t *)context->platform_context;

	pvr = platform->msvdx_pvr;
	if (!pvr)
		return NULL;

	/* Is PVRSRV_MEM_CACHE_CONSISTENT needed in the allocation flags? */
	err = PVRSRVAllocDeviceMemKM(pvr->sgx_cookie, pvr->per_proc,
			pvr->heap_info[pvr->mapping_heap_index].hDevMemHeap,
			PVRSRV_MEM_READ | PVRSRV_MEM_WRITE,
			alloc_size, 0, &mem_info, name);
	if (err != PVRSRV_OK) {
		printk(KERN_ERR "[EMGD] MSVDX: PVRSRVAllocDeviceMemKM failed, "
			"error=%u\n", (unsigned int)err);
		return NULL;
	}

	return mem_info;
}

static PVRSRV_ERROR DevInitMSVDXPart1(IMG_VOID *pvDeviceNode)
{
    /*
	PVRSRV_DEVICE_NODE *psDeviceNode = (PVRSRV_DEVICE_NODE *)pvDeviceNode;
	PVRSRV_ERROR eError;
	PVRSRV_DEV_POWER_STATE eDefaultPowerState;
    */

	/* register power operation function */
	/* FIXME: this should be in part2 init function, but
	 * currently here only OSPM needs IMG device... */
    /*
	eDefaultPowerState = PVRSRV_DEV_POWER_STATE_OFF;
	eError = PVRSRVRegisterPowerDevice(psDeviceNode->sDevId.ui32DeviceIndex,
					   MSVDXPrePowerState,
					   MSVDXPostPowerState,
					   MSVDXPreClockSpeedChange,
					   MSVDXPostClockSpeedChange,
					   (IMG_HANDLE)psDeviceNode,
					   PVRSRV_DEV_POWER_STATE_ON,
					   eDefaultPowerState);
	if (eError != PVRSRV_OK) {
		PVR_DPF((PVR_DBG_ERROR, "DevInitMSVDXPart1: failed to "
			 "register device with power manager"));
		return eError;
	}
    */

	return PVRSRV_OK;
}

static PVRSRV_ERROR DevDeInitMSVDX(IMG_VOID *pvDeviceNode)
{
	PVRSRV_DEVICE_NODE *psDeviceNode = (PVRSRV_DEVICE_NODE *)pvDeviceNode;
	PVRSRV_ERROR eError;

	/* should deinit all resource */

	eError = PVRSRVRemovePowerDevice(psDeviceNode->sDevId.ui32DeviceIndex);
	if (eError != PVRSRV_OK)
		return eError;

	return PVRSRV_OK;
}

PVRSRV_ERROR MSVDXDevInitCompatCheck(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	/* version check */

	return PVRSRV_OK;
}

PVRSRV_ERROR MSVDXRegisterDevice(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	psDeviceNode->sDevId.eDeviceType	= PVRSRV_DEVICE_TYPE_MSVDX;
	psDeviceNode->sDevId.eDeviceClass	= PVRSRV_DEVICE_CLASS_VIDEO;

	psDeviceNode->pfnInitDevice		= DevInitMSVDXPart1;
	psDeviceNode->pfnDeInitDevice		= DevDeInitMSVDX;

	psDeviceNode->pfnInitDeviceCompatCheck	= MSVDXDevInitCompatCheck;

	psDeviceNode->pfnDeviceISR = msvdx_mtx_isr;
	psDeviceNode->pvISRData = (IMG_VOID *)gpDrmDevice;

	return PVRSRV_OK;
}

/*
PVRSRV_ERROR MSVDXPrePowerState(IMG_HANDLE hDevHandle,
				 PVRSRV_DEV_POWER_STATE	eNewPowerState,
				 PVRSRV_DEV_POWER_STATE	eCurrentPowerState)
{
	// ask for a change not power on
	if ((eNewPowerState != eCurrentPowerState) &&
	    (eNewPowerState != PVRSRV_DEV_POWER_STATE_ON)) {
		struct drm_psb_private *dev_priv = gpDrmDevice->dev_private;
		struct msvdx_private *msvdx_priv = dev_priv->msvdx_private;
		MSVDX_NEW_PMSTATE(gpDrmDevice, msvdx_priv, PSB_PMSTATE_POWERDOWN);

		// context save
		psb_msvdx_save_context(gpDrmDevice);

		// internally close the device //

		// ask for power off //
		if (eNewPowerState == PVRSRV_DEV_POWER_STATE_OFF) {
			// here will deinitialize the driver if needed //
		} else {
			PVR_DPF((PVR_DBG_MESSAGE,
				"%s no action for transform from %d to %d",
				 __func__,
				eCurrentPowerState,
				eNewPowerState));
		}
	}

	return PVRSRV_OK;
}

PVRSRV_ERROR MSVDXPostPowerState(IMG_HANDLE hDevHandle,
				 PVRSRV_DEV_POWER_STATE	eNewPowerState,
				 PVRSRV_DEV_POWER_STATE	eCurrentPowerState)
{
	// if ask for change & current status is not on //
	if ((eNewPowerState != eCurrentPowerState) &&
	    (eCurrentPowerState != PVRSRV_DEV_POWER_STATE_ON)) {
		// internally open device //
		struct drm_psb_private *dev_priv = gpDrmDevice->dev_private;
		struct msvdx_private *msvdx_priv = dev_priv->msvdx_private;
		MSVDX_NEW_PMSTATE(gpDrmDevice, msvdx_priv, PSB_PMSTATE_POWERUP);

		// context restore //
		psb_msvdx_restore_context(gpDrmDevice);

		if (eCurrentPowerState == PVRSRV_DEV_POWER_STATE_OFF) {
			// here will initialize the driver if needed //
		} else {
			PVR_DPF((PVR_DBG_MESSAGE,
				"%s no action for transform from %d to %d",
				 __func__,
				eCurrentPowerState,
				eNewPowerState));
		}
	}

	return PVRSRV_OK;
}

PVRSRV_ERROR MSVDXPreClockSpeedChange(IMG_HANDLE hDevHandle,
				      IMG_BOOL bIdleDevice,
				      PVRSRV_DEV_POWER_STATE eCurrentPowerState)
{
	return PVRSRV_OK;
}

PVRSRV_ERROR MSVDXPostClockSpeedChange(IMG_HANDLE hDevHandle,
				      IMG_BOOL bIdleDevice,
				      PVRSRV_DEV_POWER_STATE eCurrentPowerState)
{
	return PVRSRV_OK;
}
*/

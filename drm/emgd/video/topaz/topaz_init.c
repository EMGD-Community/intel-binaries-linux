/*
 *-----------------------------------------------------------------------------
 * Filename: topaz_init.c
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
 *
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
#include <topaz.h>

#include <plb/regs.h>
#include <plb/context.h>

#include "topaz_hdr.h"

//#include <interrupt.h>
//#include <os/interrupt.h>

#include <drm/drm.h>
#include <drm_emgd_private.h>
#include "services_headers.h"

#include "pvr_bridge_km.h"
#include "msvdx_pvr.h"
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
#include <linux/mutex.h>
#else
#include <asm/semaphore.h>
#endif
static PVRSRV_KERNEL_MEM_INFO   *g_wb_mem_info = NULL;
static int g_topaz_initialized = 0;
static IMG_VOID *g_pMutex = IMG_NULL;
/* DDK specific DRM device */
extern struct drm_device *gpDrmDevice;

unsigned char *base_firmware_address;

extern int process_encode_mtx_messages(igd_context_t *context,
                        unsigned long *mtx_buf,
                        unsigned long size);

static int reg_ready_tnc(igd_context_t *,
		unsigned long ,
		unsigned long ,
		unsigned long );

static int write_firmware(igd_context_t *,
		unsigned long ,
		unsigned long ,
		unsigned long *,
		unsigned long );

static int upload_firmware(igd_context_t *, enc_fw_info_t *);

void topaz_write_core_reg(igd_context_t *,
			unsigned long,
			unsigned long);

void topaz_read_core_reg(igd_context_t *,
			unsigned long,
			unsigned long*);

void get_mtx_control_from_dash(igd_context_t *context);

void release_mtx_control_from_dash(igd_context_t *context);

#define RAM_SIZE (1024 * 24)
#define ENC_FW_SIZE (512 * 1024)

/* register default values */
static unsigned long topaz_def_regs[184][3] = {
	{MVEA_BASE, 0x00000000, 0x00000000},
	{MVEA_BASE, 0x00000004, 0x00000400},
	{MVEA_BASE, 0x00000008, 0x00000000},
	{MVEA_BASE, 0x0000000C, 0x00000000},
	{MVEA_BASE, 0x00000010, 0x00000000},
	{MVEA_BASE, 0x00000014, 0x00000000},
	{MVEA_BASE, 0x00000018, 0x00000000},
	{MVEA_BASE, 0x0000001C, 0x00000000},
	{MVEA_BASE, 0x00000020, 0x00000120},
	{MVEA_BASE, 0x00000024, 0x00000000},
	{MVEA_BASE, 0x00000028, 0x00000000},
	{MVEA_BASE, 0x00000100, 0x00000000},
	{MVEA_BASE, 0x00000104, 0x00000000},
	{MVEA_BASE, 0x00000108, 0x00000000},
	{MVEA_BASE, 0x0000010C, 0x00000000},
	{MVEA_BASE, 0x0000011C, 0x00000001},
	{MVEA_BASE, 0x0000012C, 0x00000000},
	{MVEA_BASE, 0x00000180, 0x00000000},
	{MVEA_BASE, 0x00000184, 0x00000000},
	{MVEA_BASE, 0x00000188, 0x00000000},
	{MVEA_BASE, 0x0000018C, 0x00000000},
	{MVEA_BASE, 0x00000190, 0x00000000},
	{MVEA_BASE, 0x00000194, 0x00000000},
	{MVEA_BASE, 0x00000198, 0x00000000},
	{MVEA_BASE, 0x0000019C, 0x00000000},
	{MVEA_BASE, 0x000001A0, 0x00000000},
	{MVEA_BASE, 0x000001A4, 0x00000000},
	{MVEA_BASE, 0x000001A8, 0x00000000},
	{MVEA_BASE, 0x000001AC, 0x00000000},
	{MVEA_BASE, 0x000001B0, 0x00000000},
	{MVEA_BASE, 0x000001B4, 0x00000000},
	{MVEA_BASE, 0x000001B8, 0x00000000},
	{MVEA_BASE, 0x000001BC, 0x00000000},
	{MVEA_BASE, 0x000001F8, 0x00000000},
	{MVEA_BASE, 0x000001FC, 0x00000000},
	{MVEA_BASE, 0x00000200, 0x00000000},
	{MVEA_BASE, 0x00000204, 0x00000000},
	{MVEA_BASE, 0x00000208, 0x00000000},
	{MVEA_BASE, 0x0000020C, 0x00000000},
	{MVEA_BASE, 0x00000210, 0x00000000},
	{MVEA_BASE, 0x00000220, 0x00000001},
	{MVEA_BASE, 0x00000224, 0x0000001F},
	{MVEA_BASE, 0x00000228, 0x00000100},
	{MVEA_BASE, 0x0000022C, 0x00001F00},
	{MVEA_BASE, 0x00000230, 0x00000101},
	{MVEA_BASE, 0x00000234, 0x00001F1F},
	{MVEA_BASE, 0x00000238, 0x00001F01},
	{MVEA_BASE, 0x0000023C, 0x0000011F},
	{MVEA_BASE, 0x00000240, 0x00000200},
	{MVEA_BASE, 0x00000244, 0x00001E00},
	{MVEA_BASE, 0x00000248, 0x00000002},
	{MVEA_BASE, 0x0000024C, 0x0000001E},
	{MVEA_BASE, 0x00000250, 0x00000003},
	{MVEA_BASE, 0x00000254, 0x0000001D},
	{MVEA_BASE, 0x00000258, 0x00001F02},
	{MVEA_BASE, 0x0000025C, 0x00000102},
	{MVEA_BASE, 0x00000260, 0x0000011E},
	{MVEA_BASE, 0x00000264, 0x00000000},
	{MVEA_BASE, 0x00000268, 0x00000000},
	{MVEA_BASE, 0x0000026C, 0x00000000},
	{MVEA_BASE, 0x00000270, 0x00000000},
	{MVEA_BASE, 0x00000274, 0x00000000},
	{MVEA_BASE, 0x00000278, 0x00000000},
	{MVEA_BASE, 0x00000280, 0x00008000},
	{MVEA_BASE, 0x00000284, 0x00000000},
	{MVEA_BASE, 0x00000288, 0x00000000},
	{MVEA_BASE, 0x0000028C, 0x00000000},
	{MVEA_BASE, 0x00000314, 0x00000000},
	{MVEA_BASE, 0x00000318, 0x00000000},
	{MVEA_BASE, 0x0000031C, 0x00000000},
	{MVEA_BASE, 0x00000320, 0x00000000},
	{MVEA_BASE, 0x00000324, 0x00000000},
	{MVEA_BASE, 0x00000348, 0x00000000},
	{MVEA_BASE, 0x00000380, 0x00000000},
	{MVEA_BASE, 0x00000384, 0x00000000},
	{MVEA_BASE, 0x00000388, 0x00000000},
	{MVEA_BASE, 0x0000038C, 0x00000000},
	{MVEA_BASE, 0x00000390, 0x00000000},
	{MVEA_BASE, 0x00000394, 0x00000000},
	{MVEA_BASE, 0x00000398, 0x00000000},
	{MVEA_BASE, 0x0000039C, 0x00000000},
	{MVEA_BASE, 0x000003A0, 0x00000000},
	{MVEA_BASE, 0x000003A4, 0x00000000},
	{MVEA_BASE, 0x000003A8, 0x00000000},
	{MVEA_BASE, 0x000003B0, 0x00000000},
	{MVEA_BASE, 0x000003B4, 0x00000000},
	{MVEA_BASE, 0x000003B8, 0x00000000},
	{MVEA_BASE, 0x000003BC, 0x00000000},
	{MVEA_BASE, 0x000003D4, 0x00000000},
	{MVEA_BASE, 0x000003D8, 0x00000000},
	{MVEA_BASE, 0x000003DC, 0x00000000},
	{MVEA_BASE, 0x000003E0, 0x00000000},
	{MVEA_BASE, 0x000003E4, 0x00000000},
	{MVEA_BASE, 0x000003EC, 0x00000000},
	{MVEA_BASE, 0x000002D0, 0x00000000},
	{MVEA_BASE, 0x000002D4, 0x00000000},
	{MVEA_BASE, 0x000002D8, 0x00000000},
	{MVEA_BASE, 0x000002DC, 0x00000000},
	{MVEA_BASE, 0x000002E0, 0x00000000},
	{MVEA_BASE, 0x000002E4, 0x00000000},
	{MVEA_BASE, 0x000002E8, 0x00000000},
	{MVEA_BASE, 0x000002EC, 0x00000000},
	{MVEA_BASE, 0x000002F0, 0x00000000},
	{MVEA_BASE, 0x000002F4, 0x00000000},
	{MVEA_BASE, 0x000002F8, 0x00000000},
	{MVEA_BASE, 0x000002FC, 0x00000000},
	{MVEA_BASE, 0x00000300, 0x00000000},
	{MVEA_BASE, 0x00000304, 0x00000000},
	{MVEA_BASE, 0x00000308, 0x00000000},
	{MVEA_BASE, 0x0000030C, 0x00000000},
	{MVEA_BASE, 0x00000290, 0x00000000},
	{MVEA_BASE, 0x00000294, 0x00000000},
	{MVEA_BASE, 0x00000298, 0x00000000},
	{MVEA_BASE, 0x0000029C, 0x00000000},
	{MVEA_BASE, 0x000002A0, 0x00000000},
	{MVEA_BASE, 0x000002A4, 0x00000000},
	{MVEA_BASE, 0x000002A8, 0x00000000},
	{MVEA_BASE, 0x000002AC, 0x00000000},
	{MVEA_BASE, 0x000002B0, 0x00000000},
	{MVEA_BASE, 0x000002B4, 0x00000000},
	{MVEA_BASE, 0x000002B8, 0x00000000},
	{MVEA_BASE, 0x000002BC, 0x00000000},
	{MVEA_BASE, 0x000002C0, 0x00000000},
	{MVEA_BASE, 0x000002C4, 0x00000000},
	{MVEA_BASE, 0x000002C8, 0x00000000},
	{MVEA_BASE, 0x000002CC, 0x00000000},
	{MVEA_BASE, 0x00000080, 0x00000000},
	{MVEA_BASE, 0x00000084, 0x80705700},
	{MVEA_BASE, 0x00000088, 0x00000000},
	{MVEA_BASE, 0x0000008C, 0x00000000},
	{MVEA_BASE, 0x00000090, 0x00000000},
	{MVEA_BASE, 0x00000094, 0x00000000},
	{MVEA_BASE, 0x00000098, 0x00000000},
	{MVEA_BASE, 0x0000009C, 0x00000000},
	{MVEA_BASE, 0x000000A0, 0x00000000},
	{MVEA_BASE, 0x000000A4, 0x00000000},
	{MVEA_BASE, 0x000000A8, 0x00000000},
	{MVEA_BASE, 0x000000AC, 0x00000000},
	{MVEA_BASE, 0x000000B0, 0x00000000},
	{MVEA_BASE, 0x000000B4, 0x00000000},
	{MVEA_BASE, 0x000000B8, 0x00000000},
	{MVEA_BASE, 0x000000BC, 0x00000000},
	{MVEA_BASE, 0x000000C0, 0x00000000},
	{MVEA_BASE, 0x000000C4, 0x00000000},
	{MVEA_BASE, 0x000000C8, 0x00000000},
	{MVEA_BASE, 0x000000CC, 0x00000000},
	{MVEA_BASE, 0x000000D0, 0x00000000},
	{MVEA_BASE, 0x000000D4, 0x00000000},
	{MVEA_BASE, 0x000000D8, 0x00000000},
	{MVEA_BASE, 0x000000DC, 0x00000000},
	{MVEA_BASE, 0x000000E0, 0x00000000},
	{MVEA_BASE, 0x000000E4, 0x00000000},
	{MVEA_BASE, 0x000000E8, 0x00000000},
	{MVEA_BASE, 0x000000EC, 0x00000000},
	{MVEA_BASE, 0x000000F0, 0x00000000},
	{MVEA_BASE, 0x000000F4, 0x00000000},
	{MVEA_BASE, 0x000000F8, 0x00000000},
	{MVEA_BASE, 0x000000FC, 0x00000000},
	{VLC_BASE, 0x00000000, 0x00000000},
	{VLC_BASE, 0x00000004, 0x00000000},
	{VLC_BASE, 0x00000008, 0x00000000},
	{VLC_BASE, 0x0000000C, 0x00000000},
	{VLC_BASE, 0x00000010, 0x00000000},
	{VLC_BASE, 0x00000014, 0x00000000},
	{VLC_BASE, 0x0000001C, 0x00000000},
	{VLC_BASE, 0x00000020, 0x00000000},
	{VLC_BASE, 0x00000024, 0x00000000},
	{VLC_BASE, 0x0000002C, 0x00000000},
	{VLC_BASE, 0x00000034, 0x00000000},
	{VLC_BASE, 0x00000038, 0x00000000},
	{VLC_BASE, 0x0000003C, 0x00000000},
	{VLC_BASE, 0x00000040, 0x00000000},
	{VLC_BASE, 0x00000044, 0x00000000},
	{VLC_BASE, 0x00000048, 0x00000000},
	{VLC_BASE, 0x0000004C, 0x00000000},
	{VLC_BASE, 0x00000050, 0x00000000},
	{VLC_BASE, 0x00000054, 0x00000000},
	{VLC_BASE, 0x00000058, 0x00000000},
	{VLC_BASE, 0x0000005C, 0x00000000},
	{VLC_BASE, 0x00000060, 0x00000000},
	{VLC_BASE, 0x00000064, 0x00000000},
	{VLC_BASE, 0x00000068, 0x00000000},
	{VLC_BASE, 0x0000006C, 0x00000000},
	{0xffffffff, 0xffffffff, 0xffffffff}
};

unsigned long ui32H264_MTXTOPAZFWTextSize = 3478;
unsigned long ui32H264_MTXTOPAZFWDataSize = 4704;
unsigned long ui32H264_MTXTOPAZFWDataLocation = 0x82883680;

unsigned long ui32H264VBR_MTXTOPAZFWTextSize = 4730;
unsigned long ui32H264VBR_MTXTOPAZFWDataSize = 3456;
unsigned long ui32H264VBR_MTXTOPAZFWDataLocation = 0x82884a00;

unsigned long ui32H264CBR_MTXTOPAZFWTextSize = 5084;
unsigned long ui32H264CBR_MTXTOPAZFWDataSize = 3104;
unsigned long ui32H264CBR_MTXTOPAZFWDataLocation = 0x82884f80;

unsigned long ui32H263CBR_MTXTOPAZFWTextSize = 4383;
unsigned long ui32H263CBR_MTXTOPAZFWDataSize = 3808;
unsigned long ui32H263CBR_MTXTOPAZFWDataLocation = 0x82884480;

unsigned long ui32H263VBR_MTXTOPAZFWTextSize = 4498;
unsigned long ui32H263VBR_MTXTOPAZFWDataSize = 3680;
unsigned long ui32H263VBR_MTXTOPAZFWDataLocation = 0x82884680;

unsigned long ui32H263_MTXTOPAZFWTextSize = 3202;
unsigned long ui32H263_MTXTOPAZFWDataSize = 4976;
unsigned long ui32H263_MTXTOPAZFWDataLocation = 0x82883240;

unsigned long ui32MPG4CBR_MTXTOPAZFWTextSize = 4403;
unsigned long ui32MPG4CBR_MTXTOPAZFWDataSize = 3776;
unsigned long ui32MPG4CBR_MTXTOPAZFWDataLocation = 0x82884500;

unsigned long ui32MPG4VBR_MTXTOPAZFWTextSize = 4519;
unsigned long ui32MPG4VBR_MTXTOPAZFWDataSize = 3664;
unsigned long ui32MPG4VBR_MTXTOPAZFWDataLocation = 0x828846c0;

unsigned long ui32MPG4_MTXTOPAZFWTextSize = 3223;
unsigned long ui32MPG4_MTXTOPAZFWDataSize = 4960;
unsigned long ui32MPG4_MTXTOPAZFWDataLocation = 0x82883280;

static enc_fw_info_t firmware[10] = {
	{ 0,0,0,0,0,0},
	{
		FW_H264_NO_RC,
		&ui32H264_MTXTOPAZFWTextSize,
		&ui32H264_MTXTOPAZFWDataSize,
		&ui32H264_MTXTOPAZFWDataLocation,
		NULL, /*aui32H264_MTXTOPAZFWText,*/
		NULL /*aui32H264_MTXTOPAZFWData*/
	},
	{
		FW_H264_VBR,
		&ui32H264VBR_MTXTOPAZFWTextSize,
		&ui32H264VBR_MTXTOPAZFWDataSize,
		&ui32H264VBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32H264VBR_MTXTOPAZFWText,*/
		NULL /*aui32H264VBR_MTXTOPAZFWData */
	},
	{
		FW_H264_CBR,
		&ui32H264CBR_MTXTOPAZFWTextSize,
		&ui32H264CBR_MTXTOPAZFWDataSize,
		&ui32H264CBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32H264CBR_MTXTOPAZFWText,*/
		NULL /*aui32H264CBR_MTXTOPAZFWData */
	},
	{
		FW_H263_NO_RC,
		&ui32H263_MTXTOPAZFWTextSize,
		&ui32H263_MTXTOPAZFWDataSize,
		&ui32H263_MTXTOPAZFWDataLocation,
		NULL, /*aui32H263_MTXTOPAZFWText,*/
		NULL /*aui32H263_MTXTOPAZFWData*/
	},
	{
		FW_H263_VBR,
		&ui32H263VBR_MTXTOPAZFWTextSize,
		&ui32H263VBR_MTXTOPAZFWDataSize,
		&ui32H263VBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32H263VBR_MTXTOPAZFWText,*/
		NULL /*aui32H263VBR_MTXTOPAZFWData*/
	},
	{
		FW_H263_CBR,
		&ui32H263CBR_MTXTOPAZFWTextSize,
		&ui32H263CBR_MTXTOPAZFWDataSize,
		&ui32H263CBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32H263CBR_MTXTOPAZFWText,*/
		NULL /*aui32H263CBR_MTXTOPAZFWData*/
	},
	{
		FW_MPEG4_NO_RC,
		&ui32MPG4_MTXTOPAZFWTextSize,
		&ui32MPG4_MTXTOPAZFWDataSize,
		&ui32MPG4_MTXTOPAZFWDataLocation,
		NULL, /*aui32MPG4_MTXTOPAZFWText,*/
		NULL /*aui32MPG4_MTXTOPAZFWData*/
	},
	{
		FW_MPEG4_VBR,
		&ui32MPG4VBR_MTXTOPAZFWTextSize,
		&ui32MPG4VBR_MTXTOPAZFWDataSize,
		&ui32MPG4VBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32MPG4VBR_MTXTOPAZFWText,*/
		NULL /*aui32MPG4VBR_MTXTOPAZFWData*/
	},
	{
		FW_MPEG4_CBR,
		&ui32MPG4CBR_MTXTOPAZFWTextSize,
		&ui32MPG4CBR_MTXTOPAZFWDataSize,
		&ui32MPG4CBR_MTXTOPAZFWDataLocation,
		NULL, /*aui32MPG4CBR_MTXTOPAZFWText,*/
		NULL /*aui32MPG4CBR_MTXTOPAZFWData*/
	}
};

IMG_VOID * HostCreateMutex(IMG_VOID)
{
	struct semaphore *psSem;

	psSem = kmalloc(sizeof(*psSem), GFP_KERNEL);
	if (psSem)
	{
		sema_init(psSem, 1);
	}

	return psSem;
}

IMG_VOID HostAquireMutex(IMG_VOID * pvMutex)
{
	BUG_ON(in_interrupt());

#if defined(PVR_DEBUG_DBGDRV_DETECT_HOST_MUTEX_COLLISIONS)
	if (down_trylock((struct semaphore *)pvMutex))
	{
		printk(KERN_INFO "HostAquireMutex: Waiting for mutex\n");
		down((struct semaphore *)pvMutex);
	}
#else
	down((struct semaphore *)pvMutex);
#endif
}

IMG_VOID HostReleaseMutex(IMG_VOID * pvMutex)
{
	up((struct semaphore *)pvMutex);
}

IMG_VOID HostDestroyMutex(IMG_VOID * pvMutex)
{
	if (pvMutex)
	{
		kfree(pvMutex);
	}
}
int topaz_shutdown_tnc(igd_context_t *context)
{
	EMGD_TRACE_ENTER;
	
	
	g_topaz_initialized = 0;
	g_wb_mem_info = NULL;

	HostDestroyMutex(g_pMutex);
	g_pMutex = IMG_NULL;
	
	EMGD_TRACE_EXIT;
	return 0;
}
int topaz_init_tnc (unsigned long wb_offset,
					void* mem_handle_writeback,
		            void *mem_handle_enc_fw)
{
	drm_emgd_priv_t *priv;
	igd_context_t *context;
	unsigned char *mmio;
	unsigned long size;
	int i;
	platform_context_tnc_t *platform = NULL;
	tnc_topaz_priv_t *topaz_priv;
	unsigned long *km_firm_addr = NULL;
	unsigned long *firm_offset_values = NULL;
	PVRSRV_ERROR err;
	PVRSRV_PER_PROCESS_DATA *ps_data = NULL;
	IMG_UINT32 pid = 0;
	PVRSRV_KERNEL_MEM_INFO *mem_info_fw = NULL;
	/* PVRSRV_KERNEL_MEM_INFO *mem_info_wb = NULL; */
	struct msvdx_pvr_info   *pvr;

	priv = gpDrmDevice->dev_private;
	context = priv->context;
	mmio = context->device_context.virt_mmadr;
#if 0
	if(!mem_handle_writeback || !mem_handle_enc_fw) {
		return -EINVAL;
	}
#else
	if (!mem_handle_enc_fw) {
		return -EINVAL;
	}
#endif
	pid = OSGetCurrentProcessIDKM();
	ps_data = PVRSRVPerProcessData(pid);
	if (!ps_data) {
		printk(KERN_ERR "TOPAZ: Cannot get process data information");
		return -1;
	}

	err = PVRSRVLookupHandle(ps_data->psHandleBase, (void **)&mem_info_fw,
			(IMG_HANDLE)mem_handle_enc_fw, PVRSRV_HANDLE_TYPE_MEM_INFO);

	if(err != PVRSRV_OK) {
		printk(KERN_ERR "TOPAZ: Cannot get fw memory context from process data");
		return -1;
	}
#if 0
	err = PVRSRVLookupHandle(ps_data->psHandleBase, (void **)&mem_info_wb,
			(IMG_HANDLE)mem_handle_writeback, PVRSRV_HANDLE_TYPE_MEM_INFO);

	if(err != PVRSRV_OK) {
		printk(KERN_ERR "TOPAZ: Cannot get wb memory context from process data");
		return -1;
	}
#endif

	if(!mem_info_fw || mem_info_fw->ui32AllocSize < ENC_FW_SIZE) {
		printk(KERN_ERR "TOPAZ: invalid fw memory context from process data");
		return -1;
	}

#if 0
	if(!mem_info_wb || mem_info_wb->ui32AllocSize < (4 * 1024)) {
		printk(KERN_ERR "TOPAZ: Invalid wb memory context from process data");
		return -1;
	}
#endif
	/* Only support Atom E6xx */
	if ((PCI_DEVICE_ID_VGA_TNC != context->device_context.did)||
	   (context->device_context.bid == PCI_DEVICE_ID_BRIDGE_TNC_ULP)) {
		return -1;
	}

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	if (g_topaz_initialized != 1)
	{
		spin_lock_init(&platform->topaz_init_tnc);		

		spin_lock(&platform->topaz_init_tnc);
		
		size = WRITEBACK_MEM_SIZE;
		GMM_SET_DEBUG_NAME("TOPAZ Writeback Memory");
		
		pvr = platform->msvdx_pvr;
		if (!g_wb_mem_info) {
			err = PVRSRVAllocDeviceMemKM(pvr->sgx_cookie, pvr->per_proc,
				pvr->heap_info[pvr->mapping_heap_index].hDevMemHeap,
				PVRSRV_MEM_READ | PVRSRV_MEM_WRITE,
				size, 0, &g_wb_mem_info, "TOPAZ writeback");
				
			if (!g_wb_mem_info) {
				printk(KERN_ERR "[EMGD] TOPAZ: Failed to allocate %u "
					"bytes from SGX heap\n",
					(unsigned int)size);
				
				spin_unlock(&platform->topaz_init_tnc);
				return -ENOMEM;
			}
		}

		platform->topaz_busy = 0;
		
		spin_unlock(&platform->topaz_init_tnc);
	}
	
	spin_lock(&platform->topaz_init_tnc);
	
	topaz_priv->topaz_busy = 0;
	topaz_priv->topaz_cmd_seq = 0;
	topaz_priv->topaz_fw_loaded = 0;
	topaz_priv->topaz_cur_codec = 0;
	topaz_priv->cur_mtx_data_size = 0;

	/*
	printk(KERN_INFO "Topaz write back memory = %p", topaz_priv->topaz_ccb_wb);
	printk(KERN_INFO "Topaz write back offset = %lx", topaz_priv->topaz_wb_offset);
	printk(KERN_INFO "Topaz write back sync memory = %p", topaz_priv->topaz_sync_addr);
	printk(KERN_INFO "Topaz write back sync offset = %lx", topaz_priv->topaz_sync_offset);
	*/
	
	if (g_topaz_initialized == 1)
	{
		spin_unlock(&platform->topaz_init_tnc);
		return 0;
	}	

	topaz_priv->topaz_wb_offset = g_wb_mem_info->sDevVAddr.uiAddr; //wb_offset;
	topaz_priv->topaz_ccb_wb = g_wb_mem_info->pvLinAddrKM; //(unsigned char *)mem_info_wb->pvLinAddrKM;

	/* Sync location will be half of the writeback memory. */
	topaz_priv->topaz_sync_addr = (unsigned long *)(topaz_priv->topaz_ccb_wb + 2048);
	topaz_priv->topaz_sync_offset = topaz_priv->topaz_wb_offset + 2048;
	
	*(topaz_priv->topaz_sync_addr) = 0; /*reset sync seq */
	topaz_priv->topaz_sync_id = 0; /*reset sync id */

	g_topaz_initialized = 1;
	
	spin_unlock(&platform->topaz_init_tnc);

	if ((g_pMutex = HostCreateMutex()) == IMG_NULL)
	{
		return 0;
	}
		
	topaz_priv->selected_codec = -1;
	
	/* firmware part */
	/* allocate memory for all firmwares */
	/* to check, is allocate or not */
	if ( !firmware[1].text && mem_info_fw->pvLinAddrKM) {
		base_firmware_address = kmalloc( 512 * 1024, GFP_KERNEL);
		if ( NULL == base_firmware_address){
			printk (KERN_INFO "Kernel memory allocation failed\n");
			printk (KERN_INFO "Kernel firmware is not loaded\n");
			return 1;
		}
		/* copy all firmware to kernel memory */
		km_firm_addr = (unsigned long *) base_firmware_address;
		firm_offset_values = km_firm_addr;
		memcpy(km_firm_addr, mem_info_fw->pvLinAddrKM, 512*1024);
		/* printk(KERN_INFO "Topaz km_firm_addr = %p", km_firm_addr); */
		/* to set firmwares */
		/* NB! all offsets in bytes */
		for ( i = 1; i < 10; i++){
			firmware[i].text = km_firm_addr + (firm_offset_values[2*(i-1) + 0] >> 2) ;
			firmware[i].data = km_firm_addr + (firm_offset_values[2*(i-1) + 1] >> 2) ;
		}
	}

#if 0
        /* DEBUG ONLY */
        /* load fw here to make sure firmware can be loaded. */
        topaz_setup_fw(context, FW_H264_NO_RC); /* just for test */

#endif

	return 0;
}

int topaz_setup_fw(igd_context_t *context, enum tnc_topaz_encode_fw codec)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long address, ctrl, core_id, core_rev;
	//unsigned long reg=0;
	int i = 0;
	enc_fw_info_t *curr_fw;
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	// do no reload firmware
	if (codec == topaz_priv->selected_codec)
	{
		return 0;
	}
	topaz_priv->selected_codec = codec;	
	EMGD_WRITE32(0x00000000, mmio + TNC_TOPAZ_MMU_CONTROL0);

	/* Reset MVEA
	 * 	MVEA_SPE_SOFT_RESET 	|
	 * 	MVEA_IPE_SOFT_RESET 	|
	 * 	MVEA_CMPRS_SOFT_RESET 	|
	 * 	MVEA_JMCOMP_SOFT_RESET 	|
	 * 	MVEA_CMC_SOFT_RESET	|
	 * 	MVEA_DCF_SOFT_RESET
	 */
	EMGD_WRITE32(0x0000003f, mmio + TNC_TOPAZ_IMG_MVEA_SRST);
	EMGD_WRITE32(0x00, mmio + TNC_TOPAZ_IMG_MVEA_SRST);

	/* topaz_set_default_regs */
	/* Set default value for Video Encode resgister */
	while (topaz_def_regs[i][0] != 0xffffffff) {
		EMGD_WRITE32(topaz_def_regs[i][2],
				mmio + topaz_def_regs[i][0] + topaz_def_regs[i][1]);
		i++;
	}

	/* topaz_upload_fw */
	/* Point to request firmware */
	if((codec < 1) || (codec > 9)) {
		return 1;
	}
	curr_fw = &firmware[codec];

	upload_firmware(context, curr_fw);

	/* topaz_write_core_reg */
	/* Start the firmware thread running */
	/*topaz_write_core_reg(context, TOPAZ_MTX_PC, PC_START_ADDRESS);*/

	/* topaz_read_core_reg */
	EMGD_WRITE32(PC_START_ADDRESS, mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_DATA);
	EMGD_WRITE32(TOPAZ_MTX_PC, mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST);
	reg_ready_tnc(context, TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST,
			0x80000000, 0x80000000);

	/* Enable auto clock gate for TOPAZ and MVEA
	 * 	TOPAZ_VLC_AUTO_CLK_GATE |
	 * 	TOPAZ_DB_AUTO_CLK_GATE  |
	 *
	 * 	MVEA_IPE_AUTO_CLK_GATE    |
	 * 	MVEA_SPE_AUTO_CLK_GATE    |
	 * 	MVEA_CMPRS_AUTO_CLK GATE  |
	 * 	MVEA_JMCOMP_AUTO_CLK_GATE |
	 */
	EMGD_WRITE32(0x00000003, mmio + TNC_TOPAZ_IMG_TOPAZ_AUTO_CLK_GATE);
	EMGD_WRITE32(0x0000000f, mmio + TNC_TOPAZ_MVEA_AUTO_CLOCK_GATING);

	/* Clear the MTX interrupt */
	EMGD_WRITE32(0x00000002, mmio + TNC_TOPAZ_IMG_TOPAZ_INTCLEAR);
	/* Enable MTX */
	EMGD_WRITE32(0x00000001, mmio + TNC_TOPAZ_MTX_ENABLE);
	/* Once enable MTX fw should generate interrupt to indicate its reday */
	reg_ready_tnc(context, TNC_TOPAZ_IMG_TOPAZ_INTSTAT,
			0x00000002, 0x00000002);
	/* Clear the MTX interrupt */
	EMGD_WRITE32(0x00000002, mmio + TNC_TOPAZ_IMG_TOPAZ_INTCLEAR);

	/* get ccb buffer addr - file hostutils.c */
	topaz_priv->topaz_ccb_buffer_addr = read_mtx_mem(
			context, MTX_DATA_BASE + RAM_SIZE - 4);
	topaz_priv->topaz_ccb_ctrl_addr = read_mtx_mem( context,
			MTX_DATA_BASE + RAM_SIZE - 8);
	topaz_priv->topaz_ccb_size = read_mtx_mem( context,
			topaz_priv->topaz_ccb_ctrl_addr + MTX_CCB_CTRL_CCB_SIZE);
	topaz_priv->topaz_cmd_windex = 0;

	/* write back the intial QP value */
	write_mtx_mem(context,
			topaz_priv->topaz_ccb_ctrl_addr + MTX_CCB_CTRL_INIT_QP,
			topaz_priv->stored_initial_qp);
	write_mtx_mem(context,
			MTX_DATA_BASE + RAM_SIZE - 12, topaz_priv->topaz_wb_offset);

	/*reg = EMGD_READ32(mmio+TNC_TOPAZ_IMG_TOPAZ_INTSTAT);
	printk( KERN_INFO "b4 first kick: TOPAZ_INTSTAT= 0x%08x ", reg);*/

	/* this kick is important to update the WB offset */
	*((unsigned long *)topaz_priv->topaz_ccb_wb) = 0x01020304;
	EMGD_WRITE32(1, mmio + TNC_TOPAZ_MTX_KICK);
	OS_SLEEP(1000);
	/* printk(KERN_INFO "Expected 0x12345678, wb return 0x%08lx",
			*((unsigned long *)topaz_priv->topaz_ccb_wb));
	   reg = EMGD_READ32(mmio+TNC_TOPAZ_IMG_TOPAZ_INTSTAT);
	   printk( KERN_INFO "after first kick: TOPAZ_INTSTAT= 0x%08x ", reg);
	   reg = EMGD_READ32(mmio+TNC_TOPAZ_MMU_STATUS);
	   printk( KERN_INFO "sync: TOPAZ_MMU_STATUS= 0x%08x ", reg);*/

	topaz_priv->topaz_busy = 0;
	topaz_priv->topaz_cmd_seq = 0;

	/* Enable MMU by pass when confguring MMU - TOPAZ_CR_MMU_BYPASS*/
	EMGD_WRITE32(0x00000800, mmio + TNC_TOPAZ_MMU_CONTROL0);

	/* Topaz MMU will point to the same PD as SGX. */
	address = EMGD_READ32(mmio + PSB_CR_BIF_DIR_LIST_BASE1);
	EMGD_WRITE32(address, mmio + TNC_TOPAZ_MMU_DIR_LIST_BASE0);
	EMGD_WRITE32(0, mmio + TNC_TOPAZ_MMU_TILE0);

	/*
	 * MMU Page size = 12
	 * MMU best count = 7
	 * MMU ADT TTE = 0
	 * MMU TTE threshold = 12
	 */
	EMGD_WRITE32(0xc070000c, mmio + TNC_TOPAZ_MMU_CONTROL1);
	EMGD_WRITE32(0x00, mmio + TNC_TOPAZ_MMU_CONTROL0);

	/* Set index register, all pointing to directory bank 0, Flush the directory
	 * cache and disable the MMU bypass */
	EMGD_WRITE32(0, mmio + TNC_TOPAZ_MMU_BANK_INDEX);
	ctrl = EMGD_READ32(mmio + TNC_TOPAZ_MMU_CONTROL0) | 0x0C; /* Flush */
	EMGD_WRITE32(ctrl, mmio + TNC_TOPAZ_MMU_CONTROL0);
	EMGD_WRITE32(0x00, mmio + TNC_TOPAZ_MMU_CONTROL0);

	core_id  = EMGD_READ32(mmio + TNC_TOPAZ_IMG_TOPAZ_CORE_ID);
	core_rev = EMGD_READ32(mmio + TNC_TOPAZ_IMG_TOPAZ_CORE_REV);

	printk(KERN_INFO "Topaz Core Id (%lx)", core_id);
	printk(KERN_INFO "Topaz Core Revision (%lx)", core_rev);

	topaz_flush_tnc(context);

	return 0;
}


static int upload_firmware(igd_context_t *context, enc_fw_info_t *fw)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long start_addr;

	EMGD_DEBUG("Encode Firmware version is %s", fw->fw_version);
	EMGD_DEBUG("Encode Firmware enum is %d", fw->idx);

	/* # refer HLD document */
	/* # MTX reset - MTX_SOFT_RESET_MTX_RESET_MASK */
	EMGD_WRITE32(1, mmio + TNC_TOPAZ_MTX_SOFT_RESET);
	OS_SLEEP(6000);

	/* topaz_mtx_upload_by_register */
	EMGD_DEBUG("Writing firmware text to core memory");
	start_addr = PC_START_ADDRESS - MTX_CODE_BASE;
	write_firmware(context, start_addr,
			*(fw->text_size), fw->text, MTX_CORE_CODE_MEM);
	/* topaz_mtx_upload_by_register */
	EMGD_DEBUG("Writing firmware data to core memory");
	start_addr = *(fw->data_offset) - MTX_DATA_BASE;
	write_firmware(context, start_addr,
			*(fw->data_size), fw->data, MTX_CORE_DATA_MEM);
	OS_SLEEP(6000);
	return 0;
}

/* topaz_mtx_upload_by_register() */
static int write_firmware(igd_context_t *context,
		unsigned long address,
		unsigned long size,
		unsigned long *data,
		unsigned long mtx_mem)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long ram_bank;
	unsigned long bank_size;
	unsigned long current_bank;
	unsigned long ram_id;
	unsigned long ctrl;
	unsigned long i;

	get_mtx_control_from_dash(context);

	ram_bank = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);
	ram_bank = 0x0a0a0606;
	bank_size = (ram_bank & 0x000f0000) >> 16;
	bank_size = (1 << (bank_size + 2));

	/* Loop writing text/code to core memory */
	current_bank = ~0L;

	for (i = 0; i < size; i++) {
		/* Wait for MCMSTAT to become be idle 1 */
		if (reg_ready_tnc(context, TNC_TOPAZ_MTX_RAM_ACCESS_STATUS,
					0x00000001, 0x00000001) == 0) {
			ram_id = mtx_mem + (address / bank_size);
			if (ram_id != current_bank) {
				/*
				 * bits 20:27    - ram bank (CODE_BASE | DATA_BASE)
				 * bits  2:19    - address
				 * bit   1       - enable auto increment addressing mode
				 */
				ctrl = (ram_id << 20) | (((address >> 2) & 0x000ffffc) << 2) |
					0x02;
				EMGD_WRITE32(ctrl, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);

				current_bank = ram_id;
			}

			address +=  4;
			EMGD_WRITE32(data[i],
					mmio + TNC_TOPAZ_MTX_RAM_ACCESS_DATA_TRANSFER);
		} else {
			/* FIXME: Should we return something here */
			printk(KERN_ERR "Timeout waiting for MCMSTAT to be idle");
		}
	}
	release_mtx_control_from_dash(context);
	return 0;
}

static int reg_ready_tnc(igd_context_t *context,
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
	EMGD_DEBUG(KERN_ERR "TIMEOUT: Got 0x%08lx while waiting for 0x%08lx", status, value);
	return 1;
}

unsigned long read_mtx_mem(igd_context_t *context, unsigned long addr)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long bank_size;
	unsigned long ram_size;
	unsigned long ram_id;
	unsigned long reg;
	unsigned long ctrl = 0;
	unsigned long val = 0;

	reg = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);

	reg = 0x0a0a0606;
	bank_size = (reg & 0xf0000) >> 16;

	ram_size = (unsigned long) (1 << (bank_size + 2));
	ram_id = (addr - MTX_DATA_BASE) / ram_size;

	/* cmd id */
	ctrl = ((0x18 + ram_id) << 20) & 0x0ff00000;
	/* Address to read */
	ctrl |= ((addr >> 2) << 2) & 0x000ffffc;
	ctrl |= 0x00000001;

	EMGD_WRITE32(ctrl, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);
	reg_ready_tnc(context, TNC_TOPAZ_MTX_RAM_ACCESS_STATUS, 1, 1);
	val = EMGD_READ32(mmio + TNC_TOPAZ_MTX_RAM_ACCESS_DATA_TRANSFER);

	return val;
}


void write_mtx_mem(igd_context_t *context,
		unsigned long addr, unsigned long value)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long bank_size;
	unsigned long ram_size;
	unsigned long ram_id;
	unsigned long reg;
	unsigned long ctrl = 0;

	reg = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);
	/*printk(KERN_INFO "write_mtx_mem: MTX_DEBUG: 0x%08x", reg);*/

	reg = 0x0a0a0606;
	bank_size = (reg & 0xf0000) >> 16;

	ram_size = (unsigned long) (1 << (bank_size + 2));
	ram_id = (addr - MTX_DATA_BASE) / ram_size;

	/* cmd id */
	ctrl = ((0x18 + ram_id) << 20) & 0x0ff00000;
	/* Address to read */
	ctrl |= ((addr >> 2) << 2) & 0x000ffffc;

	/*printk(KERN_INFO "write_mtx_mem: ctrl=0x%08x, addr=0x%08x, value=0x%08x", ctrl, addr, value);*/

	EMGD_WRITE32(ctrl, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);
	EMGD_WRITE32(value, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_DATA_TRANSFER);
	reg_ready_tnc(context, TNC_TOPAZ_MTX_RAM_ACCESS_STATUS, 1, 1);

	return;
}

void topaz_write_core_reg(igd_context_t *context,
		unsigned long reg, unsigned long val)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long tmp;

	get_mtx_control_from_dash(context);

	/* put data into MTX_RW_DATA */
	EMGD_WRITE32(val, mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_DATA);

	/* request a write */
	tmp = reg &
		~TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK;
	EMGD_WRITE32(tmp, mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST);

	/* wait for operation finished */
	reg_ready_tnc(context,
		TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST,
		TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK,
		TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK);

	release_mtx_control_from_dash(context);
}

void topaz_read_core_reg(igd_context_t *context,
		unsigned long reg, unsigned long *ret_val)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long tmp;

	get_mtx_control_from_dash(context);

	/* request a write */
	tmp = reg &
		~TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK;
	EMGD_WRITE32(tmp | TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_RNW_MASK,
		mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST);

	/* wait for operation finished */
	reg_ready_tnc(context,
		(unsigned long )(mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST),
		0, TNC_TOPAZ_MTX_REGISTER_READ_WRITE_REQUEST_MTX_DREADY_MASK);

	/* read */
	*ret_val = EMGD_READ32(mmio + TNC_TOPAZ_MTX_REGISTER_READ_WRITE_DATA);

	release_mtx_control_from_dash(context);
}

void get_mtx_control_from_dash(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	int debug_reg_slave_val;
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;
	unsigned long reg;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	reg = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);
	reg &= 0xFFFFFFF0;
	reg |= 0x00000006;

	/*printk(KERN_INFO "get_mtx:debug value = 0x%08x", reg);*/

	/* Get MTX control from dash
	 * 	TOPAZ_CR_MTX_DBG_IS_SLAVE |
	 * 	TOPAZ_CR_MTX_DBG_GPIO_OUT
	 */
	EMGD_WRITE32(reg, mmio + TNC_TOPAZ_MTX_DEBUG);

	do {
		debug_reg_slave_val = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);
	} while ((debug_reg_slave_val & 0x18) !=0 );

	topaz_priv->topaz_dash_access_ctrl = EMGD_READ32(mmio +
				TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);
}

void release_mtx_control_from_dash(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;
	unsigned long reg;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	reg = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);
	reg &= 0xFFFFFFF0;
	reg |= 0x00000004;

	/*printk(KERN_INFO "release:debug value = 0x%08x", reg);*/
	/* restore access control */
	EMGD_WRITE32(topaz_priv->topaz_dash_access_ctrl,
			mmio + TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);

	/* release bus
	 * 	TOPAZ_CR_MTX_DBG_IS_SLAVE
	 */
	EMGD_WRITE32(reg, mmio + TNC_TOPAZ_MTX_DEBUG);
}

int process_video_encode_tnc(igd_context_t *context, unsigned long offset,
		void *mem_handle, unsigned long *fence_id)
{
    unsigned long *mtx_buf;
	unsigned long size=0;
	int ret = 0;
	platform_context_plb_t *platform;
	tnc_topaz_priv_t *topaz_priv;
	PVRSRV_ERROR err;
	PVRSRV_PER_PROCESS_DATA *ps_data = NULL;
	IMG_UINT32 pid = 0;
	PVRSRV_KERNEL_MEM_INFO *mem_info_mtx_buf = NULL;

	EMGD_TRACE_ENTER;

	if(!mem_handle || !fence_id) {
		return -EINVAL;
	}

	pid = OSGetCurrentProcessIDKM();
	ps_data = PVRSRVPerProcessData(pid);
	if (!ps_data) {
		printk(KERN_ERR "TOPAZ: Cannot get process data information");
		return -1;
	}

	err = PVRSRVLookupHandle(ps_data->psHandleBase, (void **)&mem_info_mtx_buf,
			(IMG_HANDLE)mem_handle, PVRSRV_HANDLE_TYPE_MEM_INFO);

	if(err != PVRSRV_OK) {
		printk(KERN_ERR "TOPAZ: Cannot get mtx buf memory context from process data");
		return -1;
	}

	if(!mem_info_mtx_buf ||  mem_info_mtx_buf->ui32AllocSize < 0x3000) {
		printk(KERN_ERR "TOPAZ: invalid mtx buf memory context from process data");
		return -1;
	}

	platform = (platform_context_plb_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;
	HostAquireMutex(g_pMutex);
	mtx_buf = (unsigned long *) mem_info_mtx_buf->pvLinAddrKM;

	EMGD_DEBUG("process_video_encode_tnc where buf=%p, offset=%lx\n",
		mtx_buf, offset);

	if (!platform->topaz_busy)
	{
	platform->topaz_busy = 1;
	ret = process_encode_mtx_messages(context, mtx_buf, size);
	if (ret){
		printk(KERN_INFO "Invalid topaz encode cmd");
		ret = -EINVAL;
    }
		platform->topaz_busy = 0;
	}


	*fence_id = topaz_priv->topaz_sync_id;

	HostReleaseMutex(g_pMutex);
	return ret;
}

int topaz_flush_tnc(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long topaz_mmu;

	/* Only support Atom E6xx */
	if ((PCI_DEVICE_ID_VGA_TNC != context->device_context.did)||
	   (context->device_context.bid == PCI_DEVICE_ID_BRIDGE_TNC_ULP)) {
		return -1;
	}

	topaz_mmu = EMGD_READ32(mmio + TNC_TOPAZ_MMU_CONTROL0);
	topaz_mmu &= ~0x800; /* turn off MMU bypass mode if is on*/
	topaz_mmu |= 0x0C;      /* MMU_INVALDC + MMU_FLUSH */
	EMGD_WRITE32(topaz_mmu, mmio + TNC_TOPAZ_MMU_CONTROL0);

	topaz_mmu &= ~0x0C;
	EMGD_WRITE32(topaz_mmu, mmio + TNC_TOPAZ_MMU_CONTROL0);

#if 0
	unsigned long reg;

	topaz_mmu = EMGD_READ32(mmio + TNC_TOPAZ_MMU_CONTROL0);
	printk(KERN_INFO "topaz_flush_tnc: CONTROL0=0x%08x", topaz_mmu);
	reg = EMGD_READ32(mmio+TNC_TOPAZ_IMG_TOPAZ_INTSTAT);
	printk( KERN_INFO "topaz_flush_tnc: TOPAZ_INTSTAT= 0x%08x ", reg);
	reg = EMGD_READ32(mmio+TNC_TOPAZ_MMU_STATUS);
	printk( KERN_INFO "topaz_flush_tnc: MMU_STATUS= 0x%08x ", reg);
#endif

	return 0;
}

int topaz_get_frame_skip(igd_context_t *context, unsigned long *frame_skip)
{
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;

	if(!frame_skip) {
		return -EINVAL;
	}

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;
	*frame_skip = topaz_priv->topaz_frame_skip;
	/* printk(KERN_INFO "call frame skip with return %ld", *frame_skip);*/

	return 0;
}

int topaz_get_fence_id(igd_context_t *context, unsigned long *fence_id)
{
	tnc_topaz_priv_t *topaz_priv;
	unsigned long *sync_p;
	platform_context_tnc_t *platform;

	if(!fence_id) {
		return -EINVAL;
	}

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;
	sync_p = (unsigned long *)topaz_priv->topaz_sync_addr;

	*fence_id = *sync_p;

	return 0;
}

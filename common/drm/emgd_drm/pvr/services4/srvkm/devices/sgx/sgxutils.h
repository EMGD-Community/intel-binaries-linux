/**********************************************************************
 Copyright (c) Imagination Technologies Ltd.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ******************************************************************************/

#include "perproc.h"
#include "sgxinfokm.h"

#define CCB_OFFSET_IS_VALID(type, psCCBMemInfo, psCCBKick, offset) \
	((sizeof(type) <= (psCCBMemInfo)->ui32AllocSize) && \
	((psCCBKick)->offset <= (psCCBMemInfo)->ui32AllocSize - sizeof(type)))

#define	CCB_DATA_FROM_OFFSET(type, psCCBMemInfo, psCCBKick, offset) \
	((type *)(((IMG_CHAR *)(psCCBMemInfo)->pvLinAddrKM) + \
		(psCCBKick)->offset))


IMG_IMPORT
IMG_VOID SGXTestActivePowerEvent(PVRSRV_DEVICE_NODE	*psDeviceNode,
								 IMG_UINT32			ui32CallerID);

IMG_IMPORT
PVRSRV_ERROR SGXScheduleCCBCommand(PVRSRV_SGXDEV_INFO 	*psDevInfo,
								   SGXMKIF_CMD_TYPE		eCommandType,
								   SGXMKIF_COMMAND		*psCommandData,
								   IMG_UINT32			ui32CallerID,
								   IMG_UINT32			ui32PDumpFlags);
IMG_IMPORT
PVRSRV_ERROR SGXScheduleCCBCommandKM(PVRSRV_DEVICE_NODE		*psDeviceNode,
									 SGXMKIF_CMD_TYPE		eCommandType,
									 SGXMKIF_COMMAND		*psCommandData,
									 IMG_UINT32				ui32CallerID,
									 IMG_UINT32				ui32PDumpFlags);

IMG_IMPORT
PVRSRV_ERROR SGXScheduleProcessQueuesKM(PVRSRV_DEVICE_NODE *psDeviceNode);

IMG_IMPORT
IMG_BOOL SGXIsDevicePowered(PVRSRV_DEVICE_NODE *psDeviceNode);

IMG_IMPORT
IMG_HANDLE SGXRegisterHWRenderContextKM(IMG_HANDLE				psDeviceNode,
										IMG_DEV_VIRTADDR		*psHWRenderContextDevVAddr,
										PVRSRV_PER_PROCESS_DATA *psPerProc);

IMG_IMPORT
IMG_HANDLE SGXRegisterHWTransferContextKM(IMG_HANDLE				psDeviceNode,
										  IMG_DEV_VIRTADDR			*psHWTransferContextDevVAddr,
										  PVRSRV_PER_PROCESS_DATA	*psPerProc);

IMG_IMPORT
IMG_VOID SGXFlushHWRenderTargetKM(IMG_HANDLE psSGXDevInfo, IMG_DEV_VIRTADDR psHWRTDataSetDevVAddr);

IMG_IMPORT
PVRSRV_ERROR SGXUnregisterHWRenderContextKM(IMG_HANDLE hHWRenderContext);

IMG_IMPORT
PVRSRV_ERROR SGXUnregisterHWTransferContextKM(IMG_HANDLE hHWTransferContext);

#if defined(SGX_FEATURE_2D_HARDWARE)
IMG_IMPORT
IMG_HANDLE SGXRegisterHW2DContextKM(IMG_HANDLE				psDeviceNode,
									IMG_DEV_VIRTADDR		*psHW2DContextDevVAddr,
									PVRSRV_PER_PROCESS_DATA *psPerProc);

IMG_IMPORT
PVRSRV_ERROR SGXUnregisterHW2DContextKM(IMG_HANDLE hHW2DContext);
#endif

IMG_UINT32 SGXConvertTimeStamp(PVRSRV_SGXDEV_INFO	*psDevInfo,
							   IMG_UINT32			ui32TimeWraps,
							   IMG_UINT32			ui32Time);

IMG_VOID SGXCleanupRequest(PVRSRV_DEVICE_NODE	*psDeviceNode,
							IMG_DEV_VIRTADDR	*psHWDataDevVAddr,
							IMG_UINT32			ui32CleanupType);



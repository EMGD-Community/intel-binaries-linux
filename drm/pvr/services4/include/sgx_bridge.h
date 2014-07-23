/*************************************************************************/ /*!
@Title          SGX Bridge Functionality
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Header for the sgx Brdige code
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#if !defined(__SGX_BRIDGE_H__)
#define __SGX_BRIDGE_H__

#include "sgxapi_km.h"
#include "sgxinfo.h"
#include "pvr_bridge.h"

#if defined (__cplusplus)
extern "C" {
#endif


/* 
 * Bridge Cmd Ids
 */

/* *REMEMBER* to update PVRSRV_BRIDGE_LAST_SGX_CMD if you add/remove a command! 
 * Also you need to ensure all PVRSRV_BRIDGE_SGX_CMD_BASE+ offsets are sequential!
 */

#define PVRSRV_BRIDGE_SGX_CMD_BASE (PVRSRV_BRIDGE_LAST_NON_DEVICE_CMD+1)
#define PVRSRV_BRIDGE_SGX_GETCLIENTINFO			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+0)
#define PVRSRV_BRIDGE_SGX_RELEASECLIENTINFO		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+1)
#define PVRSRV_BRIDGE_SGX_GETINTERNALDEVINFO	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+2)
#define PVRSRV_BRIDGE_SGX_DOKICK				PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+3)
#define PVRSRV_BRIDGE_SGX_GETPHYSPAGEADDR		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+4)
#define PVRSRV_BRIDGE_SGX_READREGISTRYDWORD		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+5)

#define PVRSRV_BRIDGE_SGX_2DQUERYBLTSCOMPLETE   PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+9)

#define PVRSRV_BRIDGE_SGX_GETMMUPDADDR  		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+10)

#if defined(TRANSFER_QUEUE)
#define PVRSRV_BRIDGE_SGX_SUBMITTRANSFER			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+13)
#endif
#define PVRSRV_BRIDGE_SGX_GETMISCINFO				PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+14)
#define PVRSRV_BRIDGE_SGXINFO_FOR_SRVINIT			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+15)
#define PVRSRV_BRIDGE_SGX_DEVINITPART2				PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+16)

#define PVRSRV_BRIDGE_SGX_FINDSHAREDPBDESC			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+17)
#define PVRSRV_BRIDGE_SGX_UNREFSHAREDPBDESC			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+18)
#define PVRSRV_BRIDGE_SGX_ADDSHAREDPBDESC			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+19)
#define PVRSRV_BRIDGE_SGX_REGISTER_HW_RENDER_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+20)
#define PVRSRV_BRIDGE_SGX_FLUSH_HW_RENDER_TARGET	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+21)
#define PVRSRV_BRIDGE_SGX_UNREGISTER_HW_RENDER_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+22)
#if defined(SGX_FEATURE_2D_HARDWARE)
#define PVRSRV_BRIDGE_SGX_SUBMIT2D					PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+23)
#define PVRSRV_BRIDGE_SGX_REGISTER_HW_2D_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+24)
#define PVRSRV_BRIDGE_SGX_UNREGISTER_HW_2D_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+25)
#endif
#define PVRSRV_BRIDGE_SGX_REGISTER_HW_TRANSFER_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+26)
#define PVRSRV_BRIDGE_SGX_UNREGISTER_HW_TRANSFER_CONTEXT	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+27)

#define PVRSRV_BRIDGE_SGX_SCHEDULE_PROCESS_QUEUES		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+28)

#if defined(SUPPORT_SGX_HWPERF)
#define PVRSRV_BRIDGE_SGX_READ_DIFF_COUNTERS		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+29)
#define PVRSRV_BRIDGE_SGX_READ_HWPERF_CB			PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+30)
#endif

#if defined(PDUMP)
#define PVRSRV_BRIDGE_SGX_PDUMP_BUFFER_ARRAY		PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+31)
#define PVRSRV_BRIDGE_SGX_PDUMP_3D_SIGNATURE_REGISTERS	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+32)
#define PVRSRV_BRIDGE_SGX_PDUMP_COUNTER_REGISTERS	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+33)
#define PVRSRV_BRIDGE_SGX_PDUMP_TA_SIGNATURE_REGISTERS	PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+34)
#define PVRSRV_BRIDGE_SGX_PDUMP_HWPERFCB				PVRSRV_IOWR(PVRSRV_BRIDGE_SGX_CMD_BASE+35)
#endif



#define PVRSRV_BRIDGE_LAST_SGX_CMD (PVRSRV_BRIDGE_SGX_CMD_BASE+35)

/*****************************************************************************
 * Input structures for IOCTL/DRVESC 
 *****************************************************************************/

/*!
 *****************************************************************************
 *	`bridge in' SGX Get Phys Page Addr
 *****************************************************************************/ 
typedef struct PVRSRV_BRIDGE_IN_GETPHYSPAGEADDR
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevMemHeap;
	IMG_DEV_VIRTADDR sDevVAddr;
}PVRSRV_BRIDGE_IN_GETPHYSPAGEADDR;


typedef struct PVRSRV_BRIDGE_OUT_GETPHYSPAGEADDR
{
	PVRSRV_ERROR		eError;
	IMG_DEV_PHYADDR		DevPAddr;
	IMG_CPU_PHYADDR		CpuPAddr;
}PVRSRV_BRIDGE_OUT_GETPHYSPAGEADDR;


typedef struct PVRSRV_BRIDGE_IN_SGX_GETMMU_PDADDR_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	IMG_HANDLE				hDevMemContext;
}PVRSRV_BRIDGE_IN_SGX_GETMMU_PDADDR;


typedef struct PVRSRV_BRIDGE_OUT_SGX_GETMMU_PDADDR_TAG
{
	IMG_DEV_PHYADDR 		sPDDevPAddr;
	PVRSRV_ERROR			eError;
}PVRSRV_BRIDGE_OUT_SGX_GETMMU_PDADDR;


typedef struct PVRSRV_BRIDGE_IN_GETCLIENTINFO_TAG
{
	IMG_UINT32					ui32BridgeFlags;
	IMG_HANDLE					hDevCookie;
}PVRSRV_BRIDGE_IN_GETCLIENTINFO;


typedef struct PVRSRV_BRIDGE_OUT_GETINTERNALDEVINFO_TAG
{
	SGX_INTERNAL_DEVINFO	sSGXInternalDevInfo;
	PVRSRV_ERROR				eError;
}PVRSRV_BRIDGE_OUT_GETINTERNALDEVINFO;


typedef struct PVRSRV_BRIDGE_IN_GETINTERNALDEVINFO_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
}PVRSRV_BRIDGE_IN_GETINTERNALDEVINFO;


typedef struct PVRSRV_BRIDGE_OUT_GETCLIENTINFO_TAG
{
	SGX_CLIENT_INFO  	sClientInfo;
	PVRSRV_ERROR			eError;
}PVRSRV_BRIDGE_OUT_GETCLIENTINFO;


typedef struct PVRSRV_BRIDGE_IN_RELEASECLIENTINFO_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	SGX_CLIENT_INFO  	sClientInfo;
}PVRSRV_BRIDGE_IN_RELEASECLIENTINFO;


typedef struct PVRSRV_BRIDGE_IN_ISPBREAKPOLL_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
}PVRSRV_BRIDGE_IN_ISPBREAKPOLL;


typedef struct PVRSRV_BRIDGE_IN_DOKICK_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	SGX_CCB_KICK			sCCBKick;
}PVRSRV_BRIDGE_IN_DOKICK;


typedef struct PVRSRV_BRIDGE_IN_SGX_SCHEDULE_PROCESS_QUEUES_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
}PVRSRV_BRIDGE_IN_SGX_SCHEDULE_PROCESS_QUEUES;


#if defined(TRANSFER_QUEUE)

typedef struct PVRSRV_BRIDGE_IN_SUBMITTRANSFER_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	PVRSRV_TRANSFER_SGX_KICK 			sKick;
}PVRSRV_BRIDGE_IN_SUBMITTRANSFER;

#if defined(SGX_FEATURE_2D_HARDWARE)

typedef struct PVRSRV_BRIDGE_IN_SUBMIT2D_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	PVRSRV_2D_SGX_KICK 				sKick;
} PVRSRV_BRIDGE_IN_SUBMIT2D;
#endif
#endif


typedef struct PVRSRV_BRIDGE_IN_READREGDWORD_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
    IMG_PCHAR				pszKey;
    IMG_PCHAR				pszValue;
}PVRSRV_BRIDGE_IN_READREGDWORD;


typedef struct PVRSRV_BRIDGE_OUT_READREGDWORD_TAG
{
	PVRSRV_ERROR	eError;
	IMG_UINT32		ui32Data;
}PVRSRV_BRIDGE_OUT_READREGDWORD;


/*!
 *****************************************************************************
 *	`bridge in' SGXGetMiscInfo
 *****************************************************************************/
typedef struct PVRSRV_BRIDGE_IN_SGXGETMISCINFO_TAG
{
	IMG_UINT32		ui32BridgeFlags;
	IMG_HANDLE		hDevCookie;
	SGX_MISC_INFO	*psMiscInfo;
}PVRSRV_BRIDGE_IN_SGXGETMISCINFO;

/*!
 *****************************************************************************
 *	`bridge in' SGXGetInfoForSrvInit
 *****************************************************************************/
typedef struct PVRSRV_BRIDGE_IN_SGXINFO_FOR_SRVINIT_TAG
{
	IMG_UINT32		ui32BridgeFlags;
	IMG_HANDLE		hDevCookie;
}PVRSRV_BRIDGE_IN_SGXINFO_FOR_SRVINIT;

/*!
 *****************************************************************************
 *	`bridge out' SGXGetInfoForSrvInit
 *****************************************************************************/
typedef struct PVRSRV_BRIDGE_OUT_SGXINFO_FOR_SRVINIT_TAG
{
	PVRSRV_ERROR			eError;
	SGX_BRIDGE_INFO_FOR_SRVINIT	sInitInfo;
}PVRSRV_BRIDGE_OUT_SGXINFO_FOR_SRVINIT;

/*!
 *****************************************************************************
 *	`bridge in' SGXDevInitPart2
 *****************************************************************************/
typedef struct PVRSRV_BRIDGE_IN_SGXDEVINITPART2_TAG
{
	IMG_UINT32		ui32BridgeFlags;
	IMG_HANDLE		hDevCookie;
	SGX_BRIDGE_INIT_INFO	sInitInfo;
}PVRSRV_BRIDGE_IN_SGXDEVINITPART2;


typedef struct PVRSRV_BRIDGE_IN_2DQUERYBLTSCOMPLETE_TAG
{
	IMG_UINT32				ui32BridgeFlags;
	IMG_HANDLE				hDevCookie;
	IMG_HANDLE				hKernSyncInfo;
	IMG_BOOL				bWaitForComplete;
}PVRSRV_BRIDGE_IN_2DQUERYBLTSCOMPLETE;


#define PVRSRV_BRIDGE_SGX_SHAREDPBDESC_MAX_SUBMEMINFOS 10

typedef struct PVRSRV_BRIDGE_IN_SGXFINDSHAREDPBDESC_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_BOOL bLockOnFailure;
	IMG_UINT32 ui32TotalPBSize;
}PVRSRV_BRIDGE_IN_SGXFINDSHAREDPBDESC;

typedef struct PVRSRV_BRIDGE_OUT_SGXFINDSHAREDPBDESC_TAG
{
	IMG_HANDLE hKernelMemInfo;
	IMG_HANDLE hSharedPBDesc;
	IMG_HANDLE hSharedPBDescKernelMemInfoHandle;
	IMG_HANDLE hHWPBDescKernelMemInfoHandle;
	IMG_HANDLE hBlockKernelMemInfoHandle;
	IMG_HANDLE hHWBlockKernelMemInfoHandle;
	IMG_HANDLE ahSharedPBDescSubKernelMemInfoHandles[PVRSRV_BRIDGE_SGX_SHAREDPBDESC_MAX_SUBMEMINFOS];
	IMG_UINT32 ui32SharedPBDescSubKernelMemInfoHandlesCount;
	PVRSRV_ERROR eError;
}PVRSRV_BRIDGE_OUT_SGXFINDSHAREDPBDESC;

typedef struct PVRSRV_BRIDGE_IN_SGXUNREFSHAREDPBDESC_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hSharedPBDesc;
}PVRSRV_BRIDGE_IN_SGXUNREFSHAREDPBDESC;

typedef struct PVRSRV_BRIDGE_OUT_SGXUNREFSHAREDPBDESC_TAG
{
	PVRSRV_ERROR eError;
}PVRSRV_BRIDGE_OUT_SGXUNREFSHAREDPBDESC;


typedef struct PVRSRV_BRIDGE_IN_SGXADDSHAREDPBDESC_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_HANDLE hSharedPBDescKernelMemInfo;
	IMG_HANDLE hHWPBDescKernelMemInfo;
	IMG_HANDLE hBlockKernelMemInfo;
	IMG_HANDLE hHWBlockKernelMemInfo;
	IMG_UINT32 ui32TotalPBSize;
	IMG_HANDLE *phKernelMemInfoHandles;
	IMG_UINT32 ui32KernelMemInfoHandlesCount;
}PVRSRV_BRIDGE_IN_SGXADDSHAREDPBDESC;

typedef struct PVRSRV_BRIDGE_OUT_SGXADDSHAREDPBDESC_TAG
{
	PVRSRV_ERROR eError;
	IMG_HANDLE hSharedPBDesc;
}PVRSRV_BRIDGE_OUT_SGXADDSHAREDPBDESC;


#ifdef	PDUMP
typedef struct PVRSRV_BRIDGE_IN_PDUMP_BUFFER_ARRAY_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	SGX_KICKTA_DUMP_BUFFER *psBufferArray;
	IMG_UINT32 ui32BufferArrayLength;
	IMG_BOOL bDumpPolls;
} PVRSRV_BRIDGE_IN_PDUMP_BUFFER_ARRAY;

typedef struct PVRSRV_BRIDGE_IN_PDUMP_3D_SIGNATURE_REGISTERS_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_UINT32 ui32DumpFrameNum;
	IMG_BOOL bLastFrame;
	IMG_UINT32 *pui32Registers;
	IMG_UINT32 ui32NumRegisters;
}PVRSRV_BRIDGE_IN_PDUMP_3D_SIGNATURE_REGISTERS;

typedef struct PVRSRV_BRIDGE_IN_PDUMPCOUNTER_REGISTERS_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_UINT32 ui32DumpFrameNum;
	IMG_BOOL bLastFrame;
	IMG_UINT32 *pui32Registers;
	IMG_UINT32 ui32NumRegisters;
}PVRSRV_BRIDGE_IN_PDUMP_COUNTER_REGISTERS;

typedef struct PVRSRV_BRIDGE_IN_PDUMP_TA_SIGNATURE_REGISTERS_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_UINT32 ui32DumpFrameNum;
	IMG_UINT32 ui32TAKickCount;
	IMG_BOOL bLastFrame;
	IMG_UINT32 *pui32Registers;
	IMG_UINT32 ui32NumRegisters;
}PVRSRV_BRIDGE_IN_PDUMP_TA_SIGNATURE_REGISTERS;

typedef struct PVRSRV_BRIDGE_IN_PDUMP_HWPERFCB_TAG
{
	IMG_UINT32			ui32BridgeFlags;
	IMG_HANDLE			hDevCookie;
	IMG_CHAR			szFileName[PVRSRV_PDUMP_MAX_FILENAME_SIZE];
	IMG_UINT32			ui32FileOffset;
	IMG_UINT32			ui32PDumpFlags;

}PVRSRV_BRIDGE_IN_PDUMP_HWPERFCB;

#endif

typedef struct PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_RENDER_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_DEV_VIRTADDR sHWRenderContextDevVAddr;
}PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_RENDER_CONTEXT;

typedef struct PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_RENDER_CONTEXT_TAG
{
	PVRSRV_ERROR eError;
	IMG_HANDLE hHWRenderContext;
}PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_RENDER_CONTEXT;

typedef struct PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_RENDER_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_HANDLE hHWRenderContext;
}PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_RENDER_CONTEXT;

typedef struct PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_TRANSFER_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_DEV_VIRTADDR sHWTransferContextDevVAddr;
}PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_TRANSFER_CONTEXT;

typedef struct PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_TRANSFER_CONTEXT_TAG
{
	PVRSRV_ERROR eError;
	IMG_HANDLE hHWTransferContext;
}PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_TRANSFER_CONTEXT;

typedef struct PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_TRANSFER_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_HANDLE hHWTransferContext;
}PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_TRANSFER_CONTEXT;

typedef struct PVRSRV_BRIDGE_IN_SGX_FLUSH_HW_RENDER_TARGET_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_DEV_VIRTADDR sHWRTDataSetDevVAddr;
}PVRSRV_BRIDGE_IN_SGX_FLUSH_HW_RENDER_TARGET;


#if defined(SGX_FEATURE_2D_HARDWARE)
typedef struct PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_2D_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_DEV_VIRTADDR sHW2DContextDevVAddr;
}PVRSRV_BRIDGE_IN_SGX_REGISTER_HW_2D_CONTEXT;

typedef struct PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_2D_CONTEXT_TAG
{
	PVRSRV_ERROR eError;
	IMG_HANDLE hHW2DContext;
}PVRSRV_BRIDGE_OUT_SGX_REGISTER_HW_2D_CONTEXT;

typedef struct PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_2D_CONTEXT_TAG
{
	IMG_UINT32 ui32BridgeFlags;
	IMG_HANDLE hDevCookie;
	IMG_HANDLE hHW2DContext;
}PVRSRV_BRIDGE_IN_SGX_UNREGISTER_HW_2D_CONTEXT;

#define	SGX2D_MAX_BLT_CMD_SIZ		256
#endif


typedef struct PVRSRV_BRIDGE_IN_SGX_READ_DIFF_COUNTERS_TAG
{
	IMG_UINT32		ui32BridgeFlags;
	IMG_HANDLE		hDevCookie;
	IMG_UINT32		ui32Reg;
	IMG_BOOL		bNew;
	IMG_UINT32		ui32New;
	IMG_UINT32		ui32NewReset;
	IMG_UINT32		ui32CountersReg;
	IMG_UINT32		ui32Reg2;
} PVRSRV_BRIDGE_IN_SGX_READ_DIFF_COUNTERS;

typedef struct PVRSRV_BRIDGE_OUT_SGX_READ_DIFF_COUNTERS_TAG
{
	PVRSRV_ERROR				eError;
	IMG_UINT32					ui32Old;
	IMG_BOOL					bActive;
	PVRSRV_SGXDEV_DIFF_INFO		sDiffs;
} PVRSRV_BRIDGE_OUT_SGX_READ_DIFF_COUNTERS;


#if defined(SUPPORT_SGX_HWPERF)
typedef struct PVRSRV_BRIDGE_IN_SGX_READ_HWPERF_CB_TAG
{
	IMG_UINT32					ui32BridgeFlags;
	IMG_HANDLE					hDevCookie;
	IMG_UINT32					ui32ArraySize;
	PVRSRV_SGX_HWPERF_CB_ENTRY	*psHWPerfCBData;
} PVRSRV_BRIDGE_IN_SGX_READ_HWPERF_CB;

/*!
 *****************************************************************************
 *	`bridge out' SGXReadHWPerfCB
 *****************************************************************************/
typedef struct PVRSRV_BRIDGE_OUT_SGX_READ_HWPERF_CB_TAG
{
	PVRSRV_ERROR		eError;
	IMG_UINT32			ui32DataCount;
	IMG_UINT32			ui32ClockSpeed;
	IMG_UINT32			ui32HostTimeStamp;
} PVRSRV_BRIDGE_OUT_SGX_READ_HWPERF_CB;
#endif

#if defined (__cplusplus)
}
#endif

#endif


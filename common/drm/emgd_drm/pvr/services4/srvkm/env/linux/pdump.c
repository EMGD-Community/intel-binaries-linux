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

#if defined (SUPPORT_SGX)
#if defined (PDUMP)

#include <asm/atomic.h>
#include <stdarg.h>
#include "sgxdefs.h"
#include "services_headers.h"

#include "pvrversion.h"
#include "pvr_debug.h"

#include "dbgdrvif.h"
#include "sgxmmu.h"
#include "mm.h"
#include "pdump_km.h"

#include <linux/tty.h>

static IMG_BOOL PDumpWriteString2		(IMG_CHAR * pszString, IMG_UINT32 ui32Flags);
static IMG_BOOL PDumpWriteILock			(PDBG_STREAM psStream, IMG_UINT8 *pui8Data, IMG_UINT32 ui32Count, IMG_UINT32 ui32Flags);
static IMG_VOID DbgSetFrame				(PDBG_STREAM psStream, IMG_UINT32 ui32Frame);
static IMG_UINT32 DbgGetFrame			(PDBG_STREAM psStream);
static IMG_VOID DbgSetMarker			(PDBG_STREAM psStream, IMG_UINT32 ui32Marker);
static IMG_UINT32 DbgWrite				(PDBG_STREAM psStream, IMG_UINT8 *pui8Data, IMG_UINT32 ui32BCount, IMG_UINT32 ui32Flags);

#define PDUMP_DATAMASTER_PIXEL		(1)
#define PDUMP_DATAMASTER_EDM		(3)

#define MIN(a,b)       (a > b ? b : a)

#define MAX_FILE_SIZE	0x40000000

static atomic_t gsPDumpSuspended = ATOMIC_INIT(0);

static PDBGKM_SERVICE_TABLE gpfnDbgDrv = IMG_NULL;



IMG_CHAR *pszStreamName[PDUMP_NUM_STREAMS] = {	"ParamStream2",
												"ScriptStream2",
												"DriverInfoStream"};
typedef struct PDBG_PDUMP_STATE_TAG
{
	PDBG_STREAM psStream[PDUMP_NUM_STREAMS];
	IMG_UINT32 ui32ParamFileNum;

	IMG_CHAR *pszMsg;
	IMG_CHAR *pszScript;
	IMG_CHAR *pszFile;

} PDBG_PDUMP_STATE;

static PDBG_PDUMP_STATE gsDBGPdumpState = {{IMG_NULL}, 0, IMG_NULL, IMG_NULL, IMG_NULL};

#define SZ_MSG_SIZE_MAX			PVRSRV_PDUMP_MAX_COMMENT_SIZE-1
#define SZ_SCRIPT_SIZE_MAX		PVRSRV_PDUMP_MAX_COMMENT_SIZE-1
#define SZ_FILENAME_SIZE_MAX	PVRSRV_PDUMP_MAX_COMMENT_SIZE-1




IMG_VOID DBGDrvGetServiceTable(IMG_VOID **fn_table);

static inline IMG_BOOL PDumpSuspended(IMG_VOID)
{
	return atomic_read(&gsPDumpSuspended) != 0;
}

PVRSRV_ERROR PDumpOSGetScriptString(IMG_HANDLE *phScript,
									IMG_UINT32 *pui32MaxLen)
{
	*phScript = (IMG_HANDLE)gsDBGPdumpState.pszScript;
	*pui32MaxLen = SZ_SCRIPT_SIZE_MAX;
	if ((!*phScript) || PDumpSuspended())
	{
		return PVRSRV_ERROR_GENERIC;
	}
	return PVRSRV_OK;
}

PVRSRV_ERROR PDumpOSGetMessageString(IMG_HANDLE *phMsg,
									 IMG_UINT32 *pui32MaxLen)
{
	*phMsg = (IMG_HANDLE)gsDBGPdumpState.pszMsg;
	*pui32MaxLen = SZ_MSG_SIZE_MAX;
	if ((!*phMsg) || PDumpSuspended())
	{
		return PVRSRV_ERROR_GENERIC;
	}
	return PVRSRV_OK;
}

PVRSRV_ERROR PDumpOSGetFilenameString(IMG_CHAR **ppszFile,
									 IMG_UINT32 *pui32MaxLen)
{
	*ppszFile = gsDBGPdumpState.pszFile;
	*pui32MaxLen = SZ_FILENAME_SIZE_MAX;
	if ((!*ppszFile) || PDumpSuspended())
	{
		return PVRSRV_ERROR_GENERIC;
	}
	return PVRSRV_OK;
}

IMG_BOOL PDumpOSWriteString2(IMG_HANDLE hScript, IMG_UINT32 ui32Flags)
{
	return PDumpWriteString2(hScript, ui32Flags);
}

PVRSRV_ERROR PDumpOSBufprintf(IMG_HANDLE hBuf, IMG_UINT32 ui32ScriptSizeMax, IMG_CHAR* pszFormat, ...)
{
	IMG_CHAR* pszBuf = hBuf;
	IMG_UINT32 n;
	va_list	vaArgs;

	va_start(vaArgs, pszFormat);

	n = vsnprintf(pszBuf, ui32ScriptSizeMax, pszFormat, vaArgs);

	va_end(vaArgs);

	if (n>=ui32ScriptSizeMax || n==-1)
	{
		PVR_DPF((PVR_DBG_ERROR, "Buffer overflow detected, pdump output may be incomplete."));

		return PVRSRV_ERROR_PDUMP_BUF_OVERFLOW;
	}

	return PVRSRV_OK;
}

PVRSRV_ERROR PDumpOSVSprintf(IMG_CHAR *pszComment, IMG_UINT32 ui32ScriptSizeMax, IMG_CHAR* pszFormat, PDUMP_va_list vaArgs)
{
	IMG_UINT32 n;

	n = vsnprintf(pszComment, ui32ScriptSizeMax, pszFormat, vaArgs);

	if (n>=ui32ScriptSizeMax || n==-1)
	{
		PVR_DPF((PVR_DBG_ERROR, "Buffer overflow detected, pdump output may be incomplete."));

		return PVRSRV_ERROR_PDUMP_BUF_OVERFLOW;
	}

	return PVRSRV_OK;
}

IMG_VOID PDumpOSDebugPrintf(IMG_CHAR* pszFormat, ...)
{

}

PVRSRV_ERROR PDumpOSSprintf(IMG_CHAR *pszComment, IMG_UINT32 ui32ScriptSizeMax, IMG_CHAR *pszFormat, ...)
{
	IMG_UINT32 n;
	va_list	vaArgs;

	va_start(vaArgs, pszFormat);

	n = vsnprintf(pszComment, ui32ScriptSizeMax, pszFormat, vaArgs);

	va_end(vaArgs);

	if (n>=ui32ScriptSizeMax || n==-1)
	{
		PVR_DPF((PVR_DBG_ERROR, "Buffer overflow detected, pdump output may be incomplete."));

		return PVRSRV_ERROR_PDUMP_BUF_OVERFLOW;
	}

	return PVRSRV_OK;
}

IMG_UINT32 PDumpOSBuflen(IMG_HANDLE hBuffer, IMG_UINT32 ui32BufferSizeMax)
{
	IMG_CHAR* pszBuf = hBuffer;
	IMG_UINT32 ui32Count = 0;

	while ((pszBuf[ui32Count]!=0) && (ui32Count<ui32BufferSizeMax) )
	{
		ui32Count++;
	}
	return(ui32Count);
}

IMG_VOID PDumpOSVerifyLineEnding(IMG_HANDLE hBuffer, IMG_UINT32 ui32BufferSizeMax)
{
	IMG_UINT32 ui32Count = 0;
	IMG_CHAR* pszBuf = hBuffer;


	ui32Count = PDumpOSBuflen(hBuffer, ui32BufferSizeMax);


	if ((ui32Count >= 1) && (pszBuf[ui32Count-1] != '\n') && (ui32Count<ui32BufferSizeMax))
	{
		pszBuf[ui32Count] = '\n';
		ui32Count++;
		pszBuf[ui32Count] = '\0';
	}
	if ((ui32Count >= 2) && (pszBuf[ui32Count-2] != '\r') && (ui32Count<ui32BufferSizeMax))
	{
		pszBuf[ui32Count-1] = '\r';
		pszBuf[ui32Count] = '\n';
		ui32Count++;
		pszBuf[ui32Count] = '\0';
	}
}

IMG_HANDLE PDumpOSGetStream(IMG_UINT32 ePDumpStream)
{
	return (IMG_HANDLE)gsDBGPdumpState.psStream[ePDumpStream];
}

IMG_UINT32 PDumpOSGetStreamOffset(IMG_UINT32 ePDumpStream)
{
	PDBG_STREAM psStream = gsDBGPdumpState.psStream[ePDumpStream];
	return gpfnDbgDrv->pfnGetStreamOffset(psStream);
}

IMG_UINT32 PDumpOSGetParamFileNum(IMG_VOID)
{
	return gsDBGPdumpState.ui32ParamFileNum;
}

IMG_BOOL PDumpOSWriteString(IMG_HANDLE hStream,
		IMG_UINT8 *psui8Data,
		IMG_UINT32 ui32Size,
		IMG_UINT32 ui32Flags)
{
	PDBG_STREAM psStream = (PDBG_STREAM)hStream;
	return PDumpWriteILock(psStream,
					psui8Data,
					ui32Size,
					ui32Flags);
}

IMG_VOID PDumpOSCheckForSplitting(IMG_HANDLE hStream, IMG_UINT32 ui32Size, IMG_UINT32 ui32Flags)
{

	PVR_UNREFERENCED_PARAMETER(hStream);
	PVR_UNREFERENCED_PARAMETER(ui32Size);
	PVR_UNREFERENCED_PARAMETER(ui32Size);
}

IMG_BOOL PDumpOSJTInitialised(IMG_VOID)
{
	if(gpfnDbgDrv)
	{
		return IMG_TRUE;
	}
	return IMG_FALSE;
}

inline IMG_BOOL PDumpOSIsSuspended(IMG_VOID)
{
	return atomic_read(&gsPDumpSuspended) != 0;
}

IMG_VOID PDumpOSCPUVAddrToDevPAddr(PVRSRV_DEVICE_TYPE eDeviceType,
        IMG_HANDLE hOSMemHandle,
		IMG_UINT32 ui32Offset,
		IMG_UINT8 *pui8LinAddr,
		IMG_UINT32 ui32PageSize,
		IMG_DEV_PHYADDR *psDevPAddr)
{
	if(hOSMemHandle)
	{

		IMG_CPU_PHYADDR	sCpuPAddr;

		PVR_UNREFERENCED_PARAMETER(pui8LinAddr);

		sCpuPAddr = OSMemHandleToCpuPAddr(hOSMemHandle, ui32Offset);
		PVR_ASSERT((sCpuPAddr.uiAddr & (ui32PageSize - 1)) == 0);


		*psDevPAddr = SysCpuPAddrToDevPAddr(eDeviceType, sCpuPAddr);
	}
	else
	{
		IMG_CPU_PHYADDR	sCpuPAddr;

		PVR_UNREFERENCED_PARAMETER(ui32Offset);

		sCpuPAddr = OSMapLinToCPUPhys(pui8LinAddr);
		*psDevPAddr = SysCpuPAddrToDevPAddr(eDeviceType, sCpuPAddr);
	}
}

IMG_VOID PDumpOSCPUVAddrToPhysPages(IMG_HANDLE hOSMemHandle,
		IMG_UINT32 ui32Offset,
		IMG_PUINT8 pui8LinAddr,
		IMG_UINT32 *pui32PageOffset)
{
	if(hOSMemHandle)
	{

		IMG_CPU_PHYADDR     sCpuPAddr;

		PVR_UNREFERENCED_PARAMETER(pui8LinAddr);

		sCpuPAddr = OSMemHandleToCpuPAddr(hOSMemHandle, ui32Offset);
	    *pui32PageOffset = sCpuPAddr.uiAddr & (HOST_PAGESIZE() -1);
	}
	else
	{
		PVR_UNREFERENCED_PARAMETER(hOSMemHandle);
		PVR_UNREFERENCED_PARAMETER(ui32Offset);

		*pui32PageOffset = (IMG_UINT32)pui8LinAddr & (HOST_PAGESIZE() - 1);
	}
}



IMG_VOID PDumpInit(IMG_VOID)
{
	IMG_UINT32 i;


	if (!gpfnDbgDrv)
	{
		DBGDrvGetServiceTable((IMG_VOID **)&gpfnDbgDrv);




		if (gpfnDbgDrv == IMG_NULL)
		{
			return;
		}

		if(!gsDBGPdumpState.pszFile)
		{
			if(OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_FILENAME_SIZE_MAX, (IMG_PVOID *)&gsDBGPdumpState.pszFile, 0,
				"Filename string") != PVRSRV_OK)
			{
				goto init_failed;
			}
		}

		if(!gsDBGPdumpState.pszMsg)
		{
			if(OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_MSG_SIZE_MAX, (IMG_PVOID *)&gsDBGPdumpState.pszMsg, 0,
				"Message string") != PVRSRV_OK)
			{
				goto init_failed;
			}
		}

		if(!gsDBGPdumpState.pszScript)
		{
			if(OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_SCRIPT_SIZE_MAX, (IMG_PVOID *)&gsDBGPdumpState.pszScript, 0,
				"Script string") != PVRSRV_OK)
			{
				goto init_failed;
			}
		}

		for(i=0; i < PDUMP_NUM_STREAMS; i++)
		{
			gsDBGPdumpState.psStream[i] = gpfnDbgDrv->pfnCreateStream(pszStreamName[i],
														DEBUG_CAPMODE_FRAMED,
														DEBUG_OUTMODE_STREAMENABLE,
														0,
														10);

			gpfnDbgDrv->pfnSetCaptureMode(gsDBGPdumpState.psStream[i],DEBUG_CAPMODE_FRAMED,0xFFFFFFFF, 0xFFFFFFFF, 1);
			gpfnDbgDrv->pfnSetFrame(gsDBGPdumpState.psStream[i],0);
		}

		PDUMPCOMMENT("Driver Product Name: %s", VS_PRODUCT_NAME);
		PDUMPCOMMENT("Driver Product Version: %s (%s)", PVRVERSION_STRING, PVRVERSION_FILE);
		PDUMPCOMMENT("Start of Init Phase");
	}

	return;

init_failed:

	if(gsDBGPdumpState.pszFile)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_FILENAME_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszFile, 0);
		gsDBGPdumpState.pszFile = IMG_NULL;
	}

	if(gsDBGPdumpState.pszScript)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_SCRIPT_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszScript, 0);
		gsDBGPdumpState.pszScript = IMG_NULL;
	}

	if(gsDBGPdumpState.pszMsg)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_MSG_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszMsg, 0);
		gsDBGPdumpState.pszMsg = IMG_NULL;
	}

	gpfnDbgDrv = IMG_NULL;
}


IMG_VOID PDumpDeInit(IMG_VOID)
{
	IMG_UINT32 i;

	for(i=0; i < PDUMP_NUM_STREAMS; i++)
	{
		gpfnDbgDrv->pfnDestroyStream(gsDBGPdumpState.psStream[i]);
	}

	if(gsDBGPdumpState.pszFile)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_FILENAME_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszFile, 0);
		gsDBGPdumpState.pszFile = IMG_NULL;
	}

	if(gsDBGPdumpState.pszScript)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_SCRIPT_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszScript, 0);
		gsDBGPdumpState.pszScript = IMG_NULL;
	}

	if(gsDBGPdumpState.pszMsg)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, SZ_MSG_SIZE_MAX, (IMG_PVOID) gsDBGPdumpState.pszMsg, 0);
		gsDBGPdumpState.pszMsg = IMG_NULL;
	}

	gpfnDbgDrv = IMG_NULL;
}

PVRSRV_ERROR PDumpStartInitPhaseKM(IMG_VOID)
{
	IMG_UINT32 i;

	if (gpfnDbgDrv)
	{
		PDUMPCOMMENT("Start Init Phase");
		for(i=0; i < PDUMP_NUM_STREAMS; i++)
		{
			gpfnDbgDrv->pfnStartInitPhase(gsDBGPdumpState.psStream[i]);
		}
	}
	return PVRSRV_OK;
}

PVRSRV_ERROR PDumpStopInitPhaseKM(IMG_VOID)
{
	IMG_UINT32 i;

	if (gpfnDbgDrv)
	{
		PDUMPCOMMENT("Stop Init Phase");

		for(i=0; i < PDUMP_NUM_STREAMS; i++)
		{
			gpfnDbgDrv->pfnStopInitPhase(gsDBGPdumpState.psStream[i]);
		}
	}
	return PVRSRV_OK;
}

IMG_BOOL PDumpIsLastCaptureFrameKM(IMG_VOID)
{
	return gpfnDbgDrv->pfnIsLastCaptureFrame(gsDBGPdumpState.psStream[PDUMP_STREAM_SCRIPT2]);
}


IMG_BOOL PDumpIsCaptureFrameKM(IMG_VOID)
{
	if (PDumpSuspended())
	{
		return IMG_FALSE;
	}
	return gpfnDbgDrv->pfnIsCaptureFrame(gsDBGPdumpState.psStream[PDUMP_STREAM_SCRIPT2], IMG_FALSE);
}

PVRSRV_ERROR PDumpSetFrameKM(IMG_UINT32 ui32Frame)
{
	IMG_UINT32	ui32Stream;

	for	(ui32Stream = 0; ui32Stream < PDUMP_NUM_STREAMS; ui32Stream++)
	{
		if	(gsDBGPdumpState.psStream[ui32Stream])
		{
			DbgSetFrame(gsDBGPdumpState.psStream[ui32Stream], ui32Frame);
		}
	}

	return PVRSRV_OK;
}

PVRSRV_ERROR PDumpGetFrameKM(IMG_PUINT32 pui32Frame)
{
	*pui32Frame = DbgGetFrame(gsDBGPdumpState.psStream[PDUMP_STREAM_SCRIPT2]);

	return PVRSRV_OK;
}



static IMG_BOOL PDumpWriteString2(IMG_CHAR * pszString, IMG_UINT32 ui32Flags)
{
	return PDumpWriteILock(gsDBGPdumpState.psStream[PDUMP_STREAM_SCRIPT2], (IMG_UINT8 *) pszString, strlen(pszString), ui32Flags);
}


static IMG_BOOL PDumpWriteILock(PDBG_STREAM psStream, IMG_UINT8 *pui8Data, IMG_UINT32 ui32Count, IMG_UINT32 ui32Flags)
{
	IMG_UINT32 ui32Written = 0;
	IMG_UINT32 ui32Off = 0;

	if ((psStream == IMG_NULL) || PDumpSuspended() || ((ui32Flags & PDUMP_FLAGS_NEVER) != 0))
	{
		return IMG_TRUE;
	}




	if (psStream == gsDBGPdumpState.psStream[PDUMP_STREAM_PARAM2])
	{
		IMG_UINT32 ui32ParamOutPos = gpfnDbgDrv->pfnGetStreamOffset(gsDBGPdumpState.psStream[PDUMP_STREAM_PARAM2]);

		if (ui32ParamOutPos + ui32Count > MAX_FILE_SIZE)
		{
			if ((gsDBGPdumpState.psStream[PDUMP_STREAM_SCRIPT2] && PDumpWriteString2("\r\n-- Splitting pdump output file\r\n\r\n", ui32Flags)))
			{
				DbgSetMarker(gsDBGPdumpState.psStream[PDUMP_STREAM_PARAM2], ui32ParamOutPos);
				gsDBGPdumpState.ui32ParamFileNum++;
			}
		}
	}


	while (((IMG_UINT32) ui32Count > 0) && (ui32Written != 0xFFFFFFFF))
	{
		ui32Written = DbgWrite(psStream, &pui8Data[ui32Off], ui32Count, ui32Flags);




		if (ui32Written == 0)
		{
			OSReleaseThreadQuanta();
		}

		if (ui32Written != 0xFFFFFFFF)
		{
			ui32Off += ui32Written;
			ui32Count -= ui32Written;
		}
	}

	if (ui32Written == 0xFFFFFFFF)
	{
		return IMG_FALSE;
	}

	return IMG_TRUE;
}

static IMG_VOID DbgSetFrame(PDBG_STREAM psStream, IMG_UINT32 ui32Frame)
{
	gpfnDbgDrv->pfnSetFrame(psStream, ui32Frame);
}


static IMG_UINT32 DbgGetFrame(PDBG_STREAM psStream)
{
	return gpfnDbgDrv->pfnGetFrame(psStream);
}

static IMG_VOID DbgSetMarker(PDBG_STREAM psStream, IMG_UINT32 ui32Marker)
{
	gpfnDbgDrv->pfnSetMarker(psStream, ui32Marker);
}

static IMG_UINT32 DbgWrite(PDBG_STREAM psStream, IMG_UINT8 *pui8Data, IMG_UINT32 ui32BCount, IMG_UINT32 ui32Flags)
{
	IMG_UINT32	ui32BytesWritten;

	if ((ui32Flags & PDUMP_FLAGS_CONTINUOUS) != 0)
	{


		if (((psStream->ui32CapMode & DEBUG_CAPMODE_FRAMED) != 0) &&
		    (psStream->ui32Start == 0xFFFFFFFFUL) &&
		    (psStream->ui32End == 0xFFFFFFFFUL) &&
		     psStream->bInitPhaseComplete)
		{
			ui32BytesWritten = ui32BCount;
		}
		else
		{
			ui32BytesWritten = gpfnDbgDrv->pfnDBGDrivWrite2(psStream, pui8Data, ui32BCount, 1);
		}
	}
	else
	{
		if (ui32Flags & PDUMP_FLAGS_LASTFRAME)
		{
			IMG_UINT32	ui32DbgFlags;

			ui32DbgFlags = 0;
			if (ui32Flags & PDUMP_FLAGS_RESETLFBUFFER)
			{
				ui32DbgFlags |= WRITELF_FLAGS_RESETBUF;
			}

			ui32BytesWritten = gpfnDbgDrv->pfnWriteLF(psStream, pui8Data, ui32BCount, 1, ui32DbgFlags);
		}
		else
		{
			ui32BytesWritten = gpfnDbgDrv->pfnWriteBINCM(psStream, pui8Data, ui32BCount, 1);
		}
	}

	return ui32BytesWritten;
}


IMG_VOID PDumpSuspendKM(IMG_VOID)
{
	atomic_inc(&gsPDumpSuspended);
}

IMG_VOID PDumpResumeKM(IMG_VOID)
{
	atomic_dec(&gsPDumpSuspended);
}

#endif
#endif

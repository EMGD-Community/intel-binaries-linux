/*************************************************************************/ /*!
@Title          Buffer management functions for Linux
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Manages buffers mapped into two memory spaces - cpu and device,
                either of which can be virtual or physical.
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

#include "services_headers.h"

#include "sysconfig.h"
#include "hash.h"
#include "ra.h"
#include "pdump_km.h"

#define MIN(a,b)       (a > b ? b : a)


#include "lists.h"

DECLARE_LIST_ANY_VA(BM_HEAP);
DECLARE_LIST_ANY_2(BM_HEAP, PVRSRV_ERROR, PVRSRV_OK);
DECLARE_LIST_ANY_VA_2(BM_HEAP, PVRSRV_ERROR, PVRSRV_OK);
DECLARE_LIST_FOR_EACH_VA(BM_HEAP);
DECLARE_LIST_INSERT(BM_HEAP);
DECLARE_LIST_REMOVE(BM_HEAP);

DECLARE_LIST_FOR_EACH(BM_CONTEXT);
DECLARE_LIST_ANY_VA(BM_CONTEXT);
DECLARE_LIST_ANY_VA_2(BM_CONTEXT, IMG_HANDLE, IMG_NULL);
DECLARE_LIST_INSERT(BM_CONTEXT);
DECLARE_LIST_REMOVE(BM_CONTEXT);


static IMG_BOOL
ZeroBuf(BM_BUF *pBuf, BM_MAPPING *pMapping, IMG_SIZE_T ui32Bytes, IMG_UINT32 ui32Flags);
static IMG_VOID
BM_FreeMemory (IMG_VOID *pH, IMG_UINTPTR_T base, BM_MAPPING *psMapping);
static IMG_BOOL
BM_ImportMemory(IMG_VOID *pH, IMG_SIZE_T uSize,
					IMG_SIZE_T *pActualSize, BM_MAPPING **ppsMapping,
					IMG_UINT32 uFlags, IMG_UINTPTR_T *pBase);

static IMG_BOOL
DevMemoryAlloc (BM_CONTEXT *pBMContext,
				BM_MAPPING *pMapping,
				IMG_SIZE_T *pActualSize,
				IMG_UINT32 uFlags,
				IMG_UINT32 dev_vaddr_alignment,
				IMG_DEV_VIRTADDR *pDevVAddr);
static IMG_VOID
DevMemoryFree (BM_MAPPING *pMapping);

/*!
******************************************************************************

	@Function   AllocMemory

	@Description	Allocate a buffer mapped into both cpu and device virtual
				address spaces.  This is now quite simple:

				1. Choose whence to get the memory;
				2. Obtain memory from that source;
				3. Work out the actual buffer addresses in other spaces.

				In choosing whence to get the memory we work like this:

				1. If an import arena exists, use unless BP_CONTIGUOUS is set;
				2. Use a contiguous pool.

	@Input      pBMContext - BM context
	@Input      psBMHeap - BM heap
	@Input      psDevVAddr - device virtual address (optional)
	@Input      uSize - requested buffer size in bytes.
	@Input      uFlags - property flags for the buffer.
    @Input      uDevVAddrAlignment - required device virtual address
					 alignment, or 0.
    @Input      pvPrivData - opaque private data passed through to allocator
    @Input      ui32PrivDataLength - length of opaque private data

	@Output     pBuf - receives a pointer to a descriptor of the allocated
					 buffer.
	@Return 	IMG_TRUE - Success
				IMG_FALSE - Failed.

 *****************************************************************************/
static IMG_BOOL
AllocMemory (BM_CONTEXT				*pBMContext,
				BM_HEAP				*psBMHeap,
				IMG_DEV_VIRTADDR	*psDevVAddr,
				IMG_SIZE_T			uSize,
				IMG_UINT32			uFlags,
				IMG_UINT32			uDevVAddrAlignment,
				BM_BUF				*pBuf)
{
	BM_MAPPING			*pMapping;
	IMG_UINTPTR_T		uOffset;
	RA_ARENA			*pArena = IMG_NULL;

	PVR_DPF ((PVR_DBG_MESSAGE,
			  "AllocMemory (pBMContext=%08X, uSize=0x%x, uFlags=0x%x, align=0x%x, pBuf=%08X)",
			  pBMContext, uSize, uFlags, uDevVAddrAlignment, pBuf));




	if(uFlags & PVRSRV_MEM_RAM_BACKED_ALLOCATION)
	{
		if(uFlags & PVRSRV_MEM_USER_SUPPLIED_DEVVADDR)
		{

			PVR_DPF ((PVR_DBG_ERROR, "AllocMemory: combination of DevVAddr management and RAM backing mode unsupported"));
			return IMG_FALSE;
		}

		/* BM supplied DevVAddr, RAM Backing */

		/* check heap attributes */
		if(psBMHeap->ui32Attribs
		   &	(PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG
		   |PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG))
		{
			/* specify arena (VM+RAM)*/
			pArena = psBMHeap->pImportArena;
		}
		else
		{
			PVR_DPF ((PVR_DBG_ERROR, "AllocMemory: backing store type doesn't match heap"));
			return IMG_FALSE;
		}


		if (!RA_Alloc(pArena,
					  uSize,
					  IMG_NULL,
					  (IMG_VOID*) &pMapping,
					  uFlags,
					  uDevVAddrAlignment,
					  0,
					  (IMG_UINTPTR_T *)&(pBuf->DevVAddr.uiAddr)))
		{
			PVR_DPF((PVR_DBG_ERROR, "AllocMemory: RA_Alloc(0x%x) FAILED", uSize));
			return IMG_FALSE;
		}

		uOffset = pBuf->DevVAddr.uiAddr - pMapping->DevVAddr.uiAddr;
		if(pMapping->CpuVAddr)
		{
			pBuf->CpuVAddr = (IMG_VOID*) ((IMG_UINTPTR_T)pMapping->CpuVAddr + uOffset);
		}
		else
		{
			pBuf->CpuVAddr = IMG_NULL;
		}

		if(uSize == pMapping->uSize)
		{
			pBuf->hOSMemHandle = pMapping->hOSMemHandle;
		}
		else
		{
			if(OSGetSubMemHandle(pMapping->hOSMemHandle,
								 uOffset,
								 uSize,
								 psBMHeap->ui32Attribs,
								 &pBuf->hOSMemHandle)!=PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR, "AllocMemory: OSGetSubMemHandle FAILED"));
				return IMG_FALSE;
			}
		}

		/* for hm_contiguous and hm_wrapped memory, the pMapping
		 * will have a physical address, else 0 */
		pBuf->CpuPAddr.uiAddr = pMapping->CpuPAddr.uiAddr + uOffset;

		if(uFlags & PVRSRV_MEM_ZERO)
		{
			if(!ZeroBuf(pBuf, pMapping, uSize, psBMHeap->ui32Attribs | uFlags))
			{
				return IMG_FALSE;
			}
		}
	}
	else
	{
		if(uFlags & PVRSRV_MEM_USER_SUPPLIED_DEVVADDR)
		{
			/* user supplied DevVAddr, no RAM backing */
			PVR_ASSERT(psDevVAddr != IMG_NULL);

			if (psDevVAddr == IMG_NULL)
			{
				PVR_DPF((PVR_DBG_ERROR, "AllocMemory: invalid parameter - psDevVAddr"));
				return IMG_FALSE;
			}

			/* just make space in the pagetables */
			pBMContext->psDeviceNode->pfnMMUAlloc (psBMHeap->pMMUHeap,
													uSize,
													IMG_NULL,
													PVRSRV_MEM_USER_SUPPLIED_DEVVADDR,
													uDevVAddrAlignment,
													psDevVAddr);

			/* setup buf */
			pBuf->DevVAddr = *psDevVAddr;
		}
		else
		{



			pBMContext->psDeviceNode->pfnMMUAlloc (psBMHeap->pMMUHeap,
													uSize,
													IMG_NULL,
													0,
													uDevVAddrAlignment,
													&pBuf->DevVAddr);
		}

		/* allocate a mocked-up mapping */
		if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
							sizeof (struct _BM_MAPPING_),
							(IMG_PVOID *)&pMapping, IMG_NULL,
							"Buffer Manager Mapping") != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR, "AllocMemory: OSAllocMem(0x%x) FAILED"));
			return IMG_FALSE;
		}

		/* setup buf */
		pBuf->CpuVAddr = IMG_NULL;
		pBuf->hOSMemHandle = 0;
		pBuf->CpuPAddr.uiAddr = 0;

		/* setup mapping */
		pMapping->CpuVAddr = IMG_NULL;
		pMapping->CpuPAddr.uiAddr = 0;
		pMapping->DevVAddr = pBuf->DevVAddr;
		pMapping->psSysAddr = IMG_NULL;
		pMapping->uSize = uSize;
		pMapping->hOSMemHandle = 0;
	}

	/* Record the arena pointer in the mapping. */
	pMapping->pArena = pArena;

	/* record the heap */
	pMapping->pBMHeap = psBMHeap;
	pBuf->pMapping = pMapping;

	/* output some stats */
	PVR_DPF ((PVR_DBG_MESSAGE,
				"AllocMemory: pMapping=%08X: DevV=%08X CpuV=%08X CpuP=%08X uSize=0x%x",
				pMapping,
				pMapping->DevVAddr.uiAddr,
				pMapping->CpuVAddr,
				pMapping->CpuPAddr.uiAddr,
				pMapping->uSize));

	PVR_DPF ((PVR_DBG_MESSAGE,
				"AllocMemory: pBuf=%08X: DevV=%08X CpuV=%08X CpuP=%08X uSize=0x%x",
				pBuf,
				pBuf->DevVAddr.uiAddr,
				pBuf->CpuVAddr,
				pBuf->CpuPAddr.uiAddr,
				uSize));


	PVR_ASSERT(((pBuf->DevVAddr.uiAddr) & (uDevVAddrAlignment - 1)) == 0);

	return IMG_TRUE;
}


/*!
******************************************************************************

	@Function	WrapMemory

	@Description Allocate a buffer mapped into both cpu and device virtual
				address spaces.

	@Input      psBMHeap - BM heap
	@Input      uSize - requested buffer size in bytes.
	@Input      ui32BaseOffset - Offset from page of wrap.
	@Input      bPhysContig - Is the wrap physically contiguous.
	@Input      psAddr - List of pages to wrap.
	@Input      pvCPUVAddr - Optional CPU Kernel virtual address (page aligned) of memory to wrap
	@Input      uFlags - property flags for the buffer.
	@Output     Buf - receives a pointer to a descriptor of the allocated
					 buffer.
	@Return 	IMG_TRUE - Success
				IMG_FALSE - Failed.

 *****************************************************************************/
static IMG_BOOL
WrapMemory (BM_HEAP *psBMHeap,
			IMG_SIZE_T uSize,
			IMG_SIZE_T ui32BaseOffset,
			IMG_BOOL bPhysContig,
			IMG_SYS_PHYADDR *psAddr,
			IMG_VOID *pvCPUVAddr,
			IMG_UINT32 uFlags,
			BM_BUF *pBuf)
{
	IMG_DEV_VIRTADDR DevVAddr = {0};
	BM_MAPPING *pMapping;
	IMG_BOOL bResult;
	IMG_SIZE_T const ui32PageSize = HOST_PAGESIZE();

	PVR_DPF ((PVR_DBG_MESSAGE,
			  "WrapMemory(psBMHeap=%08X, size=0x%x, offset=0x%x, bPhysContig=0x%x, pvCPUVAddr = 0x%x, flags=0x%x, pBuf=%08X)",
			  psBMHeap, uSize, ui32BaseOffset, bPhysContig, pvCPUVAddr, uFlags, pBuf));

	PVR_ASSERT((psAddr->uiAddr & (ui32PageSize - 1)) == 0);

	PVR_ASSERT(((IMG_UINTPTR_T)pvCPUVAddr & (ui32PageSize - 1)) == 0);

	uSize += ui32BaseOffset;
	uSize = HOST_PAGEALIGN (uSize);


	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
						sizeof(*pMapping),
						(IMG_PVOID *)&pMapping, IMG_NULL,
						"Mocked-up mapping") != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "WrapMemory: OSAllocMem(0x%x) FAILED",sizeof(*pMapping)));
		return IMG_FALSE;
	}

	OSMemSet(pMapping, 0, sizeof (*pMapping));

	pMapping->uSize = uSize;
	pMapping->pBMHeap = psBMHeap;

	if(pvCPUVAddr)
	{
		pMapping->CpuVAddr = pvCPUVAddr;

		if (bPhysContig)
		{
			pMapping->eCpuMemoryOrigin = hm_wrapped_virtaddr;
			pMapping->CpuPAddr = SysSysPAddrToCpuPAddr(psAddr[0]);

			if(OSRegisterMem(pMapping->CpuPAddr,
							pMapping->CpuVAddr,
							pMapping->uSize,
							uFlags,
							&pMapping->hOSMemHandle) != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,	"WrapMemory: OSRegisterMem Phys=0x%08X, CpuVAddr = 0x%08X, Size=%d) failed",
					pMapping->CpuPAddr, pMapping->CpuVAddr, pMapping->uSize));
				goto fail_cleanup;
			}
		}
		else
		{
			pMapping->eCpuMemoryOrigin = hm_wrapped_scatter_virtaddr;
			pMapping->psSysAddr = psAddr;

			if(OSRegisterDiscontigMem(pMapping->psSysAddr,
							pMapping->CpuVAddr,
							pMapping->uSize,
							uFlags,
							&pMapping->hOSMemHandle) != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,	"WrapMemory: OSRegisterDiscontigMem CpuVAddr = 0x%08X, Size=%d) failed",
					pMapping->CpuVAddr, pMapping->uSize));
				goto fail_cleanup;
			}
		}
	}
	else
	{
		if (bPhysContig)
		{
			pMapping->eCpuMemoryOrigin = hm_wrapped;
			pMapping->CpuPAddr = SysSysPAddrToCpuPAddr(psAddr[0]);

			if(OSReservePhys(pMapping->CpuPAddr,
							 pMapping->uSize,
							 uFlags,
							 &pMapping->CpuVAddr,
							 &pMapping->hOSMemHandle) != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,	"WrapMemory: OSReservePhys Phys=0x%08X, Size=%d) failed",
					pMapping->CpuPAddr, pMapping->uSize));
				goto fail_cleanup;
			}
		}
		else
		{
			pMapping->eCpuMemoryOrigin = hm_wrapped_scatter;
			pMapping->psSysAddr = psAddr;

			if(OSReserveDiscontigPhys(pMapping->psSysAddr,
							 pMapping->uSize,
							 uFlags,
							 &pMapping->CpuVAddr,
							 &pMapping->hOSMemHandle) != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,	"WrapMemory: OSReserveDiscontigPhys Size=%d) failed",
					pMapping->uSize));
				goto fail_cleanup;
			}
		}
	}

	/*
	 * Allocate device memory for this buffer. Map wrapped pages as read/write
	 */
	bResult = DevMemoryAlloc(psBMHeap->pBMContext,
							 pMapping,
							 IMG_NULL,
							 uFlags | PVRSRV_MEM_READ | PVRSRV_MEM_WRITE,
							 IMG_CAST_TO_DEVVADDR_UINT(ui32PageSize),
							 &DevVAddr);
	if (!bResult)
	{
		PVR_DPF((PVR_DBG_ERROR,
				"WrapMemory: DevMemoryAlloc(0x%x) failed",
				pMapping->uSize));
		goto fail_cleanup;
	}

	/*
	 * Determine the offset of this allocation within the underlying
	 * dual mapped chunk of memory, we can assume that all three
	 * addresses associated with this allocation are placed at the same
	 * offset within the underlying chunk.
	 */
	pBuf->CpuPAddr.uiAddr = pMapping->CpuPAddr.uiAddr + ui32BaseOffset;
	if(!ui32BaseOffset)
	{
		pBuf->hOSMemHandle = pMapping->hOSMemHandle;
	}
	else
	{
		if(OSGetSubMemHandle(pMapping->hOSMemHandle,
							 ui32BaseOffset,
							 (pMapping->uSize-ui32BaseOffset),
							 uFlags,
							 &pBuf->hOSMemHandle)!=PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR, "WrapMemory: OSGetSubMemHandle failed"));
			goto fail_cleanup;
		}
	}
	if(pMapping->CpuVAddr)
	{
		pBuf->CpuVAddr = (IMG_VOID*) ((IMG_UINTPTR_T)pMapping->CpuVAddr + ui32BaseOffset);
	}
	pBuf->DevVAddr.uiAddr = pMapping->DevVAddr.uiAddr + IMG_CAST_TO_DEVVADDR_UINT(ui32BaseOffset);

	if(uFlags & PVRSRV_MEM_ZERO)
	{
		if(!ZeroBuf(pBuf, pMapping, uSize, uFlags))
		{
			return IMG_FALSE;
		}
	}

	PVR_DPF ((PVR_DBG_MESSAGE, "DevVaddr.uiAddr=%08X", DevVAddr.uiAddr));
	PVR_DPF ((PVR_DBG_MESSAGE,
				"WrapMemory: pMapping=%08X: DevV=%08X CpuV=%08X CpuP=%08X uSize=0x%x",
				pMapping, pMapping->DevVAddr.uiAddr,
				pMapping->CpuVAddr, pMapping->CpuPAddr.uiAddr, pMapping->uSize));
	PVR_DPF ((PVR_DBG_MESSAGE,
				"WrapMemory: pBuf=%08X: DevV=%08X CpuV=%08X CpuP=%08X uSize=0x%x",
				pBuf, pBuf->DevVAddr.uiAddr,
				pBuf->CpuVAddr, pBuf->CpuPAddr.uiAddr, uSize));

	pBuf->pMapping = pMapping;
	return IMG_TRUE;

fail_cleanup:
	if(ui32BaseOffset && pBuf->hOSMemHandle)
	{
		OSReleaseSubMemHandle(pBuf->hOSMemHandle, uFlags);
	}

	if(pMapping && (pMapping->CpuVAddr || pMapping->hOSMemHandle))
	{
		switch(pMapping->eCpuMemoryOrigin)
		{
			case hm_wrapped:
				OSUnReservePhys(pMapping->CpuVAddr, pMapping->uSize, uFlags, pMapping->hOSMemHandle);
				break;
			case hm_wrapped_virtaddr:
				OSUnRegisterMem(pMapping->CpuVAddr, pMapping->uSize, uFlags, pMapping->hOSMemHandle);
				break;
			case hm_wrapped_scatter:
				OSUnReserveDiscontigPhys(pMapping->CpuVAddr, pMapping->uSize, uFlags, pMapping->hOSMemHandle);
				break;
			case hm_wrapped_scatter_virtaddr:
				OSUnRegisterDiscontigMem(pMapping->CpuVAddr, pMapping->uSize, uFlags, pMapping->hOSMemHandle);
				break;
			default:
				break;
		}

	}

	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_MAPPING), pMapping, IMG_NULL);
	/*not nulling pointer, out of scope*/

	return IMG_FALSE;
}


static IMG_BOOL
ZeroBuf(BM_BUF *pBuf, BM_MAPPING *pMapping, IMG_SIZE_T ui32Bytes, IMG_UINT32 ui32Flags)
{
	IMG_VOID *pvCpuVAddr;

	if(pBuf->CpuVAddr)
	{
		OSMemSet(pBuf->CpuVAddr, 0, ui32Bytes);
	}
	else if(pMapping->eCpuMemoryOrigin == hm_contiguous
			|| pMapping->eCpuMemoryOrigin == hm_wrapped)
	{
		pvCpuVAddr = OSMapPhysToLin(pBuf->CpuPAddr,
									ui32Bytes,
									PVRSRV_HAP_KERNEL_ONLY
									| (ui32Flags & PVRSRV_HAP_CACHETYPE_MASK),
									IMG_NULL);
		if(!pvCpuVAddr)
		{
			PVR_DPF((PVR_DBG_ERROR, "ZeroBuf: OSMapPhysToLin for contiguous buffer failed"));
			return IMG_FALSE;
		}
		OSMemSet(pvCpuVAddr, 0, ui32Bytes);
		OSUnMapPhysToLin(pvCpuVAddr,
						 ui32Bytes,
						 PVRSRV_HAP_KERNEL_ONLY
						 | (ui32Flags & PVRSRV_HAP_CACHETYPE_MASK),
						 IMG_NULL);
	}
	else
	{
		IMG_SIZE_T ui32BytesRemaining = ui32Bytes;
		IMG_SIZE_T ui32CurrentOffset = 0;
		IMG_CPU_PHYADDR CpuPAddr;

		/* Walk through the pBuf one page at a time and use
		 * transient mappings to zero the memory */

		PVR_ASSERT(pBuf->hOSMemHandle);

		while(ui32BytesRemaining > 0)
		{
			IMG_SIZE_T ui32BlockBytes = MIN(ui32BytesRemaining, HOST_PAGESIZE());
			CpuPAddr = OSMemHandleToCpuPAddr(pBuf->hOSMemHandle, ui32CurrentOffset);
			/* If the CpuPAddr isn't page aligned then start by writing up to the next page
			 * boundary (or ui32BytesRemaining if less), so that subsequent iterations can
			 * copy full physical pages. */
			if(CpuPAddr.uiAddr & (HOST_PAGESIZE() -1))
			{
				ui32BlockBytes =
					MIN(ui32BytesRemaining, HOST_PAGEALIGN(CpuPAddr.uiAddr) - CpuPAddr.uiAddr);
			}

			pvCpuVAddr = OSMapPhysToLin(CpuPAddr,
										ui32BlockBytes,
										PVRSRV_HAP_KERNEL_ONLY
										| (ui32Flags & PVRSRV_HAP_CACHETYPE_MASK),
										IMG_NULL);
			if(!pvCpuVAddr)
			{
				PVR_DPF((PVR_DBG_ERROR, "ZeroBuf: OSMapPhysToLin while zeroing non-contiguous memory FAILED"));
				return IMG_FALSE;
			}
			OSMemSet(pvCpuVAddr, 0, ui32BlockBytes);
			OSUnMapPhysToLin(pvCpuVAddr,
							 ui32BlockBytes,
							 PVRSRV_HAP_KERNEL_ONLY
							 | (ui32Flags & PVRSRV_HAP_CACHETYPE_MASK),
							 IMG_NULL);

			ui32BytesRemaining -= ui32BlockBytes;
			ui32CurrentOffset += ui32BlockBytes;
		}
	}

	return IMG_TRUE;
}

/*!
******************************************************************************

	@Function	FreeBuf

	@Description	Free a buffer previously allocated with BM_Alloc() or unwrap
				one previous wrapped with BM_Wrap().
				The buffer is identified by the buffer descriptor pBuf
				returned at allocation. Note the double indirection when
				passing the buffer.

	
	@Input      pBuf - buffer descriptor to free.
	@Input      ui32Flags - flags
	@Input      bFromAllocator - Is this being called by the
					                 allocator?

	@Return 	None.

 *****************************************************************************/
static IMG_VOID
FreeBuf (BM_BUF *pBuf, IMG_UINT32 ui32Flags, IMG_BOOL bFromAllocator)
{
	BM_MAPPING *pMapping;

	PVR_DPF ((PVR_DBG_MESSAGE,
			"FreeBuf: pBuf=%08X: DevVAddr=%08X CpuVAddr=%08X CpuPAddr=%08X",
			pBuf, pBuf->DevVAddr.uiAddr, pBuf->CpuVAddr, pBuf->CpuPAddr.uiAddr));

	/* record mapping */
	pMapping = pBuf->pMapping;

	if(ui32Flags & PVRSRV_MEM_USER_SUPPLIED_DEVVADDR)
	{
		/* Submemhandle is required by exported mappings */
		if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
		{
			/* user supplied Device Virtual Address */
			if(ui32Flags & PVRSRV_MEM_RAM_BACKED_ALLOCATION)
			{
				/* RAM backed allocation */
				PVR_DPF ((PVR_DBG_ERROR, "FreeBuf: combination of DevVAddr management and RAM backing mode unsupported"));
			}
			else
			{
				/* free the mocked-up mapping */
				OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_MAPPING), pMapping, IMG_NULL);
				pBuf->pMapping = IMG_NULL;
			}
		}
	}
	else
	{
		/* BM supplied Device Virtual Address */
		if(pBuf->hOSMemHandle != pMapping->hOSMemHandle)
		{
			/* Submemhandle is required by exported mappings */
			if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
			{

				OSReleaseSubMemHandle(pBuf->hOSMemHandle, ui32Flags);
			}
		}
		if(ui32Flags & PVRSRV_MEM_RAM_BACKED_ALLOCATION)
		{

			if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
			{



				PVR_ASSERT(pBuf->ui32ExportCount == 0);
				RA_Free (pBuf->pMapping->pArena, pBuf->DevVAddr.uiAddr, IMG_FALSE);
			}
		}
		else
		{
			if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
			{
				switch (pMapping->eCpuMemoryOrigin)
				{
					case hm_wrapped:
						OSUnReservePhys(pMapping->CpuVAddr, pMapping->uSize, ui32Flags, pMapping->hOSMemHandle);
						break;
					case hm_wrapped_virtaddr:
						OSUnRegisterMem(pMapping->CpuVAddr, pMapping->uSize, ui32Flags, pMapping->hOSMemHandle);
						break;
					case hm_wrapped_scatter:
						OSUnReserveDiscontigPhys(pMapping->CpuVAddr, pMapping->uSize, ui32Flags, pMapping->hOSMemHandle);
						break;
					case hm_wrapped_scatter_virtaddr:
						OSUnRegisterDiscontigMem(pMapping->CpuVAddr, pMapping->uSize, ui32Flags, pMapping->hOSMemHandle);
						break;
					default:
						break;
				}
			}

			if (bFromAllocator)
				DevMemoryFree (pMapping);

			if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
			{
				/* free the mocked-up mapping */
				OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_MAPPING), pMapping, IMG_NULL);
				pBuf->pMapping = IMG_NULL; /*nulling pointer alias*/
			}
		}

		if ((pBuf->ui32ExportCount == 0) && (pBuf->ui32RefCount == 0))
		{
			OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_BUF), pBuf, IMG_NULL);

		}
	}
}

/*!
******************************************************************************

	@Function	BM_DestroyContext_AnyCb

	@Description	Destroy a buffer manager heap.

	@Input      psBMHeap

	@Return 	PVRSRV_ERROR

 *****************************************************************************/
PVRSRV_ERROR BM_DestroyContext_AnyCb(BM_HEAP *psBMHeap)
{
	if(psBMHeap->ui32Attribs
	& 	(PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG
		|PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG))
	{
		if (psBMHeap->pImportArena)
		{
			IMG_BOOL bTestDelete = RA_TestDelete(psBMHeap->pImportArena);
			if (!bTestDelete)
			{
				PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext_AnyCb: RA_TestDelete failed"));
				return PVRSRV_ERROR_GENERIC;
			}
		}
	}
	return PVRSRV_OK;
}


/*!
******************************************************************************

	@Function	BM_DestroyContext

	@Description	Destroy a buffer manager context. All allocated buffers must be
				free'd before calling this function.  This function is called
				also to perform cleanup during aborted initialisations so it's
				fairly careful not to assume any given resource has really been
				created/allocated.

	@Return 	PVRSRV_ERROR

 *****************************************************************************/
PVRSRV_ERROR
BM_DestroyContext(IMG_HANDLE	hBMContext,
				  IMG_BOOL		*pbDestroyed)
{
	PVRSRV_ERROR eError;
	BM_CONTEXT *pBMContext = (BM_CONTEXT*)hBMContext;

	PVR_DPF ((PVR_DBG_MESSAGE, "BM_DestroyContext"));

	if (pbDestroyed != IMG_NULL)
	{
		*pbDestroyed = IMG_FALSE;
	}

	/*
		Exit straight away if it's an invalid context handle
	*/
	if (pBMContext == IMG_NULL)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext: Invalid handle"));
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	pBMContext->ui32RefCount--;

	if (pBMContext->ui32RefCount > 0)
	{
		/* Just return if there are more references to this context */
		return PVRSRV_OK;
	}




	eError = List_BM_HEAP_PVRSRV_ERROR_Any(pBMContext->psBMHeap, BM_DestroyContext_AnyCb);
	if(eError != PVRSRV_OK)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext: List_BM_HEAP_PVRSRV_ERROR_Any failed"));
#if 0




		PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext: Cleaning up with ResManFreeSpecial"));
		if(ResManFreeSpecial() != PVRSRV_OK)
		{
			PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext: ResManFreeSpecial failed %d",eError));
		}

#endif
		return eError;
	}
	else
	{

		eError = ResManFreeResByPtr(pBMContext->hResItem);
		if(eError != PVRSRV_OK)
		{
			PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyContext: ResManFreeResByPtr failed %d",eError));
			return eError;
		}


		if (pbDestroyed != IMG_NULL)
		{
			*pbDestroyed = IMG_TRUE;
		}
	}

	return PVRSRV_OK;
}


PVRSRV_ERROR BM_DestroyContextCallBack_AnyVaCb(BM_HEAP *psBMHeap, va_list va)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
	psDeviceNode = va_arg(va, PVRSRV_DEVICE_NODE*);

	/* Free up the import arenas */
	if(psBMHeap->ui32Attribs
	& 	(PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG
		|PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG))
	{
		if (psBMHeap->pImportArena)
		{
			RA_Delete (psBMHeap->pImportArena);
		}
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_DestroyContext: backing store type unsupported"));
		return PVRSRV_ERROR_GENERIC;
	}

	/* Free up the MMU Heaps */
	psDeviceNode->pfnMMUDelete(psBMHeap->pMMUHeap);

	/* Free Heap memory */
	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_HEAP), psBMHeap, IMG_NULL);
	/*not nulling pointer, copy on stack*/

	return PVRSRV_OK;
}


static PVRSRV_ERROR BM_DestroyContextCallBack(IMG_PVOID		pvParam,
											  IMG_UINT32	ui32Param)
{
	BM_CONTEXT *pBMContext = pvParam;
	PVRSRV_DEVICE_NODE *psDeviceNode;
	PVR_UNREFERENCED_PARAMETER(ui32Param);

	/*
		Get DeviceNode from BMcontext
	*/
	psDeviceNode = pBMContext->psDeviceNode;



	if(List_BM_HEAP_PVRSRV_ERROR_Any_va(pBMContext->psBMHeap,
										BM_DestroyContextCallBack_AnyVaCb,
										psDeviceNode) != PVRSRV_OK)
	{
		return PVRSRV_ERROR_GENERIC;
	}


	if (pBMContext->psMMUContext)
	{
		psDeviceNode->pfnMMUFinalise(pBMContext->psMMUContext);
	}



	if (pBMContext->pBufferHash)
	{
		HASH_Delete(pBMContext->pBufferHash);
	}

	if (pBMContext == psDeviceNode->sDevMemoryInfo.pBMKernelContext)
	{

		psDeviceNode->sDevMemoryInfo.pBMKernelContext = IMG_NULL;
	}
	else
	{

		List_BM_CONTEXT_Remove(pBMContext);
	}

	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_CONTEXT), pBMContext, IMG_NULL);


	return PVRSRV_OK;
}


IMG_HANDLE BM_CreateContext_IncRefCount_AnyVaCb(BM_CONTEXT *pBMContext, va_list va)
{
	PRESMAN_CONTEXT	hResManContext;
	hResManContext = va_arg(va, PRESMAN_CONTEXT);
	if(ResManFindResourceByPtr(hResManContext, pBMContext->hResItem) == PVRSRV_OK)
	{

		pBMContext->ui32RefCount++;
		return pBMContext;
	}
	return IMG_NULL;
}

IMG_VOID BM_CreateContext_InsertHeap_ForEachVaCb(BM_HEAP *psBMHeap, va_list va)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
	BM_CONTEXT *pBMContext;
	psDeviceNode = va_arg(va, PVRSRV_DEVICE_NODE*);
	pBMContext = va_arg(va, BM_CONTEXT*);
	switch(psBMHeap->sDevArena.DevMemHeapType)
	{
		case DEVICE_MEMORY_HEAP_SHARED:
		case DEVICE_MEMORY_HEAP_SHARED_EXPORTED:
		{

			psDeviceNode->pfnMMUInsertHeap(pBMContext->psMMUContext, psBMHeap->pMMUHeap);
			break;
		}
	}
}

/*!
******************************************************************************

	@Function	BM_CreateContext

	@Description	Creates and initialises a buffer manager context. This function must be called
				before any other buffer manager functions.

	@Return 	valid BM context handle - Success
				IMG_NULL - Failed

 *****************************************************************************/
IMG_HANDLE
BM_CreateContext(PVRSRV_DEVICE_NODE			*psDeviceNode,
				 IMG_DEV_PHYADDR			*psPDDevPAddr,
				 PVRSRV_PER_PROCESS_DATA	*psPerProc,
				 IMG_BOOL					*pbCreated)
{
	BM_CONTEXT			*pBMContext;
	DEVICE_MEMORY_INFO	*psDevMemoryInfo;
	IMG_BOOL			bKernelContext;
	PRESMAN_CONTEXT		hResManContext;

	PVR_DPF((PVR_DBG_MESSAGE, "BM_CreateContext"));

	if (psPerProc == IMG_NULL)
	{
		bKernelContext = IMG_TRUE;
		hResManContext = psDeviceNode->hResManContext;
	}
	else
	{
		bKernelContext = IMG_FALSE;
		hResManContext = psPerProc->hResManContext;
	}

	if (pbCreated != IMG_NULL)
	{
		*pbCreated = IMG_FALSE;
	}

	/* setup the device memory info. */
	psDevMemoryInfo = &psDeviceNode->sDevMemoryInfo;

	if (bKernelContext == IMG_FALSE)
	{
		IMG_HANDLE res = (IMG_HANDLE) List_BM_CONTEXT_Any_va(psDevMemoryInfo->pBMContext,
															&BM_CreateContext_IncRefCount_AnyVaCb,
															hResManContext);
		if (res)
		{
			return res;
		}
	}

	/* allocate a BM context */
	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
					 sizeof (struct _BM_CONTEXT_),
					 (IMG_PVOID *)&pBMContext, IMG_NULL,
					 "Buffer Manager Context") != PVRSRV_OK)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_CreateContext: Alloc failed"));
		return IMG_NULL;
	}
	OSMemSet(pBMContext, 0, sizeof (BM_CONTEXT));

	/* store the associated devicenode */
	pBMContext->psDeviceNode = psDeviceNode;

	/* This hash table is used to store BM_Wraps in a global way */
	/* INTEGRATION_POINT: 32 is an abitrary limit on the number of hashed BM_wraps */
	pBMContext->pBufferHash = HASH_Create(32);
	if (pBMContext->pBufferHash==IMG_NULL)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_CreateContext: HASH_Create failed"));
		goto cleanup;
	}

	if(psDeviceNode->pfnMMUInitialise(psDeviceNode,
										&pBMContext->psMMUContext,
										psPDDevPAddr) != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_CreateContext: MMUInitialise failed"));
		goto cleanup;
	}

	if(bKernelContext)
	{

		PVR_ASSERT(psDevMemoryInfo->pBMKernelContext == IMG_NULL);
		psDevMemoryInfo->pBMKernelContext = pBMContext;
	}
	else
	{
		/*
			On the creation of each new context we must
			insert the kernel context's 'shared' and 'shared_exported'
			heaps into the new context
			 - check the kernel context and heaps exist
		*/
		PVR_ASSERT(psDevMemoryInfo->pBMKernelContext);

		if (psDevMemoryInfo->pBMKernelContext == IMG_NULL)
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_CreateContext: psDevMemoryInfo->pBMKernelContext invalid"));
			goto cleanup;
		}

		PVR_ASSERT(psDevMemoryInfo->pBMKernelContext->psBMHeap);

		/*
			insert the kernel heaps structures into the new context's shared heap list
			Note. this will include the kernel only heaps but these will not actually
			be imported into the context nor returned to the client
		 */
		pBMContext->psBMSharedHeap = psDevMemoryInfo->pBMKernelContext->psBMHeap;

		/*
			insert the shared heaps into the MMU page directory/table
			for the new context
		*/
		List_BM_HEAP_ForEach_va(pBMContext->psBMSharedHeap,
								BM_CreateContext_InsertHeap_ForEachVaCb,
								psDeviceNode,
								pBMContext);

		/* Finally, insert the new context into the list of BM contexts	*/
		List_BM_CONTEXT_Insert(&psDevMemoryInfo->pBMContext, pBMContext);
	}

	/* Increment the refcount, as creation is successful */
	pBMContext->ui32RefCount++;

	/* register with resman */
	pBMContext->hResItem = ResManRegisterRes(hResManContext,
											RESMAN_TYPE_DEVICEMEM_CONTEXT,
											pBMContext,
											0,
											&BM_DestroyContextCallBack);
	if (pBMContext->hResItem == IMG_NULL)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_CreateContext: ResManRegisterRes failed"));
		goto cleanup;
	}

	if (pbCreated != IMG_NULL)
	{
		*pbCreated = IMG_TRUE;
	}
	return (IMG_HANDLE)pBMContext;

cleanup:
	(IMG_VOID)BM_DestroyContextCallBack(pBMContext, 0);

	return IMG_NULL;
}


IMG_VOID *BM_CreateHeap_AnyVaCb(BM_HEAP *psBMHeap, va_list va)
{
	DEVICE_MEMORY_HEAP_INFO *psDevMemHeapInfo;
	psDevMemHeapInfo = va_arg(va, DEVICE_MEMORY_HEAP_INFO*);
	if (psBMHeap->sDevArena.ui32HeapID ==  psDevMemHeapInfo->ui32HeapID)
	{
		/* Match - just return already created heap */
		return psBMHeap;
	}
	else
	{
		return IMG_NULL;
	}
}

/*!
******************************************************************************

	@Function	BM_CreateHeap

	@Description	Creates and initialises a BM heap for a given BM context.

	@Return 
		valid heap handle - success
		IMG_NULL - failure


 *****************************************************************************/
IMG_HANDLE
BM_CreateHeap (IMG_HANDLE hBMContext,
			   DEVICE_MEMORY_HEAP_INFO *psDevMemHeapInfo)
{
	BM_CONTEXT *pBMContext = (BM_CONTEXT*)hBMContext;
	PVRSRV_DEVICE_NODE *psDeviceNode;
	BM_HEAP *psBMHeap;

	PVR_DPF((PVR_DBG_MESSAGE, "BM_CreateHeap"));

	if(!pBMContext)
	{
		return IMG_NULL;
	}

	psDeviceNode = pBMContext->psDeviceNode;


	/*
		We may be being asked to create a heap in a context which already has one.
		Test for refcount > 0 because PVRSRVGetDeviceMemHeapInfoKM doesn't increment the refcount.
		This does mean that the first call to PVRSRVCreateDeviceMemContextKM will first try to find
		heaps that we already know don't exist
	*/
	if(pBMContext->ui32RefCount > 0)
	{
		psBMHeap = (BM_HEAP*)List_BM_HEAP_Any_va(pBMContext->psBMHeap,
												 BM_CreateHeap_AnyVaCb,
												 psDevMemHeapInfo);

		if (psBMHeap)
		{
			return psBMHeap;
		}
	}


	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
						sizeof (BM_HEAP),
						(IMG_PVOID *)&psBMHeap, IMG_NULL,
						"Buffer Manager Heap") != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_CreateHeap: Alloc failed"));
		return IMG_NULL;
	}

	OSMemSet (psBMHeap, 0, sizeof (BM_HEAP));

	psBMHeap->sDevArena.ui32HeapID = psDevMemHeapInfo->ui32HeapID;
	psBMHeap->sDevArena.pszName = psDevMemHeapInfo->pszName;
	psBMHeap->sDevArena.BaseDevVAddr = psDevMemHeapInfo->sDevVAddrBase;
	psBMHeap->sDevArena.ui32Size = psDevMemHeapInfo->ui32HeapSize;
	psBMHeap->sDevArena.DevMemHeapType = psDevMemHeapInfo->DevMemHeapType;
	psBMHeap->sDevArena.ui32DataPageSize = psDevMemHeapInfo->ui32DataPageSize;
	psBMHeap->sDevArena.psDeviceMemoryHeapInfo = psDevMemHeapInfo;
	psBMHeap->ui32Attribs = psDevMemHeapInfo->ui32Attribs;

	/* tie the heap to the context */
	psBMHeap->pBMContext = pBMContext;

	psBMHeap->pMMUHeap = psDeviceNode->pfnMMUCreate (pBMContext->psMMUContext,
													&psBMHeap->sDevArena,
													&psBMHeap->pVMArena);
	if (!psBMHeap->pMMUHeap)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_CreateHeap: MMUCreate failed"));
		goto ErrorExit;
	}

	/* memory is allocated from the OS as required */
	psBMHeap->pImportArena = RA_Create (psDevMemHeapInfo->pszBSName,
										0, 0, IMG_NULL,
										psBMHeap->sDevArena.ui32DataPageSize,
										BM_ImportMemory,
										BM_FreeMemory,
										IMG_NULL,
										psBMHeap);
	if(psBMHeap->pImportArena == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_CreateHeap: RA_Create failed"));
		goto ErrorExit;
	}

	if(psBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG)
	{




		psBMHeap->pLocalDevMemArena = psDevMemHeapInfo->psLocalDevMemArena;
		if(psBMHeap->pLocalDevMemArena == IMG_NULL)
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_CreateHeap: LocalDevMemArena null"));
			goto ErrorExit;
		}
	}

	/* insert heap into head of the heap list */
	List_BM_HEAP_Insert(&pBMContext->psBMHeap, psBMHeap);

	return (IMG_HANDLE)psBMHeap;

	/* handle error case */
ErrorExit:

	/* Free up the MMU if we created one */
	if (psBMHeap->pMMUHeap != IMG_NULL)
	{
		psDeviceNode->pfnMMUDelete (psBMHeap->pMMUHeap);
		psDeviceNode->pfnMMUFinalise (pBMContext->psMMUContext);
	}

	/* Free the Heap memory */
	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_HEAP), psBMHeap, IMG_NULL);
	/*not nulling pointer, out of scope*/

	return IMG_NULL;
}

/*!
******************************************************************************

	@Function	BM_DestroyHeap

	@Description	Destroys a BM heap

	@Return 
		valid heap handle - success
		IMG_NULL - failure


 *****************************************************************************/
IMG_VOID
BM_DestroyHeap (IMG_HANDLE hDevMemHeap)
{
	BM_HEAP* psBMHeap = (BM_HEAP*)hDevMemHeap;
	PVRSRV_DEVICE_NODE *psDeviceNode = psBMHeap->pBMContext->psDeviceNode;

	PVR_DPF((PVR_DBG_MESSAGE, "BM_DestroyHeap"));

	if(psBMHeap)
	{
		/* Free up the import arenas */
		if(psBMHeap->ui32Attribs
		&	(PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG
			|PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG))
		{
			if (psBMHeap->pImportArena)
			{
				RA_Delete (psBMHeap->pImportArena);
			}
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_DestroyHeap: backing store type unsupported"));
			return;
		}

		/* Free up the MMU Heap */
		psDeviceNode->pfnMMUDelete (psBMHeap->pMMUHeap);

		/* remove from the heap list */
		List_BM_HEAP_Remove(psBMHeap);

		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_HEAP), psBMHeap, IMG_NULL);

	}
	else
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_DestroyHeap: invalid heap handle"));
	}
}


/*!
******************************************************************************

	@Function	BM_Reinitialise

	@Description	Reinitialise the buffer manager after a power down event.

	@Return 	IMG_TRUE - Success
				IMG_FALSE - Failed

 *****************************************************************************/
IMG_BOOL
BM_Reinitialise (PVRSRV_DEVICE_NODE *psDeviceNode)
{

	PVR_DPF((PVR_DBG_MESSAGE, "BM_Reinitialise"));
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);

        /* FIXME: Need to reenable all contexts
	  List_BM_CONTEXT_ForEach(psDeviceNode->sDevMemoryInfo.pBMContext, MMU_Enable);
        */

	return IMG_TRUE;
}

/*!
******************************************************************************

	@Function   BM_Alloc

	@Description	Allocate a buffer mapped into both cpu and device virtual
				memory maps.

	@Input      hDevMemHeap
	@Input      psDevVAddr - device virtual address specified by caller (optional)
	@Input      uSize - require size in bytes of the buffer.
	@Input      pui32Flags - bit mask of buffer property flags.
    @Input      uDevVAddrAlignment - required alignment in bytes, or 0.
    @Input      pvPrivData - opaque private data passed through to allocator
    @Input      ui32PrivDataLength - length of opaque private data

	@Output     phBuf - receives buffer handle
	@Output     pui32Flags - bit mask of heap property flags.

	@Return 	IMG_TRUE - Success
				IMG_FALSE - Failure

 *****************************************************************************/
IMG_BOOL
BM_Alloc (  IMG_HANDLE			hDevMemHeap,
			IMG_DEV_VIRTADDR	*psDevVAddr,
			IMG_SIZE_T			uSize,
			IMG_UINT32			*pui32Flags,
			IMG_UINT32			uDevVAddrAlignment,
			BM_HANDLE			*phBuf)
{
	BM_BUF *pBuf;
	BM_CONTEXT *pBMContext;
	BM_HEAP *psBMHeap;
	SYS_DATA *psSysData;
	IMG_UINT32 uFlags;

	if (pui32Flags == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_Alloc: invalid parameter"));
		PVR_DBG_BREAK;
		return IMG_FALSE;
	}

	uFlags = *pui32Flags;

	PVR_DPF ((PVR_DBG_MESSAGE,
		  "BM_Alloc (uSize=0x%x, uFlags=0x%x, uDevVAddrAlignment=0x%x)",
			uSize, uFlags, uDevVAddrAlignment));

	SysAcquireData(&psSysData);

	psBMHeap = (BM_HEAP*)hDevMemHeap;
	pBMContext = psBMHeap->pBMContext;

	if(uDevVAddrAlignment == 0)
	{
		uDevVAddrAlignment = 1;
	}

	/*
	 * Allocate something in which to record the allocation's details.
	 */
	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
				   sizeof (BM_BUF),
				   (IMG_PVOID *)&pBuf, IMG_NULL,
				   "Buffer Manager buffer") != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_Alloc: BM_Buf alloc FAILED"));
		return IMG_FALSE;
	}
	OSMemSet(pBuf, 0, sizeof (BM_BUF));

	/*
	 * Allocate the memory itself now.
	 */
	if (AllocMemory(pBMContext,
					psBMHeap,
					psDevVAddr,
					uSize,
					uFlags,
					uDevVAddrAlignment,
					pBuf) != IMG_TRUE)
	{
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof (BM_BUF), pBuf, IMG_NULL);
		/* not nulling pointer, out of scope */
		PVR_DPF((PVR_DBG_ERROR, "BM_Alloc: AllocMemory FAILED"));
		return IMG_FALSE;
	}

	PVR_DPF ((PVR_DBG_MESSAGE,
		  "BM_Alloc (uSize=0x%x, uFlags=0x%x)=%08X",
		  uSize, uFlags, pBuf));

	/*
	 * Assign the handle and return.
	 */
	pBuf->ui32RefCount = 1;
	*phBuf = (BM_HANDLE)pBuf;
	*pui32Flags = uFlags | psBMHeap->ui32Attribs;

	/*
	 * If the user has specified heap CACHETYPE flags themselves,
	 * override any CACHETYPE flags inherited from the heap.
	 */
	if(uFlags & PVRSRV_HAP_CACHETYPE_MASK)
	{
		*pui32Flags &= ~PVRSRV_HAP_CACHETYPE_MASK;
		*pui32Flags |= (uFlags & PVRSRV_HAP_CACHETYPE_MASK);
	}

	return IMG_TRUE;
}



#if defined(PVR_LMA)
/*!
******************************************************************************

	@Function   ValidSysPAddrArrayForDev

	@Description    Verify the array of system address is accessible
                    by the given device.

	@Input      psDeviceNode
    @Input      psSysPAddr - system address array
    @Input      ui32PageSize - size of address array
    
	@Return     IMG_BOOL

 *****************************************************************************/
static IMG_BOOL
ValidSysPAddrArrayForDev(PVRSRV_DEVICE_NODE *psDeviceNode, IMG_SYS_PHYADDR *psSysPAddr, IMG_UINT32 ui32PageCount, IMG_SIZE_T ui32PageSize)
{
	IMG_UINT32 i;

	for (i = 0; i < ui32PageCount; i++)
	{
		IMG_SYS_PHYADDR sStartSysPAddr = psSysPAddr[i];
		IMG_SYS_PHYADDR sEndSysPAddr;

		if (!SysVerifySysPAddrToDevPAddr(psDeviceNode->sDevId.eDeviceType, sStartSysPAddr))
		{
			return IMG_FALSE;
		}

		sEndSysPAddr.uiAddr = sStartSysPAddr.uiAddr + ui32PageSize;

		if (!SysVerifySysPAddrToDevPAddr(psDeviceNode->sDevId.eDeviceType, sEndSysPAddr))
		{
			return IMG_FALSE;
		}
	}

	return IMG_TRUE;
}

/*!
******************************************************************************

	@Function  ValidSysPAddrRangeForDev

	@Description   Verify a system address range is accessible
		   by the given device.

	@Input      psDeviceNode
    @Input      sStartSysPAddr - starting system address
    @Input      ui32Range - length of address range

	@Return     IMG_BOOL

 *****************************************************************************/
static IMG_BOOL
ValidSysPAddrRangeForDev(PVRSRV_DEVICE_NODE *psDeviceNode, IMG_SYS_PHYADDR sStartSysPAddr, IMG_SIZE_T ui32Range)
{
	IMG_SYS_PHYADDR sEndSysPAddr;

	if (!SysVerifySysPAddrToDevPAddr(psDeviceNode->sDevId.eDeviceType, sStartSysPAddr))
	{
		return IMG_FALSE;
	}

	sEndSysPAddr.uiAddr = sStartSysPAddr.uiAddr + ui32Range;

	if (!SysVerifySysPAddrToDevPAddr(psDeviceNode->sDevId.eDeviceType, sEndSysPAddr))
	{
		return IMG_FALSE;
	}

	return IMG_TRUE;
}

#define	WRAP_MAPPING_SIZE(ui32ByteSize, ui32PageOffset) HOST_PAGEALIGN((ui32ByteSize) + (ui32PageOffset))

#define	WRAP_PAGE_COUNT(ui32ByteSize, ui32PageOffset, ui32HostPageSize)	(WRAP_MAPPING_SIZE(ui32ByteSize, ui32PageOffset) / (ui32HostPageSize))

#endif


/*!
******************************************************************************

	@Function   BM_Wrap

	@Description	Create a buffer which wraps user provided system physical
				memory.
				The wrapped memory must be page aligned. BM_Wrap will
				roundup the size to a multiple of cpu pages.

	@Input      ui32Size - size of memory to wrap.
	@Input      ui32Offset - Offset into page of memory to wrap.
	@Input      bPhysContig - Is the wrap physically contiguous.
	@Input      psSysAddr - list of system physical page addresses of memory to wrap.
	@Input      pvCPUVAddr - optional CPU kernel virtual address (Page aligned) of memory to wrap.
    @Input      uFlags - bit mask of buffer property flags.
    @output     phBuf - receives the buffer handle.

	@Return 	IMG_TRUE - Success.
				IMG_FALSE - Failed

 *****************************************************************************/
IMG_BOOL
BM_Wrap (	IMG_HANDLE hDevMemHeap,
			IMG_SIZE_T ui32Size,
			IMG_SIZE_T ui32Offset,
			IMG_BOOL bPhysContig,
			IMG_SYS_PHYADDR *psSysAddr,
			IMG_VOID *pvCPUVAddr,
			IMG_UINT32 *pui32Flags,
			BM_HANDLE *phBuf)
{
	BM_BUF *pBuf;
	BM_CONTEXT *psBMContext;
	BM_HEAP *psBMHeap;
	SYS_DATA *psSysData;
	IMG_SYS_PHYADDR sHashAddress;
	IMG_UINT32 uFlags;

	psBMHeap = (BM_HEAP*)hDevMemHeap;
	psBMContext = psBMHeap->pBMContext;

	uFlags = psBMHeap->ui32Attribs & (PVRSRV_HAP_CACHETYPE_MASK | PVRSRV_HAP_MAPTYPE_MASK);

	if ((pui32Flags != IMG_NULL) && ((*pui32Flags & PVRSRV_HAP_CACHETYPE_MASK) != 0))
	{
		uFlags &= ~PVRSRV_HAP_CACHETYPE_MASK;
		uFlags |= *pui32Flags & PVRSRV_HAP_CACHETYPE_MASK;
	}

	PVR_DPF ((PVR_DBG_MESSAGE,
		  "BM_Wrap (uSize=0x%x, uOffset=0x%x, bPhysContig=0x%x, pvCPUVAddr=0x%x, uFlags=0x%x)",
			ui32Size, ui32Offset, bPhysContig, pvCPUVAddr, uFlags));

	SysAcquireData(&psSysData);

#if defined(PVR_LMA)
	if (bPhysContig)
	{
		if (!ValidSysPAddrRangeForDev(psBMContext->psDeviceNode, *psSysAddr, WRAP_MAPPING_SIZE(ui32Size, ui32Offset)))
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_Wrap: System address range invalid for device"));
			return IMG_FALSE;
		}
	}
	else
	{
		IMG_SIZE_T ui32HostPageSize = HOST_PAGESIZE();

		if (!ValidSysPAddrArrayForDev(psBMContext->psDeviceNode, psSysAddr, WRAP_PAGE_COUNT(ui32Size, ui32Offset, ui32HostPageSize), ui32HostPageSize))
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_Wrap: Array of system addresses invalid for device"));
			return IMG_FALSE;
		}
	}
#endif
	/*
	 * Insert the System Physical Address of the first page into the hash so we can optimise multiple wraps of the
	 * same memory.
	 */
	sHashAddress = psSysAddr[0];

	/* Add the in-page offset to ensure a unique hash */
	sHashAddress.uiAddr += ui32Offset;


	pBuf = (BM_BUF *)HASH_Retrieve(psBMContext->pBufferHash, (IMG_UINTPTR_T) sHashAddress.uiAddr);

	if(pBuf)
	{
		IMG_SIZE_T ui32MappingSize = HOST_PAGEALIGN (ui32Size + ui32Offset);

		/* Check base address, size and contiguity type match */
		if(pBuf->pMapping->uSize == ui32MappingSize && (pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped ||
														pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped_virtaddr))
		{
			PVR_DPF((PVR_DBG_MESSAGE,
					"BM_Wrap (Matched previous Wrap! uSize=0x%x, uOffset=0x%x, SysAddr=%08X)",
					ui32Size, ui32Offset, sHashAddress.uiAddr));

			pBuf->ui32RefCount++;
			*phBuf = (BM_HANDLE)pBuf;
			if(pui32Flags)
				*pui32Flags = uFlags;

			return IMG_TRUE;
		}
	}

	/*
	 * Allocate something in which to record the allocation's details.
	 */
	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
						sizeof (BM_BUF),
						(IMG_PVOID *)&pBuf, IMG_NULL,
						"Buffer Manager buffer") != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_Wrap: BM_Buf alloc FAILED"));
		return IMG_FALSE;
	}
	OSMemSet(pBuf, 0, sizeof (BM_BUF));

	/*
	 * Actually perform the memory wrap.
	 */
	if (WrapMemory (psBMHeap, ui32Size, ui32Offset, bPhysContig, psSysAddr, pvCPUVAddr, uFlags, pBuf) != IMG_TRUE)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_Wrap: WrapMemory FAILED"));
		OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof (BM_BUF), pBuf, IMG_NULL);
		/*not nulling pointer, out of scope*/
		return IMG_FALSE;
	}

	/* Only insert the buffer in the hash table if it is contiguous - allows for optimisation of multiple wraps
	 * of the same contiguous buffer.
	 */
	if(pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped || pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped_virtaddr)
	{
		/* Have we calculated the right Hash key ? */
		PVR_ASSERT(SysSysPAddrToCpuPAddr(sHashAddress).uiAddr == pBuf->CpuPAddr.uiAddr);

		if (!HASH_Insert (psBMContext->pBufferHash, (IMG_UINTPTR_T) sHashAddress.uiAddr, (IMG_UINTPTR_T)pBuf))
		{
			FreeBuf (pBuf, uFlags, IMG_TRUE);
			PVR_DPF((PVR_DBG_ERROR, "BM_Wrap: HASH_Insert FAILED"));
			return IMG_FALSE;
		}
	}

	PVR_DPF ((PVR_DBG_MESSAGE,
			"BM_Wrap (uSize=0x%x, uFlags=0x%x)=%08X(devVAddr=%08X)",
			ui32Size, uFlags, pBuf, pBuf->DevVAddr.uiAddr));

	/*
	 * Assign the handle and return.
	 */
	pBuf->ui32RefCount = 1;
	*phBuf = (BM_HANDLE)pBuf;
	if(pui32Flags)
	{
		/* need to override the heap attributes SINGLE PROC to MULT_PROC. */
		*pui32Flags = (uFlags & ~PVRSRV_HAP_MAPTYPE_MASK) | PVRSRV_HAP_MULTI_PROCESS;
	}

	return IMG_TRUE;
}

/*!
******************************************************************************

	@Function   BM_Export

	@Description	Export a buffer previously allocated via BM_Alloc.

	@Input      hBuf - buffer handle.
	@Input      ui32Flags - flags

	@Return 	None.

 *****************************************************************************/

IMG_VOID BM_Export (BM_HANDLE hBuf)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	pBuf->ui32ExportCount++;
}

IMG_VOID BM_FreeExport (BM_HANDLE hBuf, IMG_UINT32 ui32Flags)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	pBuf->ui32ExportCount--;
	FreeBuf (pBuf, ui32Flags, IMG_FALSE);
}

/*!
******************************************************************************
 @Function	 	BM_FreeExport

 @Description	Free a buffer previously exported via BM_Export.

 @Input         hBuf - buffer handle.
 @Input         ui32Flags - flags

 @Return   		None.
**************************************************************************/
IMG_VOID
BM_Free (BM_HANDLE hBuf,
		IMG_UINT32 ui32Flags)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;
	SYS_DATA *psSysData;
	IMG_SYS_PHYADDR sHashAddr;

	PVR_DPF ((PVR_DBG_MESSAGE, "BM_Free (h=%08X)", hBuf));
	PVR_ASSERT (pBuf!=IMG_NULL);

	if (pBuf == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_Free: invalid parameter"));
		return;
	}

	SysAcquireData(&psSysData);

	pBuf->ui32RefCount--;

	if(pBuf->ui32RefCount == 0)
	{
		if(pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped || pBuf->pMapping->eCpuMemoryOrigin == hm_wrapped_virtaddr)
		{
			sHashAddr = SysCpuPAddrToSysPAddr(pBuf->CpuPAddr);

			HASH_Remove (pBuf->pMapping->pBMHeap->pBMContext->pBufferHash,	(IMG_UINTPTR_T)sHashAddr.uiAddr);
		}
		FreeBuf (pBuf, ui32Flags, IMG_TRUE);
	}
}


IMG_CPU_VIRTADDR
BM_HandleToCpuVaddr (BM_HANDLE hBuf)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	PVR_ASSERT (pBuf != IMG_NULL);
	if (pBuf == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_HandleToCpuVaddr: invalid parameter"));
		return IMG_NULL;
	}

	PVR_DPF ((PVR_DBG_MESSAGE,
				"BM_HandleToCpuVaddr(h=%08X)=%08X",
				hBuf, pBuf->CpuVAddr));
	return pBuf->CpuVAddr;
}


/*!
******************************************************************************

	@Function   BM_HandleToDevVaddr

	@Description	Retreive the device virtual address associated with a buffer.

	@Input      hBuf - buffer handle.

	@Return 	buffers device virtual address.

 *****************************************************************************/
IMG_DEV_VIRTADDR
BM_HandleToDevVaddr (BM_HANDLE hBuf)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	PVR_ASSERT (pBuf != IMG_NULL);
	if (pBuf == IMG_NULL)
	{
		IMG_DEV_VIRTADDR	DevVAddr = {0};
		PVR_DPF((PVR_DBG_ERROR, "BM_HandleToDevVaddr: invalid parameter"));
		return DevVAddr;
	}

	PVR_DPF ((PVR_DBG_MESSAGE, "BM_HandleToDevVaddr(h=%08X)=%08X", hBuf, pBuf->DevVAddr));
	return pBuf->DevVAddr;
}


/*!
******************************************************************************

	@Function   BM_HandleToSysPaddr

	@Description	Retreive the system physical address associated with a buffer.

	@Input      hBuf - buffer handle.

	@Return 	buffers device virtual address.

 *****************************************************************************/
IMG_SYS_PHYADDR
BM_HandleToSysPaddr (BM_HANDLE hBuf)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	PVR_ASSERT (pBuf != IMG_NULL);

	if (pBuf == IMG_NULL)
	{
		IMG_SYS_PHYADDR	PhysAddr = {0};
		PVR_DPF((PVR_DBG_ERROR, "BM_HandleToSysPaddr: invalid parameter"));
		return PhysAddr;
	}

	PVR_DPF ((PVR_DBG_MESSAGE, "BM_HandleToSysPaddr(h=%08X)=%08X", hBuf, pBuf->CpuPAddr.uiAddr));
	return SysCpuPAddrToSysPAddr (pBuf->CpuPAddr);
}

/*!
******************************************************************************

	@Function   BM_HandleToMemOSHandle

	@Description	Retreive the underlying memory handle associated with a buffer.

	@Input      hBuf - buffer handle.

	@Return 	OS Specific memory handle.

 *****************************************************************************/
IMG_HANDLE
BM_HandleToOSMemHandle(BM_HANDLE hBuf)
{
	BM_BUF *pBuf = (BM_BUF *)hBuf;

	PVR_ASSERT (pBuf != IMG_NULL);

	if (pBuf == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_HandleToOSMemHandle: invalid parameter"));
		return IMG_NULL;
	}

	PVR_DPF ((PVR_DBG_MESSAGE,
				"BM_HandleToOSMemHandle(h=%08X)=%08X",
				hBuf, pBuf->hOSMemHandle));
	return pBuf->hOSMemHandle;
}


/*----------------------------------------------------------------------------
<function>
	FUNCTION:   BM_UnmapFromDev

	PURPOSE:	Unmaps a buffer from GPU virtual address space, but otherwise
				leaves buffer intact (ie. not changing any CPU virtual space
				mappings, etc).  This in conjunction with BM_RemapToDev() can
				be used to migrate buffers in and out of GPU virtual address
				space to deal with fragmentation and/or limited size of GPU
				MMU.

	PARAMETERS: In:  hBuf - buffer handle.
	RETURNS:	IMG_TRUE - Success
				IMG_FALSE - Failure
</function>
-----------------------------------------------------------------------------*/
IMG_BOOL
BM_ContiguousStatistics (IMG_UINT32 uFlags,
						 IMG_UINT32 *pTotalBytes,
						 IMG_UINT32 *pAvailableBytes)
{
	if (pAvailableBytes || pTotalBytes || uFlags);
	return IMG_FALSE;
}


static IMG_BOOL
DevMemoryAlloc (BM_CONTEXT *pBMContext,
				BM_MAPPING *pMapping,
				IMG_SIZE_T *pActualSize,
				IMG_UINT32 uFlags,
				IMG_UINT32 dev_vaddr_alignment,
				IMG_DEV_VIRTADDR *pDevVAddr)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
#ifdef PDUMP
	IMG_UINT32 ui32PDumpSize = pMapping->uSize;
#endif

	psDeviceNode = pBMContext->psDeviceNode;

	if(uFlags & PVRSRV_MEM_INTERLEAVED)
	{

		pMapping->uSize *= 2;
	}

#ifdef PDUMP
	if(uFlags & PVRSRV_MEM_DUMMY)
	{
		/* only one page behind a dummy allocation */
		ui32PDumpSize = pMapping->pBMHeap->sDevArena.ui32DataPageSize;
	}
#endif


	/* allocate device linear space */
	if (!psDeviceNode->pfnMMUAlloc (pMapping->pBMHeap->pMMUHeap,
									pMapping->uSize,
									pActualSize,
									0,
									dev_vaddr_alignment,
									&(pMapping->DevVAddr)))
	{
		PVR_DPF((PVR_DBG_ERROR, "DevMemoryAlloc ERROR MMU_Alloc"));
		return IMG_FALSE;
	}

#ifdef SUPPORT_SGX_MMU_BYPASS
	EnableHostAccess(pBMContext->psMMUContext);
#endif



	PDUMPMALLOCPAGES(psDeviceNode->sDevId.eDeviceType, pMapping->DevVAddr.uiAddr, pMapping->CpuVAddr, pMapping->hOSMemHandle, ui32PDumpSize, pMapping->pBMHeap->sDevArena.ui32DataPageSize, (IMG_HANDLE)pMapping);

	switch (pMapping->eCpuMemoryOrigin)
	{
		case hm_wrapped:
		case hm_wrapped_virtaddr:
		case hm_contiguous:
		{
			psDeviceNode->pfnMMUMapPages (	pMapping->pBMHeap->pMMUHeap,
							pMapping->DevVAddr,
							SysCpuPAddrToSysPAddr (pMapping->CpuPAddr),
							pMapping->uSize,
							uFlags,
							(IMG_HANDLE)pMapping);

			*pDevVAddr = pMapping->DevVAddr;
			break;
		}
		case hm_env:
		{
			psDeviceNode->pfnMMUMapShadow (	pMapping->pBMHeap->pMMUHeap,
							pMapping->DevVAddr,
							pMapping->uSize,
							pMapping->CpuVAddr,
							pMapping->hOSMemHandle,
							pDevVAddr,
							uFlags,
							(IMG_HANDLE)pMapping);
			break;
		}
		case hm_wrapped_scatter:
		case hm_wrapped_scatter_virtaddr:
		{
			psDeviceNode->pfnMMUMapScatter (pMapping->pBMHeap->pMMUHeap,
							pMapping->DevVAddr,
							pMapping->psSysAddr,
							pMapping->uSize,
							uFlags,
							(IMG_HANDLE)pMapping);

			*pDevVAddr = pMapping->DevVAddr;
			break;
		}
		default:
			PVR_DPF((PVR_DBG_ERROR,
				"Illegal value %d for pMapping->eCpuMemoryOrigin",
				pMapping->eCpuMemoryOrigin));
			return IMG_FALSE;
	}

#ifdef SUPPORT_SGX_MMU_BYPASS
	DisableHostAccess(pBMContext->psMMUContext);
#endif

	return IMG_TRUE;
}

static IMG_VOID
DevMemoryFree (BM_MAPPING *pMapping)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
#ifdef PDUMP
	IMG_UINT32 ui32PSize;
#endif

#ifdef PDUMP

	if(pMapping->ui32Flags & PVRSRV_MEM_DUMMY)
	{

		ui32PSize = pMapping->pBMHeap->sDevArena.ui32DataPageSize;
	}
	else
	{
		ui32PSize = pMapping->uSize;
	}

	PDUMPFREEPAGES(pMapping->pBMHeap,
                    pMapping->DevVAddr,
                    ui32PSize,
                    pMapping->pBMHeap->sDevArena.ui32DataPageSize,
                    (IMG_HANDLE)pMapping,
                    (pMapping->ui32Flags & PVRSRV_MEM_INTERLEAVED) ? IMG_TRUE : IMG_FALSE);
#endif

	psDeviceNode = pMapping->pBMHeap->pBMContext->psDeviceNode;

	psDeviceNode->pfnMMUFree (pMapping->pBMHeap->pMMUHeap, pMapping->DevVAddr, IMG_CAST_TO_DEVVADDR_UINT(pMapping->uSize));
}

static IMG_BOOL
BM_ImportMemory (IMG_VOID *pH,
			  IMG_SIZE_T uRequestSize,
			  IMG_SIZE_T *pActualSize,
			  BM_MAPPING **ppsMapping,
			  IMG_UINT32 uFlags,
			  IMG_UINTPTR_T *pBase)
{
	BM_MAPPING *pMapping;
	BM_HEAP *pBMHeap = pH;
	BM_CONTEXT *pBMContext = pBMHeap->pBMContext;
	IMG_BOOL bResult;
	IMG_SIZE_T uSize;
	IMG_SIZE_T uPSize;
	IMG_UINT32 uDevVAddrAlignment = 0;

	PVR_DPF ((PVR_DBG_MESSAGE,
			  "BM_ImportMemory (pBMContext=%08X, uRequestSize=0x%x, uFlags=0x%x, uAlign=0x%x)",
			  pBMContext, uRequestSize, uFlags, uDevVAddrAlignment));

	PVR_ASSERT (ppsMapping != IMG_NULL);
	PVR_ASSERT (pBMContext != IMG_NULL);

	if (ppsMapping == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_ImportMemory: invalid parameter"));
		goto fail_exit;
	}

	uSize = HOST_PAGEALIGN (uRequestSize);
	PVR_ASSERT (uSize >= uRequestSize);

	if (OSAllocMem(PVRSRV_OS_PAGEABLE_HEAP,
						sizeof (BM_MAPPING),
						(IMG_PVOID *)&pMapping, IMG_NULL,
						"Buffer Manager Mapping") != PVRSRV_OK)
	{
		PVR_DPF ((PVR_DBG_ERROR, "BM_ImportMemory: failed BM_MAPPING alloc"));
		goto fail_exit;
	}

	pMapping->hOSMemHandle = 0;
	pMapping->CpuVAddr = 0;
	pMapping->DevVAddr.uiAddr = 0;
	pMapping->CpuPAddr.uiAddr = 0;
	pMapping->uSize = uSize;
	pMapping->pBMHeap = pBMHeap;
	pMapping->ui32Flags = uFlags;

	/*
	 * If anyone want's to know, pass back the actual size of our allocation.
	 * There could be up to an extra page's worth of memory which will be marked
	 * as free in the RA.
	 */
	if (pActualSize)
	{
		*pActualSize = uSize;
	}

	/* if it's a dummy allocation only use one physical page */
	if(pMapping->ui32Flags & PVRSRV_MEM_DUMMY)
	{
		uPSize = pBMHeap->sDevArena.ui32DataPageSize;
	}
	else
	{
		uPSize = pMapping->uSize;
	}



	if(pBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG)
	{
		IMG_UINT32 ui32Attribs = pBMHeap->ui32Attribs;


		if (pMapping->ui32Flags & PVRSRV_HAP_CACHETYPE_MASK)
		{
			ui32Attribs &= ~PVRSRV_HAP_CACHETYPE_MASK;
			ui32Attribs |= (pMapping->ui32Flags & PVRSRV_HAP_CACHETYPE_MASK);
		}


		if (OSAllocPages(ui32Attribs,
						 uPSize,
						 pBMHeap->sDevArena.ui32DataPageSize,
						 (IMG_VOID **)&pMapping->CpuVAddr,
						 &pMapping->hOSMemHandle) != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,
					"BM_ImportMemory: OSAllocPages(0x%x) failed",
					uPSize));
			goto fail_mapping_alloc;
		}

		/* specify how page addresses are derived */
		pMapping->eCpuMemoryOrigin = hm_env;
	}
	else if(pBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG)
	{
		IMG_SYS_PHYADDR sSysPAddr;
		IMG_UINT32 ui32Attribs = pBMHeap->ui32Attribs;


		if (pMapping->ui32Flags & PVRSRV_HAP_CACHETYPE_MASK)
		{
			ui32Attribs &= ~PVRSRV_HAP_CACHETYPE_MASK;
			ui32Attribs |= (pMapping->ui32Flags & PVRSRV_HAP_CACHETYPE_MASK);
		}


		PVR_ASSERT(pBMHeap->pLocalDevMemArena != IMG_NULL);

		if (!RA_Alloc (pBMHeap->pLocalDevMemArena,
					   uPSize,
					   IMG_NULL,
					   IMG_NULL,
					   0,
					   pBMHeap->sDevArena.ui32DataPageSize,
					   0,
					   (IMG_UINTPTR_T *)&sSysPAddr.uiAddr))
		{
			PVR_DPF((PVR_DBG_ERROR, "BM_ImportMemory: RA_Alloc(0x%x) FAILED", uPSize));
			goto fail_mapping_alloc;
		}

		/* derive the CPU virtual address */
		pMapping->CpuPAddr = SysSysPAddrToCpuPAddr(sSysPAddr);
		if(OSReservePhys(pMapping->CpuPAddr,
						 uPSize,
						 ui32Attribs,
						 &pMapping->CpuVAddr,
						 &pMapping->hOSMemHandle) != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,	"BM_ImportMemory: OSReservePhys failed"));
			goto fail_dev_mem_alloc;
		}

		/* specify how page addresses are derived */
		pMapping->eCpuMemoryOrigin = hm_contiguous;
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR,	"BM_ImportMemory: Invalid backing store type"));
		goto fail_mapping_alloc;
	}


	bResult = DevMemoryAlloc (pBMContext,
								pMapping,
								IMG_NULL,
								uFlags,
								uDevVAddrAlignment,
								&pMapping->DevVAddr);
	if (!bResult)
	{
		PVR_DPF((PVR_DBG_ERROR,
				"BM_ImportMemory: DevMemoryAlloc(0x%x) failed",
				pMapping->uSize));
		goto fail_dev_mem_alloc;
	}



	PVR_ASSERT (uDevVAddrAlignment>1?(pMapping->DevVAddr.uiAddr%uDevVAddrAlignment)==0:1);

	*pBase = pMapping->DevVAddr.uiAddr;
	*ppsMapping = pMapping;

	PVR_DPF ((PVR_DBG_MESSAGE, "BM_ImportMemory: IMG_TRUE"));
	return IMG_TRUE;

fail_dev_mem_alloc:
	if (pMapping && (pMapping->CpuVAddr || pMapping->hOSMemHandle))
	{

		if(pMapping->ui32Flags & PVRSRV_MEM_INTERLEAVED)
		{
			pMapping->uSize /= 2;
		}

		if(pMapping->ui32Flags & PVRSRV_MEM_DUMMY)
		{
			uPSize = pBMHeap->sDevArena.ui32DataPageSize;
		}
		else
		{
			uPSize = pMapping->uSize;
		}

		if(pBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG)
		{
			OSFreePages(pBMHeap->ui32Attribs,
						  uPSize,
						  (IMG_VOID *)pMapping->CpuVAddr,
						  pMapping->hOSMemHandle);
		}
		else
		{
			IMG_SYS_PHYADDR sSysPAddr;

			if(pMapping->CpuVAddr)
			{
				OSUnReservePhys(pMapping->CpuVAddr,
								uPSize,
								pBMHeap->ui32Attribs,
								pMapping->hOSMemHandle);
			}
			sSysPAddr = SysCpuPAddrToSysPAddr(pMapping->CpuPAddr);
			RA_Free (pBMHeap->pLocalDevMemArena, sSysPAddr.uiAddr, IMG_FALSE);
		}
	}
fail_mapping_alloc:
	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_MAPPING), pMapping, IMG_NULL);

fail_exit:
	return IMG_FALSE;
}


/*!
******************************************************************************

	@Function   BM_FreeMemory

	@Description	Free a block of pages previously allocated via
				BM_ImportMemory.

	@Input      h - buffer manager handle, not the void type as dictated by
						 the generic nature of the resource allocator interface.
	@Input      _base - base address of blocks to free.
	@Input      psMapping - arbitrary user reference associated with the
					 underlying storage provided by BM_ImportMemory
	@Return 	None

 *****************************************************************************/
static IMG_VOID
BM_FreeMemory (IMG_VOID *h, IMG_UINTPTR_T _base, BM_MAPPING *psMapping)
{
	BM_HEAP *pBMHeap = h;
	IMG_SIZE_T uPSize;

	PVR_UNREFERENCED_PARAMETER (_base);

	PVR_DPF ((PVR_DBG_MESSAGE,
			  "BM_FreeMemory (h=%08X, base=0x%x, psMapping=0x%x)", h, _base, psMapping));

	PVR_ASSERT (psMapping != IMG_NULL);

	if (psMapping == IMG_NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "BM_FreeMemory: invalid parameter"));
		return;
	}

	DevMemoryFree (psMapping);


	if((psMapping->ui32Flags & PVRSRV_MEM_INTERLEAVED) != 0)
	{
		psMapping->uSize /= 2;
	}

	if(psMapping->ui32Flags & PVRSRV_MEM_DUMMY)
	{
		uPSize = psMapping->pBMHeap->sDevArena.ui32DataPageSize;
	}
	else
	{
		uPSize = psMapping->uSize;
	}

	if(pBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG)
	{
		OSFreePages(pBMHeap->ui32Attribs,
						uPSize,
						(IMG_VOID *) psMapping->CpuVAddr,
						psMapping->hOSMemHandle);
	}
	else if(pBMHeap->ui32Attribs & PVRSRV_BACKINGSTORE_LOCALMEM_CONTIG)
	{
		IMG_SYS_PHYADDR sSysPAddr;

		OSUnReservePhys(psMapping->CpuVAddr, uPSize, pBMHeap->ui32Attribs, psMapping->hOSMemHandle);

		sSysPAddr = SysCpuPAddrToSysPAddr(psMapping->CpuPAddr);

		RA_Free (pBMHeap->pLocalDevMemArena, sSysPAddr.uiAddr, IMG_FALSE);
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR,	"BM_FreeMemory: Invalid backing store type"));
	}

	OSFreeMem(PVRSRV_OS_PAGEABLE_HEAP, sizeof(BM_MAPPING), psMapping, IMG_NULL);


	PVR_DPF((PVR_DBG_MESSAGE,
			"..BM_FreeMemory (h=%08X, base=0x%x, psMapping=0x%x)",
			h, _base, psMapping));
}

/*!
******************************************************************************

 @Function	BM_GetPhysPageAddr

 @Description

 @Input		psMemInfo

 @Input		sDevVPageAddr

 @Output	psDevPAddr

 @Return	IMG_VOID

******************************************************************************/

IMG_VOID BM_GetPhysPageAddr(PVRSRV_KERNEL_MEM_INFO *psMemInfo,
								IMG_DEV_VIRTADDR sDevVPageAddr,
								IMG_DEV_PHYADDR *psDevPAddr)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;

	PVR_DPF((PVR_DBG_MESSAGE, "BM_GetPhysPageAddr"));

	PVR_ASSERT (psMemInfo && psDevPAddr)


	PVR_ASSERT((sDevVPageAddr.uiAddr & 0xFFF) == 0);

	/* PRQA S 0505 4 */ /* PVR_ASSERT should catch NULL ptrs */
	psDeviceNode = ((BM_BUF*)psMemInfo->sMemBlk.hBuffer)->pMapping->pBMHeap->pBMContext->psDeviceNode;

	*psDevPAddr = psDeviceNode->pfnMMUGetPhysPageAddr(((BM_BUF*)psMemInfo->sMemBlk.hBuffer)->pMapping->pBMHeap->pMMUHeap,
												sDevVPageAddr);
}


PVRSRV_ERROR BM_GetHeapInfo(IMG_HANDLE hDevMemHeap, PVRSRV_HEAP_INFO *psHeapInfo)
{
	BM_HEAP *psBMHeap = (BM_HEAP *)hDevMemHeap;

	PVR_DPF((PVR_DBG_VERBOSE, "BM_GetHeapInfo"));

	psHeapInfo->hDevMemHeap = hDevMemHeap;
	psHeapInfo->sDevVAddrBase = psBMHeap->sDevArena.BaseDevVAddr;
	psHeapInfo->ui32HeapByteSize = psBMHeap->sDevArena.ui32Size;
	psHeapInfo->ui32Attribs = psBMHeap->ui32Attribs;

	return PVRSRV_OK;
}


MMU_CONTEXT* BM_GetMMUContext(IMG_HANDLE hDevMemHeap)
{
	BM_HEAP *pBMHeap = (BM_HEAP*)hDevMemHeap;

	PVR_DPF((PVR_DBG_VERBOSE, "BM_GetMMUContext"));

	return pBMHeap->pBMContext->psMMUContext;
}

/*!
******************************************************************************
 @Function	BM_GetMMUContextFromMemContext

 @Description	utility function to return the MMU context

 @Input     hDevMemContext - the Dev mem context handle

 @Return	MMU context, else NULL
**************************************************************************/
MMU_CONTEXT* BM_GetMMUContextFromMemContext(IMG_HANDLE hDevMemContext)
{
	BM_CONTEXT *pBMContext = (BM_CONTEXT*)hDevMemContext;

	PVR_DPF ((PVR_DBG_VERBOSE, "BM_GetMMUContextFromMemContext"));

	return pBMContext->psMMUContext;
}

/*!
******************************************************************************
 @Function	BM_GetMMUHeap

 @Description	utility function to return the MMU heap handle

 @Input     hDevMemHeap - the Dev mem heap handle

 @Return	MMU heap handle, else NULL
**************************************************************************/
IMG_HANDLE BM_GetMMUHeap(IMG_HANDLE hDevMemHeap)
{
	PVR_DPF((PVR_DBG_VERBOSE, "BM_GetMMUHeap"));

	return (IMG_HANDLE)((BM_HEAP*)hDevMemHeap)->pMMUHeap;
}


/*!
******************************************************************************
 @Function	BM_GetDeviceNode

 @Description	utility function to return the devicenode from the BM Context

 @Input     hDevMemContext - the Dev Mem Context

 @Return	MMU heap handle, else NULL
**************************************************************************/
PVRSRV_DEVICE_NODE* BM_GetDeviceNode(IMG_HANDLE hDevMemContext)
{
	PVR_DPF((PVR_DBG_VERBOSE, "BM_GetDeviceNode"));

	return ((BM_CONTEXT*)hDevMemContext)->psDeviceNode;
}


/*!
******************************************************************************
 @Function	BM_GetMappingHandle

 @Description	utility function to return the mapping handle from a meminfo

 @Input     psMemInfo - kernel meminfo

 @Return	mapping handle, else NULL
**************************************************************************/
IMG_HANDLE BM_GetMappingHandle(PVRSRV_KERNEL_MEM_INFO *psMemInfo)
{
	PVR_DPF((PVR_DBG_VERBOSE, "BM_GetMappingHandle"));

	return ((BM_BUF*)psMemInfo->sMemBlk.hBuffer)->pMapping->hOSMemHandle;
}

/*!
******************************************************************************
 @Function	BM_MappingHandleFromBuffer

 @Description	utility function to get the BM mapping handle from a BM buffer

 @Input     hBuffer - Handle to BM buffer

 @Return	BM mapping handle
**************************************************************************/
IMG_HANDLE BM_MappingHandleFromBuffer(IMG_HANDLE hBuffer)
{
	BM_BUF *psBuffer;

	PVR_ASSERT(hBuffer != IMG_NULL);
	psBuffer = hBuffer;
	return psBuffer->pMapping;
}

/*!
******************************************************************************
 @Function	BM_GetVirtualSize

 @Description	utility function to get the VM size of a BM mapping

 @Input     hBMHandle - Handle to BM mapping

 @Return	VM size of mapping
**************************************************************************/
IMG_UINT32 BM_GetVirtualSize(IMG_HANDLE hBMHandle)
{
	BM_MAPPING *psMapping;

	PVR_ASSERT(hBMHandle != IMG_NULL);
	psMapping = hBMHandle;
	return psMapping->ui32ChunkSize * psMapping->ui32NumVirtChunks;
}

/*!
******************************************************************************
 @Function	BM_MapPageAtOffset

 @Description	utility function check if the specificed offset in a BM mapping
                is a page that needs tp be mapped

 @Input     hBMHandle - Handle to BM mapping

 @Input     ui32Offset - Offset into allocation

 @Return	IMG_TRUE if the page should be mapped
**************************************************************************/
IMG_BOOL BM_MapPageAtOffset(IMG_HANDLE hBMHandle, IMG_UINT32 ui32Offset)
{
	BM_MAPPING *psMapping;
	IMG_UINT32 ui32ChunkIndex;

	PVR_ASSERT(hBMHandle != IMG_NULL);
	psMapping = hBMHandle;

	ui32ChunkIndex = ui32Offset / psMapping->ui32ChunkSize;
	/* Check for overrun */
	PVR_ASSERT(ui32ChunkIndex <= psMapping->ui32NumVirtChunks);
	return psMapping->pabMapChunk[ui32ChunkIndex];
}

/*!
******************************************************************************
 @Function	BM_VirtOffsetToPhyscial

 @Description	utility function find of physical offset of a sparse allocation
                from it's virtual offset.

 @Input     hBMHandle - Handle to BM mapping

 @Input     ui32VirtOffset - Virtual offset into allocation
 
 @Output    pui32PhysOffset - Physical offset

 @Return	IMG_TRUE if the virtual offset is physically backed
**************************************************************************/
IMG_BOOL BM_VirtOffsetToPhysical(IMG_HANDLE hBMHandle,
								   IMG_UINT32 ui32VirtOffset,
								   IMG_UINT32 *pui32PhysOffset)
{
	BM_MAPPING *psMapping;
	IMG_UINT32 ui32ChunkOffset;
	IMG_UINT32 ui32PhysOffset = 0;
	IMG_UINT32 i;

	PVR_ASSERT(hBMHandle != IMG_NULL);
	psMapping = hBMHandle;

	ui32ChunkOffset = ui32VirtOffset / psMapping->ui32ChunkSize;
	if (!psMapping->pabMapChunk[ui32ChunkOffset])
	{
		return IMG_FALSE;
	}

	for (i=0;i<ui32ChunkOffset;i++)
	{
		if (psMapping->pabMapChunk[i])
		{
			ui32PhysOffset += psMapping->ui32ChunkSize;
		}
	}
	*pui32PhysOffset = ui32PhysOffset;

	return IMG_TRUE;
}
/******************************************************************************
 End of file (buffer_manager.c)
******************************************************************************/

/*
 *-----------------------------------------------------------------------------
 * Filename: memmap.h
 * $Revision: 1.6 $
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
 *  This file contains OS abstractions for memory mapping of bus addresses.
 *-----------------------------------------------------------------------------
 */

#ifndef _OAL_IO_MEMMAP_H
#define _OAL_IO_MEMMAP_H

void * os_map_io_to_mem_cache(
        unsigned long base_address,
        unsigned long size);
void * os_map_io_to_mem_nocache(
        unsigned long base_address,
        unsigned long size);
void os_unmap_io_from_mem(
        void * virt_addr,
        unsigned long size);


/*****************************************************************************
 * Function: os_map_io_to_mem_cache
 *
 * Description:
 *  This function will reserve a range of virtual memory space of "size" and
 *  map the that virtual address to the hardware "base_address" provided. This
 *  function call will enable caching of read write transactions to the region.
 * Parameters:
 *              IN: base_address -> the base bus/hardware address that will be
 *                                  used as the target of the memory mapping.
 *              IN: size-> the size of the virtual memory range requested to
 *                         be mapped
 * Return Value:
 *              NULL for failure OR
 *              valid virtual address casted as a void *
 *
 ****************************************************************************/
#define OS_MAP_IO_TO_MEM_CACHE(a, b)	os_map_io_to_mem_cache(a, b)

/*****************************************************************************
 * Function: os_map_io_to_mem_cache
 *
 * Description:
 *  This function will reserve a range of virtual memory space of "size" and
 *  map the that virtual address to the hardware "base_address" provided. This
 *  function call will NOT cache any read/write transactions to the region and
 *  apply the access directly to the hardware bus address that is mapped.
 * Parameters:
 *              IN: base_address -> the base bus/hardware address that will be
 *                                  used as the target of the memory mapping.
 *              IN: size-> the size of the virtual memory range requested to
 *                         be mapped
 * Return Value:
 *              NULL for failure OR
 *              valid virtual address casted as a void *
 *
 ****************************************************************************/
#define OS_MAP_IO_TO_MEM_NOCACHE(a, b)	os_map_io_to_mem_nocache(a, b)
#define OS_MAP_IO_TO_LARGE_MEM_NOCACHE(a, b) os_map_io_to_mem_nocache(a, b)

/*****************************************************************************
 * Function: os_map_io_to_mem_cache
 *
 * Description:
 *  This function will unmap the range of virtual memory space of "size" that
 *  was previously mapped with any of the above functions.
 * Parameters:
 *              IN: virt_address -> the base bus or hardware address that will
 *                                  be used as the target of the memory
 *                                  mapping.
 *              IN: size-> the size of the virtual memory range as requested in
 *                         os_map_io*
 * Return Value:
 *              none (N/A)
 *
 ****************************************************************************/
#define OS_UNMAP_IO_FROM_MEM(a, b)	os_unmap_io_from_mem(a, b)

#endif

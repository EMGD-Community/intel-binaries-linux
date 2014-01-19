/*
 *-----------------------------------------------------------------------------
 * Filename: memory.h
 * $Revision: 1.7 $
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
 *  This file contains OS abstracted interfaces to common memory operations.
 *-----------------------------------------------------------------------------
 */

#ifndef _OAL_MEMORY_H
#define _OAL_MEMORY_H

#include <linux/slab.h>

unsigned long os_gart_alloc_page( void );
unsigned long os_gart_virt_to_phys( unsigned char *a );
void os_gart_free_page( unsigned char *a );

/* #define INSTRUMENT_KERNEL_ALLOCS */
#ifdef INSTRUMENT_KERNEL_ALLOCS
#define MAX_FUNC_NAME 64

typedef struct _os_allocd_mem {
	void *ptr;
	unsigned int size;
	char function[MAX_FUNC_NAME];
	struct _os_allocd_mem *next;
} os_allocd_mem;

extern os_allocd_mem *list_head;
extern os_allocd_mem *list_tail;

static inline void *_os_alloc(unsigned int size, const char *function) {
	os_allocd_mem *mem;
	void *ptr = kmalloc(size, GFP_KERNEL);
	printk(KERN_DEBUG "%s OS_ALLOC(size=%u)=0x%p\n", function, size, ptr);
	mem = kmalloc(sizeof(os_allocd_mem), GFP_KERNEL);
	if (!ZERO_OR_NULL_PTR(mem)) {
		mem->ptr = ptr;
		mem->size = size;
		strncpy(mem->function, function, MAX_FUNC_NAME);
		mem->function[MAX_FUNC_NAME-1] = '\0';
		mem->next = NULL;
		if (NULL == list_tail) {
			list_head = mem;
		} else {
			list_tail->next = mem;
		}
		list_tail = mem;
	}
	return ptr;
}

static inline void _os_free(void *ptr, const char *function) {
	printk(KERN_DEBUG "%s OS_FREE(0x%p)\n", function, ptr);
	if (NULL != list_head) {
		os_allocd_mem *mem = list_head;
		os_allocd_mem *prev = NULL;
		while (NULL != mem) {
			if (mem->ptr == ptr) {
				if (mem == list_head) {
					list_head = mem->next;
					if (mem == list_tail) {
						list_tail = NULL;
					}
				} else {
					prev->next = mem->next;
					if (mem == list_tail) {
						list_tail = prev;
					}
				}
				kfree(mem);
				break;
			}
			prev = mem;
			mem = mem->next;
		}
	}
	kfree(ptr);
}

static inline void emgd_report_unfreed_memory(void) {
	os_allocd_mem *mem = list_head;
	os_allocd_mem *prev;

	printk(KERN_DEBUG "%s() REPORT ON NON-FREED MEMORY:\n", __FUNCTION__);
	while (NULL != mem) {
		printk(KERN_DEBUG "  addr=0x%p, size=%u, function=\"%s\"\n",
				mem->ptr, mem->size, mem->function);
		prev = mem;
		mem = mem->next;
		kfree(prev);
	}
}


/*!
 * void *OS_ALLOC(size_t size)
 *
 * OS_ALLOC is used by OS independent code to allocate system memory and
 * return a CPU writeable address to the allocated memory (Virtual address)
 * The returned address has no guarenteed alignment.
 * size should be <= 4k for larger sizes use OS_ALLOC_LARGE().
 *
 * Allocations returned from OS_ALLOC() should be freed with OS_FREE().
 *
 * All Full OAL implementations must implement the _OS_ALLOC entry point
 * to enable use of this function.
 *
 * @return NULL on Failure
 * @return Virtual or Flat address on Success
 */
#define OS_ALLOC(a) _os_alloc(a, __FUNCTION__)

/*!
 * void OS_FREE(void *p)
 * OS_FREE should be used to free allocations returned from OS_ALLOC()
 *
 * All Full OAL implementations must implement the _OS_FREE entry point
 * to enable use of this function.
 */
#define OS_FREE(a) _os_free(a, __FUNCTION__)

/*!
 * void *OS_ALLOC_LARGE(size_t size)
 *
 * OS_ALLOC_LARGE is used by OS independent code to allocate system memory
 * in the same manner as OS_ALLOC except that size must be > 4k.
 *
 * Allocations returned from OS_ALLOC_LARGE() should be freed with
 * OS_FREE_LARGE().
 *
 * All Full OAL implementations must implement the _OS_ALLOC_LARGE entry point
 * to enable use of this function. This entry point may be implemented
 * exactly the same as _OS_ALLOC is no diferentiation is required.
 *
 * @return NULL on Failure
 * @return Virtual or Flat address on Success
 */
#define OS_ALLOC_LARGE(a) _os_alloc(a, __FUNCTION__)

/*!
 * void OS_FREE_LARGE(void *p)
 * OS_FREE_LARGE should be used to free allocations returned from
 * OS_ALLOC_LARGE()
 *
 * All Full OAL implementations must implement the _OS_FREE_LARGE entry point
 * to enable use of this function. This entry point may be implemented
 * exactly the same as _OS_FREE is no diferentiation is required.
 *
 */
#define OS_FREE_LARGE(a) _os_free(a, __FUNCTION__)

#else /* INSTRUMENT_KERNEL_ALLOCS */

#define OS_ALLOC(a) kmalloc((a), GFP_KERNEL)
#define OS_FREE(a) kfree(a)
#define OS_ALLOC_LARGE(a) kmalloc((a), GFP_KERNEL)
#define OS_FREE_LARGE(a) kfree(a)

#endif /* INSTRUMENT_KERNEL_ALLOCS */

#define OS_ALLOC_PAGE() NULL
/*!
 * void *OS_VIRT_TO_PHYS( void *p )
 *
 * OS_VIRT_TO_PHYS is used by OS independent code to obtain the physical
 * address referenced by the virtual address p. The virtual address must be
 * one returned by OS_ALLOC_PAGE or OS_ALLOC_CONTIGUOUS.
 *
 * This entry point is OPTIONAL. Only OAL implementations that have
 * implemented the _OS_ALLOC_PAGE or _OS_ALLOC_CONTIGUOUS macros need
 * implement the _OS_VIRT_TO_PHYS macro. OS independent code that must
 * function on all implementation may not use this entry point.
 *
 * @returns Physical Address
 */
#define OS_VIRT_TO_PHYS(a) os_gart_virt_to_phys(a)

/*!
 * void OS_FREE_PAGE(void *p)
 * OS_FREE_PAGE should be used to free allocations returned from
 * OS_ALLOC_PAGE()
 *
 * This entry point is OPTIONAL. Only OAL implementations that have implemented
 * the _OS_ALLOC_PAGE macro need implement the _OS_FREE_PAGE macro.
 */
#define OS_FREE_PAGE(a) os_gart_free_page(a)

#define OS_MEMSET(a,b,c) memset(a,b,c)
#define OS_MEMCPY(a,b,c) memcpy(a,b,c)
#define OS_MEMCMP(a,b,c) memcmp(a,b,c)

#define OS_OFFSETOF(t,m) offsetof(t,m)


/*
 * void *OS_MEMSET(void *s, int c, size_t n)
 *
 * OS_MEMSET sets all bytes of the memory area referenced by address s and
 * size n to the char value c.
 *
 * ALL Full OAL implementations must implement the entry point _OS_MEMSET
 * to enable use of this function.
 *
 * @returns Address s
 */
#ifndef OS_MEMSET
#define OS_MEMSET(a,b,c) _oal_memset(a,b,c)
#endif

/*
 * void *OS_MEMCPY(void *dest, void *src, size_t n)
 *
 * OS_MEMCPY copies n bytes from the memory referenced by src to the
 * memory referenced by dest. The areas may not overlap.
 *
 * ALL Full OAL implementations must implement the entry point _OS_MEMCPY
 * to enable use of this function.
 *
 * @returns Address dest
 */
#ifndef OS_MEMCPY
#define OS_MEMCPY(a,b,c) _oal_memcpy(a,b,c)
#endif

/*
 * void *OS_MEMCMP(void *s1, void *s2, size_t n)
 *
 * OS_MEMCMP compares n bytes from the memory referenced by s1 to the
 * corresponding bytes referenced by s2.
 *
 * This entry point is available in all full OAL implementations. An OAL
 * may implement _OS_MEMCMP macro or the built-in version will be used.
 *
 * @returns < 0 if the s1 value is less than s2
 * @returns > 0 if the s1 value is greater than s2
 * @returns 0 if the values are equal
 */
#ifndef OS_MEMCMP
#define OS_MEMCMP(a,b,c) _oal_memcmp(a,b,c)
#endif


/*
 * void *OS_MEMZERO(void *s, size_t n)
 *
 * OS_MEMZERO sets all bytes of the memory area referenced by address s and
 * size n to 0.
 *
 * This entry point is available in all full OAL implementations. An OAL
 * may implement _OS_MEMZERO macro or the built-in version making use of
 * of _OS_MEMSET will be used.
 *
 * @returns Address s
 */
#ifndef OS_MEMZERO
#define OS_MEMZERO(a,b)  OS_MEMSET(a, 0, b)
#endif

/*
 * void *OS_MEMZERO(void *s, size_t n)
 *
 * OS_MEMZERO sets all bytes of the memory area referenced by address s and
 * size n to 0.
 *
 * This entry point is available in all full OAL implementations. An OAL
 * may implement _OS_MEMZERO macro or the built-in version making use of
 * of _OS_MEMSET will be used.
 *
 * @returns Address s
 */
#ifndef OS_STRNCPY
#define OS_STRNCPY(d, s, n)  _oal_strncpy(d, s, n)
#endif





/*!
 * void *OS_ALLOC_CONTIGUOUS( size_t n, size_t align )
 *
 * OS_ALLOC_CONTIGUOUS is used by OS independent code to allocate a number
 * of aligned system memory pages and return a CPU writeable address to the
 * allocated memory (Virtual address) The returned address must point to
 * physically contiguous memory aligned to the requested alignment.
 *
 * Allocations returned from OS_ALLOC_CONTIGUOUS() should be freed with
 * OS_FREE_CONTIGUOUS().
 *
 * This entry point is OPTIONAL. Only OAL implementations that have the
 * ability to allocate such pages need implement the _OS_ALLOC_CONTIGUOUS
 * macro. OS independent code that must function on all implementation may
 * not use this entry point.
 *
 * @return NULL on Failure
 * @return Virtual or Flat address on Success
 */
#define OS_ALLOC_CONTIGUOUS(a,b) _OS_ALLOC_CONTIGUOUS(a,b)
/*!
 * void OS_FREE_CONTIGUOUS(void *p)
 * OS_FREE_CONTIGUOUS should be used to free allocations returned from
 * OS_ALLOC_CONTIGUOUS()
 *
 * This entry point is OPTIONAL. Only OAL implementations that have implemented
 * the _OS_ALLOC_CONTIGUOUS macro need implement the _OS_FREE_CONTIGUOUS macro.
 */
#define OS_FREE_CONTIGUOUS(a) _OS_FREE_CONTIGUOUS(a)

/*!
 * size_t OS_OFFSETOF(type, member)
 *
 * OS_OFFSETOF is used by OS independent code to obtain the offset of a
 * given member within a type.
 *
 * @returns size_t of the offset of member m within type t
 */
#ifndef OS_OFFSETOF
#define OS_OFFSETOF(t,m) ((size_t)&(((t *)0)->m))
#endif

/*
 * This is a OS independent helper is case the operating environment
 * does not supply a memcmp() function. The OAL may use this implementation.
 */
static __inline int _oal_memcmp(const void *s1, const void *s2, unsigned long n)
{
	const unsigned char *cs1 = (const unsigned char *) s1;
	const unsigned char *cs2 = (const unsigned char *)s2;

	for ( ; n-- > 0; cs1++, cs2++) {
		if (*cs1 != *cs2) {
			return *cs1 - *cs2;
		}
	}
	return 0;
}

static __inline void *_oal_memcpy(void *dest, const void *src, size_t n)
{
	size_t i;

	i=0;
	while( i < n ) {
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
		i++;
	}
	return dest;
}

static __inline char *_oal_strncpy(char *dest, const char *src, size_t n)
{
	size_t i;

	for(i=0; i<n; i++) {
		if(!(dest[i] = src[i])) {
			for(i=i; i<n; i++) {
				dest[i] = '\0';
			}
			return dest;
		}
	}
	return dest;
}

static __inline void *_oal_memset(void *s, int c, size_t n)
{
	unsigned int i;
	for(i=0; i<n; i++) {
		((unsigned char *)s)[i] = (unsigned char)c;
	}
	return s;
}

/*
 * These macros are optional for the OAL port. They are used to do memory
 * management for virtual apterture space process.
 */

#ifdef _OS_MAP_STOLEN_MEM
#define OS_MAP_STOLEN_MEM(a, b, c)        _OS_MAP_STOLEN_MEM(a, b, c)
#else
#define OS_MAP_STOLEN_MEM(a, b, c)        0
#endif

#ifdef _OS_VIRT_APERT_AVAILABLE
#define OS_VIRT_APERT_AVAILABLE()         _OS_VIRT_APERT_AVAILABLE()
#else
#define OS_VIRT_APERT_AVAILABLE()         0
#endif

#ifdef _OS_GET_VIRT_APERT_BASE
#define OS_GET_VIRT_APERT_BASE()          _OS_GET_VIRT_APERT_BASE()
#else
#define OS_GET_VIRT_APERT_BASE()          NULL
#endif

#endif

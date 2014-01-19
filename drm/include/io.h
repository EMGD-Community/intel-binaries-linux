/*
 *-----------------------------------------------------------------------------
 * Filename: io.h
 * $Revision: 1.8 $
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
 *  This file contains Linux user-space implementations for the OAL io.h
 *  abstractions.
 *-----------------------------------------------------------------------------
 */

#include <linux/kernel.h>
#include <asm/io.h>
#include <config.h>
#include <igd_debug.h>


#ifndef _OAL_LINUX_KERNEL_IO_H
#define _OAL_LINUX_KERNEL_IO_H

extern unsigned long _os_read_port8(unsigned long port);
extern void _os_write_port8(unsigned long port, unsigned char value);
extern unsigned long _os_read_port16(unsigned long port);
extern void _os_write_port16(unsigned long port, unsigned short value);
extern unsigned long _os_read_port32(unsigned long port);
extern void _os_write_port32(unsigned long port, unsigned long value);

/*
 * Debug macros rely on the existence of emgd_debug pointer. It is defined
 * in the HAL. If an IAL wishes to use the print routines and is not
 * linked with the HAL it will need to provide an emgd_debug pointer.
 */

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef FAR
#define FAR
#endif


/*
 * All files including io.h should define MODULE_NAME to be a bit
 * within the emgd_debug_t data structure. If MODULE_NAME is not defined
 * debug printing will be controlled by the global "debug" bit rather
 * than a per-module bit.
 */
#ifndef MODULE_NAME
#define MODULE_NAME hal.debug
#endif

/*
 * Debug Print Macros use os_get_funcname to get the name of the
 * function they are in. They cannot rely on __func__ alone because
 * the function name may be overridden by the caller.
 *
 * If the caller wants to override the printed function name they may
 * call os_set_funcname prior to each call to EMGD_DEBUG/ERROR/TRACE.
 */
static const char *_os_override_funcname = NULL;

static __inline int os_set_funcname( const char *name )
{
	_os_override_funcname = name;
	return 1;
}

static __inline const char *os_get_funcname( const char *name )
{
	const char *ret;
	ret = (_os_override_funcname)?_os_override_funcname:name;
	_os_override_funcname = NULL;
	return ret;
}



/*
 * EMGD_ERROR()
 * Printing with EMGD_ERROR will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_ERROR printing is used for
 * driver errors that should not happen under normal operation. As such,
 * these messages cannot be disabled at runtime.
 *
 * All OAL implementations should result in EMGD_ERROR messsages in the
 * following format:
 * <OPTIONAL OS ERROR PREFIX> FUNCTION_NAME Message
 *
 * The FUNCTION_NAME must be obtained by calling
 * os_get_funcname(__func__) to insure that any overridden function names
 * are respected.
 */
#ifndef EMGD_ERROR
#ifdef DEBUG_BUILD_TYPE
#define EMGD_ERROR(...)							\
  do {									\
    printk(KERN_ERR "%s ERROR: ", __FUNCTION__);			\
    printk(__VA_ARGS__);						\
    printk("\n");							\
    /*fflush(stderr);*/							\
  } while(0);
#else
#define EMGD_ERROR(...)  do {} while(0)
#endif
#endif


/* The CONFIG_USE_INFO_PRIORITY macro is #define'd in "config.h" */
#ifdef CONFIG_USE_INFO_PRIORITY
#define EMGD_DEBUG_MSG_PRIORITY KERN_INFO
#else
#define EMGD_DEBUG_MSG_PRIORITY KERN_DEBUG
#endif

/*
 * EMGD_DEBUG()
 * Printing with EMGD_DEBUG will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_DEBUG printing is used for
 * debug messages that give information useful for debugging problems.
 * EMGD_DEBUG messages can be enabled/disabled at compile time and runtime
 * on a per-module basis.
 *
 * Disabling/Enabling module bits in CONFIG_DEBUG_FLAGS will control the
 *  default printing of debug messages.
 * Disabling/Enabling the bit in emgd_debug->MODULE_NAME will control
 *  the runtime printing of debug messages.
 *
 * All OAL implementations should result in EMGD_DEBUG messsages in the
 * following format:
 * <OPTIONAL OS PREFIX> function_name Message
 *
 * The FUNCTION_NAME must be obtained by calling
 * os_get_funcname(__func__) to insure that any overridden function names
 * are respected.
 */
#ifndef EMGD_DEBUG
#ifdef DEBUG_BUILD_TYPE
#define EMGD_DEBUG(...) if(emgd_debug &&emgd_debug-> MODULE_NAME)	\
  do {													\
    printk(EMGD_DEBUG_MSG_PRIORITY "[EMGD_DEBUG] %s ", __FUNCTION__);	\
    printk(__VA_ARGS__);								\
    printk("\n");										\
    /*fflush(stdout);*/									\
  } while(0);
#else
#define EMGD_DEBUG(...)  do {} while(0)
#endif
#endif

/*
 * EMGD_DEBUG_S()
 * Printing with EMGD_DEBUG_S will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_DEBUG_S printing is used for
 * debug messages that give information useful for debugging problems in
 * a shortened form (used by the HAL for multi-line prints).
 * EMGD_DEBUG_S messages can be enabled/disabled at compile time and runtime
 * on a per-module basis.
 *
 * Disabling/Enabling module bits in CONFIG_DEBUG_FLAGS will control the
 *  default printing of debug messages.
 * Disabling/Enabling the bit in emgd_debug->MODULE_NAME will control
 *  the runtime printing of debug messages.
 *
 * All OAL implementations should result in EMGD_DEBUG_S messsages in the
 * following format:
 * <OPTIONAL OS PREFIX> Message
 *
 * The FUNCTION_NAME must be obtained by calling
 * os_get_funcname(__func__) to insure that any overridden function names
 * are respected.
 */
#ifndef EMGD_DEBUG_S
#ifdef DEBUG_BUILD_TYPE
#define EMGD_DEBUG_S(...) if(emgd_debug && emgd_debug-> MODULE_NAME) 	\
  do {													\
    printk(EMGD_DEBUG_MSG_PRIORITY __VA_ARGS__);			\
    printk("\n");										\
    /* fflush(stdout);*/								\
  } while(0);
#else
#define EMGD_DEBUG_S(...)     do {} while(0)
#endif
#endif

/*
 * EMGD_TRACE_ENTER
 * Tracing with EMGD_TRACE_ENTER will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_TRACE_ENTER will print a fixed
 * "Enter" message when entering a function.
 * EMGD_TRACE_ENTER messages can be enabled/disabled at compile time and
 * runtime on a per-module basis. To Enable tracing in a module, the Global
 * emgd_debug->trace bit must be enabled as well as the per-module
 * debug bit.
 *
 * Disabling/Enabling module bits in CONFIG_DEBUG_FLAGS will control the
 *  default printing of debug messages.
 * Disabling/Enabling the bit in emgd_debug->MODULE_NAME will control
 *  the runtime printing of debug messages.
 *
 * All OAL implementations should result in EMGD_TRACE_ENTER messsages in the
 * following format:
 * <OPTIONAL OS PREFIX> function_name ENTER
 */
#ifndef EMGD_TRACE_ENTER
#ifdef DEBUG_BUILD_TYPE
#define EMGD_TRACE_ENTER if(emgd_debug && emgd_debug->hal.trace) EMGD_DEBUG("ENTER")
#else
#define EMGD_TRACE_ENTER
#endif
#endif

/*
 * EMGD_TRACE_EXIT
 * Tracing with EMGD_TRACE_EXIT will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_TRACE_EXIT will print a fixed
 * "Exit" message when exiting a function without error.
 * EMGD_TRACE_EXIT messages can be enabled/disabled at compile time and
 * runtime on a per-module basis. To Enable tracing in a module, the Global
 * emgd_debug->trace bit must be enabled as well as the per-module
 * debug bit.
 *
 * Disabling/Enabling module bits in CONFIG_DEBUG_FLAGS will control the
 *  default printing of debug messages.
 * Disabling/Enabling the bit in emgd_debug->MODULE_NAME will control
 *  the runtime printing of debug messages.
 *
 * All OAL implementations should result in EMGD_TRACE_EXIT messsages in the
 * following format:
 * <OPTIONAL OS PREFIX> function_name EXIT
 */
#ifndef EMGD_TRACE_EXIT
#ifdef DEBUG_BUILD_TYPE
#define EMGD_TRACE_EXIT if(emgd_debug && emgd_debug->hal.trace) EMGD_DEBUG("EXIT")
#else
#define EMGD_TRACE_EXIT
#endif
#endif

/*
 * EMGD_ERROR_EXIT()
 * Tracing with EMGD_ERROR_EXIT will be compiled in to all debug drivers and
 * removed from all production drivers. EMGD_ERROR_EXIT will print an error
 * as well as a fixed "Exit" message when exiting a function without error.
 * EMGD_ERROR_EXIT messages can be enabled/disabled at compile time and
 * runtime on a per-module basis. To Enable tracing in a module, the Global
 * emgd_debug->trace bit must be enabled as well as the per-module
 * debug bit.
 *
 * Note: Only the Tracing message can be disabled, the error message will
 * still print as with EMGD_ERROR().
 *
 * Disabling/Enabling module bits in CONFIG_DEBUG_FLAGS will control the
 *  default printing of debug messages.
 * Disabling/Enabling the bit in emgd_debug->MODULE_NAME will control
 *  the runtime printing of debug messages.
 *
 * All OAL implementations should result in EMGD_ERROR_EXIT messsages in the
 * following format:
 * <OPTIONAL OS ERROR PREFIX> function_name EXIT Message
 */
#ifndef EMGD_ERROR_EXIT
#ifdef DEBUG_BUILD_TYPE
#define EMGD_ERROR_EXIT													\
	if(emgd_debug && emgd_debug->hal.trace) EMGD_DEBUG("EXIT With Error..."); \
	EMGD_ERROR
#else
#define EMGD_ERROR_EXIT(...)   do {} while(0)
#endif
#endif


/*
 * EMGD_VERBOSE()
 * Verbose printing that is useful only when debugging specific problems
 * or contains lots of text should be implemented in a single function and
 * called from within the EMGD_VERBOSE macro. The default and runtime
 * control of this output can be altered by changing the value of the
 * global emgd_debug->verbose.
 */
#ifndef EMGD_VERBOSE
#ifdef DEBUG_BUILD_TYPE
#define EMGD_VERBOSE(opt, func) if(emgd_debug-> opt) func;
#else
#define EMGD_VERBOSE(opt, func)
#endif
#endif

/*
 * EMGD_ASSERT( Statement, Message, Error Code)
 * EMGD_ASSERT should be used to verify the values of statements that
 * should always be true. EMGD_ASSERT will generate code only in debug drivers
 * and therefore should only be used for things that should never be false
 * in production drivers. For example, testing for null parameters in
 * an internal interface.
 *
 * Usage: EMGD_ASSERT(char_ptr, "Char Pointer is NULL", -IGD_ERROR_INVAL)
 * Usage: EMGD_ASSERT(char_ptr, "Char Pointer is NULL", )
 */
#ifndef EMGD_ASSERT
#ifdef EMGD_DEBUG
#define EMGD_ASSERT(a, m, e) if(!(a)) { EMGD_ERROR_EXIT("ASSERT: " m); return e; }
#else
#define EMGD_ASSERT(a, m, e)
#endif
#endif

/*
 * This macro _may_ be defined by an OAL port to modify the mmio
 * base address prior to use. Commonly this is used to make the base
 * address 0 to remove code for vbios.
 */
#ifdef _EMGD_MMIO
#define EMGD_MMIO(a) _EMGD_MMIO(a)
#else
#define EMGD_MMIO(a) a
#endif



#define EMGD_READ32(addr) *(volatile unsigned int *)(addr)
#define EMGD_WRITE32(value, addr) \
		(*(volatile unsigned long *)(addr) = (value))
/*	EMGD_DEBUG ("EMGD_WRITE32: 0x%p=0x%lx\n", (addr), (value)); \*/

#define EMGD_READ8(addr) *(volatile unsigned char *)(addr)
#define EMGD_WRITE8(value, addr) \
		(*(volatile unsigned char *)(addr) = (value))
/*	EMGD_DEBUG ("EMGD_WRITE8: 0x%p=0x%x\n", (addr), (value));	\*/


/* PORT IO Macros */
#if 0
// user mode.  To be removed.
#define _EMGD_READ_PORT8(port)          _os_read_port8(port)
#define _EMGD_WRITE_PORT8(port, value)  _os_write_port8(port, value)

#define _EMGD_READ_PORT16(port)         _os_read_port16(port)
#define _EMGD_WRITE_PORT16(port, value) _os_write_port16(port, value)

#define _EMGD_READ_PORT32(port)         _os_read_port32(port)
#define _EMGD_WRITE_PORT32(port, value) _os_write_port32(port, value)
#else
// Linux kernel mode port I/O
#define EMGD_READ_PORT8(port)          inb(port)
#define EMGD_WRITE_PORT8(port, value)  outb(value, port)

#define EMGD_READ_PORT16(port)         inw(port)
#define EMGD_WRITE_PORT16(port, value) outw(value, port)

#define EMGD_READ_PORT32(port)         inl(port)
#define EMGD_WRITE_PORT32(port, value) outl(value, port)
#endif

#ifdef DEBUG_MEM
extern unsigned long global_virt_mmadr;
extern unsigned long global_virt_mmadr_offset;

static __inline *(volatile unsigned int *) _EMGD_READ32(unsigned int addr)
{
    if ((addr < 0x10100000) && (addr > 0x10000000)) {
		return *(volatile unsigned int *)(addr+global_virt_mmadr-global_virt_mmadr_offset);
	} else {
		return *(volatile unsigned int *)(addr);
    }
}

static __inline void _EMGD_WRITE32(unsigned int value, unsigned int addr) \
{
	if ((addr < 0x10100000) && (addr > 0x10000000)) {
		*(volatile unsigned int *)(addr+global_virt_mmadr-global_virt_mmadr_offset) = value;
	} else {
		*(volatile unsigned int *)addr = value;
	}
	printf("EMGD_WRITE32: *0x%x = 0x%x\n", addr, value);
}

static __inline *(volatile unsigned char *) _EMGD_READ8(unsigned int addr)
{
    if ((addr < 0x10100000) && (addr > 0x10000000)) {
		return *(volatile unsigned char *)(addr+global_virt_mmadr-global_virt_mmadr_offset);
	} else {
		return *(volatile unsigned char *)addr;
	}
}

static __inline void _EMGD_WRITE8(unsigned char value, unsigned int addr) \
{
    if ((addr < 0x10100000) && (addr > 0x10000000)) {
		(*(volatile unsigned char *)(addr+global_virt_mmadr-global_virt_mmadr_offset) = value);
    } else {
		(*(volatile unsigned char *)(addr) = value);
    }
    printf("EMGD_WRITE8: *0x%x = 0x%c\n", addr, value);
}
#endif /* DEBUG_MEM */

#define EMGD_PTR_TO_ULONG(ptr) (ptr)
#define EMGD_ULONG_TO_PTR(ulong) (ulong)

#endif

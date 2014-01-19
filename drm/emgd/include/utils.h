/*
 *-----------------------------------------------------------------------------
 * Filename: utils.h
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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <config.h>
#include <igd_mode.h>
#include <general.h>
#include <mode.h>

/* Get the MMIO pointer for a display context */
#define MMIO(dsp) \
 EMGD_MMIO((((igd_display_context_t *)dsp)->context->device_context.virt_mmadr))

/* Get the 2nd MMIO pointer for a display context */
#define MMIO_SDVO(dsp) \
 EMGD_MMIO(((igd_display_context_t *)dsp)->context->device_context.virt_mmadr_sdvo)

/* Get the STMicro SDVO MMIO pointer for a display context */
#define MMIO_SDVO_ST(dsp) \
 EMGD_MMIO(((igd_display_context_t *)dsp)->context->device_context.virt_mmadr_sdvo_st)
#define MMIO_SDVO_ST_GPIO(dsp) \
 EMGD_MMIO(((igd_display_context_t *)dsp)->context->device_context.virt_mmadr_sdvo_st_gpio)

/* Definition: value = READ_MMIO_REG(igd_display_context_t *, reg) */
#define READ_MMIO_REG(dsp, reg) \
	EMGD_READ32( MMIO(dsp) +(reg) )

/* Definition: WRITE_MMIO_REG(igd_display_context_t *, reg, value) */
#define WRITE_MMIO_REG(dsp, reg, value) \
	EMGD_WRITE32(value, ( MMIO(dsp) + (reg) ) )

/* Definition:WRITE_MMIO_REG_BITS(igd_display_context_t *, reg, value, mask) */
#define WRITE_MMIO_REG_BITS(dsp, reg, data, mask) \
{                                                 \
	unsigned long tmp;                            \
	tmp = READ_MMIO_REG((dsp), (reg));            \
	tmp &= (~(mask));                             \
	tmp |= ((data) & (mask));                     \
	WRITE_MMIO_REG((dsp), (reg), (tmp));          \
}

#define PORT_TYPE(d) (PORT(d, (d->port_number))->port_type)
#define PORT_TYPE_DH(dh) \
	(PORT(dh, (((igd_display_context_t *)dh)->port_number))->port_type)

#ifdef CONFIG_TNC
/* Based on display port determine which mmio base to use:
 *       port_type == SDVO ==> use 0:3:0 device mmio
 *       port_type == LVDS ==> use 0:2:0 device mmio
 *       port_type == LPC  ==> use 0:31:0 device mmio
 */
#define MMIO_TNC(port_type) EMGD_MMIO(get_mmio_tnc(port_type))

#define READ_MMIO_REG_TNC(pt, reg) read_mmio_reg_tnc(pt, reg)
#define WRITE_MMIO_REG_TNC(pt, reg, value) write_mmio_reg_tnc(pt, reg, value)

/* Defined in micro_mode_tnc.c */
extern unsigned char *get_mmio_tnc(unsigned long port_type);
extern unsigned long read_mmio_reg_tnc(unsigned long port_type,
	unsigned long reg);
extern void write_mmio_reg_tnc(unsigned long port_type, unsigned long reg,
	unsigned long value);
#endif


/*
 * These are temporary macros used only within this header.
 * Individual config options use these macros to generate macros that look
 * like this:
 *
 * If CONFIG_FOO is defined
 *  OPT_FOO_SYMBOL(a)
 *  OPT_FOO_VALUE(a, b)
 *  OPT_FOO_VOID_CALL(fn)
 *  OPT_FOO_CALL(fn)
 */
#define OPTIONAL_VOID_CALL(fn) fn
#define OPTIONAL_CALL(fn)			\
	{								\
		int __ret;					\
		__ret = fn;					\
		if(__ret) {					\
			EMGD_ERROR_EXIT("EXIT");	\
			return __ret;			\
		}							\
	}

#define OPTIONAL_CALL_RET(ret, fn)      (ret) = (fn)

/*
 * Debug call macro should be used to call debug printing functions
 * that will only exist in debug builds.
 */
#ifdef DEBUG_BUILD_TYPE
#define OPT_DEBUG_SYMBOL(sym)      sym
#define OPT_DEBUG_VALUE(val, alt)  val
#define OPT_DEBUG_VOID_CALL(fn)    OPTIONAL_VOID_CALL(fn)
#define OPT_DEBUG_CALL(fn)         OPTIONAL_CALL(fn)
#define OPT_DEBUG_INLINE
#else
#define OPT_DEBUG_SYMBOL(sym)
#define OPT_DEBUG_VALUE(val, alt)  alt
#define OPT_DEBUG_VOID_CALL(fn)
#define OPT_DEBUG_CALL(fn)
#define OPT_DEBUG_INLINE           static __inline
#endif

/*
 * Micro Symbols are only used when CONFIG_MICRO is not defined.
 */
#ifndef CONFIG_MICRO
#define OPT_MICRO_SYMBOL(sym)         sym
#define OPT_MICRO_VALUE(val, alt)     val
#define OPT_MICRO_VOID_CALL(fn)       OPTIONAL_VOID_CALL(fn)
#define OPT_MICRO_CALL(fn)            OPTIONAL_CALL(fn)
#define OPT_MICRO_CALL_RET(ret, fn)   OPTIONAL_CALL_RET(ret, fn)
#else
#define OPT_MICRO_SYMBOL(sym)
#define OPT_MICRO_VALUE(val, alt)  alt
#define OPT_MICRO_VOID_CALL(fn)
#define OPT_MICRO_CALL(fn)
#define OPT_MICRO_CALL_RET(ret, fn)
#endif


#ifdef CONFIG_PLB
#define DISPATCH_PLB(p) {PCI_DEVICE_ID_VGA_PLB, p},
#else
#define DISPATCH_PLB(p)
#endif
#ifdef CONFIG_TNC
#define DISPATCH_TNC(p) {PCI_DEVICE_ID_VGA_TNC, p},
#define DISPATCH_TNC_A0(p) {PCI_DEVICE_ID_VGA_TNC_A0, p},
#define DISPATCH_LNC(p) {PCI_DEVICE_ID_VGA_LNC, p},
#else
#define DISPATCH_TNC(p)
#define DISPATCH_TNC_A0(p)
#define DISPATCH_LNC(p)
#endif
#define DISPATCH_END {0, NULL}


#endif // _UTILS_H_


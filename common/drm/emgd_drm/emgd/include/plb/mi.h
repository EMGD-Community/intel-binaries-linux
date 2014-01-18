/*
 *-----------------------------------------------------------------------------
 * Filename: mi.h
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
 *  This is a header file for the Intel GFX commands.
 *  This includes commands specific to Intel hardware and structures specific
 *  to Intel hardware.  All other commands and structures are available
 *  through GFX.
 *-----------------------------------------------------------------------------
 */

#ifndef _MI_H
#define _MI_H

#include <igd_mode.h>
#include <igd_render.h>

/* FIXME: This file has not been checked for PLB. */

#define MI_FLUSH_OPTIONS_MASK 0x0000000f
#define MI_FLUSH_MAP          0x00000001
#define MI_FLUSH_RENDER       0x00000004
/* #define MI_FLUSH_END_SCENE    0x00000008  Not available on PLB? */
/* #define MI_FLUSH_WRITE_DIRTY  0x00000010  Not available on PLB? */

extern int mi_display_buffer_info_plb(igd_display_h display_h,
	int priority,
	unsigned long flags);

extern int mi_wait_scan_priority_arb_on_off_plb(igd_command_t *addr,
	int priority, int enable);

extern int mi_wait_scan_lines_excl_plb(igd_display_h display_h,
	int priority,
	unsigned long start,
	unsigned long end);

extern int mi_wait_scan_lines_incl_plb(igd_display_h display_h,
	int priority,
	unsigned long start,
	unsigned long end);

extern int mi_wait_vblank_plb(igd_display_h display_h,
	int priority);

extern int mi_flush_plb(igd_display_h display_h, int priority,
	unsigned int flush_options, unsigned int flags);

extern int mi_wait_for_scan_plb(igd_display_h display_h, int priority,
	igd_rect_t *dest_rect);

extern int mi_user_interrupt_plb(igd_display_h display_h, int priority);

#endif /* _MI_H */

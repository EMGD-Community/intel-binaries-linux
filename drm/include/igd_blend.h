/*
 *-----------------------------------------------------------------------------
 * Filename: igd_blend.h
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
 *  This is a header file for the Intel GFX commands.
 *  This includes commands specific to Intel hardware and structures specific
 *  to Intel hardware.  All other commands and structures are available
 *  through GFX.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_BLEND_H
#define _IGD_BLEND_H

#include <igd_render.h>

/* These flags must not conflict with the IGD_RENDER flags in igd_render.h */
#define IGD_FILTER_MASK             0x00000300
#define IGD_FILTER_NEAREST          0x00000000
#define IGD_FILTER_BILINEAR         0x00000100
#define IGD_FILTER_ANISOTROPIC      0x00000200
#define IGD_FILTER_4X4              0x00000300
/* Indicates multiple blends will be sent in a row and this is not the
 * last blend. */
#define IGD_BLEND_NO_FLUSH          0x00000800

/*----------------------------------------------------------------------
 * Function:
 *  int dispatch->blend(igd_display_h display_handle,
 *    int priority,
 *    igd_appcontext_h appcontext,
 *    unsigned int num_src,
 *    igd_surface_t **src_surface_list,
 *    igd_rect_t **src_rect_list,
 *    igd_rect_t **dest_rect_list,
 *    igd_surface_t *dest_surface,
 *    igd_rect_t *clip_rect,
 *    unsigned long flags);
 *
 * Description:
 *
 * Parameters:
 *
 *  flags: Flags are defined in igd_render.h
 *
 * Returns:
 *----------------------------------------------------------------------
 */

#endif /* _IGD_BLEND_H */

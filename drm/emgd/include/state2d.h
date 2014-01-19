/*
 *-----------------------------------------------------------------------------
 * Filename: state2d.h
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
 *  This file is the header file for manipulating the 2D State variables.
 *-----------------------------------------------------------------------------
 */

#ifndef _STATE2D_H
#define _STATE2D_H

/* IGD */
#include <igd_render.h>

typedef struct _state2d {
    igd_rect_t dest_rect;
	unsigned int clip_status;
} state2d_t;

#define STATE2D(ac) ((state2d_t *)ac->state2d)

#define STATE2D_SET_CLIP(ac, rect)                            \
if(ac) {                                                      \
    OS_MEMCPY(&((state2d_t *)(ac)->state2d)->dest_rect, rect, \
		sizeof(igd_rect_t));                                  \
}
 /*
  * Checking for the 2D appcontext instead of the appcontext
  * pointer because the state2d could be null during botched
  * allocation.
  */
#define STATE2D_GET_CLIP(ac) \
	(((ac) && ((state2d_t *)(ac)->state2d))? \
	(&((state2d_t *)(ac)->state2d)->dest_rect):NULL)

#define STATE2D_GET_CLIP_STATUS(ac) \
	(((ac) && ((state2d_t *)(ac)->state2d))? \
	(((state2d_t *)(ac)->state2d)->clip_status):0)

#define STATE2D_SET_CLIP_STATUS(ac, s) \
	(((ac) && ((state2d_t *)(ac)->state2d))? \
	(((state2d_t *)(ac)->state2d)->clip_status = s):0)

#endif /*_STATE2D_H */



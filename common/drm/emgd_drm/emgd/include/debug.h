/*
 *-----------------------------------------------------------------------------
 * Filename: debug.h
 * $Revision: 1.9 $
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
 *  Contains debug macros
 *-----------------------------------------------------------------------------
 */
#ifndef __DEBUG_H
#define __DEBUG_H



#define IGD_PRINTK_PALETTE(p) \
{ \
	EMGD_DEBUG("Palette Struct:"); \
	EMGD_DEBUG_S("    [%4d].id",    (unsigned int) p->palette_id); \
	EMGD_DEBUG_S("    [%4d].type",  (unsigned int) p->palette_type); \
	EMGD_DEBUG_S("    [%4d].size",  (unsigned int) p->size); \
}

#define IGD_PRINTK_3DSTRETCH(p, s, d, f) \
{ \
	EMGD_DEBUG("Ring Buffer Struct:"); \
	EMGD_DEBUG_S("    Priority = [%8d]",       (unsigned int) p); \
	EMGD_DEBUG_S("    Flags    = [%08x]",      (unsigned int) f); \
	EMGD_DEBUG_S("    --------------"); \
	EMGD_DEBUG_S("    Source surface"); \
	EMGD_DEBUG_S("    --------------"); \
	EMGD_DEBUG_S("    [%08x].addr",            (unsigned int) s->addr); \
	EMGD_DEBUG_S("    [%8d].height",           (unsigned int) s->height); \
	EMGD_DEBUG_S("    [%8d].width",            (unsigned int) s->width); \
	EMGD_DEBUG_S("    [%8d].pitch",            (unsigned int) s->pitch); \
	EMGD_DEBUG_S("    [%08x].pixel_format",    (unsigned int) s->pixel_format); \
	EMGD_DEBUG_S("    -------------------"); \
	EMGD_DEBUG_S("    Destination surface"); \
	EMGD_DEBUG_S("    -------------------"); \
	EMGD_DEBUG_S("    [%8d].offset",           (unsigned int) d->offset); \
	EMGD_DEBUG_S("    [%8d].pitch",            (unsigned int) d->pitch); \
	EMGD_DEBUG_S("    [%8d].x1",               (unsigned int) d->x1); \
	EMGD_DEBUG_S("    [%8d].y1",               (unsigned int) d->y1); \
	EMGD_DEBUG_S("    [%8d].x2",               (unsigned int) d->x2); \
	EMGD_DEBUG_S("    [%8d].y2",               (unsigned int) d->y2); \
	EMGD_DEBUG_S("    [%08x].pixel_format",    (unsigned int) d->pixel_format); \
	EMGD_DEBUG_S("    [%08x].byte_mask",       (unsigned int) d->byte_mask); \
}

#define IGD_PRINT_RB_BUFFER(a) \
{ \
	EMGD_DEBUG("Ring Buffer Struct:"); \
	EMGD_DEBUG_S("    [%08x].id",              (unsigned int) a->id); \
	EMGD_DEBUG_S("    [%08x].size",            (unsigned int) a->size); \
	EMGD_DEBUG_S("    [%08x].addr",            (unsigned int) a->addr); \
	EMGD_DEBUG_S("    [%p].virt",             a->virt); \
	EMGD_DEBUG_S("    [%p].start  = [%08x]",  a->start, (unsigned int) EMGD_READ32(a->start)); \
	EMGD_DEBUG_S("    [%p].head   = [%08x]",  a->head,  (0x001ffffc & ((unsigned int) EMGD_READ32(a->head)))); \
	EMGD_DEBUG_S("    [%p].tail   = [%08x]",  a->tail,  (0x001ffff8 & (unsigned int) EMGD_READ32(a->tail))); \
	EMGD_DEBUG_S("    [%p].ctrl   = [%08x]",  a->ctrl,  (unsigned int) EMGD_READ32(a->ctrl)); \
	EMGD_DEBUG_S("    [%08x].res",             (unsigned int) a->reservation); \
}

#define IGD_PRINTK_DISPLAY(display_context) \
{ \
	EMGD_DEBUG("Display Context:"); \
	EMGD_DEBUG_S("    [%8d].plane",  (int) display_context->plane); \
	EMGD_DEBUG_S("    [%8d].blend",  (int) display_context->blend_operation); \
	EMGD_DEBUG_S("    [%8d].pipe",   (int) display_context->pipe); \
	EMGD_DEBUG_S("    [%8d].port",   (int) display_context->port); \
	EMGD_DEBUG_S("    [%8d].graph",  (int) display_context->alloc_type); \
	EMGD_DEBUG_S("    [%8d].mode_n", (int) display_context->mode_number); \
	EMGD_DEBUG_S("    [%8d].ref",    (int) display_context->refresh); \
	EMGD_DEBUG_S("    [%8d].enable", (int) display_context->enable); \
	EMGD_DEBUG_S("    [%8d].ulPipe", (int) display_context->ulPipe); \
	EMGD_DEBUG_S("    [%8d].ulMode", (int) display_context->ulModeNumber); \
	EMGD_DEBUG_S("    [%8d].ulRef",  (int) display_context->ulRefresh); \
}

#define IGD_PRINTK_FBINFO(fb_info) \
{ \
	EMGD_DEBUG("FrameBuffer Info:"); \
	EMGD_DEBUG_S("    [%8d].width",  (int) fb_info->width); \
	EMGD_DEBUG_S("    [%8d].height",  (int) fb_info->height); \
	EMGD_DEBUG_S("    [%8d].screen_pitch",  (int) fb_info->screen_pitch); \
	EMGD_DEBUG_S("    [%8x].fb_base_offset",  (unsigned int) fb_info->fb_base_offset); \
	EMGD_DEBUG_S("    [%8x].pixel_format",  (unsigned int) fb_info->pixel_format); \
}

#define IGD_PRINTK_PTINFO(pt_info) \
{ \
	EMGD_DEBUG("Port Info:"); \
	EMGD_DEBUG_S("    [%8d].width",  (int) pt_info->width); \
	EMGD_DEBUG_S("    [%8d].height",  (int) pt_info->height); \
	EMGD_DEBUG_S("    [%8d].x_offset",  (int) pt_info->x_offset); \
	EMGD_DEBUG_S("    [%8d].y_offset",  (int) pt_info->y_offset); \
	EMGD_DEBUG_S("    [%8d].refresh",  (int) pt_info->refresh); \
	EMGD_DEBUG_S("    [%8d].hsync_start",  (unsigned int) pt_info->hsync_start); \
	EMGD_DEBUG_S("    [%8d].hsync_end",  (unsigned int) pt_info->hsync_end); \
	EMGD_DEBUG_S("    [%8d].hblank_start",  (unsigned int) pt_info->hblank_start); \
	EMGD_DEBUG_S("    [%8d].hblank_end",  (unsigned int) pt_info->hblank_end); \
	EMGD_DEBUG_S("    [%8d].vsync_start",  (unsigned int) pt_info->vsync_start); \
	EMGD_DEBUG_S("    [%8d].vsync_end",  (unsigned int) pt_info->vsync_end); \
	EMGD_DEBUG_S("    [%8d].vblank_start",  (unsigned int) pt_info->vblank_start); \
	EMGD_DEBUG_S("    [%8d].vblank_end",  (unsigned int) pt_info->vblank_end); \
	EMGD_DEBUG_S("    [%8x].flags",  (unsigned int) pt_info->flags); \
}

#define IGD_PRINTK_FBINFO_2(a,b) \
{ \
	EMGD_DEBUG("FrameBuffer Info:"); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].width",  (int) (a)->width, (int) (b)->width); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].height",  (int) (a)->height, (int) (b)->height); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].screen_pitch",  (int) (a)->screen_pitch, (int) (b)->screen_pitch); \
	EMGD_DEBUG_S("    a=[%8x] b=[%8x].fb_base_offset",  (unsigned int) (a)->fb_base_offset, (unsigned int) (b)->fb_base_offset); \
	EMGD_DEBUG_S("    a=[%8x] b=[%8x].pixel_format",  (unsigned int) (a)->pixel_format, (unsigned int) (b)->pixel_format); \
}

#define IGD_PRINTK_PTINFO_2(a,b) \
{ \
	EMGD_DEBUG("Port Info:"); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].width",  (int) a->width, (int) b->width); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].height",  (int) a->height, (int) b->height); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].x_offset",  (int) a->x_offset, (int) b->x_offset); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].y_offset",  (int) a->y_offset, (int) b->y_offset); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].refresh",  (int) a->refresh, (int) b->refresh); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].hsync_start",  (unsigned int) a->hsync_start, (unsigned int) b->hsync_start); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].hsync_end",  (unsigned int) a->hsync_end, (unsigned int) b->hsync_end); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].hblank_start",  (unsigned int) a->hblank_start, (unsigned int) b->hblank_start); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].hblank_end",  (unsigned int) a->hblank_end, (unsigned int) b->hblank_end); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].vsync_start",  (unsigned int) a->vsync_start, (unsigned int) b->vsync_start); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].vsync_end",  (unsigned int) a->vsync_end, (unsigned int) b->vsync_end); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].vblank_start",  (unsigned int) a->vblank_start, (unsigned int) b->vblank_start); \
	EMGD_DEBUG_S("    a=[%8d] b=[%8d].vblank_end",  (unsigned int) a->vblank_end, (unsigned int) b->vblank_end); \
	EMGD_DEBUG_S("    a=[%8x] b=[%8x].flags",  (unsigned int) a->flags, (unsigned int) b->flags); \
}

#define IGD_PRINT_ICH_GPIO(ich_gpio_base) \
{ \
	EMGD_DEBUG("GPIO_USE_SEL = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x0)); \
	EMGD_DEBUG("GPIO_IO_SEL = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x4)); \
	EMGD_DEBUG("GPIO_LVL = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0xc)); \
	EMGD_DEBUG("GPIO_INV = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x2c)); \
	EMGD_DEBUG("GPIO_USE_SEL2 = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x30));\
	EMGD_DEBUG("GPIO_IO_SEL2 = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x34)); \
	EMGD_DEBUG("GPIO_LVL2 = 0x%lx", EMGD_READ_PORT32(ich_gpio_base + 0x38)); \
}

#endif

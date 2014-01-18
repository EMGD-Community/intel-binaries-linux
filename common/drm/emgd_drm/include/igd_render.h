/*
 *-----------------------------------------------------------------------------
 * Filename: igd_render.h
 * $Revision: 1.18 $
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

#ifndef _IGD_RENDER_H
#define _IGD_RENDER_H

#include <igd_mode.h>
#include <igd_appcontext.h>

/*
 * Flags passed to 2d and 3d render functions.
 * Note: These flags MUST not conflict the the blend flags in igd_blend.h
 */
#define IGD_RENDER_BLOCK       0x00000000
#define IGD_RENDER_NONBLOCK    0x00000001
/* Render to Framebuffer: Causes a flush afterward */
#define IGD_RENDER_FRAMEBUFFER 0x00000002
/* Should this be an immediate blit? */
#define IGD_RENDER_IMMEDIATE   0x00000004
/* Render should prevent on-screen tearing */
#define IGD_RENDER_NOTEAR      0x00000008

/*
 * Command priority for use with 2d and 3d render functions.
 */
#define IGD_PRIORITY_NORMAL    0x0
#define IGD_PRIORITY_INTERRUPT 0x1
#define IGD_PRIORITY_POWER     0x2
#define IGD_PRIORITY_BIN       0x3
#define IGD_PRIORITY_BB        0x4


typedef struct _igd_yuv_coeffs {
	char ry;
	char ru;
	char rv;
   	char gy;
	char gu;
	char gv;
	char by;
	char bu;
	char bv;

	short r_const;
	short g_const;
	short b_const;

	unsigned char r_shift;
	unsigned char g_shift;
	unsigned char b_shift;
} igd_yuv_coeffs, *pigd_yuv_coeffs;

typedef struct _igd_palette_info {
	unsigned long *palette;
	int palette_id;
	int palette_type;
	int size;

	igd_yuv_coeffs yuv_coeffs;
} igd_palette_info_t, *pigd_palette_info_t;

/*
 *
 * 2D blit => igd_coord + igd_surface => igd_rect + igd_surface
 * 3D blend => (igd_rect + igd_surface + igd_rect)*n => igd_rect + igd_surface
 *   Blend: N source surfaces with a source rect.
 *          N destination rects.
 *          1 Destination surface.
 *          1 Destination clip rect.
 */

typedef struct _igd_coord {
	unsigned int x;
	unsigned int y;
} igd_coord_t, *pigd_coord_t;

typedef struct _igd_rect {
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
} igd_rect_t, *pigd_rect_t;

/*
 * Direct Display context for video-to-graphics bridge.
 * (Added here for compatibility between User & Kernel space)
 * Note: This structure must match EMGDHmiVideoContext.
 */
typedef struct _igd_dd_context {
	unsigned int    usage;		/* requested display plane */
	unsigned int    screen;		/* screen index for video output */
	unsigned int    screen_w;	/* width of display screen */
	unsigned int    screen_h;	/* height of display screen */
	unsigned int    video_w;	/* width of IOH video frame */
	unsigned int    video_h;	/* height of IOH video frame */
	unsigned int	video_pitch; /* pitch of IOH video frame */
	igd_rect_t      src;		/* video input source rectangle */
	igd_rect_t      dest;		/* display output dest rectangle */
} igd_dd_context_t, *pigd_dd_context_t;

/* planes where reconfiguration and Z-ordering is supported */
typedef enum _igd_plane_usage {
	IGD_PLANE_NONE,
	IGD_PLANE_HMI,
	IGD_PLANE_X11,
	IGD_PLANE_OVERLAY_VIDEO,
	IGD_PLANE_OVERLAY_POPUP,
	IGD_PLANE_SPRITE_VIDEO,
	IGD_PLANE_SPRITE_POPUP,
} igd_plane_usage_t;

typedef struct _igd_buffer_config {
	igd_plane_usage_t plane;	/* usage/ownership of this plane */
	unsigned long     offset;	/* surface's GTT offset */
	int               stride;	/* surface's stride (in bytes) */
	igd_rect_t        src;		/* input source rectangle */
	igd_rect_t        dest;		/* display destination rectangle */
	int               extended_screen;
	int               alpha_ena;	/* enables/disables constant alpha */
	int               alpha_val;	/* constant alpha opacity value */
	int               ckey_ena;	/* enables/disables color keying */
	int               ckey_val;	/* source color key value */
} igd_buffer_config_t, *pigd_buffer_config_t;

typedef struct _igd_surface {
	unsigned long offset;
	unsigned int pitch;
	unsigned int width;
	unsigned int height;
	unsigned long u_offset;     /* Needed for YUV Planar modes. */
	unsigned int u_pitch;      /* Needed for YUV Planar modes. */
	unsigned long v_offset;     /* Needed for YUV Planar modes. */
	unsigned int v_pitch;      /* Needed for YUV Planar modes. */
	unsigned long pixel_format;
	igd_palette_info_t *palette_info;
	unsigned long flags;        /* GMM Alignment flags (igd_gmm.h) */
	unsigned int logic_ops;
	unsigned int render_ops;
	unsigned int alpha;        /* Global Alpha:
								*  Surface is modulated by alpha prior
								*  to blend. Used with IGD_RENDER_OP_ALPHA.
								*  Used exclusive of diffuse which contains
								*  a full ARGB value.
								*/
	unsigned int diffuse;      /* Diffuse Color:
								*   Use with IGD_RENDER_OP_DIFFUSE.
								*  Used with alpha-only surfaces or
								*  constants color surfaces.
								*/
	unsigned long chroma_high; /* Drop pixels between low and high */
	unsigned long chroma_low;
	unsigned char *virt_addr;   /* Virtual memory address to be used for
								 * either Sys Mem to Vid Mem BLT operations
								 * or for per surface virtual memory.
								 */
	void *pvr2d_mem_info;    /* memory information, intended to be used only
								* by kernel, that allows access to surface memory
								* from variety of device memory perpectives.
								*/

	void *pvr2d_context_h;   /* TODO: This needs to be moved to the appcontext and needs to be
								accessible by blend */

	unsigned long FlipChainID;
	void * hPVR2DFlipChain;

	/* TODO: remove floating point use from code used by kernel.  who needs this??? */
	/* We do!! This is how these parameters are passed in to blend.
	   They are not processed on kernel side. */
	float proc_amp_const[6];     /* Video ProcAmpControl parameter */
} igd_surface_t, *pigd_surface_t;

typedef struct _igd_surface_list {
    unsigned long offset;
    unsigned long size;
} igd_surface_list_t, *pigd_surface_list_t;

/*!
 * @name Surface Render Ops
 * @anchor render_ops
 *
 * These flags apply to a surface and indicate features of the surface
 * that should be considered during render operations such as
 * _igd_dispatch::blend()
 *
 * - IGD_RENDER_OP_BLEND: This surface should be blended with other
 *  surfaces during rendering. In calls to _igd_dispatch::blend() this
 *  surface will blend with those below it in the stack or the destination
 *  surface (when the surface is the bottom-most in the stack).
 *   Note: xRGB surfaces used with this render op will convert to ARGB
 *   surfaces with an alpha of 1.0. Without this render op xRGB surfaces
 *   will preserve the x value from the source.
 *
 * - IGD_RENDER_OP_PREMULT: The surface contains a pre-multipled alpha
 *  value. This indicates that the per-pixel color values have already been
 *  multipled by the per-pixel alpha value. Without this bit set it is
 *  necessary to perform this multiplication as part of a blend operation.
 *
 * - IGD_RENDER_OP_ALPHA: The surface contains a global alpha. All pixel
 *  values are multiplied against the global alpha as part of any blend
 *  operation. Global alpha is used exclusive of Diffuse color.
 *
 * - IGD_RENDER_OP_DIFFUSE: The surface contains a single diffuse color.
 *  This flag is used with alpha-only surfaces or surfaces of constant
 *  color. Diffuse is used exclusively of Global Alpha.
 *
 * - IGD_RENDER_OP_CHROMA: THe surface contains a chroma-key. Pixel
 *  values between  chroma_high and chroma_low will be filtered out.
 *
 * - IGD_RENDER_OP_COLORKEY: The surface contains a colorkey. This is used
 *  on mask surfaces to indicate the location of pixels that may be drawn
 *  on the destination. Only pixels with values between chroma_high and
 *  chroma_low should "pass through" the mask to the destination. Other pixels
 *  will be filered out.
 *
 * @{
 */

#define IGD_RENDER_OP_BLEND      0x1
#define IGD_RENDER_OP_PREMULT    0x2
#define IGD_RENDER_OP_ALPHA      0x4
#define IGD_RENDER_OP_DIFFUSE    0x8
#define IGD_RENDER_OP_CHROMA     0x20
#define IGD_RENDER_OP_COLORKEY   0x40

/*
 * Make sure rotate and flip flags are not moved. Blend uses them as
 * array indexes
 */
#define IGD_RENDER_OP_ROT_MASK   0x000300
#define IGD_RENDER_OP_ROT_0      0x000000
#define IGD_RENDER_OP_ROT_90     0x000100
#define IGD_RENDER_OP_ROT_180    0x000200
#define IGD_RENDER_OP_ROT_270    0x000300
#define IGD_RENDER_OP_FLIP       0x000400
#define IGD_RENDER_OP_SKIP_ROT	 0x000800
#define IGD_RENDER_OP_SKIP_FLIP	 0x008000

/*
 * flags for post process - Deinterlacing and ProcAmpControl.
 */
#define IGD_RENDER_OP_INTERLACED 0x00001000
#define IGD_RENDER_OP_BOB_EVEN   0x00002000
#define IGD_RENDER_OP_BOB_ODD    0x00004000
#define IGD_RENDER_OP_PROCAMP    0x00010000

/*
 * Region structure. Holds information about non-surface memory
 * allocations.
 */

#define IGD_REGION_UNFINISHED 0x00000000
#define IGD_REGION_READY      0x00000001
#define IGD_REGION_QUEUED     0x00000002
#define IGD_REGION_ABANDONED  0x00000004

typedef struct _igd_region {
	unsigned long offset;
	char *virt;
	unsigned long size;
	unsigned long type;
	unsigned long flags;
} igd_region_t;
/* @} */

typedef struct _igd_dma {
	unsigned long offset;
	char *virt;
	unsigned long size;
	unsigned long head;
	unsigned long flags;
	/* unsigned long phys;
		- currently not used - maybe needed in future */
} igd_dma_t;

/*
 * Functions for setting/getting the surface information for the
 * Display, Color, Depth, and Aux buffers for a display.
 */
typedef enum _igd_buffertype {
	IGD_BUFFER_DISPLAY = 0,
	IGD_BUFFER_COLOR,
	IGD_BUFFER_DEPTH,
	IGD_BUFFER_SAVE
} igd_buffertype_t;

/*----------------------------------------------------------------------
 * Typedef: _igd_get_suface_fn_t
 *
 * Description:
 *  This function, available from the top-level igd_dispatch_t as
 *  get_surface(), allows the client to query the parameters for
 *  the display, color, depth, and aux buffers for a given display.
 *
 * Parameters:
 *  display_handle - Handle for the requested display.
 *
 *  type - One of the igd_buffertype_t enums.
 *
 *  surface - An igd_surface_t to return the information in.
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_get_surface_fn_t)(
	igd_display_h display_handle,
	igd_buffertype_t type,
	igd_surface_t *surface,
	igd_appcontext_h appcontext);

/*----------------------------------------------------------------------
 * Typedef: _igd_set_suface_fn_t
 *
 * Description:
 *  This function, available from the top-level igd_dispatch_t as
 *  set_surface(), allows the client to set the parameters for
 *  the display, color, and depth buffers for a given display.
 *
 *  This command will happen asynchronously when possible. It will
 *  be placed in the command stream and will happen at some point
 *  after the command is parsed (The next vblank if it is the display
 *  buffer)
 *
 *  When setting the depth buffer the setting will not take place
 *  until the next color buffer set. Therefore when changing the
 *  color buffer the depth buffer should always be changed first.
 *
 *  If set_surface has been used to change the displayed surface, the
 *  original framebuffer should be replaced before changing modes.
 *
 * Parameters:
 *  display_handle - Handle for the requested display.
 *
 *  priority - The priority for the command.
 *
 *  type - One of the igd_buffertype_t enums.
 *
 *  flags - Additional bitfield of options for buffer.
 *
 *  surface - An igd_surface_t that contains the parameters to apply.
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
#define IGD_BUFFER_ZERO_BIAS 0x1
#define IGD_BUFFER_FIXED_Z   0x2
#define IGD_BUFFER_ASYNC     0x4
/* Issue a Wait for last flip only, do not flip again */
#define IGD_BUFFER_WAIT      0x8
/* Do not pan */
#define IGD_BUFFER_NO_PAN		 0x10

/* ------ WARNING!!!! Read Before Changing ------ */
/*    Command should always be 32 Bit on all platforms including 64 bit */
/*    Int is assumed to be 32 bit on all the supported platforms */
typedef unsigned int igd_command_t;

typedef int (*_igd_set_surface_fn_t)(
	igd_display_h display_handle,
	int priority,
	igd_buffertype_t type,
	igd_surface_t *surface,
	igd_appcontext_h appcontext,
	unsigned long flags);

typedef enum _igd_event {
	IGD_EVENT_FLIP_PENDING = 1
} igd_event_t;

typedef int (*_igd_query_event_fn_t)(
	igd_display_h display_handle,
	igd_event_t event,
	unsigned long *status);

/*----------------------------------------------------------------------
 * Typedef: _igd_alloc_ring_fn_t
 *
 * Description:
 *
 * Parameters:
 *  display_handle - Handle for the requested display.
 *
 *  flags - IGD_ALLOC_PRIORITY_*
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_alloc_ring_fn_t)(
	igd_display_h display_handle,
	unsigned long flags);


/*----------------------------------------------------------------------
 * Typedef: _igd_exec_buffer_fn_t
 *
 * Description:
 *  This function allows a privelidged IAL to place instructions directly
 * into the HAL's command queue. Under normal circumstances an IAL does not
 * have the knowledge necessary to do this; however, some IALs (D3D, OGL, MC)
 * will have this knowledge. The contents referenced by the data parameter
 * are device dependent and may be a data structure or other structured
 * information.
 *
 * Parameters:
 *  display_handle - Handle for the requested display.
 *
 *  priority - The priority for the command.
 *
 *  data - Pointer to the device dependent command data or data strucutre.
 *
 *  size - Size in Dwords (4bytes).
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_exec_buffer_fn_t)(
	igd_display_h display_handle,
	int priority,
	igd_appcontext_h appcontext,
	const void *data,
	unsigned long size);

/*----------------------------------------------------------------------
 * Typedef: _igd_query_buffer_fn_t
 *
 * Description:
 * This function allows a privelidged IAL to query Topaz/MSVDX.
 * Under normal circumstances an IAL does not have the knowledge
 * necessary to do this; however, some IALs (D3D, OGL, MC)
 * will have this knowledge. The contents referenced by the data parameter
 * are device dependent and may be a data structure or other structured
 * information.
 *
 * Parameters:
 *  in_size - in parameter size.
 *
 *  in_buffer - input buffer.
 *
 *  out_size - out parameter size.
 *
 *  out_buffer - output buffer.
 *
 *  command - query command.
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_query_buffer_fn_t) (
	igd_display_h display_handle,
	void *in_buffer,
	void *out_buffer,
	unsigned long command);

/*
 * Flags passed to (exclusive) igd_rb_reserve
 * and igd_rb_update functions.
 */
#define IGD_RB_RESERVE_BLOCK     0x00000000
#define IGD_RB_RESERVE_NONBLOCK  0x00000001
#define IGD_RB_CACHELINE_PAD     0x00000002

/*----------------------------------------------------------------------
 * Typedef: _igd_rb_reserve_fn_t
 *
 * Description:
 *  This function allows a privelidged IAL to aquire space directly on
 * the HAL's ring buffer. Under normal circumstances an IAL does not have
 * the knowledge necessary to do this; however, some IALs (D3D, OGL, MC)
 * will have this knowledge.
 *  Any IAL making use of this interface must have full knowledge of all
 * hardware interactions, padding etc that are needed to safely access
 * the ring directly.
 *
 * Parameters:
 *  display_handle - Handle for the requested display.
 *
 *  priority - The priority for the command.
 *
 *  size - Size in Dwords (4bytes).
 *    Setting size to 0 will re-initialize the ring.
 *
 *  block - Blocking or not 1 or 0
 *
 *  addr - Virtual address returned from the call. Caller should begin
 *   writing commands at this address.
 *
 *  avail - The amount (in dwords) of space that is available in the
 *   ring. This is just a hint in case it is helpful to the IAL.
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_rb_reserve_fn_t)(
	igd_display_h display_handle,
	int priority,
	unsigned long size,
	unsigned long flags,
	igd_command_t **addr,
	unsigned long *avail);

/*----------------------------------------------------------------------
 * Typedef: _igd_rb_update_fn_t
 *
 * Description:
 *  This function updates the ring pointer after a call to rb_reserve,
 * and after the command data has been placed in the ring.
 *
 * Parameters:
 *  display_handle - Handle for the requested display. Must be the same
 *   display used with rb_reserve.
 *
 *  priority - The priority for the command. Must be the same priority
 *   used with rb_reserve.
 *
 *  addr - Virtual address after placing the commands into the ring. This
 *   should be equal to the returned addr + size from rb_reserve.
 *
 * Returns:
 *  0: Success
 *  < 0: Error
 *----------------------------------------------------------------------
 */
typedef int (*_igd_rb_update_fn_t)(
	igd_display_h display_handle,
	int priority,
	igd_command_t *addr,
	unsigned long flags);


#endif /* _IGD_RENDER_H */

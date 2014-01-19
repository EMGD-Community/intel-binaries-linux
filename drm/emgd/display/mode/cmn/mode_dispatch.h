/*
 *-----------------------------------------------------------------------------
 * Filename: mode_dispatch.h
 * $Revision: 1.17 $
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
 *  This header file defined the interface between the DI layer of the mode
 *  module and the DD layer. Additionally it defines the interface between
 *  the different DI object files within the mode module.
 *-----------------------------------------------------------------------------
 */

#ifndef _MODE_DISPATCH_H
#define _MODE_DISPATCH_H

#include <mode.h>



typedef struct _mode_kms_dispatch {
	void (*kms_program_pipe) (emgd_crtc_t *emgd_crtc);
	void (*kms_set_pipe_pwr) (emgd_crtc_t *emgd_crtc, unsigned long enable);
	void (*kms_program_plane)(emgd_crtc_t *emgd_crtc, unsigned long status);
	void (*kms_set_plane_pwr)(emgd_crtc_t *emgd_crtc, unsigned long enable);
	int  (*kms_program_port) (emgd_encoder_t *emgd_encoder,
		unsigned long status);
	int  (*kms_post_program_port)(emgd_encoder_t *emgd_encoder,
		unsigned long status);
	u32  (*kms_get_vblank_counter)(emgd_crtc_t *emgd_crtc);
	int (*kms_match_mode)(emgd_encoder_t *emgd_encoder,
		igd_framebuffer_info_t *fb_info, igd_timing_info_t **timing);
} mode_kms_dispatch_t;



typedef struct _mode_full_dispatch {
	int (*alter_cursor_pos)(igd_display_h display_handle,
		igd_cursor_info_t *cursor_info);
	int (*set_palette_entries)(igd_display_h display_handle,
		unsigned long *palette_colors, unsigned int start_index,
		unsigned int count);
	int (*wait_vsync)(igd_display_h display_handle);
	int (*query_in_vblank)(igd_display_h display_handle);
	int (*get_scanline)(igd_display_h display_handle, int *scanline);
	int (*set_display_base)(igd_display_context_t *display,
		igd_framebuffer_info_t *fb, unsigned long *x, unsigned long *y);
	void (*program_cursor)(igd_display_context_t *display,
		unsigned long status);
	int (*set_color_correct)(igd_display_context_t *display);
	int (*get_surface)(igd_display_h display_handle, igd_buffertype_t type,
		igd_surface_t *surface, igd_appcontext_h appcontext);
	int (*set_surface)(igd_display_h display_handle, int priority,
		igd_buffertype_t type, igd_surface_t *surface,
		igd_appcontext_h appcontext, unsigned long flags);
	int (*query_event)(igd_display_h display_handle, igd_event_t event,
		unsigned long *status);

	int (*set_flip_pending)(unsigned char *mmio,
		unsigned long pipe_status_reg);
	int (*check_flip_pending)(unsigned char *mmio,
		unsigned long pipe_status_reg);

	int (*get_plane_info)(void); /* dispatch routines that gets fw info */
	int (*get_pipe_info)(igd_display_h *display );
	int (*get_port_info)(void);
	/* Implementation of "public" igd_dispatch_t.register_vblank_callback(). */
	emgd_vblank_callback_h (*register_vblank_callback)(
		emgd_process_vblank_interrupt_t callback,
		void *priv,
		unsigned long port_number);
	/* Implementation of "public" igd_dispatch_t.unregister_vblank_callback().
	 */
	void (*unregister_vblank_callback)(
		emgd_vblank_callback_h callback_h);
	/* Implementation of "public" igd_dispatch_t.enable_vblank_callback(). */
	int (*enable_vblank_callback)(emgd_vblank_callback_h callback_h);
	/* Implementation of "public" igd_dispatch_t.disable_vblank_callback(). */
	void (*disable_vblank_callback)(
		emgd_vblank_callback_h callback_h);
	/*!
	 * Implementation of "protected" function (i.e. for use within the mode
	 * module) to request VBlank interrupts for a particular purpose & port.
	 * Based upon the outstanding requests, this function decides whether to
	 * touch registers, register the interrupt handler, etc.
	 *
	 * @param request_for (IN).  A bit that identifies a who and what (e.g.
	 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
	 *
	 * @param mmio (IN).  A pointer to the start of memory-mapped IO, to access
	 * registers.
	 *
	 * @return Zero for success, non-zero for failure.
	 */
	int (*request_vblanks)(unsigned long request_for, unsigned char *mmio);
	/*!
	 * Implementation of "protected" function (i.e. for use within the mode
	 * module) to end a previous request VBlank interrupts for a particular
	 * purpose & port.  Based upon the outstanding requests, this function
	 * decides whether to touch registers, unregister the interrupt handler,
	 * etc.
	 *
	 * @param request_for (IN).  A bit that identifies a who and what (e.g.
	 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
	 *
	 * @param mmio (IN).  A pointer to the start of memory-mapped IO, to access
	 * registers.
	 *
	 * @return Zero for success, non-zero for failure.
	 */
	int (*end_request)(unsigned long request_for, unsigned char *mmio);
	/*!
	 * Implementation of "protected" function (i.e. for use within the mode
	 * module) to check whether a requested VBlank interrupt occured.
	 *
	 * @param request_for (IN).  A bit that identifies a who and what (e.g.
	 *  VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4)).
	 *
	 * @return Non-zero if the requested VBlank occured, zero if not.
	 */
	int (*vblank_occured)(unsigned long request_for);
	unsigned long (*get_port_control)(unsigned long port_num, unsigned long port_reg);
	void (*lock_planes)(igd_display_h display_handle);
	int (*unlock_planes)(igd_display_h display_handle, unsigned int scrn_num);
} mode_full_dispatch_t;

typedef struct _mode_dispatch {
	int (*set_palette_entry)(igd_display_h display_handle,
		unsigned long palette_entry, unsigned long palette_color);
	int (*get_palette_entry)(igd_display_h display_handle,
		unsigned long palette_entry, unsigned long *palette_color);
	int (*wait_vblank)(igd_display_h display_handle);
	void (*program_plane)(igd_display_context_t *display,
		unsigned long status);
	void (*program_pipe)(igd_display_context_t *display, unsigned long status);
	int (*program_port)(igd_display_context_t *display,
		unsigned short port_number, unsigned long status);
	int (*post_program_port)(igd_display_context_t *display,
		unsigned short port_number, unsigned long status);
	int (*program_clock)(igd_display_context_t *display,
		igd_clock_t *clock, unsigned long dclk);
	int (*program_cdvo)(void);
	void (*reset_plane_pipe_ports)(igd_context_t *context);
	unsigned long (*get_gpio_sets)(unsigned long **);
	void (*filter_modes)(igd_context_t *context,
		igd_display_port_t *port, pd_timing_t *in_list);
	int (*check_display)(igd_display_context_t *display,
		unsigned short port_number, unsigned long status);
	int (*get_dd_timing)(igd_display_context_t *display,
		pd_timing_t *in_list);
	int (*check_port_supported)(void *port_tmp);
	int (*get_refresh_in_border)(pd_timing_t *in_list);
	bool (*dsp_is_force_alter_required)(igd_display_context_t *display,
			unsigned long current_dc, unsigned long dc_to_set);
	mode_full_dispatch_t *full;
} mode_dispatch_t;


/*
 * Firmware(VBIOS or EFI Video Driver) related information
 * that needs to be populated  before the driver re-programs
 * the Hardware Registers. This information is needed to provide
 * seamless transition from firmware to driver.
 */
typedef struct _fw_info {

	/* TODO: Fill this up */
	unsigned long fw_dc; /* The dsp module already has this value */

	/* Plane information */
	igd_framebuffer_info_t fb_info[2]; /* one for each plane */

	/* Pipe information */
	igd_display_info_t timing_arr[2]; /* one for each pipe */

	/* Port information */

	/* if the plane registers needs an update, set this field to 1 */
	int program_plane;

} fw_info_t;



typedef struct _mode_context {
	/*
	 * All of the below values will be initialized in mode module
	 * init function mode_init().
	 */
	unsigned long        first_alter;
	mode_dispatch_t     *dispatch;
	mode_kms_dispatch_t *kms_dispatch;

	igd_context_t *context;
	unsigned long display_color;
	fw_info_t* fw_info; /* This needs to be zero for VBIOS */

    /* quickboot options */
    unsigned long quickboot;
    int seamless;
    unsigned long video_input;
    int splash;
	unsigned long ref_freq;
	int tuning_wa;
	unsigned long clip_hw_fix;
	unsigned long async_flip_wa;

	/*
	 * Enable override of following registers when en_reg_override=1.
	 * Display Arbitration, FIFO Watermark Control, GVD HP_CONTROL,
	 * Bunit Chickenbits, Bunit Write Flush, Display Chickenbits
	 */
	unsigned long en_reg_override;
	unsigned long disp_arb;
	unsigned long fifo_watermark1;
	unsigned long fifo_watermark2;
	unsigned long fifo_watermark3;
	unsigned long fifo_watermark4;
	unsigned long fifo_watermark5;
	unsigned long fifo_watermark6;
	unsigned long gvd_hp_control;
	unsigned long bunit_chicken_bits;
	unsigned long bunit_write_flush;
	unsigned long disp_chicken_bits;

	/* Flags specifying whether to notify user-space of a v-blank event.
	 * This is used when synchronizing back-buffer blits */
	bool batch_blits[IGD_MAX_PORTS];

} mode_context_t;

extern int full_mode_init(igd_context_t *context,
	mode_context_t *mode_context);

extern int full_mode_query(igd_driver_h driver_handle, unsigned long dc,
	igd_display_info_t **mode_list, igd_display_port_t *port);

extern void full_clear_fb(mode_context_t *mode_context,
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb);

extern int query_seamless(unsigned long dc,
		int index,
		igd_timing_info_t *pt,
		igd_framebuffer_info_t *pf,
		unsigned long flags);

extern void swap_fb_cursor( void );

extern int set_color_correct(igd_display_context_t *display,
	const igd_range_attr_t *attr_to_set);

extern void toggle_vblank_interrupts(bool status);

extern mode_context_t mode_context[];

/*
 * NOTE: Some of these externs are declared with the struct name because the
 * contents of that struct are unavailable at the DI layer. The symbol
 * is used as the generic mode_dispatch_t which is a subset.
 */


extern mode_dispatch_t mode_dispatch_plb;
extern mode_dispatch_t mode_dispatch_tnc;

extern mode_kms_dispatch_t mode_kms_dispatch_plb;
extern mode_kms_dispatch_t mode_kms_dispatch_tnc;



/*******************************************************************************
 *
 * The following macros provide part of the "protected" interface to support
 * mode-module "requests" for VBlank interrupts.
 *
 * Requests are for a "who" and "what."  The "who" is what type of code is
 * making the request, and the "what" is the port that the requestor wants to
 * know about VBlanks for.  Here is additional information:
 *
 * - Who - which software is asking:
 *   - WAIT - The code that implements the wait_vblank() function pointer.
 *            When interrupts are requested for WAIT, the interrupt handler
 *            makes note of when VBlanks occur.  The WAIT code queries (polls)
 *            whether a VBlank has occured since its request.
 *   - FLIP - The code that implements the {check|set}_flip_pending() function
 *            pointers.  When interrupts are requested for FLIP, the interrupt
 *            handler makes note of when VBlanks occur.  The FLIP code queries
 *            (polls) whether a VBlank has occured since its request.
 *   - CB -   The non-HAL code that registers a VBlank interrupt "callback"
 *            (CB).  When interrupts are requested for CB, the interrupt
 *            handler calls the callback when VBlanks occur.
 *
 * - What - which port (Note: space is reserve for 4 ports, even only two exist
 *   at this time):
 *   - PORT2 (Port 2, Pipe A, SDVO-B)
 *   - PORT4 (Port 4, Pipe B, Int-LVDS)
 *
 * Note: internally, the requests are stored in bits within an unsigned long.
 * This helps explain the way the macros are implemented:
 *
 ******************************************************************************/

/* A requestor uses this macro to generate the bit request for who and what: */
#define VBINT_REQUEST(who,port) ((port) << (who))

/* A requestor uses one of these macros to specify a what (i.e. port): */
#define VBINT_PORT2 0x01
#define VBINT_PORT4 0x02
/* Note: the following 2 macros reserve space for 2 more (future) ports: */
#define VBINT_PORTn 0x04
#define VBINT_PORTm 0x08

/* A requestor uses one of these macros to identify itself (the what): */
/* Note: Each "who" has 4 bits (for 4 ports); the value is a shift amount: */
#define VBINT_WAIT 0
#define VBINT_FLIP 4
#define VBINT_CB   8


/*******************************************************************************
 *
 * The following macros provide part of the "private" interface to support
 * mode-module "requests" for VBlank interrupts.  The VBlank-interrupt code
 * uses these macros to manage requests, and to record VBlanks that occur
 * (a.k.a. "answers").
 *
 * Other parts of the "mode" module should not use these macros.
 *
 ******************************************************************************/

/* Answers for a request are stored in bits to the left of the request bits: */
#define VBINT_ANSWER_SHIFT 12
#define VBINT_ANSWER(who,port) (((port) << (who)) << VBINT_ANSWER_SHIFT)
#define VBINT_ANSWER4_REQUEST(request) ((request) << VBINT_ANSWER_SHIFT)

/* The following special bit is used by disable_vblank_interrupts_{plb|tnc}() to
 * disable the hardware, but not unregister the never-registered interrupt
 * handler:
 */
#define VBLANK_DISABLE_HW_ONLY BIT31

/* The following macros aggregate all of the who's can enable interrupts for a
 * given port:
 */
#define VBLANK_INT4_PORT2 (VBINT_REQUEST(VBINT_WAIT, VBINT_PORT2) | \
	VBINT_REQUEST(VBINT_FLIP, VBINT_PORT2) |						\
	VBINT_REQUEST(VBINT_CB, VBINT_PORT2))
#define VBLANK_INT4_PORT4 (VBINT_REQUEST(VBINT_WAIT, VBINT_PORT4) | \
	VBINT_REQUEST(VBINT_FLIP, VBINT_PORT4) |						\
	VBINT_REQUEST(VBINT_CB, VBINT_PORT4))

/* The following macros aggregate all of the whats (ports) that can enable
 * interrupts for a given who (they aren't used, but do help document that
 * 4 bits are reserved for each "who"):
 */
/* FIXME -- KEEP THESE??? */
#define VBLANK_INT4_WAIT 0x0000000f
#define VBLANK_INT4_FLIP 0x000000f0
#define VBLANK_INT4_CB   0x00000f00

/* The following macros tell whether interrupts are enabled, either in general,
 * or for a certain port.
 */
#define VBLANK_INTERRUPTS_ENABLED \
	(vblank_interrupt_state & (VBLANK_INT4_PORT2 | VBLANK_INT4_PORT4 | \
		VBLANK_DISABLE_HW_ONLY))
#define VBLANK_INTERRUPTS_ENABLED4_PORT2 \
	(vblank_interrupt_state & VBLANK_INT4_PORT2)
#define VBLANK_INTERRUPTS_ENABLED4_PORT4 \
	(vblank_interrupt_state & VBLANK_INT4_PORT4)

#endif

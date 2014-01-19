/*
 *-----------------------------------------------------------------------------
 * Filename: context.h
 * $Revision: 1.22 $
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

#ifndef _HAL_CONTEXT_H
#define _HAL_CONTEXT_H

#include <pci.h>

#include <igd.h>
#include <igd_init.h>

#define GET_DEVICE_POWER_STATE(d_h) \
(((igd_display_context_t *)(d_h))->context->device_context.power_state)

typedef void * platform_context_t;

typedef struct _igd_context igd_context_t;

struct _pd_reg;
struct _igd_display_context;
struct _igd_display_port;
struct _pd_timing;
struct _cmd_queue;

/*
 * Flags for reg_set_mod_state and reg_get_mode_state
 */
typedef enum _reg_state_id {
	REG_MODE_STATE_REG = 1,
	REG_MODE_STATE_CON = 2
} reg_state_id_t;

/* module_state handle */
typedef struct _module_state *module_state_h;

/*
 * Intermodule-dispatch is for calling from one module to another. All
 * functions should be considered optional and the caller should check
 * for null before using.
 */
typedef struct _inter_module_dispatch {
	struct _igd_param *init_params;

	void (*gmm_save)(igd_context_t *context, void **state);
	void (*gmm_restore)(igd_context_t *context, void *state);

	void (*mode_reset_plane_pipe_ports)(igd_context_t *context);
	void (*filter_modes)(igd_context_t *context,
		struct _igd_display_port *port,
		struct _pd_timing *in_list);

	unsigned long (*get_port_control)(unsigned long port_num, unsigned long port_reg);
	/* FIXME: This should be a dsp function */
	unsigned long (*mode_get_gpio_sets)(unsigned long **gpio);

	/* For Poulsbo, there is no flip pending hardware, so the
	 * mode module and command module need to communicate */
	int (*set_flip_pending)(unsigned char *mmio,
		unsigned long pipe_status_reg);
	int (*check_flip_pending)(unsigned char *mmio,
		unsigned long pipe_status_reg);

	int (*dsp_get_config_info)(igd_context_t *context,
		igd_config_info_t *config_info);
	int (*pi_get_config_info)(igd_context_t *context,
		igd_config_info_t *config_info);

	/* Widely used DSP interfaces */
	struct _igd_display_port *(*dsp_get_next_port)(igd_context_t *context,
		struct _igd_display_port *port, int reverse);
	struct _igd_plane *(*dsp_get_next_plane)(igd_context_t *context,
		struct _igd_plane *plane, int reverse);
	struct _igd_display_pipe *(*dsp_get_next_pipe)(igd_context_t *context,
		struct _igd_display_pipe *pipe, int reverse);
	void (*dsp_get_display)(unsigned short port_number,
		struct _igd_display_context **display,
		struct _igd_display_port **port, int display_detect);
	void (*dsp_get_dc)(unsigned long *dc,
		struct _igd_display_context **primary,
		struct _igd_display_context **secondary);
	void (*dsp_get_planes_pipes)(struct _igd_plane **primary_display_plane,
		struct _igd_plane **secondary_display_plane,
		struct _igd_display_pipe **primary_pipe,
		struct _igd_display_pipe **secondary_pipe);
	int (*dsp_alloc)(igd_context_t *context, unsigned long dc,
		unsigned long flags);
	void (*dsp_control_plane_format)(int enable, int display_plane, struct _igd_plane *plane_override);

	/* Flag to indicate FB Blend + Overlay override */
	unsigned int fb_blend_ovl_override;

	unsigned long *dsp_current_dc;
	/* Firmware  programmed DC. This information needs to be
     * accessible by the mode module.
     */
	unsigned long dsp_fw_dc;
	
	/* flag to tell if in dih_clone mode */
	unsigned long in_dih_clone_mode;
	unsigned long dih_clone_display;

	struct _igd_display_port **dsp_port_list;
	struct _igd_display_context **dsp_display_list;

	/* i2c and GMBus interfaces */
	int (*i2c_read_regs)(igd_context_t *context, unsigned long i2c_bus,
		unsigned long i2c_speed, unsigned long dab, unsigned char reg,
		unsigned char FAR *buffer, unsigned long num_bytes,
		unsigned long flags);
	int (*i2c_write_reg_list)(igd_context_t *context,
		unsigned long i2c_bus, unsigned long i2c_speed, unsigned long dab,
		struct _pd_reg *reg_list, unsigned long flags);
	int (*gpio_write_regs)(igd_context_t *context,
		struct _igd_display_port *port, unsigned long gpio_pin_pair,
		unsigned long power_state);
	int (*gpio_read_regs)(igd_context_t *context, unsigned long gpio_pin_pair,
		unsigned long *data);

	int (*reg_get_mod_state)(reg_state_id_t id, module_state_h **state,
		unsigned long **flags);

	/* Power Module Calls to Reg module */
	void *(*reg_alloc)(igd_context_t *context, unsigned long flags);
	void (*reg_free)(igd_context_t *context, void *reg_set);
	int (*reg_save)(igd_context_t *context, void *reg_set);
	int (*reg_restore)(igd_context_t *context, void *reg_set);
	void (*reg_crtc_lut_get)(igd_context_t *context, void *emgd_crtc);
	void (*reg_crtc_lut_set)(igd_context_t *context, void *emgd_crtc);

	/* Reg Module callbacks */
	int (*mode_save)(igd_context_t *context, module_state_h *state,
		unsigned long *flags);
	int (*mode_restore)(igd_context_t *context, module_state_h *state,
		unsigned long *flags);

	/* Power Module Callbacks */
	int (*mode_pwr)(igd_context_t *context, unsigned long powerstate);
	int (*overlay_pwr)(igd_context_t *context, unsigned long powerstate);
	int (*msvdx_pwr)(igd_context_t *context, unsigned long powerstate);
	int (*msvdx_status)(igd_context_t *context, unsigned long *queue_status, unsigned long *mtx_msg_status);

	/* Shutdown functions for use by init module only */
	void (*mode_shutdown)(igd_context_t *context);
	void (*dsp_shutdown)(igd_context_t *context);
	void (*pi_shutdown)(igd_context_t *context);
	void (*shutdown_2d)(igd_context_t *context);
	void (*blend_shutdown)(igd_context_t *context);
	void (*interrupt_shutdown)(igd_context_t *context);
	void (*appcontext_shutdown)(igd_context_t *context);
	void (*reset_shutdown)(igd_context_t *context);
	void (*pwr_shutdown)(igd_context_t *context);
	void (*overlay_shutdown)(igd_context_t *context);
	void (*cmd_shutdown)(igd_context_t *context);
	void (*reg_shutdown)(igd_context_t *context);

	/* Mode module Callbacks */
	int (*get_dd_timing)(struct _igd_display_context *display,
		struct _pd_timing *in_list);
	int (*check_port_supported)(void *port_tmp);
	int (*get_refresh_in_border)(struct _pd_timing *in_list);

} inter_module_dispatch_t;

/*
 * IGD device context contains device information that should exist in
 * some form across all supported platforms.
 *
 * Device information that is specific to any individual platform/core
 * etc should be in the "platform_context" which is hardware specific.
 *
 * In the event that any given core has a large number of marketing
 * SKUs that change the PCI device ID, the device ID in this structure
 * may be overwritten with a single common ID and not truly reflect
 * the actual ID.  This prevents polluting the function tables with
 * dozens of logically identical chipsets.
 *
 */
typedef struct _device_context {
	unsigned long power_state;
	unsigned long mmadr;		/* Primary mmio address */
	unsigned long mmadr_sdvo;	/* sdvo mmio address */
	unsigned long mmadr_sdvo_st;	/* STMicro sdvo mmio address */
	unsigned long mmadr_sdvo_st_gpio;	/* STMicro sdvo mmio address */
	unsigned long gpio_bar;		/* LPC GPIO address */
	unsigned char *virt_mmadr;
	unsigned char *virt_mmadr_sdvo;
	unsigned char *virt_mmadr_sdvo_st;
	unsigned char *virt_mmadr_sdvo_st_gpio;
	unsigned char *virt_gpio_bar;
	unsigned long *virt_gttadr; /* was gtt_mmap */
	unsigned long gatt_pages;   /* Number of pages addressable by GTT */
	unsigned long stolen_pages; /* Number of pages of stolen memory */
	unsigned long gmch_ctl;     /* GMCH control value */
	void *scratch_page;         /* Empty page to fill unused GTT entries */
	unsigned long fb_adr;       /* Video Memory address */
	unsigned short did;         /* Device ID for main video device */
	unsigned long rid;          /* Device revision ID for main video device */
	unsigned short bid;			/* Device ID for Bridge */
	unsigned long max_dclk;     /* maximum dotclock of all the chipset */
	unsigned long mem_size;     /* Total memory space managed by GMM */
	unsigned long reserved_mem; /* Size in pages of firmware populated mem */
	unsigned long hw_config;    /* HW Config parameter bits (see igd_init.h) */
	unsigned long hw_status_offset; /* Hw status page offset */
	unsigned short gfx_freq;	/* Graphics Frequency, used to calculate PWM */
	unsigned short core_freq;	/* Core Frequency, used to calculate DPLL freq */
} device_context_t;

struct _igd_context {
	int igd_device_error_no; /* Values are already negative */
	igd_dispatch_t dispatch;
	inter_module_dispatch_t mod_dispatch;
	device_context_t device_context;     /* Hardware independent */
	platform_context_t platform_context; /* Hardware dependent   */
	void *drm_dev;
};

typedef struct _igd_display_context {
	igd_context_t *context;
	void *plane;
	void *pipe;
	void *port[IGD_MAX_PORTS];
	void *cursor;
	unsigned long port_number;
	unsigned long allocated;
} igd_display_context_t;

#endif


/*
 *-----------------------------------------------------------------------------
 * Filename: mode.h
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
 *  Contain header information for set mode support
 *-----------------------------------------------------------------------------
 */

#ifndef _MODE_H_
#define _MODE_H_

/* IO.h is needed for the FAR define */
#include <io.h>

#include <igd_init.h>
#include <igd_mode.h>
#include <general.h>
#include <context.h>
#include <cmd.h>
#include <pd.h>
#include <edid.h>
#include <displayid.h>
#include <igd_render.h>

/* KMS-related Headers */
#include <drm/drmP.h>
#include <drm/drm.h>
#include <drm/drm_fb_helper.h>


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define IGD_INVALID_MODE 0

/*****************/
/* Plane Features */
/*****************/
#define IGD_PLANE_FEATURES_MASK      0x000000FF
#define IGD_PLANE_DISPLAY            0x00000001
#define IGD_PLANE_OVERLAY            0x00000002
#define IGD_PLANE_SPRITE             0x00000004
#define IGD_PLANE_CURSOR             0x00000008
#define IGD_PLANE_VGA                0x00000010
#define IGD_PLANE_DOUBLE             0x00000020
/*#define IGD_PLANE_CLONE            0x00000040 currently unused */
#define IGD_PLANE_DIH                0x00000080
#define IGD_PLANE_USE_PIPEA          0x00000100
#define IGD_PLANE_USE_PIPEB          0x00000200

/*****************/
/* Pipe Features */
/*****************/
/* pipe's supported features */
#define IGD_PIPE_FEATURES_MASK       0x000000FF
#define IGD_PIPE_DOUBLE              0x00000001
/* the following 2 bits are not pipe features but
 * pipe identification bits that share the same
 * pipe_features variable of the pipe structure
 * NOTE that these bit-wise OR flags in nibble-2
 * are also used in the port_features variable
 * of the port structure (same locations) but
 * called IGD_PORT_USE_PIPEX
 */
#define IGD_PIPE_IS_PIPEA            0x00000100
#define IGD_PIPE_IS_PIPEB            0x00000200

/*******************/
/* Cursor Features */
/*******************/
/*
 * Cursor's supported features
 */
/*
 * cursor's pipe usage regulations. These are copies of the PIPE_IS_ bits so
 * that a quick (pipe_features & plane_features & MASK) will tell you if
 * the pipe can be used.
 */
#define IGD_CURSOR_USE_PIPE_MASK       0x00000F00
#define IGD_CURSOR_USE_PIPEA           IGD_PIPE_IS_PIPEA
#define IGD_CURSOR_USE_PIPEB           IGD_PIPE_IS_PIPEB

/*****************/
/* Port features */
/*****************/
/*
 * Port's supported features
 *
 * port features also uses IGD_PORT_SHARE_MASK thus port feature bits
 * cannot collide with IGD_PORT_SHARE_MASK
 */
#define IGD_PORT_FEATURES_MASK       0x000000FF
#define IGD_RGBA_COLOR               0x00000001
#define IGD_RGBA_ALPHA               0x00000002
#define IGD_VGA_COMPRESS             0x00000004 /* Compress VGA to 640x480 */
#define IGD_PORT_GANG                0x00000008
/*
 * port's pipe usage regulations. These are copies of the PIPE_IS_ bits so
 * that a quick (pipe_features & port_features & MASK) will tell you if
 * the pipe can be used.
 */
#define IGD_PORT_USE_PIPE_MASK       0x00000F00
#define IGD_PORT_USE_PIPE_MASK_SHIFT 8
#define IGD_PORT_USE_PIPEA           IGD_PIPE_IS_PIPEA
#define IGD_PORT_USE_PIPEB           IGD_PIPE_IS_PIPEB
/*
 * Ports Sharing information. The port in question can share a pipe with the
 * listed ports. If a shares with b, b must share with a too.
 * Must be the same as IGD_PORT_MASK = 0x3f000
 */
#define IGD_PORT_SHARE_MASK          IGD_PORT_MASK
#define IGD_PORT_SHARE_ANALOG        IGD_PORT_ANALOG
#define IGD_PORT_SHARE_DIGITAL       IGD_PORT_DIGITAL
#define IGD_PORT_SHARE_LVDS          IGD_PORT_LVDS
#define IGD_PORT_SHARE_TV            IGD_PORT_TV

/* MAX rings, planes and ports connected to a pipe */
#define IGD_MAX_PIPE_QUEUES      4
#define IGD_MAX_PIPE_PLANES      5
#define IGD_MAX_PIPE_DISPLAYS    4
#define IGD_MAX_PIPES            2
#define MAX_DISPLAYS             IGD_MAX_DISPLAYS /* From igd_mode.h */

/* Parameters to mode_update_plane_pipe_ports */
#define MODE_UPDATE_PLANE  0x1
#define MODE_UPDATE_PIPE   0x2
#define MODE_UPDATE_PORT   0x4
#define MODE_UPDATE_NONE   0x0

#define PLANE(display) \
	((igd_display_plane_t *)(((igd_display_context_t *)display)->plane))
#define PIPE(display)  \
	((igd_display_pipe_t *)(((igd_display_context_t *)display)->pipe))
#define PORT(display, pn)  \
	((igd_display_port_t *)(((igd_display_context_t *)display)->port[pn-1]))
#define PORT_OWNER(display)  \
	((igd_display_port_t *)(((igd_display_context_t *)display)->port[display->port_number-1]))

#define MODE_IS_SUPPORTED(t) (t->mode_info_flags & IGD_MODE_SUPPORTED)
#define MODE_IS_VGA(t) \
	((((pd_timing_t *)t)->mode_info_flags & IGD_MODE_VESA) &&	\
		(t->mode_number < 0x1D))

#define IGD_KMS_PIPEA (IGD_PIPE_IS_PIPEA >> IGD_PORT_USE_PIPE_MASK_SHIFT)
#define IGD_KMS_PIPEB (IGD_PIPE_IS_PIPEB >> IGD_PORT_USE_PIPE_MASK_SHIFT)

#define KMS_PIPE_FEATURES(display) \
	(((igd_display_pipe_t *)((igd_display_context_t *)display)->pipe)->pipe_features)

#define KMS_PIPE_ID(pipe_features) \
	((pipe_features & IGD_PORT_USE_PIPE_MASK) >> IGD_PORT_USE_PIPE_MASK_SHIFT)


/* #define DC_PORT_NUMBER(dc, i) ((dc >> (i * 4)) & 0x0f) */
#define DC_PORT_NUMBER IGD_DC_PORT_NUMBER

/* This structure is used for the mode table which is a list of all
 * supported modes. */

typedef pd_timing_t igd_timing_info_t, *pigd_timing_info_t;


/*
 * NOTE: The plane typedef is a generic type. Each plane type has an
 * equivalent typedef that is more specific to the type of plane. They
 * MUST remain equivalent. If you change one you must change them all.
 */
typedef struct _igd_plane {
	unsigned long plane_reg;       /* plane control register */
	unsigned long plane_features;  /* plane feature list */
	int           inuse;           /* plane inuse ? */
	int           ref_cnt;         /* # of displays using this plane */
	unsigned long *pixel_formats;  /* supported pixel formats */
	void *plane_info;              /* ptr to plane_info */
	struct _igd_plane *mirror;     /* pointer to mirror plane */
} igd_plane_t;

typedef struct _igd_display_plane {
	unsigned long plane_reg;         /* plane contron register */
	unsigned long plane_features;    /* list of plane features */
	int           inuse;             /* plane inuse ? */
	int           ref_cnt;           /* # of displays using this plane */
	unsigned long *pixel_formats;    /* list of pixel formats supported */
	igd_framebuffer_info_t *fb_info; /* attached fb to this plane */
	struct _igd_display_plane *mirror;  /* pointer to mirror plane */
} igd_display_plane_t, *pigd_display_plane_t;

typedef struct _igd_cursor {
	unsigned long cursor_reg;        /* cursor control register */
	unsigned long plane_features;    /* cursor plane features */
	int           inuse;             /* is this cursor in use? */
	int           ref_cnt;           /* # of displays using this plane */
	unsigned long *pixel_formats;    /* list of pixel_formats supported */
	igd_cursor_info_t *cursor_info;
	struct _igd_cursor *mirror;  /* pointer to mirror plane */
} igd_cursor_t;

typedef struct _igd_clock {
	unsigned long dpll_control;     /* DPLL control register */
	unsigned long mnp;              /* FPx0 register */
	unsigned long p_shift;          /* Bit location of P within control */
	unsigned long actual_dclk;		/* Actual dotclock after calculating dpll */
}igd_clock_t;

typedef struct _igd_display_pipe {
	unsigned long pipe_num;             /* 0 Based index */
	unsigned long pipe_reg;             /* pipe configuration register */
	unsigned long timing_reg;           /* timing register(htotal) */
	unsigned long palette_reg;          /* palette register */
	igd_clock_t   *clock_reg;           /* DPLL clock registers */
	unsigned long pipe_features;        /* pipe features */
	int           inuse;                /* pipe allocated? TRUE/FALSE */
	int           ref_cnt;              /* # of displays using this pipe */
	cmd_queue_t *queue[IGD_MAX_PIPE_QUEUES]; /* Queues for this pipe */
	igd_display_plane_t   *plane;      /* dsp plane connected to pipe */
	igd_cursor_t          *cursor;     /* cursor connected to this pipe */
	void                  *sprite;     /* sprite connected to this pipe */
	igd_timing_info_t     *timing;     /* current timings on the port */
	igd_display_context_t *owner;      /* owner display of this pipe */
	unsigned long dclk;                /* current dclk running on this pipe */
}igd_display_pipe_t, *pigd_display_pipe_t;

typedef struct _igd_display_port {
	unsigned long port_type;            /* port type */
	unsigned long port_number;          /* port number */
	char          port_name[8];         /* port name DVO A, B, C, LVDS, ANALOG */
	unsigned long port_reg;             /* port control register */
	unsigned long i2c_reg;              /* GPIO pins for i2c on this port */
	unsigned long dab;                  /* i2c Device Address Byte */
	unsigned long ddc_reg;              /* GPIO pins for DDC on this port */
	unsigned long ddc_dab;
	unsigned long port_features;        /* port features */
	unsigned long clock_bits;           /* Clock input to use */
	int           inuse;                /* port is in use */
	unsigned long  power_state;         /* D Power state for the display/port */
	unsigned long bl_power_state;       /* D Power state for the FP backlight */
	struct _igd_display_port *mult_port;/* pointer to multiplexed port,
										 * if it is used in that way */
	igd_display_info_t    *pt_info;     /* port timing info */
	pd_driver_t           *pd_driver;
	void                  *pd_context;  /* Context returned from PD */
	pd_callback_t         *callback;    /* DD Callback to passed to PD */
	unsigned long         num_timing;   /* number of timings available */
	igd_timing_info_t     *timing_table; /* static/dynamic PD timings list */
	unsigned long         i2c_speed;    /* Connected encoder's I2C bus speed */
	unsigned long         ddc_speed;    /* DDC speed in KHz */
	igd_param_fp_info_t   *fp_info;     /* Flat panel parameter info if any */
	igd_param_dtd_list_t  *dtd_list;    /* EDID-less DTD info if any */
	igd_param_attr_list_t *attr_list;   /* Saved attributes if any */
	igd_attr_t            *tmp_attr;    /* Temp attr array, for copying */
	unsigned int          tmp_attr_num; /* Number of attr in temp array */
	igd_timing_info_t     *fp_native_dtd; /* FP native DTD */
	unsigned long         pd_type;      /* Display type given by port driver */
	unsigned long         pd_flags;     /* port driver flags */
	unsigned long         saved_bl_power_state;

	/* This attribute list is designed to eventually suck in things above
	 * such as fb_info.  For now, it only has color correction attributes */
	igd_attr_t            *attributes;

	unsigned char         firmware_type;
	union {
		displayid_t       *displayid;
		edid_t            *edid;         /* EDID information */
	};

    /* Added for VBIOS size Reduction */
	unsigned long         preserve;
	unsigned long         mult_preserve;
	unsigned long         vga_sync;

}igd_display_port_t, *pigd_display_port_t;

/* This structure is used to save mode state.
 * Rightnow, it is saving state of current port and its port driver state.
 * This information is used while restoring to a previously saved mode
 * state.
 * TODO: This can be extended to save all display modules (mode, dsp, pi) reg
 * information along with port driver's state information. This requires
 * changes to exiting reg module. */
#define MAX_PORT_DRIVERS     20
typedef struct _mode_pd_state {
		igd_display_port_t *port;       /* display port */
		void               *state;      /* and its port driver state */
} mode_pd_state_t;

typedef struct _mode_state_t {
	mode_pd_state_t pd_state[MAX_PORT_DRIVERS];
} mode_state_t;


/**
 * This holds information about a framebuffer
 */
typedef struct _emgd_framebuffer {
	struct drm_framebuffer base;
	enum {
		GMM_FRAMEBUFFER,
		PVR_FRAMEBUFFER
	} type;
	void *pvr_meminfo;
	struct page **pagelist;
	unsigned long gtt_offset;
	unsigned long handle;
} emgd_framebuffer_t;

typedef struct _emgdfb_par {
        struct drm_device  *dev;
        emgd_framebuffer_t *emgd_fb;
} emgdfb_par_t;


/**
 * This holds information about a CRTC.
 */
typedef struct _emgd_crtc {
        struct drm_crtc         base;

		/* Spinlock to protect access to this structure */
		spinlock_t              crtc_lock;

        int                     crtc_id;
        igd_display_pipe_t     *igd_pipe;
        emgd_framebuffer_t     *fbdev_fb;
        struct drm_mode_set     mode_set;
        struct drm_display_mode saved_mode;
        struct drm_display_mode saved_adjusted_mode;
        unsigned char           lut_r[256];
        unsigned char           lut_g[256];
        unsigned char           lut_b[256];
        unsigned char           lut_a[256];

		/* Flip request work task */
		struct work_struct      flip_work;
		unsigned char           flip_work_queued;

		/*
		 * Framebuffer that we're in the process of flipping to (may not
		 * actually show up until rendering is complete, the actual
		 * registers are programmed, and a vblank happens).
		 */
		emgd_framebuffer_t *newfb;

		/*
		 * Target for rendering completion to allow flip to proceed.
		 * Rendering operations may continue to be dispatched against this
		 * FB after the flip ioctl is called, so we need to track the
		 * number of pending operations at the point the ioctl was called
		 * and use that as our target to flip on.
		 */
		unsigned long render_complete_at;

		/*
		 * Are we waiting for the next vblank event to perform flip cleanup
		 * on this CRTC?  Flip cleanup primarily involves sending a
		 * notification event back to userspace.
		 */
		unsigned char vblank_expected;

		/* Userspace event to send back upon flip completion. */
		struct drm_pending_vblank_event *flip_event;
} emgd_crtc_t;


/**
 * This holds information about an individual encoder
 *  */
typedef struct _emgd_encoder {
        struct drm_encoder base;

        unsigned long       crtc_mask;
        unsigned long       clone_mask;
        igd_display_port_t *igd_port;
        mode_pd_state_t     state;
        /* flags is a bit mask. For information
         * on the different masks, see ENCODER_FLAG_xx
         */
		unsigned long       flags;
} emgd_encoder_t;

#define ENCODER_FLAG_FIRST_DPMS  0x1
#define ENCODER_FLAG_FIRST_ALTER 0x2
#define ENCODER_FLAG_SEAMLESS    0x4

/**
 *  * This holds information about an individual output
 *   */
typedef struct _emgd_connector {
        struct drm_connector  base;

        emgd_encoder_t       *encoder;
        unsigned long         type;
        struct drm_property **properties;
        unsigned long         num_of_properties;

        struct _drm_emgd_private *priv;
} emgd_connector_t;

/**
 *  * This holds information on our framebuffer device.
 *   */
typedef struct _emgd_fbdev {
        struct drm_fb_helper      helper;

        emgd_framebuffer_t       *emgd_fb;
        u32                       pseudo_palette[17];
        struct list_head          fbdev_list;
        struct _drm_emgd_private *priv;
} emgd_fbdev_t;




#endif // _IGD_MODE_H_



/*
 *-----------------------------------------------------------------------------
 * Filename: lvds.c
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
 *  This file is contains all necessary functions for Internal
 *  LVDS PORT DRIVER.
 *  This is written according to the port interface defined in pd.h.
 *-----------------------------------------------------------------------------
 */
#include <linux/kernel.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
#include <linux/bug.h>
#endif
#include <config.h>
#include <igd_pd.h>
#include <pd.h>
#include <pd_print.h>

#include "lvds.h"

/* One space between the #define and the backslash,else compilers complain */
#define PTR_OFFSET_UCHAR(ptr,offset)   (*((unsigned char *)ptr + offset))
#define PTR_OFFSET_USHORT(ptr, offset) (*(unsigned short *)((unsigned char *)ptr + offset))

#define PTR_OFFSET_ULONG(ptr, offset) (*(unsigned long *)((unsigned char *)ptr + offset))
/* END OF OPTIMIZATION MACROS */

/* This constant = 10,000,000.  The value is used to
 * get effective results from the integer math, and
 * to not divide by 0. */
#define PWM_FREQ_CALC_CONSTANT_1        0x00989680
/* This constant is 1,000,000 - to multiply to get
 * the Display Clock Frequency to the order of Mhz */
#define PWM_FREQ_CALC_CONSTANT_2        0x000F4240


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(p) (sizeof(p)/sizeof((p)[0]))
#endif

static pd_version_t  lvds_version = {11, 0, 0, 0}; /* driver version */
static unsigned long lvds_dab_list[] = {
	PD_DAB_LIST_END
}; /* dab list */

static unsigned long supported_chipset[] =
{
#ifdef CONFIG_855
	PCI_DEVICE_ID_VGA_855,
#endif
#ifdef CONFIG_915AL
	PCI_DEVICE_ID_VGA_915AL,
#endif
#ifdef CONFIG_945GM
	PCI_DEVICE_ID_VGA_945GM,
	PCI_DEVICE_ID_VGA_945GME,
#endif
#ifdef CONFIG_965GM
	PCI_DEVICE_ID_VGA_GM965,
	PCI_DEVICE_ID_VGA_GME965,
#endif
#ifdef CONFIG_CTG
	PCI_DEVICE_ID_VGA_CTG,
#endif
#ifdef CONFIG_PLB
	PCI_DEVICE_ID_VGA_PLB,
#endif
#ifdef CONFIG_TNC
	PCI_DEVICE_ID_VGA_TNC,
	PCI_DEVICE_ID_VGA_TNC_A0,
	PCI_DEVICE_ID_VGA_LNC,
#endif
};

static pd_driver_t  lvds_driver  = {
	PD_SDK_VERSION,
	"Internal LVDS Port Driver",
	0,
	&lvds_version,
	PD_DISPLAY_LVDS_INT,
	PD_FLAG_UP_SCALING,
	lvds_dab_list,
	100,
	lvds_validate,
	lvds_open,
	lvds_init_device,
	lvds_close,
	lvds_set_mode,
	lvds_post_set_mode,
	lvds_set_attrs,
	lvds_get_attrs,
	lvds_get_timing_list,
	lvds_set_power,
	lvds_get_power,
	lvds_save,
	lvds_restore,
	lvds_get_port_status
};

/* This is a common attribute table for all chipsets. At the end of the table
 * there are multiple end entries to add chipset specific attributes.
 * Chipset specific attributes:
 *     965GM/GM45- Maintain aspect ratio
 * Note:
 *    1. Make sure to update the chipset_attr_index whenever adding a
 *       chipset specific new attr.
 */
static pd_attr_t lvds_attrs[] =
{
	/* Range attributes */

	/*<-------ID----------->              <----TYPE-------->  <---NAME----->  <----flag---->               <---DEF_VAL---->       <--CURR_VALUE-->      min max  st */
	PD_MAKE_ATTR (PD_ATTR_ID_FP_PWR_T1,   PD_ATTR_TYPE_RANGE, "FP Power T1",  PD_ATTR_FLAG_USER_INVISIBLE, 0,                     0,                    0,  819,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_FP_PWR_T2,   PD_ATTR_TYPE_RANGE, "FP Power T2",  PD_ATTR_FLAG_USER_INVISIBLE, 0,                     0,                    0,  819,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_FP_PWR_T3,   PD_ATTR_TYPE_RANGE, "FP Power T3",  PD_ATTR_FLAG_USER_INVISIBLE, 0,                     0,                    0,  819,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_FP_PWR_T4,   PD_ATTR_TYPE_RANGE, "FP Power T4",  PD_ATTR_FLAG_USER_INVISIBLE, 0,                     0,                    0,  819,  1),
	PD_MAKE_ATTR (PD_ATTR_ID_FP_PWR_T5,   PD_ATTR_TYPE_RANGE, "FP Power T5",  PD_ATTR_FLAG_USER_INVISIBLE, 400,                 400,                    0,  3000, 1),
	PD_MAKE_ATTR (PD_ATTR_ID_PANEL_DEPTH, PD_ATTR_TYPE_RANGE, "Panel Depth",  PD_ATTR_FLAG_USER_INVISIBLE, LVDS_DEF_PANEL_DEPTH,  LVDS_DEF_PANEL_DEPTH, 18, 24,   6),

	PD_MAKE_ATTR (PD_ATTR_ID_PWM_INTENSITY,	  PD_ATTR_TYPE_RANGE, "PWM cycle",			  PD_ATTR_FLAG_USER_INVISIBLE,		100,		0,				  0,  100,  0),
	PD_MAKE_ATTR (PD_ATTR_ID_INVERTER_FREQ,	  PD_ATTR_TYPE_RANGE, "Inverter Frequency",	  PD_ATTR_FLAG_USER_INVISIBLE,		100,		0,				  0,  0,  0),
	PD_MAKE_ATTR (PD_ATTR_ID_BLM_LEGACY_MODE, PD_ATTR_TYPE_BOOL,  "Backlight Legacy mode",	  PD_ATTR_FLAG_USER_INVISIBLE,		0,		0,					  0,  0,  0),

	/*<-------ID------------>                 <----TYPE--------> <---NAME----->        <------flag--------------->  <---DEF_VAL---->   <--CURR_VALUE-->   <---pad--> */
	PD_MAKE_ATTR (PD_ATTR_ID_2_CHANNEL_PANEL, PD_ATTR_TYPE_BOOL, "Dual-channel panel", PD_ATTR_FLAG_USER_INVISIBLE, 0,                 0,                 0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_LVDS_PANEL_TYPE, PD_ATTR_TYPE_BOOL, "Panel Type",         PD_ATTR_FLAG_USER_INVISIBLE, 0,                 0,                 0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_PANEL_FIT,       PD_ATTR_TYPE_BOOL, "Panel Upscale",      PD_ATTR_FLAG_USER_INVISIBLE, LVDS_DEF_PANEL_FIT,LVDS_DEF_PANEL_FIT,0, 0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_DITHER,          PD_ATTR_TYPE_BOOL, "Dither",             PD_ATTR_FLAG_USER_INVISIBLE, LVDS_DEF_DITHER,   LVDS_DEF_DITHER,   0, 0, 0),
	PD_MAKE_ATTR (LVDS_ATTR_ID_TC_LVDS_CLK,   PD_ATTR_TYPE_BOOL, "TC LVDS CLK 110MHz", PD_ATTR_FLAG_USER_INVISIBLE, 0,                 0,                 0, 0, 0),

	/* Start of chipset specific attributes */
	/* Maintain aspect ratio */
	PD_MAKE_ATTR (PD_ATTR_LIST_END,       0,                   "",            0,                           0,                0,               0,     0, 0),
	/* Text tuning */
	PD_MAKE_ATTR (PD_ATTR_LIST_END,       0,                   "",            0,                           0,                0,               0,     0, 0),

	/* Attribute list end */
	PD_MAKE_ATTR (PD_ATTR_LIST_END,       0,                   "",            0,                           0,                0,               0,     0, 0)
};

/* Rightnow it is -3 to reach starting of chipset specific attributes */
static unsigned short chipset_attr_index = (unsigned short)
	sizeof(lvds_attrs)/sizeof(pd_attr_t) - 3;

static pd_attr_t lvds_965gm_attrs[] = {
	PD_MAKE_ATTR (PD_ATTR_ID_MAINTAIN_ASPECT_RATIO,PD_ATTR_TYPE_BOOL,      "Maintain Aspect Ratio",0,             0,               0,               0,     0, 0),
	PD_MAKE_ATTR (PD_ATTR_ID_TEXT_TUNING,          PD_ATTR_TYPE_RANGE,     "Text Enhancement",     0,             0,               0,               0,     2, 1),
};

static void lvds_write_reg(lvds_context_t *pd_context, unsigned long reg,
		unsigned long value, unsigned long change_bits, unsigned long reg_type);

static unsigned long lvds_read_reg(lvds_context_t *pd_context,
		unsigned long reg, unsigned long reg_type);
static void lvds_panel_fit(lvds_context_t *pd_context);

static void lvds_get_dclk(lvds_context_t *pd_context, pd_dvo_info_t *lvds_info);

/*----------------------------------------------------------------------------
 *
 * Function: PD_MODULE_INIT(lvds_init)
 *
 * Description:
 *    This is the entry function into LVDS port driver when
 *    it first loads. This will call pd_register() to register
 *    with Display driver.  Only the driver object is initialized in this
 *    function, similar to DrverEntry() in a WDM driver.
 *
 * Parameters:
 *    [OUT] *handle:  Not used.  Place holder for supporting dynamic pd
 *
 * Return:
 *    PD_SUCCESS(0)  success
 *    PD_ERR_XXXXXX  otherwise
 *
 *----------------------------------------------------------------------------
 */

int PD_MODULE_INIT(lvds_init, (void *handle))
{
	/* register the LVDS driver */
	return pd_register(handle, &lvds_driver);
}

/*----------------------------------------------------------------------------
 *
 * Function: PD_MODULE_EXIT(lvds_exit, (void))
 *
 * Description:
 *    This function cleans up resources used by the LVDS driver
 *
 * Parameters:
 *    None
 *
 * Return:
 *    PD_SUCCESS(0):  always returns this
 *
 *----------------------------------------------------------------------------
 */

int PD_MODULE_EXIT(lvds_exit, (void))
{
	return PD_SUCCESS;
}

/*----------------------------------------------------------------------------
 *
 * Function: lvds_validate
 *
 * Description:
 *    Place holder for a future function
 *
 * Parameters:
 *    TBD
 *
 * Return:
 *    TBD
 *
 *----------------------------------------------------------------------------
 */

unsigned long lvds_validate (unsigned long cookie)
{                                                          /* lvds_validate */
	/* Validate magic cookie */
	/* TODO: Implement the magic cookie algorithm */
	return cookie;
}                                                          /* lvds_validate */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_open
 *
 * Description:
 *    This function creates an LVDS context and intialize it with LVDS
 *    attributes.
 *
 *    Internal LVDS port is available only on MGM platform, this port driver
 *    reads the GMCH device ID with pd_read_regs(PD_REG_PCI) to verify that
 *    it is supported on the current platform.
 *
 * Parameters:
 *    [IN] callback:    callback context
 *    [INOUT] context:  Device context.  This function will set the attributes
 *                      for this context, provided it is already allocated,
 *                      i.e. not NULL.
 *
 * Return:
 *    PD_ERR_NULL_PTR:  if either of the parameters is NULL
 *    PD_ERR_NODEV:     if an LVDS device is already up and running
 *    PD_ERR_NOMEM:     if a memory allocation failed
 *    PD_SUCCESS:       if successful
 *
 *----------------------------------------------------------------------------
 */

/* lvds context structure being declared and initialized */

static lvds_context_t lvds_context = {  /* lvds context structure */
	0,                   /* fp_width     */
	0,                   /* fp_height    */
	0,                   /* dual_channel: Default single channel */
	0,                   /* panel_type : 0-SPWG 1-OpenLDI*/
	LVDS_DEF_PANEL_FIT,  /* panel_fit    */
	LVDS_DEF_PANEL_DEPTH,/* panel_depth  */
	0xFFFF,              /* dither       */
	0,                   /* Main aspect ratio, default no */
	0,                   /* panel filter: Default fuzzy filtering */
	100,                 /* PWM Intensity */
	0xFFFF,              /* Inverter Frequency*/
	0,                   /* BLM Legacy Mode */

	PD_POWER_MODE_D0,    /* power_state   */
	0,                   /* chipset       */
	0,                   /* init_done     */
	0,                   /* num_attrs */
	0,                   /* PIPE flags    */
	0,                   /* Graphics Frequency */
	0,                   /* is gn4 based LVDS controller? */
	0,                   /* is pwm_done? */
	0,                   /* is tc_110MHz_clk? i.e., TC max LVDS to 110MHz*/

	NULL,                /* ptr to callback       */
	NULL,                /* ptr to timing table   */
	lvds_attrs,          /* ptr to attribute list */
	NULL,                /* ptr to native timing  */
	NULL,                /* current mode */
};

int lvds_open(pd_callback_t *callback, void **context)
{                                                              /* lvds_open */
	lvds_context_t *pd_context = (lvds_context_t*) &(lvds_context); /* static Global */
	pd_reg_t reg_list[2];
	int ret, i, valid_chipset = 0;
	unsigned short chipset;

	PD_TRACE_ENTER;



	/* make sure parameters are valid */
	if (!callback || !context) {
		PD_ERROR("invalid parameter");
		return (PD_ERR_NULL_PTR);
	}
	/* GMCH cannot support more than one device */
	if (lvds_driver.num_devices > 0) {
		return (PD_ERR_NODEV);
	}
	/* Verify that this is an GMCH with Internal LVDS available */
	reg_list[0].reg = 2;
	reg_list[1].reg = PD_REG_LIST_END;
	ret = callback->read_regs(callback->callback_context, reg_list, PD_REG_PCI);
	if(ret != PD_SUCCESS) {
		return ret;
	}
	chipset = (unsigned short)(reg_list[0].value & 0xffff);
	for (i = 0; i < ARRAY_SIZE(supported_chipset); i++) {
		if (chipset == supported_chipset[i]){
			valid_chipset = 1;
			break;
		}
	}
	if(!valid_chipset){
		return PD_ERR_NODEV;
	}

	/* Special handling for gn4 and beyond chipsets */
	if (chipset == PCI_DEVICE_ID_VGA_GM965 ||
		chipset == PCI_DEVICE_ID_VGA_GME965 ||
		chipset == PCI_DEVICE_ID_VGA_CTG ||
		chipset == PCI_DEVICE_ID_VGA_TNC ||
		chipset == PCI_DEVICE_ID_VGA_TNC_A0 ||
		chipset == PCI_DEVICE_ID_VGA_LNC) {
		lvds_context.gn4_plus = 1;
	}

	/* Initialize number of attributes */
	/* +1 is to include the end attribute */
	lvds_context.num_attrs = (unsigned char)chipset_attr_index + 1;

	/* Add chipset specific attrbutes.
	 * This can be expanded into a switch statement in future if required. */
	if (lvds_context.gn4_plus) {
		lvds_context.num_attrs += sizeof(lvds_965gm_attrs)/sizeof(pd_attr_t);
		for (i=0; i < sizeof(lvds_965gm_attrs)/sizeof(pd_attr_t); i++) {
			lvds_attrs[chipset_attr_index+i] = lvds_965gm_attrs[i];
		}
	}
	/* Make this a compile time so that size of vBIOS doesn't become > 64KB */
#if defined(CONFIG_PLB) || defined(CONFIG_TNC)
	/* pwm backlight control needs graphics frequency. we currently implement
	 * pwm backlight control for pouslbo only to limit the vbios lvds size. Each
	 * chipset have a different method of getting this value and chipset has a
	 * different multiplier. */
	if (chipset == PCI_DEVICE_ID_VGA_PLB ||
	    chipset == PCI_DEVICE_ID_VGA_TNC ||
	    chipset == PCI_DEVICE_ID_VGA_TNC_A0 ||
	    chipset == PCI_DEVICE_ID_VGA_LNC) {

		/* For plb/tnc, graphics frequency is obtained by sending an opcode to
		 * port 5 in the SCH Message Network. We call the read_regs with
		 * PD_REG_BRIDGE_OPCODE specifically for this purpose only.
		 *
		 * The input for this read_reg is the opcode that register data that
		 * we send into the Message control register*/
		reg_list[1].reg = PD_REG_LIST_END;
		ret = callback->read_regs(callback->callback_context,
			reg_list, PD_REG_BRIDGE_OPCODE);
		if(ret != PD_SUCCESS) {
			return ret;
		}

		/*set the graphics frequency*/
		pd_context->gfx_freq = (unsigned short) reg_list[0].value;
	}
#endif

	pd_context->callback = callback;           /* Save callback context */
	pd_context->chipset = chipset;            /* save the chipset ID   */
	*context = (void *) pd_context;

	PD_TRACE_EXIT;
	return PD_SUCCESS;
}                                                              /* lvds_open */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_init_device
 *
 * Description:
 *    Initializes the LVDS device, using the device context from the parameter
 *
 * Parameters:
 *    [INOUT] context:  device context
 *
 * Return:
 *    PD_ERR_NULL_PTR:  if parameter is invalid
 *    PD_SUCCESS:       if successful
 *
 *----------------------------------------------------------------------------
 */

int lvds_init_device (void *context)
{                                                       /* lvds_init_device */
	if (!context) {
		return (PD_ERR_NULL_PTR);
	}

	/* Don't need to do much to initialize this device */
	((lvds_context_t *)context)->init_done = 1;
	lvds_driver.num_devices++;

	return (PD_SUCCESS);
}                                                       /* lvds_init_device */


/*----------------------------------------------------------------------------
 *
 * Function: lvds_close
 *
 * Description:
 *    Releases resources allocated during lvds_open().  This function
 *    essentially frees a device object.
 *
 * Parameters:
 *    [INOUT] device_context:  device to be freed
 *
 * Return:
 *    PD_SUCCESS: always returns this
 *
 *----------------------------------------------------------------------------
 */

int lvds_close(void *device_context)
{
#ifndef CONFIG_MICRO

	lvds_context_t *pd_context = (lvds_context_t *)device_context;

	/* lvds_close */

	PD_TRACE_ENTER;

	/* Deallocate memory occupied by this device */
	if (device_context) {
		if ( NULL != pd_context->timing_table) {
			pd_free(pd_context->timing_table);
			pd_context->timing_table = NULL;
		}

		/* Free attribute list, if necessary */
		/* FIXME -- The following test will never call pd_free(), because the
		 * expression "!lvds_driver.num_devices" will yield 0 or 1, and that
		 * will never be greater than 1.  This is a potentially small memory
		 * leak, unless some other code frees it.
		 */
		if (!lvds_driver.num_devices > 1) {
			pd_free(pd_context->attr_list);
		}
		/* This allocated statically no need to free it */
		/* pd_free(device_context); */
		lvds_driver.num_devices--;
		pd_context->init_done = 0;
	}

	PD_TRACE_EXIT;
#endif
	return (PD_SUCCESS);
}                                                             /* lvds_close */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_set_mode
 *
 * Description:
 *    Sets LVDS to a new mode, specified by "mode".
 *
 * Parameters:
 *    [INOUT] context:  device context
 *    [IN]    mode:     information about the mode to switch to
 *    [IN]    flags:    can contain the following value
 *                PD_SET_MODE_FLAG_TEST:  only testing to see if mode is supported
 *
 * Return:
 *    PD_ERR_NULL_PTR:     if invalid parameter detected
 *    PD_ERR_MODE_NOTSUPP: if mode specified is not supported
 *    PD_SUCCESS:          if successful
 *
 *----------------------------------------------------------------------------
 */
int lvds_set_mode(void *context, pd_timing_t *mode, unsigned long flags)
{
	lvds_context_t *pd_context  = NULL;

	PD_DEBUG("lvds_set_mode)\n");
	PD_TRACE_ENTER;

	pd_context = (lvds_context_t *)context;

	/* Make sure these parameters are valid */
	if (!pd_context || !mode) {
		return (PD_ERR_NULL_PTR);
	}
	PD_DEBUG("lvds_set_mode: %ux%u", mode->width, mode->height);

	/* Make sure specified mode is supported */
	if ((pd_context->fp_width && (mode->width > pd_context->fp_width)) ||
		(pd_context->fp_height && (mode->height > pd_context->fp_height))) {
		return PD_ERR_MODE_NOTSUPP;
	}

	/* Do nothing if we are only want to know if a mode is supported */
	if (flags & PD_SET_MODE_FLAG_TEST) {
		return PD_SUCCESS;
	}

	pd_context->current_mode = mode;
	pd_context->pipe = flags;
	/* Enable panel fitting and return */
	lvds_panel_fit(pd_context);

	PD_TRACE_EXIT;

	return PD_SUCCESS;
}

int lvds_post_set_mode(void *context, pd_timing_t *mode, unsigned long flags)
{                                                          /* lvds_set_mode */
	lvds_context_t *pd_context  = NULL;
	unsigned long  port_control = 0;
	unsigned long  preserve     = 0;
	int            ret          = 0;

	PD_TRACE_ENTER;
	pd_context = (lvds_context_t *)context;

	PD_DEBUG("lvds_post_set_mode)\n");
	/* Make sure these parameters are valide */
	if (!pd_context || !mode) {
		return (PD_ERR_NULL_PTR);
	}

	/* Before enabling the LVDS port, make sure that display PLL for this pipe
	 * is enabled and the port is power sequenced on using the panel power
	 * sequencing logic. */

	preserve = 0x3E007803;
	port_control = preserve & lvds_read_reg(pd_context, 0x61180, PD_REG_MIO);
	port_control |= BIT(31);          /* enable LVDS port */
	if (flags & PD_SET_MODE_PIPE_B) {
		port_control |= BIT(30);
	}
	port_control |= (BIT(9)|BIT(8));  /* power up */
	port_control &= ~(BIT(21)|BIT(20)); /* Default sync polarites active high */

	/* For gn4+, dither moved to port_control from panel_fit reg */
	if (pd_context->gn4_plus) {
		if (pd_context->panel_depth == 18) {
			port_control |= BIT(25);
		}

		if (pd_context->dither != 0xFFFF) {
			if (pd_context->dither) {
				port_control |= BIT(25);
			} else {
				port_control &= ~BIT(25);
			}
		}
	}

	if((!pd_context->panel_fit) &&
	   ( mode->width < pd_context->fp_width ||
	     mode->height < pd_context->fp_height ) ){
		port_control |= BIT(15); /* enable border in active for centering */
	}

	/*
	 * Bit 24 for OpenLDI should be set to a 0 (1x18.0, 2x18.0, 1x24.0, 2x24.0).
	 * From (B-Spec, Ref# 22316, section 1.15.3.8.3)
	 *
	 * Bit 24 for SPWG should be set to a 1 (1x24.1 or 2x24.1).
	 *
	 * This was verified by comparing the timing diagram for 1x24.0 in
	 * OpenLDI spec (5.4.2.2 24-bit Single Pixel Mode, Unbalanced) with the
	 * same diagram in the Display BSpec for Napa or Gen4.
	 * A0 – A3 signals match the 1x24.0.
	 *
	 * From the OpenLDI spec (bit mappings are different):
	 *
	 * Table 5-2, Bit Number Mappings
	 * 18 bpp bit# 24 bpp bit# OpenLDI bit#
	 * 5           7           5
	 * 4           6           4
	 * 3           5           3
	 * 2           4           2
	 * 1           3           1
	 * 0           2           0
	 *             1           7
	 *             0           6
	 */

	/* Attribute panel_type description:
	 *  Attr ID    Attr Value        IntLVDS dataformat
	 *  =======    ===============   ==================
	 *    49       0  (SPWG)           1 (value of Bit 24)
	 *    49       1  (OpenLDI)        0 (value of Bit 24)
	 */
	if (pd_context->panel_type == 0) {
		port_control |= BIT(24); /* Dataformat 0 = SPWG, 1 = OpenLDI */
	}

	/* If the dual-channel is required, then power up second channel
	 * ClkB and B0, B1, B2, (B3) */
	if (pd_context->dual_channel) {
		port_control |= (BIT(5)|BIT(4));  /* ClkB */
		port_control |= (BIT(3)|BIT(2));  /* B0, B1, B2, (B3) */
	}

	/* Check for 18 or 24 bit panel */
	if (pd_context->panel_depth == 24) {
		/* If the panel is 24-bit (8-bpp), enable A3, (B3) pair. */
		port_control |= (BIT(7)|BIT(6));
	}

	/* Set the sync polarities correctly if there is a native dtd */
	if (pd_context->native_dtd) {
		/* Set bit 20 for hsync active low */
		if ((pd_context->native_dtd->mode_info_flags & PD_HSYNC_HIGH) == 0) {
			port_control |= BIT(20);
		}
		/* Set bit 21 for vsync active low */
		if ((pd_context->native_dtd->mode_info_flags & PD_VSYNC_HIGH) == 0) {
			port_control |= BIT(21);
		}
	}

    lvds_write_reg(pd_context, 0x61204, 0xABCD0000, 0xFFFFFFFF, PD_REG_MIO);

	lvds_write_reg(pd_context, 0x61180, port_control, 0xFFFFFFFF, PD_REG_MIO);
	ret = lvds_set_power(pd_context, PD_POWER_MODE_D0);
	if (ret) {
		PD_ERROR("PD set_power (D0) returned: 0x%x", ret);
		return ret;
	}

    lvds_write_reg(pd_context, 0x61204, BIT(1), BIT(1), PD_REG_MIO);


	PD_TRACE_EXIT;
	/* Set the mode as per given timings */
	return PD_SUCCESS;
}                                                          /* lvds_set_mode */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_set_attrs
 *
 * Description:
 *    Incorporate attributes in "list" into the device context.  This function
 *    will override the initial attributes set by init_attrs.
 *
 * Parameters:
 *    [INOUT] context:  device context
 *    [IN] num:         not used, but must not be 0.
 *    [IN] list:        list of attributes to incorporate into device context
 *
 * Return:
 *    PD_ERR_NULL_PTR:          if one of the parameters is invalid
 *    PD_ERR_ATTR_CANT_CHANGE:  attributes cannot be modified
 *    PD_SUCCESS: if successful
 *
 *----------------------------------------------------------------------------
 */
/* Tables required by Optimization Code. lvds_set_attrs() */
typedef struct _opt_table_data {
	unsigned long id;
	unsigned short block;
	unsigned short offset;
} lvds_opt_table_data_t;
static lvds_opt_table_data_t table_opt_data1[] = {

	/*<--- id ---------->  <---block---> <------ offset -------------------> */
	{PD_ATTR_ID_PANEL_DEPTH,     1, PD_OFFSETOF(lvds_context_t, panel_depth) },

	{PD_ATTR_ID_2_CHANNEL_PANEL, 1, PD_OFFSETOF(lvds_context_t, dual_channel)},
	{PD_ATTR_ID_LVDS_PANEL_TYPE, 1, PD_OFFSETOF(lvds_context_t, panel_type) },
	{PD_ATTR_ID_PANEL_FIT,       1, PD_OFFSETOF(lvds_context_t, panel_fit)   },
	{PD_ATTR_ID_DITHER,          1, PD_OFFSETOF(lvds_context_t, dither)      },
	{PD_ATTR_ID_MAINTAIN_ASPECT_RATIO,1,PD_OFFSETOF(lvds_context_t,aspect_ratio)},
	{PD_ATTR_ID_TEXT_TUNING,     1, PD_OFFSETOF(lvds_context_t, text_tune)},
	{PD_ATTR_ID_PWM_INTENSITY,   1, PD_OFFSETOF(lvds_context_t, pwm_intensity)},
	{PD_ATTR_ID_INVERTER_FREQ,   1, PD_OFFSETOF(lvds_context_t, inverter_freq)},
	{PD_ATTR_ID_BLM_LEGACY_MODE, 1, PD_OFFSETOF(lvds_context_t, blm_legacy_mode)},
	{LVDS_ATTR_ID_TC_LVDS_CLK,   1, PD_OFFSETOF(lvds_context_t, tc_110MHz_clk)},

	/*<--- id ---------->  <---block---> <------ offset -------------------> */
	{PD_ATTR_ID_FP_PWR_T1,       2,     0 }, /* 6 */
	{PD_ATTR_ID_FP_PWR_T2,       2,     0 }, /* 7 */
	{PD_ATTR_ID_FP_PWR_T3,       2,     0 }, /* 8 */
	{PD_ATTR_ID_FP_PWR_T4,       2,     0 }, /* 9 */
	{PD_ATTR_ID_FP_PWR_T5,       2,     0 }  /* 10 */
};
/* End of Tables required by Optimization Code. lvds_set_attrs() */

int lvds_set_attrs (void *context, unsigned long num, pd_attr_t *list)
{
	lvds_context_t *pd_context = (lvds_context_t *) context;
	pd_attr_t      *attr       = NULL;
	unsigned short i           = 0;
	unsigned short j           = 0;
	int            ret         = PD_SUCCESS;
	/* no of case IDs in the global table */
	int num_case_ids = sizeof(table_opt_data1)/sizeof(lvds_opt_table_data_t);

	/* basic parameter check */
	if (!context || !num || !list) {
		return PD_ERR_NULL_PTR;
	}

	PD_DEBUG("lvds_set_attrs()\n");
	for (i = 0; i < num; i++, list++) {
		/* do nothing if the attribute has not been changed */
		if (!(list->flags & PD_ATTR_FLAG_VALUE_CHANGED)) {
			continue;
		}

		/* attributes can't be changed after init has been completed */
		if (list->flags & PD_ATTR_FLAG_USER_INVISIBLE &&
			pd_context->init_done) {
			return PD_ERR_ATTR_CANT_CHANGE;
		}

		/* Set the internal attributes' list.  Note that although get_attr() can
		 * return NULL theortically, it will not do so here because all the
		 * attribute IDs in this switch statement comes from lvds_attrs[],
		 * a list that is automatically initialized into pd_context.  This is
		 * why we are not checking for NULL after calling get_attr().
		 */
#if 0	/* ORIGINAL SWITCH STATEMENT */
		switch (list->id) {
			case PD_ATTR_ID_FP_PWR_T1:
			case PD_ATTR_ID_FP_PWR_T2:
			case PD_ATTR_ID_FP_PWR_T3:
			case PD_ATTR_ID_FP_PWR_T4:
			case PD_ATTR_ID_FP_PWR_T5:
				/* current_value should not exceed the predefined max value */
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				attr->current_value = LVDS_MIN(
						((pd_range_attr_t *)list)->current_value,
						attr->_pad1);
				break;

			case PD_ATTR_ID_PANEL_DEPTH:
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				attr->current_value = list->current_value;
				pd_context->panel_depth = (unsigned char) attr->current_value;
				break;

			case PD_ATTR_ID_2_CHANNEL_PANEL:
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				attr->current_value = ((pd_bool_attr_t *)list)->current_value;
				pd_context->dual_channel = (attr->current_value?1:0);
				break;

			case PD_ATTR_ID_LVDS_PANEL_TYPE:
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				attr->current_value = ((pd_bool_attr_t *)list)->current_value;
				pd_context->panel_type = (attr->current_value?1:0);
				break;

			case PD_ATTR_ID_PANEL_FIT:
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				attr->current_value = ((pd_bool_attr_t *)list)->current_value;
				pd_context->panel_fit = (attr->current_value?1:0);
				break;

			default:
				/* do nothing if we have an unknown ID */
				break;
		}
#endif
		/* OPTIMIZATION CODE BEGINS FOR THE ABOVE SWITCH
		 *----------------------------------------------
		 * Step 1: First identify the code common to all case blocks.
		 *         This we call it "common code block" Since this is to be
		 *         executed by all the case blocks.
		 *
		 * Step 2: Group the cases into blocks based on how we can combine them
		 *
		 *         Eg: case 0: ptr->x = 1; break; // similar code
		 *        case 1: ptr->y = 1; break; // similar code
		 *  ------Combined block ----
		 *        case 0:
		 *        case 1: PTR_OFFSET_TYPE(ptr, table[i].offset) = 1; break;
		 *        This is an important step because we save code space by
		 *        mapping many cases to smaller number of blocks. In the above
		 *        we use index to get the right offset of the ptr.
		 *
		 * Step 3: Assign block IDs to each block. Put the case ID, block ID
		 *         and other information such as offsets of ptr in a global
		 *         table.
		 *
		 * Step 4: During run-time , search through the global table, find the
		 *         matching case ID, and execute the common code. Next get the
		 *         block ID and execute the block specific code. For the index,
		 *         retrieve it from the table for the corresponding case ID.
		 *
		 * Step 5: If no matching case ID is found, error it out.
		 *
		 * Let's look at the optimization code for the above switch.
		 * The code below searches for the list->id in a global table where
		 * we store all the case values along with block numbers and other
		 * information. After we find a valid id in the table , we execute the
		 * common code for all cases first and then we retrieve the block id.
		 * The block id is necesary to determine which block does the id belong
		 * to. This is used to execute block specific code. Similar to switch
		 * cases ONLY here we try to minimize the no of blocks.
		 *
		 * In this example , we store the offsets of field names of a ptr in the
		 * global table.This is necessary to combine cases with "similar" but
		 * not "same" codes.
		 *
		 * Eg: case 0: ptr->x = 1; break; // similar code
		 *     case 1: ptr->y = 1; break; // similar code
		 *  -- Combined block ----
		 *    case 0:
		 *    case 1: PTR_OFFSET_TYPE(ptr, table[i].offset) = 1; break;
		 *
		 * By reducing the number of blocks we save on Code space. After we
		 * finish our work , we exit the for loop and check for invalid ID
		 * passed by the upper layer. This is akin to default in the
		 * switch block
		 *
		 * CAUTION: If there is any change in the switch above, this code
		 * along with the tables have to be re-written and changed according
		 * to the new behaviour of the switch. Examples include adding a new
		 * case in the switch. The reason for all this mumbo-jumbo is to
		 * reduce code size in VBIOS, where we are running out of code space.
		 */
		for(j = 0; j < num_case_ids; j++) {
			/* Search for the attribute ID in the global table */
			if(list->id == table_opt_data1[j].id) {
				/* Run the common code for all the blocks */
				attr = pd_get_attr(pd_context->attr_list, pd_context->num_attrs,
					list->id, 0);
				/* Once we get a valid ID, need to find out which block it
				 *  belongs so we can execute block specific code.
				 */
				if(table_opt_data1[j].block == 1) { /* block 1 */
					/* Got the block. Need the offset to the struct for that ID
					* so we can store value at corresponding offset to get the
					* desired behaviour for that ID.This is the code that makes
					* the whole optimization work because we are combining the
					* case IDs into a single block which saves code.
					*/
					attr->current_value = list->current_value;
					PTR_OFFSET_USHORT(pd_context, table_opt_data1[j].offset) =
					(unsigned short) attr->current_value;

				} else { /* block 2. We only have two blocks. */
					attr->current_value = LVDS_MIN(
						list->current_value, ((pd_range_attr_t *)attr)->max);
				}
				break; /* We found a valid ID, so break inner for loop */
			}
		}
	}
	/* panel_type 0 (SPWG) isn't available for 18-bit depth */
	PD_DEBUG("in LVDS_set_attributes()\n");
	if (pd_context->panel_depth == 18) {
		pd_context->panel_type = 1;
	}
	PD_DEBUG("IntLVDS: dual_channel=%u", pd_context->dual_channel);
	PD_DEBUG("IntLVDS: panel_type=%u panel_fit=%u panel_dep=%u dither=%u",
		pd_context->panel_type, pd_context->panel_fit,
		pd_context->panel_depth, pd_context->dither);
	PD_DEBUG("IntLVDS: keep_aspect_ratio=%u text_tune=%lu",
		pd_context->aspect_ratio, pd_context->text_tune);
	PD_DEBUG("IntLVDS: PWM Intensity=%u Inverter Freq=%u, BLM legacy mode =%u",
		pd_context->pwm_intensity, pd_context->inverter_freq,
		pd_context->blm_legacy_mode);
	PD_DEBUG("IntLVDS: tc_110MHz_clk = %u", pd_context->tc_110MHz_clk);

	if (pd_context->init_done) {
		/* When emgd_driver_pre_init() pokes new attrs into this port driver,
		 * pd_context->current_mode must be set before calling
		 * lvds_panel_fit(), so set it to the first entry in the timing table:
		 */
		if (pd_context->current_mode == NULL) {
			pd_context->current_mode = pd_context->timing_table;
		}
		lvds_panel_fit(pd_context);
	}

	return ret;
} /* lvds_set_attrs */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_get_attrs
 *
 * Description:
 *    Extracts the attribute list and the number of elements in the list
 *    from the device context.
 *
 * Parameters:
 *    [IN] context: device context to extract information from
 *    [OUT] num:    number of elements in list
 *    [OUT] list:   list of attributes from the device context
 *
 * Return:
 *    PD_ERR_NULL_PTR: if one of the parameters is invalid
 *    PD_SUCCESS:      if successful
 *
 *----------------------------------------------------------------------------
 */
int lvds_get_attrs (void *context, unsigned long *num, pd_attr_t **list)
{                                                         /* lvds_get_attrs */
	/* basic parameter check */
	if (!context || !num || !list) {
		return PD_ERR_NULL_PTR;
	}

	PD_DEBUG("lvds_get_attrs()\n");
	/* Nothing fancy, just extracting the elements from the list */
	*list = ((lvds_context_t *)context)->attr_list;
	*num  = ((lvds_context_t *)context)->num_attrs;

	return PD_SUCCESS;
}                                                         /* lvds_get_attrs */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_get_timing_list
 *
 * Description:
 *
 *
 * Parameters:
 *    [IN] context: device context to extract information from
 *    [OUT] in_list:
 *    [OUT] list:
 *
 * Return:
 *    PD_ERR_NULL_PTR: if one of the parameters is invalid
 *    PD_ERR_NOMEM:    if internal memory allocate failed
 *    PD_SUCCESS:      if successful
 *
 *----------------------------------------------------------------------------
 */
int lvds_get_timing_list (void *context, pd_timing_t *in_list,
	pd_timing_t **list)
{                                                   /* lvds_get_timing_list */
	lvds_context_t *pd_context = (lvds_context_t *)context;
	pd_dvo_info_t lvds_info = {0, 0, 1, 0, 0, 0, 0, 0};
	pd_display_info_t lvds_display_info = {0, 0, 0, 0, NULL};
	int ret;

	PD_DEBUG("lvds_get_timing_list()\n");
	lvds_get_dclk( pd_context, &lvds_info );

	PD_DEBUG("chipset = 0x%x", pd_context->chipset);
	lvds_display_info.panel_fit = (unsigned char) pd_context->panel_fit;
	ret = pd_filter_timings(pd_context->callback->callback_context,
		in_list, &pd_context->timing_table, &lvds_info, &lvds_display_info);

	/* Helper function will return the below values */
	pd_context->native_dtd = lvds_display_info.native_dtd;
	pd_context->fp_width = lvds_display_info.width;
	pd_context->fp_height = lvds_display_info.height;

	*list = pd_context->timing_table;
	return ret;
}                                                   /* lvds_get_timing_list */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_set_power
 *
 * Description:
 *  Sets LVDS to the specified power state
 *
 *  Conversion between IEGD timer values to LVDS port timer values
 *
 *  SEPG(?) IEGD    LVDS Port    Program bits     min     max
 *  ----    ------- ------------ ---------------- ------ ---------
 *  T1+T2   T1 ms   T1+T2 100us  0x61208 [28:16]  0 ms   819.2 ms
 *  T5      T2 ms   T5    100us  0x61208 [12:00]  0 ms   819.2 ms
 *  T6      T3 ms   Tx    100us  0x6120C [12:00]  0 ms   819.2 ms
 *  T3      T4 ms   T3    100us  0x6120C [28:16]  0 ms   819.2 ms
 *  T4      T5 ms   T4    100ms  0x61210 [04:00]  0 ms   3200  ms
 *
 *  Reg     = Value
 *  ------    -------
 *  0x61208 = [T1 T2]
 *  0x6120C = [T4 T3]
 *  0x61210 = [   T5]
 *
 * Parameters:
 *    [IN] context: device context to extract information from
 *    [OUT] in_list:
 *    [OUT] list:
 *
 * Return:
 *    PD_ERR_NULL_PTR:      if context is NULL
 *    PD_ERR_INVALID_POWER: if "state" is invalid
 *    PD_SUCCESS:           if successful
 *
 *----------------------------------------------------------------------------
 */
/* Tables required by the optimization codes in lvds_set_power() */
typedef struct {
	unsigned char id1;
	unsigned char id2;
	unsigned char bit;
	unsigned long reg;
} opt_set_power_t;

static opt_set_power_t table_set_power[] = {
	/* id1                  id2                   bit    reg */
	{ PD_ATTR_ID_FP_PWR_T1, PD_ATTR_ID_FP_PWR_T2, 1,     0x61208 },  /* D0 */
	{ PD_ATTR_ID_FP_PWR_T4, PD_ATTR_ID_FP_PWR_T3, 0,     0x6120C },  /* Dx */
};

int lvds_set_power(void *context, unsigned long state)
{                                                         /* lvds_set_power */
	unsigned long  i           = 0;
	lvds_context_t *pd_context = (lvds_context_t *)context;
	pd_attr_t      *tattr      = NULL;  /* holds time delay b/ pwr transition */
	unsigned long            delay = 0, delay1;

	PD_DEBUG("state = %lu", state);

	PD_DEBUG("lvds_set_power() to state = %lu\n",state);
	/* Basic parameter check */
	if (!context) {
		PD_DEBUG("No context");
		return PD_ERR_NULL_PTR;
	}
	PD_DEBUG("pd_context=0x%lx", (unsigned long)pd_context);

	/* Check for invalid state */
	if (state > PD_POWER_MODE_D3) {
		PD_DEBUG("Invalid power state");
		return PD_ERR_INVALID_POWER;
	}

	/* Get the index into above table */
	if (state == PD_POWER_MODE_D0) {
		i = 0;
	} else {
		i = 1;
	}

	lvds_write_reg(pd_context, 0x61204, 0xABCD0000, 0xFFFFFFFF, PD_REG_MIO);

	/* Program panel power up/down delays: Either T1/T2 or T3/T4*/
	tattr = pd_get_attr(pd_context->attr_list,
		pd_context->num_attrs, table_set_power[i].id1, 0);
	/* Convert ms to 100us */
	delay1 = tattr->current_value;
	delay = (tattr->current_value * 10) << 16;
	tattr = pd_get_attr(pd_context->attr_list,
		pd_context->num_attrs, table_set_power[i].id2, 0);
	delay1 += tattr->current_value;
	delay |= tattr->current_value * 10;

	lvds_write_reg(pd_context, table_set_power[i].reg, delay, 0x1FFF1FFF, PD_REG_MIO);

	/* Program power cycle delay: convert ms to 100ms */
	delay = pd_get_attr(pd_context->attr_list,
		pd_context->num_attrs, PD_ATTR_ID_FP_PWR_T5, 0)->current_value;
	delay1 += delay;
	/* TODO: Write reference divider [31:8] */
	delay = ((delay/100+1) & 0xFF) | ((pd_context->gfx_freq*100/2-1)<<8);
	lvds_write_reg(pd_context, 0x61210, delay, 0xFFFFFFFF, PD_REG_MIO);

	/* Power state target */
	lvds_write_reg(pd_context, 0x61204, table_set_power[i].bit, BIT(0), PD_REG_MIO);

	/* Power down on reset available on crestline onwards */
	lvds_write_reg(pd_context, 0x61204, BIT(1), BIT(1), PD_REG_MIO);

/* Make this a compile time so that size of vBIOS doesn't become > 64KB */
#if defined(CONFIG_PLB) || defined(CONFIG_TNC)
	/* PWM is a method of controlling the backlight intensity.
	 * It is not method to turn on baclkight.
	 * We still need the PD method to turn on the backlight.
	 *
	 * This feature is for Pouslbo Only. We check that the user has set the
	 * inverter frequency. Default intensity, if not set, is 100%
	 *
	 * Due to the high amount of calculation, we want to only set this register
	 * if it has not been ser previously. The register could be
	 * "brought forward" from VBIOS.
	 */
	if(pd_context->inverter_freq != 0xFFFF &&  /* Overwritten by set_attr */
		(pd_context->chipset == PCI_DEVICE_ID_VGA_PLB ||
		 pd_context->chipset == PCI_DEVICE_ID_VGA_TNC ||
		 pd_context->chipset == PCI_DEVICE_ID_VGA_TNC_A0 ||
		 pd_context->chipset == PCI_DEVICE_ID_VGA_LNC) &&
		 !pd_context->pwm_done) {
		unsigned long reg_value = 0;
		unsigned long percentage = 0;
		unsigned long calculation = 0;
		unsigned long blc_pwm_ctl2 = 0;

		/* We first need to get the graphics frequency, which will be used to
		 * calculate Backlight Modulation Frequency[BMF]. BMF will be used to
		 * fill up the 15 MSB in the 0x61254 register
		 *
		 * The calculation for the Modulation Frequency field in the
		 * BLC_PWM_CTL Register is:
		 *
         *     Reference Clock Freq               1
         *     -----------------------   x    ------------------
         *            Divider                   PWM Freq in Hz
		 *
		 */
#if 0
		/* GMA accurate calculation that requires "calculation" to be an
		 * unsigned long long typedef */
		calculation = pd_context->gfx_freq * PWM_FREQ_CALC_CONSTANT_2;
		calculation = calculation / 0x20; /*pouslbo specific divider*/
		calculation = calculation * PWM_FREQ_CALC_CONSTANT_1;
		calculation = calculation / pd_context->inverter_freq;
		calculation = calculation / PWM_FREQ_CALC_CONSTANT_1;
#endif
		/* Some system bios cannot take 64 bit data type. Using a more
		 * simplified calculation that is not too accurate if the inputs
		 * are not round numbers */
		calculation = pd_context->inverter_freq * 0x20; /* plb/tnc divider */
		calculation = (pd_context->gfx_freq * PWM_FREQ_CALC_CONSTANT_2) /
			calculation;

		/* Writing the register: 15 MSB is the max lvds clock / 32.
		* Bit 16 can either be legacy or non legacy depending upon Attr #72. */
		if (pd_context->gn4_plus) {
			blc_pwm_ctl2 = (1L << 31) |
				(pd_context->blm_legacy_mode << 30) |
				((pd_context->pipe == PD_SET_MODE_PIPE_B)?1L:0L << 29);
			lvds_write_reg(pd_context, 0x61250, blc_pwm_ctl2, 0xFFFFFFFF, PD_REG_MIO);
			reg_value = (calculation & 0xFFFF)<<16;
		} else {
			reg_value = ((calculation & 0xFFFE) |
				pd_context->blm_legacy_mode)<<16;
		}

		/* The 16 LSB is a value that the user sets in configuration.
		 * user sets the value in percentage.
		 * We convert it into the clock speed */
		percentage = ( pd_context->pwm_intensity * (unsigned long)calculation);
		reg_value |= (unsigned long)( percentage / (int)100 ) & 0xFFFE;
		lvds_write_reg(pd_context, 0x61254, reg_value, 0xFFFFFFFF, PD_REG_MIO);

		/* set the flag so that we only do this function once */
		pd_context->pwm_done = 1;
	}
#endif

	if (state != PD_POWER_MODE_D0) {
		/* Wait until the current power up/down sequence is complete */
		i = 0;
		while(lvds_read_reg(pd_context, 0x61200, PD_REG_MIO) & 0x80000000L) {
			i++;
			if(i > 0x100000L) {
				break;
			}
		}
		lvds_write_reg(pd_context, 0x61180, 0, BIT(31), PD_REG_MIO);
	}

#ifdef CONFIG_TNC
#if 0

/*-----------------------------------------------------------------------------
 * LPC Register Offsets. Used for LVDS BKLT control. Registers are part
 * Atom E6xx [D31:F0]
 ----------------------------------------------------------------------------*/
#define RGEN    0x20
#define RGIO    0x24
#define RGLVL   0x28
#define TNC_LVDS_VDDEN	 BIT(0)
#define TNC_LVDS_BKLTEN  BIT(1)
#define TNC_LVDS_BKLTCTL BIT(2)

	if (pd_context->chipset == PCI_DEVICE_ID_VGA_TNC ||
		pd_context->chipset == PCI_DEVICE_ID_VGA_TNC_A0 ||
		pd_context->chipset == PCI_DEVICE_ID_VGA_LNC) {
		unsigned long value;

		/* Enable backlight for LVDS: based on observed si behavior:
		 * Subject to change based on si DE feedback */
		if (state == PD_POWER_MODE_D0) {
			value = TNC_LVDS_BKLTCTL|TNC_LVDS_BKLTEN|TNC_LVDS_BKLTCTL;
		} else {
			value = 0;
		}
		lvds_write_reg(pd_context, RGEN, value,
			TNC_LVDS_BKLTCTL|TNC_LVDS_BKLTEN|TNC_LVDS_BKLTCTL, PD_REG_LPC);
		lvds_write_reg(pd_context, RGIO, value,
			TNC_LVDS_BKLTCTL|TNC_LVDS_BKLTEN|TNC_LVDS_BKLTCTL, PD_REG_LPC);
	}
#endif
#endif

	/* update power state */
	pd_context->power_state = state;
	return PD_SUCCESS;
}                                                         /* lvds_set_power */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_get_power
 *
 * Description:
 *   Returns the current LVDS power state back to the caller.
 *
 * Parameters:
 *    [IN] context: device context to extract information from
 *    [OUT] state:  current power state
 *
 * Return:
 *    PD_ERR_NULL_PTR:      if one of the parameters is invalid
 *    PD_SUCCESS:           if successful
 *
 *----------------------------------------------------------------------------
 */

int lvds_get_power (void *context, unsigned long *state)
{                                                         /* lvds_get_power */
	if ((NULL == context) || (NULL == state)) {
		return PD_ERR_NULL_PTR;
	}
	PD_DEBUG("lvds_get_power()\n");
	/* The caller should be able to do this himself, but whatever */
	*state = ((lvds_context_t *) context)->power_state;
	return PD_SUCCESS;
}                                                         /* lvds_get_power */

int lvds_save(void *context, void **state, unsigned long flags)
{
	*state = NULL;
	return PD_SUCCESS;
}

int lvds_restore(void *context, void *state, unsigned long flags)
{
	int ret = PD_SUCCESS;
	return ret;
}

/*----------------------------------------------------------------------
 * Function: lvds_get_port_status()
 *
 * Description:  It is called to get the information about the display
 *
 * Parameters:  context - Port driver's context
 *				port_status - Returns the display type and connection state
 *
 * Return:      PD_SUCCESS(0)  success
 *              PD_ERR_XXXXXX  otherwise
 *----------------------------------------------------------------------*/
int lvds_get_port_status(void *context, pd_port_status_t *port_status)
{
	/* Display connection cannot be determined */
	port_status->display_type = PD_DISPLAY_LVDS_INT;
	port_status->connected    = PD_DISP_STATUS_UNKNOWN;
	return PD_SUCCESS;
}

static unsigned long lvds_read_reg(lvds_context_t *pd_context,unsigned long reg,
	unsigned long reg_type)
{
	pd_reg_t list[2];
	int ret;

	list[0].reg = reg;
	list[0].value = 0;
	list[1].reg = PD_REG_LIST_END;
	ret = pd_context->callback->read_regs(
		pd_context->callback->callback_context, list, reg_type);
	if (ret) {
		PD_ERROR("LVDS read regs: Failed.");
	}
	return list[0].value;
}

/*----------------------------------------------------------------------------
 *
 * Function: lvds_write_reg
 *
 * Description:
 *    Writes bits in "value" into a register.  Bits written are dictated by
 *    the "change_bits" mask.
 *
 * Parameters:
 *    [IN] pd_context:  device context, dispatcher to the actual write_reg
 *                      function
 *    [IN] reg:  register to write value to
 *    [IN] value:  value to change the register to
 *    [IN] change_bits:  bit mask, the bits set to "1" will be modified by
 *                       the corresponding bits in "value"
 *
 * Return:
 *    None
 *
 *----------------------------------------------------------------------------
 */
static void lvds_write_reg(lvds_context_t *pd_context, unsigned long reg,
		unsigned long value,
		unsigned long change_bits,
		unsigned long reg_type)
{                                                         /* lvds_write_reg */
	pd_reg_t list[2];
	int ret;

	PD_DEBUG("ENTER");

	list[0].reg = reg;
	list[0].value = (lvds_read_reg(pd_context, reg, PD_REG_MIO) & ~change_bits) | value;
	list[1].reg = PD_REG_LIST_END;
	ret = pd_context->callback->write_regs(
		pd_context->callback->callback_context, list, reg_type);
	if (ret) {
		PD_ERROR("LVDS write regs: Failed.");
	}
	PD_DEBUG("EXIT");
	return;
}                                                         /* lvds_write_reg */

/*----------------------------------------------------------------------------
 *
 * Function: lvds_panel_fit
 *
 * Description:
 *    Enables panel fitting
 *
 * Parameters:
 *    [IN] pd_context:  device context
 *
 * Return:
 *    None
 *
 *----------------------------------------------------------------------------
 */
static void lvds_panel_fit(lvds_context_t *pd_context)
{
	/* enable auto vertical ratio */
	/* enable auto horizantal ratio */
	/* no dither */
	unsigned long panel_fit_reg = 0x00000220;
	PD_DEBUG("lvds_panel_fit() \n");

	PD_TRACE_ENTER;

	if (pd_context->current_mode->width != pd_context->fp_width ||
		pd_context->current_mode->height != pd_context->fp_height) {
		/* Enable panel fitting */
		/* vertical interpolation = bilinear */
		/* horizontal interpolation = bilinear */
		panel_fit_reg |= 0x80000440;
	}
	/* Enable dither based on default/user-set value:
	 *   By default
	 *        dither = 1 for 18-bit panels
	 *               = 0 for 24-bit panels.
	 *   But this behavior can be changed by setting the DITHER attribute.
	 *        When user sets the attribute, dither will be updated
	 *        as part of attribute processing in set attributes. */
	/* For gn4 based chipsets dither is controlled in port_control register */
	if (!pd_context->gn4_plus) {
		/* Default behavior */
		if (pd_context->panel_depth == 18) {
			panel_fit_reg |= BIT(3);
		}

		/* Overwritten by set attribute */
		if (pd_context->dither != 0xFFFF) {
			if (pd_context->dither) {
				panel_fit_reg |= BIT(3);
			} else {
				panel_fit_reg &= ~BIT(3);
			}
		}
	}

	if (pd_context->gn4_plus) {
		unsigned long src_ratio, dest_ratio;
		panel_fit_reg = 0;
		if (pd_context->native_dtd &&
			(pd_context->current_mode->width != pd_context->native_dtd->width ||
			pd_context->current_mode->height !=
				pd_context->native_dtd->height)) {
			/* Enable panel fitter */
			panel_fit_reg = 0x80000000;

			/* Select the pipe */
			if (pd_context->pipe & PD_SET_MODE_PIPE_B) {
				panel_fit_reg |= BIT(29);   /* bits[30:29] = 01 for pipe B */
			}

			/* Scaling mode:
			 *    Default - Auto scaling src_ratio == dest_ratio
			 *    Piller box scaling - src_ratio < dest_ratio
			 *    Letter box scaling - src_ratio > dest_ratio */

			/* To make this work correctly, port driver shall know the
			 * size of the framebuffer, not the src mode. Most of the
			 * times the src mode is fb, but not all the cases.
			 * User has an attribute to change
			 *    1. Between Pillerbox and auto, and vice versa
			 *                and
			 *    2. Between Letterbox and auto, and vice versa.
			 */
			if (pd_context->aspect_ratio) {
				src_ratio = (pd_context->current_mode->width << 10)/
					(pd_context->current_mode->height);
				dest_ratio = (pd_context->native_dtd->width << 10)/
					(pd_context->native_dtd->height);

				if (dest_ratio > src_ratio) {
					/* Pillarbox scaling */
					panel_fit_reg |= BIT(27);
				} else if (dest_ratio < src_ratio) {
					/* Letterbox scaling */
					panel_fit_reg |= BIT(27) | BIT(26);
				}
			}

			/* Filter coefficient select: pd_context->text_tune = 0,1,2 */
			panel_fit_reg |= (pd_context->text_tune << 24);
		}
	}

	lvds_write_reg(pd_context, 0x61230, panel_fit_reg, 0xFFFFFFFF, PD_REG_MIO);
	PD_DEBUG("panel_fit_reg 0x61230 = 0x%lx", panel_fit_reg);
}


/*----------------------------------------------------------------------------
 *
 * Function: lvds_get_dclk
 *
 * Description:
 *    Gets the Dclk for LVDS
 *
 * Parameters:
 *    [IN] pd_context:  device context
 *	  [OUT]lvds_info:   Structure that contains the min and max dclk
 *
 * Return:
 *    None
 *
 *----------------------------------------------------------------------------
 */
static void lvds_get_dclk(lvds_context_t *pd_context, pd_dvo_info_t *lvds_info )
{
	PD_DEBUG("lvds_get_dclk()\n");
	/* Get the min and max dclks for lvds */
	if (pd_context->dual_channel) {
		lvds_info->min_dclk = LVDS_MIN_DCLK * 2 ;
		lvds_info->max_dclk = LVDS_MAX_DCLK * 2;
	} else {
		lvds_info->min_dclk = LVDS_MIN_DCLK;
		lvds_info->max_dclk = LVDS_MAX_DCLK;
	}
/* This #define is the result of code size reduction effort. */
#ifdef CONFIG_CTG
	/* Set dclk for GM965 */
	if(pd_context->chipset==PCI_DEVICE_ID_VGA_CTG){
		/* Set dclk for GM965/CTG */
		if (pd_context->dual_channel) {
			lvds_info->min_dclk = LVDS_GM965_DUAL_MIN_DCLK ;
			lvds_info->max_dclk = LVDS_GM965_DUAL_MAX_DCLK;
		} else {
			lvds_info->min_dclk = LVDS_GM965_SINGLE_MIN_DCLK;
			lvds_info->max_dclk = LVDS_GM965_SINGLE_MAX_DCLK;
		}
	}
#else

	/* Set dclk for 915GM */
	if(pd_context->chipset==PCI_DEVICE_ID_VGA_915AL){
		if (pd_context->dual_channel) {
			lvds_info->min_dclk = LVDS_915GM_DUAL_MIN_DCLK ;
			lvds_info->max_dclk = LVDS_915GM_DUAL_MAX_DCLK;
		} else {
			lvds_info->min_dclk = LVDS_915GM_SINGLE_MIN_DCLK;
			lvds_info->max_dclk = LVDS_915GM_SINGLE_MAX_DCLK;
		}
	} else if(pd_context->chipset==PCI_DEVICE_ID_VGA_945GM ||
				pd_context->chipset==PCI_DEVICE_ID_VGA_945GME){
		/* Set dclk for 945GM */
		if (pd_context->dual_channel) {
			lvds_info->min_dclk = LVDS_945GM_DUAL_MIN_DCLK ;
			lvds_info->max_dclk = LVDS_945GM_DUAL_MAX_DCLK;
		} else {
			lvds_info->min_dclk = LVDS_945GM_SINGLE_MIN_DCLK;
			lvds_info->max_dclk = LVDS_945GM_SINGLE_MAX_DCLK;
		}
	} else if(pd_context->chipset==PCI_DEVICE_ID_VGA_GM965 ||
				pd_context->chipset==PCI_DEVICE_ID_VGA_GME965){
		/* Set dclk for GM965 */
		if (pd_context->dual_channel) {
			lvds_info->min_dclk = LVDS_GM965_DUAL_MIN_DCLK ;
			lvds_info->max_dclk = LVDS_GM965_DUAL_MAX_DCLK;
		} else {
			lvds_info->min_dclk = LVDS_GM965_SINGLE_MIN_DCLK;
			lvds_info->max_dclk = LVDS_GM965_SINGLE_MAX_DCLK;
		}
	}
#endif

#ifdef CONFIG_TNC
	/* Get the min and max dclks for Atom E6xx lvds */
	if ((pd_context->chipset == PCI_DEVICE_ID_VGA_TNC) ||
	    (pd_context->chipset == PCI_DEVICE_ID_VGA_TNC_A0) ||
	    (pd_context->chipset == PCI_DEVICE_ID_VGA_LNC)) {
		lvds_info->min_dclk = LVDS_TNC_SINGLE_MIN_DCLK;
		/* Experimental feature to raise TC LVDS clk to 110MHz. */
		if (pd_context->tc_110MHz_clk) {
			lvds_info->max_dclk = 110000L;
		} else {
			lvds_info->max_dclk = LVDS_TNC_SINGLE_MAX_DCLK;
		}
	}
#endif
}

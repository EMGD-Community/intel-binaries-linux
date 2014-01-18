/*
 *-----------------------------------------------------------------------------
 * Filename: lvds.h
 * $Revision: 1.8 $
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
 *  This is header file for Internal LVDS PORT DRIVER.
 *-----------------------------------------------------------------------------
 */

#ifndef _PD_LVDS_H
#define _PD_LVDS_H

#include <pd.h>
#include <pci.h>

/* Values received from DPG hardware engineer. These are for single channel.  */

#ifndef PCI_DEVICE_ID_VGA_855
#define PCI_DEVICE_ID_VGA_855      0x3582
#endif
#ifndef PCI_DEVICE_ID_VGA_915AL
#define PCI_DEVICE_ID_VGA_915AL    0x2592
#endif
#ifndef PCI_DEVICE_ID_VGA_945GM
#define PCI_DEVICE_ID_VGA_945GM    0x27A2
#endif
#ifndef PCI_DEVICE_ID_VGA_945GME
#define PCI_DEVICE_ID_VGA_945GME   0x27AE
#endif
#ifndef PCI_DEVICE_ID_VGA_GM965
#define PCI_DEVICE_ID_VGA_GM965    0x2A02
#endif
#ifndef PCI_DEVICE_ID_VGA_GME965
#define PCI_DEVICE_ID_VGA_GME965   0x2A12
#endif
#ifndef PCI_DEVICE_ID_VGA_CTG
#define PCI_DEVICE_ID_VGA_CTG      0x2A42
#endif
#ifndef PCI_DEVICE_ID_VGA_PLB
#define PCI_DEVICE_ID_VGA_PLB      0x8108
#endif
#ifndef PCI_DEVICE_ID_VGA_TNC
#define PCI_DEVICE_ID_VGA_TNC      0x4108
#endif
#ifndef PCI_DEVICE_ID_VGA_TNC_A0
#define PCI_DEVICE_ID_VGA_TNC_A0   0x4100
#endif
#ifndef PCI_DEVICE_ID_VGA_LNC
#define PCI_DEVICE_ID_VGA_LNC      0x4102
#endif


#define LVDS_MIN_DCLK 12000L    /* in KHz */
#define LVDS_MAX_DCLK 112000L   /* in KHz */
/* definition for 915GM */
#define LVDS_915GM_SINGLE_MIN_DCLK   20000L
#define LVDS_915GM_SINGLE_MAX_DCLK  112000L
#define LVDS_915GM_DUAL_MIN_DCLK     25000L
#define LVDS_915GM_DUAL_MAX_DCLK    224000L

/* definition for 945GM */
#define LVDS_945GM_SINGLE_MIN_DCLK   25000L
#define LVDS_945GM_SINGLE_MAX_DCLK  112000L
#define LVDS_945GM_DUAL_MIN_DCLK     25000L
#define LVDS_945GM_DUAL_MAX_DCLK    224000L

/* definition for GM965 */
#define LVDS_GM965_SINGLE_MIN_DCLK   25000L
#define LVDS_GM965_SINGLE_MAX_DCLK  112000L
#define LVDS_GM965_DUAL_MIN_DCLK     25000L
#define LVDS_GM965_DUAL_MAX_DCLK    224000L

/* definition for Atom E6xx */
#define LVDS_TNC_SINGLE_MIN_DCLK     19750L
#define LVDS_TNC_SINGLE_MAX_DCLK     79500L

#define LVDS_MIN(a, b)       ((a)<(b)?(a):(b))

/* Local attributes
 * BIT15 of attribute Id specifies that it is a local attribute */
#define LVDS_ATTR_ID_DATA_FORMAT   (BIT(15)|1)
#define LVDS_ATTR_ID_TC_LVDS_CLK   (BIT(15)|2)

/* Default values */
#define LVDS_DEF_PANEL_DEPTH    18      /* Default panel depth */
#define LVDS_DEF_DITHER          1      /* default=1 as default panel depth=18*/
#define LVDS_DEF_SCALE_ON        1      /* Default scaling is on */
#define LVDS_DEF_PANEL_FIT       1      /* Default panel fit in on */

typedef struct _lvds_context {
	unsigned short fp_width;
	unsigned short fp_height;
	unsigned short dual_channel;
	unsigned short panel_type;
	unsigned short panel_fit;
	unsigned short panel_depth;
	unsigned short dither;
	unsigned short aspect_ratio;
	unsigned long  text_tune;
	unsigned long  pwm_intensity;
	unsigned long  inverter_freq;
	unsigned long  blm_legacy_mode;

	unsigned long power_state;
	unsigned short chipset;
	unsigned char init_done;
	unsigned char num_attrs;
	unsigned long pipe;
	unsigned short gfx_freq;
	unsigned char gn4_plus;
	unsigned char pwm_done;
	unsigned char tc_110MHz_clk;

	pd_callback_t *callback;
	pd_timing_t   *timing_table;
	pd_attr_t     *attr_list;
	pd_timing_t   *native_dtd;
	pd_timing_t   *current_mode;
} lvds_context_t;

extern int PD_MODULE_INIT(lvds_init, (void *handle));
extern int PD_MODULE_EXIT(lvds_exit, (void));
extern unsigned long lvds_validate(unsigned long cookie);
extern int lvds_open(pd_callback_t *callback, void **context);
extern int lvds_init_device(void *context);
extern int lvds_close(void *context);
extern int lvds_set_mode(void *context, pd_timing_t *mode,
			unsigned long flags);
extern int lvds_post_set_mode(void *context, pd_timing_t *mode,
			unsigned long flags);
extern int lvds_set_attrs(void *context, unsigned long num, pd_attr_t *list);
extern int lvds_get_attrs(void *context, unsigned long*num, pd_attr_t **list);
extern int lvds_get_timing_list(void *context, pd_timing_t *in_list,
			pd_timing_t **list);
extern int lvds_set_power(void *context, unsigned long state);
extern int lvds_get_power(void *context, unsigned long *state);
extern int lvds_save(void *context, void **state, unsigned long flags);
extern int lvds_restore(void *context, void *state, unsigned long flags);
extern int lvds_get_port_status(void *context, pd_port_status_t *port_status);

#endif


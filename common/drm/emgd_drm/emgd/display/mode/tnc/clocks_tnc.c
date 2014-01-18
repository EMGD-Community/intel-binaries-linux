/*
 *-----------------------------------------------------------------------------
 * Filename: clocks_tnc.c
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
 *  Clock programming for Atom E6xx
 *  program clocks used for LVDS port based on Lincroft
 *  program clocks used for SDVO port based on Atom E6xx overlay
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <io.h>
#include <sched.h>

#include <igd_init.h>
#include <igd_mode.h>

#include <context.h>
#include <dsp.h>
#include <utils.h>
#include <mode.h>
#include <pi.h>
#include <intelpci.h>
#include "drm_emgd_private.h"

#include <tnc/regs.h>

#include <tnc/igd_tnc_wa.h> /* needed for vbios for register defines */

#ifdef CONFIG_DEBUG
#define FLAG(a) a
/* Debug flag to turn off CDVO reset sequence */
static int flag_enable_cdvo_reset = 1;
#else
/* Turn all workaround for release driver */
#define FLAG(a) 1
#endif

extern unsigned long get_port_type(int crtc_id);
static void wait_dpll(void);

int program_cdvo_tnc(void);
static int kms_program_cdvo(emgd_crtc_t *emgd_crtc);

static cdvo_regs_t cdvo_reset[] = {
           /*turn off the SDVO port*/
           {0x61140, 0},
           /*turn off the stall register*/
           {0x6102C, 0},
           /*disable SDVO*/
           {0x61170, 0},
           /*programmable cdvo stall*/
           {0x6102c, 0xf},
           {0x7000, 0x40},
           /*reset*/
           {0x7000, 0x51},
           {0x7000, 0x50},
           /*High*/
           {0x7014, 0x00004800},
           /*strobe data tuning default*/
           {0x700c, 0x000BB4FF},
           /*enable sdvo*/
           {0x61170, 0x20022160},
           {0x7010, 0x06000200},
           /* -- Begin Sequence of 3 pixel signal --*/
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*reset*/
           {0x7000, 0x51},
           {0x7000, 0x50},
           /*High*/
           {0x7014, 0x00004800},
           /*strobe data tuning default*/
           {0x700c, 0x000BB4FF},
           /*enable sdvo*/
           {0x61170, 0x20022160},
           {0x7010, 0x06000200},
           /* -- Begin Sequence of 3 pixel signal --*/
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Low*/
           {0x700c, 0x00008000},
           {0x700c, 0x0008B400},
           /*High*/
           {0x700c, 0x0008B4FF},
           /*Reset*/
           {0x7000, 0x51},
           {0x7000, 0x50},
           {0x7010, 0x02000200},    /*gtl gtl*/
           {0x7014, 0x00004000},    /*ODT*/
           {0x61170, 0x20022160},    /*enable SDVO*/
           {0x7010, 0x02000200},    /*gtl gtl*/
           {0x7014, 0x00000800},
           {0x7014, 0x00004800},    /*rcomp*/
           {0x7014, 0x00000000},
           {0x7000, 0x40},            /*enable cdvo*/
           {0x7000, 0x50},            /*enable cdvo*/
           {0x7014, 0x00004000},    /*ODT*/
           {0x61170, 0x20022160},    /*enable SDVO*/
           {0,0}
};




/*!
 * @addtogroup display_group
 * @{
 */

/* Enable this if calculated values are wrong and required fixed table */
#define CONFIG_FIXED_TABLE  0

#ifdef CONFIG_TNC

extern int program_clock_sdvo_tnc(igd_display_context_t *display,
	igd_clock_t *clock, unsigned long dclk);

/*===========================================================================
; File Global Data
;--------------------------------------------------------------------------*/
#if CONFIG_FIXED_TABLE
typedef struct _fixed_clock {
	unsigned long dclk;
	unsigned long n;
	unsigned long m;
	unsigned long p1;
} fixed_clock_t;

/* set mmio register required values */
static fixed_clock_t lvds_fixed_clock_table[] =  {
	/* Clock       N     M     P1 */
	{ 65000,      0x01, 0x00, 0x00 },
	{ 0xffffffff, 0x01, 0x00, 0x00}
};

/* set mmio register required values */
static fixed_clock_t sdvo_fixed_clock_table[] =  {
	/* Clock       N     M    P1 */
	{ 65000,      0x00, 0x00, 0x00 },
	{ 0xffffffff, 0x00, 0x00, 0x00}
};
#endif

#define LVDS_M_MIN 10

/* This table is also used by mode_tnc.c */
const unsigned long lvds_m_converts[] = {
	0x2B, 0x15, 0x2A, 0x35, 0x1A, 0x0D, 0x26, 0x33, 0x19, 0x2C,
	0x36, 0x3B, 0x1D, 0x2E, 0x37, 0x1B, 0x2D, 0x16, 0x0B, 0x25,
	0x12, 0x09, 0x24, 0x32, 0x39, 0x1c,
};

const unsigned long LVDS_M_CONVERTS_LEN=26;   /* Length of the array above*/

#define TARGET_ERROR 46

typedef const struct _tnc_limits {
	unsigned long ref_freq;
	unsigned long min_m;
	unsigned long max_m;

	unsigned long min_n;
	unsigned long max_n;

	unsigned long min_p1;
	unsigned long max_p1;

	unsigned long min_p2;
	unsigned long max_p2;

	unsigned long min_vco;
	unsigned long max_vco;
} tnc_limits_t;

/* m, n, p value limits:
 * source: http://moss.amr.ith.intel.com/sites/LCD/LNC/HAS/Secured
 * %20Documents/LNC%20HAS%20work%20area/Lincroft%20DPLL_1.2_ww31_08.docx
 *
 * Note:
 * VCO range for 100L is not given: so put a range big enough to go through
 * calculations for the for loop.
 */
static tnc_limits_t tnc_lvds_limits[] =
{
	/* reffreq  m      n     p1     p2          vco */
	{ 200000, 10, 17, 1, 1, 2, 8, 14, 14, 2000000, 3400000},  /* SKU 100 */
	{ 100000, 20, 34, 1, 1, 2, 8, 14, 14, 1000000, 3400000},  /* SKU 100L */
	{ 166000, 12, 20, 1, 1, 2, 7, 14, 14, 2000000, 3333333},  /* SKU 83 */
};

static tnc_limits_t tnc_sdvo_limits[] =
{
	/* reffreq  m      n     p1     p2          vco */
	{  96000, 80,137, 3, 7, 1, 2, 10, 10, 1400000, 2800000},  /* SDVO */
};

/*!
 *
 * @param dclk
 * @param ref_freq
 * @param limits
 * @param m
 * @param n
 * @param p
 * @param target_error
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int calculate_clock(unsigned long dclk,
	unsigned long ref_freq,
	tnc_limits_t *l,
	unsigned long *m,
	unsigned long *n,
	unsigned long *p1,
	unsigned long target_error,
	unsigned long *actual_dclk,
	unsigned long port_type,
	unsigned long pd_type)
{
	unsigned long  pdiv;
	unsigned long  target_vco, actual_freq;
	long freq_error, min_error;
	unsigned long dclk_10000;

	unsigned long current_m, current_n, current_p1;

	EMGD_TRACE_ENTER;

	min_error = 100000;

	*m = 0;
	*n = 0;
	*p1 = 0;

	/* dclk * 10000, so it does not have to be calculated within the
	 * loop */
	dclk_10000 = dclk * 10000;

	for(current_m = l->min_m; current_m <= l->max_m; current_m++) {
		for(current_n = l->min_n; current_n <= l->max_n; current_n++) {
			for(current_p1 = l->min_p1; current_p1 <= l->max_p1; current_p1++) {

				/* For both LVDS/SDVO ports min_p2 and max_p2 are same,
				 * so assign use either min/max p2 */
				pdiv = current_p1 * l->min_p2;
				target_vco = dclk * pdiv;

				if (target_vco > l->max_vco) {
					/* target_vco continues to increase, so start with
					 * next current_n */
					break;
				}

				if (target_vco >= l->min_vco) {
					/* Frequency calculations for SDVO and LVDS are different
					 * SDVO Dotclock_frequency = (Reference Frequency * (M+2)) / (N*(P1*P2))
					 * LVDS DotClk_Frequency = (ReferenceFrequency * M)/ (P1* P2)
					 * need to find a way to differentiate SDVO and LVDS
					 * Works for now but signal generated will be different */
					actual_freq = (ref_freq * current_m) / (current_n*pdiv);
					freq_error = 10000 - (dclk_10000 / actual_freq);

					if (freq_error < -min_error) {
						/* freq_error continues to decrease once this
						 * point is reached, so start with next
						 * current_n */
						break;
					}

					if (freq_error < 0) {
						freq_error = -freq_error;
					}
					if (freq_error < min_error)  {
						*n = current_n;
						*m = current_m;
						*p1 = current_p1;
						min_error = freq_error;
						*actual_dclk = actual_freq;
					}
				}
			}
		}
		if (min_error == 0) {
			break;
		}
	}

	if (pd_type == PD_DISPLAY_TVOUT) {
		EMGD_TRACE_EXIT;
		return 0;
	}
	/*
	 * No clock found that meets error requirement
	 */
	if (min_error > (long)target_error) {
		EMGD_TRACE_EXIT;
		return 1;
	}

	EMGD_DEBUG("dclk = (ref*m)/(n*p1*p2) => %lu = (%lu*%lu)/(%lu*%lu*%lu) = %lu",
		dclk, ref_freq, *m, *n, *p1, l->min_p2,
		(ref_freq*(*m))/((*n)*(*p1)*l->min_p2));
	EMGD_DEBUG("min_error:%ld", min_error);
	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param dclk
 * @param ref_freq
 * @param limits
 * @param m
 * @param n
 * @param p
 * @param target_error
 * @param port_type
 * @param pd_type
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int get_clock(unsigned long dclk,
	unsigned long ref_freq,
	tnc_limits_t *l,
	unsigned long *m,
	unsigned long *n,
	unsigned long *p1,
	unsigned long target_error,
	unsigned long port_type,
	unsigned long *actual_dclk,
	unsigned long pd_type)
{

#if CONFIG_FIXED_TABLE
	fixed_clock_t *fixed;
	EMGD_TRACE_ENTER;
    /* Enable this if calculated values are wrong and required fixed table */
	if (port_type == IGD_PORT_LVDS) {
		fixed = lvds_fixed_clock_table;
	} else {
		fixed = sdvo_fixed_clock_table;
	}

	/* First check for a fixed clock from the table. These are ones that
	 * can't be calculated correctly. */
	while (fixed->dclk != 0xffffffff) {
		if (fixed->dclk == dclk) {
			EMGD_DEBUG("Using Fixed Clock From table for clock %ld", dclk);
			*m = fixed->m;
			*n = fixed->n;
			*p1 = fixed->p1;
			EMGD_TRACE_EXIT;
			return 0;
		}
		fixed++;
	}
#endif

	EMGD_TRACE_ENTER;
	/* No fixed clock found so calculate one. */
	EMGD_DEBUG("Calculating dynamic clock for clock %ld", dclk);

	if (calculate_clock(dclk, ref_freq, l, m, n, p1, target_error, actual_dclk, port_type, pd_type)) {
		/* No usable clock.  Cannot use 640x480@60 as default, because
		 * there are several vcos and several reference clocks. */
		EMGD_ERROR("Could not calculate clock %ld, returning default.", dclk);
		EMGD_TRACE_EXIT;
		return 1;
	}

	if (pd_type == PD_DISPLAY_TVOUT) {
		*p1 = 2;
		*m = 100;
		*n = 5;
	}

	/* Translate returned values to m,n,p1 register values */
	/* No change required for *n value */
	if (port_type == IGD_PORT_SDVO) {
		*p1 = (1L << (*p1 - 1));
		*m -= 2;
		*n = (1L << (*n -1));
	} else {
		*p1 = (1L << (*p1 - 2));
		*m = lvds_m_converts[*m - LVDS_M_MIN];
	}

	EMGD_DEBUG("reg m=%lu n=%lu p1=%lu p2=%lu", *m, *n, *p1, l->min_p2);

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param emgd_crtc
 * @param clock
 * @param dclk
 *
 * @return 0 on success
 * @return 1 on failure
 */
int kms_program_clock_lvds_tnc(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock,
	unsigned long dclk)
{
	struct drm_device  *dev = NULL;
	igd_display_pipe_t *pipe = NULL;
	igd_context_t      *context = NULL;
	igd_display_port_t *port = NULL;
	struct drm_encoder *encoder = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	unsigned long pt;
	int ret;
	unsigned long m, n, p1;
	unsigned long control;
	unsigned long ref_freq;
	tnc_limits_t *l = NULL;
	unsigned long count;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	pt = get_port_type(emgd_crtc->crtc_id);

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			break;
		}
	}


	/* LNC ref_freq is determined by SKU, convert to KHz */
#if 0
	ref_freq = display->context->device_context.gfx_freq * 1000L;
#else
	ref_freq = context->device_context.core_freq;
	ref_freq = ref_freq * 1000;
#endif

#if 0
	/* Find m,n,p,vco limits */
	if (ref_freq == 200000) {
		l = &tnc_lvds_limits[0];
	} else if (ref_freq == 100000) {
		l = &tnc_lvds_limits[1];
	} else if (ref_freq == 166000) {
		l = &tnc_lvds_limits[2];
	}
#endif

	for(count=0; count<3; count++){
		if (tnc_lvds_limits[count].ref_freq == ref_freq){
			l = &tnc_lvds_limits[count];
			break;
		}
	}

/* PO Debug */
#if 0
	/* WRITE_PORT80(0xED); */

	if(ref_freq == 166000){
	WRITE_PORT80(0xEF);
	}

	if (!l){
		/* FATAL ERROR */
		DEAD_LOOP(0xDD);
	}
#endif

	/* Per UMG, there is no defined target_error for LVDS, it supposed to
	 * work for all expected dclks. */
#if 1
	if (port) {
		ret = get_clock(dclk, ref_freq, l, &m, &n, &p1, dclk /* target_error */,
			port->port_type, &clock->actual_dclk, port->pd_driver->type);
	} else {
		ret = 1;
	}
	if (ret) {
		EMGD_ERROR_EXIT("Clock %ld could not be programmed", dclk);
		/* DEAD_LOOP(0xFF); */
		return ret;
	}
#else
	/* Hard code the values for now */
	m = 0x2D;  // ITP uses 0x2E, should change
	p1 = 2;

#endif

#if 0
	/* If clocks are already running at required clocks, just return */
	if (PIPE(display)->dclk == dclk) {
		return 0;
	}
#endif

	/* Disable DPLL */
#if 0
	control = BIT28 | (1<<clock->p_shift); //Why are we shifting 1 to P1, GMA carry over???
#else
	control = BIT28;
#endif
	EMGD_WRITE32(control, context->device_context.virt_mmadr + clock->dpll_control);
	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Program M */
	EMGD_WRITE32((m<<8), context->device_context.virt_mmadr + clock->mnp);

	EMGD_WRITE32((m<<8), context->device_context.virt_mmadr + 0xF044); //BUGBUG

	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->mnp, (m<<8));

	/* Enable DPLL */
	control = (BIT31 | BIT28 | BIT27) | (p1<<clock->p_shift);

/* PO Debug..*/
#if 0
	/* set VCO select */
	if (ref_freq == 166000) {
		control |= BIT16;
	}
#endif


	EMGD_WRITE32(control, context->device_context.virt_mmadr + clock->dpll_control);
	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Wait 150us for the DPLL to stabilize */
	OS_SLEEP(150);
	pipe->dclk = dclk;

	EMGD_TRACE_EXIT;
	return 0;
}
/*!
 *
 * @param display
 * @param clock
 * @param dclk
 *
 * @return 0 on success
 * @return 1 on failure
 */
int program_clock_lvds_tnc(igd_display_context_t *display,
	igd_clock_t *clock,
	unsigned long dclk)
{
	int ret;
	unsigned long m, n, p1;
	unsigned long control;
	unsigned long ref_freq;
	igd_display_port_t *port;
	tnc_limits_t *l = NULL;
	unsigned long count;

	EMGD_TRACE_ENTER;

	port = PORT_OWNER(display);

	/* LNC ref_freq is determined by SKU, convert to KHz */
#if 0
	ref_freq = display->context->device_context.gfx_freq * 1000L;
#else
	ref_freq = display->context->device_context.core_freq;
	ref_freq = ref_freq * 1000;
#endif

#if 0
	/* Find m,n,p,vco limits */
	if (ref_freq == 200000) {
		l = &tnc_lvds_limits[0];
	} else if (ref_freq == 100000) {
		l = &tnc_lvds_limits[1];
	} else if (ref_freq == 166000) {
		l = &tnc_lvds_limits[2];
	}
#endif

	for(count=0; count<3; count++){
		if (tnc_lvds_limits[count].ref_freq == ref_freq){
			l = &tnc_lvds_limits[count];
			break;
		}
	}

/* PO Debug */
#if 0
	/* WRITE_PORT80(0xED); */

	if(ref_freq == 166000){
	WRITE_PORT80(0xEF);
	}

	if (!l){
		/* FATAL ERROR */
		DEAD_LOOP(0xDD);
	}
#endif

	/* Per UMG, there is no defined target_error for LVDS, it supposed to
	 * work for all expected dclks. */
#if 1
	ret = get_clock(dclk, ref_freq, l, &m, &n, &p1, dclk /* target_error */,
		port->port_type, &clock->actual_dclk, port->pd_driver->type);
	if (ret) {
		EMGD_ERROR_EXIT("Clock %ld could not be programmed", dclk);
		/* DEAD_LOOP(0xFF); */
		return ret;
	}
#else
	/* Hard code the values for now */
	m = 0x2D;  // ITP uses 0x2E, should change
	p1 = 2;

#endif

#if 0
	/* If clocks are already running at required clocks, just return */
	if (PIPE(display)->dclk == dclk) {
		return 0;
	}
#endif

	/* Disable DPLL */
#if 0
	control = BIT28 | (1<<clock->p_shift); //Why are we shifting 1 to P1, GMA carry over???
#else
	control = BIT28;
#endif
	WRITE_MMIO_REG(display, clock->dpll_control, control);
	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Program M */
	WRITE_MMIO_REG(display, clock->mnp, (m<<8));

	WRITE_MMIO_REG(display, 0xF044, (m<<8)); //BUGBUG

	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->mnp, (m<<8));

	/* Enable DPLL */
	control = (BIT31 | BIT28 | BIT27) | (p1<<clock->p_shift);

/* PO Debug..*/
#if 0
	/* set VCO select */
	if (ref_freq == 166000) {
		control |= BIT16;
	}
#endif


	WRITE_MMIO_REG(display, clock->dpll_control, control);
	EMGD_DEBUG("lvds: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Wait 150us for the DPLL to stabilize */
	OS_SLEEP(150);
	PIPE(display)->dclk = dclk;

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param emgd_crtc
 * @param clock
 * @param dclk
 *
 * @return 0 on success
 * @return 1 on failure
 */
int kms_program_clock_sdvo_tnc(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock,
	unsigned long dclk)
{
	struct drm_device  *dev          = NULL;
	igd_display_pipe_t *pipe         = NULL;
	igd_context_t      *context      = NULL;
	igd_display_port_t *port         = NULL;
	struct drm_encoder *encoder      = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;
	unsigned long pt;
	int ret;
	unsigned long m, n, p1;
	unsigned long control;
	unsigned long ref_freq;
	unsigned long port_mult, vga_mult;
	unsigned long target_error, actual_dclk;
	tnc_limits_t *l;

	EMGD_DEBUG("Enter program_clock");

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	pt = get_port_type(emgd_crtc->crtc_id);

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			break;
		}
	}
	if (!port) {
		EMGD_ERROR_EXIT("No port");
		return -1;
	}

	/* FIXME: No info available in EAS and waiting for info. */
	if (dclk > 100000) {          /* 100-200 MHz */
		port_mult = 1;
	} else if (dclk > 50000) {    /* 50-100 Mhz */
		port_mult = 2;
	} else {                      /* 25-50 Mhz */
		port_mult = 4;
	}

	dclk *= port_mult;

	l = &tnc_sdvo_limits[0];
	ref_freq = l->ref_freq;

	vga_mult = READ_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control) & 0x3;

	target_error = TARGET_ERROR;

	/* For external clock sources always use ref_clock == dclk */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ref_freq = dclk;
		/* When clock source sdvo device, allowed error is 0. */
		target_error = 0;
	}

	if (port) {
		ret = get_clock(dclk, ref_freq, l, &m, &n, &p1, target_error,
			IGD_PORT_SDVO, &actual_dclk, port->pd_driver->type);
	} else {
		ret = 1;
	}

	clock->actual_dclk = actual_dclk/port_mult;

	if (ret) {
		EMGD_ERROR("Clock %ld could not be programmed", dclk);
		return ret;
	}

	/* Disable DPLL, Write 2 into P for saftey */
	control = BIT28 | (2<<clock->p_shift) | vga_mult;

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Program N, M */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->mnp, (n<<16) | m);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->mnp, (n<<16)|m);

	/* Enable DPLL, Disable VGA mode and sitck in new P values */
	control = BIT31 | BIT30 | BIT28 | (p1<<clock->p_shift) | vga_mult;

	/* Set the clock source correctly based on PD settings */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		control |= port->clock_bits;
	}

	/* sDVO Multiplier bits[7:0] */
	if (port_mult == 2) {
		control |= (1 << 4);
	} else if (port_mult == 3) {
		control |= (2 << 4);
	} else if (port_mult == 4) {
		control |= (3 << 4);
	}

	/* Double buffered */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* We must wait for 150 us for the dpll clock to warm up */
	//OS_SLEEP(150);
	wait_dpll();

	kms_program_cdvo(emgd_crtc);

	return 0;
}
/*!
 *
 * @param display
 * @param clock
 * @param dclk
 *
 * @return 0 on success
 * @return 1 on failure
 */
int program_clock_sdvo_tnc(igd_display_context_t *display,
	igd_clock_t *clock,
	unsigned long dclk)
{
	int ret;
	unsigned long m, n, p1;
	unsigned long control;
	unsigned long ref_freq;
	unsigned long port_mult, vga_mult;
	unsigned long target_error, actual_dclk;
	igd_display_port_t *port;
	tnc_limits_t *l;

	EMGD_DEBUG("Enter program_clock");

	port = PORT_OWNER(display);

	/* FIXME: No info available in EAS and waiting for info. */
	if (dclk > 100000) {          /* 100-200 MHz */
		port_mult = 1;
	} else if (dclk > 50000) {    /* 50-100 Mhz */
		port_mult = 2;
	} else {                      /* 25-50 Mhz */
		port_mult = 4;
	}

	dclk *= port_mult;

	l = &tnc_sdvo_limits[0];
	ref_freq = l->ref_freq;

	vga_mult = READ_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control) & 0x3;

	target_error = TARGET_ERROR;

	/* For external clock sources always use ref_clock == dclk */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ref_freq = dclk;
		/* When clock source sdvo device, allowed error is 0. */
		target_error = 0;
	}

	ret = get_clock(dclk, ref_freq, l, &m, &n, &p1, target_error,
		IGD_PORT_SDVO, &actual_dclk, port->pd_driver->type);

	clock->actual_dclk = actual_dclk/port_mult;

	if (ret) {
		EMGD_ERROR("Clock %ld could not be programmed", dclk);
		return ret;
	}

	/* Disable DPLL, Write 2 into P for saftey */
	control = BIT28 | (2<<clock->p_shift) | vga_mult;

	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* Program N, M */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->mnp, (n<<16) | m);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->mnp, (n<<16)|m);

	/* Enable DPLL, Disable VGA mode and sitck in new P values */
	control = BIT31 | BIT30 | BIT28 | (p1<<clock->p_shift) | vga_mult;

	/* Set the clock source correctly based on PD settings */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		control |= port->clock_bits;
	}

	/* sDVO Multiplier bits[7:0] */
	if (port_mult == 2) {
		control |= (1 << 4);
	} else if (port_mult == 3) {
		control |= (2 << 4);
	} else if (port_mult == 4) {
		control |= (3 << 4);
	}

	/* Double buffered */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, clock->dpll_control, control);
	EMGD_DEBUG("sdvo: EMGD_WRITE32: 0x%lx = 0x%lx",
		clock->dpll_control, control);

	/* We must wait for 150 us for the dpll clock to warm up */
	//OS_SLEEP(150);
	wait_dpll();

	program_cdvo_tnc();

	return 0;
}

int kms_program_clock_tnc(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock,
	unsigned long dclk)
{
	EMGD_TRACE_ENTER;

	if (get_port_type(emgd_crtc->crtc_id) == IGD_PORT_LVDS) {
		EMGD_TRACE_EXIT;
		return kms_program_clock_lvds_tnc(emgd_crtc, clock, dclk);
	} else {
		EMGD_TRACE_EXIT;
		return kms_program_clock_sdvo_tnc(emgd_crtc, clock, dclk);
	}
}

int program_clock_tnc(igd_display_context_t *display,
	igd_clock_t *clock,
	unsigned long dclk)
{
	EMGD_TRACE_ENTER;

	if (PORT_TYPE(display) == IGD_PORT_LVDS) {
		EMGD_TRACE_EXIT;
		return program_clock_lvds_tnc(display, clock, dclk);
	} else {
		EMGD_TRACE_EXIT;
		return program_clock_sdvo_tnc(display, clock, dclk);
	}
}


/*!
 * Poll for DPLL register lock. This is only valid for DPLLB
 *
 * @return void
 */
static void wait_dpll(void)
{
#ifndef CONFIG_MICRO
	unsigned long temp;
	os_alarm_t timeout;

	EMGD_TRACE_ENTER;

	/* Wait for DPLL lock, about 50 msec (20Hz). */
	timeout = OS_SET_ALARM(50);
	do {
		OS_SCHEDULE();
		/* Check for Bit16, 1 means DPLL is locked (TNC EAS Ver 2.0) */
		temp = (READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x606C) & 0x10000) >> 16;

		/* Check for timeout */
	} while ((!temp) && (!OS_TEST_ALARM(timeout)));

	if (!temp) {
		EMGD_ERROR_EXIT("Timeout waiting for pipe enable/disable");
	}

	EMGD_TRACE_EXIT;
#else
	/* to save space in VBIOS, we use to old method of waiting for the DPLL
	* to warm up */
	OS_SLEEP(150);
#endif
	return;
}


/* This is the initialization code for B0 stepping */
static int kms_program_cdvo(emgd_crtc_t *emgd_crtc)
{
	int counter = 0;
	igd_display_pipe_t *pipe = NULL;

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;

    #ifndef CONFIG_MICRO
	/*
	 * CDVO reset has been done. Check that offset 0x7000 has the value 0x50
	 * and this would mean that reset has been done. We only need to do cdvo
	 * reset once per warm reset
	 */
	if((READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x7000) == 0x50) ||
		!FLAG(flag_enable_cdvo_reset)){
		EMGD_TRACE_EXIT;
		return TRUE;
	}

    #endif

	/* pipe_temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPE(display)->pipe_reg); */

	/* Disable pipe */
	WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, pipe->pipe_reg, 0);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, pipe->pipe_reg, 0);

	/* Sleep for a while to wait for the pipe to disable. There are no
	* status bits to check if pipe B has been enable */
	OS_SLEEP(25);

    #if 0
       /* Disable DPLL */
       vga_mult = READ_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control) & 0x3;
       control = BIT28 | (2<<PIPE(display)->clock_reg->p_shift) | vga_mult;

       WRITE_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control, control);
       WRITE_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control, control);
    #endif

	/* the checking is needed for VBIOS but not needed for driver */
	do{
		WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, cdvo_reset[counter].reg,
			cdvo_reset[counter].value);
			counter++;
	}while(cdvo_reset[counter].reg != 0);

	/* Enable sDVOB port */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, 0x61140,
	READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x61140) | 0xC0000018 );


	EMGD_TRACE_EXIT;

	return TRUE;
}

/* This is the initialization code for B0 stepping */
int program_cdvo_tnc(void)
{
	int 		  counter = 0;
	unsigned long pipe_temp;

	EMGD_TRACE_ENTER;

    #ifndef CONFIG_MICRO
	/*
	 * CDVO reset has been done. Check that offset 0x7000 has the value 0x50
	 * and this would mean that reset has been done. We only need to do cdvo
	 * reset once per warm reset
	 */
	if((READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x7000) == 0x50) ||
		!FLAG(flag_enable_cdvo_reset)){
		EMGD_TRACE_EXIT;
		return TRUE;
	}

    #endif

	pipe_temp = READ_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_CONF);

	/* Disable pipe */
	WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PIPEB_CONF, 0);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_CONF, 0);

	/* Sleep for a while to wait for the pipe to disable. There are no
	* status bits to check if pipe B has been enable */
	OS_SLEEP(25);

#if 0
       /* Disable DPLL */
       vga_mult = READ_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control) & 0x3;
       control = BIT28 | (2<<PIPE(display)->clock_reg->p_shift) | vga_mult;

       WRITE_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control, control);
       WRITE_MMIO_REG_TNC(IGD_PORT_SDVO,
           PIPE(display)->clock_reg->dpll_control, control);
#endif

	/* the checking is needed for VBIOS but not needed for driver */
	do{
		WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, cdvo_reset[counter].reg,
			cdvo_reset[counter].value);
			counter++;
	}while(cdvo_reset[counter].reg != 0);

	/* Enable sDVOB port */
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, 0x61140,
	READ_MMIO_REG_TNC(IGD_PORT_SDVO, 0x61140) | 0xC0000018 );

	/* Restore Pipe B configuration value */
	WRITE_MMIO_REG_TNC(IGD_PORT_LVDS, PIPEB_CONF, pipe_temp);
	WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, PIPEB_CONF, pipe_temp);

	EMGD_TRACE_EXIT;

	return TRUE;
}
#endif


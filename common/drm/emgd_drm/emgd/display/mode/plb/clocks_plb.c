/*
 *-----------------------------------------------------------------------------
 * Filename: clocks_plb.c
 * $Revision: 1.11 $
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

#define MODULE_NAME hal.mode

#include <io.h>

#include <igd_mode.h>
#include <igd_init.h>

#include <context.h>
#include <intelpci.h>
#include <dsp.h>
#include <utils.h>
#include <mode.h>
#include <pi.h>
#include <sched.h>

#include "drm_emgd_private.h"
#include <plb/regs.h>

/*!
 * @addtogroup display_group
 * @{
 */

/*===========================================================================
; File Global Data
;--------------------------------------------------------------------------*/
typedef struct _fixed_clock {
	unsigned long dclk;
	unsigned long n;
	unsigned long m1;
	unsigned long m2;
	unsigned long p;
}fixed_clock_t;

static fixed_clock_t fixed_clock_table[] =  {
	/* Clock    N    M1    M2  Post Div */
    { 43163, 0x03, 0x12, 0x06, 0x84},  /* 43.163 GTF for 640x480 @ 100Hz */
    { 81624, 0x03, 0x18, 0x04, 0x82},  /* 81.624MHz */
	{0xffffffff, 0x00, 0x00, 0x00, 0x00}
};

/*!
 * This function translates from the calculated M value to the M1, M2
 * register values.
 *
 * @param m
 * @param *m1
 * @param *m2
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int calculate_m1_m2(unsigned long m,
	unsigned long *m1,
	unsigned long *m2)
{
	unsigned long current_m1, current_m2;

	EMGD_DEBUG("Enter calculate_m1_m2 %d", 1);

	/* ori was in steps of 2*/
	for(current_m1 = (10+2); current_m1 <= (20+2); current_m1 += 1) {
		for(current_m2 = (5+2); current_m2 <= (9+2); current_m2++)  {
			if((current_m1 * 5 + current_m2) == m) {
				*m1 = current_m1 - 2;
				*m2 = current_m2 - 2;
				return 0;
			}

		}
	}

	EMGD_DEBUG("M1, M2 not found for M == %ld", m);
	return 1;
}

#define MAX_M 120 //216
#define MIN_M 70 //96

#define MAX_N 8 //16
#define MIN_N 3

#define MAX_P 80 //126
#define MIN_P 5 //2

#define LVDS_MAX_P 98
#define LVDS_MIN_P 7

#define FIX_P2_LO 5 //126
#define FIX_P2_HI 10 //2

#define MAX_P1 8 //126
#define MIN_P1 1 //2

#define REF_FREQ 96000 //48000
#define MAX_VCO 2800000 //1400000
#define MAX_FP 200000
/* For LVDS port */
#define LVDS_FIX_P2_LO 7
#define LVDS_FIX_P2_HI 14

/*!
 *
 * @param dclk
 * @param ref_freq
 * @param m1
 * @param m2
 * @param n
 * @param p
 * @param min_vco
 * @param target_error
 * @param port_type
 * @param dual_channel
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int calculate_clock(unsigned long dclk,
	unsigned long ref_freq,
	unsigned long *m1,
	unsigned long *m2,
	unsigned long *n,
	unsigned long *p,
	unsigned long min_vco,
	unsigned long target_error,
	unsigned long port_type,
	unsigned long dual_channel)

{
	unsigned long p1;
	unsigned long p2;
	/* Parameters */
    unsigned long freqmult_p2;

	/* Intermidiate variables */
	unsigned long  pdiv;
	unsigned long  target_vco, actual_freq, actual_vco;
	long freq_error, min_error;

	unsigned long current_m, current_n, current_p1;
	unsigned long best_m = 0;
	unsigned long best_n = 0;
	unsigned long best_p1 = 0;


	EMGD_DEBUG("Enter calculate_clock");

	min_error = 100000;

	if (dclk > MAX_FP) {
		freqmult_p2 = FIX_P2_LO;
	} else {
		freqmult_p2 = FIX_P2_HI;
	}

	/* For LVDS port */
	if(port_type==IGD_PORT_LVDS){
		/* Test if we are dual channel */
		if(dual_channel){
			freqmult_p2=LVDS_FIX_P2_LO;
		} else{
			freqmult_p2=LVDS_FIX_P2_HI;
		}

	}

	for(current_m = MIN_M; current_m <= MAX_M; current_m++) {
		for(current_n = MIN_N; current_n < MAX_N; current_n++) {
			for(current_p1 = MIN_P1; current_p1 <= MAX_P1; current_p1++) {

				pdiv = freqmult_p2 * current_p1;
				target_vco = dclk * pdiv;

				if ((target_vco <= MAX_VCO) && (target_vco >= min_vco)) {
					actual_freq = (ref_freq * current_m) /
						(current_n * pdiv);
					actual_vco = actual_freq * pdiv;
					freq_error = 10000 - (dclk * 10000 / actual_freq);

					if (freq_error < 0) {
						freq_error = -freq_error;
					}
					if (freq_error < min_error)  {
						best_n = current_n;
						best_m = current_m;
						best_p1 = current_p1;
						min_error = freq_error;
					}
					if (min_error == 0) {
						break;
					}
				}
			}
			if (min_error == 0) {
				break;
			}
		}
		if (min_error == 0) {
			break;
		}
	}
	/*
	 * No clock found that meets error requirement
	 */
	if (min_error > (long)target_error) {
		return 1;
	}


	/* Translate M,N,P to m1,m2,n,p register values */
	*n = best_n - 2;
	if(calculate_m1_m2(best_m, m1, m2)) {
		/* No M1, M2 match for M */
		return 1;
	}

	p1 = (1 << (best_p1-1));
	if((freqmult_p2-5)) {
		p2 = 0x0;
	}
	else {
		p2 = 0x1;
	}
	/* Set p2 for LVDS */
	if(port_type==IGD_PORT_LVDS){
		if(dual_channel){
			p2=0x1;
		}else{
			p2=0x0;
		}

	}
	*p = (  p1 | (p2<<8) );

	return 0;
}

#define MIN_VCO_PASS1 2000000 //1000000
#define MIN_VCO_PASS2 1400000 //930000

/*!
 *
 * @param dclk
 * @param ref_freq
 * @param m1
 * @param m2
 * @param n
 * @param p
 * @param target_error
 * @param port_type
 * @param dual_channel
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int get_clock(unsigned long dclk,
	unsigned long ref_freq,
	unsigned long *m1,
	unsigned long *m2,
	unsigned long *n,
	unsigned long *p,
	unsigned long target_error,
	unsigned long port_type,
	unsigned long dual_channel)
{
	fixed_clock_t *fixed = fixed_clock_table;

	EMGD_DEBUG("Enter get_clock");

	/*
	 * First check for a fixed clock from the table. These are ones that
	 * can't be calculated correctly.
	 */
	while(fixed->dclk != 0xffffffff) {
		if(fixed->dclk == dclk) {
			EMGD_DEBUG("Using Fixed Clock From table for clock %ld", dclk);
			*m1 = fixed->m1;
			*m2 = fixed->m2;
			*n = fixed->n;
			*p = fixed->p;
			return 0;
		}
		fixed++;
	}


	/*
	 * No fixed clock found so calculate one.
	 */
	EMGD_DEBUG("Calculating dynamic clock for clock %ld", dclk);

	/*
	 * First pass try to find a clock with min_vco at 1000000.
	 * If a clock doesn't come out then try 930000.
	 */
	if(calculate_clock(dclk, ref_freq, m1, m2, n, p, MIN_VCO_PASS1,
		   target_error,port_type,dual_channel)) {
		if(calculate_clock(dclk, ref_freq, m1, m2, n, p, MIN_VCO_PASS2,
			   target_error,port_type,dual_channel)) {
			/* No usable clock */
			EMGD_ERROR("Could not calculate clock %ld, returning default.",
				dclk);
			*m1 = 0x14;
			*m2 = 0xc;
			*n = 0x3;
			*p = 0x82;
			return 1;
		}
	}

	return 0;
}

#define TARGET_ERROR 46

/*!
 *
 * @param emgd_crtc
 * @param clock
 * @param dclk
 *
 * @return 0 on success
 * @return 1 on failure
 */
int kms_program_clock_plb(emgd_crtc_t *emgd_crtc,
	igd_clock_t *clock,
	unsigned long dclk)
{
	unsigned long m1, m2, n, p;
	unsigned long control;
	unsigned long ref_freq;
	int ret;
	unsigned long port_mult, vga_mult;
	unsigned long dual_channel = 0;
	unsigned long index;
	unsigned long pt;
	struct drm_device  *dev          = NULL;
	igd_display_pipe_t *pipe         = NULL;
	igd_context_t      *context      = NULL;
	igd_display_port_t *port         = NULL;
	struct drm_encoder *encoder      = NULL;
	emgd_encoder_t     *emgd_encoder = NULL;

	EMGD_DEBUG("Enter program_clock");

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (((struct drm_crtc *)(&emgd_crtc->base)) == encoder->crtc) {
			emgd_encoder = container_of(encoder, emgd_encoder_t, base);
			port = emgd_encoder->igd_port;
			pt = port->port_type;
			break;
		}
	}
	if (!port) {
		EMGD_ERROR_EXIT("No port");
		return -1;
	}

	if (dclk > 100000) {        /*	100-200 MHz	*/

		port_mult = 1;
	}
	else if (dclk > 50000) {    /*	50-100 Mhz	*/

		port_mult = 2;
	}
	else {                      /*	25-50 Mhz	*/

		port_mult = 4;
	}

	/*
	 * Clock Multiplier : sDVO ports on all plb chipsets
	 */
	if (pt == IGD_PORT_DIGITAL) {

		dclk *= port_mult;
	}

	vga_mult = EMGD_READ32(context->device_context.virt_mmadr + clock->dpll_control) & 0x3;

	/* For Int-LVDS need to find out if its dual channel and pass
	 * that info into caluculating for p2. Apperently halving
	 * of dot-clock is also required by Ch7017 when operating in
	 * dual channel
	 */
	if (pt == IGD_PORT_LVDS) {
		/* Find PD_ATTR_ID_2_CHANNEL_PANEL attr value*/
		pi_pd_find_attr_and_value(port,
				PD_ATTR_ID_2_CHANNEL_PANEL,
				0/*no PD_FLAG for 2_channel*/,
				NULL,
				&dual_channel);
	}

	/* For external clock sources always use ref_clock == dclk */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ref_freq = dclk;
	} else {
		ref_freq = 96000;
	}
	/* LVDS reference clock can be 96 or 100 MHz. However there
	 * are no mention in the specification to specify which register
	 * to select/set this.
	 */

	/* When the clock source is provided externally by the port driver,
	 * the allowed error range is 0. */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ret = get_clock(dclk, ref_freq, &m1, &m2, &n, &p, 0
				,pt,dual_channel);
	} else {
		ret = get_clock(dclk, ref_freq, &m1, &m2, &n, &p, TARGET_ERROR
				,pt,dual_channel);
	}

	if(ret) {
		EMGD_ERROR("Clock %ld could not be programmed", dclk);
		return ret;
	}

	/* Disable DPLL, Write an 0x80 into P for saftey */
	control = 0x10000000 | (0x80<<clock->p_shift) | BIT26 | vga_mult;
	EMGD_WRITE32(control, context->device_context.virt_mmadr + clock->dpll_control);

	/* Program N, M1,and M2 */
	EMGD_WRITE32((n<<16) | (m1<<8) | m2, context->device_context.virt_mmadr + clock->mnp);

	/* Enable DPLL, Disable VGAm Mode and sitck in new P values */
	if(pt == IGD_PORT_LVDS){
		/* If LVDS set the appropriate bits for mode select */
		control = (BIT31 | BIT28 | BIT27 )
			| (p<<clock->p_shift) | vga_mult;

		if(port->attr_list) {

			for(index = 0; index < port->attr_list->num_attrs; index++) {

				/* Set spread spectrum and pulse phase */
				if(port->attr_list->attr[index].id == PD_ATTR_ID_SSC) {

					/*
					 * Pulse Phase for Poulsbo only has valid values between
					 * 3 and 9
					 */
					if(port->attr_list->attr[index].value >= 3 &&
						port->attr_list->attr[index].value <= 9) {

						control |= BIT13 | BIT14;
						/*
						 * Set the Pulse Phase to the clock phase specified by
						 * the user
						 */
						control |= (port->attr_list->attr[index].value<<9);
					}
					break;
				}
			}
		}
	} else{
	/* else DAC/SDVO */
		control = (BIT31 | BIT28 | BIT26) | (p<<clock->p_shift) | vga_mult;
	}
	/*
	 * Poulsbo has high speed clock on always
	 */
	control |= BIT30;



	/* Set the clock source correctly based on PD settings */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		control |= port->clock_bits;
	} else {
		control |= port->clock_bits & ~0x00006000;
	}

	/* sDVO Multiplier bits[7:0] */
	if (pt == IGD_PORT_DIGITAL) {

		if (port_mult == 2) {

			control |= (1 << 4);

		} else if (port_mult == 4) {

			control |= (3 << 4);
		}
	}

	EMGD_WRITE32(control, context->device_context.virt_mmadr + clock->dpll_control);

	/* We must wait for 150 us for the dpll clock to warm up */
	OS_SLEEP(150);
	pipe->dclk = dclk;

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
int program_clock_plb(igd_display_context_t *display,
	igd_clock_t *clock,
	unsigned long dclk)
{
	unsigned long m1, m2, n, p;
	unsigned long control;
	unsigned long ref_freq;
	int ret;
	igd_display_port_t *port;
	unsigned long port_mult, vga_mult;
	unsigned long dual_channel = 0;
	unsigned long index;

	EMGD_DEBUG("Enter program_clock");

	port=PORT_OWNER(display);

	if (dclk > 100000) {        /*	100-200 MHz	*/

		port_mult = 1;
	}
	else if (dclk > 50000) {    /*	50-100 Mhz	*/

		port_mult = 2;
	}
	else {                      /*	25-50 Mhz	*/

		port_mult = 4;
	}

	/*
	 * Clock Multiplier : sDVO ports on all plb chipsets
	 */
	if (port->port_type == IGD_PORT_DIGITAL) {

		dclk *= port_mult;
	}

	vga_mult = READ_MMIO_REG(display, clock->dpll_control) & 0x3;

	/* For Int-LVDS need to find out if its dual channel and pass
	 * that info into caluculating for p2. Apperently halving
	 * of dot-clock is also required by Ch7017 when operating in
	 * dual channel
	 */
	if (port->port_type == IGD_PORT_LVDS) {
		/* Find PD_ATTR_ID_2_CHANNEL_PANEL attr value*/
		pi_pd_find_attr_and_value(PORT_OWNER(display),
				PD_ATTR_ID_2_CHANNEL_PANEL,
				0/*no PD_FLAG for 2_channel*/,
				NULL,
				&dual_channel);
	}

	/* For external clock sources always use ref_clock == dclk */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ref_freq = dclk;
	} else {
		ref_freq = 96000;

		/* If SSC is enabled, then the reference frequency needs to be
		 * 100000 instead of 96000.
		 */
		for(index = 0; index < port->attr_list->num_attrs; index++) {
			if(port->attr_list->attr[index].id == PD_ATTR_ID_SSC) {
				ref_freq = 100000;
				break;
			}
		}
	}
	/* LVDS reference clock can be 96 or 100 MHz. However there
	 * are no mention in the specification to specify which register
	 * to select/set this.
	 */

	/* When the clock source is provided externally by the port driver,
	 * the allowed error range is 0. */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		ret = get_clock(dclk, ref_freq, &m1, &m2, &n, &p, 0
				,port->port_type,dual_channel);
	} else {
		ret = get_clock(dclk, ref_freq, &m1, &m2, &n, &p, TARGET_ERROR
				,port->port_type,dual_channel);
	}

	if(ret) {
		EMGD_ERROR("Clock %ld could not be programmed", dclk);
		return ret;
	}

	/* Disable DPLL, Write an 0x80 into P for saftey */
	control = 0x10000000 | (0x80<<clock->p_shift) | BIT26 | vga_mult;
	WRITE_MMIO_REG(display, clock->dpll_control, control);

	/* Program N, M1,and M2 */
	WRITE_MMIO_REG(display, clock->mnp, (n<<16) | (m1<<8) | m2);

	/* Enable DPLL, Disable VGAm Mode and sitck in new P values */
	if(port->port_type==IGD_PORT_LVDS){
		/* If LVDS set the appropriate bits for mode select */
		control = (BIT31 | BIT28 | BIT27 )
			| (p<<clock->p_shift) | vga_mult;

		if(port->attr_list) {

			for(index = 0; index < port->attr_list->num_attrs; index++) {

				/* Set spread spectrum and pulse phase */
				if(port->attr_list->attr[index].id == PD_ATTR_ID_SSC) {

					/*
					 * Pulse Phase for Poulsbo only has valid values between
					 * 3 and 9
					 */
					if(port->attr_list->attr[index].value >= 3 &&
						port->attr_list->attr[index].value <= 9) {

						control |= BIT13 | BIT14;
						/*
						 * Set the Pulse Phase to the clock phase specified by
						 * the user
						 */
						control |= (port->attr_list->attr[index].value<<9);
					}
					break;
				}
			}
		}
	} else{
	/* else DAC/SDVO */
		control = (BIT31 | BIT28 | BIT26) | (p<<clock->p_shift) | vga_mult;
	}
	/*
	 * Poulsbo has high speed clock on always
	 */
	control |= BIT30;



	/* Set the clock source correctly based on PD settings */
	if(port->pd_flags & PD_FLAG_CLK_SOURCE) {
		control |= port->clock_bits;
	} else {
		control |= port->clock_bits & ~0x00006000;
	}

	/* sDVO Multiplier bits[7:0] */
	if (port->port_type == IGD_PORT_DIGITAL) {

		if (port_mult == 2) {

			control |= (1 << 4);

		} else if (port_mult == 4) {

			control |= (3 << 4);
		}
	}

	WRITE_MMIO_REG(display, clock->dpll_control, control);

	/* We must wait for 150 us for the dpll clock to warm up */
	OS_SLEEP(150);
	PIPE(display)->dclk = dclk;

	return 0;
}

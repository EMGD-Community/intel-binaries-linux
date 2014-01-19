/*
 *-----------------------------------------------------------------------------
 * Filename: reg_tnc.c
 * $Revision: 1.19 $
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
 *  This is the implementation file for the register module's main API's called
 *  by external devices.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.state

#include <io.h>
#include <memory.h>
#include <sched.h>
#include <pci.h>

#include <igd_init.h>
#include <igd_mode.h>
#include <igd_pwr.h>
#include <igd_errno.h>

#include <context.h>
#include <utils.h>
#include <dsp.h>
#include <mode.h>
#include <utils.h>
#include <vga.h>
#include <intelpci.h>

#include <tnc/regs.h>
#include <plb/context.h>

#include "../cmn/reg_dispatch.h"

/*!
 * @addtogroup state_group
 * @{
 */

#define PLANE_LATCH_COUNT  4

#define RING_BUFFER        0
#define MMIO_MISC          1

static reg_buffer_t *reg_alloc_tnc(igd_context_t *context,
	unsigned long flags, void *_platform_context);
static void reg_free_tnc(igd_context_t *context, reg_buffer_t *reg_buffer,
	void *_platform_context);
static int reg_save_tnc(igd_context_t *context, reg_buffer_t *reg_buffer,
	void *_platform_context);
static int reg_restore_tnc(igd_context_t *context, reg_buffer_t *reg_buffer,
	void *_platform_context);
static void reg_crtc_lut_get_tnc(igd_context_t *context,
    emgd_crtc_t *emgd_crtc);
static void reg_crtc_lut_set_tnc(igd_context_t *context,
    emgd_crtc_t *emgd_crtc);

/* GR registers being saved */
static unsigned char gr_regs_tnc[] = {
	0x00, 0x01, 0x02, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x08,
	0x10, 0x11,
	0x18,
	0xff
};
#define GR_REG_COUNT sizeof(gr_regs_tnc)

/* SR registers being saved */
static unsigned char sr_regs_tnc[] = {
	0x00, 0x01, 0x02, 0x03, 0x04,
	0x07,
	0xff
};
#define SR_REG_COUNT sizeof(sr_regs_tnc)

/* AR registers being saved */
static unsigned char ar_regs_tnc[] = {
	0x00, 0x01, 0x02, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13,
	0x14,
	0xff,
};
#define AR_REG_COUNT sizeof(ar_regs_tnc)

/* CR registers being saved */
static unsigned char cr_regs_tnc[] = {
	0x00, 0x01, 0x02, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13,
	0x14, 0x15, 0x16, 0x17, 0x18,
	0xff
};
#define CR_REG_COUNT sizeof(cr_regs_tnc)


/* MMIO states register to be saved */
static unsigned long mmio_regs_tnc[] = {
#if  MMIO_MISC
	/* Fence Registers */
	FENCE0, FENCE1, FENCE2, FENCE3, FENCE4, FENCE5, FENCE6, FENCE7,  // Fence
	FENCE8, FENCE9, FENCE10, FENCE11, FENCE12, FENCE13, FENCE14, FENCE15,

	/* GTT Control */
	PGTBL_CTL,
#endif

	/* Memory Arbitration */
	FW_BLC_SELF, MI_ARB_STATE, G_DEBUG,

	/* Program Clocks */
	VGA0_DIVISOR, VGA1_DIVISOR, VGA_PD,

	FPA0, FPA1, FPB0, FPB1, P2D_CG_DIS,

	DPLLACNTR, DPLLBCNTR,

	/* Enable Ports */
	/* SDVO Port Enable */
	SDVOBCNTR,

	/* Panel Power */
	LVDS_PNL_PWR_CTL, PP_ON_DELAYS, PP_OFF_DELAYS, PP_DIVISOR,

	/* Panel Fitting */
	PFIT_CONTROL, PFIT_PGM_RATIOS,

	/* Backlight Control */
	BLC_PWM_CTL2, BLC_PWM_CTL, BLM_HIST_CTL,

	/* LVDS Port Enable */
	LVDSCNTR,

	/* Program Pipe A */
	PIPEA_STAT, PIPEA_DISP_ARB_CTRL, HTOTAL_A, HBLANK_A, HSYNC_A, VTOTAL_A,
	VBLANK_A, VSYNC_A, PIPEASRC, BCLRPAT_A,

	/* Program Pipe B */
	PIPEB_STAT, HTOTAL_B, HBLANK_B, HSYNC_B, VTOTAL_B,
	VBLANK_B, VSYNC_B, PIPEBSRC, BCLRPAT_B,


	/* Enable Pipes */
	PIPEA_CONF, PIPEB_CONF,

	/* Plane A */
	DSPASTRIDE, DSPAKEYVAL, DSPAKEYMASK,

	/* Plane B */
	DSPBSTRIDE, DSPBKEYVAL, DSPBKEYMASK,

	/* Plane C */
	DSPCSTRIDE, DSPCPOS, DSPCSIZE, DSPCKEYMASK,
	DSPCKEYMINVAL, DSPCKEYMAXVAL, DCLRC0, DCLRC1, DPYC_GAMC5,
	DPYC_GAMC4, DPYC_GAMC3, DPYC_GAMC2, DPYC_GAMC1, DPYC_GAMC0,

	/* Enable Plane C */
	DSPCCNTR, DSPCLINOFF, DSPCSURF,

	/* Enable Plane B */
	DSPBCNTR, DSPBLINOFF, DSPBSURF,

	/* Enable Plane A */
	DSPACNTR, DSPALINOFF, DSPASURF,

	/* Enable VGA Plane */
	VGACNTRL,

	/* Software Flags A */
	SWF00, SWF01, SWF02, SWF03, SWF04, SWF05, SWF06, SWF07,
	SWF08, SWF09, SWF0A, SWF0B, SWF0C, SWF0D, SWF0E, SWF0F,

	/* Software Flags B */
	SWF10, SWF11, SWF12, SWF13, SWF14, SWF15, SWF16, SWF17,
	SWF18, SWF19, SWF1A, SWF1B, SWF1C, SWF1D, SWF1E, SWF1F,

	/* Cursor A */
	CURSOR_A_BASE, CURSOR_A_POS, CURSOR_A_PAL0, CURSOR_A_PAL1,
	CURSOR_A_PAL2, CURSOR_A_PAL3,

	/* Cursor B */
	CURSOR_B_BASE, CURSOR_B_POS, CURSOR_B_PAL0, CURSOR_B_PAL1,
	CURSOR_B_PAL2, CURSOR_B_PAL3,

	/* Interrupt and hardware status */
	HWS_PGA, HWSTAM, SCPD0, IER, IMR, EIR, EMR,

	0xffffffff
};
#define MMIO_REG_COUNT(x) (sizeof(mmio_regs_tnc)/4)

/* MMIO states register to be saved */
static unsigned long mmio_regs_sdvo_tnc[] = {

	/* Program Clocks */
	VGA0_DIVISOR,

	SDVO_BUFF_CTRL_REG,

	FPB0, P2D_CG_DIS,

	DPLLBCNTR,

	/* Enable Ports */
	/* SDVO Port Enable */
	SDVOBCNTR,

	/* Program Pipe B */
	PIPEB_STAT, HTOTAL_B, HBLANK_B, HSYNC_B, VTOTAL_B,
	VBLANK_B, VSYNC_B, PIPEBSRC, BCLRPAT_B, PROG_STALL,

	/* CDVO */
	/* CDVO_CTRL,*/
	CDVO_SLEW_RATE, CDVO_STRENGTH, CDVO_RCOMP_UPDATE,

	/* TNC_SDVORAM */
	DPLL_ANACORE_CTRL, DPLL_MISC, DPLL_MONITOR, DPLL_MONITOR2,
	DPLL_TEST_COMPARATOR,

	/* Enable Pipes */
	PIPEB_CONF,

	/* Interrupt and hardware status */
	IER, IMR,

	0xffffffff
};
#define MMIO_REG_SDVO_COUNT(x) (sizeof(mmio_regs_sdvo_tnc)/4)

typedef struct _reg_platform_context_tnc {
	unsigned long *mmio_regs;
	unsigned long *mmio_regs_sdvo;
	unsigned char *ar;
	unsigned char *cr;
	unsigned char *sr;
	unsigned char *gr;
	unsigned long pipes;
} reg_platform_context_tnc_t;

typedef struct _vga_state_tnc  {
	unsigned char fcr; // Feature Control register
	unsigned char msr; // Miscellaneous Output register
	unsigned char sr_index; // Sequencer index register
	unsigned char cr_index; // CRT Controller index register
	unsigned char ar_index; // Attribute Controller index register
	unsigned char gr_index; // Graphics Controller index register
	unsigned char ar[AR_REG_COUNT];  // Attribute Contr regs (AR00-AR14)
	unsigned char sr[SR_REG_COUNT];  // Sequencer registers (SR01-SR04)
	unsigned char cr[CR_REG_COUNT];  // CRT Controller regs (CR00-CR18)
	unsigned char gr[GR_REG_COUNT];  // Graphics Contr regs
	unsigned char plane_latches[PLANE_LATCH_COUNT];
} vga_state_tnc_t;

#define DAC_DATA_COUNT  256  /* 256 sets of (R, G, B) data */
typedef struct _dac_state_tnc {
	unsigned long palette_a[DAC_DATA_COUNT]; /* Pipe A palette data */
	unsigned long palette_b[DAC_DATA_COUNT]; /* Pipe B palette data */
	unsigned char mode;  // DAC state register
	unsigned char index; // DAC index register
	unsigned char mask;  // 0x3C6, Palette Pixel Mask Register
} dac_state_tnc_t;

/* Structure to store the 3D registers during power management
 * These are 3D specific registers, but named as D3D because
 * variabl names cannot start with a number*/
typedef struct _d3d_state_tnc {
	unsigned long cin_ctl;
	unsigned long bin_scene;
	unsigned long bmp_buffer;
	unsigned long bmp_get;
	unsigned long bmp_put;
} d3d_state_tnc_t;

typedef struct _reg_buffer_tnc {
	unsigned long *mmio_state;
	unsigned long *mmio_state_sdvo;
	//unsigned long mem_mode; /* Reg 0x20CC */
	//unsigned long instpm;   /* Reg 0x20C0 */
	unsigned long gtt[128*1024]; /* PLB is 128 */
	void *vga_mem;
	vga_state_tnc_t vga_state;
	dac_state_tnc_t dac_state;
	d3d_state_tnc_t d3d_state;
	void *rb_state;
	unsigned long pci_lbb;
} reg_buffer_tnc_t;

static reg_platform_context_tnc_t reg_platform_context_tnc = {
	mmio_regs_tnc,
	mmio_regs_sdvo_tnc,
	ar_regs_tnc,
	cr_regs_tnc,
	sr_regs_tnc,
	gr_regs_tnc,
	2
};

reg_dispatch_t reg_dispatch_tnc = {
	reg_alloc_tnc,
	reg_free_tnc,
	reg_save_tnc,
	reg_restore_tnc,
	reg_crtc_lut_get_tnc,
	reg_crtc_lut_set_tnc,
	&reg_platform_context_tnc
};

/******************************************************************************
 *  Local Functions
 *****************************************************************************/
static int reg_save_gtt_tnc(igd_context_t *context, unsigned char *mmio,
	reg_buffer_tnc_t *reg_args);
static int reg_restore_gtt_tnc(igd_context_t *context,
	reg_buffer_tnc_t *reg_args);

/*!
 * This procedure simply waits for the next vertical syncing (vertical retrace)
 * period. If the display is already in a vertical syncing period, this
 * procedure exits unlike "util_Wait_VSync_Start()" function that waits for
 * the beginning of the next vertical sync period.
 *
 * Note: A timeout is included to prevent an endless loop.
 *
 * @param context
 *
 * @return FALSE if timed out
 * @return TRUE otherwise
 */
int reg_wait_vsync_tnc(igd_context_t *context)
{
	unsigned long i = 0;
	unsigned char *mmio;

	mmio = context->device_context.virt_mmadr;

	if ((EMGD_READ32(mmio + VP00) & BIT31)) {
		return 1;
	}

	while ((i++ < 0x100000) &&  /* Check for timeout */
		((EMGD_READ8(mmio + STATUS_REG_01) & BIT3) == 0x00)) {
		;
	}

	if (i >= 0x100000) {
		return 0;
	}
	return 1;
}

/*!
 * Saves the current VGA register state of the video chipset into the
 * given state buffer.
 *
 * This function first saves the 4 plane latches, and then it saves
 * the SR, GR, AR, CR registers.
 *
 * @param context the current device context
 * @param vga_buffer this is where the VGA register state is saved
 * @param ar_regs AR registers to save
 * @param cr_regs CR registers to save
 * @param sr_regs SR registers to save
 * @param gr_regs GR registers to save
 *
 * @return 0
 */
static int reg_save_vga_tnc(
	igd_context_t *context,
	vga_state_tnc_t *vga_buffer,
	unsigned char *ar_regs,
	unsigned char *cr_regs,
	unsigned char *sr_regs,
	unsigned char *gr_regs)
{
	unsigned char *mmio;
	int i;

	mmio = context->device_context.virt_mmadr;

	/* First, save a few registers that will be modified to read the latches.
	 * We need to use GR04 to go through all the latches.  Therefore, we must
	 * first save GR04 before continuing.  However, GR04 will only behave the
	 * way we want it to if we are not in Odd/Even mode and not in Chain 4
	 * mode.  Therefore, we have to make sure GR05 (GFX mode reg),
	 * and SR04 (Mem Mode reg) are set properly.
	 * According to B-spec, we are not supposed to program GR06 via MMIO.
	 */
	READ_VGA(mmio, GR_PORT, GR04, vga_buffer->gr[0x04]);
	READ_VGA(mmio, SR_PORT, SR04, vga_buffer->sr[0x04]); /* Memory Mode */
	READ_VGA(mmio, GR_PORT, GR05, vga_buffer->gr[0x05]); /* GFX Mode Reg */
	READ_VGA(mmio, GR_PORT, GR06, vga_buffer->gr[0x06]); /* Misc Reg */

	WRITE_VGA(mmio, SR_PORT, SR04, 0x06);
	WRITE_VGA(mmio, GR_PORT, GR05, 0x00);
	WRITE_VGA(mmio, GR_PORT, GR06, 0x05);

	/* Save Memory Latch Data latches */
	for (i=0; i<PLANE_LATCH_COUNT; i++) {
		WRITE_VGA(mmio, GR_PORT, GR04, (unsigned char)i);
		READ_VGA(mmio, CR_PORT, CR22, vga_buffer->plane_latches[i]);
	}

	/* Restore the modified regs */
	WRITE_VGA(mmio, GR_PORT, GR06, vga_buffer->gr[0x06]);
	WRITE_VGA(mmio, GR_PORT, GR05, vga_buffer->gr[0x05]);
	WRITE_VGA(mmio, GR_PORT, GR04, vga_buffer->gr[0x04]);
	WRITE_VGA(mmio, SR_PORT, SR04, vga_buffer->sr[0x04]);


	/* Save Feature Controller register. */
	vga_buffer->fcr = EMGD_READ8(mmio + FEATURE_CONT_REG_READ);

	/* Save Miscellaneous Output Register. */
	vga_buffer->msr = EMGD_READ8(mmio + MSR_READ_PORT);

	/* Save index registers. */
	vga_buffer->sr_index = EMGD_READ8(mmio + SR_PORT);
	vga_buffer->cr_index = EMGD_READ8(mmio + CR_PORT);
	vga_buffer->gr_index = EMGD_READ8(mmio + GR_PORT);

#if 0
	/*
	 * Save the AR index register and last write status. Not sure that
	 * this is really necessary so skip it for now.
	 */
	READ_VGA(mmio, CR_PORT, CR24, isARData);  // Next write to AR index reg
	isARData &= 0x80;

	// Save AR index and last write status
	vga_buffer->ar_index = EMGD_READ8(mmio + AR_PORT) | isARData;
#endif

	/* Save CRT Controller registers. */
	for (i=0; cr_regs[i] != 0xff; i++) {
		READ_VGA(mmio, CR_PORT, cr_regs[i], vga_buffer->cr[i]);
	}

	/* Save Attribute Controller registers. */
	for (i=0; ar_regs[i] != 0xff; i++) {
		reg_wait_vsync_tnc(context);
		READ_AR(mmio, (unsigned char)i, vga_buffer->ar[i]);
	}

	/* Save Graphics Controller registers. */
	for (i=0; gr_regs[i] != 0xff; i++) {
		READ_VGA(mmio, GR_PORT, gr_regs[i], vga_buffer->gr[i]);
	}

	/* Save Sequencer registers. */
	for (i=0; sr_regs[i] != 0xff; i++) {
		READ_VGA(mmio, SR_PORT, sr_regs[i], vga_buffer->sr[i]);
	}

	return 0;
}

/*!
 * Restore the VGA Register state from the specifed state buffer.
 *
 * @param context the current device context
 * @param vga_buffer this is where the VGA register state is saved
 * @param ar_regs AR registers to save
 * @param cr_regs CR registers to save
 * @param sr_regs SR registers to save
 * @param gr_regs GR registers to save
 *
 * @return 0
 */
static int reg_restore_vga_tnc(
	igd_context_t *context,
	vga_state_tnc_t *vga_buffer,
	unsigned char *ar_regs,
	unsigned char *cr_regs,
	unsigned char *sr_regs,
	unsigned char *gr_regs)
{
	unsigned long i;
	unsigned char *mmio;
	unsigned long bit_mask;
	mmio = context->device_context.virt_mmadr;

	/* Restore the plane latches.
	 *
	 * BP: I don't understand what this block is doing and it doesn't
	 * seem necessary. Should check this against the spec and figure
	 * out what it really does.
	 */

	/* Memory Mode Register */
	WRITE_VGA(mmio, SR_PORT, SR04, 0x06);
	/* GR05, Graphics Mode Register */
	WRITE_VGA(mmio, GR_PORT, GR05, 0x00);
	/* GR06, Micsellaneous Register */
	WRITE_VGA(mmio, GR_PORT, GR06, 0x05);
	/* GR08, Bit Mask Register */
	WRITE_VGA(mmio, GR_PORT, GR08, 0xFF);

	for (i=0, bit_mask=1; i<PLANE_LATCH_COUNT; i++, bit_mask<<= 1)  {
		/* Set plane select register */
		WRITE_VGA(mmio, GR_PORT, GR04, i);
		/* Plane/Map mask register */
		WRITE_VGA(mmio, SR_PORT, SR02, bit_mask);
	}

	for (i=0, bit_mask=1; i<PLANE_LATCH_COUNT; i++, bit_mask<<= 1)  {
		/* Plane/Map mask register again?? */
		WRITE_VGA(mmio, SR_PORT, SR02, bit_mask);
	}

	/* Restore standard VGA registers.
	 * 2) Sequence registers
	 * 1) MSR register
	 * 3) CRTC registers
	 * 4) Graphics registers
	 * 5) Attribute registers
	 * 6) VGA Feature register
	 * 7) Index restisters
	 */


	WRITE_VGA(mmio, SR_PORT, SR00, 01); /* Do sync reset */

	for (i=0; sr_regs[i] != 0xff; i++) {
		 WRITE_VGA(mmio, SR_PORT, sr_regs[i], vga_buffer->sr[i]);
	}


	EMGD_WRITE8(vga_buffer->msr, mmio + MSR_PORT);
	WRITE_VGA(mmio, SR_PORT, SR00, 0x03); /* Set to normal operation */

	/* Unlock CRTC */
	WRITE_VGA(mmio, CR_PORT, CR11, vga_buffer->cr[0x11] & 0x7F);
	for (i=0; cr_regs[i] != 0xff; i++) {
		 WRITE_VGA(mmio, CR_PORT, cr_regs[i], vga_buffer->cr[i]);
	}

	for (i=0; gr_regs[i] != 0xff; i++) {
		 WRITE_VGA(mmio, GR_PORT, gr_regs[i], vga_buffer->gr[i]);
	}

	for (i=0; ar_regs[i] != 0xff; i++) {
		 WRITE_AR(mmio, ar_regs[i], vga_buffer->ar[i]);
	}

	EMGD_WRITE8(vga_buffer->fcr, mmio + FEATURE_CONT_REG);

	/* Restore index registers. Is this necessary?  */
	EMGD_WRITE8(vga_buffer->sr_index, mmio + SR_PORT);
	EMGD_WRITE8(vga_buffer->cr_index, mmio + CR_PORT);
	EMGD_WRITE8(vga_buffer->gr_index, mmio + GR_PORT);

	/* Lock CRTC */
	WRITE_VGA(mmio, CR_PORT, CR11, vga_buffer->cr[0x11] | 0x80);

	return 0;
}

/*!
 * Saves the DAC registers and lookup table.
 *
 * @param context
 * @param dac_state
 * @param platform_context
 *
 * @return 0
 */
static int reg_save_dac_tnc(
	igd_context_t *context,
	dac_state_tnc_t *dac_state,
	reg_platform_context_tnc_t *platform_context)
{
	unsigned char *mmio;
	int	i;

	mmio = context->device_context.virt_mmadr;

	/* Save DACMASK register */
	dac_state->mask = EMGD_READ8(mmio + DAC_PEL_MASK);

	/* Save DAC State register */
	dac_state->mode = EMGD_READ8(mmio + DAC_STATE);

	/* Save DAC Index register */
	dac_state->index = EMGD_READ8(mmio + DAC_WRITE_INDEX);

	/*
	 * Save DAC data
	 * Start from first DAC location
	 */
	EMGD_WRITE8(0, mmio + DAC_WRITE_INDEX);

	/* Save Pipe A Palette */
	/* Or if Pipe A is used for VGA */
	if (((EMGD_READ32(mmio + PIPEA_CONF) & PIPE_ENABLE) &&
		(EMGD_READ32(mmio + PIPEASRC))) ||
		((!(EMGD_READ32(mmio + 0x71400) & 0x80000000)) &&
		(!(EMGD_READ32(mmio + 0x71400) & 0x20000000)))) {
		for (i=0; i<DAC_DATA_COUNT; i++)  {
			dac_state->palette_a[i] = EMGD_READ32(mmio + i*4 + DPALETTE_A);
		}
	}

	if (platform_context->pipes == 2) {
		/* If Pipe B is enabled, save Pipe B Palette */
		/* Or if Pipe B is used for VGA */
		if (((EMGD_READ32(mmio + PIPEB_CONF) & PIPE_ENABLE) &&
			(EMGD_READ32(mmio + PIPEBSRC)))  ||
			((! (EMGD_READ32(mmio + 0x71400) & 0x80000000)) &&
			((EMGD_READ32(mmio + 0x71400) & 0x20000000)))) {

			for (i=0; i<DAC_DATA_COUNT; i++)  {
				dac_state->palette_b[i] = EMGD_READ32(mmio + i*4 + DPALETTE_B);
			}
		}
	}

	return 0;
}

static void reg_crtc_lut_set_tnc(
	igd_context_t *context,
	emgd_crtc_t *emgd_crtc)
{
	int i;
	unsigned long pal_reg;
	unsigned char *mmio;

	EMGD_TRACE_ENTER;

	mmio = context->device_context.virt_mmadr;

    /* Check if the pipe is enabled */
    if ((emgd_crtc->crtc_id == IGD_KMS_PIPEA &&
		((EMGD_READ32(mmio + PIPEA_CONF) & PIPE_ENABLE) &&
        (EMGD_READ32(mmio + PIPEASRC)))) ||
		(emgd_crtc->crtc_id == IGD_KMS_PIPEB &&
		((EMGD_READ32(mmio + PIPEB_CONF) & PIPE_ENABLE) &&
        (EMGD_READ32(mmio + PIPEBSRC))))) {

		pal_reg = (unsigned long)(mmio + DPALETTE_A +
			(emgd_crtc->crtc_id * (DPALETTE_B - DPALETTE_A)));

        for (i=0; i<DAC_DATA_COUNT; i++)  {
            EMGD_WRITE32((emgd_crtc->lut_r[i] << 16) |
				(emgd_crtc->lut_g[i] << 8) |
				emgd_crtc->lut_b[i],
				pal_reg + i*4);
        }
    }

	EMGD_TRACE_EXIT;
}

static void reg_crtc_lut_get_tnc(
	igd_context_t *context,
	emgd_crtc_t *emgd_crtc)
{
	int i;
	unsigned long pal_reg;
	unsigned char *mmio;
	unsigned long lut_value;

	EMGD_TRACE_ENTER;

	mmio = context->device_context.virt_mmadr;

    /* Check if the pipe is enabled, is this necessary? */
    if ((emgd_crtc->crtc_id == IGD_KMS_PIPEA &&
		((EMGD_READ32(mmio + PIPEA_CONF) & PIPE_ENABLE) &&
        (EMGD_READ32(mmio + PIPEASRC)))) ||
		(emgd_crtc->crtc_id == IGD_KMS_PIPEB &&
		((EMGD_READ32(mmio + PIPEB_CONF) & PIPE_ENABLE) &&
        (EMGD_READ32(mmio + PIPEBSRC))))) {

		pal_reg = (unsigned long)(mmio + DPALETTE_A +
			(emgd_crtc->crtc_id * (DPALETTE_B - DPALETTE_A)));

        for (i=0; i<DAC_DATA_COUNT; i++)  {
            lut_value = EMGD_READ32(pal_reg + i*4);
			emgd_crtc->lut_r[i] = (lut_value & 0xFF0000) >> 16;
			emgd_crtc->lut_g[i] = (lut_value & 0x00FF00) >> 8;
			emgd_crtc->lut_b[i] = (lut_value & 0x0000FF);
		}
    }

	EMGD_TRACE_EXIT;
}


/*!
 * Restore previously saved DAC palette from the specifed state buffer.
 *
 * @param context
 * @param dac_state
 * @param platform_context
 *
 * @return 0
 */
static int reg_restore_dac_tnc(
	igd_context_t *context,
	dac_state_tnc_t *dac_state,
	reg_platform_context_tnc_t *platform_context)
{
	int i;
	unsigned char *mmio;
	unsigned char temp;

	mmio = context->device_context.virt_mmadr;

	/* If Pipe A is enabled, restore Palette */
	/* Or if Pipe A is used for VGA */
	if (((EMGD_READ32(mmio + PIPEA_CONF) & PIPE_ENABLE) &&
		(EMGD_READ32(mmio + PIPEASRC))) ||
		((! (EMGD_READ32(mmio + 0x71400) & 0x80000000)) &&
		(! (EMGD_READ32(mmio + 0x71400) & 0x20000000)))) {

		/*
		 * Restore DAC data
		 * Start from first DAC location
		 */
		EMGD_WRITE8(0, mmio + DAC_WRITE_INDEX);

		/* Restore Pipe A Palette */
		for (i=0; i<DAC_DATA_COUNT; i++)  {
			EMGD_WRITE32(dac_state->palette_a[i], mmio + i*4 + DPALETTE_A);
		}
	}

	/* If this is a single pipe device. */
	if (platform_context->pipes == 2) {
		/* If Pipe B is enabled, restore Palette */
		/* Or if Pipe B is used for VGA */
		if (((EMGD_READ32(mmio + PIPEB_CONF) & PIPE_ENABLE) &&
			(EMGD_READ32(mmio + PIPEBSRC)))  ||
			((! (EMGD_READ32(mmio + 0x71400) & 0x80000000)) &&
			((EMGD_READ32(mmio + 0x71400) & 0x20000000)))) {

			for (i=0; i<DAC_DATA_COUNT; i++)  {
				EMGD_WRITE32(dac_state->palette_b[i], mmio + i*4 + DPALETTE_B);
			}
		}
	}

	/* Restore DACMASK register */
	EMGD_WRITE8(dac_state->mask, mmio + DAC_PEL_MASK);

	/* Restore DAC Index register */
	if (dac_state->mode & 1) {  /* Last write was to "write index register" */
		EMGD_WRITE8(dac_state->index, mmio + DAC_WRITE_INDEX);
	} else {  /* Last index write was to "read index register" */
		EMGD_WRITE8(dac_state->index - 1, mmio + DAC_READ_INDEX);

		/* Dummy read to set DACSTATE register and to increment read index to
		 * last saved.
		 */
		temp = EMGD_READ8(mmio + DAC_DATA_REG);
	}

	return 0;
}

/*!
 * Initializes memory to store register values
 *
 * @param context pointer to igd_context
 * @param flags indicate which register type to save
 * @param _platform_context
 *
 * @return pointer to structure which will eventually be saved data defining
 * 	register state
 * @return NULL on failure
 */
static reg_buffer_t *reg_alloc_tnc(igd_context_t *context,
	unsigned long flags,
	void *_platform_context)
{
	reg_buffer_tnc_t* reg_args;
	reg_buffer_t *reg_buffer;

	EMGD_DEBUG("Entry - reg_alloc");

	reg_buffer = (reg_buffer_t*)OS_ALLOC(sizeof(reg_buffer_t));
	if (reg_buffer == NULL) {
		return NULL;
	}

	reg_args = (reg_buffer_tnc_t*)OS_ALLOC(sizeof(reg_buffer_tnc_t));
	if (reg_args == NULL) {
		OS_FREE(reg_buffer);
		return NULL;
	}
	reg_buffer->mode_buffer = NULL;
	reg_buffer->platform_buffer = reg_args;

	OS_MEMSET(reg_args, 0, sizeof(reg_buffer_tnc_t));
	reg_buffer->flags = flags;

	reg_args->mmio_state = (void *)OS_ALLOC(sizeof(mmio_regs_tnc));
	reg_args->mmio_state_sdvo = (void *)OS_ALLOC(sizeof(mmio_regs_sdvo_tnc));

	if (!reg_args->mmio_state) {
		EMGD_DEBUG("Failed Allocating mmio memory");
		/* free the "parent" memory*/
		OS_FREE(reg_args);
		return NULL;
	}

	if (!reg_args->mmio_state_sdvo) {
		EMGD_DEBUG("Failed Allocating sdvo mmio memory");
		/*Free other dependaant memory*/
		OS_FREE(reg_args->mmio_state);
		OS_FREE(reg_args);
		return NULL;
	}

	return reg_buffer;
}

/*!
 * This function calculates the size of memory to store data
 *
 * @param context void pointer to main igd context
 * @param reg_buffer pointer to register structure returned from a reg_alloc
 * @param _platform_context
 *
 * @return 0
 */
static void reg_free_tnc(igd_context_t *context,
	reg_buffer_t *reg_buffer,
	void *_platform_context)
{
	reg_buffer_tnc_t* reg_args;

	EMGD_DEBUG("Entry - reg_free");

	if (reg_buffer) {
		reg_args = (reg_buffer_tnc_t*)reg_buffer->platform_buffer;

		if (reg_args) {
			if (NULL != reg_args->mmio_state) {
				OS_FREE(reg_args->mmio_state);
			}

			if (NULL != reg_args->mmio_state_sdvo) {
				OS_FREE(reg_args->mmio_state_sdvo);
			}

			if (NULL != reg_args->vga_mem) {
				OS_FREE(reg_args->vga_mem);
			}

			if (NULL != reg_args->rb_state) {
				OS_FREE(reg_args->rb_state);
			}
			OS_FREE(reg_args);
		}
		OS_FREE(reg_buffer);
	}

	return;
}

/*!
 * This functions save the regs
 *
 * @param context void pointer to main igd context
 * @param reg_buffer pointer to register structure returned from a reg_alloc
 * @param _platform_context
 *
 * @return 0 on success
 * @return -1 on failure
 */
static int reg_save_tnc(igd_context_t *context,
	reg_buffer_t *reg_buffer,
	void *_platform_context)
{
	reg_buffer_tnc_t           *reg_args;
	unsigned long              *buffer, *buffer_sdvo;
	reg_platform_context_tnc_t *reg_platform_context =
		(reg_platform_context_tnc_t *)_platform_context;
	int                        i;
	unsigned char              *mmio, *mmio_sdvo, *mmio_sdvo_st, *mmio_sdvo_st_gpio;
	platform_context_tnc_t *platform_context;
	platform_context = (platform_context_tnc_t *)context->platform_context;


	EMGD_TRACE_ENTER;

	if (reg_buffer == NULL) {
		return 0;
	}

	/* Before save need to idle the ring buffer */
	context->dispatch.idle((igd_driver_h)context);

	reg_args = (reg_buffer_tnc_t *)reg_buffer->platform_buffer;
	if (reg_args == NULL) {
		return 0;
	}

	mmio = context->device_context.virt_mmadr;
	mmio_sdvo = context->device_context.virt_mmadr_sdvo;
	mmio_sdvo_st = context->device_context.virt_mmadr_sdvo_st;
	mmio_sdvo_st_gpio = context->device_context.virt_mmadr_sdvo_st_gpio;


	/* Save vga registers */
	if (reg_buffer->flags & IGD_REG_SAVE_VGA) {
		EMGD_DEBUG("Saving VGA registers");
		reg_save_vga_tnc(context, &reg_args->vga_state,
			reg_platform_context->ar, reg_platform_context->cr, reg_platform_context->sr,
			reg_platform_context->gr);
	}

	/* Save VGA memory */
#if 0
	if (reg_buffer->flags & IGD_REG_SAVE_VGA_MEM) {
		EMGD_DEBUG("Saving VGA memory");
		if (!reg_args->vga_mem) {
			reg_args->vga_mem = (void *)OS_ALLOC_LARGE(64 * 1024);
			if (reg_args->vga_mem != NULL) {
				OS_MEMCPY(reg_args->vga_mem,
		            (void *)context->device_context.virt_fb_adr, 64*1024);
			} else {
				EMGD_ERROR("Unable to allocate memory for vga memory");
			}
		} else {
			OS_MEMCPY(reg_args->vga_mem,
	            (void *)context->device_context.virt_fb_adr, 64*1024);
		}
	}
#endif

	/* Save mmio registers */
	if (reg_buffer->flags & IGD_REG_SAVE_MMIO) {
		EMGD_DEBUG("Saving MMIO registers");
		buffer = reg_args->mmio_state;
		for (i=0; reg_platform_context->mmio_regs[i] != 0xffffffff; i++) {
			*buffer++ = EMGD_READ32(mmio + reg_platform_context->mmio_regs[i]);
		}

		buffer_sdvo = reg_args->mmio_state_sdvo;
		for (i=0; reg_platform_context->mmio_regs_sdvo[i] != 0xffffffff; i++) {
			*buffer_sdvo++ = EMGD_READ32(mmio_sdvo + reg_platform_context->mmio_regs_sdvo[i]);
		}

		if (reg_buffer->flags & IGD_REG_SAVE_GTT) {
			reg_save_gtt_tnc(context, mmio, reg_args);
		}
	}

	/* Save DAC registers */
	if (reg_buffer->flags & IGD_REG_SAVE_DAC) {
		EMGD_DEBUG("Saving DAC registers");
		reg_save_dac_tnc(context, &reg_args->dac_state, reg_platform_context);
	}

	/* Save Mode state */
	if(reg_buffer->flags & IGD_REG_SAVE_MODE) {
		EMGD_DEBUG("Saving mode state");
		/* At driver initialization though mode_save is requested, mode isn't
		 * initialized. So skip the mode_save if dispatch function isn't
		 * available. In this case, mode_save() will be done as part of
		 * mode_init(). */
		if (context->mod_dispatch.mode_save && !reg_buffer->mode_buffer) {
			context->mod_dispatch.mode_save(context,
				&reg_buffer->mode_buffer,
				&reg_buffer->flags);
		} else {
			EMGD_DEBUG("mode_save() is skipped as mode_init() isn't done.");
		}
	}


	/* Save the PCI Config Legacy Backlight Brightness Register */
	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev0,
				0xF4, &reg_args->pci_lbb)) {
		EMGD_DEBUG("Reading Legacy Backlight Brightness");
	}

	return 0;
} /* reg_save */

/*!
 * This function restores the regs states
 *
 * @param context void pointer to main igd context
 * @param reg_buffer pointer to register structure returned from a reg_alloc
 * @param _platform_context
 *
 * @return 0 on success
 * @return -1 on failure
 */
int reg_restore_tnc(igd_context_t *context,
	reg_buffer_t *reg_buffer,
	void *_platform_context)
{
	reg_buffer_tnc_t* reg_args;
	reg_platform_context_tnc_t *reg_platform_context =
		(reg_platform_context_tnc_t *)_platform_context;
	unsigned char *mmio, *mmio_sdvo, *mmio_sdvo_st, *mmio_sdvo_st_gpio;
	unsigned long *buffer, *buffer_sdvo;
	unsigned long lbb;
	int i;

	platform_context_tnc_t *platform_context;
	platform_context = (platform_context_tnc_t *)context->platform_context;

	EMGD_DEBUG("Entry - reg_restore");

	if (reg_buffer == NULL) {
		return 0;
	}

	reg_args = (reg_buffer_tnc_t *)reg_buffer->platform_buffer;
	if (reg_args == NULL) {
		return 0;
	}

	EMGD_DEBUG(" flags = 0x%lx", reg_buffer->flags);

	mmio = context->device_context.virt_mmadr;
	mmio_sdvo = context->device_context.virt_mmadr_sdvo;
	mmio_sdvo_st = context->device_context.virt_mmadr_sdvo_st;
	mmio_sdvo_st_gpio = context->device_context.virt_mmadr_sdvo_st_gpio;

	/* Restore Mode state */
	if (reg_buffer->flags & IGD_REG_SAVE_MODE) {
		if (reg_buffer->mode_buffer) {
			context->mod_dispatch.mode_restore(context,
				&reg_buffer->mode_buffer,
				&reg_buffer->flags);
		} else {
			EMGD_DEBUG("mode_restore() already happened in mode_shutdown().");
		}
	}

	/* Restore MMIO registers */
	if (reg_buffer->flags & IGD_REG_SAVE_MMIO) {
		EMGD_DEBUG("Restoring MMIO registers");
		buffer = reg_args->mmio_state;
		for (i=0; reg_platform_context->mmio_regs[i] != 0xffffffff; i++) {
			if (reg_platform_context->mmio_regs[i] == MI_ARB_STATE) {
				EMGD_DEBUG("Handle special masked register case");
				EMGD_WRITE32(0x04100000 | *buffer++, mmio +
					reg_platform_context->mmio_regs[i]);
			} else if (reg_platform_context->mmio_regs[i] == DPLLACNTR ||
				reg_platform_context->mmio_regs[i] == DPLLBCNTR) {

					EMGD_DEBUG("Handle special DPLL settling");
					EMGD_WRITE32(*buffer++,
						mmio + reg_platform_context->mmio_regs[i]);
					OS_SLEEP(150);
			} else {
				EMGD_WRITE32(*buffer++, mmio + reg_platform_context->mmio_regs[i]);
			}
		}
//		reg_restore_gtt_tnc(context, reg_args);

		buffer_sdvo = reg_args->mmio_state_sdvo;
		for (i=0; reg_platform_context->mmio_regs_sdvo[i] != 0xffffffff; i++) {
			if (reg_platform_context->mmio_regs_sdvo[i] == MI_ARB_STATE) {
				EMGD_DEBUG("Handle special masked register case");
				EMGD_WRITE32(0x04100000 | *buffer_sdvo++, mmio_sdvo +
					reg_platform_context->mmio_regs_sdvo[i]);
			} else if (reg_platform_context->mmio_regs_sdvo[i] == DPLLACNTR ||
				reg_platform_context->mmio_regs_sdvo[i] == DPLLBCNTR) {

					EMGD_DEBUG("Handle special DPLL settling");
					EMGD_WRITE32(*buffer_sdvo++,
						mmio_sdvo + reg_platform_context->mmio_regs_sdvo[i]);
					OS_SLEEP(150);
			} else {
				EMGD_WRITE32(*buffer_sdvo++, mmio_sdvo + reg_platform_context->mmio_regs_sdvo[i]);
			}
		}

		if (reg_buffer->flags & IGD_REG_SAVE_GTT) {
			reg_restore_gtt_tnc(context, reg_args);
		}
	}

#if 0
	/* Restore VGA memory */
	if (reg_buffer->flags & IGD_REG_SAVE_VGA_MEM) {
		if (reg_args->vga_mem) {
			OS_MEMCPY((void *)context->device_context.virt_fb_adr,
					reg_args->vga_mem, 64*1024);
		} else {
			EMGD_ERROR("Trying to restore NULL VGA memory");
		}
	}
#endif

	/* Restore DAC registers */
	if (reg_buffer->flags & IGD_REG_SAVE_DAC) {
		reg_restore_dac_tnc(context, &reg_args->dac_state, reg_platform_context);
	}

	/* Restore VGA registers */
	if (reg_buffer->flags & IGD_REG_SAVE_VGA) {
		reg_restore_vga_tnc(context, &reg_args->vga_state,
			reg_platform_context->ar, reg_platform_context->cr, reg_platform_context->sr,
			reg_platform_context->gr);
	}

	/* restore the Legacy Backlight Brightness register. This register
	 * will be 0 when returning from power management causing backlight to be
	 * off if the register is not saved and restored during power management
	 */
	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev0, 0xF4, &lbb)) {
		EMGD_DEBUG("Reading Legacy Backlight Brightness");

	}

	/* only of the LBB has no value, causing the backlight to be off, we restore
	 * the saved LBB value. This is just in case there are user applications
	 * or firmware setting this register before the driver restores */
	if(!(lbb & 0xFF)){
		if(OS_PCI_WRITE_CONFIG_32(platform_context->pcidev0,
										0xF4, (lbb | reg_args->pci_lbb))){
			EMGD_DEBUG("Writing into Legacy Backlight Brightness");
		}
	}

	return 0;
}

/*!
 * This function saves the GTT table entries into a buffer so that the GTT
 * can be restored later.
 *
 * @param context needs this to get the GTT table size and to get the
 * 	virtual address to the GTT table
 * @param mmio virtual address to MMIO
 * @param reg_args a pre-allocated context where GTT entries are saved
 *
 * @return 0 on success
 * @return -1 on failure
 */
static int reg_save_gtt_tnc(igd_context_t *context, unsigned char *mmio,
	reg_buffer_tnc_t *reg_args)
{
	unsigned int  i;

	/* Read the GTT entries from GTT ADR and save it in the array. */
	for (i = 0; i < (context->device_context.gatt_pages); i++) {
		reg_args->gtt[i] = EMGD_READ32(
			context->device_context.virt_gttadr + i);
	}

	return 0;
}

/*!
 *  This function restores the GTT table entries.
 *
 * @param context the context contains the GTT size and address
 * @param reg_args this has the saved GTT entries
 *
 * @return 0 on success
 * @return -1 on failure
 */
static int reg_restore_gtt_tnc(igd_context_t *context,
	reg_buffer_tnc_t *reg_args)
{
	unsigned int i;

	/* If the first element is 0, then nothing was saved */
	if (0 == reg_args->gtt[0]) {
		return 0;
	}

	/* Restore the GTT tablep */
	for (i=0; i < context->device_context.gatt_pages; i++) {
		EMGD_WRITE32(reg_args->gtt[i],
			context->device_context.virt_gttadr + i);
	}

	return 0;
}

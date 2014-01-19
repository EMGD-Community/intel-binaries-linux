/*
 *-----------------------------------------------------------------------------
 * Filename: vga_mode.c
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
 *  This file contains VGA plane/pipe programming functions that can
 *  be used on any VGA compatible hardware. Hardware specific functions
 *  should put the device in VGA compatible mode prior to calling thses
 *  functions.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.mode

#include <context.h>

#include <sched.h>

#include <igd_vga.h>

#include "drm_emgd_private.h"

#include <mode.h>
#include <utils.h>
#include <vga.h>

/*!
 * @addtogroup display_group
 * @{
 */

/*
 * For mono modes this is 0  (CRTC is at 0x3b4)
 * For other modes it is 0x20 (CRTC is at 0x3d4)
 */
unsigned char vga_port_offset = 0x20;

/* This is a global variable used by the vBIOS to override the mode table
 * if a User Defined Mode Table is set. */
vga_mode_data_t FAR (*vga_mode_data_ptr)[]=&vga_mode_data;

/* This is a global variable used by the vBIOS to enable/disable the
 * loading of the palette during a mode change.  A global variable is
 * the best choice, since this is not really an OAL interface.
 *   0=Do load the default palette on a mode change.
 *   1=Do NOT load the default palette on a mode change. */
unsigned char vga_disable_default_palette_load = 0;

/*!
 *
 * @param mmio
 * @param port
 * @param index
 * @param value
 *
 * @return void
 */
void write_vga_reg(unsigned char *mmio, unsigned short port,
	unsigned char index, unsigned char value)
{
	WRITE_VGA(mmio, port, index, value);
}

/*!
 *
 * @param display
 * @param timings
 *
 * @return void
 */
void kms_program_plane_vga(unsigned char *mmio,
	igd_timing_info_t *timings)
{

	EMGD_DEBUG("Enter program_plane_vga");

	/* Set Bit 5 so the plane remains off.  It will be turned on
	 * in the IAL.  This is necessary, so the clear screen can occur
	 * before the mode is enabled. */
	write_vga_reg(mmio, SR_PORT, 0x01,
		(*vga_mode_data_ptr)[timings->mode_number].sr_regs[0] | 0x20);
	OS_SLEEP(1000);

	return;
}

/*!
 *
 * @param display
 * @param timings
 *
 * @return void
 */
void program_plane_vga(igd_display_context_t *display,
	igd_timing_info_t *timings)
{
	unsigned char *mmio;

	EMGD_DEBUG("Enter program_plane_vga");

	mmio = (unsigned char *)MMIO(display);
	/* Set Bit 5 so the plane remains off.  It will be turned on
	 * in the IAL.  This is necessary, so the clear screen can occur
	 * before the mode is enabled. */
	write_vga_reg(mmio, SR_PORT, 0x01,
		(*vga_mode_data_ptr)[timings->mode_number].sr_regs[0] | 0x20);
	OS_SLEEP(1000);

	return;
}

unsigned char mono_colors[] = {0x00, 0x2a, 0x00, 0x3f};
unsigned char normal_colors[] = {0x00, 0x2a, 0x15, 0x3f};

unsigned char mono_color_bits[] = {8,16,8,16,8,16};
unsigned char p16_color_bits[] = {4,16,2,16,1,16};
unsigned char p64_color_bits[] = {4,32,2,16,1,8};

/*!
 *
 * @param mmio
 * @param n
 *
 * @return void
 */
void set_3f_palette(unsigned char *mmio, int n)
{
	int i;
	for(i=0; i<n*3; i++) {
		EMGD_WRITE8(0x3f, EMGD_MMIO(mmio) + 0x3c9);
	}
}

/*!
 *
 * @param mmio
 * @param n
 * @param bits
 * @param colors
 * @param palette_hack
 *
 * @return void
 */
void set_palette_vga(unsigned char *mmio, int n, unsigned char *bits,
	unsigned char *colors, unsigned char palette_hack)
{
	int i, j;
	unsigned char *current_bits;
	unsigned char color;

	for(i=0; i<n; i++) {
		current_bits = bits;
		for(j=0; j<3; j++) {
			color = colors[((i & current_bits[1])?2:0) |
				((i & current_bits[0])?1:0)];
			/* Nasty Hack for Special Case VGA palette */
			if(palette_hack && (color == 0x2a) && (i%8==6) && (j==1)) {
				color = 0x15;
			}
			EMGD_WRITE8( color, EMGD_MMIO(mmio) + 0x3c9);
			current_bits += 2;
		}
	}
}

/*!
 *
 * @param mmio
 *
 * @return void
 */
void set_256_palette(unsigned char *mmio)
{
	unsigned char top_of_range[] = {
		0x3f, 0x1c, 0x10
	};
	unsigned char bottom_of_range[] = {
		0x00, 0x1f, 0x2d,
		0x00, 0x0e, 0x14,
		0x00, 0x08, 0x0b
	};
	unsigned char mono_256_color[]={
		0x00, 0x05, 0x08,
		0x0B, 0x0E, 0x11,
		0x14, 0x18, 0x1C,
		0x20, 0x24, 0x28,
		0x2D, 0x32, 0x38,
		0x3F
	};
	unsigned char p256_color_bits[] = {4,8,2,8,1,8};
	unsigned char *bottom = bottom_of_range;
	unsigned char *top = top_of_range;
	unsigned char pattern_number[] = {4, 0, 0};
	unsigned char changing_pattern = 2;
	unsigned short adder;
	char diff = 1;
	int h,i,j,k,l;

	set_palette_vga(mmio, 16, p256_color_bits, normal_colors, 1);

	/* mono data */
	for(i=0; i<16; i++) {
		for (j=0; j<3; j++) {
			EMGD_WRITE8(mono_256_color[i], EMGD_MMIO(mmio) + 0x3c9);
		}
	}
	for(l=0; l<3; l++) {
		for(k=0; k<3; k++) {
			/* Adder is in 14.2 fixed point */
			adder =  *top - *bottom;
			for(j=0; j<6; j++) {
				for(i=0; i<4; i++) {
					for(h=2; h>=0; h--) {
						EMGD_WRITE8( *bottom + (pattern_number[h]*adder + 1)/4,
							EMGD_MMIO(mmio) + 0x3c9);
					}
					pattern_number[changing_pattern] += diff;
				}
				diff = -diff;
				changing_pattern++;
				changing_pattern %= 3;
			}
			bottom++;
		}
		top++;
	}
	set_3f_palette(mmio, 8);
}

static unsigned char next_ar;
static unsigned char next_cr;
static unsigned char next_gr;
static unsigned char *g_mmio;

/*!
 * This is a little trick to save some space. Write these registers
 * sequentially to save us a little bit of data space.
 *
 * @param value
 *
 * @return void
 */
void write_next_ar_reg(unsigned char value)
{
	WRITE_AR(g_mmio, next_ar, value);
	next_ar++;
}

/*!
 * This is a little trick to save some space. Write these registers
 * sequentially to save us a little bit of data space.
 *
 * @param value
 *
 * @return void
 */
void write_next_cr_reg(unsigned char value)
{
	write_vga_reg(g_mmio, CR_PORT, next_cr, value);
	next_cr++;
}

/*!
 * This is a little trick to save some space. Write these registers
 * sequentially to save us a little bit of data space.
 *
 * @param value
 *
 * @return void
 */
void write_next_gr_reg(unsigned char value)
{
	write_vga_reg(g_mmio, GR_PORT, next_gr, value);
	next_gr++;
}

/*!
 * This function programs the Timing registers and clock registers and
 * other control registers for PIPE.
 *
 * @param emgd_crtc
 * @param timings
 *
 * @return void
 */
void kms_program_pipe_vga(emgd_crtc_t *emgd_crtc,
	igd_timing_info_t *timings)
{
	struct drm_device  *dev = NULL;
	igd_context_t      *context = NULL;
	igd_display_pipe_t *pipe = NULL;
	unsigned char *mmio, i;
	unsigned char *colors=0;
	unsigned char palette_hack=0;
	unsigned char *color_bits = 0;
	int mode_index;
	unsigned char msr_temp;

	/* This is a mapping from the HAL mode number to the type of Palette
	 * being programmed for this mode. */
	char palette_type[] = {
	/* 9=Don't care.
	 *  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
		1, 1, 1, 1, 1, 1, 1, 2,	9, 9, 9, 9, 9, 1, 1, 9,
		9, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 4 };

	EMGD_TRACE_ENTER;

	pipe = emgd_crtc->igd_pipe;
	dev = ((struct drm_crtc *)(&emgd_crtc->base))->dev;
	context = ((drm_emgd_priv_t *)dev->dev_private)->context;
	mmio = (unsigned char *)context->device_context.virt_mmadr;
	g_mmio = mmio;
	mode_index = timings->mode_number;

	EMGD_DEBUG("IGD Mode#:0x%x", mode_index);

	/* Disable Group 0 Protection */
	write_vga_reg(mmio, CR_PORT, 0x11, 0x00);

	/*
	 * Note: for monochrome modes this will cause the IO port to change
	 * for the CRTC and the Status regs.
	 */
	 msr_temp = (*vga_mode_data_ptr)[mode_index].misc_reg;

#ifdef CONFIG_GN4
	/*
	 * According to Chrontel the VGA By-pass requires the HSYNC and VSYNC to be
	 * of positive polarity.  In the MSR - Miscellaneous Output register
	 * Bit 7 is CRT VSYNC polarity (0 = Positive, 1 = Negative)
	 * Bit 6 is CRT HSYNC polarity (0 = Positive, 1 = Negative)
	 */
	{
		pt = get_port_type(emgd_crtc->crtc_id);
		if (pt == IGD_PORT_DIGITAL) {
			msr_temp &= (~(BIT(7)|BIT(6)));
		}
	}
#endif
	EMGD_WRITE8(msr_temp, EMGD_MMIO(mmio) + 0x3c2);

	if((*vga_mode_data_ptr)[mode_index].misc_reg & 1) {
		vga_port_offset = 0x20;
	} else {
		vga_port_offset = 0;
	}

	/* Sequencer registers */
	/*
	 * Note: Most specs say 0 in SR00 scratch bits but in practice seems
	 * that everyone uses 3.
	 */
	write_vga_reg(mmio, SR_PORT, 0x00, 0x03);
	/* SR01 is on/off and done in program plane */
	for (i=2; i<=4; i++) {
		write_vga_reg(mmio, SR_PORT, i,
			/* The SR Regs in the table are from SR01-SR04, there is
			 * no SR00 in the table, so -1. */
			(*vga_mode_data_ptr)[mode_index].sr_regs[i-1]);
	}

	/* Graphics control registers 0x0-0x8,0x10 */
	next_gr = 0;
	for(i=0; i<=0x8; i++) {
		write_next_gr_reg((*vga_mode_data_ptr)[mode_index].gr_regs[i]);
	}
	/*
	 * GR10 is a non-standard register that controls the mapping of
	 * 0xa000 to MMIO or GTT memory.
	 */
	next_gr = 0x10;
	write_next_gr_reg(0x0);

	next_ar = 0;
	for(i=0; i<=0x13; i++) {
		write_next_ar_reg((*vga_mode_data_ptr)[mode_index].ar_regs[i]);
	}
	/* Spec says 0x8 for text modes, not done in practice */
	write_next_ar_reg(0x00);

	/* Ensure the Pixel Data Mask Register does not mask the pixel data */
	EMGD_WRITE8(0xFF, EMGD_MMIO(mmio) + 0x3c6);

	/* set DAC data value */
	EMGD_WRITE8(0, EMGD_MMIO(mmio) + 0x3c8);

	/* Load RAMDAC*/
	switch(palette_type[mode_index]){
	case 0:
		color_bits = p64_color_bits;
		colors = normal_colors;
		palette_hack = 0;
		break;
	case 1:
		color_bits = p16_color_bits;
		colors = normal_colors;
		palette_hack = 1;
		break;
	case 2:
		color_bits = mono_color_bits;
		colors = mono_colors;
		palette_hack = 0;
		break;
	default:
		break;
	}
	/* Program the Palette based on the mode. */
	if (!vga_disable_default_palette_load) {
		if (color_bits) {
			set_palette_vga(mmio, 64, color_bits, colors, palette_hack);
			set_3f_palette(mmio, 192);
		} else {
			set_256_palette(mmio);
		}
	}

	/* Timings */
	next_cr = 0;
	for(i=0; i<=0x18; i++) {
		write_next_cr_reg((*vga_mode_data_ptr)[mode_index].crtc_regs[i]);
	}

	return;
}


/*!
 * This function programs the Timing registers and clock registers and
 * other control registers for PIPE.
 *
 * @param display
 * @param timings
 *
 * @return void
 */
void program_pipe_vga(igd_display_context_t *display,
	igd_timing_info_t *timings)
{
	unsigned char *mmio, i;
	unsigned char *colors=0;
	unsigned char palette_hack=0;
	unsigned char *color_bits = 0;
	int mode_index;
	unsigned char msr_temp;

	/* This is a mapping from the HAL mode number to the type of Palette
	 * being programmed for this mode. */
	char palette_type[] = {
	/* 9=Don't care.
	 *  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
		1, 1, 1, 1, 1, 1, 1, 2,	9, 9, 9, 9, 9, 1, 1, 9,
		9, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 4 };

	EMGD_DEBUG("Enter program_pipe_vga");

	mmio = (unsigned char *)MMIO(display);
	g_mmio = mmio;
	mode_index = timings->mode_number;

	EMGD_DEBUG("IGD Mode#:0x%x", mode_index);

	/* Disable Group 0 Protection */
	write_vga_reg(mmio, CR_PORT, 0x11, 0x00);

	/*
	 * Note: for monochrome modes this will cause the IO port to change
	 * for the CRTC and the Status regs.
	 */
	 msr_temp = (*vga_mode_data_ptr)[mode_index].misc_reg;

#ifdef CONFIG_GN4
	/*
	 * According to Chrontel the VGA By-pass requires the HSYNC and VSYNC to be
	 * of positive polarity.  In the MSR - Miscellaneous Output register
	 * Bit 7 is CRT VSYNC polarity (0 = Positive, 1 = Negative)
	 * Bit 6 is CRT HSYNC polarity (0 = Positive, 1 = Negative)
	 */
	{
		igd_display_port_t *port;
		port = PORT_OWNER(display);
		if (port->port_type == IGD_PORT_DIGITAL) {
			msr_temp &= (~(BIT(7)|BIT(6)));
		}
	}
#endif
	EMGD_WRITE8(msr_temp, EMGD_MMIO(mmio) + 0x3c2);

	if((*vga_mode_data_ptr)[mode_index].misc_reg & 1) {
		vga_port_offset = 0x20;
	} else {
		vga_port_offset = 0;
	}

	/* Sequencer registers */
	/*
	 * Note: Most specs say 0 in SR00 scratch bits but in practice seems
	 * that everyone uses 3.
	 */
	write_vga_reg(mmio, SR_PORT, 0x00, 0x03);
	/* SR01 is on/off and done in program plane */
	for (i=2; i<=4; i++) {
		write_vga_reg(mmio, SR_PORT, i,
			/* The SR Regs in the table are from SR01-SR04, there is
			 * no SR00 in the table, so -1. */
			(*vga_mode_data_ptr)[mode_index].sr_regs[i-1]);
	}

	/* Graphics control registers 0x0-0x8,0x10 */
	next_gr = 0;
	for(i=0; i<=0x8; i++) {
		write_next_gr_reg((*vga_mode_data_ptr)[mode_index].gr_regs[i]);
	}
	/*
	 * GR10 is a non-standard register that controls the mapping of
	 * 0xa000 to MMIO or GTT memory.
	 */
	next_gr = 0x10;
	write_next_gr_reg(0x0);

	next_ar = 0;
	for(i=0; i<=0x13; i++) {
		write_next_ar_reg((*vga_mode_data_ptr)[mode_index].ar_regs[i]);
	}
	/* Spec says 0x8 for text modes, not done in practice */
	write_next_ar_reg(0x00);

	/* Ensure the Pixel Data Mask Register does not mask the pixel data */
	EMGD_WRITE8(0xFF, EMGD_MMIO(mmio) + 0x3c6);

	/* set DAC data value */
	EMGD_WRITE8(0, EMGD_MMIO(mmio) + 0x3c8);

	/* Load RAMDAC*/
	switch(palette_type[mode_index]){
	case 0:
		color_bits = p64_color_bits;
		colors = normal_colors;
		palette_hack = 0;
		break;
	case 1:
		color_bits = p16_color_bits;
		colors = normal_colors;
		palette_hack = 1;
		break;
	case 2:
		color_bits = mono_color_bits;
		colors = mono_colors;
		palette_hack = 0;
		break;
	default:
		break;
	}
	/* Program the Palette based on the mode. */
	if (!vga_disable_default_palette_load) {
		if (color_bits) {
			set_palette_vga(mmio, 64, color_bits, colors, palette_hack);
			set_3f_palette(mmio, 192);
		} else {
			set_256_palette(mmio);
		}
	}

	/* Timings */
	next_cr = 0;
	for(i=0; i<=0x18; i++) {
		write_next_cr_reg((*vga_mode_data_ptr)[mode_index].crtc_regs[i]);
	}

	return;
}

/*
 * The HAL mode numbers match up to VGA modes in this way:
 *   VGA Mode : HAL Mode
 *      00         00
 *      01         01
 *      02         02
 *      03         03
 *      04         04
 *      05         05
 *      06         06
 *      07         07
 *      0d         0d
 *      0e         0e
 *      0f         11
 *      10         12
 *      00*        13
 *      01*        14
 *      02*        15
 *      03*        16
 *      00+        17
 *      01+        17
 *      02+        18
 *      03+        18
 *      07+        19
 *      11         1A
 *      12         1B
 *      13         1C
 */

/* This can not be a static, since the vBIOS will use this as a global */
vga_mode_data_t VB_CODE_SEG vga_mode_data[] = {
	/*============================================================================*/
	/* Modes 00h (Text 40 x 25 Colors 16 gray Font 8 x 8) - Table Entry 00h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x0800,			/* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,
		 0x00,0xC7,0x06,0x07,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 01h (Text 40 x 25 Colors 16 gray Font 8 x 8) - Table Entry 01h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x0800,			/* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,
		 0x00,0xC7,0x06,0x07,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 02h (Text 80 x 25 Colors 16 gray Font 8 x 8) - Table Entry 02h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x08,	/* Text Columns Rows Font */
		0x1000,			/* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0xC7,0x06,0x07,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 03h (Text 80 x 25 Colors 16 Font 8 x 8) - Table Entry 03h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x08,	/* Text Columns Rows Font */
		0x1000,			/* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0xC7,0x06,0x07,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 04h (Graphics 320 x 200 Colors 4 Font 8 x 8) - Table Entry 04h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x4000,	        /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0x80,0xBF,0x1F,
		 0x00,0xC1,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0xA2,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x13,0x15,0x17,0x02,0x04,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x01,0x00,0x03,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x30,0x0F,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 05h (Graphics 320 x 200 Colors 4 gray Font 8 x 8) - Table 05h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x4000,	        /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0x80,0xBF,0x1F,
		 0x00,0xC1,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0xA2,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x13,0x15,0x17,0x02,0x04,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x01,0x00,0x03,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x30,0x0F,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 06h (Graphics 640 x 200 Colors 2 Font 8 x 8) - Table Entry 06h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x08,	/* Text Columns Rows Font */
		0x4000,	        /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x01,0x00,0x06},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
		 0x00,0xC1,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xC2,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x17,0x17,0x17,0x17,0x17,0x17,0x17,
		 0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,
		 0x01,0x00,0x01,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 07h (Text 80 x 25 Colors 2 Font 9 x 14) - Table Entry 07h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x1000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x00,0x03,0x00,0x03},

		/* Miscellaneous output register value. */
		0xA6,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x4D,0x0B,0x0C,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x28,0x0D,0x63,0xBA,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
		 0x10,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
		 0x0E,0x00,0x0F,0x08},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0A,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 08h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x10,	/* Text Columns Rows Font */
		0x7D00,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x21,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 09h */
	/*----------------------------------------------------------------------------*/
	{
		0x00,0x00,0x00,	/* Text Columns Rows Font */
		0x0000,         /* Page size */

		/* Sequencer register values. */
		{0x00,0x00,0x00,0x00},

		/* Miscellaneous output register value. */
		0x00,

		/* CRT Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00},

		/* Attribute Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 0Ah */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x4000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x00,0x00,0x00,0x03},

		/* Miscellaneous output register value. */
		0x23,

		/* CRT Controller register values. */
		{0x37,0x27,0x2D,0x37,0x31,0x15,0x04,0x11,
		 0x00,0x47,0x06,0x07,0x00,0x00,0x00,0x00,
		 0xE1,0x24,0xC7,0x14,0x08,0xE0,0xF0,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 0Bh */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x00,0x00,	/* Text Columns Rows Font */
		0x0000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x29,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0x62,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x0F,0x00,0x00,0x08,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 0Ch */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x00,0x00,	/* Text Columns Rows Font */
		0x0000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x29,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x0F,0x00,0x00,0x08,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 0Dh (Graphics 320 x 200 Colors 16 Font 8 x 8) - Table Entry 0Dh */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08,	/* Text Columns Rows Font */
		0x2000,         /* Page size */

		/* Sequencer register values. */
		{0x09,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0x80,0xBF,0x1F,
		 0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 0Eh (Graphics 640 x 200 Colors 16 Font 8 x 8) - Table Entry 0Eh */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x08,	/* Text Columns Rows Font */
		0x4000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
		 0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 0Fh */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x8000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x05,0x0F,0x00,0x00},

		/* Miscellaneous output register value. */
		0xA2,

		/* CRT Controller register values. */
		{0x60,0x4F,0x56,0x1A,0x50,0xE0,0x70,0x1F,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x5E,0x2E,0x5D,0x14,0x00,0x5E,0x6E,0x8B,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x08,0x00,0x00,0x18,0x18,0x00,0x00,
		 0x00,0x08,0x00,0x00,0x00,0x18,0x00,0x00,
		 0x0B,0x00,0x05,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x07,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Blank Entry - Table Entry 10h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x8000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x05,0x0F,0x00,0x00},

		/* Miscellaneous output register value. */
		0xA7,

		/* CRT Controller register values. */
		{0x5B,0x4F,0x53,0x17,0x50,0xBA,0x6C,0x1F,
		 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x5E,0x2B,0x5D,0x14,0x0F,0x5F,0x0A,0x8B,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x00,0x00,0x04,0x07,0x00,0x00,
		 0x00,0x01,0x00,0x00,0x04,0x07,0x00,0x00,
		 0x01,0x00,0x05,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x07,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 0Fh; (Graphics 640 x 350 Colors 2 Font 8 x 14) - Table Entry 11h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x8000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0xA2,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x28,0x0F,0x63,0xBA,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x08,0x00,0x00,0x18,0x18,0x00,0x00,
		 0x00,0x08,0x00,0x00,0x00,0x18,0x00,0x00,
		 0x0B,0x00,0x05,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 10h; (Graphics 640 x 350 Colors 16 Font 8 x 14) - Table Entry 12h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x8000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0xA3,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x28,0x0F,0x63,0xBA,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 00h; (Text 40 x 25 Colors 16 gray Font 8 x 14) - Table Entry 13h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x0E,	/* Text Columns Rows Font */
		0x0800,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0xA3,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,
		 0x00,0x4D,0x0B,0x0C,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x14,0x1F,0x63,0xBA,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 01h; (Text 40 x 25 Colors 16 Font 8 x 14) - Table Entry 14h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x0E,	/* Text Columns Rows Font */
		0x0800,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x09,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0xA3,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,
		 0x00,0x4D,0x0B,0x0C,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x14,0x1F,0x63,0xBA,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
// ; Modes 02h; (Text 80 x 25 Colors 16 gray Font 8 x 14) - Table Entry 15h //
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E,	/* Text Columns Rows Font */
		0x1000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0xA3,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x4D,0x0B,0x0C,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x28,0x1F,0x63,0xBA,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 03h; (Text 80 x 25 Colors 16 Font 8 x 14) - Table Entry 16h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x0E, /* Text Columns Rows Font */
		0x1000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0xA3,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x4D,0x0B,0x0C,0x00,0x00,0x00,0x00,
		 0x83,0x85,0x5D,0x28,0x1F,0x63,0xBA,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x08,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 00h+ ; 01h+ (Text 40 x 25 Colors 16 Font 9 x 16) - Table 17h */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x10, /* Text Columns Rows Font */
		0x0800,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x08,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x67,

		/* CRT Controller register values. */
		{0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,
		 0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x14,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x0C,0x00,0x0F,0x08},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 02h+ ; 03h+ (Text 80 x 25 Colors 16 Font 9 x 16) - Table 18h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x10,	/* Text Columns Rows Font */
		0x1000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x00,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x67,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x0C,0x00,0x0F,0x08},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 07h+ (Text 80 x 25 Colors 2 Font 9 x 16) - Table Entry 19h */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x18,0x10,	/* Text Columns Rows Font */
		0x1000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x00,0x03,0x00,0x02},

		/* Miscellaneous output register value. */
		0x66,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
		 0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x0F,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
		 0x10,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
		 0x0E,0x00,0x0F,0x08},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x10,0x0A,0x00,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 11h (Graphics 640 x 480 Colors 2 Font 8 x 16) - Table Entry 1Ah */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x1D,0x10,	/* Text Columns Rows Font */
		0xA000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0xE3,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0xEA,0x8C,0xDF,0x28,0x00,0xE7,0x04,0xC3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
		 0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
		 0x01,0x00,0x01,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x01,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 12h (Graphics 640 x 480 Colors 16 Font 8 x 16) - Table Entry 1Bh */
	/*----------------------------------------------------------------------------*/
	{
		0x50,0x1D,0x10,	/* Text Columns Rows Font */
		0xA000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x06},

		/* Miscellaneous output register value. */
		0xE3,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,
		 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
		 0xEA,0x8C,0xDF,0x28,0x00,0xE7,0x04,0xE3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,
		 0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		 0x01,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,
		 0xFF},
	},

	/*============================================================================*/
	/* Modes 13h (Graphics 320 x 200 Colors 256 Font 8 x 8) - Table Entry 1Ch */
	/*----------------------------------------------------------------------------*/
	{
		0x28,0x18,0x08, /* Text Columns Rows Font */
		0x2000,         /* Page size */

		/* Sequencer register values, SR01-SR04. */
		{0x01,0x0F,0x00,0x0E},

		/* Miscellaneous output register value. */
		0x63,

		/* CRT Controller register values. */
		{0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
		 0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
		 0x9C,0x8E,0x8F,0x28,0x40,0x96,0xB9,0xA3,
		 0xFF},

		/* Attribute Controller register values. */
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
		 0x41,0x00,0x0F,0x00},

		/* Graphics Controller register values. */
		{0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,
		 0xFF}
	},
};


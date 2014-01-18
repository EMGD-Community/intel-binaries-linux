/*
 *-----------------------------------------------------------------------------
 * Filename: i2c_bitbash_tnc.c
 * $Revision: 1.10 $
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
 *  Bitbash to read GPIO pins are done on LPC device 0:31:0. So do not
 *  use EMGD_READ32 and EMGD_WRITE32 macros for read/write to device 31. These
 *  macros do mmio only on device2.
 *  To properly read/write mmio on device 31, use
 *  READ_MMIO_REG_TNC(port_type, reg) and
 *  WRITE_MMIO_REG_TNC(port_type, reg, data).
 *  These macros properly set to work for all OSes including VBIOS, but
 *  generate more code compared to EMGD_READ32 and EMGD_WRITE32. So use as
 *  necessary.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <io.h>
#include <memory.h>
#include <sched.h>

#include <igd_pwr.h>

#include <general.h>
#include <context.h>
#include <mode.h>
#include <utils.h>

#include <tnc/regs.h>

#include <intelpci.h>

#include "../cmn/i2c_dispatch.h"

/*!
 * @addtogroup display_group
 * @{
 */

static int i2c_error_recovery_tnc(
	unsigned long hold_time);

static int i2c_write_byte_tnc(
	unsigned char value,
	unsigned long hold_time);

static int i2c_read_byte_tnc(
	unsigned char *value,
	unsigned char ack,
	unsigned long hold_time);

int i2c_read_regs_gpio(
	igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	unsigned char reg,
	unsigned char FAR *buffer,
	unsigned long num_bytes,
	unsigned long flags);

int i2c_write_reg_list_gpio(
	igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	pd_reg_t *reg_list,
	unsigned long flags);

/* The LVDS GPIO clock lines are GPIOSUS[3]
 * The LVDS GPIO data lines are GPIOSUS[4]
 */
#define GPIO_CLOCK	0x08
#define GPIO_DATA	0x10

/*!
 *
 * @param context
 *
 * @return void
 */
#if 0
static void enable_gpio_tnc(igd_context_t *context)
{
	/*
	 * NOTE: This really should be a system BIOS job.
	 * The driver would not touch these register anymore since
	 * it would cause the 13x7 panel fail to start.
	 */

	/* Enabling LVDS Data and LVDS Clock */
	unsigned long temp;

	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGEN);
	temp |= (GPIO_DATA | GPIO_CLOCK);
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGEN, temp);

	return;
}
#endif

/*!
 *
 * @param clock
 * @param data
 *
 * @return void
 */
static void i2c_get(unsigned long *clock,
	unsigned long *data)
{
	unsigned long temp;
	/* Set Data as Input */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGIO);
	temp |= (GPIO_DATA);
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGIO, temp);
	/* Read Data */
	*data = (READ_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL) & GPIO_DATA) ? 1:0;
	*clock = (READ_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL) & GPIO_CLOCK) ? 1:0;

#if 0
	EMGD_WRITE32(0, EMGD_MMIO(mmio) + i2c_bus);
	c = EMGD_READ32(EMGD_MMIO(mmio) + i2c_bus)>>4;
	*data = (c>>8) & 1;
	*clock &= 1;
#endif
}

/*!
 *
 * @param data
 * @param hold_time
 *
 * @return void
 */
static void i2c_set_data(int data,
	unsigned long hold_time)
{
	unsigned long temp;
	/* The LVDS GPIO data lines are GPIOSUS[4] */
	/* Set as Output */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGIO);
	temp &= ~GPIO_DATA;
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGIO, temp);
	/* Read status register */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL);

	if(data){
		/* Set level to High */
		temp |= GPIO_DATA;
	} else {
		/* Set level to low */
		temp &= ~GPIO_DATA;
	}
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL, temp);

	OS_DELAY(hold_time);

#if 0
	/* Implementation using Display GPIO
	 * For alm, the default data value "could" be 0
	 */
	/*
	 * Simplified definition for the bits
	 * 11: GPIO data Value
	 * 10: GPIO Data Mask
	 * 9: GPIO Data Direction Value
	 * 8: GPIO Data Direction Mask
	 */
	EMGD_WRITE32(data ? 0x500 : 0x700, EMGD_MMIO(mmio) + i2c_bus);
	EMGD_WRITE32(data ? 0x400 : 0x600, EMGD_MMIO(mmio) + i2c_bus);
	OS_DELAY(hold_time);
#endif
}

/*!
 *
 * @param clock
 * @param hold_time
 *
 * @return void
 */
static void i2c_set_clock(int clock,
	unsigned long hold_time)
{
	unsigned long temp;
	/* The LVDS GPIO clock lines are GPIOSUS[3] */
	/* Set as Output */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGIO);
	temp &= ~GPIO_CLOCK;
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGIO, temp);
	/* Read Status Register */
	temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL);

	if(clock){
		/* Set level to High */
		temp |= GPIO_CLOCK;

	} else {
		/* Set level to low */
		temp &= ~GPIO_CLOCK;
	}
	WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGLVL, temp);

	OS_DELAY(hold_time);

#if 0
	/*
	 * Simplified definition for the bits
	 * 3: GPIO Clock Value
	 * 2: GPIO Clock Mask
	 * 1: GPIO Clock Direction Value
	 * 0: GPIO Clock Direction Mask
	 */

	EMGD_WRITE32(clock ? 0x5 : 0x7, EMGD_MMIO(mmio) + i2c_bus);
	EMGD_WRITE32(clock ? 0x4 : 0x6, EMGD_MMIO(mmio) + i2c_bus);
	OS_DELAY(hold_time);
#endif
}

/*!
 *
 * @param hold_time
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_start_tnc(unsigned long hold_time)
{
	unsigned long sc, sd;

	/* set sd high */
	i2c_set_data(1, hold_time);

	/* set clock high */
	i2c_set_clock(1, hold_time);

	/* Start condition happens when sd goes high to low when sc is high */
	i2c_get(&sc, &sd);

	if( 0 == sc ) {
		// Data must be high
		i2c_error_recovery_tnc(hold_time);
		return 1;
	}

	i2c_set_data(0, hold_time);
	i2c_set_clock(0, hold_time);

	return 0;
} /* end i2c_start */

/*!
 *
 * @param hold_time
 *
 * @return 0
 */
static int i2c_stop_tnc(unsigned long hold_time)
{
	/* Stop condition happens when sd goes low to high when sc is high */
	unsigned long sc,sd;

	i2c_set_clock(0, hold_time);
	i2c_set_data(0, hold_time);

	i2c_set_clock(1, hold_time);

	i2c_get(&sc, &sd);
	/* Try another time */
	if (sc == 0) {
		i2c_set_clock(1, hold_time);
	}
	i2c_set_data(1, hold_time);

	return 0;
} /* end i2c_stop */

/*!
 *
 * @param hold_time
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_error_recovery_tnc(unsigned long hold_time)
{
	unsigned char max_retries = 9;
	unsigned long sc, sd;

	while (max_retries--) {
		i2c_get(&sc, &sd);
		if (sd == 1 && sc == 1) {
			return 0;
		} else {
			i2c_stop_tnc(hold_time);
		}
	}
	EMGD_ERROR("Cannot recover I2C error.");

	return 1;
}

/*!
 *
 * @param value
 * @param hold_time
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_write_byte_tnc(unsigned char value,
	unsigned long hold_time)
{
	int i;
	unsigned long sc,sd;

	/* I2C_DEBUG("i2c_write_byte"); */
	for(i=7; i>=0; i--) {
		i2c_set_clock(0, hold_time);
		i2c_set_data(value>>i & 1, hold_time);

		i2c_set_clock(1, hold_time);
	}

	/* Get ACK */
	i2c_set_clock(0, hold_time);
	/* Set data low. Possible inteference in some lvds panel */
	i2c_set_data(0, hold_time);
	i2c_set_clock(1, hold_time);
	OS_DELAY(hold_time);

	i2c_get(&sc, &sd);

	i2c_set_clock(0, hold_time);

	if (sd != 0) {
		EMGD_ERROR("No ACK for byte 0x%x", value);
		i2c_error_recovery_tnc(hold_time);
		return 1;
	}

	return 0;

} /* end i2c_write_byte */

/*!
 *
 * @param value
 * @param ack
 * @param hold_time
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_read_byte_tnc(unsigned char *value,
	unsigned char ack,
	unsigned long hold_time)
{
	int i;
	unsigned long sc, sd, temp;

	*value = 0;
	for(i=7; i>=0; i--) {
		i2c_set_clock(1, hold_time);
		i2c_get(&sc, &sd);
		OS_DELAY(hold_time);
		if(!sc) {
			EMGD_DEBUG("Clock low on read %d", i);
			i2c_error_recovery_tnc(hold_time);
			return 1;
		}
		*value |= (sd & 1)<<i;
		i2c_set_clock(0, hold_time);
	}

	if (ack) {
		i2c_set_data(0, hold_time);
	}

	/* Master does not ACK */
	i2c_set_clock(1, hold_time);
	i2c_set_clock(0, hold_time);

	if (ack) {
		/* Set data as input as we continue to read */
		temp = READ_MMIO_REG_TNC(IGD_PORT_LPC, RGIO);
		temp |= GPIO_DATA;
		WRITE_MMIO_REG_TNC(IGD_PORT_LPC, RGIO, temp);
	}

	return 0;
} /* end i2c_read_byte */

/*!
 *
 * @param context
 * @param i2c_bus
 * @param i2c_speed
 * @param dab
 * @param reg
 * @param buffer
 * @param num_bytes
 *
 * @return 0 on success
 * @return 1 on failure
 */
int i2c_read_regs_gpio(igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	unsigned char reg,
	unsigned char FAR *buffer,
	unsigned long num_bytes,
	unsigned long flags	
	)
{
	unsigned long hold_time;
	unsigned char temp;
	int i;
	if (!i2c_speed) {
		EMGD_DEBUG("i2c Speed failed.");
		return 1;
	}

	/*
	 * We are holding the clock LOW for "hold_time" and then HIGH for
	 * "hold_time". Therefore, we double the clock speed in this calculation.
	 */
	if (flags & IGD_I2C_WRITE_FW){
		hold_time = 1;
	} else {
	hold_time = 1000/(i2c_speed * 2);
	}

	/* enable_gpio_tnc(context); */

	if (i2c_start_tnc(hold_time)) {
		EMGD_DEBUG("i2c Start failed.");
		return 1;
	}

	if (i2c_write_byte_tnc((unsigned char)dab & 0xFE,
			hold_time)) {
		EMGD_DEBUG("i2c DAB(W) failed.");
		return 1;
	}

	if (i2c_write_byte_tnc(reg, hold_time)) {
		EMGD_DEBUG("RAB failed.");
		return 1;
	}

	if (i2c_start_tnc(hold_time)) {
		EMGD_DEBUG("i2c ReStart failed");
		return 1;
	}

	if (i2c_write_byte_tnc((unsigned char)dab | 0x01,
			hold_time)) {
		EMGD_ERROR("i2c DAB(R) failed");
		return 1;
	}


	/* Read the requested number of bytes */
	for(i=0; i<(int)(num_bytes-1); i++) {
		/*
		 * Use a local temp so that the FAR pointer doesn't have to
		 * get passed down.
		 */
		if (i2c_read_byte_tnc(&temp, 1, hold_time)) {
			EMGD_DEBUG("Read data byte %d failed", i);
			EMGD_DEBUG("Exit i2c_read_regs_tnc with error");
			return 1;
		}
		buffer[i] = temp;
	}

	/* No ACK on the last read */
	if(i2c_read_byte_tnc(&temp, 0, hold_time)) {
		EMGD_DEBUG("Read Data %d Failed", i);
		EMGD_DEBUG("Exit i2c_read_regs_tnc with error");
		return 1;
	}
	buffer[i] = temp;

	i2c_stop_tnc(hold_time);
	i2c_stop_tnc(hold_time);

	return 0;
}

/*!
 *
 * @param context
 * @param i2c_bus
 * @param i2c_speed
 * @param dab
 * @param reg_list
 * @param flags
 *
 * @return 0 on success
 * @return 1 on failure
 */
int i2c_write_reg_list_gpio(igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	pd_reg_t *reg_list,
	unsigned long flags)
{
	unsigned long hold_time;

	if (!i2c_speed) {
		return 1;
	}

	/*
	 * We are holding the clock LOW for "hold_time" and then HIGH for
	 * "hold_time". Therefore, we double the clock speed in this calculation.
	 */
	if (flags & IGD_I2C_WRITE_FW){
		hold_time = 1;	
	} else {
	hold_time = 1000/(i2c_speed * 2);
	}

	/* enable_gpio_tnc(context); */

	while(reg_list->reg != PD_REG_LIST_END) {
		if (i2c_start_tnc(hold_time)) {
			EMGD_DEBUG("Start failed");
			return 1;
		}

		if (i2c_write_byte_tnc((unsigned char)dab & 0xFE,
				hold_time)) {
			EMGD_DEBUG("DAB(W) failed");
			return 1;
		}

		/* Register Address */
		if (i2c_write_byte_tnc((unsigned char)reg_list->reg, hold_time)) {
			EMGD_DEBUG("RAB failed");
			return 1;
		}

		do {
			/*  New Value */
			if (i2c_write_byte_tnc((unsigned char)reg_list->value, hold_time)) {
				EMGD_DEBUG("Data failed");
				return 1;
			}

			if(reg_list[1].reg != (reg_list[0].reg + 1)) {
				reg_list++;
				break;
			}

			EMGD_DEBUG("I2C Multi-Write Reg[%x] = 0x%x",
				(unsigned short)reg_list->reg,
				(unsigned short)reg_list->value);
			reg_list++;
		} while(flags & IGD_I2C_SERIAL_WRITE);


		i2c_stop_tnc(hold_time);
		i2c_stop_tnc(hold_time);
	}

	return 0;
}


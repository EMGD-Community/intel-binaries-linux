/*
 *-----------------------------------------------------------------------------
 * Filename: i2c_plb.c
 * $Revision: 1.14 $
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

#define MODULE_NAME hal.dpd

#include <io.h>
#include <memory.h>
#include <sched.h>

#include <igd_pwr.h>

#include <general.h>
#include <context.h>
#include <mode.h>
#include <utils.h>

#include <plb/regs.h>

#include "../cmn/i2c_dispatch.h"

/*!
 * @addtogroup display_group
 * @{
 */

/*......................................................................... */
extern igd_display_port_t dvob_port_plb;

/*......................................................................... */
static int i2c_read_regs_plb(
	igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	unsigned char reg,
	unsigned char FAR *buffer,
	unsigned long num_bytes,
	unsigned long flags);

static int i2c_write_reg_list_plb(
	igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	pd_reg_t *reg_list,
	unsigned long flags);

i2c_dispatch_t i2c_dispatch_plb = {
	i2c_read_regs_plb,
	i2c_write_reg_list_plb,
};


/*.......................................................................... */
typedef enum {

	GMBUS_SPEED_50K     = 0x0100,
	GMBUS_SPEED_100K	= 0x0000,
	GMBUS_SPEED_400K	= 0x0200,
	GMBUS_SPEED_1000K	= 0x0300,

} gmbus_speed_t;

typedef enum {

	SDVOB_ADDR	= 0x70,
	SDVOC_ADDR	= 0x72,

} sdvo_dev_addr_t;

typedef enum {

	DDC1_ADDR = 0xA0,
	DDC2_ADDR = 0xA2,

} gmbus_ddc_addr_t;


typedef enum {

	GMBUS_PINS_DEDICATED = 1, /* Dedicated Control/GMBUS Pins */
	/* LCTRCLKA, LCTRLCLKB SSC Clock Device */
	GMBUS_PINS_ANALOG	= 2,  /* Analog DDC */
	GMBUS_PINS_INT_LVDS = 3,  /* Alviso : Integrated Digital Panel */
	GMBUS_PINS_SDVO     = 5,  /* SDVO Registers, DDC, PROM */

} gmbus_pins_pair_t;


typedef enum {

	I2C_WRITE = 0,
	I2C_READ  = 1,

} i2c_bus_dir_t;

/*.......................................................................... */
typedef enum {

	SDVO_BUS_PROM = BIT(0),
	SDVO_BUS_DDC1 = BIT(1),
	SDVO_BUS_DDC2 = BIT(2),

} sdvo_bus_switch_t;

#define SDVO_OPCODE_BUS_SWITCH	0x7A

#define SDVO_INDEX_PARAM_1		0x07
#define SDVO_INDEX_OPCODE		0x08
#define SDVO_INDEX_STATUS		0x09

#define SDVO_STATUS_SUCCESS     0x01
#define SDVO_STATUS_PENDING     0x04

/*.......................................................................... */
/*
 * In 16-bit, the mmio is a 16-bit pointer, the watcom 1.2 compiler will have
 * error if directly convert it to unsigned long.  Normally, have to cast it to
 * unsigned short first then cast again to unsigned long; then, it will be
 * correct.  But this type of casting may cause some error in the 32 and 64 bit
 * code.  Since mmio will be equal to zero for 16-bit code.  Add the checking
 * for MICRO definition code to correct the macro by remove mmio.
 */
#define READ_GMCH_REG(reg)			EMGD_READ32(EMGD_MMIO(mmio) + reg)
#define WRITE_GMCH_REG(reg, data)	EMGD_WRITE32(data, EMGD_MMIO(mmio) + reg)

static int gmbus_init(unsigned char *mmio, unsigned long i2c_bus,
	unsigned long i2c_speed);

static int gmbus_read_edid(unsigned char *mmio,
	unsigned long ddc_addr,
	unsigned long slave_addr,
	unsigned long index,
	unsigned long num_bytes,
	unsigned char FAR *buffer);

static int gmbus_read_reg(unsigned char *mmio,
	unsigned long slave_addr,
	unsigned long index,
	unsigned char FAR *data);

static int gmbus_write_reg(unsigned char *mmio,
	unsigned long slave_addr,
	unsigned long index,
	unsigned char data);

static int gmbus_set_control_bus_switch(unsigned char *mmio,
	unsigned long slave_addr,
	gmbus_ddc_addr_t ddc_addr);

static int gmbus_wait_event_one(unsigned char *mmio, unsigned long bit);
static int gmbus_wait_event_zero(unsigned char *mmio, unsigned long bit);
static int gmbus_error_handler(unsigned char *mmio);

/*!
 * i2c_read_regs_plb is called to read Edid or a single sDVO register
 *
 * @param context
 * @param i2c_bus port->ddc_reg, port->i2c_reg
 * @param i2c_speed 50, 100, 400, 1000 (Khz)
 * @param dab 0x70/0x72 (sDVO Regs), 0xA0/0xA2 (sDVO/Analog DDC)
 * @param reg I2C Reg Index
 * @param num_bytes <= 508
 * @param buffer Data read
 * @param flags unused, put in to match TNC func signature
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_read_regs_plb(igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	unsigned char reg,
	unsigned char FAR *buffer,
	unsigned long num_bytes,
	unsigned long flags)
{
	unsigned char *mmio = EMGD_MMIO(context->device_context.virt_mmadr);
	unsigned long slave_addr;

	if (! gmbus_init(mmio, i2c_bus, i2c_speed)) {
		EMGD_DEBUG("Error ! i2c_read_regs_plb : gmbus_init() failed");
		return 1;
	}

	/*	If the request is to read Edid from sDVO display, find out the */
	/*	i2c addres of the sDVO device */
	if (i2c_bus == GMBUS_DVOB_DDC) {
		slave_addr = dvob_port_plb.dab;

	} else {
		slave_addr = 0;

	}

	switch (i2c_bus) {
	case GMBUS_ANALOG_DDC :
	case GMBUS_INT_LVDS_DDC :
	case GMBUS_DVOB_DDC :
	case GMBUS_DVOC_DDC :
		if (! gmbus_read_edid(mmio, dab, slave_addr, reg, num_bytes, buffer)) {

			EMGD_DEBUG("Error ! i2c_read_regs_plb : gmbus_read_edid() failed");
			return 1;
		}
		break;

	case GMBUS_DVO_REG :
		if (! gmbus_read_reg(mmio, dab, reg, buffer)) {

			EMGD_DEBUG("Error ! i2c_read_regs_plb : gmbus_read_reg() failed");
			return 1;
		}
		break;

	default :
		EMGD_ERROR("Error ! i2c_read_regs_plb : Invalid i2c_bus=0x%lx",
			i2c_bus);
		return 1;
	}

	return 0;
}

/*!
 * i2c_write_reg_list_plb is called to write a list of i2c registers to sDVO
 * device
 *
 * @param context
 * @param i2c_bus NAP_GMBUS_DVOB_DDC/NAP_GMBUS_DVOC_DDC
 * @param i2c_speed 1000 Khz
 * @param dab 0x70/0x72
 * @param reg_list List of i2c indexes and data, terminated with register index
 *  set to PD_REG_LIST_END
 *
 * @return 0 on success
 * @return 1 on failure
 */
static int i2c_write_reg_list_plb(igd_context_t *context,
	unsigned long i2c_bus,
	unsigned long i2c_speed,
	unsigned long dab,
	pd_reg_t *reg_list,
	unsigned long flags)
{
	unsigned char *mmio = EMGD_MMIO(context->device_context.virt_mmadr);
	unsigned long reg_num = 0, ddc_addr = 0, slave_addr = 0;

	if (! gmbus_init(mmio, i2c_bus, i2c_speed)) {

		EMGD_DEBUG("Error ! i2c_write_reg_list_plb : gmbus_init() failed");
		return 1;
	}
	/*If it is SDVO Make sure we issue SDVO command to enable DDC access*/
	if ((i2c_bus == GMBUS_DVOB_DDC) || (i2c_bus == GMBUS_DVOC_DDC)) {
		if (i2c_bus == GMBUS_DVOB_DDC) {
			slave_addr = dvob_port_plb.dab;
		}else if (i2c_bus == GMBUS_DVOC_DDC) {
			/* Is DVOC available in PLB? */
			//slave_addr = dvoc_port_plb.dab;
			slave_addr = 0;
		}
		ddc_addr = 0;
		if (! gmbus_set_control_bus_switch(mmio, slave_addr, ddc_addr)) {
			EMGD_DEBUG("Error ! i2c_write_reg_list_plb : gmbus_set_control_bus_switch()"
					 " failed");
			return 1;
		}
		while (reg_list[reg_num].reg != PD_REG_LIST_END) {

			if (! gmbus_write_reg(mmio, dab, reg_list[reg_num].reg,
					  (unsigned char)reg_list[reg_num].value)) {

				EMGD_DEBUG("Error ! i2c_write_reg_list_plb : gmbus_write_reg() failed, reg_num=%lu",
					reg_num);

				return 1;
			}
		reg_num++;
		}
		/*...................................................................... */
		/* Issue a Stop Command */
		gmbus_wait_event_one(mmio, HW_WAIT);
		WRITE_GMCH_REG(GMBUS1, STO | SW_RDY | ddc_addr);
		gmbus_wait_event_one(mmio, HW_RDY);
		gmbus_wait_event_zero(mmio, GA);
		gmbus_error_handler(mmio);
		WRITE_GMCH_REG(GMBUS1, SW_RDY);
		WRITE_GMCH_REG(GMBUS1, SW_CLR_INT);
		WRITE_GMCH_REG(GMBUS1, 0);
		WRITE_GMCH_REG(GMBUS5, 0);
		WRITE_GMCH_REG(GMBUS0, 0);
		/*...................................................................... */
		return 0;
	}
	while (reg_list[reg_num].reg != PD_REG_LIST_END) {

		if (! gmbus_write_reg(mmio, dab, reg_list[reg_num].reg,
				(unsigned char)reg_list[reg_num].value)) {

			EMGD_DEBUG("Error ! i2c_write_reg_list_plb : gmbus_write_reg() failed, reg_num=%lu",
				reg_num);

			return 1;
		}

		reg_num++;
	}

	return 0;
}

/*!
 * gmbus_init initializes the GMBUS controller with specified bus and speed
 *
 * @param mmio
 * @param i2c_bus sDVO B/C Reg/DDC or Analog DDC
 * @param i2c_speed 50/100/400/1000 Khz
 *
 * @return TRUE(1) on success
 * @return FALSE(0) on failure
 */
static int gmbus_init(unsigned char *mmio, unsigned long i2c_bus,
	unsigned long i2c_speed)
{
	gmbus_pins_pair_t pin_pair;
	gmbus_speed_t bus_speed;

	switch (i2c_bus) {

	case GMBUS_DVO_REG :
	case GMBUS_DVOB_DDC :
	case GMBUS_DVOC_DDC :
		pin_pair = GMBUS_PINS_SDVO;
		break;

	case GMBUS_ANALOG_DDC :
		pin_pair = GMBUS_PINS_ANALOG;
		break;

	case GMBUS_INT_LVDS_DDC :
		pin_pair = GMBUS_PINS_INT_LVDS;
		break;

	default :
		EMGD_ERROR("Error ! gmbus_init : Invalid i2c_bus=0x%lx", i2c_bus);
		return 0;
	}

	switch (i2c_speed) {

	case 50 :		/* Slow speed */
		bus_speed = GMBUS_SPEED_50K;
		break;

	case 400 :		/* SPD */
		bus_speed = GMBUS_SPEED_400K;
		break;

	case 1000 :     /* sDVO Registers */
		bus_speed = GMBUS_SPEED_1000K;
		break;

	case 100 :      /* DDC */
	default :
		bus_speed = GMBUS_SPEED_100K;
		break;
	}

	WRITE_GMCH_REG(GMBUS5, 0);   /* Clear the word index reg */
	WRITE_GMCH_REG(GMBUS0, pin_pair | bus_speed);

	return 1;
}

/*!
 * gmbus_wait_event_zero waits for specified GMBUS2 register bit to be deasserted
 *
 * @param mmio
 * @param bit
 *
 * @return TRUE(1) on success. The bit was deasserted in the specified timeout period
 * @return FALSE(0) on failure
 */
static int gmbus_wait_event_zero(unsigned char *mmio, unsigned long bit)
{
	unsigned long i;
	unsigned long status;

	for (i = 0; i < 0x1000; i++) {

		status = READ_GMCH_REG(GMBUS2);

		if ((status & bit) == 0) {

			return 1;
		}
	}

	EMGD_DEBUG("Error ! gmbus_wait_event_zero : Failed : bit=0x%lx, status=0x%lx, forcing reset",
		bit, status);

	/* If we are here, that means that the GBMUS is busy or in a bad
	 * state, the situation was observed
	 * that the GMBUS never becomes available (idle state) after S3.
	 * This results in the driver never being able to set the display.
	 *
	 * To fix this, we force force a reset of the GMBUS.
	 */
	WRITE_GMCH_REG(GMBUS1, SW_RDY);
	WRITE_GMCH_REG(GMBUS1, SW_CLR_INT);
	WRITE_GMCH_REG(GMBUS1, 0);

	return 1;
}

/*!
 * gmbus_wait_event_one wait for specified GMBUS2 register bits to be asserted
 *
 * @param mmio
 * @param bit
 *
 * @return TRUE(1) on success. The bit was asserted in the specified timeout period
 * @return FALSE(0) on failure
 */
static int gmbus_wait_event_one(unsigned char *mmio, unsigned long bit)
{
	unsigned long i;
	unsigned long status;

	for (i = 0; i < 0x10000; i++) {

		status = READ_GMCH_REG(GMBUS2);
		if ((status & bit) != 0) {

			return 1;
		}
	}

	EMGD_DEBUG("Error ! gmbus_wait_event_one : Failed : bit=0x%lx, status=0x%lx",
		bit, status);

	return 0;
}

/*!
 * gmbus_error_handler attempts to recover from timeout error
 *
 * @param mmio
 *
 * @return TRUE(1) error was detected and handled
 * @return FALSE(0) there was no error
 */
static int gmbus_error_handler(unsigned char *mmio)
{
	unsigned long status = READ_GMCH_REG(GMBUS2);

	/* Clear the SW_INT, wait for HWRDY and GMBus active (GA) */
	if ((status & HW_BUS_ERR) || (status & HW_TMOUT)) {

		EMGD_DEBUG("Error ! gmbus_error_handler : Resolving error=0x%lx",
			status);

		WRITE_GMCH_REG(GMBUS1, SW_RDY);
		WRITE_GMCH_REG(GMBUS1, SW_CLR_INT);
		WRITE_GMCH_REG(GMBUS1, 0);

		gmbus_wait_event_zero(mmio, GA);

		return 1;	/* Handled the error */
	}

	return 0;	/* There was no error */
}

/*!
 * Assemble 32 bit GMBUS1 command
 *
 * @param slave_addr 0x70/0x72
 * @param index 0 - 256
 * @param num_bytes Bytes to transfer
 * @param flags Bits 25-31 of GMBUS1
 * @param i2c_dir I2C_READ / I2C_WRITE
 *
 * @return The assembled command
 */
static unsigned long gmbus_assemble_command(unsigned long slave_addr, unsigned long index,
	unsigned long num_bytes, unsigned long flags,
	i2c_bus_dir_t i2c_dir)
{
	unsigned long cmd = flags | ENIDX | ENT | (num_bytes << 16) | (index << 8) |
						slave_addr | i2c_dir;

	return cmd;
}

/*!
 * gmbus_send_pkt transmits a block a data to specified i2c slave device
 *
 * @param mmio
 * @param slave_addr I2C device address
 * @param index Starting i2c register index
 * @param pkt_size 1 - 508 bytes
 * @param pkt Bytes to send
 *
 * @return TRUE(1) if successful in sending the specified number of bytes
 * @return FALSE(0) on failure
 */
static int gmbus_send_pkt(unsigned char *mmio,
	unsigned long slave_addr, unsigned long index,
	unsigned long pkt_size, void *pkt)
{
	unsigned long gmbus1_cmd;
	unsigned long bytes_sent;
	unsigned long *data;

	if ((pkt_size == 0) || (pkt == NULL) || (pkt_size > 508)) {

		return 0;
	}

	data = (unsigned long *)pkt;

	/*...................................................................... */
	gmbus_error_handler(mmio);

	gmbus1_cmd = gmbus_assemble_command(slave_addr, index, pkt_size,
										STA, I2C_WRITE);
	if (pkt_size <= 4) {

		gmbus1_cmd |= SW_RDY;
	}

	/*...................................................................... */
	bytes_sent = 0;

	do {

		WRITE_GMCH_REG(GMBUS3, *data);

		if (bytes_sent == 0) {

			WRITE_GMCH_REG(GMBUS1, gmbus1_cmd);
		}

		if (! gmbus_wait_event_one(mmio, HW_RDY)) {

			EMGD_DEBUG("Error ! gmbus_send_pkt : Failed to get HW_RDY, bytes_sent=%ld",
				bytes_sent);

			return 0;
		}

		if (gmbus_error_handler(mmio)) {

			EMGD_DEBUG("Error ! gmbus_send_pkt : gmbus error, bytes_sent=%ld",
				bytes_sent);

			return 0;
		}

		data++;

		if (pkt_size >= 4) {
			bytes_sent += 4;

		} else {
			bytes_sent += pkt_size;
		}

	} while (bytes_sent < pkt_size);

	/*...................................................................... */
	if (bytes_sent != pkt_size) {

		return 0;

	} else {

		return 1;
	}
}

/*!
 * gmbus_recv_pkt reads a block of data from specified i2c slave device
 *
 * @param mmio
 * @param slave_addr I2C device address
 * @param index Starting i2c register index
 * @param pkt_size 1 - 508 bytes
 * @param pkt Bytes to send
 *
 * @return TRUE(1) if successful in receiving specified number of bytes
 * @return FALSE(0) on failure
 */
static int gmbus_recv_pkt(unsigned char *mmio,
	unsigned long slave_addr, unsigned long index,
	unsigned long pkt_size, void FAR *pkt)
{
	unsigned long gmbus1_cmd;
	unsigned long bytes_rcvd;
	unsigned long FAR *data;

	if ((pkt_size == 0) || (pkt == NULL) || (pkt_size > 508)) {

		return 0;
	}

	data = (unsigned long FAR *)pkt;

	/*...................................................................... */
	gmbus_error_handler(mmio);

	/* Program the command */
	gmbus1_cmd = gmbus_assemble_command(slave_addr, index, pkt_size,
										STA | SW_RDY, I2C_READ);
	WRITE_GMCH_REG(GMBUS1, gmbus1_cmd);

	/*...................................................................... */
	bytes_rcvd = 0;
	do {

		unsigned long gmbus3_data;
		unsigned long bytes_left = pkt_size - bytes_rcvd;

		if (! gmbus_wait_event_one(mmio, HW_RDY)) {

			EMGD_DEBUG("Error ! gmbus_recv_pkt : Failed to get HW_RDY, "
				"bytes_rcvd=%ld", bytes_rcvd);
			break;
		}

		if (gmbus_error_handler(mmio)) {

			EMGD_DEBUG("Error ! gmbus_recv_pkt : gmbus error, bytes_rcvd=%ld",
				bytes_rcvd);
			break;
		}

		gmbus3_data = READ_GMCH_REG(GMBUS3);

		switch (bytes_left) {

		case 1 :
			*(unsigned char *)data = (unsigned char)gmbus3_data;
			break;

		case 2 :
			*(unsigned short *)data = (unsigned short)gmbus3_data;
			break;

		case 3 :
		{
			unsigned char *dest = (unsigned char *)data;
			 unsigned char *src  = (unsigned char *)&(gmbus3_data);
			dest[0] = src[0];
			dest[1] = src[1];
			dest[2] = src[2];

			break;
		}

		default :	/* >= 4 */
			*data = gmbus3_data;
			break;
		}

		if (bytes_left > 4) {
			bytes_rcvd += 4;
			data++;

		} else {
			bytes_rcvd += bytes_left;

		}

	} while (bytes_rcvd < pkt_size);

	/*...................................................................... */
	if (bytes_rcvd < pkt_size) {
		return 0;

	} else {
		return 1;
	}
}

/*!
 * gmbus_set_control_bus_switch sends sDVO command to switch i2c bus to read EDID
 * or SPD data
 *
 * @param mmio
 * @param slave_addr sDVO device address (0x70/0x72)
 * @param ddc_addr DDC1_ADDR/DDC2_ADDR
 *
 * @return TRUE(1) if successful in sending the opcode
 * @return FALSE(0) on failure
 */
static int gmbus_set_control_bus_switch(unsigned char *mmio,
	unsigned long slave_addr,
	gmbus_ddc_addr_t ddc_addr)
{
	unsigned char data;
	sdvo_bus_switch_t bus_switch;
	int retry;

	bus_switch = SDVO_BUS_DDC1;

	/*...................................................................... */
	/*	Transmit the Arguments */
	if (! gmbus_send_pkt(mmio, slave_addr, SDVO_INDEX_PARAM_1, 1, &bus_switch)) {

		EMGD_DEBUG("Error ! gmbus_set_control_bus_switch : gmbus_send_pkt() failed");

		return 0;
	}

	/*...................................................................... */
	/* Generate I2C stop cycle */
	gmbus_wait_event_one(mmio, HW_WAIT);
	WRITE_GMCH_REG(GMBUS1, STO | SW_RDY | slave_addr);
	gmbus_wait_event_one(mmio, HW_RDY);
	gmbus_wait_event_zero(mmio, GA);

	/*...................................................................... */
	/* Transmit the Opcode */
	data = SDVO_OPCODE_BUS_SWITCH;
	if (! gmbus_send_pkt(mmio, slave_addr, SDVO_INDEX_OPCODE, 1, &data)) {

		EMGD_DEBUG("Error ! gmbus_set_control_bus_switch : gmbus_send_pkt(Opcode)"
				 " failed");

		return 0;
	}

	/*...................................................................... */
	/* Read Status */
	for (retry = 0; retry < 3; retry++) {
		if (! gmbus_recv_pkt(mmio, slave_addr, SDVO_INDEX_STATUS, 1, &data)) {

			continue;
		}

		if (data != SDVO_STATUS_PENDING) {

			break;
		}
	}

	/*...................................................................... */
	/* Send Stop */
	gmbus_wait_event_one(mmio, HW_WAIT);
	WRITE_GMCH_REG(GMBUS1, STO | SW_RDY | slave_addr);
	gmbus_wait_event_one(mmio, HW_RDY);
	gmbus_wait_event_zero(mmio, GA);

	/*...................................................................... */
	if (data != SDVO_STATUS_SUCCESS) {

		EMGD_DEBUG("Error ! gmbus_set_control_bus_switch : Opcode Bus Switch failed");

		return 0;
	}

	return 1;
}

/*!
 * gmbus_read_edid reads specified number of Edid data bytes
 *
 * @param mmio
 * @param ddc_addr 0xA0/0xA2 (DDC1/DDC2)
 * @param slave_addr 0x70/0x72 (sDVOB, sDVOC), 0 Analog
 * @param index i2c register index
 * @param num_bytes <= 508
 * @param buffer Edid data read from the display
 *
 * @return TRUE(1) if successful in reading Edid
 * @return FALSE(0) on failure
 */
static int gmbus_read_edid(unsigned char *mmio,
	unsigned long ddc_addr,
	unsigned long slave_addr,
	unsigned long index,
	unsigned long num_bytes,
	unsigned char FAR *buffer)
{
	int status;

	if ((slave_addr == SDVOB_ADDR) || (slave_addr == SDVOC_ADDR)) {

		if (! gmbus_set_control_bus_switch(mmio, slave_addr, ddc_addr)) {

			EMGD_DEBUG("Error ! gmbus_read_edid : gmbus_set_control_bus_switch()"
					 " failed");

			return 0;
		}
	} else {
		/*	Reset the bus */
		gmbus_recv_pkt(mmio, ddc_addr, 0, 1, buffer);
	}

	status = gmbus_recv_pkt(mmio, ddc_addr, index, num_bytes, buffer);
	if (! status) {

		EMGD_DEBUG("Error ! gmbus_read_edid : gmbus_recv_pkt() failed");
	}

	/*...................................................................... */
	/* Issue a Stop Command */

	gmbus_wait_event_one(mmio, HW_WAIT);
	WRITE_GMCH_REG(GMBUS1, STO | SW_RDY | ddc_addr);
	gmbus_wait_event_one(mmio, HW_RDY);

	gmbus_wait_event_zero(mmio, GA);

	gmbus_error_handler(mmio);
	WRITE_GMCH_REG(GMBUS1, SW_RDY);
	WRITE_GMCH_REG(GMBUS1, SW_CLR_INT);
	WRITE_GMCH_REG(GMBUS1, 0);
	WRITE_GMCH_REG(GMBUS5, 0);
	WRITE_GMCH_REG(GMBUS0, 0);

	/*...................................................................... */
	return status;
}

/*!
 * gmbus_read_reg reads one i2c register
 *
 * @param mmio
 * @param slave_addr 0x70/0x72 (sDVOB, sDVOC)
 * @param index i2c register index
 * @param data register data
 *
 * @return TRUE(1) if successful in reading the i2c register
 * @return FALSE(0) on failure
 */
static int gmbus_read_reg(unsigned char *mmio,
	unsigned long slave_addr,
	unsigned long index,
	unsigned char FAR *data)
{
	unsigned long gmbus1_cmd;

	WRITE_GMCH_REG(GMBUS5, 0x0);		/* Clear Word Index register */

	if (! gmbus_wait_event_zero(mmio, GA)) {

		EMGD_DEBUG("Error ! gmbus_read_reg : Failed to get GA(1)");

		return 0;
	}

	gmbus1_cmd = gmbus_assemble_command(slave_addr, index, 1,
										STO | STA, I2C_READ);
	WRITE_GMCH_REG(GMBUS1, gmbus1_cmd);

	if (! gmbus_wait_event_zero(mmio, GA)) {

		EMGD_DEBUG("Error ! gmbus_read_reg : Failed to get GA(2)");

		return 0;
	}

	*data = (unsigned char)READ_GMCH_REG(GMBUS3);

	return 1;
}

/*!
 * gmbus_write_reg writes one i2c register
 *
 * @param mmio
 * @param slave_addr 0x70/0x72 (sDVOB, sDVOC)
 * @param index i2c register index
 * @param data register data
 *
 * @return TRUE(1) if successful in updating the i2c register
 * @return FALSE(0) if failed to update the register
 */
static int gmbus_write_reg(unsigned char *mmio,
	unsigned long slave_addr,
	unsigned long index,
	unsigned char data)
{
	unsigned long gmbus1_cmd;

	WRITE_GMCH_REG(GMBUS5, 0x0);		/* Clear Word Index register */

	if (! gmbus_wait_event_zero(mmio, GA)) {

		EMGD_DEBUG("Error ! gmbus_write_reg : Failed to get GA(1)");

		return 0;
	}

	WRITE_GMCH_REG(GMBUS3, data);

	gmbus1_cmd = gmbus_assemble_command(slave_addr, index, 1,
										STO | STA, I2C_WRITE);
	WRITE_GMCH_REG(GMBUS1, gmbus1_cmd);

	if (! gmbus_wait_event_zero(mmio, GA)) {

		EMGD_DEBUG("Error ! gmbus_write_reg : Failed to get GA(2)");
		return 0;
	}

	return 1;
}


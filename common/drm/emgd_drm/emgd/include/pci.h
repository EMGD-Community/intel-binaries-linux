/*
 *-----------------------------------------------------------------------------
 * Filename: pci.h
 * $Revision: 1.6 $
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
 *  This file contains OS abstractions for PCI function calls.
 *-----------------------------------------------------------------------------
 */

#ifndef _OAL_PCI_H
#define _OAL_PCI_H


/*
 * Standard PCI register definitions.
 * Only the first 64 bytes are standardized so thiese must all be
 * defined to numbers less than 0x40
 */
#define PCI_RID            0x08

#define PCI_BAR_0           0x10
#define PCI_BAR_1           0x14
#define PCI_BAR_2           0x18
#define PCI_BAR_3           0x1c
#define PCI_BAR_4           0x20
#define PCI_BAR_5           0x24

#define PCI_INTERRUPT_LINE  0x3c

/*
 * This macro _may_ be defined by an OAL port to enable the PCI device
 * Commonly this is used to enable the device on EFI
 * The prototype of the function looks like this:
 * int os_enable_pci(os_pci_dev dev);
 * The return value from this function should be 0 to indicate success
 * or non zero to indicate failure.
 */
#ifdef _OS_ENABLE_PCI
#define OS_ENABLE_PCI(a) _OS_ENABLE_PCI(a)
#else
#define OS_ENABLE_PCI(a) 0
#endif

#define OS_PCI_GET_SLOT_ADDRESS(p, b, s, f) os_pci_get_slot_address(p, b, s, f)
#define OS_PCI_READ_CONFIG_8(p, o, v)       os_pci_read_config_8(p, o, (v))
#define OS_PCI_READ_CONFIG_16(p, o, v)      os_pci_read_config_16(p, o, (v))
#define OS_PCI_READ_CONFIG_32(p, o, v)      os_pci_read_config_32(p, o, (v))
#define OS_PCI_WRITE_CONFIG_8(p, o, v)      os_pci_write_config_8(p, o, v)
#define OS_PCI_WRITE_CONFIG_16(p, o, v)     os_pci_write_config_16(p, o, v)
#define OS_PCI_WRITE_CONFIG_32(p, o, v)     os_pci_write_config_32(p, o, v)
#define OS_PCI_FREE_DEVICE(p)               os_pci_free_device(p)

#define OS_PCI_FIND_DEVICE(v, d, p, bus, dev, func) \
	os_pci_find_device(v, d, p, bus, dev, func)

/*****************************************************************************
 * Variable: os_pci_dev_t
 *
 * Description:
 *  This is a data type that serves as a handle for allocated PCI device.
 *
 ****************************************************************************/
typedef unsigned char *os_pci_dev_t;


/*****************************************************************************
 * Function: os_pci_find_device
 *
 * Parameters:
 *  vendor_id : The vendor ID for the device to be found.
 *  device_id : The vendor ID for the device to be found.
 *  bus       : The bus number of the device in the PCI topology
 *  dev       : The device number of the device in the PCI topology
 *  func      : The function number of the device in the PCI topology
 *  pci_device: The last found os_pci_dev_t or NULL.
 *
 * Description:
 *  This function will find the PCI device for the paticular vendor and device,
 *  bus, device, and function number.The pci_device parameter should be NULL when
 *  calling the first time
 *  and the last returned value when searching for multiple devices of
 *  the same ID.
 *
 *  Notes: If the bus number is 0xFFFF, then the function searches for that
 *  vendor_id, device_id pair in the whole PCI topology of the system i.e
 *  it goes through all the buses, devices, functions in the system
 *
 ****************************************************************************/
os_pci_dev_t os_pci_find_device(
		unsigned short vendor_id,
		unsigned short device_id,
		unsigned short bus,
		unsigned short dev,
		unsigned short func,
		os_pci_dev_t pci_dev);

/*****************************************************************************
 * Function: os_get_slot_address
 *
 * Parameters:
 *  pci_device: The os_pci_dev_t to query.
 *  bus: The returned bus or NULL if the bus is not needed.
 *  slot: The returned slot or NULL if the bus is not needed.
 *  func: The returned func or NULL if the bus is not needed.
 *
 * Description:
 *  This function will return the bus slot and function for an os_pci_dev_t
 *  previously obtained from os_pci_find_device(). Any of the bus/slot/func
 *  parameters may be null if the information is not needed.
 *
 ****************************************************************************/
int os_pci_get_slot_address(
	os_pci_dev_t pci_dev,
	unsigned int *bus,
	unsigned int *slot,
	unsigned int *func);

/*****************************************************************************
 * Function: os_pci_read_config_8
 *
 * Description:
 *  This function retrieves a byte of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 ****************************************************************************/
int os_pci_read_config_8(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned char* val
		);


/*****************************************************************************
 * Function: os_pci_read_config_16
 *
 * Description:
 *  This function retrieves a word of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 ****************************************************************************/
int os_pci_read_config_16(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned short* val
		);


/*****************************************************************************
 * Function: os_pci_read_config_32
 *
 * Description:
 *  This function retrieves double word of information, starting at the
 *  specified offset, from the PCI configuration space on a particular PCI
 *  device.
 *
 ****************************************************************************/
int os_pci_read_config_32(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned long* val
		);


/*****************************************************************************
 * Function: os_pci_write_config_8
 *
 * Description:
 *  This function sets a byte of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 ****************************************************************************/
int os_pci_write_config_8(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned char val
		);


/*****************************************************************************
 * Function: os_pci_write_config_16
 *
 * Description:
 *  This function sets a word of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 ****************************************************************************/
int os_pci_write_config_16(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned short val
		);


/*****************************************************************************
 * Function: os_pci_write_config_32
 *
 * Description:
 *  This function sets double word of data, starting at the specified offset,
 *  to the PCI configuration space for a particular PCI device.
 *
 ****************************************************************************/
int os_pci_write_config_32(
		os_pci_dev_t pci_dev,
		unsigned long offset,
		unsigned long val
		);


/*****************************************************************************
 * Function: os_pci_disable_legacy_vga_decoding
 *
 * Description:
 *  Disabled legacy VGA decoding on a specific PCI device if the kernel is
 *  compiled with support for the VGA arbiter.  If the VGA arbiter is not
 *  compiled in, this function is a noop.
 *
 ****************************************************************************/
int os_pci_disable_legacy_vga_decoding(
		os_pci_dev_t pci_dev
		);


/*****************************************************************************
 * Function: os_pci_free_device
 *
 * Description:
 *  This function free the os_pci_dev_t * that previously allocated with the
 *  os_pci_find_device.
 *
 ****************************************************************************/
void os_pci_free_device(
		os_pci_dev_t pci_dev
		);

#endif

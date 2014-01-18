/*
 *-----------------------------------------------------------------------------
 * Filename: pci.c
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
 *  This file contains Linux kernel space PCI API for the OAL abstractions.
 *-----------------------------------------------------------------------------
 */

#include <memory.h>
#include <pci.h>
#include <linux/pci.h>
#include <io.h>

#if defined(CONFIG_VGA_ARB)
#include <linux/vgaarb.h>
#endif


//
// This is our local representation of a PCI device.
// This struct is private to this file, and exposed to calling functions
// only through an opaque "os_pci_dev_t" handle.
//
typedef struct _linuxkernel_pci {
  struct pci_dev *dev;
  unsigned int bus;
  unsigned int slot;
  unsigned int func;
}linuxkernel_pci_t;

/*----------------------------------------------------------------------
 * Function:
 *pci_find_device_generic
 * Parameters:
 *unsigned short vendor,
 *unsigned short device,
 *os_pci_dev_t os_pci_dev
 * Description:
 *This function finds a PCI device by going through 255 buses, 32 devices
 *and 8 functions and tries to match each ones vendor and device ids with
 *the ones given to it as parameters. Stops for the first device it finds
 *with a matching vendor and device, so it will not find multiple devices.
 *This is _NOT_ an exported OAL
 *function.
 * Returns:
 *os_pci_dev_t
 *---------------------------------------------------------------------
 */
static os_pci_dev_t pci_find_device_generic(
					    unsigned short vendor_id,
					    unsigned short device_id,
					    os_pci_dev_t pci_dev_handle
					    )
{
  struct pci_dev *our_device; // Kernel struct for a PCI device
  linuxkernel_pci_t *pdev; // Our struct for a PCI device.

    // Locate the device, and lock it. Start search at the start of the list.
    our_device = pci_get_device(vendor_id, device_id, NULL);
    // If we didn't find it, return an error.
    if (!our_device){
      return (os_pci_dev_t)NULL;
    }

    // Get the pointer to the destination for the data.
    // pci_dev_handle is an opaque pointer to a linuxkernel_pci_t struct
    // If there isn't one, allocate it.
    pdev = (linuxkernel_pci_t *)pci_dev_handle;
    if(!pdev) {

      // Caller did not supply a handle to a PCI device struct.
      // Allocate one.
      pdev = (linuxkernel_pci_t *)OS_ALLOC(sizeof(linuxkernel_pci_t));
      if(!pdev) {
	return (os_pci_dev_t)NULL;
      }
      // Zero the destination memory
      memset(pdev, 0, sizeof(linuxkernel_pci_t));
    }

    // Copy over from the kernel struct to our struct.
    // It is safe to copy a pointer to the pci_dev, since we
    // have a lock on it.
    pdev->dev = our_device;
    pdev->bus = our_device->bus->number;
    pdev->slot = PCI_SLOT(our_device->devfn);
    pdev->func = PCI_FUNC(our_device->devfn);

    return (os_pci_dev_t)pdev;
}


/*----------------------------------------------------------------------
 * Function:
 *os_pci_find_device
 *
 *  Parameters:
 *unsigned short vendor,
 *unsigned short device,
 *unsigned short bus,
 *unsigned short dev,
 *unsigned short func,
 *os_pci_dev_t os_pci_dev
 *
 *  Description:
 *  This function finds a PCI device by scanning the specified bus, dev, func
 *and tries to match vendor and device ids with the ones given to it as
 *parameters.
 *
 *  Notes: If the bus number is 0xFFFF, then the function searches for that
 *  vendor_id, device_id pair in the whole PCI topology of the system i.e
 *  it goes through all the buses, devices, functions in the system
 *
 * Returns:
 *os_pci_dev_t
 *---------------------------------------------------------------------
 */
os_pci_dev_t os_pci_find_device(
				unsigned short vendor_id,
				unsigned short device_id,
				unsigned short bus,
				unsigned short dev,
				unsigned short func,
				os_pci_dev_t pci_dev)
{
  /* TODO: Right now, Just fallback to pci_find_device_generic
   * But we need to implement this
   */
  return pci_find_device_generic(vendor_id, device_id, pci_dev);
}

int os_pci_get_slot_address(
			    os_pci_dev_t pci_dev,
			    unsigned int *bus,
			    unsigned int *slot,
			    unsigned int *func)
{

  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);

  if(bus) {
    *bus = pdev->bus;
  }
  if(slot) {
    *slot = pdev->slot;
  }
  if(func) {
    *func = pdev->func;
  }
  return 0;
}

int os_pci_read_config_8(
			 os_pci_dev_t pci_dev,
			 unsigned long offset,
			 unsigned char* val
			 )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  EMGD_ASSERT(val, "Invalid pointer", 0);
  return pci_read_config_byte(pdev->dev, offset, val);
}

int os_pci_read_config_16(
			  os_pci_dev_t pci_dev,
			  unsigned long offset,
			  unsigned short* val
			  )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  EMGD_ASSERT(val, "Invalid pointer", 0);
  return pci_read_config_word(pdev->dev, offset,val);
}

int os_pci_read_config_32(
			  os_pci_dev_t pci_dev,
			  unsigned long offset,
			  unsigned long* val
			  )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  EMGD_ASSERT(val, "Invalid pointer", 0);
  return pci_read_config_dword(pdev->dev, offset, (u32*)val);
}

int os_pci_write_config_8(
			  os_pci_dev_t pci_dev,
			  unsigned long offset,
			  unsigned char val
			  )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  return pci_write_config_byte(pdev->dev, offset, val);
}

int os_pci_write_config_16(
			   os_pci_dev_t pci_dev,
			   unsigned long offset,
			   unsigned short val
			   )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  return pci_write_config_word(pdev->dev, offset, val);
}

int os_pci_write_config_32(
			   os_pci_dev_t pci_dev,
			   unsigned long offset,
			   unsigned long val
			   )
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
  EMGD_ASSERT(pdev, "Invalid pci device", 0);
  return pci_write_config_dword(pdev->dev, offset, val);
}

int os_pci_disable_legacy_vga_decoding(
			os_pci_dev_t pci_dev
		)
{
#if defined(CONFIG_VGA_ARB)
	linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;
	EMGD_ASSERT(pdev, "Invalid pci device", 0);

	vga_set_legacy_decoding(pdev->dev, VGA_RSRC_NONE);
#else
	/* Noop if the VGA arbiter isn't compiled into the kernel */
#endif

	return 0;
}


void os_pci_free_device(
			os_pci_dev_t pci_dev
			)
{
  linuxkernel_pci_t *pdev = (linuxkernel_pci_t *)pci_dev;

  // Release the lock on our PCI device struct
  pci_dev_put(pdev->dev);

  // Free our local structure.
  OS_FREE(pdev);

  return;
}


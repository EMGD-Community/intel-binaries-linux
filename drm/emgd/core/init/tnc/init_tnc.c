/*
 *-----------------------------------------------------------------------------
 * Filename: init_tnc.c
 * $Revision: 1.24 $
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

#define MODULE_NAME hal.init

#include <io.h>
#include <pci.h>
#include <memmap.h>
#include <gart.h>

#include <memory.h>
#include <igd.h>
#include <igd_errno.h>
#include <igd_init.h>
#include <igd_gart.h>

#include <context.h>
#include <intelpci.h>
#include <general.h>

#include <tnc/regs.h>
#include <tnc/context.h>
#include <tnc/igd_tnc_wa.h>

#include "../cmn/init_dispatch.h"
#include "/usr/include/linux/pci_regs.h"

/*!
 * @addtogroup core_group
 * @{
 */

static int bus_master_enable_tnc(platform_context_tnc_t *platform_context);
static int full_config_vga_tnc(igd_context_t *context,
	init_dispatch_t *dispatch);
static int get_stolen_mem_tnc(igd_context_t *context, unsigned long *pages);

int full_get_param_tnc(igd_context_t *context,
	unsigned long id,
	unsigned long *value);


/*!
 *
 * @param context
 * @param dispatch
 *
 * @return 1 on failure
 * @return 0 on success
 */
int full_config_tnc(igd_context_t *context,
	init_dispatch_t *dispatch)
{
	unsigned long reserved_mem;
	platform_context_tnc_t *platform_context;
	int ret;

	EMGD_TRACE_ENTER;

	platform_context = (platform_context_tnc_t *)context->platform_context;

	_sgx_base = 0x80000;
	_msvdx_base = 0x90000;
	_topaz_base = 0xA0000;

	/*
	 * Enable bus mastering for platforms whose BIOS did not perform this
	 * task for us.
	 */
	ret = bus_master_enable_tnc(platform_context);
	if(ret) {
		EMGD_ERROR("Error: Enabling bus master");
	}

	/* Config VGA */
	ret = full_config_vga_tnc(context, dispatch);
	if(ret) {
		EMGD_ERROR_EXIT("Config VGA Failed");
		return ret;
	}

	get_stolen_mem_tnc(context, &context->device_context.reserved_mem);

	/* Get mem reservation param if it exists */
	if(!full_get_param_tnc(context, IGD_PARAM_MEM_RESERVATION,
			&reserved_mem)) {
		context->device_context.reserved_mem = reserved_mem;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param platform_context
 *
 * @return -1 on failure
 * @return 0 on success
 */
/*
 */
static int bus_master_enable_tnc(platform_context_tnc_t *platform_context){
	int ret;
	unsigned char tmp, tmp_sdvo;

	EMGD_TRACE_ENTER;

	ret = OS_PCI_READ_CONFIG_8(platform_context->pcidev0, PCI_COMMAND_MASTER, &tmp);
	if(ret) {
		EMGD_ERROR_EXIT("PCI read of bus master");
		return -1;
	}

	/*
	 * Get Bit 2, 1, and 0 and see if it is == 1
	 * all 3 bits has to be enabled. This is to enable register read/write
	 * in the case of a PCI card being added
	 */
	if((tmp & 0x7) != 0x7 ) {

		tmp |= 0x7;
		ret = OS_PCI_WRITE_CONFIG_8(platform_context->pcidev0,PCI_COMMAND_MASTER, tmp);
		if(ret) {
			EMGD_ERROR_EXIT("PCI write of bus master");
			return -1;
		}
	}


	if (platform_context->pcidev1) {
		ret = OS_PCI_READ_CONFIG_8(platform_context->pcidev1,PCI_COMMAND_MASTER, &tmp_sdvo);
		if(ret) {
			EMGD_ERROR_EXIT("PCI read of bus master");
			return -1;
		}

		if((tmp_sdvo & 0x7) != 0x7 ) {
			tmp_sdvo |= 0x7;
			ret = OS_PCI_WRITE_CONFIG_8(platform_context->pcidev1,
				PCI_COMMAND_MASTER,tmp_sdvo);
			if(ret) {
				EMGD_ERROR_EXIT("PCI write of bus master");
				return -1;
			}
		}
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param context
 * @param dispatch
 *
 * @return -IGD_ERROR_NODEV on failure
 * @return 0 on success
 */
static int full_config_vga_tnc(igd_context_t *context,
	init_dispatch_t *dispatch)
{
	platform_context_tnc_t *platform_context =
		(platform_context_tnc_t *)context->platform_context;

	EMGD_TRACE_ENTER;

	/*
	 * Map the Device 2 MMIO register. These registers are similar to LNC
	 * and is located in the Atom E6xx Hard Macro
	 */
	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev0,
			TNC_PCI_MMADR, (void*)&context->device_context.mmadr)) {
		EMGD_ERROR_EXIT("Reading MMADR");
		return -IGD_ERROR_NODEV;
	}

	context->device_context.mmadr &= 0xfffffff9;
	context->device_context.virt_mmadr =
		OS_MAP_IO_TO_MEM_NOCACHE(context->device_context.mmadr, TNC_D2_MMIO_SIZE);

	if (!context->device_context.virt_mmadr) {
		EMGD_ERROR_EXIT("Failed to map MMADR");
		return -IGD_ERROR_NODEV;
	}

	EMGD_DEBUG("mmadr mapped %dKB @ (phys):0x%lx  (virt):%p",
		TNC_D2_MMIO_SIZE/1024,
		context->device_context.mmadr,
		context->device_context.virt_mmadr);


	/*
	 * Map the device 3 MMIO registers. These are Atom E6xx specific registers
	 * located in the Atom E6xx Overlay. On LNC, these registers are part of
	 * the IOH (Langwell)
	 */
	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev1,
			TNC_PCI_MMADR, (void*)&context->device_context.mmadr_sdvo)) {
		EMGD_ERROR_EXIT("Reading MMADR");
		return -IGD_ERROR_NODEV;
	}

	context->device_context.mmadr_sdvo &= 0xfffffff9;
	context->device_context.virt_mmadr_sdvo =
		OS_MAP_IO_TO_MEM_NOCACHE(context->device_context.mmadr_sdvo,
		TNC_D3_MMIO_SIZE);

	if (!context->device_context.virt_mmadr_sdvo) {
		EMGD_ERROR_EXIT("Failed to map MMADR");
		return -IGD_ERROR_NODEV;
	}

	EMGD_DEBUG("sdvo mmadr mapped %dKB @ (phys):0x%lx  (virt):%p",
		TNC_D3_MMIO_SIZE/1024,
		context->device_context.mmadr_sdvo,
		context->device_context.virt_mmadr_sdvo);

	/* Map the STMicro SDVO registers. */
	if(platform_context->stbridgedev) {
		if(OS_PCI_READ_CONFIG_32(platform_context->stbridgedev,
					TNC_PCI_MMADR, (void*)&context->device_context.mmadr_sdvo_st)) {
			EMGD_ERROR_EXIT("Reading MMADR");
			return -IGD_ERROR_NODEV;
		}

		context->device_context.mmadr_sdvo_st &= 0xfffffff9;
		context->device_context.virt_mmadr_sdvo_st =
			OS_MAP_IO_TO_MEM_NOCACHE(context->device_context.mmadr_sdvo_st,
					TNC_ST_SDVO_MMIO_SIZE);

		if (!context->device_context.virt_mmadr_sdvo_st) {
			EMGD_ERROR_EXIT("Failed to map MMADR");
			return -IGD_ERROR_NODEV;
		}

		EMGD_DEBUG("STMicro sdvo mmadr mapped %dKB @ (phys):0x%lx  (virt):%p",
				TNC_ST_SDVO_MMIO_SIZE/1024,
				context->device_context.mmadr_sdvo_st,
				context->device_context.virt_mmadr_sdvo_st);

		/* Map the STMicro GPIO registers. */
		if(OS_PCI_READ_CONFIG_32(platform_context->stgpiodev,
					TNC_PCI_MMADR, (void*)&context->device_context.mmadr_sdvo_st_gpio)) {
			EMGD_ERROR_EXIT("Reading MMADR");
			return -IGD_ERROR_NODEV;
		}

		context->device_context.mmadr_sdvo_st_gpio &= 0xfffffff9;
		context->device_context.virt_mmadr_sdvo_st_gpio =
			OS_MAP_IO_TO_MEM_NOCACHE(context->device_context.mmadr_sdvo_st_gpio,
					TNC_ST_SDVO_MMIO_SIZE);

		if (!context->device_context.virt_mmadr_sdvo_st_gpio) {
			EMGD_ERROR_EXIT("Failed to map MMADR");
			return -IGD_ERROR_NODEV;
		}

		EMGD_DEBUG("STMicro sdvo gpio mmadr mapped %dKB @ (phys):0x%lx  (virt):%p",
				TNC_ST_SDVO_MMIO_SIZE/1024,
				context->device_context.mmadr_sdvo_st_gpio,
				context->device_context.virt_mmadr_sdvo_st_gpio);
	}

	/* Map the GPIO BAR. Provides the 64 bytes of I/O space for GPIO
	 * BAR is defined by bits 15:6 */

	if(OS_PCI_READ_CONFIG_16(platform_context->lpc_dev,
			TNC_PCI_GBA, (void*)&context->device_context.gpio_bar)) {

	}
	context->device_context.gpio_bar |= (1L<<31);
	/* Enable the decode of IO Range ppointed to by the BA */
	if(OS_PCI_WRITE_CONFIG_32(platform_context->lpc_dev,
		TNC_PCI_GBA, context->device_context.gpio_bar)) {
		EMGD_ERROR_EXIT("Writing LPC GPIO BAR Enable");
	}

	/* read the GPIO BAR (OFFSET 44:47) */
	if(OS_PCI_READ_CONFIG_16(platform_context->lpc_dev,
			TNC_PCI_GBA, (void*)&context->device_context.gpio_bar)) {
		EMGD_ERROR_EXIT("Reading LPC GPIO BAR");
		/* We cannot read the GPIO BAR. It is a problem but we can go on with init
		 * return with NO ERROR*/
		return 0;
	}

	context->device_context.gpio_bar &= 0xffc0;

	context->device_context.virt_gpio_bar =
		OS_MAP_IO_TO_MEM_NOCACHE(
		context->device_context.gpio_bar, 64);

	if (!context->device_context.virt_gpio_bar) {
		EMGD_ERROR_EXIT("Failed to map LPC GPIO BAR");
		return -IGD_ERROR_NODEV;
	}

	EMGD_DEBUG("GPIO mapped %dKB @ (phys):0x%lx  (virt):%p",
		64,
		context->device_context.gpio_bar,
		context->device_context.virt_gpio_bar);

	/* PCI Interrupt Line */
	if(OS_PCI_READ_CONFIG_8(platform_context->pcidev0,
			PCI_INTERRUPT_LINE, (void*)&platform_context->irq)) {
		platform_context->irq = 0;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * Get the # of pages used for video memory. This does not use information from
 * the scratch register, since this is done later if it exists.
 *
 * @param context
 * @param pages
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
static int get_stolen_mem_tnc(igd_context_t *context, unsigned long *pages)
{
	platform_context_tnc_t *platform_context;
	os_pci_dev_t           vga_dev;
	unsigned short         gmch_ctl;
	unsigned long          stolen_mem; /* in bytes */
	int ret;

	EMGD_TRACE_ENTER;

	platform_context = (platform_context_tnc_t *)context->platform_context;
	vga_dev = platform_context->pcidev0;

	ret = OS_PCI_READ_CONFIG_16(vga_dev, TNC_PCI_GC, &gmch_ctl);
	if (ret) {
		EMGD_ERROR_EXIT("Unable to read TNC_PCI_GC");
		return -IGD_ERROR_INVAL;
	}

	switch (gmch_ctl & 0x70) {
	case 0x00:
		stolen_mem = 0;
		break;
	case 0x10:
		/* 1M */
		stolen_mem = 1*1024*1024;
		break;
	case 0x20:
		/* 4M */
		stolen_mem = 4*1024*1024;
		break;
	case 0x30:
		/* 8M */
		stolen_mem = 8*1024*1024;
		break;
	case 0x40:
		/* 16M */
		stolen_mem = 16*1024*1024;
		break;
	case 0x50:
		/* 32M */
		stolen_mem = 32*1024*1024;
		break;
	case 0x60:
		/* 48M */
		stolen_mem = 48*1024*1024;
	case 0x70:
		/* 64M */
		stolen_mem = 64*1024*1024;
		break;
	default:
		EMGD_ERROR_EXIT("Unknown Stolen Memory Size");
		return -IGD_ERROR_INVAL;
	}

	if (stolen_mem) {
		/*
		 * Subtract off the size of the GTT which is
		 * (number of entries in DWORDS) * 4 to get it into bytes
		 */
		stolen_mem -= context->device_context.gatt_pages*4;
		/* Subtract off 1 page for the scratch page */
		stolen_mem -= 4*1024;
	}

	/* Convert to the # of pages available for stolen memory */
	*pages = stolen_mem / 4096;

	EMGD_DEBUG("Stolen memory: 0x%lx pages", *pages);

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param context
 * @param id
 * @param value
 *
 * @return -IGD_ERROR_INVAL on failure
 * @return 0 on success
 */
int full_get_param_tnc(igd_context_t *context,
	unsigned long id,
	unsigned long *value)
{
	int ret = 0;
	unsigned char *mmio;
	unsigned long control_reg;

	EMGD_TRACE_ENTER;

	EMGD_DEBUG("ID: 0x%lx", id);

	/* Scratch registers used as below:
	 *
	 * 0x71410:
	 * --------
	 *   Bits 31-16 - EID Firmware identifier 0xE1DF
	 *   Bits 15-00 - Tell what data is present.
	 * Here are bits for what we are using know:
	 *   Bit 0 - Panel id
	 *   Bit 1 - List of ports for which displays are attached
	 *   Bit 2 - Memory reservation
	 * If any of the above bits is set that mean data is followed
	 * in the next registers.
	 *
	 * 0x71414:
	 * --------
	 *   Bits 07-00 - Panel Id
	 *   Bits 11-08 - Port list
	 * Information for Port list: If any of the bit is set means,
	 *   a display is attached to that port as follows:
	 *    Bit 08 - CRT
	 *    Bit 09 - DVOA/Internal LVDS
	 *    Bit 10 - DVOB/RGBA
	 *    Bit 11 - DVOC
	 *
	 * 0x71418:
	 * --------
	 *	 Bits 15-00 - Reserved Memory value in number of 4k size pages
	 */
	mmio = context->device_context.virt_mmadr;
	control_reg = EMGD_READ32(EMGD_MMIO(mmio) + 0x71410);
	*value = 0;

	switch(id) {
	case IGD_PARAM_PANEL_ID:
		/*
		 * Check for Embedded firmware
		 */
		if ((control_reg>>16) != 0xE1DF) {
			EMGD_DEBUG("No Embedded vBIOS found");
			EMGD_TRACE_EXIT;
			return -IGD_ERROR_INVAL;
		}

		/*
		 * The panel id bit must be set in the control register
		 * to indicate valid panel (config) ID value.
		 */
		if (control_reg & 0x1) {
			*value = EMGD_READ32(EMGD_MMIO(mmio) + 0x71414) & 0xFF;
			if(!(*value)) {
				/* we cannot allow for config id = 0 */
				ret = -IGD_ERROR_INVAL;
			}
		} else {
			EMGD_DEBUG("Panel ID read failed: Incorrect Operation");
			ret = -IGD_ERROR_INVAL;
		}
		break;
	case IGD_PARAM_MEM_RESERVATION:
		/*
		 * Check for Embedded firmware
		 */
		if ((control_reg>>16) != 0xE1DF) {
			EMGD_DEBUG("No Embedded vBIOS found");
			EMGD_TRACE_EXIT;
			return -IGD_ERROR_INVAL;
		}

		/*
		 * The mem reservation bit must be set in the control register
		 * to indicate valid mem reservation value.
		 */
		if (control_reg & 0x4) {
			*value = (EMGD_READ32(EMGD_MMIO(mmio) + 0x71418) & 0xFFFF);
		} else {
			EMGD_DEBUG("Mem Reservation read failed: Incorrect Operation");
			ret = -IGD_ERROR_INVAL;
		}
		break;
	default:
		ret = -IGD_ERROR_INVAL;
		break;
	}

	EMGD_TRACE_EXIT;
	return ret;
}

/*!
 *
 * @param context
 *
 * @return void
 */
void full_shutdown_tnc(igd_context_t *context)
{
	platform_context_tnc_t *platform_context =
		(platform_context_tnc_t *)context->platform_context;

	EMGD_TRACE_ENTER;

	/* unmap registers */
	if(context->device_context.virt_mmadr) {
		EMGD_DEBUG("Unmapping Gfx registers and GTT Table...");
		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_mmadr,
			TNC_D2_MMIO_SIZE);
		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_mmadr_sdvo,
			TNC_D3_MMIO_SIZE);
		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_mmadr_sdvo_st,
			TNC_ST_SDVO_MMIO_SIZE);
		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_mmadr_sdvo_st_gpio,
			TNC_ST_SDVO_MMIO_SIZE);
		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_gpio_bar, 64);

		OS_UNMAP_IO_FROM_MEM((void *)context->device_context.virt_gttadr,
			context->device_context.gatt_pages * 4);
	} else {
		printk(KERN_ERR "Unmapping MMIO space failed.\n");
	}

	if (platform_context) {
		OS_PCI_FREE_DEVICE(platform_context->pcidev0);
		OS_PCI_FREE_DEVICE(platform_context->pcidev1);
		OS_PCI_FREE_DEVICE(platform_context->bridgedev);
		if (platform_context->lpc_dev) {
			OS_PCI_FREE_DEVICE(platform_context->lpc_dev);
		}
	}
	EMGD_TRACE_EXIT;
}


 int query_2d_caps_hwhint_tnc(
  	         igd_context_t *context,
  	         unsigned long caps_val,
  	         unsigned long *status)
{
	platform_context_tnc_t *platform_context;
	  	 
	EMGD_TRACE_ENTER;

 
	platform_context = (platform_context_tnc_t *)context->platform_context;
 	 	 
	/*
	* Only 2D BLT capability query is currently implemented.
	* This is required by TNC B0 flickering workaround.
	* Query for other 2D capability can be implemented
	* in the future if neeeded.
	*/
	switch (caps_val) {
		case IGD_2D_CAPS_BLT:
			if((context->device_context.did == PCI_DEVICE_ID_VGA_TNC) &&
				(context->device_context.rid == TNC_B0_RID) &&
				(platform_context->tnc_dev3_rid == TNC_B0_DEV3_RID)) {
					*status = IGD_2D_HW_DISABLE;                    
					/* disable 2d blt hardware acceleration for TNC B0 */
			} else {
					*status = IGD_2D_HW_ENABLE;                             
					/* enable 2d blt hardware acceleration */  	
			}
		break;
  	 
		default:
			*status = IGD_2D_CAPS_UNKNOWN;                          
			/* Unknown 2d capability to query */
			EMGD_ERROR("2D caps to query is unknown!");
		break;
	}
  	 
	EMGD_TRACE_EXIT;
	return 0;
}


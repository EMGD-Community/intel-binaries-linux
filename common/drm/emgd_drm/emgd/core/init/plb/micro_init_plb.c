/*
 *-----------------------------------------------------------------------------
 * Filename: micro_init_plb.c
 * $Revision: 1.13 $
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

#include <igd.h>
#include <igd_errno.h>
#include <igd_init.h>

#include <context.h>
#include <intelpci.h>
#include <general.h>
#include <utils.h>

#include <igd_debug.h>
#include <drmP.h>
#include <memory.h>
#include <asm/cacheflush.h>

#include <plb/regs.h>
#include <plb/context.h>

#include "user_config.h"
#include "../cmn/init_dispatch.h"

/*!
 * @addtogroup core_group
 * @{
 */

#define PFX "EMGD: "

#define SCR1    0x71410 /* scratch register set by vbios indicating status*/
#define SCR2    0x71418 /* scratch register set by vbios indicating amount of stolen memory */
#define FW_ID   0xE1DF0000 /* firmware identifier */
#define ST_BIT  0x00000004 /* bit2- stolen memory bit */
#define PSB_GMCH_CTRL       0x52
#define PSB_GMCH_ENABLED    0x04
#define PSB_PGETBL_CTL      0x00002020
#define PSB_PGETBL_ENABLED  0x00000001
#define PSB_GATT_RESOURCE   2
#define PSB_GTT_RESOURCE    3
#define PSB_BSM             0x5c
#define PSB_PTE_VALID       0x0001


#ifdef CONFIG_PLB

extern unsigned char io_mapped;
extern unsigned short io_base;
extern emgd_drm_config_t config_drm;

extern int full_config_plb(igd_context_t *context,
	init_dispatch_t *dispatch);
extern int get_revision_id_plb(igd_context_t *context, os_pci_dev_t vga_dev);
extern int full_get_param_plb(igd_context_t *context, unsigned long id,
	unsigned long *value);
extern void full_shutdown_plb(igd_context_t *context);

static int query_plb(igd_context_t *context,init_dispatch_t *dispatch,
	os_pci_dev_t vga_dev, unsigned int *bus, unsigned int *slot,
	unsigned int *func);
static int config_plb(igd_context_t *context,
	init_dispatch_t *dispatch);
static int set_param_plb(igd_context_t *context, unsigned long id,
	unsigned long value);
static int get_param_plb(igd_context_t *context, unsigned long id,
	unsigned long *value);
static void shutdown_plb(igd_context_t *context);

static void gtt_shutdown_plb(igd_context_t *context);
static void gtt_init_plb(igd_context_t *context);


static platform_context_plb_t platform_context_plb;

/* Graphics frequency list. This is valid for pouslbo only. This value is obtained
 * From the Cspec - SCH Message Network-Port 5*/
static unsigned short plb_gfx_freq_list[] =
{
	100, 133, 150, 178, 200, 266, 0, 0
};

init_dispatch_t init_dispatch_plb = {
	"Intel SCH US15 Chipset",
	"US15",
	"lvds",
	query_plb,
	config_plb,
	set_param_plb,
	get_param_plb,
	shutdown_plb,
	NULL
};



/*
 * GTT shutdown.
 *
 * Unmap the GTT mapping that was done during init time.
 */
static void gtt_shutdown_plb(igd_context_t *context)
{
	if (context->device_context.virt_gttadr) {
		iounmap(context->device_context.virt_gttadr);

		context->device_context.virt_gttadr = NULL;
	}
}


/*
 * Initialize the GTT.
 *   - Find the size of stolen memory
 *   - Add stolen memory to the GTT
 *   - Map the GTT and video memory
 */

static void gtt_init_plb(igd_context_t *context)
{
	struct drm_device *dev;
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long dvmt_mode = 0;
	unsigned long gtt_pages = 0;
	unsigned long stolen_mem_size = 0;
	unsigned long scratch;
	unsigned long base;
	unsigned long pte;
	unsigned short gmch_ctl;
	unsigned long pge_ctl;
	unsigned long gtt_phys_start;
	unsigned long gatt_start;
	unsigned long gatt_pages;
	unsigned long gtt_start;
	unsigned long gtt_order;
	unsigned long stolen_mem_base;
	unsigned long *gtt_table;
	int gtt_enabled = FALSE;
	struct page *gtt_table_page;
	int i;

	dev = (struct drm_device *)context->drm_dev;

	/* Enable the GMCH */
	OS_PCI_READ_CONFIG_16((os_pci_dev_t)dev->pdev, PSB_GMCH_CTRL,
			&gmch_ctl);
	OS_PCI_WRITE_CONFIG_16((os_pci_dev_t)dev->pdev, PSB_GMCH_CTRL,
			(gmch_ctl | PSB_GMCH_ENABLED));
	context->device_context.gmch_ctl = gmch_ctl;

	/* Get the page table control register */
	pge_ctl = readl(mmio + PSB_PGETBL_CTL);
	gtt_phys_start = pge_ctl & PAGE_MASK;

	gtt_enabled = pge_ctl & PSB_PGETBL_ENABLED;

	/* Create a scratch page to initialize empty GTT entries */
	context->device_context.scratch_page = alloc_page(GFP_DMA32 | __GFP_ZERO);

	/*
	* Is pci_resource_start(dev->pdev, PSB_GATT_RESOURCE); the same
	* as pci_read_config_dword(dev->pdev, 0x1C, &value)?
	*
	* PSB_GATT_RESOURCE length is the amount of memory addressable
	* by the GTT table.
	*/
	gatt_start = pci_resource_start(dev->pdev, PSB_GATT_RESOURCE);
	gatt_pages = (pci_resource_len(dev->pdev, PSB_GATT_RESOURCE) >> PAGE_SHIFT);
	context->device_context.gatt_pages = gatt_pages;

	/*
	 * The GTT wasn't set up by the vBios
	 */
	if (!gtt_enabled) {
		context->device_context.stolen_pages = 0;

		gtt_pages = pci_resource_len(dev->pdev, PSB_GTT_RESOURCE) >> PAGE_SHIFT;
		gtt_order = get_order(gtt_pages << PAGE_SHIFT);
		gtt_table = (unsigned long *)__get_free_pages(GFP_KERNEL, gtt_order);
		/* Make sure allocation was successful */
		if (NULL == gtt_table) {
			EMGD_ERROR("Failed to allocate kernel pages for GTT");
			return;
		}
		context->device_context.virt_gttadr = gtt_table;

		for (i=0; i < (1 << gtt_order); i++) {
			gtt_table_page = virt_to_page(gtt_table + (PAGE_SIZE * i));
			EMGD_DEBUG("Setting reserved bit on %p", gtt_table_page);
			set_bit(PG_reserved, &gtt_table_page->flags);
		}

		gtt_phys_start = virt_to_phys(gtt_table);

		for (i = 0; i < gtt_pages; i++) {
			gtt_table[i] = (unsigned long)context->device_context.scratch_page;
		}

		printk(KERN_INFO "Detected GTT was not enabled by firmware");
		printk(KERN_INFO "GMMADR(region 0) start: 0x%08lx (%ldM).\n",
			gatt_start, (gatt_pages / 256));
		printk(KERN_INFO "GTTADR(region 3) start: 0x%08lx (can map %ldM RAM), and "
			"actual RAM base 0x%08lx.\n",
			(unsigned long)gtt_table, (gtt_pages * 4), gtt_phys_start);

		/* Enable the newly created GTT */
		EMGD_DEBUG("Enabling new GTT");
		writel((gtt_phys_start|PSB_PGETBL_ENABLED), mmio + PSB_PGETBL_CTL);
		pge_ctl = readl(mmio + PSB_PGETBL_CTL);

	} else {

		/*
		 * Get the start address of the GTT page table
		 *
		 * In full_config_vga, this is done differently.  The address is read
		 * from pcidev0's pci config space, at TNC_PCI_GTTADR and the size comes
		 * from TNC_OFFSET_VGA_MSAC. The value read for size is a size id
		 *    1 = 128, 2 = 256, 3 = 512
		 * emgd_gtt->gtt_start = OS_PCI_READ_CONFIG_32(
		 *            context->platform_context->pcidev0, TNC_PCI_GTTADDR)
		 * gtt_pages = OS_PCI_READ_CONFIG_8(context->platform_context->pcidev0,
		 *            TNC_OFFSET_VGA_MSAC) * 1024;
		 *
		 * PSB_GTT_RESOURCE length is the size of the GTT table. Thus,
		 * gtt_pages is the number of pages that make up the table.
		 */
		gtt_start = pci_resource_start(dev->pdev, PSB_GTT_RESOURCE);
		gtt_pages = (pci_resource_len(dev->pdev, PSB_GTT_RESOURCE) >> PAGE_SHIFT);

		/* Get stolen memory configuration. */
		pci_read_config_dword(dev->pdev, PSB_BSM, (u32 *)&stolen_mem_base);
		stolen_mem_size = gtt_phys_start - stolen_mem_base - PAGE_SIZE;

		/* Display useful information in the kernel log */
		printk(KERN_INFO "GMMADR(region 0) start: 0x%08lx (%ldM).\n",
				gatt_start, (gatt_pages / 256));
		printk(KERN_INFO "GTTADR(region 3) start: 0x%08lx (can map %ldM RAM), and "
				"actual RAM base 0x%08lx.\n",
				gtt_start, (gtt_pages * 4), gtt_phys_start);
		printk(KERN_INFO "Stolen memory information \n");
		printk(KERN_INFO "       base in RAM: 0x%lx \n", stolen_mem_base);
		printk(KERN_INFO "       size: %luK, calculated by (GTT RAM base) - "
				"(Stolen base)\n", (stolen_mem_size / 1024));
		dvmt_mode = (gmch_ctl >> 4) & 0x7;
		printk(KERN_INFO "       size: %dM (dvmt mode=%ld)\n",
				(dvmt_mode == 1) ? 1 : (2 << (dvmt_mode - 1)), dvmt_mode);

		context->device_context.virt_gttadr =
		ioremap_nocache(gtt_start, gtt_pages << PAGE_SHIFT);

		if (!context->device_context.virt_gttadr) {
			printk(KERN_ERR "Failed to map the GTT.\n");
			/* TODO: Clean up somelthing here */
			return;
		}

		/* Insert stolen memory pages into the beginning of GTT */
		base = stolen_mem_base >> PAGE_SHIFT;
		context->device_context.stolen_pages = stolen_mem_size >> PAGE_SHIFT;

		printk(KERN_INFO "Set up %ld stolen pages starting at 0x%08lx, "
			"GTT offset %dK\n", context->device_context.stolen_pages, base, 0);

		for (i = 0; i < context->device_context.stolen_pages; i++) {
			pte = ((base + i) << PAGE_SHIFT) | PSB_PTE_VALID;
			writel(pte, context->device_context.virt_gttadr + i);
		}

	}

	/* Update the scratch registers to say we have no stolen memory */
	scratch = readl(mmio + SCR1);
	if ((scratch & FW_ID) == FW_ID) {
		/* if an EMGD vBios modify only the stolen memory bit */
		scratch |= ST_BIT;
		writel(scratch, mmio + SCR1);
	} else {
		/* Not an EMGD vBios so just set the entire register to a known value */
		writel((FW_ID|ST_BIT), mmio + SCR1);
	}

	/*
	 * Report back that there is 0MB of stolen memory regardless of
	 * what was really in there.  Fresh pages will be inserted over
	 * the top of the existing stolen memory.
	 */
	writel(0, mmio + SCR2);

	/*
	 * FIXME: Shouldn't this fill in all the GTT page table entries with
	 * the scratch page?
	 */

	return;
}



/*!
 *
 * @param context
 * @param dispatch
 * @param vga_dev
 * @param bus
 * @param slot
 * @param func
 *
 * @return -IGD_ERROR_NODEV on failure
 * @return 0 on success
 */
static int query_plb(
	igd_context_t *context,
	init_dispatch_t *dispatch,
	os_pci_dev_t vga_dev,
	unsigned int *bus,
	unsigned int *slot,
	unsigned int *func)
{
	platform_context_plb_t *platform_context = &platform_context_plb;

	EMGD_TRACE_ENTER;

	context->platform_context = (void *)&platform_context_plb;

	OS_PTHREAD_MUTEX_INIT(&platform_context_plb.flip_mutex, NULL);

	/*
	 * Current specs indicate that PLB has only one PCI function.
	 * If this changes then we need to make sure we have func 0
	 * here as in previous chips.
	 */
	platform_context->pcidev0 = vga_dev;

	OS_PCI_GET_SLOT_ADDRESS(vga_dev, bus, slot, func);

	OPT_MICRO_CALL(get_revision_id_plb(context, vga_dev));

	/*
	 * Read BSM.
	 * This must be in query so it is available early for the vBIOS.
	 */
	if(OS_PCI_READ_CONFIG_32(vga_dev,
			PLB_PCI_BSM, &context->device_context.fb_adr)) {
		EMGD_ERROR_EXIT("Reading BSM");
		return -IGD_ERROR_NODEV;
	}
	context->device_context.fb_adr &= 0xFFFFF000;

	EMGD_DEBUG("BSM (High)@: 0x%lx, (Low) 0x%4lx", (context->device_context.fb_adr >> 16), context->device_context.fb_adr);

	/*
	 * Read IO Base.
	 * This must be in query so it is available early for the vBIOS.
	 */
	if(OS_PCI_READ_CONFIG_16(vga_dev, PLB_PCI_IOBAR, &io_base)) {
		EMGD_ERROR_EXIT("Reading IO Base");
		return -IGD_ERROR_NODEV;
	}
	io_base &= 0xfffe;
	EMGD_DEBUG("io @: 0x%x", io_base);

	/* Gen4 is always io_mapped */
	io_mapped = 1;

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
static int config_plb(igd_context_t *context,
	init_dispatch_t *dispatch)
{
	unsigned long coreclk;
	unsigned long graphics_frequency;

	platform_context_plb_t *platform_context =
		(platform_context_plb_t *)context->platform_context;

	EMGD_TRACE_ENTER;

	OPT_MICRO_CALL(full_config_plb(context, dispatch));

	/* If KMS is set, we need to unset it as KMS is not supported on PLB */
	config_drm.kms = 0;

	/* Set the Max Dclock */
	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev0,
			INTEL_OFFSET_VGA_CORECLK, &coreclk)) {
		EMGD_ERROR_EXIT("PCI Read of VGA Core Clock");
		return -IGD_ERROR_NODEV;
	}

	/* Get graphics frequency param if it exists */
	if(!get_param_plb(context, IGD_PARAM_GFX_FREQ,
			&graphics_frequency)) {
		context->device_context.gfx_freq = (unsigned short)graphics_frequency;
	}

	/*
	 * FIXME:
	 *  Coreclk register above is used to determine some clocking information
	 *  there is also a fuse to limit the dclk. More research needed.
	 */
	context->device_context.max_dclk = 762000;

	gtt_init_plb(context);

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
static int get_param_plb(igd_context_t *context, unsigned long id,
	unsigned long *value)
{
	int ret = 0;
	unsigned long control_reg;
	os_pci_dev_t bridge_dev = (os_pci_dev_t)0;

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
	 * Bits 15-00 - Reserved Memory value in number of 4k size pages
	 */
	*value = 0;

	switch(id) {
	case IGD_PARAM_PORT_LIST:

		control_reg = EMGD_READ32(EMGD_MMIO(context->device_context.virt_mmadr) +
			0x71410);

		/*
		 * Verify that the Embedded Firware is present.
		 */
		if ((control_reg>>16) != 0xE1DF) {
			EMGD_DEBUG("Exit No Embedded vBIOS found");
			EMGD_TRACE_EXIT;
			return -IGD_ERROR_INVAL;
		}

		/*
		 * If the port list bit is set in control register,
		 * read the port list
		 */
		if (control_reg & 0x2) {
			unsigned char temp;
			int i = 0;

			temp = (unsigned char)((EMGD_READ32(EMGD_MMIO(context->device_context.virt_mmadr) + 0x71414)>>8) & 0xFF);
			EMGD_DEBUG("Connected Port Bits: 0x%x", temp);

			/*
			 * The meanings of bits in temp were dictated by VBIOS
			 * and should not change due to backward compatibility
			 * with Legacy VBIOS
			 */
			if (temp & 0x01) {
				/* Analog port */
				value[i++] = 5;
			}
			if (temp & 0x02) {
				/* Internal LVDS port */
				value[i++] = 4;
			}
			if (temp & 0x04) {
				/* DVOB Port */
				value[i++] = 2;
			}
		} else {
			EMGD_DEBUG("Port List read failed: Incorrect Operation");
			ret = -IGD_ERROR_INVAL;
		}
		break;
	case IGD_PARAM_GFX_FREQ:
		/* Query register values from the bridge. This method uses the Poulsbo
		 * SCH Message Network. Setting offset 0xD0 in the host bridge config
		 * register sends an opcode to the Message Network. Reading register 0xD4
		 * from the host bridge config register will get the return value of the
		 * sent opcode.
		 *
		 * This feature is for Pouslbo Only */

		bridge_dev = OS_PCI_FIND_DEVICE(
				PCI_VENDOR_ID_INTEL,
				PCI_DEVICE_ID_BRIDGE_PLB,
				0xFFFF, /* Scan the whole PCI bus */
				0,
				0,
				(os_pci_dev_t)0);

		if(!bridge_dev) {
			EMGD_ERROR_EXIT("Bridge device NOT found.");
			return -IGD_ERROR_INVAL;
		}
		/* write into the Message Control Register (MCR)
		 * [INPUT] should contain the formatted opcode
		 * that needs to be sent into the MCR */
		ret = OS_PCI_WRITE_CONFIG_32(bridge_dev, 0xD0,
			(0xD0<<24)/*opcode*/|(5<<16)/*port*/|(3<<8)/*reg*/|(0xF<<4));
		if(ret) {
			EMGD_ERROR("Writing into the MCR Failed");
			return -IGD_ERROR_INVAL;
		}
		/* read from the Message Data Register (MDR) */
		if(OS_PCI_READ_CONFIG_32(bridge_dev, 0xD4,
			(void*) &control_reg)) {
			EMGD_ERROR_EXIT("Reading from MDR Failed");
			return -IGD_ERROR_INVAL;
		}
		*value = plb_gfx_freq_list[control_reg & 0x7];
		OS_PCI_FREE_DEVICE(bridge_dev);
		break;
	default:
		/*
		 * If the param is not found here then it may only be in the
		 * full version.
		 */
		OPT_MICRO_CALL_RET(ret, full_get_param_plb(context, id, value));
		break;
	}

	EMGD_TRACE_EXIT;
	return ret;
}

/*!
 * No Settable Params for PLB
 *
 * @param context
 * @param id
 * @param value
 *
 * @return -IGD_ERROR_INVAL
 */
static int set_param_plb(igd_context_t *context, unsigned long id,
	unsigned long value)
{
	return -IGD_ERROR_INVAL;
}

/*!
 *
 * @param context
 *
 * @return void
 */
static void shutdown_plb(igd_context_t *context)
{
	gtt_shutdown_plb(context);

	OPT_MICRO_VOID_CALL(full_shutdown_plb(context));
}

#endif

/*
 *-----------------------------------------------------------------------------
 * Filename: micro_init_tnc.c
 * $Revision: 1.26 $
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

#include <tnc/regs.h>
#include <tnc/context.h>
#include <linux/pci_ids.h>

#include "../cmn/init_dispatch.h"

#define PFX "EMGD: "

#define SCR1	0x71410 /* scratch register set by vbios indicating status*/
#define SCR2	0x71418 /* scratch register set by vbios indicating amount of stolen memory */
#define FW_ID	0xE1DF0000 /* firmware identifier */
#define ST_BIT	0x00000004 /* bit2- stolen memory bit */
#define PSB_GMCH_CTRL       0x52
#define PSB_GMCH_ENABLED    0x04
#define PSB_PGETBL_CTL      0x00002020
#define PSB_GATT_RESOURCE   2
#define PSB_GTT_RESOURCE    3
#define PSB_BSM             0x5c
#define PSB_PTE_VALID       0x0001



/*!
 * @addtogroup core_group
 * @{
 */

#ifdef CONFIG_TNC

extern unsigned char io_mapped;
extern unsigned short io_base;

/* For dev2 [0:2:0] */
extern unsigned char io_mapped_lvds;
extern unsigned short io_base_lvds;

/* For dev3 [0:3:0] */
extern unsigned char io_mapped_sdvo;
extern unsigned short io_base_sdvo;

/* For dev31 [0:31:0] */
extern unsigned char io_mapped_lpc;
extern unsigned short io_base_lpc;

/* For STMicro SDVO [6:0:1] */
extern unsigned char io_mapped_sdvo_st;
extern unsigned short io_base_sdvo_st;
extern unsigned char io_mapped_sdvo_st_gpio;
extern unsigned short io_base_sdvo_st_gpio;

extern int full_config_tnc(igd_context_t *context,
	init_dispatch_t *dispatch);
extern int get_revision_id_tnc(igd_context_t *context, os_pci_dev_t vga_dev, os_pci_dev_t sdvo_dev);
extern int full_get_param_tnc(igd_context_t *context, unsigned long id,
	unsigned long *value);
extern void full_shutdown_tnc(igd_context_t *context);

extern int query_2d_caps_hwhint_tnc(
  	         igd_context_t *context,
  	         unsigned long caps_val,
  	         unsigned long *status);

static int query_tnc(igd_context_t *context,init_dispatch_t *dispatch,
	os_pci_dev_t vga_dev, unsigned int *bus, unsigned int *slot,
	unsigned int *func);
static int config_tnc(igd_context_t *context,
	init_dispatch_t *dispatch);
static int set_param_tnc(igd_context_t *context, unsigned long id,
	unsigned long value);
static int get_param_tnc(igd_context_t *context, unsigned long id,
	unsigned long *value);
static void shutdown_tnc(igd_context_t *context);

static void gtt_shutdown_tnc(igd_context_t *context);
static void gtt_init_tnc(igd_context_t *context);

/* Helper Functions */
static int query_sch_message(unsigned long reg, unsigned long* value);
static int dump_fuse_values(void);

static platform_context_tnc_t platform_context_tnc;

os_pci_dev_t bridge_dev;

init_dispatch_t init_dispatch_tnc = {
	"Intel Atom E6xx Processor",
	"Atom_E6xx",
	"lvds",
	query_tnc,
	config_tnc,
	set_param_tnc,
	get_param_tnc,
	shutdown_tnc,
	query_2d_caps_hwhint_tnc
};

/* Array to keep the Bridge ID. Atom E6xx ULP uses a different bridge ID */
static unsigned short bridge_id[] =
{
	PCI_DEVICE_ID_BRIDGE_TNC,
	PCI_DEVICE_ID_BRIDGE_TNC_ULP,
	0
};

#define SKU_NO 3
#define RATIO_NO 8
/*
 * Atom E6xx GFX frequencies
 * The gfx clock frequencies depends on the board SKU and ratio
 * The table of frequencies can be found in Atom E6xx EAS
 * Chapter: Clocks and Reset Unit
 */
static unsigned short tnc_gfx_freq_list[RATIO_NO][SKU_NO] =
{
	/* rows represent the gfx clock ratio,
	 * columns the sku */

	/* sku_100 sku_100L sku_83 */
	{200,	100,	166}, /*1:1*/
	{266,	133,	222}, /*4:3*/
	{320,	160,	266}, /*8:5*/
	{400,	200,	333}, /*2:1 DEFAULT*/
	{0,		0,		0 }, /*16:7 RSVD*/
	{533,	266,	444}, /*8:3*/
	{640,	320,	553}, /*16:5*/
	{800,	400,	666}	 /*4:1 RSVD*/
};

static unsigned short tnc_core_freq_list[SKU_NO] =
{
/* sku_100 sku_100L sku_83 */
	200,	100,	166
};

/* MCR define */
#define READ_FUS_EFF0			0xD08106F0
#define READ_FUS_EFF1			0xD08107F0
#define READ_FUS_EFF2			0xD08108F0
#define READ_FUS_EFF3			0xD08109F0
#define READ_FUS_EFF4			0xD0810AF0
#define READ_FUS_EFF5			0xD0810BF0



/*
 * GTT shutdown.
 *
 * Unmap the GTT mapping that was done during init time.
 */
static void gtt_shutdown_tnc(igd_context_t *context)
{
	if (context->device_context.virt_gttadr) {
		iounmap(context->device_context.virt_gttadr);

		context->device_context.virt_gttadr = NULL;
	}
	if(context->device_context.scratch_page){
		__free_page(context->device_context.scratch_page);
		context->device_context.scratch_page = NULL;
	}
}


/*
 * Initialize the GTT.
 *   - Find the size of stolen memory
 *   - Add stolen memory to the GTT
 *   - Map the GTT and video memory
 */

static void gtt_init_tnc(igd_context_t *context)
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

	/* Create a scratch page to initialize empty GTT entries */
	if(NULL == context->device_context.scratch_page){
		context->device_context.scratch_page = alloc_page(GFP_DMA32 | __GFP_ZERO);
	}

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
	if (!pge_ctl) {
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
		writel(gtt_phys_start, mmio + PSB_PGETBL_CTL);
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
 * Helper function to query MCR registers
 * @param reg
 * @param value
 *
 * @return -IGD_ERROR_INVAL on error
 * @return 0 on success
 */
static int query_sch_message(unsigned long reg, unsigned long* value){

	platform_context_tnc_t *platform_context = &platform_context_tnc;

	/* Send the opcode into the MCR */
	if(OS_PCI_WRITE_CONFIG_32(platform_context->bridgedev,
		0xD0, reg)){
		EMGD_ERROR_EXIT("Writing into the MCR Failed");
		return -IGD_ERROR_INVAL;
	}

	if(OS_PCI_READ_CONFIG_32(platform_context->bridgedev,
		0xD4, value)) {

		EMGD_ERROR_EXIT("Writing to MDR Failed");
		return -IGD_ERROR_INVAL;
	}

	return 0;
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
static int query_tnc(
	igd_context_t *context,
	init_dispatch_t *dispatch,
	os_pci_dev_t vga_dev,
	unsigned int *bus,
	unsigned int *slot,
	unsigned int *func)
{
	platform_context_tnc_t *platform_context = &platform_context_tnc;

	int i = 0;

	EMGD_TRACE_ENTER;



	/* So we don't have to pollute our function tables with multiple
	 * entries for every variant of TNC, update the device ID if one
	 * of the other SKUs is found
	 */
	context->device_context.did = PCI_DEVICE_ID_VGA_TNC;

	platform_context->did = context->device_context.did;
	context->platform_context = (void *)&platform_context_tnc;

	OS_PTHREAD_MUTEX_INIT(&platform_context_tnc.flip_mutex, NULL);

	/* find and store the bridge dev since we will be using it a lot
	 * in the init modules */
	while(bridge_id[i] != 0){
	platform_context->bridgedev = OS_PCI_FIND_DEVICE(
				PCI_VENDOR_ID_INTEL,
				bridge_id[i],
				0xFFFF, /* Scan the whole PCI bus */
				0,
				0,
				(os_pci_dev_t)0);
		if(platform_context->bridgedev){
			bridge_dev = platform_context->bridgedev;
	        context->device_context.bid = bridge_id[i];
	        break;
		}
		i++;
	 }
	/*
	 * Current specs indicate that Atom E6xx has only one PCI function.
	 * If this changes then we need to make sure we have func 0
	 * here as in previous chips.
	 */
	platform_context->pcidev0 = vga_dev;

	/* find device 3 */
	platform_context->pcidev1 = OS_PCI_FIND_DEVICE(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_SDVO_TNC,
			0,
			3,
			0,
			(os_pci_dev_t)0);

	platform_context->stbridgedev = OS_PCI_FIND_DEVICE(PCI_VENDOR_ID_STMICRO,
			PCI_DEVICE_ID_SDVO_TNC_ST,
			6,
			0,
			1,
			(os_pci_dev_t)0);

	if (platform_context->stbridgedev) {
		platform_context->stgpiodev = OS_PCI_FIND_DEVICE(PCI_VENDOR_ID_STMICRO,
			PCI_DEVICE_ID_SDVO_TNC_ST_GPIO,
			3,
			0,
			0,
			(os_pci_dev_t)0);

		if (!platform_context->stgpiodev) {
			platform_context->stgpiodev = OS_PCI_FIND_DEVICE(PCI_VENDOR_ID_STMICRO,
				PCI_DEVICE_ID_SDVO_TNC_ST_GPIO,
				4,
				0,
				5,
				(os_pci_dev_t)0);
			if (!platform_context->stgpiodev) {
				printk("Using STM device, but is not CUT1 or CUT2\n");
				EMGD_ERROR_EXIT("Using STM device, but is not Cut1 or Cut2");
				return -IGD_ERROR_NODEV;
			}
		}
	}

	/* Set to NULL, so full_shutdown_tnc() knows whether it was initialized: */
	platform_context->lpc_dev = NULL;

	/*
	 * finds the bus, device, func to be returned. Do this for D2:F0 only.
	 * the OS does not need to know the existence of D3:F0
	 */
	OS_PCI_GET_SLOT_ADDRESS(vga_dev, bus, slot, func);

	get_revision_id_tnc(context, vga_dev, platform_context->pcidev1);

	/*
	 * Read BSM.
	 * This must be in query so it is available early for the vBIOS.
	 */
	if(OS_PCI_READ_CONFIG_32(vga_dev,
			TNC_PCI_BSM, &context->device_context.fb_adr)) {
		EMGD_ERROR_EXIT("Reading BSM");
		return -IGD_ERROR_NODEV;
	}
	context->device_context.fb_adr &= 0xFFFFF000;

	EMGD_DEBUG("BSM (High)@: 0x%lx, (Low) 0x%4lx",
		(context->device_context.fb_adr >> 16), context->device_context.fb_adr);

	/*
	 * Read IO Base.
	 * This must be in query so it is available early for the vBIOS.
	 */
	if(OS_PCI_READ_CONFIG_16(vga_dev, TNC_PCI_IOBAR, &io_base)) {
		EMGD_ERROR_EXIT("Reading IO Base");
		return -IGD_ERROR_NODEV;
	}

	/* Base Address is defined in Bits 15:3*/
	io_base_lvds = io_base &= 0xfff8;
	EMGD_DEBUG("io @: 0x%x", io_base);

	/* Gen4 is always io_mapped */
	io_mapped_lvds = io_mapped = 1;

	/* Set dev3 iobase.  */
	if(OS_PCI_READ_CONFIG_16((os_pci_dev_t)platform_context->pcidev1,
		TNC_PCI_IOBAR, &io_base_sdvo)) {

		EMGD_ERROR_EXIT("Reading SDVO IO Base");
		return -IGD_ERROR_NODEV;
	}

	/* Base Address is defined in Bits 15:3*/
	io_base_sdvo &= 0xfff8;

	io_mapped_sdvo = 1;
	EMGD_DEBUG("sdvo io @: 0x%x", io_base_sdvo);

	/* Set stmicro sdvo iobase.  */
	if(OS_PCI_READ_CONFIG_16((os_pci_dev_t)platform_context->stbridgedev,
		TNC_PCI_IOBAR, &io_base_sdvo_st)) {

		EMGD_ERROR_EXIT("Reading SDVO IO Base");
		return -IGD_ERROR_NODEV;
	}

	/* Base Address is defined in Bits 15:3*/
	io_base_sdvo_st &= 0xfff8;

	io_mapped_sdvo_st = 1;
	EMGD_DEBUG("STMicro's sdvo io @: 0x%x", io_base_sdvo_st);

	/* Set stmicro gpio sdvo iobase.  */
	if(OS_PCI_READ_CONFIG_16((os_pci_dev_t)platform_context->stgpiodev,
		TNC_PCI_IOBAR, &io_base_sdvo_st_gpio)) {

		EMGD_ERROR_EXIT("Reading SDVO IO Base");
		return -IGD_ERROR_NODEV;
	}

	/* Base Address is defined in Bits 15:3*/
	io_base_sdvo_st_gpio &= 0xfff8;

	io_mapped_sdvo_st_gpio = 1;
	EMGD_DEBUG("STMicro's gpio io @: 0x%x", io_base_sdvo_st_gpio);

	/* ---------------------------------------------------
	 * Initialize Device 31 : LPC Interface
	 * --------------------------------------------------*/
	/*
	 * Map the LPC Interface Configuration [D31:F0]GPIO_BAR.
	 * The Atom E6xx LVDS pins are connected to GPIO pins,
	 * accessible using LPC Interface GPIO_BAR. These registers
	 * will later be used to "bit bash" the LVDS DDC signals
	 * SDVO does not need these registers.
	 * VBIOS may need access to these registers
	 */

	platform_context->lpc_dev = OS_PCI_FIND_DEVICE(PCI_VENDOR_ID_INTEL,
			PCI_DEVICE_ID_LPC_TNC,
			0,
			31, /* LPC[D31:F0] */
			0,
			(os_pci_dev_t)0);

	if(!platform_context->lpc_dev){
		/*
		 * We could not detect the LPC interface in the PCI Bus. This will
		 * be a problem. Sound the alarm, return with NO ERROR so that we do
		 * not go and map the GPIO_BAR
		 */
		EMGD_ERROR_EXIT("Reading GPIO BAR");
		return 0;
	}

	/* Set dev31 iobase */

	/* Do not enable LPC device as System BIOS owns and does this */

	/* read the GPIO BAR (OFFSET 44:47) */
	if(OS_PCI_READ_CONFIG_16(platform_context->lpc_dev,
			TNC_PCI_GBA, &io_base_lpc)) {
		EMGD_ERROR_EXIT("Reading LPC GPIO BAR");
		/* We cannot read the GPIO BAR. It is a problem but we can go on with init
		 * return with NO ERROR*/
		return 0;
	}

	io_base_lpc &= 0xffc0;

	io_mapped_lpc = 1;
	EMGD_DEBUG("lpc io @: 0x%x", io_base_lpc);

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
static int config_tnc(igd_context_t *context,
	init_dispatch_t *dispatch)
{
	unsigned long freq[2];
	platform_context_tnc_t *platform_context = &platform_context_tnc;
	unsigned long lbb;
#ifndef CONFIG_MICRO
	unsigned int lp_ctrl_reg;
	unsigned int hp_ctrl_reg;
	unsigned int ved_cg_dis_reg;
#endif

	EMGD_TRACE_ENTER;

	OPT_MICRO_CALL(full_config_tnc(context, dispatch));

	/* Get graphics and core frequency param if it exists */
	if(!get_param_tnc(context, IGD_PARAM_GFX_FREQ,
			freq)) {
		context->device_context.gfx_freq = (unsigned short)freq[0];
		context->device_context.core_freq = (unsigned short)freq[1];
	}

	/*
	 * FIXME:
	 *  Coreclk register above is used to determine some clocking information
	 *  there is also a fuse to limit the dclk. More research needed.
	 */
	/* From KT: Atom E6xx LVDS min and max dot clocks are 19.75 MHz to 79.5 MHz,
	 *          Atom E6xx SDVO min and max dot clocks are 25 MHz to 165 MHz */
	context->device_context.max_dclk = 79500;   /* in KHz */

#ifndef CONFIG_MICRO
	/* This breaks VBIOS LVDS display.  If this is truly a workaround for
	 * system BIOS then we need to understand what the system BIOS is going
	 * to do and make sure it doesn't re-break VBIOS. The hp_ctrl_reg write
	 * is the write that actually breaks LVDS display.  Tested with BIOS34
	 * which has the P-Unit workaround and LVDS still works.
     */

	/* This is just a workaround.
	 * GVD.G_LP_Control register is set to default mode for BIT0~BIT3.
	 * GVD.H_HP Control register's BIT1 is set 1.
	 * TODO: Removed this after this is fix in system BIOS.
	 */
	lp_ctrl_reg = EMGD_READ32(EMGD_MMIO(context->device_context.virt_mmadr) + 0x20f4);
	lp_ctrl_reg |= BIT1;
	lp_ctrl_reg &= ~(BIT2 | BIT3);
    EMGD_WRITE32(lp_ctrl_reg, EMGD_MMIO(context->device_context.virt_mmadr) + 0x20f4);

	hp_ctrl_reg = EMGD_READ32(EMGD_MMIO(context->device_context.virt_mmadr) + 0x20f8);
	hp_ctrl_reg |= BIT1;
    EMGD_WRITE32(hp_ctrl_reg, EMGD_MMIO(context->device_context.virt_mmadr) + 0x20f8);

	/* This is just a workaround.
	 * GVD.VED_CG_DIS register is set to disable clock gating for BIT16, BIT0~BIT8.
	 * Tested with Punit B0_500309_CFG2 and Punit C0_060510_CFG2 in BIOS39 and
	 * BIOS41 or above.
	 */
	ved_cg_dis_reg = EMGD_READ32(EMGD_MMIO(context->device_context.virt_mmadr) + 0x2064);
	ved_cg_dis_reg |= (BIT16 | BIT8 | 0xFF);
    EMGD_WRITE32(ved_cg_dis_reg, EMGD_MMIO(context->device_context.virt_mmadr) + 0x2064);

	/* read out the fuse values */
	dump_fuse_values( );
#endif

	gtt_init_tnc(context);

       /* 
	 * Setting the LBB to 0xFF if it is 0. 
	 * This register is used to dynamic LVDS backlight control. By default, 
	 * the register will reset to 0x0, this will cause the LVDS to be "off" when 
	 * PD_ATTR_ID_BLM_LEGACY_MODE attribute is set. Customers could write 
	 * application to set this register. 
	 *
	 * TODO: The right way to fix this is to check for the attribute in lvds.c
	 * then set the register through pd. But this will add more code to VBIOS
	 * (as we need to add dispatch functions in pd)
	 */

	if(OS_PCI_READ_CONFIG_32(platform_context->pcidev0, 0xF4, &lbb)) {
		EMGD_DEBUG("Reading Legacy Backlight Brightness");
		return -IGD_ERROR_NODEV;
	} 
	if(!(lbb & 0xFF)){
		if(OS_PCI_WRITE_CONFIG_32(platform_context->pcidev0,
			0xF4, (lbb | 0xFF))){
			EMGD_DEBUG("Writing into Legacy Backlight Brightness");
			return -IGD_ERROR_INVAL;
		}
	}


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
static int get_param_tnc(igd_context_t *context, unsigned long id,
	unsigned long *value)
{
#define FB_SKU_MASK  (BIT12|BIT13|BIT14)
#define FB_SKU_SHIFT 12
#define FB_GFX_CLOCK_DIVIDE_MASK  (BIT20|BIT21|BIT22)
#define FB_GFX_CLOCK_DIVIDE_SHIFT 20

	int ret = 0;
	unsigned long control_reg;
	unsigned short sku;
	unsigned short ratio;

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

		/* Read the fuse value */
		if(query_sch_message(READ_FUS_EFF3, &control_reg)){
			EMGD_ERROR("Cannot read GFX clock");
		}
		EMGD_DEBUG("SKU [reg 0x%x] value = 0x%lx", READ_FUS_EFF3, control_reg);

		/*
		 * Sku and Ratio bits determine the gfx clock speed
		 * sku - 0:sku_100 1:sku_100L 2:sku_83
		 * ratio - 0-1:1 1-4:3 2-8:5 3-2:1 4-16:7(rsvd) 5-8:3 6-16:5 7:4:1(rsvd)
		 */
		sku = (unsigned short)((control_reg & FB_SKU_MASK) >> FB_SKU_SHIFT) & 0x3;
		ratio = (unsigned short)((control_reg & FB_GFX_CLOCK_DIVIDE_MASK) >> FB_GFX_CLOCK_DIVIDE_SHIFT) & 0x7;

		EMGD_DEBUG("sku = 0x%x Ratio = 0x%x", sku, ratio);

		if(sku < SKU_NO && ratio < RATIO_NO){
			/* get the graphics clock speed from the sku-ratio array */
			value[0] = tnc_gfx_freq_list[ratio][sku];
			value[1] = tnc_core_freq_list[sku];
		} else {
			EMGD_ERROR("tnc_gfx_freq_list ARRAY OUT OF RANGE");
			/* set to the lowest default value */
			value[0] = 333;
			value[1] = 166;
		}

		EMGD_DEBUG("TNC GFX core frequency = %lu MHz", value[0]);
		EMGD_DEBUG("TNC Core clock frequency = %lu MHz", value[1]);

		break;

	default:
		/* If the param is not found here then it may only be in the
		 * full version.
		 */
		OPT_MICRO_CALL_RET(ret, full_get_param_tnc(context, id, value));
		break;
	}

	EMGD_TRACE_EXIT;
	return ret;
}

/*!
 *
 * @param context
 * @param id
 * @param value
 *
 * @return -IGD_ERROR_INVAL
 */
static int set_param_tnc(igd_context_t *context, unsigned long id,
	unsigned long value)
{
	return 0;
}

/*!
 * Functions reads all the fuse values and dumps out the value
 * @return -IGD_ERROR_INVAL
 */
#ifndef CONFIG_MICRO
static int dump_fuse_values(void)
{
	unsigned long value = 0;

	if(query_sch_message(READ_FUS_EFF0, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF0 [%lx]", value);

	if(query_sch_message(READ_FUS_EFF1, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF1 [%lx]", value);

	if(query_sch_message(READ_FUS_EFF2, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF2 [%lx]", value);

	if(query_sch_message(READ_FUS_EFF3, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF3 [%lx]", value);

	if(query_sch_message(READ_FUS_EFF4, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF4 [%lx]", value);

	if(query_sch_message(READ_FUS_EFF5, &value)){
		EMGD_ERROR_EXIT("Reading Fuse Value Failed");
	}
	EMGD_DEBUG("READ_FUS_EFF5 [%lx]", value);

	return 0;
}
#endif
/*!
 *
 * @param context
 *
 * @return void
 */
static void shutdown_tnc(igd_context_t *context)
{
	gtt_shutdown_tnc(context);

	OPT_MICRO_VOID_CALL(full_shutdown_tnc(context));
}


/*!
 *
 * @param context
 * @param dispatch
 * @param vga_dev
 *
 * @return -IGD_ERROR_NODEV on failure
 * @return 0 on success
 */
int get_revision_id_tnc(igd_context_t *context,
	os_pci_dev_t vga_dev,
	os_pci_dev_t sdvo_dev)
{
	platform_context_tnc_t *platform_context;

	EMGD_TRACE_ENTER;
  	 
	platform_context = (platform_context_tnc_t *)context->platform_context;

	/* Read RID */
	if(OS_PCI_READ_CONFIG_8(vga_dev, PCI_RID,
		(unsigned char *)&context->device_context.rid)) {
		EMGD_ERROR_EXIT("Error occured reading RID");
		return -IGD_ERROR_NODEV;
	}
  	 
	if(OS_PCI_READ_CONFIG_8(sdvo_dev, PCI_RID,
		&platform_context->tnc_dev3_rid)) {
		EMGD_ERROR_EXIT("Error occured reading TNC SDVO RID");
		return -IGD_ERROR_NODEV;
	}
  	 
	EMGD_DEBUG(" rid = 0x%lx", context->device_context.rid);
	
	EMGD_TRACE_EXIT;
	return 0;
}

#endif

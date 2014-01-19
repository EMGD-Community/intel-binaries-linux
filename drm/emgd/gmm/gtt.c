/*
 *-----------------------------------------------------------------------------
 * Filename: gtt.c
 * $Revision: 1.17 $
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
 *  Manage the GTT.
 *-----------------------------------------------------------------------------
 */
#include <igd_debug.h>
#include <drmP.h>
#include <memlist.h>
#include <io.h>
#include <memory.h>
#include <asm/cacheflush.h>
#include <linux/version.h>

#define PFX "EMGD: "

#define SCR1	0x71410 /* scratch register set by vbios indicating status*/
#define SCR2	0x71418 /* scratch register set by vbios indicating amount of stolen memory */
#define FW_ID	0xE1DF0000 /* firmware identifier */
#define ST_BIT	0x00000004 /* bit2- stolen memory bit */
#define PSB_PTE_VALID  0x0001

void emgd_free_pages(gmm_mem_buffer_t *mem);

static DEFINE_MUTEX(client_sem);
static DEFINE_MUTEX(gtt_sem);

struct client_list_struct {
	struct list_head list;
	struct vm_area_struct *vma;
	pid_t pid;
};

static LIST_HEAD(client_list);

static void ipi_handler(void *null) {
	//flush_agp_cache(); /* from agp.h */
	wbinvd();
}

static void emgd_cache_flush(void) {
	if (on_each_cpu(ipi_handler, NULL, 1) != 0)
		panic(PFX "timed out waiting for the other CPUs!\n");
}

static void tlb_flush(void) {
	/* If needed, this would flush the SGX/MSVDX mmu TLB's */
}

/* FIXME - TURNS OUT THAT THIS FUNCTION IS CURRENTLY A NOOP, BECAUSE
 * client_list NEVER HAS ANYTHING ADDED TO IT.
 */
static void invalidate_vma(unsigned long pg_offset, unsigned long bus_addr) {
	int zap;
	struct list_head *tmp;
	struct client_list_struct *entry;
	unsigned long addr_start=0;
	unsigned long addr_end=0;
	unsigned long addr_offset=0;
	unsigned long vaddr;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	mutex_lock(&client_sem);
	list_for_each(tmp, &client_list) {
		entry = list_entry(tmp, struct client_list_struct, list);

		/*
		 * We need to handle invalidating VMA's that are only mapping
		 * a portion of the virtual aperture.  Calculate what if
		 * any invalidated pages need to be zapped
		 */
		addr_start = (entry->vma->vm_pgoff << PAGE_SHIFT)
			- bus_addr;
		addr_end = addr_start + (entry->vma->vm_end - entry->vma->vm_start);
		addr_offset = pg_offset << PAGE_SHIFT;

		vaddr = entry->vma->vm_start + (addr_offset - addr_start);

		zap=0;
		pgd=NULL;
		pud=NULL;
		pmd=NULL;
		pte=NULL;

		/*
		 * Look up page table entries for all VMAs that currently
		 * have the virtual aperture mapped -- to see if the page
		 * has ever faulted
		 */
		pgd = pgd_offset(entry->vma->vm_mm, vaddr);
		if (!pgd_none(*pgd)) {
			pud = pud_offset(pgd, vaddr);
			if (!pud_none(*pud)) {
				pmd = pmd_offset(pud, vaddr);
				if (!pmd_none(*pmd)) {
					pte = pte_offset_map(pmd, vaddr);
					if (!pte_none(*pte)) {
						zap=1;
					}
				}
			}
		}
		/*
		 * Only zap a page if it falls within the mapped region
		 * and it has previously faulted
		 */
		if (zap && (addr_offset >= addr_start) &&
				(addr_offset < addr_end)) {


			if (!page_mapcount(pte_page(*pte))) {
				printk(KERN_ERR "[EMGD] ERROR No mapcount\n");
				printk(KERN_ALERT "[EMGD] ZR %p %08lX %d %d %p\n",
						pte_page(*pte),
						pte_page(*pte)->flags, page_count(pte_page(*pte)),
						page_mapcount(pte_page(*pte)), pte_page(*pte)->mapping);
			} else {
				atomic_add_negative(-1, &pte_page(*pte)->_mapcount);
				put_page(pte_page(*pte));
#if  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34))
				dec_mm_counter(entry->vma->vm_mm, file_rss);
#else
				dec_mm_counter(entry->vma->vm_mm, MM_FILEPAGES);
#endif
			}

			pte_clear(entry->vma->vm_mm, vaddr, pte);
		}

		if(pte) {
			pte_unmap(pte);
		}
	}
	mutex_unlock(&client_sem);
}



/*
 * Allocate pages from the kernel and store in a page list.
 */
gmm_mem_buffer_t *emgd_alloc_pages(unsigned long num_pages, int type) {
	gmm_mem_buffer_t *mem;
	size_t list_size;
	struct page *page;
	int i;
	int order;

	mem = (gmm_mem_buffer_t *)kzalloc(sizeof(gmm_mem_buffer_t), GFP_KERNEL);
	if (mem == NULL) {
		return NULL;
	}

	/* First allocate page array */
	list_size = num_pages * sizeof(struct page *);
	mem->vmalloc_flag = false;

	if (list_size <= (2 * PAGE_SIZE)) {
		mem->pages = kmalloc(list_size, GFP_KERNEL | __GFP_NORETRY);
	}

	if (mem->pages == NULL) {
		mem->pages = vmalloc(list_size);
		mem->vmalloc_flag = true;
	}

	if (mem->pages == NULL) {
		printk(KERN_ERR "[EMGD] Failed to allocate memory info struct.\n");
		kfree(mem);
		return NULL;
	}

	/*
	 * If we need phyical contiguous memory, then do things differently.
	 *   Call alloc_pages(GFP_KERNEL, pages) to allocate all the pages.
	 *   The page structure returned is just the first page of the group.
	 *   ? is it a virtual address ?
	 *
	 *   mem->pages[0] = virt_to_phys(page)
	 *   mem->pages[1] = mem->pages[0] + PAGE_SIZE
	 */

	if ((type == 1) || (type == 0)) {
	/* Next allocate the pages */
		for (i = 0; i < num_pages; i++) {
			page = alloc_page(GFP_KERNEL | GFP_DMA32 | __GFP_ZERO);
			if (page == NULL) {
				/* Error! */
				printk(KERN_ERR "[EMGD] Memory allocation failure!\n");
				if (mem->vmalloc_flag) {
					vfree(mem->pages);
				} else {
					kfree(mem->pages);
				}
				kfree(mem);
				return NULL;
			}

			/* Make sure this page isn't cached */
			if (set_memory_uc((unsigned long) page_address(page), 1) < 0) {
				printk(KERN_ERR "[EMGD] Unable to set page attributes for newly"
					" allocated graphics memory.\n");
				/* Rely on the fact that we've kept up the data structures: */
				emgd_free_pages(mem);
				/* XXX - THIS IS WHAT SOME OLD IEGD CODE DID--A GOOD IDEA??? */
				set_memory_wb((unsigned long) page_address(page), 1);
				__free_page(page);
				return NULL;
			}

			get_page(page);
			mem->pages[i] = page;
			mem->page_count++;
		}
	} else {
		if (num_pages == 1) {
			order = 0;
		} else if (num_pages == 4) {
			order = 2;
		} else if (num_pages == 8) {
			order = 3;
		} else {
			printk(KERN_ERR "[EMGD] Page count is not valid for physical "
				"allocation.\n");
			if (mem->vmalloc_flag) {
				vfree(mem->pages);
			} else {
				kfree(mem->pages);
			}
			kfree(mem);
			return NULL;
		}

		page = alloc_pages(GFP_KERNEL, order);
		if (page == NULL) {
			/* Error! */
			printk(KERN_ERR "[EMGD] Memory allocation failure!\n");
			if (mem->vmalloc_flag) {
				vfree(mem->pages);
			} else {
				kfree(mem->pages);
			}
			kfree(mem);
			return NULL;
		} else {
			/* Make sure these pages aren't cached */
			if (set_memory_uc((unsigned long) page_address(page),
				num_pages) < 0) {
				printk(KERN_ERR "[EMGD] Unable to set page attributes for newly"
					" allocated physical graphics memory.\n");
				/* XXX - THIS IS WHAT SOME OLD IEGD CODE DID--A GOOD IDEA??? */
				set_memory_wb((unsigned long) page_address(page), num_pages);
				__free_pages(page, num_pages);
				if (mem->vmalloc_flag) {
					vfree(mem->pages);
				} else {
					kfree(mem->pages);
				}
				kfree(mem);
				return NULL;
			}

			get_page(page);
			mem->pages[0] = page;
			if (num_pages > 1) {
				for (i = 1; i < num_pages; i++) {
					mem->pages[i] = mem->pages[i-1] + 1;
				}
			}
			mem->physical = page_to_phys(mem->pages[0]);
			mem->page_count = num_pages;
		}
	}

	return mem;
}

/*
 * Free memory pages.
 */
void emgd_free_pages(gmm_mem_buffer_t *mem) {
	int i;
	struct page *page;

	for (i = 0; i < mem->page_count; i++) {
		page = mem->pages[i];
		/* XXX - THIS IS WHAT SOME OLD IEGD CODE DID--A GOOD IDEA??? */
		set_memory_wb((unsigned long) page_address(page), 1);
		put_page(page);
		__free_page(page);
		mem->pages[i] = NULL;
	}

	if (mem->vmalloc_flag) {
		vfree(mem->pages);
	} else {
		kfree(mem->pages);
	}

	kfree(mem);
}


/*
 * Need a function to populate the GTT with the pages.
 *
 * The caller provides the offset into the GTT where the memory needs
 * to go.  This simply needs to allocate the pages and insert them
 * into the GTT.
 */
void emgd_gtt_insert(igd_context_t *context,
		gmm_mem_buffer_t *mem,
		unsigned long offset)
{
	struct page *page;
	unsigned long pte;
	unsigned long pg_off;
	int i, j;

	pg_off = offset >> PAGE_SHIFT;

	/* Check that the offset is within the gtt's range */
	if ((pg_off + mem->page_count) > context->device_context.gatt_pages) {
		printk(KERN_ERR "[EMGD] Attempt to insert a offset beyond of GTT range.\n");
		return;
	}

	/* Flush before inserting pages into the GTT */
	emgd_cache_flush();
	tlb_flush();


	/* Insert the pages into the GTT */
	mutex_lock(&gtt_sem);
	for (i = 0, j = pg_off; i < mem->page_count; i++, j++) {
		page = mem->pages[i];

		/* Mark the page as valid */
		pte = page_to_phys(page) | PSB_PTE_VALID;
		writel(pte, (context->device_context.virt_gttadr + j));
		readl(context->device_context.virt_gttadr + j);

	}

	mutex_unlock(&gtt_sem);

	(void)readl(context->device_context.virt_gttadr + j - 1);

	/* Invalidate VMA's */
	invalidate_vma(j,
		(context->device_context.gmch_ctl | PCI_BASE_ADDRESS_MEM_MASK));

	/* Flush */
	emgd_cache_flush();
	tlb_flush();

	return;
}

/*
 * Need a function to remove pages from the GTT (and replace with the
 * scratch page?) and free the pages.
 */

void emgd_gtt_remove(igd_context_t *context,
		gmm_mem_buffer_t *mem,
		unsigned long offset)
{
	struct page *page;
	unsigned long pte;
	int i;
	unsigned long pg_start;

	pg_start = offset >> PAGE_SHIFT;

	/* Flush before inserting pages into the GTT */
	emgd_cache_flush();
	tlb_flush();

	mutex_lock(&gtt_sem);

	page = context->device_context.scratch_page;
	pte = page_to_phys(page) | PSB_PTE_VALID;

	/* Insert the scratch page into the GTT */
	for (i = pg_start; i < (mem->page_count + pg_start); i++) {
		/* FIXME: Apparently we don't really need to copy stolen memory pages.
		 * If so, what should we do about the following code?  Is it correct to
		 * do nothing?
		 */
		if (i < context->device_context.stolen_pages) {
			/* This is stolen memory.... */
		} else {
			writel(pte, context->device_context.virt_gttadr + i);
			(void)readl(context->device_context.virt_gttadr + i);
		}

	}

	mutex_unlock(&gtt_sem);

	/* Invaidate VMA's */
	invalidate_vma(i,
			(context->device_context.gmch_ctl | PCI_BASE_ADDRESS_MEM_MASK));

	/* Flush */
	emgd_cache_flush();
	tlb_flush();
}

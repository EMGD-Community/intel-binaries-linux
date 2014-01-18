/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_mmap.c
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
 *  Memory mapping functions.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.gart

#include <linux/mm.h>
#include <drm_emgd_private.h>
#include <emgd_drv.h>
#include <emgd_drm.h>
#include <memory.h>
#include <memlist.h>
#include <io.h>

/*
 * Bottom 256MB reserved for display
 */
#define DRM_PSB_FILE_PAGE_OFFSET (0x10000000UL >> PAGE_SHIFT)

extern gmm_chunk_t *gmm_get_chunk(igd_context_t *context,
		unsigned long vm_pgoff);
extern int PVRMMap(struct file *pFile, struct vm_area_struct *ps_vma);

static int emgd_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
static void emgd_vm_open(struct vm_area_struct *vma);
static void emgd_vm_close(struct vm_area_struct *vma);

static struct vm_operations_struct emgd_vm_ops = {
	.fault = emgd_vm_fault,
	.open = emgd_vm_open,
	.close = emgd_vm_close
};

/*
 * Create a virtual address mapping for physical pages of memory.
 *
 * This needs to handle requrests for both the EMGD display driver
 * and the IMG 2D/3D drivers.
 *
 * If the page offset falls below the 256MB limit for display,
 * then map display memory. If above, route to the IMG handler.
 */
int emgd_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_file *file_priv;
	drm_emgd_priv_t *emgd_priv;
	gmm_chunk_t *chunk;
	unsigned long offset;

	/*
	 * re-direct offsets beyond the 256MB display range to PVRMMap
	 */
	if (vma->vm_pgoff > DRM_PSB_FILE_PAGE_OFFSET) {
		EMGD_DEBUG("emgd_mmap: Calling PVRMMap().");
		return PVRMMap(filp, vma);
	}

	file_priv = (struct drm_file *) filp->private_data;
	emgd_priv = (drm_emgd_priv_t *)file_priv->minor->dev->dev_private;
	offset = vma->vm_pgoff << PAGE_SHIFT;

	/*
	 * Look up the buffer in the gmm chunk list based on offset
	 * and size.
	 */
	/* chunk = emgd_priv->context->dispatch->gmm_get_chunk(vma->vm_pgoff);*/
	chunk = gmm_get_chunk(emgd_priv->context, offset);
	if (chunk == NULL) {
		printk(KERN_ERR "emgd_mmap: Failed to find memory at 0x%lx.", offset);
	}

	/*
	 * Fill in the vma
	 */
	vma->vm_ops = &emgd_vm_ops;
	vma->vm_private_data = chunk;
	vma->vm_flags |= VM_RESERVED | VM_IO | VM_MIXEDMAP | VM_DONTEXPAND;
	pgprot_val(vma->vm_page_prot) =
		pgprot_val(vma->vm_page_prot) | _PAGE_CACHE_UC_MINUS;

	return 0;
}


static int emgd_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	unsigned long offset;
	unsigned long pg_offset;
	gmm_chunk_t *chunk;
	struct page *page;

	offset = (unsigned long)vmf->virtual_address - vma->vm_start;
	pg_offset = offset >> PAGE_SHIFT;

	chunk = (gmm_chunk_t *)vma->vm_private_data;

	if (chunk == NULL) {
		printk(KERN_ERR "emgd_vm_fault: Chunk is NULL.\n");
		vmf->page = NULL;
		return VM_FAULT_SIGBUS;
	}

	if (pg_offset > chunk->gtt_mem->page_count) {
		printk(KERN_ERR "emgd_vm_fault: page offet (%lu) > page count (%d)\n",
				pg_offset, chunk->gtt_mem->page_count);
		return VM_FAULT_SIGBUS;
	}

	page = chunk->gtt_mem->pages[pg_offset];
	get_page(page);
	vmf->page = page;

	return 0;
}

/*
 * Increase the refrence count on the memory object
 */
static void emgd_vm_open(struct vm_area_struct *vma)
{
	struct drm_file *priv = vma->vm_file->private_data;
	struct drm_device *dev = priv->minor->dev;


	/*
	 * Does the DRM really need to keep track of the count if we're managing
	 * everything?
	 */
	atomic_inc(&dev->vma_count);

	/*
	 * DRM code maintains a list of vma's and when open is
	 * called, the vma is added to the list.
	 */

}

/*
 * Decrease the reference count on the memory object and
 * remove the refrence to the memory object.
 */
static void emgd_vm_close(struct vm_area_struct *vma)
{
	struct drm_file *priv = vma->vm_file->private_data;
	struct drm_device *dev = priv->minor->dev;

	/*
	 * Does the DRM really need to keep track of the count if we're managing
	 * everything?
	 */
	atomic_dec(&dev->vma_count);

	/*
	 * DRM code maintains a list of vma's and when close is
	 * called, the vma is removed to the list.
	 */

}


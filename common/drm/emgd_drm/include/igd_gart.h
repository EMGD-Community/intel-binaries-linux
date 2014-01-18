/*
 *-----------------------------------------------------------------------------
 * Filename: igd_gart.h
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_GART_H_
#define _IGD_GATR_H_

typedef struct _gtt_info {
	void* virt_mmadr;
	void* virt_gttadr;
	void* virt_gttadr_upper;

	unsigned long new_gtt;
	unsigned long gtt_phyadr;
	unsigned long is_virt_aperture;
	unsigned long reset_gtt_entries;
	unsigned long fb_phys_addr;
	unsigned long scratch_phys;

	unsigned long num_gtt_entries;
	unsigned long gtt_entry_start;
	unsigned long gtt_entry_end;
	unsigned long num_contig_allocs;
	unsigned char **cont_pages_virts;
	/* FIXME!!! - this cant handle 64-bit Architecture*/
	unsigned long *cont_pages_phys;
	unsigned long *cont_pages_sizes;
#ifdef D3D_DPM_ALLOC
   void* virt_gttadr_dpm;
#endif
}gtt_info_t;

/*
 * Note: Platforms extend this data structure so the pointer can be used
 * as either this DI dispatch or cast to the DD dipatch.
 */
typedef struct _init_gart_dispatch {
	int (*get_gtt_ctl)(void* gtt_info);
	int (*init_gtt_table)(void* gtt_info);
	int (*flush_gtt_tlb)(void* gtt_info);
	int (*shutdown_gtt)(void* gtt_info);
} init_gart_dispatch_t;

void* igd_get_gtt_dispatch(unsigned long dev_id);
int igd_get_gtt_ctl(void* gtt_info, unsigned long dev_id);
int igd_init_gtt(void* gtt_info, unsigned long dev_id);
int igd_flush_gtt(void* gtt_info, unsigned long dev_id);
int igd_shutdown_gtt(void* gtt_info, unsigned long dev_id);

#endif

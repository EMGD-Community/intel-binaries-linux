/*
 *-----------------------------------------------------------------------------
 * Filename: topaz.c
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

#include <io.h>
#include <pci.h>
#include <memmap.h>
#include <sched.h>

#include <igd.h>
#include <igd_errno.h>
#include <igd_init.h>

#include <context.h>
#include <intelpci.h>
#include <general.h>
#include <utils.h>
#include <topaz.h>
#include "topaz_hdr.h"

#include <plb/regs.h>
#include <plb/context.h>

#define MAX_CURRENT_TOPAZ_CMD_SIZE 5
#define MAX_TOPAZ_CMD_SIZE 0x20

void write_mtx_mem_multiple_setup(igd_context_t *context,
		unsigned long addr)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long bank_size;
	unsigned long ram_size;
	unsigned long ram_id;
	unsigned long reg;
	unsigned long ctrl = 0;

	reg = EMGD_READ32(mmio + TNC_TOPAZ_MTX_DEBUG);

	reg = 0x0a0a0606;
	bank_size = (reg & 0xf0000) >> 16;

	ram_size = (unsigned long) (1 << (bank_size + 2));
	ram_id = (addr - MTX_DATA_BASE) / ram_size;

	/* cmd id */
	ctrl = ((0x18 + ram_id) << 20) & 0x0ff00000;
	/* Address to read */
	ctrl |= ((addr >> 2) << 2) & 0x000ffffc;
	ctrl |= 0x02;
	/*printk(KERN_INFO "write_mtx_multiple_setup: ctrl=0x%08x, addr=0x%08x", ctrl, addr);*/
	EMGD_WRITE32(ctrl, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_CONTROL);

	return;
}

void write_mtx_mem_multiple(igd_context_t *context,
		unsigned long cmd)
{
	unsigned char *mmio = context->device_context.virt_mmadr;

	EMGD_WRITE32(cmd, mmio + TNC_TOPAZ_MTX_RAM_ACCESS_DATA_TRANSFER);
}

/*
 * This is the function that actually passes the message to the MTX
 * firmware. The contents of the message are opaque to this function.
 *
 * Currently, the only supported message type is RENDER.
 */
int mtx_send_tnc(igd_context_t *context, unsigned long *msg)
{
	struct topaz_cmd_header *cur_cmd_header =
			(struct topaz_cmd_header *) msg;
	unsigned long cmd_size = cur_cmd_header->size;
	unsigned long read_index, write_index;
	const unsigned long *cmd_pointer = (unsigned long *)msg;
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;
	int ret = 0;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	write_index = topaz_priv->topaz_cmd_windex;
	if (write_index + cmd_size + 1 > topaz_priv->topaz_ccb_size)
	{
		int free_space = topaz_priv->topaz_ccb_size - write_index;

		EMGD_DEBUG("TOPAZ: wrap CCB write point");
		if (free_space > 0)
		{
			struct topaz_cmd_header pad_cmd;

			pad_cmd.id = MTX_CMDID_NULL;
			pad_cmd.size = free_space;
			pad_cmd.seq = 0x7fff & topaz_priv->topaz_cmd_seq++;

			EMGD_DEBUG("TOPAZ: MTX_CMDID_NULL:"
				" size(%d),seq (0x%04x)\n",
				pad_cmd.size, pad_cmd.seq);

			TOPAZ_BEGIN_CCB(context);
			TOPAZ_OUT_CCB(context, pad_cmd.val);
			TOPAZ_END_CCB(context, 1);
		}
		POLL_WB_RINDEX(context, 0);
		if (ret == 0) {
			topaz_priv->topaz_cmd_windex = 0;
		} else {
			printk(KERN_ERR "TOPAZ: poll rindex timeout\n");
			return ret; /* HW may hang, need reset */
		}
		EMGD_DEBUG("TOPAZ: -------wrap CCB was done.\n");
	}

	read_index = CCB_CTRL_RINDEX(context);/* temperily use CCB CTRL */
	write_index = topaz_priv->topaz_cmd_windex;

	/*printk(KERN_INFO "TOPAZ: write index(%ld), read index(%ld,WB=%ld)\n",
		write_index, read_index, WB_CCB_CTRL_RINDEX(context));
	printk(KERN_INFO "cmd size to kick %d",cmd_size);
	for(count=0;count<cmd_size;count++){
		printk(KERN_INFO "value to kick 0x%08x", *(cmd_pointer+count));
	}*/

	TOPAZ_BEGIN_CCB(context);
	while (cmd_size > 0) {
		TOPAZ_OUT_CCB(context, *cmd_pointer++);
		--cmd_size;
	}
	TOPAZ_END_CCB(context, 1);

	/*POLL_WB_RINDEX(context, topaz_priv->topaz_cmd_windex);
	printk(KERN_INFO "RI after kick =%ld", CCB_CTRL_RINDEX(context));*/


#if 0 /* kept for memory callback test */
	for (i = 0; i < 100; i++) {
		if (WB_CCB_CTRL_RINDEX(context) == topaz_priv->topaz_cmd_windex)
			break;
		else
			OS_SLEEP(100);
	}
	if (WB_CCB_CTRL_RINDEX(context) != topaz_priv->topaz_cmd_windex) {
		EMGD_ERROR("TOPAZ: poll rindex timeout\n");
		ret = -IGD_ERROR_HWERROR;
	}
#endif

	return ret;
}

void topaz_sync_tnc(igd_context_t *context)
{
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;
	unsigned long sync_cmd[3];
	unsigned long *sync_p, temp_ret;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;
	sync_p = (unsigned long *)topaz_priv->topaz_sync_addr;
	topaz_priv->topaz_sync_id++;

	/* insert a SYNC command here */
	topaz_priv->topaz_sync_cmd_seq = (1 << 15) |
				topaz_priv->topaz_cmd_seq++;
	sync_cmd[0] = (MTX_CMDID_SYNC << 1) | (3 << 8) |
		(topaz_priv->topaz_sync_cmd_seq << 16);
	sync_cmd[1] = topaz_priv->topaz_sync_offset;
	sync_cmd[2] = topaz_priv->topaz_sync_id;
	temp_ret = mtx_send_tnc(context, sync_cmd);
	if (0 != temp_ret){
	    EMGD_DEBUG("TOPAZ: sync error: %ld\n", temp_ret);
	}

	topaz_priv->topaz_frame_skip = CCB_CTRL_FRAMESKIP(context);

#if 0
	/* debug code to make sure SYNC can be done */
	{
		int count = 1000;
		while (count && *sync_p != topaz_priv->topaz_sync_id) {
		OS_SLEEP(1000);
		--count;
	}
	if ((count == 0) && (*sync_p != 0x45)) {
		EMGD_ERROR("TOPAZ: wait sync timeout (0x%08x),"
			"actual 0x%08x\n",
			topaz_priv->topaz_sync_id, *sync_p);
		}
		}
#endif

}


/*
 * To process this buffer, find the MTX firmware messages and send each
 * one to the MTX firmware.
 */

int process_encode_mtx_messages(igd_context_t *context,
			unsigned long *mtx_buf,
			unsigned long size)
{
	unsigned long *command = (unsigned long *) mtx_buf;
	unsigned long cmd_size = 0;
	int ret = 0;
	struct topaz_cmd_header *cur_cmd_header;
	unsigned long cur_cmd_size, cur_cmd_id;
	unsigned long codec;
	tnc_topaz_priv_t *topaz_priv;
	platform_context_tnc_t *platform;

	platform = (platform_context_tnc_t *)context->platform_context;
	topaz_priv = &platform->tpz_private_data;

	cur_cmd_header = (struct topaz_cmd_header *) command;

	if(!cur_cmd_header) {
		printk(KERN_ERR "TOPAZ: Invalid Command\n");
		return -IGD_ERROR_INVAL;
	}

	cur_cmd_size = cur_cmd_header->size;
	cur_cmd_id = cur_cmd_header->id;

	/* Verify the incoming current command size */
	if((cur_cmd_size == 0) || (cur_cmd_size > MAX_CURRENT_TOPAZ_CMD_SIZE)) {
		printk(KERN_ERR "TOPAZ: Invalid Command Size\n");
		return -IGD_ERROR_INVAL;
	}

	while (cur_cmd_id != MTX_CMDID_NULL) {

		switch (cur_cmd_id) {
			case MTX_CMDID_SW_NEW_CODEC:
				codec = *((unsigned long *) mtx_buf + 1);
				EMGD_DEBUG("TOPAZ: setup new codec %ld\n", codec);
				if (topaz_setup_fw(context, codec)) {
					printk(KERN_ERR "TOPAZ: upload FW to HW failed\n");
					return -IGD_ERROR_INVAL;
				}
				topaz_priv->topaz_cur_codec = codec;
				break;
			case MTX_CMDID_SW_ENTER_LOWPOWER:
				EMGD_DEBUG("TOPAZ: enter lowpower.... \n");
				EMGD_DEBUG("XXX: implement it\n");
				break;
			case MTX_CMDID_SW_LEAVE_LOWPOWER:
				EMGD_DEBUG("TOPAZ: leave lowpower... \n");
				EMGD_DEBUG("XXX: implement it\n");
				break;
			/* ordinary commmand */
			case MTX_CMDID_START_PIC:
				/* XXX: specially handle START_PIC hw command */
				CCB_CTRL_SET_QP(context,
					*(command + cur_cmd_size - 1));
				/* strip the QP parameter (it's software arg) */
				cur_cmd_header->size--;
			case MTX_CMDID_DO_HEADER:
			case MTX_CMDID_ENCODE_SLICE:
			case MTX_CMDID_END_PIC:
			case MTX_CMDID_SYNC:
			case MTX_CMDID_ENCODE_ONE_ROW:
			case MTX_CMDID_FLUSH:

				cur_cmd_header->seq = 0x7fff &
					topaz_priv->topaz_cmd_seq++;
				EMGD_DEBUG("TOPAZ: %ld: size(%ld), seq (0x%04x)\n",
					cur_cmd_id, cur_cmd_size, cur_cmd_header->seq);
				ret = mtx_send_tnc(context, command);
				if (ret) {
					printk(KERN_ERR "TOPAZ: error -- ret(%d)\n", ret);
					return -IGD_ERROR_INVAL;
				}
				break;
			default:
				printk(KERN_ERR "TOPAZ: Invalid Command\n");
				return -IGD_ERROR_INVAL;
			}

		/* current command done */
		command += cur_cmd_size;
		cmd_size += cur_cmd_size;

		/* Verify that the incoming commands are of reasonable size */
		if((cmd_size >= MAX_TOPAZ_CMD_SIZE)) {
			printk(KERN_ERR "TOPAZ: Invalid Command Size\n");
			return -IGD_ERROR_INVAL;
		}

		/* Get next command */
		cur_cmd_header = (struct topaz_cmd_header *) command;

		if(!cur_cmd_header) {
			printk(KERN_ERR "TOPAZ: Invalid Command\n");
			return -IGD_ERROR_INVAL;
		}

		cur_cmd_size = cur_cmd_header->size;
		cur_cmd_id = cur_cmd_header->id;

		/* Verify the incoming current command size */
		if((cur_cmd_size == 0) || (cur_cmd_size > MAX_CURRENT_TOPAZ_CMD_SIZE)) {
			printk(KERN_ERR "TOPAZ: Invalid Command Size\n");
			return -IGD_ERROR_INVAL;
		}

	}
	topaz_sync_tnc(context); 

	return 0;
}

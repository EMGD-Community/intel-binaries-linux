/*
 *-----------------------------------------------------------------------------
 * Filename: msvdx.c
 * $Revision: 1.28 $
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
 *  Send commands to the MSVDX video decode engine.
 *  The host communicates with the firmware via messages. The following
 *  messages are supported:
 *  RENDER           -> MTX
 *  DEBLOCK          -> MTX
 *  OOLD             -> MTX
 *  MSG_PADDING      -> MTX
 *  CMD_COMPLETED    <- MTX
 *  DEBLOCK_REQUIRED <- MTX
 *  CMD_FAILED       <- MTX
 *  HW_PANIC         <- MTX
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
#include <msvdx.h>

#include <plb/regs.h>
#include <plb/context.h>
#include "services_headers.h"
#include <drm_emgd_private.h>



//extern int msvdx_init_plb(void);
extern void msvdx_reset_plb(igd_context_t *context);

int send_to_mtx(igd_context_t *context, unsigned long *msg);
int msvdx_poll_mtx_irq(igd_context_t *context);
void msvdx_mtx_interrupt_plb(igd_context_t *context);
unsigned long populate_fence_id(igd_context_t *context, unsigned long *mtx_msgs,
		unsigned long mtx_msg_cnt);
int process_mtx_messages(igd_context_t *context,
		unsigned long *mtx_msgs, unsigned long mtx_msg_cnt,
		unsigned long fence);
#ifdef DEBUG_BUILD_TYPE
static void debug_mesg_info(igd_context_t *context,
		unsigned long *msg,
		unsigned long num_words);
/* To eliminate warnings, am temporarily #ifdef'ing this function.  Please
 * remove the #ifdef when this function is to be used.
 */
#ifdef USE_DEBUG_DUMP
static void debug_dump(igd_context_t *context);
#endif
static void lldma_dump(unsigned long *cmd_base, unsigned long offset);
static void dump_all_messages(igd_context_t *context);
#define DEBUG_MESG_INFO(a, b, c) debug_mesg_info(a, b, c)
#define DEBUG_DUMP(a) debug_dump(a)
#define LLDMA_DUMP(a, b) lldma_dump(a, b)
#define DUMP_ALL_MESSAGES(a) dump_all_messages(a)
#else
#define DEBUG_MESG_INFO(a, b, c)
#define DEBUG_DUMP(a)
#define LLDMA_DUMP(a, b)
#define DUMP_ALL_MESSAGES(a)
#endif

/* These are used to debug the message buffer */
unsigned long *save_msg;
unsigned long save_msg_cnt;

/* This is used to discover when MSVDX gets in a bad state and stops generating
 * interrupts.  By keeping track of when the last time a cmd was dequeued, when
 * cmds continue to be queued, but never dequeued, this bad hardware state can
 * be detected.
 */
unsigned long jiffies_at_last_dequeue = 0;

int msvdx_dequeue_send(igd_context_t *context)
{
    platform_context_plb_t *platform;
    struct msvdx_cmd_queue *msvdx_cmd = NULL;
    int ret = 0;

    EMGD_TRACE_ENTER;

    platform = (platform_context_plb_t *)context->platform_context;

    if (list_empty(&platform->msvdx_queue)) {
       	//printk(KERN_ERR "MSVDXQUE: msvdx list empty\n");
        platform->msvdx_busy = 0;
        return -EINVAL;
    }

	//printk(KERN_INFO "MSVDXQUE:  get from the msvdx list\n");
    msvdx_cmd =  list_first_entry(&platform->msvdx_queue,
				struct msvdx_cmd_queue, head);

    ret = process_mtx_messages(context, msvdx_cmd->cmd, msvdx_cmd->cmd_size, platform->msvdx_fence);

	jiffies_at_last_dequeue = jiffies;

    if (ret) {
        printk(KERN_ERR "MSVDXQUE: process_mtx_messages failed\n");
        ret = -EINVAL;
    }

    list_del(&msvdx_cmd->head);
	msvdx_cmd->cmd = NULL;
	kfree(msvdx_cmd);

    return ret;
}

unsigned long populate_fence_id(igd_context_t *context, unsigned long *mtx_msgs,
		unsigned long mtx_msg_cnt)
{
	platform_context_plb_t *platform;
	unsigned long submit_size = 0;
	unsigned long submit_id = 0;
	unsigned long context_id = 0;
	unsigned int msg;

	platform = (platform_context_plb_t *)context->platform_context;

	for (msg = 0; msg < mtx_msg_cnt; msg++) {
		submit_size = (mtx_msgs[0] & 0x000000ff);
		submit_id = (mtx_msgs[0] & 0x0000ff00) >> 8;

		if (submit_id == IGD_MSGID_RENDER) {
			mtx_msgs[4] = ++platform->msvdx_fence;
			//printk(KERN_INFO "Update fence id %lx\n", mtx_msgs[4]);
		}

		context_id = mtx_msgs[3];
		mtx_msgs += (submit_size / sizeof(unsigned long));
	}

	return context_id;
}

/*
 * The incoming command buffer holds MTX firmware messages, register pairs,
 * and the LLDMA linked lists. There is a simple header at the beginning of
 * the buffer that has the MTX message offset and the number of MTX messages.
 *
 * To process this buffer, find the MTX firmware messages and send each
 * one to the MTX firmware.
 */
int process_mtx_messages(igd_context_t *context,
		unsigned long *mtx_msgs, unsigned long mtx_msg_cnt,
		unsigned long fence)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	platform_context_plb_t *platform;
	unsigned long submit_size;
	unsigned long submit_id;
	unsigned int msg;
	unsigned long skipped_msg_cnt;
    unsigned long msvdx_status;

	EMGD_TRACE_ENTER;

	platform = (platform_context_plb_t *)context->platform_context;

    // message processing is about to start .. set the flag=bit 2
    spin_lock(&platform->msvdx_init_plb);
    platform->msvdx_status = platform->msvdx_status | 2;
    msvdx_status = platform->msvdx_status;
    spin_unlock(&platform->msvdx_init_plb);

	if (msvdx_status & 1)
	{
	    // OOPS: reset/fw load in progress ... return from here
        spin_lock(&platform->msvdx_init_plb);
        platform->msvdx_status = platform->msvdx_status & ~2;  // unset message processing status.
        spin_unlock(&platform->msvdx_init_plb);

        return 0;
    }

	save_msg = mtx_msgs;
	save_msg_cnt = mtx_msg_cnt;
	skipped_msg_cnt = 0;

	for (msg = 0; msg < mtx_msg_cnt; msg++) {

		if(!mtx_msgs) {
			printk(KERN_ERR "Invalid message");
			return -IGD_ERROR_INVAL;
		}

		submit_size = (mtx_msgs[0] & 0x000000ff);
		submit_id = (mtx_msgs[0] & 0x0000ff00) >> 8;

		if (submit_id != IGD_MSGID_RENDER) {
			/* Error, unknown message id, skip it */
			EMGD_ERROR("Unknown MTX message id 0x%lx", submit_id);
			skipped_msg_cnt++;
			continue;
		}

		if(!(mtx_msgs + sizeof(unsigned long))) {
			printk(KERN_ERR "Invalid message");
			return -IGD_ERROR_INVAL;
		}

		/* reuse the sgx phy PD */
		mtx_msgs[1] = platform->psb_cr_bif_dir_list_base1 | 1;

		/*
		 * If the send returns busy, then retry sending the message, otherwise
		 * move to next message in buffer.
		 */
		if (send_to_mtx(context, (unsigned long *)mtx_msgs) != -IGD_ERROR_BUSY) {
			mtx_msgs += (submit_size / sizeof(unsigned long));
			/*
			 * msvdx_mtx_interrupt_plb checks the firmware-to-host buffer
			 * and if there are messages there, processes them.
			 */
			 //msvdx_mtx_interrupt_plb(context);
		} else {
			msg--; /* Reset count back to unsent message */
			/* Should this wait a bit? */
		}
	}

    // We are done processing messages .. unset the flag
	spin_lock(&platform->msvdx_init_plb);
    platform->msvdx_status = platform->msvdx_status & ~2;
    spin_unlock(&platform->msvdx_init_plb);

	EMGD_TRACE_EXIT;
	if (skipped_msg_cnt == mtx_msg_cnt) {
		/* We failed to submit anything; the entire buffer was bad.
		 * Just return a failure code so we can unmark the video engine
		 * busy bit. */
		return -IGD_ERROR_INVAL;
	} else {
		return 0;
	}
}




/*
 * This is the function that actually passes the message to the MTX
 * firmware. The contents of the message are opaque to this function.
 *
 * Currently, the only supported message type is RENDER.
 */
int send_to_mtx(igd_context_t *context, unsigned long *msg)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long pad_msg;
	unsigned long num_words;
	unsigned long words_free;
	unsigned long read_idx, write_idx;
	platform_context_plb_t *platform =
		(platform_context_plb_t *)context->platform_context;
	int padding_flag = 0;

	EMGD_TRACE_ENTER;

	/* Enable all clocks before touching VEC local ram */
	EMGD_WRITE32(PSB_CLK_ENABLE_ALL, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

	/* The first two longs in the msg have the message ID and size */
	num_words = ((msg[0] & 0xff) + 3) / 4;

	/* Is message too big? */
	if (num_words > platform->mtx_buf_size) {
		/* TODO: Error ? */
		EMGD_ERROR("Message is too large (size=%ld, max=%ld).", num_words,
				platform->mtx_buf_size);
		return -IGD_ERROR_INVAL;
	}

	/*
	 * Safely increment the "number of messages in flight" counter, but only
	 * if this isn't a "padding" message.
	 */
	/*
	if (((msg[0] & 0xff00) >> 8) != FWRK_MSGID_PADDING) {
		platform->mtx_submitted++;
	}
	*/

	/*
	 * Make sure the MTX is enabled
	 */
	EMGD_WRITE32(MSVDX_MTX_ENABLE_MTX_ENABLE_MASK, mmio + PSB_MSVDX_MTX_ENABLE);


	read_idx  = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_MTX_RD_INDEX);
	write_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);

	EMGD_DEBUG("MTX read = 0x%08lx  write = 0x%08lx", read_idx, write_idx);

	/*
	 * Check to see if there is room for this message, if not send
	 * a pad message to use up all the space and wait for the next
	 * slot.
	 */
	if ((write_idx + num_words) > platform->mtx_buf_size) {
		/*
		 * if the read pointer is at zero, then the engine is probably
		 * hung and not processing the message. This is bad.
		 */
		if (read_idx == 0) {
			platform->msvdx_needs_reset = 1;
			EMGD_ERROR("MSVDX Engine is hung? Aborting send.");
			DUMP_ALL_MESSAGES(context);
			return -IGD_ERROR_BUSY;
		}

		/*
		 * The message id and size are encoded into the first word of
		 * the message.
		 * bits  0:7  size (in long words?)
		 * bits  8:15 message id
		 */
		pad_msg = (platform->mtx_buf_size - write_idx) << 2; /* size */
		pad_msg |= (FWRK_MSGID_PADDING << 8); /* message id */
		EMGD_DEBUG("Sending a pad_mesg: 0x%x, size = %ld", FWRK_MSGID_PADDING,
				(pad_msg & 0xff));

		/*
		 * Maybe just try writing the message directly here, instead of calling
		 * this fuction recursivly??
		 */
		EMGD_WRITE32(pad_msg,
				mmio + platform->mtx_buf_offset + (write_idx << 2));
		write_idx = 0;

		/* Update the write index to the next free location */
		EMGD_WRITE32(write_idx, mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);
		EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);

		padding_flag = 1;
	}

	/* Verify free space available */
	words_free = (write_idx >= read_idx) ?
		platform->mtx_buf_size - (write_idx - read_idx) : read_idx - write_idx;

	if (num_words > (words_free - 1)) {
		/* There is no space available, this isn't an error */

		if (padding_flag) {
			/* Make sure clocks are enabled before we kick */
			EMGD_WRITE32(PSB_CLK_ENABLE_ALL, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

			/* Send an interrupt to the MTX to let it know about the message */
			EMGD_WRITE32(1, mmio + PSB_MSVDX_MTX_KICK);
		}

		return - IGD_ERROR_BUSY;
	}

	/*
	 * DEBUGGING info:
	 *  Call a function to try and output some debugging info about the message
	 */
	/* DEBUG_MESG_INFO(context, msg, num_words); */

	/* Write the message to the firmware */
	while (num_words > 0) {
		EMGD_WRITE32(*msg++,
				mmio + platform->mtx_buf_offset + (write_idx << 2));
		num_words--;
		write_idx++;
	}

	/* Check for wrap in the buffer */
	if (write_idx == platform->mtx_buf_size) {
		write_idx = 0;
	}

	/* Update the write index to the next free location */
	EMGD_WRITE32(write_idx, mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);

	/* Check for overwrite */
	if (write_idx == read_idx) {
		EMGD_ERROR("Overwrite detected, resetting MSVDX.");
		platform->msvdx_needs_reset = 1;
	}

	/* Send an interrupt to the MTX to let it know about the message */
	EMGD_WRITE32(1, mmio + PSB_MSVDX_MTX_KICK);

	/* Read MSVDX Register several times in case idle signal assert */		
	EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);
	EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);
	EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);
	EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);
	

#if 0
	DEBUG_DUMP(context); /* For lots of additional debugging info */
#endif

	EMGD_TRACE_EXIT;
	return IGD_SUCCESS;
}


/*
 * This function will look at messages sent from the MTX Firmware
 * and decode them.
 */

void msvdx_mtx_interrupt_plb(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long msg[128];
	unsigned long read_idx, write_idx;
	unsigned long num_words;
	unsigned long msg_offset;
	unsigned long fence = 0;
    unsigned long flags = 0;
	unsigned long status;
	platform_context_plb_t *platform =
		(platform_context_plb_t *)context->platform_context;

	/* Check that the clocks are enabled before trying to read message */
	EMGD_WRITE32(PSB_CLK_ENABLE_ALL, mmio + PSB_MSVDX_MAN_CLK_ENABLE);

	do {
		read_idx  = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_HOST_RD_INDEX);
		write_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_HOST_WRT_INDEX);
		EMGD_DEBUG("HOST read=0x%lx write=0x%lx", read_idx, write_idx);

		if (read_idx != write_idx) {
			/* Read the first waiting message from the buffer */
			msg_offset = 0;
			msg[msg_offset] = EMGD_READ32(mmio + (platform->host_buf_offset +
						(read_idx << 2)));

			/* Size of message rounded to nearest number of words */
			num_words = ((msg[0] & 0x000000ff) + 3) / 4;

			if (++read_idx >= platform->host_buf_size) {
				read_idx = 0;
			}

			for (msg_offset++; msg_offset < num_words; msg_offset++) {
				msg[msg_offset] = EMGD_READ32(mmio + platform->host_buf_offset +
						(read_idx << 2));

				if (++read_idx >= platform->host_buf_size) {
					read_idx = 0;
				}
			}

			EMGD_WRITE32(read_idx, mmio + PSB_MSVDX_COMMS_TO_HOST_RD_INDEX);

			/* Check message ID */
			switch ((msg[0] & 0x0000ff00) >> 8) {
			case IGD_MSGID_CMD_FAILED:
				fence  = msg[1]; /* Fence value */
				status = msg[2]; /* Failed IRQ Status */
				printk(KERN_ERR "MSGID_CMD, status = 0x%lx, "
					"fence = 0x%lx\n", status, fence);

				/* This message from MTX is for informational purposes and
				   firmware does not expect hardware to be reset */
				platform->msvdx_needs_reset = 1;

				if (msg[1] != 0) {
					platform->mtx_completed = fence;
				}
				DUMP_ALL_MESSAGES(context);
				goto done;
				break;
			case IGD_MSGID_CMD_COMPLETED:
				fence = msg[1];
                flags = msg[2];

				//printk(KERN_INFO "MSGID_CMD_COMPLETED: fence 0x%lx\n", fence);
				platform->mtx_completed = fence;
				jiffies_at_last_dequeue = 0;

                if (flags & FW_VA_RENDER_HOST_INT) {
                    spin_lock(&platform->msvdx_lock);
                    msvdx_dequeue_send(context);
                    spin_unlock(&platform->msvdx_lock);
                }

				break;
			case IGD_MSGID_CMD_HW_PANIC:
				fence  = msg[1]; /* Fence value */
				status = msg[2]; /* Failed IRQ Status */
				printk(KERN_ERR "MSGID_CMD_HW_PANIC: fence 0x%08lx, "
					"Irq 0x%08lx\n", msg[1], msg[2]);
				platform->msvdx_needs_reset = 1;
				platform->mtx_completed = fence;
				DUMP_ALL_MESSAGES(context);
				goto done;
				break;
			default:
				EMGD_ERROR("Unknown message ID 0x%lx response from firmware.", (msg[0] & 0x0000ff00) >> 8);
				break;
			}

		}
	} while (read_idx != write_idx);

done:
	return;
}


/*
 * Simple function to check for a MSVDX interrupt.  Clear the interrupt status
 * if one occured.  Mostly just ignore them for now.
 */

#define MSVDX_MMU_FAULT_IRQ_MASK                0x00000F00
#define MSVDX_MTX_IRQ_MASK                      0x00004000
#define MSVDX_MMU_CONTROL0_CR_MMU_PAUSE_MASK    0x00000002

int msvdx_poll_mtx_irq(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long status;
	platform_context_plb_t *platform =
		(platform_context_plb_t *)context->platform_context;
	int poll_cnt = 100;

	EMGD_TRACE_ENTER;

	while (poll_cnt) {
		status = EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);
		if (status & MSVDX_MMU_FAULT_IRQ_MASK) {
			EMGD_DEBUG("MMU FAULT Interrupt");
			/* Pause the MMU */
			EMGD_WRITE32(MSVDX_MMU_CONTROL0_CR_MMU_PAUSE_MASK,
					mmio + PSB_MSVDX_MMU_CONTROL0);

			/* Clear interrupt bit */
			EMGD_WRITE32(MSVDX_MMU_FAULT_IRQ_MASK,
					mmio + PSB_MSVDX_INTERRUPT_CLEAR);
			EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_CLEAR);

			platform->msvdx_needs_reset = 1;
			DUMP_ALL_MESSAGES(context);
			return 0;
		} else if (status & MSVDX_MTX_IRQ_MASK) {
			/* Clear all interrupt bits */
			EMGD_WRITE32(0xffffffff, mmio + PSB_MSVDX_INTERRUPT_CLEAR);
			EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_CLEAR);

			msvdx_mtx_interrupt_plb(context);
			return 0;
		} else if (status) {
			/* Might want to make this debug only. */
			if (status & 0x00000001) EMGD_DEBUG("IRQ - VEC_END_OF_SLICE");
			if (status & 0x00000002) EMGD_DEBUG("IRQ - VEC_ERROR_DETECTETED_SR");
			if (status & 0x00000004) EMGD_DEBUG("IRQ - VEC_ERROR_DETECTETED_ENTDEC");
			if (status & 0x00000008) EMGD_DEBUG("IRQ - VEC_RENDEC_ERROR");
			if (status & 0x00000010) EMGD_DEBUG("IRQ - VEC_RENDEC_OVERFLOW");
			if (status & 0x00000020) EMGD_DEBUG("IRQ - VEC_RENDEC_UNDERFLOW");
			if (status & 0x00000040) EMGD_DEBUG("IRQ - VEC_RENDEC_MTXBLOCK");
			if (status & 0x00000080) EMGD_DEBUG("IRQ - VEC_RENDEC_END_OF_SLICE");
			if (status & 0x00000100) EMGD_DEBUG("IRQ - MMU_FAULT_IRQ");
			if (status & 0x00000200) EMGD_DEBUG("IRQ - MMU_FAULT_IRQ");
			if (status & 0x00000400) EMGD_DEBUG("IRQ - MMU_FAULT_IRQ");
			if (status & 0x00000800) EMGD_DEBUG("IRQ - MMU_FAULT_IRQ");
			if (status & 0x00001000) EMGD_DEBUG("IRQ - FE_WDT_CM0");
			if (status & 0x00002000) EMGD_DEBUG("IRQ - FE_WDT_CM1");
			if (status & 0x00004000) EMGD_DEBUG("IRQ - MTX_IRQ");
			if (status & 0x00008000) EMGD_DEBUG("IRQ - MTX_GPIO_IRQ");
			if (status & 0x00010000) EMGD_DEBUG("IRQ - VDMC_IRQ");
			if (status & 0x00020000) EMGD_DEBUG("IRQ - VDEB_PICTURE_DONE_IRQ");
			if (status & 0x00040000) EMGD_DEBUG("IRQ - VDEB_SLICE_DONE_IRQ");
			if (status & 0x00080000) EMGD_DEBUG("IRQ - VDEB_FLUSH_DONE_IRQ");
			if (status & 0x00100000) EMGD_DEBUG("IRQ - DMAC_IRQ");
			if (status & 0x00200000) EMGD_DEBUG("IRQ - DMAC_IRQ");
			if (status & 0x00400000) EMGD_DEBUG("IRQ - DMAC_IRQ");
			if (status & 0x00800000) EMGD_DEBUG("IRQ - VDEB_FAULT_IRQ");
			if (status & 0x01000000) EMGD_DEBUG("IRQ - SYS_COMMAND_TIMEOUT_IRQ");
			if (status & 0x02000000) EMGD_DEBUG("IRQ - SYS_READ_TIMEOUT_IRQ");
			if (status & 0x04000000) EMGD_DEBUG("IRQ - MTX_COMMAND_TIMEOUT_IRQ");
			if (status & 0x08000000) EMGD_DEBUG("IRQ - MTX_READ_TIMEOUT_IRQ");
			if (status & 0x10000000) EMGD_DEBUG("IRQ - SYS_WDT");
			if (status & 0x20000000) EMGD_DEBUG("IRQ - BE_WDT_CM0");
			if (status & 0x40000000) EMGD_DEBUG("IRQ - BE_WDT_CM1");
			if (status & 0x80000000) EMGD_DEBUG("IRQ - VEC_RENDEC_SLICE_SKIPPED");

			EMGD_DEBUG("  Watchdog Control = 0x%x  0x%x  0x%x",
					EMGD_READ32(mmio + MSVDX_BASE + 0x0670),
					EMGD_READ32(mmio + MSVDX_BASE + 0x0674),
					EMGD_READ32(mmio + MSVDX_BASE + 0x0678));

			/* Clear the interrupt */
			EMGD_WRITE32(status, mmio + PSB_MSVDX_INTERRUPT_CLEAR);
			EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_CLEAR);
			return 0;
		}

		poll_cnt--;
		OS_SLEEP(10);
	}

	EMGD_ERROR("Timout polling interrupt status.");
	return 1;
}


/*
 * msvdx_mtx_irq
 *
 *  Process a MTX interrupt.  Look at the interrupt status bits
 *  and do the processing dictated by the bits.  Currently only
 *  two interrupts are really handled:
 *     MMU Fault, which pauses the MMU
 *     MTX IRQ, which then processes any firmware to hosts messages
 *  Other interrupts will output a debugging message.  These are fault
 *  conditions that are not handled.
 *
 *  return 0 if the IRQ was handled
 *  return 1 if it wasn't
 */
//static void msvdx_mtx_irq(struct drm_device *dev)
IMG_BOOL msvdx_mtx_isr(IMG_VOID *pvData)
{
    struct drm_device *dev;
    drm_emgd_priv_t *priv;
    igd_context_t *context;
	unsigned char *mmio;
	platform_context_plb_t *platform;
    unsigned long msvdx_stat,temp;

	//EMGD_TRACE_ENTER;
    dev = (struct drm_device *)pvData;
    priv = dev->dev_private;
    context = priv->context;
    mmio =  context->device_context.virt_mmadr;
    platform = context->platform_context;

    msvdx_stat = EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_STATUS);

	//printk(KERN_ALERT "MSVDX_IRQ\n");

	if (msvdx_stat & MSVDX_MMU_FAULT_IRQ_MASK) {
		EMGD_DEBUG(KERN_ERR "MMU FAULT Interrupt\n");
		EMGD_DEBUG(KERN_ERR "INTERRUPT_STATUS Register=0x%x\n",msvdx_stat);
		temp = EMGD_READ32(mmio + PSB_MSVDX_MMU_DIR_LIST_BASE0);
        EMGD_DEBUG(KERN_ERR "MMU_DIR_LIST_BASE0 = 0x%x\n", temp);
	    	temp = EMGD_READ32(mmio + PSB_MSVDX_MMU_DIR_LIST_BASE1);
		EMGD_DEBUG(KERN_ERR "MMU_DIR_LIST_BASE1 = 0x%x\n", temp);
	    	temp = EMGD_READ32(mmio + PSB_MSVDX_MMU_DIR_LIST_BASE2);
		EMGD_DEBUG(KERN_ERR "MMU_DIR_LIST_BASE2 = 0x%x\n", temp);
	    	temp = EMGD_READ32(mmio + PSB_MSVDX_MMU_DIR_LIST_BASE3);
		EMGD_DEBUG(KERN_ERR "MMU_DIR_LIST_BASE3 = 0x%x\n", temp);
		/* Pause the MMU */
		EMGD_WRITE32(MSVDX_MMU_CONTROL0_CR_MMU_PAUSE_MASK,
				mmio + PSB_MSVDX_MMU_CONTROL0);


		EMGD_WRITE32(MSVDX_MMU_FAULT_IRQ_MASK,
				mmio + PSB_MSVDX_INTERRUPT_CLEAR);

		//printk(KERN_INFO "FAULT ADDR=%x\n", EMGD_READ32(mmio + PSB_MSVDX_MMU_STATUS));
		platform->msvdx_needs_reset = 1;
		DUMP_ALL_MESSAGES(context);
		//return 0;
	} else if (msvdx_stat & MSVDX_MTX_IRQ_MASK) {
		/* Read the firmware to host messages */

        EMGD_WRITE32(0xffffffff, mmio + PSB_MSVDX_INTERRUPT_CLEAR);
		EMGD_READ32(mmio + PSB_MSVDX_INTERRUPT_CLEAR);
		msvdx_mtx_interrupt_plb(context);
		//return 0;
	} else if (msvdx_stat) {
		/*
		 * Print a debug message for any other interrupt cases.
		 */
		if (msvdx_stat & 0x00000001) printk(KERN_INFO "IRQ - VEC_END_OF_SLICE\n");
		if (msvdx_stat & 0x00000002) printk(KERN_INFO "IRQ - VEC_ERROR_DETECTETED_SR\n");
		if (msvdx_stat & 0x00000004) printk(KERN_INFO "IRQ - VEC_ERROR_DETECTETED_ENTDEC\n");
		if (msvdx_stat & 0x00000008) printk(KERN_INFO "IRQ - VEC_RENDEC_ERROR\n");
		if (msvdx_stat & 0x00000010) printk(KERN_INFO "IRQ - VEC_RENDEC_OVERFLOW\n");
		if (msvdx_stat & 0x00000020) printk(KERN_INFO "IRQ - VEC_RENDEC_UNDERFLOW\n");
		if (msvdx_stat & 0x00000040) printk(KERN_INFO "IRQ - VEC_RENDEC_MTXBLOCK\n");
		if (msvdx_stat & 0x00000080) printk(KERN_INFO "IRQ - VEC_RENDEC_END_OF_SLICE\n");
		if (msvdx_stat & 0x00000100) printk(KERN_INFO "IRQ - MMU_FAULT_IRQ\n");
		if (msvdx_stat & 0x00000200) printk(KERN_INFO "IRQ - MMU_FAULT_IRQ\n");
		if (msvdx_stat & 0x00000400) printk(KERN_INFO "IRQ - MMU_FAULT_IRQ\n");
		if (msvdx_stat & 0x00000800) printk(KERN_INFO "IRQ - MMU_FAULT_IRQ\n");
		if (msvdx_stat & 0x00001000) printk(KERN_INFO "IRQ - FE_WDT_CM0\n");
		if (msvdx_stat & 0x00002000) printk(KERN_INFO "IRQ - FE_WDT_CM1\n");
		if (msvdx_stat & 0x00004000) printk(KERN_INFO "IRQ - MTX_IRQ\n");
		if (msvdx_stat & 0x00008000) printk(KERN_INFO "IRQ - MTX_GPIO_IRQ\n");
		if (msvdx_stat & 0x00010000) printk(KERN_INFO "IRQ - VDMC_IRQ\n");
		if (msvdx_stat & 0x00020000) printk(KERN_INFO "IRQ - VDEB_PICTURE_DONE_IRQ\n");
		if (msvdx_stat & 0x00040000) printk(KERN_INFO "IRQ - VDEB_SLICE_DONE_IRQ\n");
		if (msvdx_stat & 0x00080000) printk(KERN_INFO "IRQ - VDEB_FLUSH_DONE_IRQ\n");
		if (msvdx_stat & 0x00100000) printk(KERN_INFO "IRQ - DMAC_IRQ\n");
		if (msvdx_stat & 0x00200000) printk(KERN_INFO "IRQ - DMAC_IRQ\n");
		if (msvdx_stat & 0x00400000) printk(KERN_INFO "IRQ - DMAC_IRQ\n");
		if (msvdx_stat & 0x00800000) printk(KERN_INFO "IRQ - VDEB_FAULT_IRQ\n");
		if (msvdx_stat & 0x01000000) printk(KERN_INFO "IRQ - SYS_COMMAND_TIMEOUT_IRQ\n");
		if (msvdx_stat & 0x02000000) printk(KERN_INFO "IRQ - SYS_READ_TIMEOUT_IRQ\n");
		if (msvdx_stat & 0x04000000) printk(KERN_INFO "IRQ - MTX_COMMAND_TIMEOUT_IRQ\n");
		if (msvdx_stat & 0x08000000) printk(KERN_INFO "IRQ - MTX_READ_TIMEOUT_IRQ\n");
		if (msvdx_stat & 0x10000000) printk(KERN_INFO "IRQ - SYS_WDT\n");
		if (msvdx_stat & 0x20000000) printk(KERN_INFO "IRQ - BE_WDT_CM0\n");
		if (msvdx_stat & 0x40000000) printk(KERN_INFO "IRQ - BE_WDT_CM1\n");
		if (msvdx_stat & 0x80000000) printk(KERN_INFO "IRQ - VEC_RENDEC_SLICE_SKIPPED\n");

		printk(KERN_INFO "  Watchdog Control = 0x%x  0x%x  0x%x\n",
				EMGD_READ32(mmio + MSVDX_BASE + 0x0670),
				EMGD_READ32(mmio + MSVDX_BASE + 0x0674),
				EMGD_READ32(mmio + MSVDX_BASE + 0x0678));

		return 0;
	}
    /*
    else {
		return ;
	} */
    return IMG_TRUE;
}


#ifdef DEBUG_BUILD_TYPE

static void dump_all_messages(igd_context_t *context)
{
	unsigned long msg;
	unsigned long submit_size;
	unsigned long num_words;

	if (save_msg) {
		for (msg = 0; msg < save_msg_cnt; msg++) {
			submit_size = (save_msg[0] & 0x000000ff);
			num_words = ((save_msg[0] & 0xff) + 3) / 4;

			DEBUG_MESG_INFO(context, save_msg, num_words);

			save_msg += (submit_size / sizeof(unsigned long));
		}

		save_msg = NULL;
		save_msg_cnt = 0;
	}
}

/*
 * This function is for debugging firmware messages. It should not be part of the
 * production code.   Make sure it is removed (if def'ed out) before building a
 * production version.
 */

static void debug_mesg_info(igd_context_t *context,
		unsigned long *msg,
		unsigned long num_words)
{
	unsigned long msg_size;
	unsigned char msg_id;
	unsigned short buffer_size;
	unsigned long buffer_address;
	unsigned short last_mb;
	unsigned short first_mb;
	unsigned long *cmd_base;

	msg_size = (msg[0] & 0xff);
	msg_id = (unsigned char)((msg[0] & 0xff00) >> 8);
	buffer_size = (unsigned short)((msg[0] & 0xffff0000) >> 16);

	EMGD_DEBUG("MSG: size = %ld, id = 0x%x, buffer_size = %d",
			msg_size, msg_id, buffer_size);
	if (msg_size > 4) {
		EMGD_DEBUG("MSG: MMUPTD         = 0x%08lx", msg[1]);
	}
	if (msg_size > 8) {
		buffer_address = msg[2];
		/* FIXME - The following line of code is wrong.  The buffer_address is
		 * not an offset to GMM-allocated memory, and so cmd_base is always
		 * NULL (see fixme in lldma_dump()).  The buffer_address is an offset
		 * within a larger buffer allocated through the user-space PVR2D code
		 * (and thus, allocated by PVRSRV).  Unless the PVRSRV mapping between
		 * buffer_address and a kernel-visible address can be found, this
		 * buffer cannot be accessed, as this code is being called from a
		 * kernel interrupt handler, which has no access to a user-space
		 * process's memory.
		 */
		cmd_base = (unsigned long *)context->dispatch.gmm_map(buffer_address);
		EMGD_DEBUG("MSG: Buffer Offset  = 0x%08lx", buffer_address);
		EMGD_DEBUG("MSG: Context id     = 0x%lx", msg[3]);
		EMGD_DEBUG("MSG: Fence value    = 0x%lx", msg[4]);
		EMGD_DEBUG("MSG: Operating Mode = 0x%lx", msg[5]);
		last_mb = (unsigned short)((msg[6] & 0xffff0000) >> 16);
		first_mb = (unsigned short)(msg[6] & 0x0000ffff);
		EMGD_DEBUG("MSG: First MB       = %d, last mb = %d", first_mb, last_mb);
		EMGD_DEBUG("MSG: Flags          = 0x%lx", msg[7]);

		/* Dump some of the lldma contents here */
		LLDMA_DUMP(cmd_base, buffer_address);
	}
}


/* To eliminate warnings, am temporarily #ifdef'ing this function.  Please
 * remove the #ifdef when this function is to be used.
 */
#ifdef USE_DEBUG_DUMP
/*
 * This function dumps the to firmware message buffer.
 */
static void debug_dump(igd_context_t *context)
{
	unsigned char *mmio = context->device_context.virt_mmadr;
	unsigned long read_idx, write_idx;
	unsigned long i;
	platform_context_plb_t *platform;

	platform = (platform_context_plb_t *)context->platform_context;
	read_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_HOST_RD_INDEX);
	write_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_HOST_WRT_INDEX);
	EMGD_DEBUG("HOST buffer: RDIDX 0x%08lx   WRIDX 0x%08lx, %lu",
		read_idx, write_idx, platform->host_buf_size);
	for (i = 0; (i < read_idx || i < write_idx); i++) {
		unsigned long value =
			EMGD_READ32(mmio + platform->host_buf_offset + (i <<2));
		EMGD_DEBUG("   %02lx: 0x%08lx", i, value);
	}

	read_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_MTX_RD_INDEX);
	write_idx = EMGD_READ32(mmio + PSB_MSVDX_COMMS_TO_MTX_WRT_INDEX);
	EMGD_DEBUG("MTX buffer: RDIDX 0x%08lx   WRIDX 0x%08lx, %lu",
		read_idx, write_idx, platform->mtx_buf_size);
	for (i = 0; (i < read_idx || i < write_idx); i++) {
		unsigned long value =
			EMGD_READ32(mmio + platform->mtx_buf_offset + (i <<2));
		EMGD_DEBUG("   %02lx: 0x%08lx", i, value);
	}
}
#endif

unsigned long lldma_decode(unsigned long val,
		unsigned long to,
		unsigned long from)
{
	if (to < 31) {
		val &= ~(0xffffffff << (to + 1));
	}
	val = val >> from;

	return val;
}

static void lldma_dump(unsigned long *cmd_base, unsigned long offset)
{
	int i;

	/* FIXME - When the code in debug_mesg_info() is fixed (see the fixme in
	 * that function), the following if-return code can be removed.
	 */
	if (cmd_base == NULL) {
		return;
	}

	for (i = 0; i < 8; i++) {
		if (i == 0) {
			EMGD_DEBUG(" 0x%08lx LLDMA: BSWAP     = %ld", offset, lldma_decode(*cmd_base, 31, 31));
			EMGD_DEBUG("            LLDMA: DIR       = %ld", lldma_decode(*cmd_base, 30, 30));
			EMGD_DEBUG("            LLDMA: PW        = %ld", lldma_decode(*cmd_base, 29, 28));
		} else if (i == 1) {
			EMGD_DEBUG(" 0x%08lx LLDMA: List_Fin  = %ld", offset+1, lldma_decode(*cmd_base, 31, 31));
			EMGD_DEBUG("            LLDMA: List_INT  = %ld", lldma_decode(*cmd_base, 30, 30));
			EMGD_DEBUG("            LLDMA: PI        = %ld", lldma_decode(*cmd_base, 18, 17));
			EMGD_DEBUG("            LLDMA: INCR      = %ld", lldma_decode(*cmd_base, 16, 16));
			EMGD_DEBUG("            LLDMA: LEN       = %ld", lldma_decode(*cmd_base, 15, 0));
		} else if (i == 2) {
			EMGD_DEBUG(" 0x%08lx LLDMA: ADDR      = 0x%lx", offset+2, lldma_decode(*cmd_base, 22, 0));
		} else if (i == 3) {
			EMGD_DEBUG(" 0x%08lx LLDMA: ACC_DEL   = %ld", offset+3, lldma_decode(*cmd_base, 31, 29));
			EMGD_DEBUG("            LLDMA: BURST     = %ld", lldma_decode(*cmd_base, 28, 26));
			EMGD_DEBUG("            LLDMA: EXT_SA    = 0x%lx", lldma_decode(*cmd_base, 3, 0));
		} else if (i == 4) {
			EMGD_DEBUG(" 0x%08lx LLDMA: 2D_MODE   = %ld", offset+4, lldma_decode(*cmd_base, 16, 16));
			EMGD_DEBUG("            LLDMA: REP_CNT   = %ld", lldma_decode(*cmd_base, 10, 0));
		} else if (i == 5) {
			EMGD_DEBUG(" 0x%08lx LLDMA: LINE_ADD  = 0x%lx", offset+5, lldma_decode(*cmd_base, 25, 16));
			EMGD_DEBUG("            LLDMA: ROW_LEN   = %ld", lldma_decode(*cmd_base, 9, 0));
		} else if (i == 6) {
			EMGD_DEBUG(" 0x%08lx LLDMA: SA        = 0x%lx", offset+6, lldma_decode(*cmd_base, 31, 0));
		} else if (i == 7) {
			EMGD_DEBUG(" 0x%08lx LLDMA: LISTPTR   = 0x%lx", offset+7, lldma_decode(*cmd_base, 27, 0));
		}
		cmd_base++;
	}
}
#endif

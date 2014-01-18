/*
 *-----------------------------------------------------------------------------
 * Filename: rb.h
 * $Revision: 1.11 $
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
 *  This header file describes the inter-module interface for using the
 *  ring buffer. This is a legacy interface that is not strictly
 *  device-independent. It is kept here because many legacy chipsets use
 *  this interface.
 *-----------------------------------------------------------------------------
 */

#ifndef _RB_H
#define _RB_H

#include <igd_rb.h>
#include <igd_errno.h>

#include <context.h>
#include <module_init.h>
#include <mode.h>

#include <io.h>
#include <memory.h>
#include <sched.h>


/*
 * The rb_buffer contents should be treated as opaque but are defined
 * here for the use in the inline functions below.
 */
typedef struct _rb_buffer {
	unsigned int id;       /* ring index */
	unsigned long size;    /* size of rb in bytes */
	unsigned long tail_off;
	unsigned long dead_off;
	unsigned long avail;
	unsigned long addr;    /* starting address of rb */
	unsigned char *virt;    /* virt starting address of rb */
	unsigned char *start;   /* ring buffer start register offset */
	unsigned char *head;    /* ring buffer head register offset */
	unsigned char *tail;    /* ring buffer tail register offset */
	unsigned char *ctrl;    /* ring buffer ctrl register offset */
	unsigned char *sync;    /* used to synchronize s/w interface
								   * code via rb_sync() */
	unsigned long next_sync;
	int state;             /* on or off state */
	unsigned long last_head_value; /* used for checking ring buf lockup,
									* keeps last head value*/
	os_alarm_t last_head_time; /* used for checking ring buf lockup,
								* keeps last head time*/
	unsigned long reservation; /* increments upon each rb_reserver(),
								* reset to zero upon rb_update() */
	unsigned long sync_wrap;   /* Sync number to wrap at */
	igd_context_t * context_ptr; /*back pointer to the main driver context */
	igd_appcontext_h appcontext; /* Appcontext that is current on the ring */
        unsigned int skip_timeout;  /* skip the timeout check */
	unsigned int force_reset_rb;  /* force a rb reset */
} rb_buffer_t;


#define MODE_GET_RING(d, p) \
	((rb_buffer_t *)((igd_display_pipe_t *)((igd_display_context_t *)d)->pipe)->queue[p])

/*
 * Flags used exclusively by rb_reserve()
 */
#define RB_RESERVE_BLOCK     0x00000000
#define RB_RESERVE_NONBLOCK  0x00000001


/*
 * Determine how much space is available in the ring buffer.
 */
unsigned long rb_avail(rb_buffer_t *buffer);


#define MODE_PIPE_ALLOCED(d) \
	((igd_display_pipe_t *)((igd_display_context_t *)d)->pipe)

extern igd_command_t *rb_slow_reserve(rb_buffer_t *buffer,
	unsigned long size,
	unsigned long flags);
/*
 * For Debug do not use the inline ring functions. It makes it hard
 * to add breakpoints.
 */
#ifdef DEBUG_BUILD_TYPE
extern igd_command_t *rb_reserve(rb_buffer_t *buffer,
	unsigned long size,
	unsigned long flags);

extern int rb_update(rb_buffer_t *buffer,
	igd_command_t *addr);

#else
#ifdef CONFIG_CMD
static __inline igd_command_t *rb_reserve(rb_buffer_t *buffer,
	unsigned long size,
	unsigned long flags)
{
	if (buffer->state == CMD_CONTROL_OFF) {
		/* ring buffer is turned off, so don't allow a rb_reserve() */
		buffer->context_ptr->igd_device_error_no = -IGD_ERROR_PWRDOWN;
		return(NULL);
	}

	/*
	 * Change size to bytes for efficiency.
	 */
	size = ((size<<2) + 7) & ~7;

	if(buffer->reservation) {
		buffer->avail += buffer->reservation;
	}
	buffer->reservation = size;

	if(buffer->avail > size) {
		buffer->avail -= size;
		return (igd_command_t *)(buffer->virt + buffer->tail_off);
	}

	return rb_slow_reserve(buffer, size, flags);
}


static __inline int rb_update(rb_buffer_t *buffer,
	igd_command_t *addr)
{
	unsigned long tail_off;
/*	tail_off = (addr - buffer->virt); */
	tail_off = (unsigned long)((unsigned char *)(addr) - buffer->virt);

	buffer->reservation = 0;
	buffer->tail_off = tail_off;

	if(buffer->tail_off & 0x7) {
		EMGD_WRITE32(0, buffer->virt + buffer->tail_off);
		buffer->tail_off += 4;
		buffer->avail -= 4;
	}

	EMGD_WRITE32(buffer->tail_off, buffer->tail);

	return 0;
}
#else
static __inline igd_command_t *rb_reserve(rb_buffer_t *buffer,
	unsigned long size,
	unsigned long flags)
{
	return NULL;
}

static __inline int rb_update(rb_buffer_t *buffer,
	igd_command_t *addr)
{
	return 0;
}
#endif /* CONFIG_CMD */
#endif

#endif /* _RB_H */


/*-----------------------------------------------------------------------------
* Copyright (c) Chrontel Inc.
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
* @file  ch7036_port.h
* @version 1.2.2
*-----------------------------------------------------------------------------
*/


#ifndef _CH7036_PORT_H_
#define _CH7036_PORT_H_



#include "ch7036_intf.h"
#include "ch7036_attr.h"

int ch7036_open(pd_callback_t *p_callback, void **p_context);
int ch7036_init_device(void *p_context);
int ch7036_get_timing_list(void *p_context, pd_timing_t *p_in_list,
	pd_timing_t **pp_out_list);
int ch7036_set_mode(void *p_context, pd_timing_t *p_mode, unsigned long flags);
int ch7036_post_set_mode(void *p_context, pd_timing_t *p_mode,
	unsigned long flags);

int ch7036_get_attributes(void *p_context, unsigned long *p_num_attr,
	pd_attr_t **pp_list);
int ch7036_set_attributes(void *p_context, unsigned long num_attr,
	pd_attr_t *p_list);
unsigned long ch7036_validate(unsigned long cookie);
int ch7036_close(void *p_context);

int ch7036_set_power(void *p_context, unsigned long state);
int ch7036_get_power(void *p_context, unsigned long *p_state);
int ch7036_save(void *p_context, void **pp_state, unsigned long flags);
int ch7036_restore(void *p_context, void *p_state, unsigned long flags);
int ch7036_get_port_status(void *context, pd_port_status_t *port_status);


#endif 

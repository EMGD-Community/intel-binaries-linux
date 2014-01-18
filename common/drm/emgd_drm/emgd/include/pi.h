/*
 *-----------------------------------------------------------------------------
 * Filename: pi.h
 * $Revision: 1.7 $
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

#ifndef _PI_H_
#define _PI_H_

#include <context.h>
#include <mode.h>
#include <pd.h>

/* power states */
#define IGD_DEVICE_ON 1
#define IGD_DEVICE_OFF 0

#define PI_FIRMWARE_EDID          0x1
#define PI_FIRMWARE_DISPLAYID     0x2

/* get_native_dtd() flags */
#define PI_ALL_TIMINGS            0x0001
#define PI_SUPPORTED_TIMINGS      0x0002
extern unsigned long get_native_dtd(igd_timing_info_t *timing_table,
	unsigned long flags, pd_timing_t **native_dtd, unsigned long native_flags);

extern igd_timing_info_t crt_timing_table[];
extern int crt_timing_table_size;

#ifndef CONFIG_MICRO
extern igd_timing_info_t cea_timing_table[];
extern int cea_timing_table_size;
extern type_std_t cea_std_lookup[];
extern int cea_std_lookup_size;
#endif

extern int pi_pd_register(pd_driver_t *pd_driver);
extern int pi_pd_register(pd_driver_t *pd_driver);
extern int pi_read_regs(void *callback_context, pd_reg_t *list,
		unsigned long reg_type);
extern int pi_write_regs(void *callback_context, pd_reg_t *list,
		unsigned long reg_type);
extern int pi_program_port_dvo(igd_display_context_t *, unsigned long);
extern int pi_program_port_analog(igd_display_context_t *, unsigned long);
extern int pi_program_port_rgba(igd_display_context_t *, unsigned long);
extern int pi_program_port_lvds(igd_display_context_t *, unsigned long);

/* Function to get attr value from port driver attr list */
extern int pi_pd_find_attr_and_value(igd_display_port_t *port,
						  unsigned long attr_id,
						  unsigned long flag,
						  pd_attr_t   **caller_pd_attr,
						  unsigned long * attr_value);

/* Function to get attr value from user provided init attribute list */
extern int pi_get_port_init_attr(igd_display_port_t *port,
		unsigned long id,
		unsigned long *value);
extern int pi_save_mode_state(igd_display_port_t *port,
		reg_state_id_t reg_state_id);
#endif /* _PI_H_ */

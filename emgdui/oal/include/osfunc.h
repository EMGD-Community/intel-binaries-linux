/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: osfunc.h
 * $Revision: 1.4 $
 *-----------------------------------------------------------------------------
 * INTEL CONFIDENTIAL
 * Copyright (2002-2008) Intel Corporation All Rights Reserved.
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intel's prior express written permission.
 * 
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 * 
 * 
 *-----------------------------------------------------------------------------
 * Description:
 *  This file contains function headers and macros that abstract the OAL from
 *  other layers.
 *-----------------------------------------------------------------------------
 */

#ifndef _OSFUNCS_H
#define _OSFUNCS_H

#include <halcmn.h>
#include <emgd_info.h>
#include "intelpci.h"

#define ALL_DISPLAYS                0xFF
#define MAX_DISPLAYS                0x5
#define DEVICE_2                    2
#define DEVICE_3                    3

typedef struct _chipset_name_t {

	unsigned short chipset;
	char chipset_name[MAX_SIZE];

} chipset_name_t;

void display_error_message();

bool os_initialize(IN bool is_driver_in_binary);
bool os_uninitialize();

bool os_dump_regs(
	IN bool type_mmio,
	IN unsigned long reg,
	IN unsigned long size_in_ulong,
	OUT unsigned long *buffer);
bool os_dump_mem(
	IN unsigned long address,
	IN unsigned long offset,
	IN unsigned long size_in_ulong,
	OUT unsigned long *buffer);
bool os_get_chipset_name(OUT chipset_info_t *c_info);
bool os_get_cpu_info(OUT iegd_hal_info_t *args);
bool os_get_driver_version(OUT iegd_hal_info_t *args);
bool os_get_name(OUT os_info_t *os_info);
bool os_get_system_memory(OUT iegd_hal_info_t *args);
bool os_get_sys_bios_version(OUT iegd_hal_info_t *args);
bool os_read_reg_value(
	IN unsigned long type,
	IN unsigned long reg,
	OUT unsigned long *value,
	IN int device = DEVICE_2);
bool os_write_mem_value(
	IN bool type_mmio,
	IN unsigned long base,
	IN unsigned long address,
	IN unsigned long offset,
	IN unsigned long value,
	IN unsigned long length);
bool os_write_reg_value(
	IN bool type_mmio,
	IN unsigned long base,
	IN unsigned long reg,
	IN unsigned long value);
int os_send_escape(
	IN int escape,
	IN int input_size,
	IN char *input,
	IN int output_size,
	OUT char *output);
bool os_set_scrn_res(
	IN unsigned long screen_num = ALL_DISPLAYS,
	IN unsigned short width = 0,
	IN unsigned short height = 0,
	IN unsigned short refresh = 0,
	IN unsigned short bpp = 0,
	IN unsigned long rotation = 0,
	IN unsigned long force_reset = 0);
bool os_get_scrn_res(
	IN unsigned long screen_num,
	OUT unsigned long *width,
	OUT unsigned long *height,
	OUT unsigned long *refresh,
	OUT unsigned long *bpp,
	OUT unsigned long *rotation);
bool os_change_disp_settings(
	IN iegd_esc_set_dc_t *set_dc,
	IN bool was_extended_mode,
	IN scrn_info_t *scrn_info);
bool os_get_pci_info(
	IN unsigned short bus,
	IN unsigned short dev,
	IN unsigned short func,
	OUT void *pci_info);

bool os_is_funcX_enabled(IN int func);
unsigned short os_read_pci_word(IN int pos);
bool os_pci_system_init (void);
void os_pci_system_cleanup(void);
bool os_read_pci_config(OUT unsigned long *read_data);
bool os_write_pci_config(IN unsigned long offset);
bool os_wait(IN unsigned long num_seconds);
void os_get_full_path(IN const char *file, OUT char *full_path);
void os_get_intel_displays(
	OUT disp_port_info_t *disp_port_array,
	OUT unsigned long *disp_port_array_size);
void os_set_debug_volume(IN unsigned int debug_vol);
int is_emgd_driver(char *disp_name);
#endif

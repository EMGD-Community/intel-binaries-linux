/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: halcmn.h
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
 *  This file contains data structures, enums and function prototypes of
 *  all the things that are shared in between different layers. The OS
 *  abstraction layer, the Interface abstraction layer and the hardware
 *  abstraction layer.
 *-----------------------------------------------------------------------------
 */


#ifndef _HALCMN_H
#define _HALCMN_H

#include "emgd_info.h"

#define LAST_FUNC                   0xFFFF
#define C_ASSERT(e)                 typedef char __C_ASSERT__[(e)?1:-1]
#define INTEL_CORP                  "Intel Corporation"
#define EMGD_STRING                 "Embedded Media and Graphics Driver"
#define EMGD_ID						0xE1D0

/****************************************************************************
* This enum contains the return stages of each function within the libraries
* Each function can contain 3 return stages:
* RET_USAGE means that the caller is just querying the function for its usage.
* RET_EXECUTE_FUNC means that the caller is asking the function to execute
* its functionality.
****************************************************************************/
enum {
	RET_USAGE = 1,
	RET_EXECUTE_FUNC
};


typedef struct _iegd_hal_info {

	int current_args_filled;
	int total_args_filled;
	int retStage;
	bool more_args_to_fill;
	char arg[MAX_ARGS][MAX_SIZE];

} iegd_hal_info_t;

typedef struct _globals_t
{
	char prog_name[MAX_SIZE];
	char log_file[MAX_SIZE];
	int severity;
	int for_external_use;
} globals_t;


extern globals_t globals;

enum COMMON_FUNCS {

	GET_OS_NAME,
	GET_CHIPSET_NAME,
	GET_SYSTEM_MEMORY,
	GET_CPU_INFO,
	GET_DRIVER_VERSION,
	GET_PLANE_REGS,
	GET_PORT_REGS,
	GET_PIPE_REGS,
	GET_FUSE_REGS,
	GET_CURRENT_DISP_MODE,
	GET_CURRENT_DISP_CONFIG,
	GET_DISP_INFO,
	GET_SYS_BIOS_VERSION,
	GET_VBIOS_VERSION,
	GET_CMOS_SETTINGS,
	GET_VBIOS_SETTINGS,
	GET_DEVICES_INFO,
	GET_VGACNTRL,
	GET_REG_LVDS,
	READ_ERROR,
	GET_PTR_TAIL,
	GET_PTR_START,
	GET_RING_BACKTRACE,
	READ_MMIO,
	WRITE_MMIO,
	GET_ROT_STATUS,
	GET_PORT_ATTRIB,
	GET_EDID_INFO,
	READ_I2C_REG,
	WRITE_I2C_REG,
	READ_MMIO_REGS,
	READ_MEM,
	WRITE_MEM,
	GET_TV_REGS,
	SET_DISPLAY_MODE,
	ENABLE_PORT,
	DISABLE_PORT,
	GET_CURRENT_DC,
	WAIT,
	SET_SCRN_RES,
	READ_RING_INS,
	DECODE_INS,
	ROTATE,
	FLIP,
	GET_DC_LIST,
	GET_DRIVER_MODE_LIST,
	GET_DISP_PORT_MAPPING,
	CHECK_REGS,
	GET_CRC_VALUES,
	GET_FRAME_BUFFER,
	SET_DEBUG_VOL,
	PARSE_CRG,
	GET_PCI_INFO,
	VIDEO_CRC_START,
	VIDEO_CRC_STOP,
	GET_CRC,
	GET_CRC_DEBUG_INFO,
	GET_VIDEO_FPS,
	GET_SURFACE_LIST,
	GET_SURFACE_DATA,
	TOTAL_COMMON_FUNCS			/* DO NOT ADD ANY ENUMS FOR COMMON_FUNCS
									AFTER THIS LINE. ADD BEFORE IT*/
};

typedef EXIT_CODE (* OSFUNC_PTR) (iegd_hal_info_t *args);

typedef struct _hal_table {
	int func_val;
	char func_name[MAX_SIZE];
	int min_num_params;
	int max_num_params;
	bool can_be_called_from_all;
	OSFUNC_PTR func;
} hal_table_t ;


#define COPY_CASE(id, strId) case id: strcpy(strId, #id); \
								break

hal_table_t *WINAPI ret_osfunc_jump_table(OUT int *size);
void WINAPI decode_func(IN int func, OUT char *str_func);
int  WINAPI strprefix(IN const char * buffer, IN const char * prefix);
void WINAPI remove_leading_ending_spaces(
	IN char * buffer,
	OUT char * mod_buffer);

#endif

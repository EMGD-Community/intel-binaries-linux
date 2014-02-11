/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: halos.h
 * $Revision: 1.2 $
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
 *  This file contains function headers that are required to communicate with
 *  the OAL layer.
 *-----------------------------------------------------------------------------
 */

#ifndef _HALOS_H
#define _HALOS_H

#include "halcmn.h"

extern hal_table_t g_osfunc_jump_table[];

int ret_osfunc_jump_table_size();
void decode_os_func(IN int func, OUT char *str_func);

#endif

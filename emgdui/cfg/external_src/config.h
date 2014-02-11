/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: config.h
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
 *  This file configures the behavior of the EMGD Info utility through the
 *  use of macros. These macros can be used as knobs to turn on/off some
 *  functionality within the application.
 *-----------------------------------------------------------------------------
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include <igd_version.h>

#define FOR_EXTERNAL_USE 1
#define SOURCE_CODE_ONLY 1
#define OUTPUT_TO_FILE   0

#define PROGRAM_NAME    "EMGDUI"
#define LOG_FILE_NAME   "emgdui.log"
#define INTEL_TRADEMARK "Intel(R) Embedded Media and Graphics Driver"

#endif


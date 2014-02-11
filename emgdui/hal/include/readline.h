/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: readline.h
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
 *  This file contains function headers required to talk to the editline
 *  library.
 *-----------------------------------------------------------------------------
 */


#ifndef _READLINE_H
#define _READLINE_H

#include "halcmn.h"
#include <config.h>


void init_readline();
char * readln();
void readln_add_history(IN char *cmd);

char *command_generator(IN const char *text, IN int state);
void param_generator(IN const char *cmd);
char *dupstr(IN char *s);

#if !FOR_EXTERNAL_USE

#include <os_readline.h>

#if !NO_READLINE


#define INIT_READLINE() init_readline()
#define READLINE() readln()
#define READLINE_ADD_HISTORY(cmd) readln_add_history(cmd)

#else

#define INIT_READLINE()
#define READLINE() readln()
#define READLINE_ADD_HISTORY(cmd)

#endif

#else

#define INIT_READLINE()
#define READLINE() readln()
#define READLINE_ADD_HISTORY(cmd)

#endif
#endif

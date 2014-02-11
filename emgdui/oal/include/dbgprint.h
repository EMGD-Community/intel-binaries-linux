/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: dbgprint.h
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
 *  This file contains function headers related to debugging purposes.
 *-----------------------------------------------------------------------------
 */


#ifndef _DBGPRINT_H
#define _DBGPRINT_H

#include <stdio.h>
#include <stdarg.h>
#include <fstream>

using namespace std;

#define IN
#define OUT

#define STRM_DBG    1
#define STRM_CON    2
#define STRM_LOG    4

#define STRM_ALL    STRM_LOG + STRM_CON + STRM_DBG

#define MAX_STREAMS 3

#define MSG_ERROR   1
#define MSG_WARNING 2
#define MSG_SUCCESS 3
#define MSG_INFO    4

#define PASS		1
#define FAIL		2

#ifndef _MAX_PATH
#define _MAX_PATH   260
#endif


#ifdef WIN32
#include <direct.h>
#define STRCASECMP  _stricmp
#define STRNCASECMP _strnicmp
#define GETCWD      _getcwd
#else
#include <unistd.h>
#define STRCASECMP  strcasecmp
#define STRNCASECMP strncasecmp
#define GETCWD      getcwd
#endif

extern unsigned int g_debug_flag;
extern int g_strmFlag;
extern ofstream *g_logfile;


void	writePassFail(IN const char*, int);
void	writeFormattedMsg(IN int, IN const char*, IN const char*, ...);
int		writeMsg(IN int, IN const char*, ...);


#define WRITE_MSG(level, args) if(g_debug_flag > level) { writeMsg args; }

#define ENTER_CRITICAL_SECTION()
#define LEAVE_CRITICAL_SECTION()

#endif

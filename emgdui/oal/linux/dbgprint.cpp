/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: dbgprint.cpp
 * $Revision: 1.3 $
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
 *  This file has debugging related functions in it.
 *-----------------------------------------------------------------------------
 */

#include <dbgprint.h>
#include <string.h>

unsigned int g_debug_flag = 0x2;
int g_strmFlag = STRM_ALL;
ofstream *g_logfile = NULL;

/*-----------------------------------------------------------------------------
 * Function:
 *	writePassFail
 * Parameters:
 *	IN const char* msg - The message that needs to be printed
 *	                      (usually test name)
 *	int result			- This can only be PASS or FAIL.
 * Description:
 *	This function writes whether a test passed or failed
 * Returns:
 *	void
 *-----------------------------------------------------------------------------
 */
void writePassFail(IN const char* msg, int result)
{
	char msgToSend[_MAX_PATH];
	int length, msgColor, used_length;

	strcpy(msgToSend, msg);
	length = strlen(msg);
	used_length = length;

	for(int counter = 0; counter < (70 - length); counter++) {
		strcat(msgToSend, ".");
		used_length++;
	}

	switch(result) {
	case PASS:
		strncat(msgToSend, "PASS", (_MAX_PATH - used_length));
		msgColor = MSG_SUCCESS;
		break;
	case FAIL:
		strncat(msgToSend, "FAIL", (_MAX_PATH - used_length));
		msgColor = MSG_ERROR;
		break;
	default:
		msgColor = MSG_ERROR;
		strncat(msgToSend, "INVALID RESULT", (_MAX_PATH - used_length));
		break;
	}

	writeMsg(msgColor, msgToSend);
	writeMsg(MSG_INFO, "\n");
}

/*-----------------------------------------------------------------------------
 * Function:
 *	writeFormattedMsg
 * Parameters:
 *	IN int errorLevel - Whether this is an error, warning, success or info
 *	                The following are valid values:
 *	                MSG_ERROR   - The message will show up in RED color
 *	                                on the console
 *	                MSG_WARNING - The message will show up in YELLOW color
 *	                                on the console
 *	                MSG_SUCCESS - The message will show up in GREEN color
 *	                                on the console
 *	                MSG_INFO    - The message will show up in WHITE color
 *	                                on the console
 *	IN const char* prefix - The prefix before msg to display. "..." will show
 *	                up after the prefix
 *	IN char* msg - the message to write to stdout and the logfile
 * Description:
 *	This function writes a formatted message with ....s to a certain point
 *	to display information that the callers want
 * Returns:
 *	void
 *-----------------------------------------------------------------------------
 */
void  writeFormattedMsg(
	IN int errorLevel,
	IN const char* prefix,
	IN const char* msg,
	...)
{
	char msgAdd[_MAX_PATH];
	va_list ap;
	va_start(ap, msg);
	char msgToSend[_MAX_PATH];
	int length, used_length;
	strcpy(msgToSend, prefix);

	length = strlen(prefix);
	used_length = length;

	for(int counter = 0; counter < (50 - length); counter++) {
		strcat(msgToSend, ".");
		used_length++;
	}
	vsnprintf(msgAdd, (_MAX_PATH - used_length), msg, ap);
	strcat(msgToSend, msgAdd);

	writeMsg(errorLevel, msgToSend);
}


/*-----------------------------------------------------------------------------
 * Function:
 *	writeMsg
 * Parameters:
 *	IN char * msg - the message to write to stdout and the logfile
 *	IN ... - extra parameters can be sent to this function as well
 * Description:
 *	This function just writes a message to both stdout and a logfile
 * Returns:
 *	int - Always returns 0, the reason this is so is to make this
 *	function exactly like the printf function so that sometimes
 *	if we have to use function pointers then we can change this
 *	function name with the printf function name and it would
 *	work fine.
 *-----------------------------------------------------------------------------
 */
int  writeMsg(IN int errorLevel, IN const char * msg, ...)
{
	char msgAdd[_MAX_PATH], errorPrnt[_MAX_PATH] = "\0";

	va_list ap;
	va_start(ap, msg);
	vsnprintf(msgAdd, _MAX_PATH, msg, ap);

	if(isatty(fileno(stdout))) {

		switch(errorLevel) {

		case MSG_SUCCESS:
			/* This will print the message in green color */
			sprintf(errorPrnt, "%s", "\033[01;32m");
			break;
		case MSG_WARNING:
			/* This will print the message in yellow color */
			sprintf(errorPrnt, "%s", "\033[01;33m");
			break;
		case MSG_ERROR:
			/* This will print the message in red color */
			sprintf(errorPrnt, "%s", "\033[01;31m");
			break;
		case MSG_INFO:
		default:
			sprintf(errorPrnt, "%s", "" /*"\033[01;37m"*/);
			break;
		}
	}

	for(int counter = 0, shift = 1; counter < MAX_STREAMS; counter++,
		shift <<= 1) {

		switch(g_strmFlag & shift) {

		case STRM_CON:
			printf("%s", errorPrnt);
			printf("%s\n", msgAdd);
			if(isatty(fileno(stdout))) {
				printf("\033[01;00m");
			}
			break;
		case STRM_LOG:
			if(g_logfile) {
				(*g_logfile)<<msgAdd<<endl;
			}
			break;
		default:
			break;
		}
	}

	return 0;
}


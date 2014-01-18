/*******************************************************************************
 * Copyright 2012 Intel Corporation All Rights Reserved.
 *
 * The source code, information and material ("Material") contained herein is
 * owned by Intel Corporation or its suppliers or licensors, and title to such
 * Material remains with Intel Corporation or its suppliers or licensors. The
 * Material contains proprietary information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright laws and treaty
 * provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed or disclosed in any way
 * without Intel's prior express written permission. No license under any patent,
 * copyright or other intellectual property rights in the Material is granted to
 * or conferred upon you, either expressly, by implication, inducement, estoppel
 * or otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter this
 * notice or any other notice embedded in Materials by Intel or Intel's suppliers
 * or licensors in any way.
 * 
 *-----------------------------------------------------------------------------
 * Description:
 *  Include file for Intel's driver control protocol and client library
 *-----------------------------------------------------------------------------
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD OR AN X PROJECT TEAM SPECIFICATION */

#ifndef _INTEL_ESCAPE_H_
#define _INTEL_ESCAPE_H_

#include <X11/Xfuncproto.h>
#include <X11/Xmd.h>

#define X_IegdQueryVersion           0
#define X_IegdSetClientVersion       1
#define X_IegdEscape                 2

/* won't work, normal X apps won't have X11/extensions in the include path */
/* #include <iegd_escape.h> */

#ifndef _IEGD_SERVER_

_XFUNCPROTOBEGIN

Bool iegd_query_version(
	Display*	/* dpy */,
	int*		/* majorVersion */,
	int*		/* minorVersion */
);

Bool iegd_query_extension(
	Display*	/* dpy */,
	int*		/* event_base */,
	int*		/* error_base */
);

Bool iegd_set_client_version(
	Display*	/* dpy */
);

int iegd_escape(
	Display*         /* dpy */,
	unsigned long    /* escape function code */,
	unsigned long    /* input data size */,
	void*            /* input data pointer */,
	unsigned long    /* output data size */,
	void*            /* output data pointer */
) ;

int iegd_uninstall_extension(Display *);

_XFUNCPROTOEND

#endif

#endif

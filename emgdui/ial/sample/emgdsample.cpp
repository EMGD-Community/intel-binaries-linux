/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: iegdsample.cpp
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
 *  This file demonstrates the use of EMGDUI library interface to communicate
 *  with the EMGD driver on both Windows and Linux OSs
 *-----------------------------------------------------------------------------
 */

#include <iostream>
#include <iomanip>
#include <string.h>
#include <emgd_info.h>

using namespace std;

EXIT_CODE get_mode_list(
	IN unsigned long dc,
	IN unsigned long disp,
	OUT iegd_esc_mode_list_t **mode_list,
	OUT unsigned long *mode_list_size);

int main()
{
	EXIT_CODE exit_code, ret_exit_code = EXIT_OK;
	version_t version_info;
	os_info_t os_info;
	char str_exit_code[MAX_SIZE], str_flip_status[MAX_SIZE];
	char **str_dc_table;

	unsigned long *dc_table = NULL, dc_table_size, index, cur_dc;
	unsigned long pri_mode_table_size, sec_mode_table_size;
	unsigned long pri_master, flip_status, rot_degree;
	iegd_esc_mode_list_t *pri_mode_table = NULL, *sec_mode_table = NULL;
	iegd_esc_driver_info_t driver_info;

	/*
	 * 1) First thing to do is to call iegd_open to initialize the library.
	 * NOTE that we do not specify any parameters to the iegd_open call.
	 * This means that we do not have a kernel mode service that can
	 * read mmio for us and that functionality will not be exposed to us.
	 */
	exit_code = iegd_open();

	/* Only run the following tests if iegd_open was successful */
	if(exit_code == EXIT_OK) {

		/* 2) Get the library version */
		/*********************************************************************/
		exit_code = iegd_get_lib_version(&version_info);

		if(exit_code == EXIT_OK) {

			cout<<"Lib version: "
				<<version_info.major<<"."
				<<version_info.minor<<"."
				<<version_info.build<<endl;
		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code
				<<" while getting library version"<<endl;
		}
		/*********************************************************************/

		cout<<endl;

		/* 3) Get the os name */
		/*********************************************************************/
		exit_code = iegd_get_os_name(&os_info);

		if(exit_code == EXIT_OK) {

			if(strlen(os_info.os_name) != 0) {

				cout<<"OS name: "<<os_info.os_name<<endl;
			}

			if(strlen(os_info.version_info) != 0) {

				cout<<"Version: "<<os_info.version_info<<endl;
			}

			if(strlen(os_info.distribution) != 0) {

				cout<<"Distribution: "<<os_info.distribution<<endl;
			}

			if(strlen(os_info.additional_info) != 0) {

				cout<<"Additional Info: "<<os_info.additional_info<<endl;
			}
		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code<<" while getting os name"<<endl;
		}
		/*********************************************************************/

		cout<<endl;

		/* For these next few calls we need EMGD to be present on the system */

		/* 4) Get driver information */
		/*********************************************************************/
		exit_code = iegd_get_driver_info(&driver_info);

		if(exit_code == EXIT_OK) {

			cout<<"Driver name: "<<driver_info.name<<endl
				<<"Driver version: "<<driver_info.major
				<<driver_info.minor<<driver_info.build<<endl
				<<"Driver build date: "<<driver_info.date<<endl
				<<"Chipset name: "<<driver_info.chipset<<endl
				<<"Config id used: "<<driver_info.config_id<<endl;

		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code<<" while getting driver info"<<endl;
		}
		/*********************************************************************/

		cout<<endl;


		/* 5) Get a list of all DCs available */
		/*********************************************************************/
		exit_code = iegd_get_dc_list(&dc_table, &str_dc_table, &dc_table_size);

		if(exit_code == EXIT_OK) {

			cout<<"Total DCs available: "<<dc_table_size<<endl
				<<"DC list: "<<endl;

			for(index = 0; index < dc_table_size; index++) {

				cout<<dec<<"["<<index + 1<<"] 0x"<<hex
					<<dc_table[index]<<endl;
			}
		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code<<" while getting dc list"<<endl;
		}
		iegd_free_dc_list(dc_table, str_dc_table, dc_table_size);
		/*********************************************************************/

		cout<<dec<<endl;

		/* 6) Get the current display configuration */
		/*********************************************************************/
		cur_dc = iegd_get_current_dc();
		cout<<"Current DC: 0x"<<hex<<cur_dc<<endl;
		/*********************************************************************/

		cout<<dec<<endl;

		/* 7a) Get the primary mode list supported by the current dc */
		/*********************************************************************/
		exit_code = get_mode_list(cur_dc, PRIMARY_DISP,
			&pri_mode_table,&pri_mode_table_size);

		if(exit_code != EXIT_OK) {

			ret_exit_code = exit_code;
		}

		cout<<endl;

		/* 7b) Get the secondary mode list supported by the current dc */
		exit_code = get_mode_list(cur_dc, SECONDARY_DISP,
			&sec_mode_table, &sec_mode_table_size);

		if(exit_code != EXIT_OK) {

			ret_exit_code = exit_code;
		}
		/*********************************************************************/

		cout<<endl;

		/* 8) Set the mode on the primary display */
		/*********************************************************************/
		cout<<"Setting 16 bits per pixel"<<endl;
		exit_code = iegd_set_mode(PRIMARY_DISP, &(pri_mode_table[0]), 16);

		if(exit_code == EXIT_OK) {

			cout<<"Set the Primary mode to: "
				<<pri_mode_table[0].width<<" x "
				<<pri_mode_table[0].height<<" x "
				<<pri_mode_table[0].refresh<<endl;


		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code<<" while setting primary mode"<<endl;
		}
		/*********************************************************************/

		cout<<endl;

		/*********************************************************************/
		cout<<"Setting 32 bits per pixel"<<endl;
		exit_code = iegd_set_mode(PRIMARY_DISP, &(pri_mode_table[0]), 32);

		if(exit_code == EXIT_OK) {

			cout<<"Set the Primary mode to: "
				<<pri_mode_table[0].width<<" x "
				<<pri_mode_table[0].height<<" x "
				<<pri_mode_table[0].refresh<<endl;


		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code<<" while setting primary mode"<<endl;
		}
		/*********************************************************************/

		cout<<endl;

		/* 9) Get the current port on primary location 0 */
		/*********************************************************************/
		pri_master = iegd_get_port(cur_dc, PRIMARY_DISP, 0);
		cout<<"Port on Primary display location 0 is "
			<<pri_master<<endl;
		/*********************************************************************/

		/*
		 * 10) Get the flip and rotation status on the port on
		 * primary location 0
		 */
		/*********************************************************************/
		exit_code = iegd_get_flip_rot(pri_master, &flip_status, &rot_degree);

		if(exit_code == EXIT_OK) {

			strcpy(str_flip_status,
				(flip_status == FLIP_NONE) ? " is not " : " is ");
			cout<<"Port "<<pri_master<<str_flip_status<<"flipped"<<endl
				<<"Port "<<pri_master<<" is rotated by "
				<<rot_degree<<" degrees"<<endl;

		} else {

			ret_exit_code = exit_code;
			iegd_decode_exit_code(exit_code, str_exit_code);
			cout<<"ERROR: "<<str_exit_code
				<<" while getting rotation and flip status"<<endl;
		}
		/*********************************************************************/

		/* 11) Don't forget to free the mode table list */
		/*********************************************************************/
		iegd_free_mode_list(pri_mode_table);
		pri_mode_table = NULL;

		iegd_free_mode_list(sec_mode_table);
		sec_mode_table = NULL;
		/*********************************************************************/

		cout<<endl;
	}

	/*
	 * 23) Don't forget to close the library so that it can uninitialize its
	 * interfaces
	 */
	iegd_close();

	return (int) ret_exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *	get_mode_list
 * Parameters:
 *	IN unsigned long dc - The display config that we need to get a mode list for
 *	IN unsigned long disp - The display of the dc. This parameter can only be
 *                          PRIMARY_DISP or SECONDARY_DISP
 *	OUT iegd_esc_mode_list_t **mode_list - The returned mode list from the
 *                          driver
 *	OUT unsigned long *mode_list_size - The returned mode list size
 * Description:
 *	This function gets a mode list for a particular display, given a dc
 *	and the primary/secondary display flag
 * Returns:
 *	EXIT_CODE -
 *		EXIT_OK       = SUCCESS
 *		Anything else = FAILURE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_mode_list(
	IN unsigned long dc,
	IN unsigned long disp,
	OUT iegd_esc_mode_list_t **mode_list,
	OUT unsigned long *mode_list_size)
{

	EXIT_CODE exit_code = EXIT_OK;
	char str_exit_code[MAX_SIZE];
	unsigned long index;

	/* Get the mode list supported by the current dc */
	exit_code = iegd_get_mode_list(dc, disp, mode_list, mode_list_size);

	if(exit_code == EXIT_OK) {

		cout<<"Width\tHeight\tRefresh Rate"<<endl<<endl;
		for(index = 0; index < *mode_list_size; index++) {

			cout<<(*mode_list)[index].width<<"\t"
				<<(*mode_list)[index].height<<"\t"
				<<(*mode_list)[index].refresh<<endl;
		}
	} else {

		iegd_decode_exit_code(exit_code, str_exit_code);
		cout<<"ERROR: "<<str_exit_code<<" while getting mode list"<<endl;
	}

	return exit_code;
}

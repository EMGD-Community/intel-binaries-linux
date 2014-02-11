/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: hal_interface.cpp
 * $Revision: 1.6 $
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
 *  This file contains functions that are used as an Interface to the EMGD
 *  Info library.
 *-----------------------------------------------------------------------------
 */

#include <hal.h>
#include <halos.h>
#include <osfunc.h>
#include <dbgprint.h>

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

hal_table_t *g_main_jump_table = NULL;
globals_t globals;

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_open
 *
 * Parameters:
 *
 *  IN bool is_driver_present - This parameter is reserved and must be set
 *                          to false
 *
 * Description:
 *
 *  This function will initialize the library. It MUST be called before
 *  calling any other function of the library. When done with the library,
 *  call iegd_close to free any internal dynamically allocated data
 *  structures that the library created.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_open(IN bool is_driver_present)
{
	EXIT_CODE exit_code = EXIT_OK;
	bool ret_val;

	ret_val = os_initialize(is_driver_present);

	/*
	 * If we were able to successfully initialize the OAL layer then we
	 * can find out what chipset we are running on
	 */
	if(ret_val) {

		INIT_HW_GEN();
	} else {

		exit_code = EXIT_OS_CALL_ERROR;
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_lib_version
 *
 * Parameters:
 *
 *  OUT version_t *version_info - Library version information
 *
 * Description:
 *
 *  This function will get the library version.
 *
 * Returns:
 *
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_lib_version(OUT version_t *version_info)
{
	EXIT_CODE exit_code = EXIT_OK;

	if(!version_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input version info is NULL. Caller must specify"
			" a NON NULL version info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		version_info->major = EMGD_LIB_MAJOR;
		version_info->minor = EMGD_LIB_MINOR;
		version_info->build = EMGD_LIB_BUILD;
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_os_name
 *
 * Parameters:
 *
 *  OUT os_info_t *os_info - Operating System information such as name,
 *                          version, distribution etc.
 *
 * Description:
 *
 *  This function will get the operating system name.
 *
 * Returns:
 *
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE iegd_get_os_name(OUT os_info_t *os_info)
{
	EXIT_CODE exit_code = EXIT_OK;

	if(!os_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input os info is NULL. Caller must specify"
			" a NON NULL os info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		if(!os_get_name(os_info)) {

			exit_code = EXIT_OS_CALL_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_chipset_name
 *
 * Parameters:
 *
 *  OUT chipset_info_t *chipset_info - Chipset information such as chipset
 *                          name, revision, number etc.
 *
 * Description:
 *
 *  This function will get the chipset name.
 *
 * Returns:
 *
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE iegd_get_chipset_name(OUT chipset_info_t *chipset_info)
{
	EXIT_CODE exit_code = EXIT_OK;

	if(!chipset_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input chipset info is NULL. Caller must specify"
			" a NON NULL chipset info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		memset(chipset_info, 0, sizeof(chipset_info_t));

		if(!OS_GET_CHIPSET_NAME(chipset_info)) {

			exit_code = EXIT_OS_CALL_ERROR;

		} else {

			decode_chipset_name(chipset_info->chipset,
				chipset_info->chipset_name);
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_mode_list
 *
 * Parameters:
 *
 *  IN unsigned long dc - The display configuration that the user wants
 *                          to get modes for
 *  IN unsigned long display_num - Display number that the user wants to
 *                          get modes for. Valid values are PRIMARY_DISP,
 *                          SECONDARY_DISP
 *  OUT iegd_esc_mode_t **mode_table - The set of modes that this port
 *                          can currently support.
 *  OUT unsigned long *mode_table_size - The total number of modes filled
 *                          in the previous argument.
 *
 * Description:
 *
 *  This function will return the current set of modes supported by the
 *  user specified display number. This function will allocate and return
 *  a block of memory to hold the results. The caller should not allocate
 *  memory for this data on its own. In order to free the mode list,
 *  call iegd_free_mode_list.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *      EXIT_NOT_AVAIL_ERROR - This particular interface is not available
 *                          on this Operating System/Chipset combination.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_mode_list(
	IN unsigned long dc,
	IN unsigned long display_num,
	OUT iegd_esc_mode_list_t **mode_table,
	OUT unsigned long *mode_table_size)
{
	EXIT_CODE exit_code = EXIT_OK;
	unsigned long port_location;

	if(display_num == PRIMARY_DISP) {

		port_location = PRIMARY_MASTER;
	} else if(display_num == SECONDARY_DISP) {

		port_location = SECONDARY_MASTER;
	} else {

		exit_code = EXIT_INVALID_PARAM_ERROR;
	}

	if(exit_code == EXIT_OK) {

		/*
		 * If this variable is not NULL, that means the caller sent us an
		 * already allocated variable which is going to cause memory leaks.
		 * This is not an error, but at least we should warn the caller.
		 */
		if(*mode_table) {

			WRITE_MSG(0, (MSG_WARNING,
				"WARNING: Input mode table is not NULL. This may cause memory"
				" leaks"));
		}

		if(!mode_table_size) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR: Input mode table size is NULL. Caller must specify"
				" a NON NULL mode size"));
			exit_code = EXIT_INVALID_PARAM_ERROR;
		} else {

			/* First we get the number of modes that are possible */
			exit_code  = get_num_timings(dc, port_location, mode_table_size);

			if(exit_code == EXIT_OK) {

				*mode_table = new iegd_esc_mode_list_t[*mode_table_size];

				/* Get a list of timings that are associated with this new dc*/
				exit_code = get_timings(dc,
					(*mode_table_size) * sizeof(iegd_esc_mode_list_t),
					port_location,
					*mode_table);
			}
		}
	}


	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_free_mode_list
 *
 * Parameters:
 *
 *  IN iegd_esc_mode_list_t *mode_table - The set of modes. This MUST be the
 *                          same pointer that was returned by the
 *                          iegd_get_mode_list call.
 *
 * Description:
 *
 *  This function will free the memory allocated by iegd_get_mode_list call.
 *  There must be a corresponding iegd_free_mode_list call for each
 *  iegd_get_mode_list call made by the caller or the library would leak
 *  memory.
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void WINAPI iegd_free_mode_list(IN iegd_esc_mode_list_t *mode_table)
{
	/* If some memory exists, delete it */
	if(mode_table) {

		delete [] mode_table;
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_set_mode
 *
 * Parameters:
 *
 *  IN unsigned long display_num - The display number to set the mode on.
 *                          Valid values are PRIMARY_DISP, SECONDARY_DISP
 *  IN iegd_esc_mode_list_t *mode_to_set - The current mode to set
 *  IN unsigned short bpp - Bits Per Pixel to set
 *
 * Description:
 *
 *  This function will set a particular mode on a particular display
 *  specified by the user.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *      EXIT_NOT_AVAIL_ERROR - This particular interface is not available
 *                          on this Operating System/Chipset combination.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_mode(
	IN unsigned long display_num,
	IN iegd_esc_mode_list_t *mode_to_set,
	IN unsigned short bpp)
{
	EXIT_CODE exit_code = EXIT_OK;
	unsigned long cur_dc;
	iegd_esc_mode_list_t primary_mode;
	unsigned long primary_width, primary_height, primary_refresh;
	unsigned long primary_rotation;
	unsigned long primary_bpp;

	if(!mode_to_set) {

		exit_code = EXIT_INVALID_PARAM_ERROR;
	} else {

		if(display_num == SECONDARY_DISP ) {

			/*
			 * If this is for the clone or exteneded display, we will need to
			 * set dc to give the secondary mode. So first we need to get
			 * the current dc, and the current primary mode.
			 */
			cur_dc = iegd_get_current_dc();
			/* Make sure that the previous call succeeds */
			if(os_get_scrn_res(PRIMARY_DISP, &primary_width, &primary_height,
				&primary_refresh, &primary_bpp, &primary_rotation)) {

				WRITE_MSG(2, (MSG_INFO,
					"Primary's Width = %d, Height = %d, Refresh = %d",
					primary_width, primary_height, primary_refresh));
				primary_mode.width = (unsigned short) primary_width;
				primary_mode.height = (unsigned short) primary_height;
				primary_mode.refresh = (unsigned short) primary_refresh;
				exit_code = iegd_set_dc(cur_dc, &primary_mode, mode_to_set,
						(unsigned short)primary_bpp, bpp, ALL_DISP_CHANGED);

				/* Set the mode on the primary */
				if(exit_code == EXIT_OK) {

					if(IS_CLONE_MODE(cur_dc)) {
						display_num = PRIMARY_DISP;
					}

					if(!os_set_scrn_res(
						display_num,
						primary_mode.width,
						primary_mode.height,
						primary_mode.refresh,
						(unsigned short) bpp)) {

						exit_code = EXIT_OS_CALL_ERROR;
					}
				}
			} else {

				exit_code = EXIT_OS_CALL_ERROR;
			}
		} else {

			/* Set the mode */
			if(!os_set_scrn_res(display_num,
				mode_to_set->width,
				mode_to_set->height,
				mode_to_set->refresh,
				(unsigned short) bpp)) {

				exit_code = EXIT_OS_CALL_ERROR;
			}
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_dc_list
 *
 * Parameters:
 *
 *  OUT unsigned long **dc_table - The set of display configurations that
 *                          EMGD can currently support.
 *  OUT char ***str_dc_table - A pointer to an array of pointers so that
 *                          we can get the entire dc list in string format.
 *  OUT unsigned long *dc_table_size - The total number of display
 *                          configurations filled in the previous argument.
 *
 * Description:
 *
 *  This function will return the current set of display configurations
 *  (dcs) supported by the underlying hardware. This function will allocate
 *  and return a block of memory to hold the results. The caller should not
 *  allocate memory for this data on its own. In order to free the dc list,
 *  call iegd_free_dc_list.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_dc_list(
	OUT unsigned long **dc_table,
	OUT char ***str_dc_table,
	OUT unsigned long *dc_table_size)
{
	EXIT_CODE exit_code = EXIT_OK;
	unsigned long index;
	char str_dc[MAX_SIZE];

	if(!dc_table_size) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input dc table size is NULL. Caller must specify"
			" a NON NULL dc size"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		/*
		 * If this variable is not NULL, that means the caller sent us an
		 * already allocated variable which is going to cause memory leaks.
		 * This is not an error, but at least we should warn the caller.
		 */
		if(*dc_table) {

			WRITE_MSG(0, (MSG_WARNING,
				"WARNING: Input dc table is not NULL. This may cause memory"
				" leaks"));
		}

		exit_code = get_num_dc(dc_table_size);

		if(exit_code == EXIT_OK) {

			*dc_table = new unsigned long[*dc_table_size];
			*str_dc_table = new char *[*dc_table_size];

			/* Next we get the DC list */
			exit_code = get_dc_list(*dc_table,
				(*dc_table_size) * sizeof(unsigned long));

			for(index = 0; index < *dc_table_size; index++) {

				(*str_dc_table)[index] = new char[MAX_SIZE];

				conv_dc_to_str((*dc_table)[index], str_dc);
				strcpy((*str_dc_table)[index], str_dc);
			}
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_free_dc_list
 *
 * Parameters:
 *
 *  IN unsigned long *dc_table - The set of dcs. This MUST be the same
 *                           pointer that was returned by the iegd_get_dc_list
 *                           call
 *  IN char **str_dc_table - The set of dcs in string format. This is an
 *                           array of pointers so all dcs need to be deleted
 *  IN unsigned long dc_table_size - The size of dc list so that we can
 *                           delete individual elements from the string array.
 *
 * Description:
 *
 *  This function will free the memory allocated by iegd_get_dc_list call.
 *  There must be a corresponding iegd_free_dc_list call for each
 *  iegd_get_dc_list call made by the caller or the library would leak memory.
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void WINAPI iegd_free_dc_list(
	IN unsigned long *dc_table,
	IN char **str_dc_table,
	IN unsigned long dc_table_size)
{
	unsigned long index;

	if(dc_table) {

		delete [] dc_table;
	}

	if(str_dc_table) {

		for(index = 0; index < dc_table_size; index++) {

			if(str_dc_table[index]) {

				delete [] str_dc_table[index];
			}
		}

		delete [] str_dc_table;
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_set_dc
 *
 * Parameters:
 *  IN unsigned long dc_to_set - The current display configuration to set.
 *  IN iegd_esc_mode_t *primary_mode - The primary displays mode
 *  IN iegd_esc_mode_t *secondary_mode - The secondary displays mode. This
 *                          parameter is an optional parameter and is only
 *                          needed if the new display configuration is going
 *                          to have a secondary display in it. Example:
 *                          Extended or Clone configurations.
 *  IN unsigned short chng_flag - primary or secondary mode changed flag
 *                    bit 1- primary mode changed 1-changed 0-not changed
 *                    bit 2- secondary mode changed 1-changed 0-not changed
 *  if dc=200058,if analog mode changed, chng_flag=1,PRIMARY_CHANGED
 *  if SDVOB mode changed, chng_flag=2,SECONDARY_CHANGED
 *  if both displays mode has changed then chng_flag=3,ALL_DISP_CHANGED
 *  if no mode change chng_flag=0
 *  default value = 3,ALL_DISP_CHANGED
 *  IN unsigned short primary_bpp - primary displays bit depth
 *  IN unsigned short secondary_bpp - secondary displays bit depth. This
 *                          parameter is an optional parameter and is only
 *                          needed if the new display configuration is going
 *                          to have a secondary display in it.
 *
 * Description:
 *
 *  This function will set a particular display configuration as specified
 *  by the user.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_dc(
	IN unsigned long dc_to_set,
	IN iegd_esc_mode_list_t *primary_mode,
	IN iegd_esc_mode_list_t *secondary_mode,
	IN unsigned short primary_bpp,
	IN unsigned short secondary_bpp,
	IN unsigned short chng_flag)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_set_dc_t set_dc;
	unsigned long current_dc, new_dc=0;
	int esc_status;
	unsigned short primary_port_chng = 0;
	scrn_info_t scrn_info;
	bool was_extended = false;
	bool scrn_res_chng_only = false;

	/* Caller must provide at least the primary mode */
	if(!primary_mode) {

		WRITE_MSG(0, (MSG_ERROR, "Must provide a primary mode"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		memset(&set_dc, 0, sizeof(iegd_esc_set_dc_t));
		memset(&scrn_info, 0, sizeof(scrn_info_t));
		//get_screen_num(&scrn_info);
		/* fill in the change_flag info for primary and secondary
		 * if primary mode changed chng_flag[PRIMARY_DISP]=PRIMARY_CHANGED
		 * if secondary mode changed chng_flag[SECONDARY_DISP]=SECONDARY_CHANGED
		 */
		scrn_info.chng_flag = chng_flag;

		set_dc.dc = dc_to_set;
		set_dc.iegd_timings[0].width = primary_mode->width;
		set_dc.iegd_timings[0].height = primary_mode->height;
		set_dc.iegd_timings[0].refresh = primary_mode->refresh;
		set_dc.iegd_fb_info[0].bit_depth = primary_bpp;

		if(secondary_mode) {

			set_dc.iegd_timings[1].width = secondary_mode->width;
			set_dc.iegd_timings[1].height = secondary_mode->height;
			set_dc.iegd_timings[1].refresh = secondary_mode->refresh;
			if (!secondary_bpp) {
				secondary_bpp = primary_bpp;
			}
			set_dc.iegd_fb_info[1].bit_depth = secondary_bpp;

		}

		exit_code = get_cur_dc(&current_dc);

		if(exit_code == EXIT_OK) {
			/* get the screen numbers of displays */
			get_screen_info(&scrn_info);
			was_extended = IS_EXTENDED_MODE(current_dc);
			/*
			 * Call our macro definition to detach an extended mode if we are
			 * switching to another extended mode. This should only be called in
			 * Windows and should return EXIT_OK in Linux.
			 */
			exit_code = EXTENDED_TO_EXTENDED_CHECK(&current_dc, &dc_to_set,
				&set_dc, &scrn_info, &was_extended);
			if (exit_code != EXIT_OK) {
				return exit_code;
			}

			/*
			 * Here is a special case:
			 * If we were in extended mode and going away from it now
			 * then we should first tell windows to detach the second
			 * display.
			 */
			if(IS_EXTENDED_MODE(current_dc) &&!IS_EXTENDED_MODE(dc_to_set)) {

					os_change_disp_settings(&set_dc, true, &scrn_info);
					/*
					 * We have detached the secondary, so reset the flag,
					 * so we do not detach it gain in the last call to os_change
					 * display_settings()
					 */
					was_extended = false;
					/* Check if the primary port changed */
					primary_port_chng = IS_PRIMARY_PORT_CHANGED(current_dc,
						dc_to_set);
			}

			/*
			 * If we are going to extended mode and we are not in extended mode
			 * now, Check if the primary port changed
			 * Change to single with the new primary display port
			 */
			if(!IS_EXTENDED_MODE(current_dc) && IS_EXTENDED_MODE(dc_to_set)) {
				/* Change to single with the new primary display port */
				if(IS_PRIMARY_PORT_CHANGED(current_dc, dc_to_set)) {
					exit_code = change_to_single(set_dc);
					if(exit_code != EXIT_OK) {
						return exit_code;
					}
				}
			}
			/* if we are in clone mode, the only way the secondary's resolution
			 * can be set is thru set_dc because OS is unaware of the secondary
			 * so in this case, set_dc needs to be called
			*/
			if(current_dc != set_dc.dc || (IS_CLONE_MODE(set_dc.dc)
				&& (SECONDARY_CHANGED & chng_flag))){
				esc_status = os_send_escape(INTEL_ESCAPE_SET_DC,
					sizeof(iegd_esc_set_dc_t), (char *)&set_dc, 0, NULL);
			} else {
				scrn_res_chng_only = true;
			}

			/* Check if the dc was successfully set */
			exit_code = get_cur_dc(&new_dc);
			if(new_dc != set_dc.dc) {
				return exit_code;
			} else if(!IS_EXTENDED_MODE(current_dc) ||
					IS_EXTENDED_MODE(dc_to_set) || primary_port_chng ||
					scrn_res_chng_only) {
				memset(&scrn_info, 0, sizeof(scrn_info_t));
				scrn_info.chng_flag = chng_flag;
				/* get the screen numbers of displays */
				get_screen_info(&scrn_info);
				/* Calling windows to set mode */
				os_change_disp_settings(&set_dc, was_extended, &scrn_info);
			}
		}
	}
	return exit_code;
}
/*-----------------------------------------------------------------------------
 * Function:
 *
 *  extended_to_extended_check
 *
 * Parameters:
 *  IN unsigned long *current_dc - The currenct display configuration.
 *  IN unsigned long dc_to_set - The current display configuration to set.
 *  IN iegd_esc_set_dc_t *set_dc - New display mode structure.
 *  IN scrn_info_t *scrn_info - Pointer to screen info structure.
 *
 * Description:
 *
 *  This function checks for the case where we are going from one extended
 *  display to a different extended display.  In Windows we need to detach
 *  the second display before switching to the other extended mode or the 
 *  displays will not be swapped correctly.  This should only be called in
 *  Windows.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI extended_to_extended_check(
	IN unsigned long *current_dc,
	IN unsigned long *dc_to_set,
	IN iegd_esc_set_dc_t *set_dc,
	IN scrn_info_t *scrn_info,
	IN bool *was_extended)
{
	EXIT_CODE exit_code = EXIT_OK;

	/*
	 * We are in extended and going to an extended mode, then detach
	 * secondary, then change ports to new dc single.
	 */
	if(IS_EXTENDED_MODE(*current_dc) && IS_EXTENDED_MODE(*dc_to_set)
		&& (*current_dc != *dc_to_set) ) {
		/* Detach secondary , so pass in a single dc with new primary */
		iegd_esc_set_dc_t temp_dc = *set_dc;
		temp_dc.dc = GET_SINGLE_DC(temp_dc.dc);
		os_change_disp_settings(&temp_dc,true,scrn_info);
		/* Change to single with the new primary display port */
		exit_code = change_to_single(*set_dc);
		if(exit_code != EXIT_OK) {
			return exit_code;
		}
		/*
		 * This is to make sure that os_change_display settings is
		 * called again by satisying the if last condition.
		 */
		get_cur_dc(current_dc);
		*was_extended = false;
	}
	return EXIT_OK;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_current_dc
 *
 * Parameters:
 *
 *  NONE
 *
 * Description:
 *
 *  This function will get the current display configuration
 *
 * Returns:
 *
 *  unsigned long - The current display configuration
 *-----------------------------------------------------------------------------
 */
unsigned long WINAPI iegd_get_current_dc()
{
	unsigned long cur_dc = 0;
	get_cur_dc(&cur_dc);
	return cur_dc;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_port
 *
 * Parameters:
 *
 *  IN unsigned long cur_dc - Display configuration that we need to use
 *                          to get port number from
 *  IN unsigned long display - This can be one of the following values
 *                              PRIMARY_DISP or SECONDARY_DISP
 *  IN unsigned long port_location - Port location. This can be a number
 *                          0 - 3 for PRIMARY_DISP and 0 - 2 for SECONDARY_DISP
 *
 * Description:
 *
 *  This function will get the port number based on a display configuration
 *  and the location of the port given to the library from the caller.
 *
 * Returns:
 *
 *  unsigned long - The port at the caller specified dc and port_location
 *-----------------------------------------------------------------------------
 */
unsigned long WINAPI iegd_get_port(
	IN unsigned long cur_dc,
	IN unsigned long display,
	IN unsigned long port_location)
{
	bool invalid_param = false;
	unsigned long current_port;
	unsigned long cur_port_location;


	if(display == PRIMARY_DISP) {

		if(port_location < 0 || port_location > 3) {

			invalid_param = true;
		} else {

			cur_port_location = PRIMARY_MASTER + port_location;
		}

	} else if(display == SECONDARY_DISP) {

		if(port_location < 0 || port_location > 2) {

			invalid_param = true;

		} else {

			cur_port_location = SECONDARY_MASTER + port_location;
		}

	} else {

		invalid_param = true;
	}

	if(invalid_param) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid parameter display or port location"));
		current_port = 0;
	} else {

		current_port = IGD_DC_PORT_NUMBER(cur_dc, cur_port_location);
	}

	return current_port;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_flip_rot
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to find flip/rotate status for
 *  OUT unsigned long *flip_enable - FLIP_NONE = Not Enabled
 *                                   FLIP_HORIZ = Enabled.
 *  OUT unsigned long *rot_degree - The rotation degree (0, 90, 180, 270)
 *
 * Description:
 *
 *  This function will find out rotation and flip status on a particular port.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_flip_rot(
	IN unsigned long port_num,
	OUT unsigned long *flip_enable,
	OUT unsigned long *rot_degree)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_get_rotation_flip_t rot_status;
	iegd_esc_port_in_t port_in;
	int esc_status;

	port_in.port_number = port_num;

	if((esc_status = os_send_escape(INTEL_ESCAPE_GET_ROTATION_FLIP,
				sizeof(iegd_esc_port_in_t),
				(char*) &port_in,
				sizeof(iegd_esc_get_rotation_flip_t),
				(char *) &rot_status)) == INTEL_ESCAPE_SUCCESS) {

		*flip_enable = rot_status.flip;
		*rot_degree = rot_status.rotation;
	} else {

		WRITE_MSG(0, (MSG_ERROR, "ERROR while sending"
						" INTEL_ESCAPE_GET_ROTATION_FLIP escape"
						" key"));
		exit_code = 
			(esc_status == INTEL_ESCAPE_ERROR)
			? EXIT_OS_CALL_ERROR
			: EXIT_NOT_IMPLEMENTED_ERROR;
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_set_flip_rot
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to rotate or flip.
 *  IN unsigned long flip_enable - FLIP_NONE = No Flip
 *                                 FLIP_HORIZ = Flip Enabled
 *  IN unsigned long rot_degree - The rotation degree (0, 90, 180, 270)
 *
 * Description:
 *
 *  This function will rotate and/or flip a particular port by a user
 *  specified rotation mode and flip enable flag.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_flip_rot(
	IN unsigned long port_num,
	IN unsigned long flip_enable,
	IN unsigned long rot_degree)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_set_rotation_flip_t rot_flip;
	iegd_esc_get_rotation_flip_t rot_status;
	iegd_esc_port_info_t port_info;
	iegd_esc_port_in_t port_in;
	unsigned long rotation = 0, force_reset;
	int esc_status;

	if(rot_degree != 0    &&
		rot_degree != 90  &&
		rot_degree != 180 &&
		rot_degree != 270) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid Rotation specified"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	}

	if(flip_enable != FLIP_NONE && flip_enable != FLIP_HORIZ) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid Flip specified"));
		exit_code = EXIT_INVALID_PARAM_ERROR;
	}

	if(exit_code == EXIT_OK) {

		port_in.port_number = port_num;
		/* First we need to know the current status */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_ROTATION_FLIP,
			sizeof(iegd_esc_port_in_t),
			(char *) &port_in,
			sizeof(iegd_esc_get_rotation_flip_t),
			(char *) &rot_status)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_ROTATION_FLIP"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}

		if(exit_code == EXIT_OK) {

			rot_flip.port_number = port_num;
			/* Use the existing value for flip */
			rot_flip.flip = flip_enable;
			rot_flip.rotation = rot_degree;

			WRITE_MSG(2, (MSG_INFO, "Port number is: %d", port_num));
			WRITE_MSG(2, (MSG_INFO, "Flip value is: %d", flip_enable));
			WRITE_MSG(2, (MSG_INFO, "Rotation degree is: %d", rot_degree));

			if((esc_status = os_send_escape(INTEL_ESCAPE_SET_ROTATION_FLIP,
				sizeof(iegd_esc_set_rotation_flip_t),
				(char *) &rot_flip,
				0,
				NULL)) == INTEL_ESCAPE_SUCCESS) {

				WRITE_MSG(2, (MSG_INFO,
					"Setting the default screen resolution"));

				rotation = abs((long)(rot_status.rotation - rot_flip.rotation));

				exit_code = iegd_get_port_info(port_num, &port_info);

				if(exit_code == EXIT_OK) {

					force_reset = (flip_enable == rot_status.flip) ? 0 : 1;
					/*
					 * Set the default modes.
					 * The way rotation works is that you call the EMGD driver
					 * to set the rotation and flip bits but the change doesn't
					 * take effect until a set mode call is made to the driver
					 * Therefore, we must call a set mode operation with the
					 * default values for width/height/refresh.
					 */
					if(!os_set_scrn_res(/*port_info.display_id*/
						ALL_DISPLAYS,0, 0, 0, 0,
						rotation, force_reset)) {

						exit_code = EXIT_OS_CALL_ERROR;
					}
				}
			}
		} else {

			WRITE_MSG(0, (MSG_ERROR, "ERROR while sending"
				" IOCTL_INTEL_SET_ROTATION_FLIP escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_driver_info
 *
 * Parameters:
 *
 *  OUT iegd_esc_driver_info_t *driver_info - The driver version information
 *
 * Description:
 *
 *  This function will get driver version information.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_driver_info(
	OUT iegd_esc_driver_info_t *driver_info)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!driver_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to driver info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_DRIVER_INFO,
			0,
			NULL,
			sizeof(iegd_esc_driver_info_t),
			(char *) driver_info)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_DRIVER_INFO"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;

		} /* end if(os_send_escape */
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_port_info
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to get information for.
 *  OUT iegd_esc_port_info_t *port_info - The port information such as
 *                           id, type, name etc.
 *
 * Description:
 *
 *  This function will get port information such as id, type, name, etc.
 *  The caller will have to specify a port number.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_port_info(
	IN unsigned long port_num,
	OUT iegd_esc_port_info_t *port_info)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_port_in_t port_input;
	int esc_status;

	port_input.port_number = port_num;

	if(!port_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to port info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		memset(port_info, 0, sizeof(iegd_esc_port_info_t));

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_PORT_INFO,
			sizeof(iegd_esc_port_in_t),
			(char*) &port_input,
			sizeof(iegd_esc_port_info_t),
			(char*) port_info)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_PORT_INFO"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;

		} /* end if(os_send_escape */
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_port_attributes
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to get information for.
 *  OUT iegd_esc_attr_t **attr_info - A list of port attributes such as
 *                          scaling support, dual channel support etc.
 *  OUT unsigned long *num_attributes - The total number of attributes
 *                          filled in the previous argument.
 *
 * Description:
 *
 *  This function will get port attributes such as scaling etc. The caller
 *  will have to specify a port number. This function will allocate and
 *  return a block of memory to hold the results.  The caller should not
 *  allocate memory for this data on its own. Also the library would return
 *  the total number of attributes allocated.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_port_attributes(
	IN unsigned long port_num,
	OUT iegd_esc_attr_t **attr_info,
	OUT unsigned long *num_attributes)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_port_in_t port_in;
	unsigned long length;
	int esc_status;

	if(!num_attributes) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input number of attributes pointer is NULL. Caller"
			" must specify a NON NULL pointer"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		port_in.port_number = port_num;

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_NUM_PD_ATTRIBUTES,
			sizeof(iegd_esc_port_in_t),
			(char*) &port_in,
			sizeof(unsigned long),
			(char*) num_attributes)) == INTEL_ESCAPE_SUCCESS) {

			/* If the num_pd_attribs = 0 then just return */
			if(num_attributes) {

				length = sizeof(iegd_esc_attr_t) * (*num_attributes);
				*attr_info = new iegd_esc_attr_t[*num_attributes];

				memset(*attr_info, 0, length);

				port_in.port_number = port_num;

				if((esc_status = os_send_escape(
					INTEL_ESCAPE_GET_AVAIL_PD_ATTRIBUTES,
					sizeof(iegd_esc_port_in_t),
					(char*) &port_in,
					length,
					(char*) *attr_info)) != INTEL_ESCAPE_SUCCESS) {

					WRITE_MSG(0, (MSG_ERROR,
						"ERROR while sending"
						" INTEL_ESCAPE_GET_AVAIL_PD_ATTRIBUTES escape key"));
					exit_code = 
						(esc_status == INTEL_ESCAPE_ERROR)
						? EXIT_OS_CALL_ERROR
						: EXIT_NOT_IMPLEMENTED_ERROR;

				} /* end if(os_send_escape */
			}
		} else {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending"
				" INTEL_ESCAPE_GET_NUM_PD_ATTRIBUTES escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_free_port_attributes
 *
 * Parameters:
 *
 *  IN iegd_esc_attr_t *attr_info - A list of port attributes such as
 *                          scaling support, dual channel support etc.
 *
 * Description:
 *
 *  This function will free the memory allocated by iegd_get_port_attributes
 *  call. There must be a corresponding iegd_free_port_attributes call for
 *  each iegd_get_port_attributes call made by the caller or the library
 *  would leak memory.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_free_port_attributes(
	IN iegd_esc_attr_t *attr_info)
{
	EXIT_CODE exit_code = EXIT_OK;

	if(attr_info) {

		delete [] attr_info;
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_set_port_attributes
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to get information for.
 *  IN iegd_esc_attr_t *attr - A port attribute such as scaling
 *                          support, dual channel support etc.
 *
 * Description:
 *
 *  This function will set port driver attributes. The caller will have to
 *  specify a port number and the attribute information that they need to set.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_port_attributes(
	IN unsigned long port_num,
	IN iegd_esc_attr_t *attr)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_set_attr_t input_data;
	iegd_esc_status_t status;
	int esc_status;

	input_data.port_number = port_num;

	if(!attr) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Input attributes pointer is NULL. Caller"
			" must specify a NON NULL pointer"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		memset(&status, 0, sizeof(iegd_esc_status_t));

		memcpy(&input_data.attribute, attr, sizeof(iegd_esc_attr_t));

		if((esc_status = os_send_escape(INTEL_ESCAPE_SET_PD_ATTRIBUTES,
			sizeof(iegd_esc_set_attr_t),
			(char*) &input_data,
			sizeof(iegd_esc_status_t),
			(char *) &status)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending"
				" INTEL_ESCAPE_SET_PD_ATTRIBUTES escape key"));
			exit_code =
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		} /* end if(os_send_escape */
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_set_overlay_params
 *
 * Parameters:
 *
 *  IN iegd_esc_color_params_t *color_params - Overlay parameters such as
 *                          gamma, brightness etc.
 *
 * Description:
 *
 *  This function will set overlay parameters such as gamma, brightness etc.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_overlay_params(
	IN iegd_esc_color_params_t *color_params)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!color_params) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to color params"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		if((esc_status = os_send_escape(INTEL_ESCAPE_SET_OVL_COLOR_PARAMS,
			sizeof(iegd_esc_color_params_t),
			(char*) color_params,
			0,
			NULL)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_PORT_INFO"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		} /* end if(os_send_escape */
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_overlay_params
 *
 * Parameters:
 *
 *  OUT iegd_esc_color_params_t *color_params - Overlay parameters such as
 *                          gamma, brightness etc.
 *
 * Description:
 *
 *  This function will get overlay parameters such as gamma, brightness etc.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_overlay_params(
	OUT iegd_esc_color_params_t *color_params)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!color_params) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to color params"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		memset(color_params, 0, sizeof(iegd_esc_color_params_t));

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_OVL_COLOR_PARAMS,
			0,
			NULL,
			sizeof(iegd_esc_color_params_t),
			(char*) color_params)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_PORT_INFO"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		} /* end if(os_send_escape */
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_display_firmware
 *
 * Parameters:
 *
 *  IN unsigned long port_num - The port number to get information for.
 *  IN unsigned long block_num - The block number for which we need
 *                          information. Only 0 is valid currently.
 *  OUT unsigned char *fw_info - Firmware Information (128 bytes)
 *
 * Description:
 *
 *  This function will get EDID or Display ID information. The caller will
 *  have to specify a port number.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_display_firmware(
	IN unsigned long port_num,
	IN unsigned long block_num,
	OUT unsigned char *fw_info)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_port_in_t port_in;
	iegd_esc_edid_info_t edid_info;
	int esc_status;


	if(!fw_info) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to firmware info"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		port_in.port_number = port_num;

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_EDID_INFO,
			sizeof(iegd_esc_port_in_t),
			(char*) &port_in,
			sizeof(iegd_esc_edid_info_t),
			(char*) &edid_info)) == INTEL_ESCAPE_SUCCESS) {

			memcpy(fw_info, edid_info.edid, 128);

		} else {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_EDID_INFO"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_close
 *
 * Parameters:
 *
 *  NONE
 *
 * Description:
 *
 *  This function will uninitialize the library. It MUST be called after
 *  all other calls to this library have been made. No calls to the
 *  library are permitted after uninitalization.
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void WINAPI iegd_close()
{
	if(g_main_jump_table) {

		free(g_main_jump_table);
		g_main_jump_table = NULL;
	}

	UNINIT_INT();

	os_uninitialize();
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_decode_exit_code
 *
 * Parameters:
 *
 *  IN int exit_code - The exit code in integer format
 *
 *  OUT char *str_exit_code - The string conversion of the same exit
 *                          code.
 *
 * Description:
 *
 *  This function converts an integer representation of an exit code
 *  into a string representation of the same exit code.
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void WINAPI iegd_decode_exit_code(IN int exit_code, OUT char *str_exit_code)
{

	if(!str_exit_code) {

		assert(str_exit_code);

	} else {

		switch(exit_code) {

			COPY_CASE(EXIT_OK, str_exit_code);
			COPY_CASE(EXIT_GENERIC_ERROR, str_exit_code);
			COPY_CASE(EXIT_PARSER_ERROR, str_exit_code);
			COPY_CASE(EXIT_CFGFILE_ERROR, str_exit_code);
			COPY_CASE(EXIT_LOGFILE_ERROR, str_exit_code);
			COPY_CASE(EXIT_INVALID_PARAM_ERROR, str_exit_code);
			COPY_CASE(EXIT_OS_CALL_ERROR, str_exit_code);
			COPY_CASE(EXIT_LIB_INIT_ERROR, str_exit_code);
			COPY_CASE(EXIT_REGISTRY_ERROR, str_exit_code);
			COPY_CASE(EXIT_DEVICE_NOT_FOUND, str_exit_code);

		default:

			strcpy(str_exit_code, "Unknown");
			break;
		}
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_wait
 *
 * Parameters:
 *
 *  IN unsigned long num_seconds - The number of seconds to wait for
 *
 * Description:
 *
 *  Waits for user specified number of seconds
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_wait(IN unsigned long num_seconds)
{
	EXIT_CODE exit_code = EXIT_OK;

	/* First we need to get the current dc */
	if(!os_wait(num_seconds)) {

		WRITE_MSG(0, (MSG_ERROR, "ERROR while waiting"));
		exit_code = EXIT_OS_CALL_ERROR;
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_disp_port_mapping
 *
 * Parameters:
 *
 *  OUT disp_port_info_t **disp_port_table - An array containing a mapping
 *                          of displays and ports. This parameter should
 *                          be NULL when calling. The caller needs to call
 *                          iegd_free_disp_port_mapping to free the display
 *                          port table.
 *  OUT unsigned long *disp_port_table_size - The size of this array. This
 *                          parameter can NOT be NULL when calling
 *
 * Description:
 *
 *  This function figures out a mapping in between the displays (as seen
 *  by the OS) and ports (as seen by EMGD).
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_disp_port_mapping(
	OUT disp_port_info_t **disp_port_table,
	OUT unsigned long *disp_port_table_size)
{
	EXIT_CODE exit_code = EXIT_OK;


	if(*disp_port_table) {

		WRITE_MSG(0, (MSG_WARNING,
			"WARNING: Display Port table is not NULL. This may cause memory"
			" leaks"));
	}

	if(!disp_port_table_size) {

		WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Display Port table size is NULL. Caller must specify"
			" a NON NULL mode size"));
		exit_code = EXIT_INVALID_PARAM_ERROR;
	} else {

		*disp_port_table = new disp_port_info_t[MAX_DISPLAYS];
		memset(*disp_port_table, 0, sizeof(disp_port_info_t) * MAX_DISPLAYS);

		/* First we find out which displays are ours */
		os_get_intel_displays(*disp_port_table, disp_port_table_size);

		exit_code = match_disp_ports(*disp_port_table, *disp_port_table_size);
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_free_disp_port_mapping
 *
 * Parameters:
 *
 *  IN disp_port_info_t *disp_port_table - A pointer to an array
 *                          containing a mapping of displays and ports.
 *                          This MUST be the same pointer that was returned
 *                          by the iegd_free_disp_port_mapping call.
 *
 * Description:
 *
 *  This function will free the memory allocated by iegd_get_disp_port_mapping
 *  call. There must be a corresponding iegd_free_disp_port_mapping call for
 *  each iegd_get_disp_port_mapping call made by the caller or the library would
 *  leak memory.
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void WINAPI iegd_free_disp_port_mapping(IN disp_port_info_t *disp_port_table)
{
	if(disp_port_table) {

		delete [] disp_port_table;
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *  iegd_en_dis_port
 * Parameters:
 *   IN unsigned long is_enable - If 1, enable the port, if 0, disable it
 *   IN int port_num - The number of the port to enable or disable 
 *         1 -TV,2-SDVO-B,3 SDVO_C,4 INT_LVDS,5-ANALOG
 * Description:
 *   This function enables or disables a port
 * Returns:
 *   EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_en_dis_port(IN unsigned long is_enable, IN int port_num)
{
	EXIT_CODE exit_code = EXIT_OK;

	exit_code = en_dis_port_num(is_enable, port_num);
	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  match_disp_ports
 *
 * Parameters:
 *
 *  OUT disp_port_info_t *disp_port_table - The display and port table to
 *                          be returned to the caller after a mapping is done
 *  IN unsigned long disp_port_table_size - The size of this table.
 *
 * Description:
 *
 *  This function matches which ports lie on which displays (from an OS
 *  perspective, and returns this mapping to the caller in the first parmaeter
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE match_disp_ports(
	OUT disp_port_info_t *disp_port_table,
	IN unsigned long disp_port_table_size)
{
	EXIT_CODE exit_code = EXIT_OK;
	unsigned long current_dc, port_index, port_num, disp_index;
	unsigned long disp_counter, primary_id;
	bool pci_is_primary, found;
	iegd_esc_port_info_t port_info;

	static disp_set_t disp_set_table[] = {

		{MAX_PRIMARY_PORTS,   PRIMARY_DISP  },
		{MAX_SECONDARY_PORTS, SECONDARY_DISP},
	};

	/*
	 * We should only query for ports if there are displays owned by EMGD
	 */
	if(disp_port_table_size) {

		current_dc = iegd_get_current_dc();

		/* Return right-a-way if our dc is not valid */
		if(!current_dc) {

			return EXIT_OS_CALL_ERROR;
		}

		/*
		 * First we should figure out if there is an external PCI card plugged
		 * in, and if so, is it primary. If it is primary, then none of our
		 * displays will be primary (so just check for that)
		 */
		for(disp_counter = 0;
			disp_counter < disp_port_table_size;
			disp_counter++) {

			/* Break if we find a primary */
			if(disp_port_table[disp_counter].is_primary) {

				break;
			}
		}

		/*
		 * If disp_counter went all the way to disp_port_table_size then we
		 * didn't find our primary, and pci is primary
		 */
		pci_is_primary = (disp_counter >= disp_port_table_size) ? true : false;

		/* Go through both Primary and Secondary displays */
		for(disp_index = 0;
			disp_index < ARRAY_SIZE(disp_set_table);
			disp_index++) {

			/* Go through all the ports on the primary/secondary display */
			for(port_index = 0;
				port_index < disp_set_table[disp_index].max_ports;
				port_index++) {

				/* First get the port number from the index */
				port_num = iegd_get_port(current_dc,
					disp_set_table[disp_index].disp_flag, port_index);

				/* Port number should be valid */
				if(port_num) {

					/* Next get the port info for this port */
					exit_code = iegd_get_port_info(port_num, &port_info);

					if(exit_code == EXIT_OK) {

						found = false;
						/*
						 * Now we gotta match the display number with port
						 * numbers
						 */
						for(disp_counter = 0;
							disp_counter < disp_port_table_size;
							disp_counter++) {

							if(pci_is_primary) {

								/*
								 * If pci is primary then we rely upon our
								 * relative display number order
								 */
								if(disp_port_table[disp_counter].\
									rel_display_num ==
									port_info.display_id) {

									found = true;

									/*
									 * If PCI is primary then none of our
									 * displays will be primary. We still need
									 * to tell the caller (GUI) something is
									 * primary, so we make the first one as
									 * primary.
									 */
									if(port_info.display_id == 0) {

										disp_port_table[disp_counter].\
											is_primary = true;
									}
								}

							} else {

								/*
								 * If pci is not primary, then we rely upon
								 * who is primary (from an OS perspective) and
								 * match it with who is primary (from a driver
								 * perspective)
								 */
								primary_id =
									(disp_port_table[disp_counter].is_primary)
									? 0 : 1;
								if(primary_id == port_info.display_id) {

									found = true;
								}
							}

							/* If found, we add stuff to the disp port table */
							if(found) {

								disp_port_table[disp_counter].\
									port_array[disp_port_table\
									[disp_counter].port_array_size].port_num =
									port_num;

								memcpy(&disp_port_table[disp_counter].\
									port_array[disp_port_table\
									[disp_counter].port_array_size++].\
									port_info, &port_info,
									sizeof(iegd_esc_port_info_t));

								found = false;
							}
						}
					}
				}
			}
		}
	}
	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  conv_dc_to_str
 *
 * Parameters:
 *
 *  IN unsigned long dc - The dc in hex format
 *  OUT char *str_dc - A char array passed down to this function to be filled
 *                     with string version of the dc
 *
 * Description:
 *
 *  This function converts a dc into a string and returns the string in
 *  the second parameter
 *
 * Returns:
 *
 *  void
 *-----------------------------------------------------------------------------
 */
void conv_dc_to_str(IN unsigned long dc, OUT char *str_dc)
{
	unsigned long mask;
	int x, c;
	int port;
	int used_length = 0;
	char tstr[MAX_SIZE] = {'\0'};
	const char *pname[6] = {

		"Unused",
		"TV",
		"DVOB",
		"DVOC",
		"LVDS",
		"ANALOG",
	};

	mask = 0x0000000f;
	c = 0;

	for(x = 0; x < 7; x++) {

		mask = mask << 4;
		port = IGD_DC_PORT_NUMBER(dc, (x+1));

		if((x == 4) && (port != 0)) {

			if(IS_EXTENDED_MODE(dc)) {

				strncat(tstr, " -> ", (MAX_SIZE - used_length));
				used_length += 4;

			} else {

				strncat(tstr, " + ", (MAX_SIZE - used_length));
				used_length += 3;
			}

			c = 0;
		}

		if(port > 0 && port < 6) {

			if(c) {

				strncat(tstr, ",", (MAX_SIZE - used_length));
				used_length += 1;
			}

			strncat(tstr, pname[port], (MAX_SIZE - used_length));
			used_length += strlen(pname[port]);
			c = 1;
		}
	}

	WRITE_MSG(2, (MSG_INFO, "%-20s 0x%X", tstr, dc));
	strcpy(str_dc, tstr);
}


/*-----------------------------------------------------------------------------
 * Function:
 *		get_cur_dc
 * Parameters:
 *		OUT unsigned long *current_dc - The current display configuration
 *										to be obtained from the driver
 * Description:
 *		This function finds out the current display configuration from the
 *		driver.
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_cur_dc(OUT unsigned long *current_dc)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!current_dc) {

		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		/* First we need to get the current dc */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_CURRENT_DC,
			0,
			NULL,
			sizeof(unsigned long),
			(char *) current_dc)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_CURRENT_DC"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		get_num_dc
 * Parameters:
 *		OUT unsigned long *num_dc
 * Description:
 *		This function finds out the total number of dc's available.
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_num_dc(OUT unsigned long *num_dc)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!num_dc) {

		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		/* Get the number of dc's */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_NUM_DC,
			0,
			NULL,
			sizeof(unsigned long),
			(char *) num_dc)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_CURRENT_DC"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		get_dc_list
 * Parameters:
 *		OUT unsigned long *dc_list - The list of dc's to be obtained
 *										from the driver.
 *		IN unsigned long dc_list_size - The size of the dc list
 * Description:
 *		This function gets a list of dc's avaible from the driver
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_dc_list(
	OUT unsigned long *dc_list,
	IN unsigned long dc_list_size)
{
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	if(!dc_list) {

		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		/* First we need to get the current dc */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_DC_LIST,
			0,
			NULL,
			dc_list_size,
			(char *) dc_list)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_CURRENT_DC"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		get_num_timings
 * Parameters:
 *		IN unsigned long current_dc - The current display configuration
 *		IN unsigned long port_location - This can be one of two values
 *							PRIMARY_MASTER, SECONDARY_MASTER
 *		OUT unsigned long *num_timings - The number of timing modes
 * Description:
 *		This function finds out the number of timings that are supported
 *		by a particular display.
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_num_timings(
	IN unsigned long current_dc,
	IN unsigned long port_location,
	OUT unsigned long *num_timings)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_mode_in_t avail_modes;
	int esc_status;

	avail_modes.port_number = IGD_DC_PORT_NUMBER(current_dc, port_location);
	avail_modes.dc = current_dc;

	if(!num_timings) {

		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		WRITE_MSG(2, (MSG_INFO, "Port number is %d", avail_modes.port_number));
		WRITE_MSG(2, (MSG_INFO, "Current dc is 0x%X", avail_modes.dc));

		/* First we need to get the number of modes */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_NUM_MODES,
			sizeof(iegd_esc_mode_in_t),
			(char *) &avail_modes,
			sizeof(unsigned long),
			(char *) num_timings)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_NUM_MODES"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		get_timings
 * Parameters:
 *		IN unsigned long current_dc - The current display configuratio
 *		IN unsigned long mode_size - The mode list size
 *		IN unsigned long port_location - This can be one of two values
 *							PRIMARY_MASTER, SECONDARY_MASTER
 *		OUT void *modes - The list of modes to be obtained from the driver
 * Description:
 *		This function gets a list of available modes that are supported
 *		by the driver.
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_timings(
	IN unsigned long current_dc,
	IN unsigned long mode_size,
	IN unsigned long port_location,
	OUT void *modes)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_mode_in_t avail_modes;
	int esc_status;

	avail_modes.port_number = IGD_DC_PORT_NUMBER(current_dc, port_location);
	avail_modes.dc = current_dc;

	if(!modes) {

		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_AVAIL_MODES,
			sizeof(iegd_esc_mode_in_t),
			(char *) &avail_modes,
			mode_size,
			(char *) modes)) != INTEL_ESCAPE_SUCCESS) {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_GET_AVAIL_MODES"
				" escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		decode_chipset_name
 * Parameters:
 *		IN unsigned short chipset - hex number of the chipset
 *		OUT char *chipset_name - string representation of the chipset
 * Description:
 *		This function decodes the chipset name from a hex to a string
 *		literal. If you want to add more chipsets, add them to this
 *		table. DO NOT touch any other part of this function.
 * Returns:
 *		void
 *-----------------------------------------------------------------------------
 */
void decode_chipset_name(IN unsigned short chipset, OUT char *chipset_name)
{
	int size;
	bool found = false;

	static chipset_name_t chipset_conv_table[] = {

		{PCI_DEVICE_ID_810,     "Intel 810 Chipset"},
		{PCI_DEVICE_ID_810DC,   "Intel 810dc100 Chipset"},
		{PCI_DEVICE_ID_810E,    "Intel 810e Chipset"},
		{PCI_DEVICE_ID_815,     "Intel 815 Chipset"},
		{PCI_DEVICE_ID_855,     "Intel 855GM Chipset"},
		{PCI_DEVICE_ID_830,     "Intel 830M  Chipset"},
		{PCI_DEVICE_ID_835,     "Intel 835 Chipset"},
		{PCI_DEVICE_ID_845,     "Intel 845G Chipset"},
		{PCI_DEVICE_ID_865,     "Intel 865G Chipset"},
		{PCI_DEVICE_ID_915_GD0, "Intel 915G/GV/GL Chipset"},
		{PCI_DEVICE_ID_915_GD0, "Intel 910GL Chipset"},
		{PCI_DEVICE_ID_915_AL0, "Intel 915GM/GMS/910GML Chipset"},
		{PCI_DEVICE_ID_945G_C,  "Intel 945G Chipset"},
		{PCI_DEVICE_ID_945GM,   "Intel 945GM Chipset"},
		{PCI_DEVICE_ID_945GME,  "Intel 945GME/GSE Chipset"},
		{PCI_DEVICE_ID_Q35,     "Intel Q35 Chipset"},
		{PCI_DEVICE_ID_Q35A2,   "Intel Q35 Chipset"},		
		{PCI_DEVICE_ID_946GZ,   "Intel 946GZ Embedded Chipset"},
		{PCI_DEVICE_ID_965_G,   "Intel 965G Embedded Chipset"},
		{PCI_DEVICE_ID_Q965,    "Intel Q963/Q965 Embedded Chipset"},
		{PCI_DEVICE_ID_G965,    "Intel G965 Embedded Chipset"},
		{PCI_DEVICE_ID_GME965,  "Intel GLE960/GME965 Embedded Chipset"},
		{PCI_DEVICE_ID_PLB,     "Intel SCH US15W Embedded Chipset"},
		{PCI_DEVICE_ID_CTG,     "Intel GM45/GS45/GL40 Embedded Chipset"},
		{PCI_DEVICE_ID_ELK,     "Intel EagleLake SuperSKU"}, /* EagleLake */
		{PCI_DEVICE_ID_Q45,     "Intel Q45 Embedded Chipset"}, 
		{PCI_DEVICE_ID_Q43,     "Intel Q43 Embedded Chipset"},
		{PCI_DEVICE_ID_G45,     "Intel G45 Embedded Chipset"},
		{PCI_DEVICE_ID_G43,     "Intel G43 Embedded Chipset"},
		{PCI_DEVICE_ID_G41,     "Intel G41 Embedded Chipset"},
		{PCI_DEVICE_ID_TNC,     "Intel Atom E6xx Processor"},
		{PCI_DEVICE_ID_SDVO_TNC,     "Intel Atom E6xx Processor SDVO"},
	};


	size = sizeof(chipset_conv_table) / sizeof(chipset_name_t);

	for(int counter = 0; counter < size; counter++) {

		if(chipset == chipset_conv_table[counter].chipset) {

			strcpy(chipset_name, chipset_conv_table[counter].chipset_name);
			found = true;
			break;
		} /* end if */

	} /* end for */


	/*
	 * if we didn't find this chipset in the table above, then we should put in
	 * some unknown name in there
	 */
	if(!found) {

		strcpy(chipset_name, "Unknown Chipset Name");
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *		remove_leading_ending_spaces
 * Parameters:
 *		IN char * buffer - The buffer to remove spaces from
 *		OUT char * mod_buffer - The modified buffer with spaces and tabs
 *								removed
 * Description:
 *		This function removes spaces and tabs from the beginning and end
 *		of a buffer
 * Returns:
 *		void
 *-----------------------------------------------------------------------------
 */
void WINAPI remove_leading_ending_spaces(
	IN char * buffer,
	OUT char * mod_buffer)
{
	while(buffer[0] == ' ' || buffer[0] == '\t') {
		buffer++;
	}

	while(buffer[strlen(buffer) - 1] == ' ' ||
		buffer[strlen(buffer) - 1] == '\t') {

		buffer[strlen(buffer) - 1] = '\0';
	}

	strcpy(mod_buffer, buffer);

}

/*-----------------------------------------------------------------------------
 * Function:
 *		strprefix
 * Parameters:
 *		IN char *buffer
 *		IN char *prefix
 * Description:
 *		Checks whether the second argument is a prefix of the first one
 * Returns:
 *		int
 *			-1 or 1 = This is not a prefix of the buffer
 *			0 = This IS a prefix of the buffer
 *-----------------------------------------------------------------------------
 */
int WINAPI strprefix(IN const char *buffer, IN const char *prefix)
{
	if(buffer == NULL || prefix == NULL) {

		return -1;
	}

	int length_of_prefix = (int) strlen(prefix);
	int length_of_buffer = (int) strlen(buffer);

	if(length_of_prefix == 0) {

		return 1;
	} else if(length_of_buffer == 0) {

		return -1;
	}

	for(int counter = 0; counter < length_of_prefix; counter++) {

		if(toupper(buffer[counter]) < toupper(prefix[counter])) {

			return -1;
		} else if(toupper(buffer[counter]) > toupper(prefix[counter])) {

			return 1;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		change_to_single
 * Parameters:
 *		IN iegd_esc_set_dc_t set_dc - new dc to set 
 * Description:
 *		sets dc to single mode with the new primary
  * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE change_to_single(
	IN iegd_esc_set_dc_t set_dc)
{
	iegd_esc_set_dc_t temp_dc = set_dc;
	EXIT_CODE exit_code = EXIT_OK;
	int esc_status;

	temp_dc.dc = GET_SINGLE_DC(temp_dc.dc);
	esc_status = os_send_escape(INTEL_ESCAPE_SET_DC,sizeof(iegd_esc_set_dc_t),
		(char *) &temp_dc,0,NULL);

	if(esc_status != INTEL_ESCAPE_SUCCESS) {
		WRITE_MSG(0, (MSG_ERROR,
			"ERROR while sending INTEL_ESCAPE_SET_DC"
			" escape key"));
		exit_code = (esc_status == INTEL_ESCAPE_ERROR)? EXIT_OS_CALL_ERROR
			: EXIT_NOT_IMPLEMENTED_ERROR;
		return exit_code;
	}
	return exit_code;
}


/*-----------------------------------------------------------------------------
 * Function:
 *   enable_disable_port
 * Parameters:
 *   IN unsigned long iegd_esc_port_ctrl_t port_ctrl
 * Description:
 *   This function enables or disables a port
 * Returns:
 *   EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE enable_disable_port(IN iegd_esc_port_ctrl_t port_ctrl)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_status_t status;
	int esc_status;

	memset(&status, 0, sizeof(iegd_esc_status_t));
	if(port_ctrl.port == 0) {

		WRITE_MSG(0, (MSG_ERROR, "Invalid port specified\n"));
		exit_code = EXIT_INVALID_PARAM_ERROR;

	} else {

		if((esc_status = os_send_escape(INTEL_ESCAPE_ENABLE_PORT,
			sizeof(iegd_esc_port_ctrl_t),
			(char *) &port_ctrl,
			sizeof(iegd_esc_status_t),
			(char *) &status)) == INTEL_ESCAPE_SUCCESS && status.status == 0) {


		} else {

			WRITE_MSG(0, (MSG_ERROR,
				"ERROR while sending INTEL_ESCAPE_ENABLE_PORT escape key"));
			exit_code = 
				(esc_status == INTEL_ESCAPE_ERROR)
				? EXIT_OS_CALL_ERROR
				: EXIT_NOT_IMPLEMENTED_ERROR;
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *   en_dis_port
 * Parameters:
 *   IN unsigned long is_enable - If 1, enable the port, if 0, disable it
 *   IN int port_num - port number to enable or disable
 * Description:
 *   This function is a wrapper for enable_disable_port
 * Returns:
 *   EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE en_dis_port_num(IN unsigned long is_enable, IN int port_num)
{
	EXIT_CODE exit_code = EXIT_OK;
	iegd_esc_port_ctrl_t port_ctrl;

	memset(&port_ctrl, 0, sizeof(iegd_esc_port_ctrl_t));

	/* Fill out the params */
	port_ctrl.enable = (int) is_enable;
	port_ctrl.port = port_num;

	exit_code = enable_disable_port(port_ctrl);
	return exit_code;
}



/*-----------------------------------------------------------------------------
 * Function:
 *		get_screen_info
 * Parameters:
 *		IN disp_scrn_info_t *disp
 * Description:
 *		calls iegd_get_disp_port_mapping() and finds the relative scrn
 * number of the displays attached to primary and secondary ports  It also fills
 * in the is_primary field -  true if the display is primary from a windows
 * perpective, false otherwise 
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE get_screen_info(IN scrn_info_t *disp)
{
	unsigned long disp_port_table_size = 0, index, port_index;
	unsigned long cur_dc;
	unsigned long prim_pipe_port,sec_pipe_port;
	EXIT_CODE exit_code;
	disp_port_info_t *disp_port_table = NULL;

	cur_dc = iegd_get_current_dc();
	if (cur_dc == 0){
		return EXIT_INVALID_PARAM_ERROR;
	}
	prim_pipe_port = GET_DC_PORT_NUM(cur_dc,1);
	sec_pipe_port = GET_DC_PORT_NUM(cur_dc,5);

	exit_code = iegd_get_disp_port_mapping(&disp_port_table,
		&disp_port_table_size);

	/* If successful, we should display the mappings */

	if(exit_code == EXIT_OK) {
		for(index = 0; index < disp_port_table_size; index++) {
			for(port_index = 0;
				port_index < disp_port_table[index].port_array_size;
				port_index++) {
				if(disp_port_table[index].port_array[port_index].port_num
					== prim_pipe_port) {
						disp->scrn_num[0] =
							disp_port_table[index].rel_display_num;
						disp->is_primary[0] =
							disp_port_table[index].is_primary;
				}
				if(disp_port_table[index].port_array[port_index].port_num
					== sec_pipe_port) {
						disp->scrn_num[1] =
							disp_port_table[index].rel_display_num;
						disp->is_primary[1] =
							disp_port_table[index].is_primary;
				}
			}
		}
	}
	iegd_free_disp_port_mapping(disp_port_table);

	return exit_code;
}


/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_video_fps
 *
 * Parameters:
 *  IN int start - If the value is 1, it starts/restarts the counting of video
 *				   frames. If the value is 0, it stops the counting and get the
 *				   
 *
 *  OUT igd_esc_video_info_t *video_fps_info - contains video information, i.e.
 *											   frame counts, start time and end
 *											   time, hardware/software decode flag,
 *											   and blend/flip flag.
 *                          
 *
 * Description:
 *
 *  This function will get video information, i.e. frame counts, start time and end
 *	time, hardware/software decode flag and blend/flip flag,
 *
 * Returns:
 *
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_video_fps(IN int start, OUT igd_esc_video_info_t *video_fps_info)
{
	EXIT_CODE exit_code = EXIT_OK;

	int esc_status;
	
	if(start == 1){
		
		/* Call escape to start counting frame when video is played */
		if((esc_status = os_send_escape(INTEL_ESCAPE_GET_VID_RENDERING_INFO,
						sizeof(int),
						(char *) &start,
						0,
						NULL)) == INTEL_ESCAPE_SUCCESS) {

		} else {

			WRITE_MSG(0, (MSG_ERROR, "ERROR while sending"
								" INTEL_ESCAPE_GET_VID_RENDERING_INFO escape"
								" key"));
			exit_code = 
			(esc_status == INTEL_ESCAPE_ERROR)
			? EXIT_OS_CALL_ERROR
			: EXIT_NOT_IMPLEMENTED_ERROR;
		}


	} else {

		if(!video_fps_info){
			
			WRITE_MSG(0, (MSG_ERROR,
			"ERROR: Invalid pointer to video fps info"));
			exit_code = EXIT_INVALID_PARAM_ERROR;

		} else {

			/* 
			 * Call escape to stop counting frame and retrieve number of frames
			 * , start time, end time, flag to indicate if it's
			 * hardware decoded and flag to indicate if overlay
			 * is used
			 */
			if((esc_status = os_send_escape(INTEL_ESCAPE_GET_VID_RENDERING_INFO,
								sizeof(int),
								(char *) &start,
								sizeof(igd_esc_video_info_t),
								(char *) video_fps_info)) == INTEL_ESCAPE_SUCCESS) {
				
			
			} else {

				WRITE_MSG(0, (MSG_ERROR, "ERROR while sending"
								" INTEL_ESCAPE_GET_VID_RENDERING_INFO escape"
								" key"));
				exit_code = 
					(esc_status == INTEL_ESCAPE_ERROR)
					? EXIT_OS_CALL_ERROR
					: EXIT_NOT_IMPLEMENTED_ERROR;
			}
		}
	}

	return exit_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_golden_htotal
 *
 * Parameters:
 *
 *  IN iegd_esc_mode_list_t **in_mode_table,
 *              mode that we want to obtain the Htotal value
 *  OUT iegd_esc_mode_list_t **out_mode_table, 
 *              The mode returned from ioctl that 
 *              contains the additional golden htotal variable stored in
 *              reserved_dd variable in the mode table structure.
 *
 * Description:
 *
 *  This function will return a mode with a a golden htotal value
 *  We will send in a mode in this function for the driver to calculate
 *  the golden htotal.
 *
 * Returns:
 *
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *      EXIT_NOT_AVAIL_ERROR - This particular interface is not available
 *                          on this Operating System/Chipset combination.
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_golden_htotal(
    IN iegd_esc_mode_list_t **in_mode,
    OUT iegd_esc_mode_list_t **out_mode)
{
    EXIT_CODE exit_code = EXIT_OK;
    int esc_status;

    /*
    * If this variable is not NULL, that means the caller sent us an
    * already allocated variable which is going to cause memory leaks.
    * This is not an error, but at least we should warn the caller.
    */
    if(!(*out_mode)) {

        WRITE_MSG(0, (MSG_WARNING,
            "ERROR: Output mode NULL. Caller must allocate"
            " memory for output"));
        exit_code = EXIT_INVALID_PARAM_ERROR;
        return exit_code;
    }

    if(!(*in_mode)) {

        WRITE_MSG(0, (MSG_ERROR,
             "ERROR: Input mode NULL. Caller must specify"
            " a NON NULL mode"));
        exit_code = EXIT_INVALID_PARAM_ERROR;
        return exit_code;
    }

    if(exit_code == EXIT_OK) {

        /* parameters are checked out, now call the ioctl */
        if((esc_status = os_send_escape(INTEL_ESCAPE_QUERY_GOLDEN_HTOTAL,
            sizeof(iegd_esc_mode_list_t),
            (char *) *in_mode,
            sizeof(iegd_esc_mode_list_t),
            (char *) *out_mode)) != INTEL_ESCAPE_SUCCESS) {

            WRITE_MSG(0, (MSG_ERROR,
                "ERROR while sending INTEL_ESCAPE_QUERY_GOLDEN_HTOTAL"
                " escape key"));
            exit_code =
                (esc_status == INTEL_ESCAPE_ERROR)
                ? EXIT_OS_CALL_ERROR
                : EXIT_NOT_IMPLEMENTED_ERROR;
           return exit_code;
        }
    }

    return exit_code;

}


/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: emgd_info.h
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
 *  This file contains EMGD Info library interface function headers.
 *-----------------------------------------------------------------------------
 */

#ifndef _EMGD_INFO_H
#define _EMGD_INFO_H

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifdef WIN32
#define WINAPI      __stdcall
#else
#define WINAPI
#endif

extern "C" {

#include <iegd_escape.h>

}



#define FLIP_NONE                   0
#define FLIP_HORIZ                  1
#define PRIMARY_DISP                0
#define SECONDARY_DISP              1
#define MAX_ARGS                    23
#define MAX_SIZE                    256
#define MAX_PRIMARY_PORTS           0x4
#define MAX_SECONDARY_PORTS         0x3
#define IS_SINGLE_MODE(dc)          (((dc & 0xf) == MULTI_TYPE_SINGLE)   ? 1 : 0)
#define IS_TWIN_MODE(dc)            (((dc & 0xf) == MULTI_TYPE_TWIN)     ? 1 : 0)
#define IS_CLONE_MODE(dc)           (((dc & 0xf) == MULTI_TYPE_CLONE)    ? 1 : 0)
#define IS_EXTENDED_MODE(dc)        (((dc & 0xf) == MULTI_TYPE_EXTENDED) ? 1 : 0)
#define PRIMARY_CHANGED             0x1
#define SECONDARY_CHANGED           0x2
#define ALL_DISP_CHANGED            PRIMARY_CHANGED | SECONDARY_CHANGED
#define IS_PRIMARY_PORT_CHANGED(cur_dc,set_dc)  \
		(((GET_PRIMARY_PORT(cur_dc)) != (GET_PRIMARY_PORT(set_dc)))   ? 1 : 0 )
#define DISP_CHANGED                 0x1
#define GET_SINGLE_DC(dc)            (dc & 0x000000F0) | 0x1
/* 
 * GET_DC_PORT_NUM
 *
 * Get the port number at location i in dc
*/ 
#define GET_DC_PORT_NUM(dc, i) ((dc >> (i * 4)) & 0x0f)

enum EXIT_CODE {

	EXIT_OK,
	EXIT_GENERIC_ERROR,
	EXIT_PARSER_ERROR,
	EXIT_CFGFILE_ERROR,
	EXIT_LOGFILE_ERROR,
	EXIT_INVALID_PARAM_ERROR,
	EXIT_OS_CALL_ERROR,
	EXIT_LIB_INIT_ERROR,
	EXIT_REGISTRY_ERROR,
	EXIT_DEVICE_NOT_FOUND,
	EXIT_NOT_IMPLEMENTED_ERROR,
};

/* Version Info */
typedef struct _version_t {
	unsigned long major;
	unsigned long minor;
	unsigned long build;
} version_t;

/* Operating System Info */
typedef struct _os_info_t {
	char os_name[MAX_SIZE];
	char version_info[MAX_SIZE];
	char distribution[MAX_SIZE];
	char additional_info[MAX_SIZE];
} os_info_t;

/* Chipset Info */
typedef struct _chipset_info_t {
	unsigned short chipset;
	unsigned short rev_id;
	char chipset_name[MAX_SIZE];
} chipset_info_t;

typedef struct _port_info_t {
	unsigned long port_num;
	iegd_esc_port_info_t port_info;
} port_info_t;

typedef struct _disp_port_info_t {
	unsigned long abs_display_num;
	unsigned long rel_display_num;
	bool is_primary;
	port_info_t port_array[MAX_PRIMARY_PORTS];
	unsigned long port_array_size;
} disp_port_info_t;

typedef struct _disp_set_t {
	unsigned long max_ports;
	unsigned long disp_flag;
} disp_set_t;

typedef struct _scrn_info_t {
	unsigned long scrn_num[2];
	unsigned long is_primary[2];
	unsigned short chng_flag;
} scrn_info_t;

/*----------------------------------------------------------------------
 * Function:
 *
 *  iegd_open
 *
 * Parameters:
 *
 *  IN bool is_driver_present - This parameter is reserved and must be set
 *                              set to false.
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_open(IN bool is_driver_present = false);

/*----------------------------------------------------------------------
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
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_lib_version(OUT version_t *version_info);

/*----------------------------------------------------------------------
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
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *---------------------------------------------------------------------
 */
EXIT_CODE iegd_get_os_name(OUT os_info_t *os_info);

/*----------------------------------------------------------------------
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
 *  EXIT_CODE
 *      EXIT_OK - The function call succeeded.
 *      EXIT_INVALID_PARAM_ERROR - An invalid parameter was sent to this
 *                          function.
 *      EXIT_OS_CALL_ERROR - There was an error in the underlying
 *                          operating system calls.
 *---------------------------------------------------------------------
 */
EXIT_CODE iegd_get_chipset_name(OUT chipset_info_t *chipset_info);


/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
void WINAPI iegd_decode_exit_code(IN int exit_code, OUT char *str_exit_code);

/*----------------------------------------------------------------------
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
 *  OUT iegd_esc_mode_list_t **mode_table - The set of modes that this port
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_mode_list(
	IN unsigned long dc,
	IN unsigned long display_num,
	OUT iegd_esc_mode_list_t **mode_table,
	OUT unsigned long *mode_table_size);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
void WINAPI iegd_free_mode_list(IN iegd_esc_mode_list_t *mode_table);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_mode(
	IN unsigned long display_num,
	IN iegd_esc_mode_list_t *mode_to_set,
	IN unsigned short bpp);

/*----------------------------------------------------------------------
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
 *  (dc's) supported by the underlying hardware. This function will allocate
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_dc_list(
	OUT unsigned long **dc_table,
	OUT char ***str_dc_table,
	OUT unsigned long *dc_table_size);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
void WINAPI iegd_free_dc_list(
	IN unsigned long *dc_table,
	IN char **str_dc_table,
	IN unsigned long dc_table_size);

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
	IN iegd_esc_mode_list_t *secondary_mode=0,
	IN unsigned short primary_bpp=0,
	IN unsigned short secondary_bpp=0,
	IN unsigned short chng_flag=ALL_DISP_CHANGED);

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
	IN bool *was_extended);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
unsigned long WINAPI iegd_get_current_dc();

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
unsigned long WINAPI iegd_get_port(
	IN unsigned long cur_dc,
	IN unsigned long display,
	IN unsigned long port_location);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_flip_rot(
	IN unsigned long port_num,
	OUT unsigned long *flip_enable,
	OUT unsigned long *rot_degree);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_flip_rot(
	IN unsigned long port_num,
	IN unsigned long flip_enable,
	IN unsigned long rot_degree);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_driver_info(
	OUT iegd_esc_driver_info_t *driver_info);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_port_info(
	IN unsigned long port_num,
	OUT iegd_esc_port_info_t *port_info);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_port_attributes(
	IN unsigned long port_num,
	OUT iegd_esc_attr_t **attr_info,
	OUT unsigned long *num_attributes);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_free_port_attributes(
	IN iegd_esc_attr_t *attr_info);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_port_attributes(
	IN unsigned long port_num,
	IN iegd_esc_attr_t *attr);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_set_overlay_params(
	IN iegd_esc_color_params_t *color_params);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_overlay_params(
	OUT iegd_esc_color_params_t *color_params);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_display_firmware(
	IN unsigned long port_num,
	IN unsigned long block_num,
	OUT unsigned char *fw_info);


/*----------------------------------------------------------------------
 * Function:
 *
 *  iegd_wait
 *
 * Parameters:
 *
 *  IN iegd_hal_info_t *args
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_wait(IN unsigned long num_seconds);


/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
void WINAPI iegd_close();


/*----------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_disp_port_mapping
 *
 * Parameters:
 *
 *  OUT disp_port_info_t **disp_port_table - A pointer to an array
 *                          containing a mapping of displays and ports.
 *                          This parameter should be NULL when calling.
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
 *---------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_get_disp_port_mapping(
	OUT disp_port_info_t **disp_port_table,
	OUT unsigned long *disp_port_table_size);


/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
void WINAPI iegd_free_disp_port_mapping(IN disp_port_info_t *disp_port_table);

/*-----------------------------------------------------------------------------
 * Function:
 *		iegd_en_dis_port
 * Parameters:
 *		IN unsigned long is_enable - If 1, enable the port, if 0, disable it
 *		IN int port_num - The number of the port to enable or disable
 *							1-TV,2-SDVO-B,3-SDVO_C,4-INT_LVDS,5-ANALOG
 * Description:
 *		This function enables or disables a port
 * Returns:
 *		EXIT_CODE
 *-----------------------------------------------------------------------------
 */
EXIT_CODE WINAPI iegd_en_dis_port(IN unsigned long is_enable, IN int port_num);

/*----------------------------------------------------------------------
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
 *---------------------------------------------------------------------
 */
EXIT_CODE match_disp_ports(
	OUT disp_port_info_t *disp_port_table,
	IN unsigned long disp_port_table_size);

/*-----------------------------------------------------------------------------
 * Function:
 *
 *  iegd_get_video_fps
 *
 * Parameters:
 *  IN int start - If the value is 1, it starts/restarts the counting of video
 *				   frames. If the value is 0, it stops the counting and get the
 *				   frame counts, start time and end time, hardware/software
 *				   decode flag, and blend/flip flag.
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
EXIT_CODE WINAPI iegd_get_video_fps(IN int start, OUT igd_esc_video_info_t *video_fps_info);


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
 *  This function will return a mode  with a a golden htotal value
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
     OUT iegd_esc_mode_list_t **out_mode);


#endif

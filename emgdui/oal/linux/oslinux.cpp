/* -*- pse-c -*-
 *-----------------------------------------------------------------------------
 * Filename: oslinux.cpp
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
 *  This file has the code for communicating with the Linux OS and the EMGD
 *  driver through the use of Escapes/IOCTLs.
 *-----------------------------------------------------------------------------
 */



extern "C" {

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xrandr.h>
#include <intel_escape.h>
#include <xorg-server.h>
}

#include <osfunc.h>
#include <halcmn.h>
#include <dbgprint.h>
#include "oslinux.h"
#include <string.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fstream>

/* Global variables */
int g_fd = 0;
void *g_mem = NULL, *g_mmio = NULL;
unsigned char g_raw_device[256];
emgd_pci_device g_device;
bool g_device_present = false;

/*-----------------------------------------------------------------------------
 * Function:
 *		os_initialize
 * Parameters:
 *		IN bool reserved - Only present for windows compatibility. No
 *							current use.
 * Description:
 *		This function will do any laods for this OS
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_initialize(IN bool reserved)
{
	bool ret_val = true;

	ret_val = DETECT_GMCH(&g_mmio, &g_mem);

	if(ret_val) {

		g_device_present = true;
	}

	return ret_val;
}


/*-----------------------------------------------------------------------------
 * Function:
 *		os_uninitialize
 * Parameters:
 *		NONE
 * Description:
 *		This function will do any unloads for this OS.
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_uninitialize()
{
	if(g_mmio) {
		/* Unmapping mmio space that we may have allocated */
		munmap(g_mmio, 0x8000);
	}

	if(g_mem) {
		/* Unmapping any graphics memory that we may have allocated */
		munmap(g_mem, 128*1024*1024);
	}

	close(g_fd);

	return true;
}


/*-----------------------------------------------------------------------------
 * Function:
 *		os_get_name
 * Parameters:
 *		OUT os_info_t *os_info
 * Description:
 *		This function gets the opearting system name
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_get_name(OUT os_info_t *os_info)
{
	bool ret_code = true;
	utsname uts;
	char line[MAX_SIZE];
	ifstream infile;

	uname(&uts);

	sprintf(os_info->os_name, "OS NAME: %s, Release: %s",
		uts.sysname, uts.release);
	sprintf(os_info->version_info, "Version: %s", uts.version);

	infile.open("/etc/issue", ios::in);

	if(infile) {

		infile.getline(line, MAX_SIZE);
		snprintf(os_info->distribution, MAX_SIZE, "Distribution: %s", line);

		infile.close();
	}

	sprintf(os_info->additional_info, 
		"X Server Version: %d.%d.%d.%d\n",
		XORG_VERSION_CURRENT / 10000000,
		(XORG_VERSION_CURRENT % 10000000) / 100000,
		(XORG_VERSION_CURRENT % 100000) / 1000,
		XORG_VERSION_CURRENT % 1000);

	return ret_code;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		os_get_system_memory
 * Parameters:
 *		OUT iegd_hal_info_t *args
 * Description:
 *		Get the system memory info
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_get_system_memory(OUT iegd_hal_info_t *args)
{
	bool ret_val = true;
	char line[MAX_SIZE];
	ifstream infile;
	int index = 0;

	/* we open up the /proc/meminfo file to read some stats from it */
	infile.open("/proc/meminfo", ios::in);


	if(!infile) {

		WRITE_MSG(0, (MSG_ERROR, "Couldn't open /proc/meminfo"));
		ret_val = false;

	} else {

		while(!infile.eof() && infile.getline(line, MAX_SIZE)) {

			if(strprefix(line, "MemTotal:") == 0   ||
				strprefix(line, "MemFree:") == 0   ||
				strprefix(line, "SwapTotal:") == 0 ||
				strprefix(line, "SwapFree:") == 0) {

				strcpy(args->arg[index++], line);

			}
		}

		args->total_args_filled = index;

		infile.close();

	}

	return ret_val;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		os_get_cpu_info
 * Parameters:
 *		OUT iegd_hal_info_t *args
 * Description:
 *		Get the CPU info by opening /proc/cpuinfo file
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_get_cpu_info(OUT iegd_hal_info_t *args)
{
	/* We can use /usr/sbin/x86info to get the CPU Information. */
	char buf[MAX_SIZE], mod_buffer[MAX_SIZE];
	const char cpuid_file[MAX_SIZE] = "/proc/cpuinfo";
	const char start_keyword[MAX_SIZE] = "vendor_id";
	const char end_keyword[MAX_SIZE] = "model name";

	bool ret_val = true, capture_flag = false;
	ifstream infile;
	int index = 0;

	infile.open(cpuid_file, ios::in);

	/* Make sure that the above command created a file */
	if(infile) {

		/* Go through that log file */
		while(!infile.eof() && infile.getline(buf, MAX_SIZE)) {

			/*
			 * Some lines in this file have leading and ending spaces so we
			 * gotta remove them first
			 */
			remove_leading_ending_spaces(buf, mod_buffer);

			if(strprefix(mod_buffer, start_keyword) == 0) {

				capture_flag = true;

			}

			if (capture_flag) {

				/*
				 * Start capturing information as soon as we get to the
				 * Family line
				 */
				sprintf(args->arg[index++], "%s", mod_buffer);

				if(strprefix(mod_buffer, end_keyword) == 0) {

					capture_flag = false;
					break;
				}
			}
		}

		args->total_args_filled = index;
		infile.close();
	} else {

		WRITE_MSG(0, (MSG_ERROR, "ERROR: No cpuid logfile file found"));
		ret_val = false;
	}

	return ret_val;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		os_send_escape
 * Parameters:
 *		IN int escape - escape command to send
 *		IN int input_size - The input parameter size in bytes
 *		IN char *input - Input parameter to the escape API
 *		IN int output_size - The output paramater size in bytes
 *		OUT char *output - Output parameter to the escape API
 * Description:
 *		This function sends an escape command to the display driver
 * Returns:
 *		int -
 *			INTEL_ESCAPE_NOT_SUPPORTED	-1
 *			INTEL_ESCAPE_SUCCESS		0
 *			INTEL_ESCAPE_ERROR			1
 *-----------------------------------------------------------------------------
 */
int os_send_escape(IN int escape,
					IN int input_size,
					IN char *input,
					IN int output_size,
					OUT char *output)
{
	int status = INTEL_ESCAPE_ERROR;
	const char *display = ":0.0";
	Display *dpy = NULL;
	int event_basep, error_basep;

	dpy = XOpenDisplay(display);

	if(!dpy) {

		WRITE_MSG(0, (MSG_WARNING,
				"Error opening display. Check your DISPLAY"
				" environment variable"));
	} else {

		if(iegd_query_extension(dpy, &event_basep, &error_basep)) {

			status = iegd_escape(dpy, escape,
				input_size, input, output_size, output);

		} else {

			WRITE_MSG(0, (MSG_WARNING, "WARNING: EMGD not loaded"));
		}

		XCloseDisplay(dpy);
	}

	return status;
}

/*-----------------------------------------------------------------------------
 * Function:
 *	os_change_disp_settings
 * Parameters:
 *	IN iegd_esc_set_dc_t *set_dc - new display mode structure
 *	IN bool was_extended_mode - whether we were in extended mode or not
 *	IN scrn_info_t scrn_info - pointer to screen info structure
 * Description:
 *	This function changes the display settings and gets Linux OS in
 *	sync with the driver's new settings.
 * Returns:
 *	bool -
 *		true  = success
 *		fales = failure
 *-----------------------------------------------------------------------------
 */
bool os_change_disp_settings(
	IN iegd_esc_set_dc_t *set_dc,
	IN bool was_extended_mode,
	IN scrn_info_t *scrn_info)
{
	/* This function is not implemented yet */
	bool ret_val = true;

	/* If the new display mode is SINGLE, TWIN or CLONE */
	if(IS_SINGLE_MODE(set_dc->dc) ||
		IS_CLONE_MODE(set_dc->dc) ||
		IS_TWIN_MODE(set_dc->dc)) {

		/*
		 * ...and we were in extended mode previously, then we need to detach
		 * the secondary display
		 */
		if(was_extended_mode) {

			WRITE_MSG(0, (MSG_ERROR, "ERROR: Changing from Extended to any"
						" other display mode is not allowed"));
			ret_val = false;
		} else {

			/* else we just need to set the mode */
			ret_val = os_set_scrn_res(0,
					set_dc->iegd_timings[0].width,
					set_dc->iegd_timings[0].height,
					set_dc->iegd_timings[0].refresh, 32);
		}

	} else if(IS_EXTENDED_MODE(set_dc->dc) && !was_extended_mode) {

		/*
		 * The above if says if the new display mode is EXTENDED and we were
		 * not in extended mode before this change happened.
		 * In such a situation we need to extend the desktop through the OS
		 * APIs
		 */
		WRITE_MSG(0, (MSG_ERROR, "ERROR: Changing from any"
					" display mode to Extended mode is not allowed"));
		ret_val = false;
	} else {

		/*
		 * else we could be going in this case if we were in extended mode
		 * before and are also in extended mode now
		 * In this case, all we need to do is a set mode
		 */
		ret_val = os_set_scrn_res(0,
				set_dc->iegd_timings[0].width,
				set_dc->iegd_timings[0].height,
				set_dc->iegd_timings[0].refresh, 32);

		if(ret_val) {

			ret_val = os_set_scrn_res(1,
					set_dc->iegd_timings[1].width,
					set_dc->iegd_timings[1].height,
					set_dc->iegd_timings[1].refresh, 32);
		}
	}

	return ret_val;
}

/*-----------------------------------------------------------------------------
 * Function:
 *		os_set_scrn_res
 * Parameters:
 *		IN unsigned long screen_num - The screen number to set the width of
 *		IN unsigned short width - The new width to set
 *		IN unsigned short height - The new height to set
 *		IN unsigned short refresh - The new refresh rate to set
 *		IN unsigned short bpp - The new bits per pixel to set
 *		IN unsigned long rotation - New rotation value
 *		IN unsigned long force_reset - Force a reset if 1
 * Description:
 *		This function sets a particular screens resolution to some
 *		user specified value
 * Returns:
 *		bool -
 *			true  = success
 *			fales = failure
 *-----------------------------------------------------------------------------
 */
bool os_set_scrn_res(
	IN unsigned long screen_num,
	IN unsigned short width,
	IN unsigned short height,
	IN unsigned short refresh,
	IN unsigned short bpp,
	IN unsigned long rotation,
	IN unsigned long force_reset)
{
	long index, size, new_size;
	int nsize, current_rate;
	bool ret_val = true;
	unsigned long disp_array[MAX_DISPLAYS];
	Window root;
	XRRScreenConfiguration *sc;
	SizeID current_size;
	Rotation current_rotation;
	XRRScreenSize *sizes;
	Status status = RRSetConfigFailed;
	int event_base, error_base;
	Display *dpy = NULL;
	char display[MAX_SIZE];
	int major_opcode, first_event, first_error;

	sprintf(display, ":0.%d", (int) screen_num);

	size = 0;

	dpy = XOpenDisplay(display);

	if(!dpy) {

		WRITE_MSG(0, (MSG_WARNING,
				"Error opening display. Check your DISPLAY"
				" environment variable"));
		ret_val = false;

	} else {

		for(index = 0; index < ScreenCount(dpy); index++) {
			disp_array[size++] = index;
		}

		/*
		 * If the user gave us a particular display then we should only change
		 * the mode settings for that display
		 */
		if(screen_num != ALL_DISPLAYS) {

			if(screen_num < (unsigned long) size) {

				size = 1;
				/* Put the n'th display number as the first one */
				disp_array[0] = disp_array[screen_num];

			} else {

				WRITE_MSG(0, (MSG_ERROR, "ERROR: Invalid screen number"));
				ret_val = false;
			}
		}

		if(!XQueryExtension(dpy, "RANDR", &major_opcode,
				&first_event, &first_error)) {

			WRITE_MSG(0, (MSG_ERROR, "ERROR: XRANDR Extension not loaded"));
			ret_val = false;
		}

		for(index = 0; index < size && ret_val; index++) {

			root = RootWindow(dpy, disp_array[index]);

			sc = XRRGetScreenInfo(dpy, root);
			current_rate = XRRConfigCurrentRate(sc);

			if(sc) {

				current_size = XRRConfigCurrentConfiguration(sc,
						&current_rotation);

				sizes = XRRConfigSizes(sc, &nsize);
				/*
				 * If the width, height and refresh are all 0, then this
				 * means that we need to get the default width, height
				 * and refresh
				 */
				if(width == 0 && height == 0 && refresh == 0) {

					new_size = current_size;

				} else {

					for(new_size = 0; new_size < nsize; new_size++) {

						if(sizes[new_size].width == width &&
							sizes[new_size].height == height) {

							break;
						}
					}

					/*
					 * If we didn't find any sizes that match, then return an
					 * error
					 */
					if(new_size >= nsize) {

						WRITE_MSG(0,
								(MSG_ERROR, "ERROR: Invalid modes specified"));
						ret_val = false;
					}
				}


				if(ret_val) {

					/* we should test configureNotify on the root window */
					XSelectInput(dpy, root, StructureNotifyMask);

					XRRSelectInput(dpy, root, RRScreenChangeNotifyMask);

					status = XRRSetScreenConfigAndRate(dpy, sc,
						DefaultRootWindow (dpy), (SizeID) new_size,
						(Rotation) 1, refresh, CurrentTime);

					XRRQueryExtension(dpy, &event_base, &error_base);
					XRRFreeScreenConfigInfo(sc);
				}
			}
		}

		XCloseDisplay(dpy);
	}

	return ret_val;
}


/*-----------------------------------------------------------------------------
 * Function:
 *		os_wait
 * Parameters:
 *		IN unsigned long num_seconds - The number of seconds to wait for
 * Description:
 *		This function makes the program go to sleep for the specified
 *		number of seconds. This function can be used when running a script
 *		and the user requires to see display changes before the next
 *		command is run.
 * Returns:
 *		bool -
 *			true  = success
 *			false = failure
 *-----------------------------------------------------------------------------
 */
bool os_wait(IN unsigned long num_seconds)
{
	unsigned long sleep_counter;

	WRITE_MSG(1, (MSG_INFO, "Waiting for: %5ld seconds\n\n"));

	for(sleep_counter = num_seconds; sleep_counter > 0; sleep_counter--) {

		fflush(stdout);
		sleep(1);
	}

	return true;
}

/*-----------------------------------------------------------------------------
 * Function:
 *
 * Parameters:
 *
 * Description:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------
 */
void os_get_full_path(IN const char *file, OUT char *full_path)
{
	char *path, *dir, candidate[MAX_SIZE];

	strcpy(full_path, file);
	path = getenv("PATH");
	if(path) {

		path = strdup(path);
		for(dir = strtok(path, ":"); dir != NULL; dir = strtok(NULL, ":")) {

			sprintf(candidate, "%s/%s", dir, file);
			if(access(candidate, X_OK) == 0) {

				strcpy(full_path, candidate);
				break;
			}
		}
		free(path);
	}
}

/*-----------------------------------------------------------------------------
 * Function:
 *	os_get_intel_displays
 * Parameters:
 *	OUT disp_port_info_t *disp_port_array
 *	OUT unsigned long *disp_port_array_size
 * Description:
 *
 * Returns:
 *  disp_status_t
 *-----------------------------------------------------------------------------
 */
void os_get_intel_displays(
	OUT disp_port_info_t *disp_port_array,
	OUT unsigned long *disp_port_array_size)
{

}

/*-----------------------------------------------------------------------------
 * Function:
 *	os_get_scrn_res
 * Parameters:
 *	IN unsigned long screen_num - The screen number whose display info we need
 *	OUT unsigned long *width - Width of the display
 *	OUT unsigned long *height - Height of the display
 *	OUT unsigned long *refresh - Refresh of the display
 *	OUT unsigned long *bpp - Bits Per Pixel of the display
 *	OUT unsigned long *rotation - Rotation info of the display
 * Description:
 *	This function returns the current screen resolution of a particular display
 *	according to the OS.
 * Returns:
 *	bool -
 *		true  = success
 *		false = failure
 *-----------------------------------------------------------------------------
 */
bool os_get_scrn_res(
	IN unsigned long screen_num,
	OUT unsigned long *width,
	OUT unsigned long *height,
	OUT unsigned long *refresh,
	OUT unsigned long *bpp,
	OUT unsigned long *rotation)
{
	long index, size;
	int nsize, current_rate;
	bool ret_val = true;
	unsigned long disp_array[MAX_DISPLAYS];
	Window root;
	XRRScreenConfiguration *sc;
	SizeID current_size;
	Rotation current_rotation;
	XRRScreenSize *sizes;
	Display *dpy = NULL;
	char display[MAX_SIZE];
	int major_opcode, first_event, first_error;

	sprintf(display, ":0.%d", (int) screen_num);

	size = 0;

	dpy = XOpenDisplay(display);

	if(!dpy) {

		WRITE_MSG(0, (MSG_WARNING,
				"Error opening display. Check your DISPLAY"
				" environment variable"));
		ret_val = false;

	} else {

		for(index = 0; index < ScreenCount(dpy); index++) {
			disp_array[size++] = index;
		}


		if(screen_num >= (unsigned long) size) {

			WRITE_MSG(0, (MSG_ERROR, "ERROR: Invalid screen number"));
			XCloseDisplay(dpy);
			return false;

		}

		if(!XQueryExtension(dpy, "RANDR", &major_opcode,
				&first_event, &first_error)) {

			WRITE_MSG(0, (MSG_ERROR, "ERROR: XRANDR Extension not loaded"));
			XCloseDisplay(dpy);
			return false;
		}

		root = RootWindow(dpy, disp_array[screen_num]);

		sc = XRRGetScreenInfo(dpy, root);
		current_rate = XRRConfigCurrentRate(sc);

		if(sc) {

			current_size = XRRConfigCurrentConfiguration(sc,
					&current_rotation);

			sizes = XRRConfigSizes(sc, &nsize);

			*width    = sizes[current_size].width;
			*height   = sizes[current_size].height;
			*refresh  = current_rate;
			switch(current_rotation) {
			case 1:
				*rotation = 90;
				break;
			case 2:
				*rotation = 180;
				break;
			case 3:
				*rotation = 270;
				break;
			case 0:
			default:
				*rotation = 0;
				break;
			}

		} else {

			ret_val = false;
		}

		XCloseDisplay(dpy);
	}

	return ret_val;
}

/*
 *-----------------------------------------------------------------------------
 * Filename: pd.c
 * $Revision: 1.11 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2010, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *-----------------------------------------------------------------------------
 * Description:
 *  This file contains all the necessary common functions for port driver
 *  module. These functions are called only from hardware specific port
 *  drivers. These are exported via pd.h.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <io.h>

#include <memory.h>
#include <sched.h>

#include <mode.h>
#include <pd.h>
#include <pi.h>
/*!
 * @addtogroup display_group
 * @{
 */

/*!
 *
 * @param size
 *
 * @return void
 */
void *pd_malloc(unsigned long size)
{
	return OS_ALLOC(size);
} /* end pd_malloc */

/*!
 *
 * @param address
 * @param c
 * @size
 *
 * @return void
 */
void *pd_memset(void *address, int c, unsigned long size)
{
	return OS_MEMSET(address, c, size);
} /* end pd_memset */

/*!
 *
 * @param dst
 * @param src
 * @param size
 *
 * @return void
 */
void *pd_memcpy(void *dst, void *src, unsigned long size)
{
	return OS_MEMCPY(dst, src, size);
} /* end pd_memcpy */

/*!
 *
 * @param size
 *
 * @return void
 */
void pd_free(void *ptr)
{
	OS_FREE(ptr);
} /* end pd_free */

/*!
 *
 * @param usec
 *
 * @return void
 */
void pd_usleep(unsigned long usec)
{

	if (usec <= 1000) {
		OS_SLEEP(usec);
	} else {
		os_alarm_t alarm = OS_SET_ALARM((usec+999)/1000);
		do {
			OS_SCHEDULE();
		} while (!OS_TEST_ALARM(alarm));
	}

}

/*!
 *
 * @param usec
 *
 * @return void
 */
void pd_ui_usleep(unsigned long usec)
{
	OS_UISLEEP(usec);

}

/*!
 *
 * @param dest
 * @param src
 *
 * @return dest
 */
char *pd_strcpy(char *dest, const char *src)
{
	int i = 0;
	/* This can be optimized by assigning 32 bit quantities instead
	 * of 8 bit quantities. This requires knowing the length first then
	 * move the quantities. For now, this is OK. */
	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

/*!
 *
 * @param handle
 * @param driver
 *
 * @return pi_pd_register()
 */
int pd_register(void *handle, pd_driver_t *driver)
{
	return (pi_pd_register(driver));
} /* end pd_register */

/*!
 *
 * @param curr
 * @param in
 *
 * @return PD_SUCCESS on success
 * @return PD_ERR_INVALID_ATTR or PD_ERR_INCORR_ATTR_VALUE on failure
 */
int pd_check_attr(pd_attr_t *curr, pd_attr_t *in)
{
	if (!curr || !in) {
		return PD_ERR_NULL_PTR;
	}

	if (curr->id != in->id) {
		return PD_ERR_INVALID_ATTR;
	}

	switch (curr->type) {
		case PD_ATTR_TYPE_RANGE:
			if ((in->current_value < RATTR(curr)->min) ||
				(in->current_value > RATTR(curr)->max)) {
				return PD_ERR_INCORR_ATTR_VALUE;
			}
			break;
		case PD_ATTR_TYPE_LIST:
			if ((in->current_value < 1) ||
				(in->current_value > LHATTR(curr)->num_entries)) {
				return PD_ERR_INCORR_ATTR_VALUE;
			}
			break;
		case PD_ATTR_TYPE_BOOL:
			if ((in->current_value != TRUE) &&
				(in->current_value != FALSE)) {
				return PD_ERR_INCORR_ATTR_VALUE;
			}
			break;
		default:
			return PD_ERR_INVALID_ATTR;
	}
	return PD_SUCCESS;
}

/*!
 * This function searches for the requested attr_id in the attribute list
 * and returns the pointer.
 *
 * In case of LIST attribute, it will return the proper list entry.
 *
 * @param attr_list
 * @param num_attr
 * @param attr_id
 * @param flag
 *
 * @return pd_attr_t on success
 * @return NULL on failure
 */
pd_attr_t *pd_get_attr(pd_attr_t *attr_list, unsigned long num_attrs,
		unsigned long attr_id, unsigned long flag)
{
	unsigned long i;

	if (!attr_list) {
		return NULL;
	}

	for (i = 0; i < num_attrs; i++) {
		if (attr_list[i].id == attr_id) {
			if (attr_list[i].type == PD_ATTR_TYPE_LIST) {
				if (flag == PD_GET_ATTR_LIST_ENTRY) {
					return (&(attr_list[i+attr_list[i].current_value]));
				} else {
					return (&(attr_list[i]));
				}
			}
			return (&(attr_list[i]));
		}
	}
	return NULL;
} /* end pd_get_attr() */

/*!
 * Common mode filter algorithm for all port drivers
 *
 * @param call_back
 * @param in_list
 * @param out_list
 * @param dvo_info
 * @param display_info
 *
 * @return PD_SUCCESS on success
 * @return PD_ERR_NULL_PTR or PD_ERR_NOMEM on failure
 */
int pd_filter_timings(
	void *callback_context,
	pd_timing_t *in_list,
	pd_timing_t **out_list,
	pd_dvo_info_t *dvo_info,
	pd_display_info_t *display_info)
{
	igd_display_port_t *port = (igd_display_port_t *)callback_context;
	pd_timing_t    *timing   = NULL, *native_dtd = NULL;
	int            j;
	int            count     = 0;
	unsigned short fp_refresh = 60;
	pd_timing_t    *olist = NULL;
	unsigned long  fixed_timing = 0;
	int i, ret;

	if (!port || !in_list || !out_list) {
		return PD_ERR_NULL_PTR;
	}

	/* Start with fixed_res = 0 */
	display_info->fixed_res = 0;

	/* DisplayID FP width are specified */
	if (port->firmware_type == PI_FIRMWARE_DISPLAYID) {
		display_info->width = port->displayid->display_params.horz_pixels;
		display_info->height = port->displayid->display_params.vert_pixels;
		fixed_timing = port->displayid->display_params.fixed_timing;
		display_info->fixed_res = port->displayid->display_params.fixed_res;
	}

	/* Overwrite DisplayID FP values with config fpinfo, later
	 * these will be overwritten by native DTD width, height */
	if (port->fp_info) {
		/* This is done for backward compatibility:
		 * Set width, height from display port fp_info
		 * This also required dropping fp_info width and height attributes
		 * from all port drivers */
		display_info->width = (unsigned short) port->fp_info->fp_width;
		display_info->height = (unsigned short) port->fp_info->fp_height;
		/* Backward compatibility: If width and height are specified,
		 * that means it is a fixed resolution panel */
		if (port->fp_info->fp_width && port->fp_info->fp_height) {
			display_info->fixed_res = 1;
			fp_refresh = 60;
		}
	}

	/* If fixed timing also comes from user attributes then override DisplayID
	 * fixed timing and fpinfo values */
	ret = pi_get_port_init_attr(port, PD_ATTR_ID_FIXED_TIMING, &fixed_timing);
	if (fixed_timing) {
		display_info->fixed_res = 1;
		fp_refresh = 60;
	}

	/* Do gmch filtering:
	 * There is no way to get the mode context inorder to reach the
	 * gmch filtering function */

	/* First find the native resolution */
	get_native_dtd(in_list, PI_SUPPORTED_TIMINGS,
		&display_info->native_dtd, PD_MODE_DTD_FP_NATIVE);

	/* If no FP Native DTD provided, then get the native DTD
	 *   either user DTD or edid DTD */
	if (!display_info->native_dtd) {
		get_native_dtd(in_list, PI_SUPPORTED_TIMINGS,
			&display_info->native_dtd, PD_MODE_DTD_USER);
	}
	if (!display_info->native_dtd) {
		get_native_dtd(in_list, PI_SUPPORTED_TIMINGS,
			&display_info->native_dtd, PD_MODE_DTD);
	}

	/* Set up the fp width, height and refresh for the comparison */
	if (display_info->native_dtd) {
#ifndef CONFIG_MICRO
		/* If fp width, height doesn' match with native width and height,
		 * Configuration isn't correct */
		if ((display_info->width &&
				(display_info->width != display_info->native_dtd->width)) &&
			(display_info->height &&
				(display_info->height != display_info->native_dtd->height))) {
			EMGD_DEBUG("FP Width Height doesn't match with Native DTD.");
		}
#endif
		/* Overwrite native width height as panel width and height */
		display_info->width = display_info->native_dtd->width;
		display_info->height = display_info->native_dtd->height;
		fp_refresh = display_info->native_dtd->refresh;
	} else if (ret &&
		port->firmware_type != PI_FIRMWARE_DISPLAYID &&
		dvo_info->upscale) {
		/* TODO:
		 * For time being this function has to assume all upscaling encoders
		 * are connected to a fixed timing panel if no fixed_timing is
		 * specified.
		 *
		 * Once fixed_timing init-time attribute becomes mandatory, below check
		 * can be removed.
		 *
		 * If customer uses old config system and didn't specified init time
		 * fixed_timing attribute, then driver assumes it is a fixed-resolution
		 * panel. If user do specifies fixed_timing attribute, then above check
		 * will fail, and algorithm continues with whatever value set to
		 * fixed_res attr.
		 *
		 * ret != 0 means no fixed_timing user attribute
		 * firmware_type == DISPLAYID means firmware provided fixed_timing,
		 * so don't change it.
		 */
		display_info->fixed_res = 1;
	}

	/* Check native_dtd for fixed_res display */
	if (display_info->fixed_res && !display_info->native_dtd) {
		/* This happens if user provides fp_width, fp_height and didn't set
		 * fixed res parameter. In this case native_dtd will be set as part
		 * of the while loop while filtering the modes */
		EMGD_DEBUG("pd_filter_timings: No native dtd for fixed_resolution");
	}

	if (!display_info->width && !display_info->height) {
		/* If fp width and height isn't known, then enable all modes as
		 * non-fixed res display */
		display_info->fixed_res = 0;
	}

	EMGD_DEBUG("fixed_res = %u fixed_timing = %lu",
		display_info->fixed_res, fixed_timing);
	EMGD_DEBUG("fp_width = %u, fp_height = %u, fp_refresh = %u",
		display_info->width, display_info->height, fp_refresh);
	EMGD_DEBUG("min_dclk = %lu, max_dclk = %lu",
		dvo_info->min_dclk, dvo_info->max_dclk);

	/* This function can be called with following
	 * ---------------------------------------------------------------
	 *  DVO device PanelFit Display AvailableModes
	 *  DOWN  UP            Fixed?
	 * --------------------------------------------------------
	 *   0     0      0       0     All supported modes
	 *   1     0      0       0     All supported modes
	 *   0     1      0       0     All supported modes
	 *   1     1      0       0     All supported modes
	 *   0     0      1       0     All supported modes
	 *   1     0      1       0     All supported modes
	 *   0     1      1       0     All supported modes
	 *   1     1      1       0     All supported modes
	 *
	 *   0     0      0       Y     Only one mode
	 *   1     0      0       Y     Only one mode
	 *   0     1      0       Y     Only one mode
	 *   1     1      0       Y     Only one mode
	 *   0     0      1       Y     Only one mode
	 *
	 *   0     1      1       Y     All upscalable modes
	 *   1     0      1       Y     All downscalable modes
	 *
	 *   1     1      1       Y     All UP & DOWN scalable modes
	 * -------------------------------------------------------------------------
	 */

	for (i = 0; i < 2; ++i) {
		j = 0;
		timing = in_list;
		while(timing->width != PD_TIMING_LIST_END) {
			/* If mode supported and dclk is within the range */
			if ((timing->mode_info_flags & PD_MODE_SUPPORTED) &&
				((!dvo_info->max_dclk || (timing->dclk <= dvo_info->max_dclk))&&
				(!dvo_info->min_dclk || (timing->dclk >= dvo_info->min_dclk)))){

				if (
					/* fixed_res = 0 */
					!display_info->fixed_res ||

					/* no panel fit */
					(!display_info->panel_fit &&
					timing->refresh == fp_refresh &&
					timing->width   == display_info->width &&
					timing->height  == display_info->height) ||

					/* panel fit and upscale or downscale */
					(display_info->panel_fit &&
					timing->refresh == fp_refresh &&
					((dvo_info->upscale &&
					timing->width  <= display_info->width &&
					timing->height <= display_info->height &&
					(!dvo_info->upscale_min_width ||
						timing->width  >= dvo_info->upscale_min_width) &&
					(!dvo_info->upscale_min_height ||
					timing->height >= dvo_info->upscale_min_height))
#if 0
					||
					(dvo_info->downscale &&
					timing->width  >= display_info->width &&
					timing->height >= display_info->height &&
					(!dvo_info->downscale_max_width ||
						timing->width  <= dvo_info->downscale_max_width) &&
					(!dvo_info->downscale_max_height ||
						timing->height >= dvo_info->downscale_max_height))
#endif
					))){

 					if((port->pd_driver->type & (PD_DISPLAY_CRT_EXT |PD_DISPLAY_CRT)) &&
                                                  (timing->width % 8 != 0) &&
                                                        !(timing->mode_info_flags&PD_MODE_DTD_USER||timing->mode_info_flags&PD_MODE_DTD)){

							/*Filter non-standard build-in timing at vga display, 
							Do NOT filter User timing and EDID timing*/
							EMGD_DEBUG("pd_filter_timings**********: NON_CRT_TIMING: %ux%u@%u",
							timing->width,
							timing->height,
							timing->refresh);

						} else if(!i){
							count++;
						} else {

							/* copy timing */
							olist[j] = *timing;

						/* save the native_dtd timing */
						if ((timing->width   == display_info->width) &&
							(timing->height  == display_info->height) &&
							(timing->refresh == fp_refresh)) {
							native_dtd = &(olist[j]);
						}

						/* The native DTD pointer is pointing in the in_list,
						 * reset this pointer to point in the out_list */
						if (timing == display_info->native_dtd) {
							display_info->native_dtd = &olist[j];
						}

						j++;
					}
				}
			}
			timing++;
			if ((timing->width == PD_TIMING_LIST_END) && timing->extn_ptr) {
				timing = timing->extn_ptr;
			}
		}
		if(!i) {
			count++;
			olist = (pd_timing_t *) pd_malloc(count * sizeof(pd_timing_t));
			if(!olist) {
				return PD_ERR_NOMEM;
			}
		} else {
			/* Copy the END of LIST entry */
			olist[j] = *timing;
		}
	}

	/* If there is no native_dtd, then use the first matching
	 * resolution with fp width and height as native dtd */
	if (!display_info->native_dtd) {
		display_info->native_dtd = native_dtd;
	}
	if (display_info->native_dtd) {
		EMGD_DEBUG("pd_filter_timings: NativeDTD: %ux%u@%u",
			display_info->native_dtd->width,
			display_info->native_dtd->height,
			display_info->native_dtd->refresh);
		display_info->native_dtd->mode_info_flags |= PD_MODE_DTD_FP_NATIVE;
	}

	*out_list = olist;
	return PD_SUCCESS;
}



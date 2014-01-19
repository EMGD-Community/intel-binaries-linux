/*
 *-----------------------------------------------------------------------------
 * Filename: igd_pi.c
 * $Revision: 1.9 $
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
 *  Callback functions to give to port drivers. All functions are provided
 *  to port driver as callback functions. Only port driver should call
 *  these functions.
 *-----------------------------------------------------------------------------
 */

#include <context.h>

#include <memory.h>
#include <sched.h>

/* #include <igd_pi.h> */
#include <igd_debug.h>

#include <pi.h>
#include <pd.h>

/*!
 * @addtogroup display_group
 * @{
 */

/*!
 * Function to register with main driver.
 *
 * @param handle
 * @param pd_driver
 *
 * @return pi_pd_register()
 */
int igd_pd_register(void *handle, void *pd_driver)
{
	return pi_pd_register(pd_driver);
} /* igd_pd_register */

/*!
 * Function to allocate memory
 *
 * @param handle
 * @param pd_driver
 *
 * @return void
 */
void *igd_pd_malloc(unsigned long size)
{
	return OS_ALLOC(size);
} /* end igd_pd_malloc */

/*!
 * Function to set the memory
 *
 * @param address
 * @param c
 * @param size
 *
 * @return void
 */
void *igd_pd_memset(void *address, int c, unsigned long size)
{
	return OS_MEMSET(address, c, size);
} /* end igd_pd_memset */

/*!
 * Function to copy block of memory
 *
 * @param dst
 * @param src
 * @param size
 *
 * @return void
 */
void *igd_pd_memcpy(void *dst, void *src, unsigned long size)
{
	return OS_MEMCPY(dst, src, size);
} /* end igd_pd_memcpy */

/* Functions to free memory */
void igd_pd_free(void *ptr)
{
	OS_FREE(ptr);
} /* end igd_pd_free */

/*!
 * Function to sleep in micro seconds. This can be called with millisecond
 * ranges.
 *
 * @param usec
 *
 * @return void
 */
void igd_pd_usleep(unsigned long usec)
{
	if (usec <= 1000) {
		OS_SLEEP(usec);
	} else {
		os_alarm_t alarm = OS_SET_ALARM((usec+999)/1000);
		do {
			OS_SCHEDULE();
		} while (!OS_TEST_ALARM(alarm));
	}
} /* end igd_pd_usleep() */

/*!
 * Function to do a string copy
 *
 * @param dest
 * @param src
 *
 * @return dest
 */
char *igd_pd_strcpy(char *dest, char const *src)
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
} /* end igd_pd_strcpy() */

/*!
 * Function to check value of an attribute
 *
 * @param curr
 * @param in
 *
 * @return PD_SUCCESS on success
 * @return PD_ERR_NULL_PTR,PD_ERR_INVALID_ATTR, or PD_ERR_INCORR_ATTR_VALUE
 * 	on failure
 */
int igd_pd_check_attr(pd_attr_t *curr, pd_attr_t *in)
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
} /* end igd_pd_check_attr() */

/*!
 * This function searches for the requested attr_id in the attribute list
 * and returns the pointer.
 *
 * In case of LIST attribute, it will return the proper list entry.
 *
 * @param attr_list
 * @param num_attrs
 * @param attr_id
 * @param flag
 *
 * @return attr_list on success
 * @return NULL on failure
 */
pd_attr_t *igd_pd_get_attr(pd_attr_t *attr_list, unsigned long num_attrs,
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
} /* end igd_pd_get_attr() */

/*!
 * Common mode filter algorithm for all port drivers
 *
 * @param context
 * @param ilist
 * @param olist
 * @param dvo
 * @param display
 *
 * @return pd_filter_timings()
 */
int igd_pd_filter_timings(
	void *context,
	pd_timing_t *ilist,
	pd_timing_t **olist,
	pd_dvo_info_t *dvo,
	pd_display_info_t *display)
{
	return pd_filter_timings(context, ilist, olist, dvo, display);
}

igd_debug_t *igd_pd_get_igd_debug( void )
{
	return emgd_debug;
}


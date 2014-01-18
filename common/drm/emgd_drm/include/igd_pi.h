/*
 *-----------------------------------------------------------------------------
 * Filename: igd_pi.h
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
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_PI_H
#define _IGD_PI_H

/* Typedefinitions */
typedef int        (*pd_register_p)  (void *handle, void *driver);
typedef void      *(*pd_malloc_p)(unsigned long size);
typedef void      *(*pd_memset_p)(void *address, int c, unsigned long size);
typedef void      *(*pd_memcpy_p)(void *dst, void *src, unsigned long size);
typedef void       (*pd_free_p)  (void *address);
typedef void       (*pd_usleep_p)(unsigned long usec);
typedef void       (*pd_ui_usleep_p)(unsigned long usec);
typedef char      *(*pd_strcpy_p)(char *dest, char const *src);
typedef int       *(*pd_printf_p)(const char *format, ...);
typedef int        (*pd_check_attr_p)(void *curr, void *in);
typedef void      *(*pd_get_attr_p)(void *attr_list,
		unsigned long num_attrs, unsigned long attr_id, unsigned long flag);
typedef void *(*pd_get_igd_debug_p)( void );
typedef unsigned long *(*pd_get_dropped_debug_messages_p)( void );
typedef void *(*pd_get_debug_log_mutex_p)( void );
typedef int        (*pd_filter_timings_p)(void *context, void *inlist,
		void **olist, void *dvo_info, void *display_info);



/* Data structures to pass callback functions from child to parent */
typedef struct _igd_main_cb {
	unsigned long       version;
	pd_register_p       pd_register;
	pd_malloc_p         pd_malloc;
	pd_memset_p         pd_memset;
	pd_memcpy_p         pd_memcpy;
	pd_free_p           pd_free;
	pd_usleep_p         pd_usleep;
	pd_ui_usleep_p      pd_ui_usleep;
	pd_strcpy_p         pd_strcpy;
	pd_check_attr_p     pd_check_attr;
	pd_get_attr_p       pd_get_attr;
	pd_filter_timings_p pd_filter_timings;
	pd_get_igd_debug_p  pd_get_igd_debug;
	pd_get_debug_log_mutex_p        pd_get_debug_log_mutex;
	pd_get_dropped_debug_messages_p pd_get_dropped_debug_messages;
} igd_main_cb_t;

typedef struct _igd_dpd_cb {
	unsigned long   version;
} igd_dpd_cb_t;


/* Port driver init and exit function type definitions */
typedef int (*pd_init_p)  (void *);
typedef int (*pd_exit_p)  (void);

/* Function to register with main driver. */
int igd_pd_register(void *handle, void *pd_driver);

/* Function to allocate memory */
void *igd_pd_malloc(unsigned long size);

/* Function to set the memory */
void *igd_pd_memset(void *address, int c, unsigned long size);

/* Function to copy block of memory */
void *igd_pd_memcpy(void *dst, void *src, unsigned long size);

/* Functions to free memory */
void igd_pd_free(void *ptr);

/* Function to sleep in micro seconds. This can be called with millisecond
 * ranges. */
void igd_pd_usleep(unsigned long usec);

/* Function to sleep in micro seconds uniterrupted. This can be called with millisecond
 * ranges. */
void igd_pd_ui_usleep(unsigned long usec);


/* Function to do a string copy */
char *igd_pd_strcpy(char *dest, char const *src);

/* Function to check value of an attribute */
int igd_pd_check_attr(void *curr, void *in);

/* This function searches for the requested attr_id in the attribute list
 * and returns the pointer.
 *
 * In case of LIST attribute, it will return the proper list entry. */
void *igd_pd_get_attr(void *attr_list, unsigned long num_attrs,
		unsigned long attr_id, unsigned long flag);

int igd_pd_filter_timings(
	void *context,
	void *ilist,
	void **olist,
	void *dvo,
	void *display);

void *igd_pd_get_igd_debug( void );

#endif /* _IGD_PI_H */

/*
 *-----------------------------------------------------------------------------
 * Filename: sched.h
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
 *  This file contains Linux User-space implementations for the OAL io.h
 *  abstractions.
 *-----------------------------------------------------------------------------
 */

#ifndef _OAL_LINUX_USER_SCHED_H
#define _OAL_LINUX_USER_SCHED_H

#include <config.h>

#include <linux/sched.h>
#include <linux/delay.h>

typedef unsigned long os_alarm_t;

static __inline os_alarm_t _linux_kernel_set_alarm(unsigned long t)
{
	return (msecs_to_jiffies(t) + jiffies);
}

static __inline int _linux_kernel_test_alarm(os_alarm_t t)
{
	return (jiffies >= t) ? 1 : 0;
}

#define OS_SET_ALARM(t) _linux_kernel_set_alarm(t)
#define OS_TEST_ALARM(t) _linux_kernel_test_alarm(t)


/*----------------------------------------------------------------------------
 *  Function: OS_INIT_INTERRUPT
 *
 *  Description:
 *  This function establishes the communication for sending requests for interrupt
 *
 *  int OS_INIT_INTERRUPT( did, mmadr )
 *---------------------------------------------------------------------------*/
#ifdef CONFIG_INTERRUPT
#define OS_INIT_INTERRUPT(did, mmadr) _OS_INIT_INTERRUPT(did, mmadr)
#else
#define OS_INIT_INTERRUPT(did, mmadr) 0
#endif



/*----------------------------------------------------------------------------
 * Function: OS_SCHEDULE
 *
 * Description:
 *  This function can be called with the caller wished to give up processor
 *  control until the next available timeslice. This will allow other OS tasks
 *  to run before returning to the current context. This function should be
 *  used with caution and observation of reentrant coding principals.
 *
 *  void OS_SCHEDULE( void );
 *---------------------------------------------------------------------------*/

#define OS_SCHEDULE() {									\
	__set_current_state(TASK_INTERRUPTIBLE);					\
	schedule_timeout(1); }

#if 0
#define OS_DELAY(usec) {					\
    unsigned long j_timeout = usecs_to_jiffies(usec);		\
    unsigned long j_start = jiffies;				\
    unsigned long j_current;					\
    do {							\
      j_current = jiffies;					\
    }while((j_current - j_start) < j_timeout);			\
}
#else 
#define OS_DELAY(usec)	udelay(usec);
#endif

#if 0
// Implement _OS_SLEEP at a wait-loop
#define _OS_SLEEP(usec) {					\
	unsigned long j_timeout = usecs_to_jiffies(usec);		\
	unsigned long j_start = jiffies;				\
	unsigned long j_current;					\
	do {							\
		j_current = jiffies;					\
	}while((j_current - j_start) < j_timeout);			\
}
#else
// Implement _OS_SLEEP as a true sleep

/* We could also try TASK_UNINTERRUPTIBLE here */
/*----------------------------------------------------------------------------
 * Function: OS_SLEEP
 *
 * Parameters:
 *  time_val: Unsigned long time value in Micro-seconds (1/1000000 of a second)
 *            for the task to sleep.
 *
 * Description:
 *  This function causes the caller to delay further processing
 *  for the number of micro-seconds ( 1/1000000 or a second ) requested.
 *  This function should only be used with small time values
 *  ( < 1/100 of a second ) as lengthy sleeps could degrade the kernel
 *  response time.
 *
 *  void OS_SLEEP( unsigned long time_val );
 *---------------------------------------------------------------------------*/

#define OS_SLEEP(usec) {									\
	__set_current_state(TASK_KILLABLE);						\
	schedule_timeout(usecs_to_jiffies(usec)); }
#endif

/*----------------------------------------------------------------------------
 * Function: OS_UISLEEP
 *
 * Parameters:
 *  time_val: Unsigned long time value in Micro-seconds (1/1000000 of a second)
 *            for the task to sleep.
 *
 * Description:
 *  This function causes the caller to delay further processing
 *  for the number of micro-seconds ( 1/1000000 or a second ) requested.
 *  This sleep is uninterrruptable.
 *  This function should only be used with small time values
 *  ( < 1/100 of a second ) as lengthy sleeps could degrade the kernel
 *  response time.
 *
 *  void OS_UISLEEP( unsigned long time_val );
 *---------------------------------------------------------------------------*/

/* un interuuptable sleep */
#define OS_UISLEEP(usec) {					\
	__set_current_state(TASK_UNINTERRUPTIBLE);			\
	schedule_timeout(usecs_to_jiffies(usec)); 			\
}

#ifndef OS_PTHREAD_CREATE
typedef int os_pthread_t;
// This function is not being used.
// #define OS_PTHREAD_CREATE(t, a, f, p) (-1)
#endif

#ifndef OS_PTHREAD_JOIN
// This function is not being used.
// #define OS_PTHREAD_JOIN(t, p) (0)
#endif

/*
 * Note, the POSIX pthread implementation does not take the thread as a
 * parameter but it makes porting easier.
 */
#define UNREFERENCED_PARAMETER(parm) ((parm)=(parm))

#ifndef OS_PTHREAD_EXIT
// This function is not being used.
// #define OS_PTHREAD_EXIT(t, r)
#endif

#ifndef OS_PTHREAD_MUTEX_INIT
typedef struct mutex os_pthread_mutex_t;
#define OS_PTHREAD_MUTEX_INIT(m, a) mutex_init(m)
#endif

#ifndef OS_PTHREAD_MUTEX_LOCK
#define OS_PTHREAD_MUTEX_LOCK(m) mutex_lock_interruptible(m)
#endif

#ifndef OS_PTHREAD_MUTEX_UNLOCK
#define OS_PTHREAD_MUTEX_UNLOCK(m) mutex_unlock(m)
#endif

#endif

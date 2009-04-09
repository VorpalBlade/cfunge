/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2009 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at the proxy's option) any later version. Arvid Norlander is a
 * proxy who can decide which future versions of the GNU General Public
 * License can be used.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "HRTI.h"
#include "../../stack.h"

#include <stdint.h>

// This code tries to select the best timer:
//  1. clock_gettime() with CLOCK_MONOTONIC
//  2. clock_gettime() with CLOCK_REALTIME
//  3. gettimeofday()

#ifdef HAVE_clock_gettime
#  include <unistd.h> /* _POSIX_MONOTONIC_CLOCK */
#  include <time.h>
typedef struct timespec timetype;
typedef long res_type;

// Select monotonic or realtime:
#  if defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK > 0)
#    define TIMER_TYPE CLOCK_MONOTONIC
#  else
#    define TIMER_TYPE CLOCK_REALTIME
#  endif

#  define TIMERFUNC(x) clock_gettime(TIMER_TYPE, (x))

#  define SECONDS_MUTIPLIER 1000000000
#  define MICRO_TO_SMALL 1000

#  define MSEC(x) ((x) / 1000)
#  define MSEC_P(x) ((x)->tv_nsec / 1000)

#  define SMALL_P(x) ((x)->tv_nsec)

#  define ZERO_TIMETYPE(x) \
	{ \
		(x)->tv_sec = 0; \
		(x)->tv_nsec = 0; \
	}

#else
// Fallback on gettimeofday.
#  include <sys/time.h>
typedef struct timeval timetype;
typedef suseconds_t res_type;

#  define TIMERFUNC(x) gettimeofday((x), NULL)

#  define SECONDS_MUTIPLIER 1000000
#  define MICRO_TO_SMALL 1

#  define MSEC(x) (x)
#  define MSEC_P(x) ((x)->tv_usec)

#  define SMALL_P(x) ((x)->tv_usec)

#  define ZERO_TIMETYPE(x) \
	{ \
		(x)->tv_sec = 0; \
		(x)->tv_usec = 0; \
	}

#endif


/// The resolution.
static res_type resolution = 0;

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline funge_cell get_difference(const timetype * restrict before,
                                        const timetype * restrict after)
{
	return 1000000 * ((funge_cell)after->tv_sec - (funge_cell)before->tv_sec)
	       + MSEC((funge_cell)SMALL_P(after) - (funge_cell)SMALL_P(before));
}

/// This function checks that the IP have a non-null HRTI data pointer.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline bool check_ip_have_HRTI(instructionPointer * ip)
{
	if (!ip->fingerHRTItimestamp) {
		ip->fingerHRTItimestamp = cf_malloc_noptr(sizeof(timetype));
		if (FUNGE_UNLIKELY(!ip->fingerHRTItimestamp))
			return false;
		ZERO_TIMETYPE((timetype*)ip->fingerHRTItimestamp);
	}
	return true;
}

/// E - Erase Mark
static void finger_HRTI_erase_mark(instructionPointer * ip)
{
	if (!check_ip_have_HRTI(ip)) {
		ip_reverse(ip);
		return;
	}

	ZERO_TIMETYPE((timetype*)ip->fingerHRTItimestamp);
}

/// G - Granularity
static void finger_HRTI_granularity(instructionPointer * ip)
{
	stack_push(ip->stack, (funge_cell)resolution);
}

/// M - Mark
static void finger_HRTI_mark(instructionPointer * ip)
{
	if (!check_ip_have_HRTI(ip)) {
		ip_reverse(ip);
		return;
	}

	TIMERFUNC(ip->fingerHRTItimestamp);
}

/// T - Timer
static void finger_HRTI_timer(instructionPointer * ip)
{
	if (!ip->fingerHRTItimestamp || (((timetype*)ip->fingerHRTItimestamp)->tv_sec == 0)) {
		ip_reverse(ip);
	} else {
		timetype curTime;
		TIMERFUNC(&curTime);
		stack_push(ip->stack, get_difference(ip->fingerHRTItimestamp, &curTime));
	}
}

/// S - Second
static void finger_HRTI_second(instructionPointer * ip)
{
	timetype curTime;
	TIMERFUNC(&curTime);
	stack_push(ip->stack, (funge_cell)MSEC_P(&curTime));
}

FUNGE_ATTR_FAST static inline bool setup_HRTI(instructionPointer * ip)
{
	// This bit is global
	if (resolution == 0) {
		timetype before;
		timetype after;

		TIMERFUNC(&before);
		do {
			TIMERFUNC(&after);
			resolution = SECONDS_MUTIPLIER * (after.tv_sec - before.tv_sec) + SMALL_P(&after) - SMALL_P(&before);
		} while (resolution < MICRO_TO_SMALL);
	}
	resolution = MSEC(resolution);
	// Per IP, set up the data
	return check_ip_have_HRTI(ip);
}

bool finger_HRTI_load(instructionPointer * ip)
{
	if (FUNGE_UNLIKELY(!setup_HRTI(ip)))
		return false;
	manager_add_opcode(HRTI, 'E', erase_mark)
	manager_add_opcode(HRTI, 'G', granularity)
	manager_add_opcode(HRTI, 'M', mark)
	manager_add_opcode(HRTI, 'T', timer)
	manager_add_opcode(HRTI, 'S', second)

	return true;
}

/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
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
#include <sys/time.h>
#include <assert.h>

/// The resolution.
static suseconds_t resolution = 0;

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline fungeCell get_difference(const struct timeval * before,
                                       const struct timeval * after)
{
	return 1000000 * ((fungeCell)after->tv_sec - (fungeCell)before->tv_sec)
	       + (fungeCell)after->tv_usec - (fungeCell)before->tv_usec;
}

/// This function checks that the IP got a non-null HRTI data pointer.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline bool check_ip_got_HRTI(instructionPointer * ip)
{
	if (!ip->fingerHRTItimestamp) {
		ip->fingerHRTItimestamp = cf_malloc_noptr(sizeof(struct timeval));
		if (!ip->fingerHRTItimestamp)
			return false;
		ip->fingerHRTItimestamp->tv_sec = 0;
		ip->fingerHRTItimestamp->tv_usec = 0;
	}
	return true;
}

/// E - Erase Mark
static void finger_HRTI_erase_mark(instructionPointer * ip)
{
	if (!check_ip_got_HRTI(ip)) {
		ip_reverse(ip);
		return;
	}

	ip->fingerHRTItimestamp->tv_sec = 0;
	ip->fingerHRTItimestamp->tv_usec = 0;
}

/// G - Granularity
static void finger_HRTI_granularity(instructionPointer * ip)
{
	stack_push(ip->stack, (fungeCell)resolution);
}

/// M - Mark
static void finger_HRTI_mark(instructionPointer * ip)
{
	if (!check_ip_got_HRTI(ip)) {
		ip_reverse(ip);
		return;
	}

	gettimeofday(ip->fingerHRTItimestamp, NULL);
}

/// T - Timer
static void finger_HRTI_timer(instructionPointer * ip)
{
	if (!ip->fingerHRTItimestamp || (ip->fingerHRTItimestamp->tv_sec == 0)) {
		ip_reverse(ip);
	} else {
		struct timeval curTime;
		gettimeofday(&curTime, NULL);
		stack_push(ip->stack, get_difference(ip->fingerHRTItimestamp, &curTime));
	}
}

/// S - Second
static void finger_HRTI_second(instructionPointer * ip)
{
	struct timeval curTime;
	gettimeofday(&curTime, NULL);
	stack_push(ip->stack, (fungeCell)curTime.tv_usec);
}

FUNGE_ATTR_FAST static inline bool setup_HRTI(instructionPointer * ip)
{
	// This bit is global
	if (resolution == 0) {
		struct timeval before;
		struct timeval after;

		gettimeofday(&before, NULL);
		do {
			gettimeofday(&after, NULL);
			resolution = 1000000 * (after.tv_sec - before.tv_sec) + after.tv_usec - before.tv_usec;
		} while (resolution == 0);
	}
	// Per IP, set up the data
	return check_ip_got_HRTI(ip);
}

bool finger_HRTI_load(instructionPointer * ip)
{
	if (!setup_HRTI(ip))
		return false;
	manager_add_opcode(HRTI, 'E', erase_mark)
	manager_add_opcode(HRTI, 'G', granularity)
	manager_add_opcode(HRTI, 'M', mark)
	manager_add_opcode(HRTI, 'T', timer)
	manager_add_opcode(HRTI, 'S', second)

	return true;
}

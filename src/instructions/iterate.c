/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

#include "../global.h"
#include "iterate.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#ifdef AFL_FUZZ_TESTING
#  ifdef CONCURRENT_FUNGE
#    define RUNSELF() run_iterate_implement(ip, IPList, threadindex, true, maxRecursion-1)
#  else
#    define RUNSELF() run_iterate_implement(ip, true, maxRecursion-1)
#  endif
#else
#  ifdef CONCURRENT_FUNGE
#    define RUNSELF() run_iterate_implement(ip, IPList, threadindex, true)
#  else
#    define RUNSELF() run_iterate_implement(ip, true)
#  endif
#endif

#ifdef CONCURRENT_FUNGE
#  define RUNINSTR() execute_instruction(kInstr, ip, threadindex)
#else
#  define RUNINSTR() execute_instruction(kInstr, ip)
#endif

#ifndef DISABLE_TRACE
static inline void print_trace(funge_cell iters, funge_cell kInstr)
{
	if (FUNGE_UNLIKELY(setting_trace_level > 5))
		fprintf(stderr, "  * In k: iteration: %" FUNGECELLPRI " instruction: %c (%" FUNGECELLPRI ")\n",
		        iters, (char)kInstr, kInstr);
}
#endif

/// This moves IP to next instruction, with respect to ;, space and current delta.
static inline funge_cell find_next_instr(instructionPointer * restrict ip, funge_cell kInstr)
{
	bool injump = false;
	if (kInstr == ';')
		injump = true;
	while (true) {
		ip_forward(ip);
		kInstr = fungespace_get(&ip->position);
		if (kInstr == ';') {
			injump = !injump;
			continue;
		} else if (kInstr == ' ') {
			continue;
		} else {
			if (injump)
				continue;
			else
				break;
		}
	}
	return kInstr;
}

/**
 * Implements the k instruction, prototype differ depending on if
 * CONCURRENT_FUNGE is defined.
 * @param ip Instruction pointer to operate on.
 * @param IPList Pointer to IP list (only if CONCURRENT_FUNGE is defined).
 * @param threadindex What index in IPList the IP we operate on is.
 *                    (only if CONCURRENT_FUNGE is defined).
 * @param isRecursive Should be false, only set to true by k itself when iterating over another k.
 */
#ifdef AFL_FUZZ_TESTING

#  ifdef CONCURRENT_FUNGE
static FUNGE_ATTR_FAST void run_iterate_implement(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex, bool isRecursive, long maxRecursion)
#  else
static FUNGE_ATTR_FAST void run_iterate_implement(instructionPointer * restrict ip, bool isRecursive, long maxRecursion)
#  endif

#else

#  ifdef CONCURRENT_FUNGE
static FUNGE_ATTR_FAST void run_iterate_implement(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex, bool isRecursive)
#  else
static FUNGE_ATTR_FAST void run_iterate_implement(instructionPointer * restrict ip, bool isRecursive)
#  endif

#endif
{
	funge_cell iters = stack_pop(ip->stack);
	#ifdef AFL_FUZZ_TESTING
	if (maxRecursion == 0)
		exit(123);
	if (iters > 600)
		iters = 600;
	#endif
	if (iters == 0) {
		funge_cell kInstr;
		// Skip past next instruction.
		ip_forward(ip);
		kInstr = fungespace_get(&ip->position);
		if (kInstr == ' ' || kInstr == ';') {
			find_next_instr(ip, kInstr);
		}
	} else if (iters < 0) {
		ip_reverse(ip);
	} else {
		funge_cell kInstr;
		// Note that:
		//   * Instruction executes *at* k
		//   * In Funge-109 we skip over the cell we executed
		//     (if position or delta didn't change).
		//   * In Funge-98 we don't do that.

		// This is used in case of spaces and with Funge-109
		funge_vector oldpos = ip->position;
		// And this is for knowing where to move past (in 109)
		funge_vector posinstr;
		// Fetch instruction
		ip_forward(ip);
		kInstr = fungespace_get(&ip->position);

		// We should reach past any spaces and ;; pairs and execute first
		// instruction we find. This is unclear/undef in 98 but defined in 109.
		if (kInstr == ' ' || kInstr == ';') {
			kInstr = find_next_instr(ip, kInstr);
		}

		// First store pos where we got to restore to to "move past" instruction in Funge-109.
		posinstr = ip->position;
		// Then go back and execute it at k...
		ip->position = oldpos;

		// We special case some stuff here that breaks otherwise.
		switch (kInstr) {
			case 'z':
				return;
			case '@':
				// Iterating over @ is insane, to avoid issues when doing
				// concurrent execution lets just kill current IP.
				// In other words, execute this once.
				RUNINSTR();
				break;
			default: {
				// Ok we got to execute it!
				// Storing second part of the current IP state (for Funge-109)
				funge_vector olddelta = ip->delta;

				// This horrible kludge is needed because iplist_duplicate_ip
				// calls realloc so IP pointer may end up invalid. A horrible
				// hack yes.
#ifdef CONCURRENT_FUNGE
				ssize_t oldindex = *threadindex;
#endif
				while (iters--) {
#ifndef DISABLE_TRACE
					print_trace(iters, kInstr);
#endif /* DISABLE_TRACE */

					switch (kInstr) {
#ifdef CONCURRENT_FUNGE
						case 't': {
							ssize_t new_index = iplist_duplicate_ip(IPList, *threadindex);
							if (new_index != -1) {
								*threadindex = new_index;
							} else {
								// Yeah this is the same as the child normally,
								// the program should check that the parent still exists.
								ip_reverse(ip);
							}
							break;
						}
#endif
						case 'k':
							// I HATE this one...
							ip->position = posinstr;
							RUNSELF();
							// Cludge for realloc again...
#if defined(CONCURRENT_FUNGE) && defined(LARGE_IPLIST)
							ip = (*IPList)->ips[oldindex];

#elif defined(CONCURRENT_FUNGE)
							ip = &((*IPList)->ips[oldindex]);
#endif
							// Check position here.
							if (posinstr.x == ip->position.x
							    && posinstr.y == ip->position.y)
								ip->position = oldpos;
							break;
						default:
							RUNINSTR();
							break;
					}
				}
#if defined(CONCURRENT_FUNGE) && defined(LARGE_IPLIST)
				if (kInstr == 't')
					ip = (*IPList)->ips[oldindex];
#elif defined(CONCURRENT_FUNGE)
				if (kInstr == 't')
					ip = &((*IPList)->ips[oldindex]);
#endif
				// If delta and ip did not change, move forward in Funge-109.
				// ...unless we are recursive, to ensure correct behaviour...
				if (setting_current_standard == stdver109 && !isRecursive) {
					if (olddelta.x == ip->delta.x
					    && olddelta.y == ip->delta.y
					    && oldpos.x == ip->position.x
					    && oldpos.y == ip->position.y)
						ip->position = posinstr;
				}
				break;
			}
		}
	}
}

#ifdef AFL_FUZZ_TESTING
#  ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex)
{
	run_iterate_implement(ip, IPList, threadindex, false, 5);
}
#  else
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip)
{
	run_iterate_implement(ip, false, 5);
}
#  endif
#else
#  ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex)
{
	run_iterate_implement(ip, IPList, threadindex, false);
}
#  else
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip)
{
	run_iterate_implement(ip, false);
}
#  endif
#endif

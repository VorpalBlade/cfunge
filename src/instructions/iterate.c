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

#include "../global.h"
#include "iterate.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#ifdef CONCURRENT_FUNGE
#  define RUNSELF() run_iterate(ip, IPList, threadindex, true)
#  define RUNINSTR() execute_instruction(kInstr, ip, threadindex)
#else
#  define RUNSELF() run_iterate(ip, true)
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

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex, bool isRecursive)
#else
FUNGE_ATTR_FAST void run_iterate(instructionPointer * restrict ip, bool isRecursive)
#endif
{
	funge_cell iters = stack_pop(ip->stack);
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
						case 't':
							*threadindex = iplist_duplicate_ip(IPList, *threadindex);
							break;
#endif
						case 'k':
							// I HATE this one...
							ip->position = posinstr;
							RUNSELF();
							// Cludge for realloc again...
#ifdef CONCURRENT_FUNGE
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
#ifdef CONCURRENT_FUNGE
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

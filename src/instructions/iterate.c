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

#include "../global.h"
#include "iterate.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#ifdef CONCURRENT_FUNGE
FUNGE_FAST void RunIterate(instructionPointer * restrict ip, ipList ** IPList, ssize_t * restrict threadindex)
#else
FUNGE_FAST void RunIterate(instructionPointer * restrict ip)
#endif
{
	FUNGEDATATYPE iters = StackPop(ip->stack);
	if (iters == 0) {
		ipForward(ip, 1);
	} else if (iters < 0) {
		ipReverse(ip);
	} else {
		FUNGEDATATYPE kInstr;
		// The weird stuff below, is, as described by CCBI:
		//   Instruction executes *at* k
		//   If the instruction k executes, changes delta or position, we are finished.
		//   If it doesn't we should jump to *after* the instruction k executed.
		// Delta is stored below, we only need pos at this point as we don't
		// know if we will execute anything yet, pointless storing it at this
		// point then.
		// It is also used in case of spaces
		fungePosition oldpos = ip->position;
		// And this is for knowing where to move past
		fungePosition posinstr;
		// Fetch instruction
		ipForward(ip, 1);
		kInstr = FungeSpaceGet(&ip->position);

		// We should reach past any spaces and execute first instruction we find
		// This is undef in 98 but defined in 108.
		if (kInstr == ' ') {
			do {
				ipForward(ip, 1);
			} while ((kInstr = FungeSpaceGet(&ip->position)) == ' ');
		}
		// First store pos where we got to restore to to "move past" instruction.
		posinstr = ip->position;
		// Then go back and execute it at k...
		ip->position = oldpos;

		switch (kInstr) {
			case 'z':
				return;
			case 'k':
			case ';':
				if (SettingWarnings)
				fprintf(stderr, "WARN: k at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI " cannot execute: %c (%" FUNGEDATAPRI ")\n",
				        ip->position.x, ip->position.y, (char)kInstr, kInstr);
				ipReverse(ip);
				break;
			case '@':
				// Iterating over @ is insane, to avoid issues when doing concurrent execution lets just kill current IP.
#ifdef CONCURRENT_FUNGE
				ExecuteInstruction(kInstr, ip, threadindex);
#else
				ExecuteInstruction(kInstr, ip);
#endif
				break;
			default: {
				// Ok we got to execute it!
				// Storing second part of the current IP state (why: see above)
				ipDelta olddelta = ip->delta;

				// This horrible kludge is needed because ipListDuplicateIP calls realloc
				// so IP pointer may end up invalid. A horrible hack yes.
#ifdef CONCURRENT_FUNGE
				ssize_t oldindex = *threadindex;
#endif
				while (iters--) {
#    ifndef DISABLE_TRACE
					if (SettingTraceLevel > 5)
						fprintf(stderr, "  * In k: iteration: %" FUNGEDATAPRI " instruction: %c (%" FUNGEDATAPRI ")\n",
						        iters, (char)kInstr, kInstr);
#    endif /* DISABLE_TRACE */
#ifdef CONCURRENT_FUNGE
					if (kInstr == 't')
						*threadindex = ipListDuplicateIP(IPList, *threadindex);
					else
						ExecuteInstruction(kInstr, ip, threadindex);
#else
					ExecuteInstruction(kInstr, ip);
#endif
				}
#ifdef CONCURRENT_FUNGE
				if (kInstr == 't')
					ip = &((*IPList)->ips[oldindex]);
#endif
				// If delta and ip did not change, move forward (why: see above)
				if (olddelta.x == ip->delta.x
					&& olddelta.y == ip->delta.y
					&& oldpos.x == ip->position.x
					&& oldpos.y == ip->position.y)
					ip->position = posinstr;
				break;
			}
		}
	}
}

/*
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


void RunIterate(instructionPointer * ip)
{
	FUNGEDATATYPE iters = StackPop(ip->stack);
	if (iters == 0) {
		ipForward(1, ip);
	} else if (iters < 0) {
		ipReverse(ip);
	} else {
		FUNGEDATATYPE kInstr;
		// Fetch instruction
		ipForward(1, ip);
		kInstr = fungeSpaceGet(&ip->position);
		ipForward(-1, ip);

		if (kInstr == ' ' || kInstr == 'z')
			return;
		else if ((kInstr == 'k') || (kInstr == ';')) {
			if (SettingWarnings)
				fprintf(stderr, "WARN: k at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI " cannot execute: %c (%" FUNGEDATAPRI ")\n", ip->position.x, ip->position.y, (char)kInstr, kInstr);
			ipReverse(ip);
		} else {
			// Ok we got to execute it!
			// The weird stuff below, is, as described by CCBI:
			// Instruction executes *at* k
			// If the instruction k executes, changes delta or position, we are finished.
			// If it doesn't we should jump to *after* the instruction k executed.
			ipDelta olddelta = ip->delta;
			fungePosition oldpos = ip->position;

			while (iters--)
				ExecuteInstruction(kInstr, ip);
			if (olddelta.x == ip->delta.x
			    && olddelta.y == ip->delta.y
			    && oldpos.x == ip->position.x
			    && oldpos.y == ip->position.y)
				ipForward(1, ip);
		}
	}
}

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

#include "MODU.h"
#include "../../stack.h"

// This was more or less taken from CCBI

static inline FUNGEDATATYPE floordiv(FUNGEDATATYPE x, FUNGEDATATYPE y) __attribute__((const));

static inline FUNGEDATATYPE floordiv(FUNGEDATATYPE x, FUNGEDATATYPE y)
{
	x /= y;
	if (x < 0)
		return x - 1;
	else
		return x;
}

static void FingerMODUSignedResult(instructionPointer * ip) {
	FUNGEDATATYPE x, y;
	y = StackPop(ip->stack);
	x = StackPop(ip->stack);
	if (y) {
		StackPush(x - floordiv(x, y) * y, ip->stack);
	} else {
		StackPush(0, ip->stack);
	}
}

static void FingerMODUUnsignedResult(instructionPointer * ip) {
	FUNGEDATATYPE x, y;
	y = StackPop(ip->stack);
	x = StackPop(ip->stack);
	if (y) {
		FUNGEDATATYPE r = x % y;
		if (r < 0)
			r = ABS(r);
		StackPush(r, ip->stack);
	} else {
		StackPush(0, ip->stack);
	}
}

// C style reminder.
static void FingerMODURemainder(instructionPointer * ip) {
	FUNGEDATATYPE x, y;
	y = StackPop(ip->stack);
	x = StackPop(ip->stack);

	if (y) {
		// Well that's easy, this is C.
		StackPush(x % y, ip->stack);
	} else {
		StackPush(0, ip->stack);
	}
}


bool FingerMODUload(instructionPointer * ip) {
	if (!OpcodeStackAdd(ip, 'M', &FingerMODUSignedResult))
		return false;
	if (!OpcodeStackAdd(ip, 'U', &FingerMODUUnsignedResult))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerMODURemainder))
		return false;
	return true;
}

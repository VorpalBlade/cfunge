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

#include "SUBR.h"
#include "../../stack.h"
#include "../../vector.h"

#include <stdio.h>
#include <assert.h>

#define ALLOCCHUNK 8

static const ipDelta SUBRnewDelta = { .x = 1, .y = 0 };

static void FingerSUBRCall(instructionPointer * ip) {
	FUNGEDATATYPE n;
	fungePosition pos;
	fungeStack *tmpstack;

	n = StackPop(ip->stack);
	pos = StackPopVector(ip->stack);

	tmpstack = StackCreate();

	for (FUNGEDATATYPE i = 0; i < n; ++i)
		StackPush(StackPop(ip->stack), tmpstack);

	StackPushVector(&ip->position, ip->stack);
	StackPushVector(&ip->delta, ip->stack);
	while (n--)
		StackPush(StackPop(tmpstack), ip->stack);
	StackFree(tmpstack);

	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &SUBRnewDelta);
	ip->NeedMove = false;
}

static void FingerSUBRJump(instructionPointer * ip) {
	fungePosition pos;

	pos = StackPopVector(ip->stack);
	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &SUBRnewDelta);
	ip->NeedMove = false;
}

static void FingerSUBRReturn(instructionPointer * ip) {
	FUNGEDATATYPE n;
	fungePosition pos;
	fungeVector vec;
	fungeStack *tmpstack;

	n = StackPop(ip->stack);

	tmpstack = StackCreate();

	for (FUNGEDATATYPE i = 0; i < n; ++i)
		StackPush(StackPop(ip->stack), tmpstack);

	vec = StackPopVector(ip->stack);
	pos = StackPopVector(ip->stack);
	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &vec);

	while (n--)
		StackPush(StackPop(tmpstack), ip->stack);

	StackFree(tmpstack);
}


bool FingerSUBRload(instructionPointer * ip) {
	if (!OpcodeStackAdd(ip, 'C', &FingerSUBRCall))
		return false;
	if (!OpcodeStackAdd(ip, 'J', &FingerSUBRJump))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerSUBRReturn))
		return false;
	return true;
}

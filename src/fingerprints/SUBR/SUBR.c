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

#include "SUBR.h"
#include "../../stack.h"
#include "../../vector.h"

#include <stdio.h>
#include <assert.h>

#define ALLOCCHUNK 8

static const ipDelta SUBRnewDelta = { .x = 1, .y = 0 };

/// A - Change to absolute addressing
static void FingerSUBRabsolute(instructionPointer * ip)
{
	ip->fingerSUBRisRelative = false;
}

/// C - Call
static void FingerSUBRcall(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	fungePosition pos;
	fungeStack *tmpstack;

	n = StackPop(ip->stack);
	// Pop vector
	pos = StackPopVector(ip->stack);
	// Stupid to change a fingerprint after it is published.
	if (ip->fingerSUBRisRelative) {
		pos.x += ip->storageOffset.x;
		pos.y += ip->storageOffset.y;
	}

	tmpstack = StackCreate();

	for (FUNGEDATATYPE i = 0; i < n; ++i)
		StackPush(tmpstack, StackPop(ip->stack));

	StackPushVector(ip->stack, &ip->position);
	StackPushVector(ip->stack, &ip->delta);
	while (n--)
		StackPush(ip->stack, StackPop(tmpstack));
	StackFree(tmpstack);

	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &SUBRnewDelta);
	ip->needMove = false;
}

/// J - Jump
static void FingerSUBRjump(instructionPointer * ip)
{
	fungePosition pos;

	pos = StackPopVector(ip->stack);
	// Stupid to change a fingerprint after it is published.
	if (ip->fingerSUBRisRelative) {
		pos.x += ip->storageOffset.x;
		pos.y += ip->storageOffset.y;
	}

	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &SUBRnewDelta);
}

/// O - Change to relative addressing
static void FingerSUBRrelative(instructionPointer * ip)
{
	ip->fingerSUBRisRelative = true;
}

/// R - Return from call
static void FingerSUBRreturn(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	fungePosition pos;
	fungeVector vec;
	fungeStack *tmpstack;

	n = StackPop(ip->stack);

	tmpstack = StackCreate();

	for (FUNGEDATATYPE i = 0; i < n; ++i)
		StackPush(tmpstack, StackPop(ip->stack));

	vec = StackPopVector(ip->stack);
	pos = StackPopVector(ip->stack);
	ipSetPosition(ip, &pos);
	ipSetDelta(ip, &vec);

	while (n--)
		StackPush(ip->stack, StackPop(tmpstack));

	StackFree(tmpstack);
}


bool FingerSUBRload(instructionPointer * ip)
{
	ManagerAddOpcode(SUBR,  'A', absolute)
	ManagerAddOpcode(SUBR,  'C', call)
	ManagerAddOpcode(SUBR,  'J', jump)
	ManagerAddOpcode(SUBR,  'O', relative)
	// No not a keyword in this case
	ManagerAddOpcode(SUBR,  'R', return)
	return true;
}

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

#include "JSTR.h"
#include "../../stack.h"

/// G - Read with delta
static void FingerJSTRpushN(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	fungeVector pos, delta;

	n     = StackPop(ip->stack);
	pos   = StackPopVector(ip->stack);
	delta = StackPopVector(ip->stack);

	if (n <= 0) {
		ipReverse(ip);
		return;
	}

	StackPush(ip->stack, 0);

	while (n--) {
		StackPush(ip->stack, FungeSpaceGet(&pos));
		pos.x += delta.x;
		pos.y += delta.y;
	}
}

/// P - Write with delta
static void FingerJSTRpopN(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	fungeVector pos, delta;

	n     = StackPop(ip->stack);
	pos   = StackPopVector(ip->stack);
	delta = StackPopVector(ip->stack);

	if (n <= 0) {
		ipReverse(ip);
		return;
	}

	while (n--) {
		FungeSpaceSet(StackPop(ip->stack), &pos);
		pos.x += delta.x;
		pos.y += delta.y;
	}
}

bool FingerJSTRload(instructionPointer * ip)
{
	ManagerAddOpcode(JSTR,  'G', pushN)
	ManagerAddOpcode(JSTR,  'P', popN)
	return true;
}

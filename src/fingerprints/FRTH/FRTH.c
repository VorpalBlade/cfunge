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

#include "FRTH.h"
#include "../../stack.h"

// This was partly based on how CCBI does it.

/// D - Push depth of stack to tos
static void FingerFRTHstackSize(instructionPointer * ip)
{
	StackPush(ip->stack, (fungeCell)ip->stack->top);
}

/// L - Forth Roll command
static void FingerFRTHforthRoll(instructionPointer * ip)
{
	// FIXME: Move most of this functionality into stack.c
	fungeCell u;
	size_t s;
	u = StackPop(ip->stack);
	s = ip->stack->top;

	if (u >= (fungeCell)s) {
		StackPush(ip->stack, 0);
	} else {
		fungeCell * elems;
		fungeCell xu;

		elems = cf_malloc_noptr(sizeof(fungeCell) * ip->stack->top);
		if (!elems) {
			ipReverse(ip);
			return;
		}
		memcpy(elems, ip->stack->entries, sizeof(fungeCell) * ip->stack->top);
		xu = elems[s - (u+1)];

		StackPopNDiscard(ip->stack, u + 1);
		for (size_t i = s - u; i < s; i++) {
			StackPush(ip->stack, elems[i]);
		}
		StackPush(ip->stack, xu);
		cf_free(elems);
	}
}

/// O - Forth Over command
static void FingerFRTHforthOver(instructionPointer * ip)
{
	fungeCell a, b;
	b = StackPop(ip->stack);
	a = StackPop(ip->stack);

	StackPush(ip->stack, a);
	StackPush(ip->stack, b);
	StackPush(ip->stack, a);
}

/// P - Forth Pick command
static void FingerFRTHforthPick(instructionPointer * ip)
{
	fungeCell u;
	fungeCell s;
	u = StackPop(ip->stack);
	s = (fungeCell)ip->stack->top;

	if (u >= s) {
		StackPush(ip->stack, 0);
	} else {
		fungeCell i = StackGetIndex(ip->stack, s - u);
		StackPush(ip->stack, i);
	}
}

/// R - Forth Rot command
static void FingerFRTHforthRot(instructionPointer * ip)
{
	fungeCell a, b, c;
	c = StackPop(ip->stack);
	b = StackPop(ip->stack);
	a = StackPop(ip->stack);

	StackPush(ip->stack, b);
	StackPush(ip->stack, c);
	StackPush(ip->stack, a);
}

bool FingerFRTHload(instructionPointer * ip)
{
	ManagerAddOpcode(FRTH,  'D', stackSize)
	ManagerAddOpcode(FRTH,  'L', forthRoll)
	ManagerAddOpcode(FRTH,  'O', forthOver)
	ManagerAddOpcode(FRTH,  'P', forthPick)
	ManagerAddOpcode(FRTH,  'R', forthRot)
	return true;
}

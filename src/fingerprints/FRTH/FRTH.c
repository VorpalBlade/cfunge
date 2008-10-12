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
	stack_push(ip->stack, (fungeCell)ip->stack->top);
}

/// L - Forth Roll command
static void FingerFRTHforthRoll(instructionPointer * ip)
{
	// FIXME: Move most of this functionality into stack.c
	fungeCell u;
	size_t s;
	u = stack_pop(ip->stack);
	s = ip->stack->top;

	if (u >= (fungeCell)s) {
		stack_push(ip->stack, 0);
	} else {
		fungeCell * elems;
		fungeCell xu;

		elems = cf_malloc_noptr(sizeof(fungeCell) * ip->stack->top);
		if (!elems) {
			ip_reverse(ip);
			return;
		}
		memcpy(elems, ip->stack->entries, sizeof(fungeCell) * ip->stack->top);
		xu = elems[s - (u+1)];

		stack_pop_n_discard(ip->stack, u + 1);
		for (size_t i = s - u; i < s; i++) {
			stack_push(ip->stack, elems[i]);
		}
		stack_push(ip->stack, xu);
		cf_free(elems);
	}
}

/// O - Forth Over command
static void FingerFRTHforthOver(instructionPointer * ip)
{
	fungeCell a, b;
	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	stack_push(ip->stack, a);
	stack_push(ip->stack, b);
	stack_push(ip->stack, a);
}

/// P - Forth Pick command
static void FingerFRTHforthPick(instructionPointer * ip)
{
	fungeCell u;
	fungeCell s;
	u = stack_pop(ip->stack);
	s = (fungeCell)ip->stack->top;

	if (u >= s) {
		stack_push(ip->stack, 0);
	} else {
		fungeCell i = stack_get_index(ip->stack, s - u);
		stack_push(ip->stack, i);
	}
}

/// R - Forth Rot command
static void FingerFRTHforthRot(instructionPointer * ip)
{
	fungeCell a, b, c;
	c = stack_pop(ip->stack);
	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	stack_push(ip->stack, b);
	stack_push(ip->stack, c);
	stack_push(ip->stack, a);
}

bool FingerFRTHload(instructionPointer * ip)
{
	manager_add_opcode(FRTH,  'D', stackSize)
	manager_add_opcode(FRTH,  'L', forthRoll)
	manager_add_opcode(FRTH,  'O', forthOver)
	manager_add_opcode(FRTH,  'P', forthPick)
	manager_add_opcode(FRTH,  'R', forthRot)
	return true;
}

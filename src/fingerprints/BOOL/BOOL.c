/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2010 Arvid Norlander <anmaster AT tele2 DOT se>
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

#include "BOOL.h"
#include "../../stack.h"

/// A - and
static void finger_BOOL_and(instructionPointer * ip)
{
	funge_cell a = stack_pop(ip->stack);
	funge_cell b = stack_pop(ip->stack);
	stack_push(ip->stack, a & b);
}

/// N - not
static void finger_BOOL_not(instructionPointer * ip)
{
	funge_cell a = stack_pop(ip->stack);
	stack_push(ip->stack, ~a);
}

/// O - or
static void finger_BOOL_or(instructionPointer * ip)
{
	funge_cell a = stack_pop(ip->stack);
	funge_cell b = stack_pop(ip->stack);
	stack_push(ip->stack, a | b);
}

/// X - xor
static void finger_BOOL_xor(instructionPointer * ip)
{
	funge_cell a = stack_pop(ip->stack);
	funge_cell b = stack_pop(ip->stack);
	stack_push(ip->stack, a ^ b);
}

bool finger_BOOL_load(instructionPointer * ip)
{
	manager_add_opcode(BOOL, 'A', and)
	manager_add_opcode(BOOL, 'N', not)
	manager_add_opcode(BOOL, 'O', or)
	manager_add_opcode(BOOL, 'X', xor)
	return true;
}

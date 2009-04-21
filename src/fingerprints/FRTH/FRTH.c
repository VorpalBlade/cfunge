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

#include "FRTH.h"
#include "../../stack.h"

#include <string.h> /* memcpy */

// This was partly based on how CCBI does it.

/// D - Push depth of stack to tos
static void finger_FRTH_stack_size(instructionPointer * ip)
{
	stack_push(ip->stack, (funge_cell)ip->stack->top);
}

/// L - Forth Roll command
static void finger_FRTH_forth_roll(instructionPointer * ip)
{
	// FIXME: Move most of this functionality into stack.c
	funge_cell u;
	size_t s;
	u = stack_pop(ip->stack);
	if (u < 0) {
		ip_reverse(ip);
		return;
	}
	s = ip->stack->top;

	if (u >= (funge_cell)s) {
		stack_push(ip->stack, 0);
	} else {
		funge_cell * restrict elems;
		funge_cell xu;

		elems = cf_malloc_noptr(sizeof(funge_cell) * ip->stack->top);
		if (!elems) {
			ip_reverse(ip);
			return;
		}
		memcpy(elems, ip->stack->entries, sizeof(funge_cell) * ip->stack->top);
		xu = elems[s - (u+1)];

		stack_discard(ip->stack, (size_t)(u + 1));
		for (size_t i = s - (size_t)u; i < s; i++) {
			stack_push(ip->stack, elems[i]);
		}
		stack_push(ip->stack, xu);
		cf_free(elems);
	}
}

/// O - Forth Over command
static void finger_FRTH_forth_over(instructionPointer * ip)
{
	funge_cell a, b;
	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	stack_push(ip->stack, a);
	stack_push(ip->stack, b);
	stack_push(ip->stack, a);
}

/// P - Forth Pick command
static void finger_FRTH_forth_pick(instructionPointer * ip)
{
	funge_cell u;
	funge_cell s;
	u = stack_pop(ip->stack);
	s = (funge_cell)ip->stack->top;

	if (u < 0) {
		ip_reverse(ip);
		return;
	}

	if (u >= s) {
		stack_push(ip->stack, 0);
	} else {
		funge_cell i = stack_get_index(ip->stack, (size_t)(s - u));
		stack_push(ip->stack, i);
	}
}

/// R - Forth Rot command
static void finger_FRTH_forth_rot(instructionPointer * ip)
{
	funge_cell a, b, c;
	c = stack_pop(ip->stack);
	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	stack_push(ip->stack, b);
	stack_push(ip->stack, c);
	stack_push(ip->stack, a);
}

bool finger_FRTH_load(instructionPointer * ip)
{
	manager_add_opcode(FRTH, 'D', stack_size)
	manager_add_opcode(FRTH, 'L', forth_roll)
	manager_add_opcode(FRTH, 'O', forth_over)
	manager_add_opcode(FRTH, 'P', forth_pick)
	manager_add_opcode(FRTH, 'R', forth_rot)
	return true;
}

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

#include "ORTH.h"
#include "../../rect.h"
#include "../../stack.h"
#include "../../interpreter.h"
#include "../../funge-space/funge-space.h"

#include <stdio.h> /* fputs */


static void finger_ORTH_bit_and(instructionPointer * ip)
{
	funge_cell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x & y);
}

static void finger_ORTH_bit_or(instructionPointer * ip)
{
	funge_cell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x | y);
}

static void finger_ORTH_bit_xor(instructionPointer * ip)
{
	funge_cell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x ^ y);
}

// ortho get
static void finger_ORTH_get(instructionPointer * ip)
{
	funge_vector v = stack_pop_vector(ip->stack);
	stack_push(ip->stack, fungespace_get(vector_create_ref(v.y, v.x)));
}

// ortho put
static void finger_ORTH_put(instructionPointer * ip)
{
	funge_vector v;
	funge_cell c;

	v = stack_pop_vector(ip->stack);
	c = stack_pop(ip->stack);

	fungespace_set(c, vector_create_ref(v.y, v.x));
}

// output string
static void finger_ORTH_output_string(instructionPointer * ip)
{
	char * restrict str = (char*)stack_pop_string(ip->stack, NULL);
	// puts add newline, we therefore do fputs on stdout
	fputs(str, stdout);
	stack_free_string(str);
}

// change dx
static void finger_ORTH_change_dx(instructionPointer * ip)
{
	ip->delta.x = stack_pop(ip->stack);
}

// change dy
static void finger_ORTH_change_dy(instructionPointer * ip)
{
	ip->delta.y = stack_pop(ip->stack);
}

// change x
static void finger_ORTH_change_x(instructionPointer * ip)
{
	ip_set_position(ip, vector_create_ref(stack_pop(ip->stack), ip->position.y));
}

// change y
static void finger_ORTH_change_y(instructionPointer * ip)
{
	ip_set_position(ip, vector_create_ref(ip->position.x, stack_pop(ip->stack)));
}

// ramp if zero
static void finger_ORTH_ramp_if_zero(instructionPointer * ip)
{
	if (!stack_pop(ip->stack))
		ip_forward(ip);
}

bool finger_ORTH_load(instructionPointer * ip)
{
	manager_add_opcode(ORTH, 'A', bit_and)
	manager_add_opcode(ORTH, 'O', bit_or)
	manager_add_opcode(ORTH, 'E', bit_xor)
	manager_add_opcode(ORTH, 'G', get)
	manager_add_opcode(ORTH, 'P', put)
	manager_add_opcode(ORTH, 'S', output_string)
	manager_add_opcode(ORTH, 'V', change_dx)
	manager_add_opcode(ORTH, 'W', change_dy)
	manager_add_opcode(ORTH, 'X', change_x)
	manager_add_opcode(ORTH, 'Y', change_y)
	manager_add_opcode(ORTH, 'Z', ramp_if_zero)
	return true;
}

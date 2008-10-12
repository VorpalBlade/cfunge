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

#include "ORTH.h"
#include "../../rect.h"
#include "../../stack.h"
#include "../../interpreter.h"
#include "../../funge-space/funge-space.h"

#include <stdio.h>


static void FingerORTHbitAnd(instructionPointer * ip)
{
	fungeCell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x & y);
}

static void FingerORTHbitOr(instructionPointer * ip)
{
	fungeCell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x | y);
}

static void FingerORTHbitXor(instructionPointer * ip)
{
	fungeCell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	stack_push(ip->stack, x ^ y);
}

// ortho get
static void FingerORTHget(instructionPointer * ip)
{
	fungeVector v = stack_pop_vector(ip->stack);
	stack_push(ip->stack, fungespace_get(vector_create_ref(v.y, v.x)));
}

// ortho put
static void FingerORTHput(instructionPointer * ip)
{
	fungeVector v;
	fungeCell c;

	v = stack_pop_vector(ip->stack);
	c = stack_pop(ip->stack);

	fungespace_set(c, vector_create_ref(v.y, v.x));
}

// output string
static void FingerORTHoutputString(instructionPointer * ip)
{
	char * restrict str = stack_pop_string(ip->stack);
	// puts add newline, we therefore do fputs on stdout
	fputs(str, stdout);
	stack_freeString(str);
}

// change dx
static void FingerORTHchangeDx(instructionPointer * ip)
{
	ip->delta.x = stack_pop(ip->stack);
}

// change dy
static void FingerORTHchangeDy(instructionPointer * ip)
{
	ip->delta.y = stack_pop(ip->stack);
}

// change x
static void FingerORTHchangeX(instructionPointer * ip)
{
	ip_set_position(ip, vector_create_ref(stack_pop(ip->stack), ip->position.y));
}

// change y
static void FingerORTHchangeY(instructionPointer * ip)
{
	ip_set_position(ip, vector_create_ref(ip->position.x, stack_pop(ip->stack)));
}

// ramp if zero
static void FingerORTHrampIfZero(instructionPointer * ip)
{
	if (!stack_pop(ip->stack))
		ip_forward(ip, 1);
}

bool FingerORTHload(instructionPointer * ip)
{
	manager_add_opcode(ORTH,  'A', bitAnd)
	manager_add_opcode(ORTH,  'O', bitOr)
	manager_add_opcode(ORTH,  'E', bitXor)
	manager_add_opcode(ORTH,  'G', get)
	manager_add_opcode(ORTH,  'P', put)
	manager_add_opcode(ORTH,  'S', outputString)
	manager_add_opcode(ORTH,  'V', changeDx)
	manager_add_opcode(ORTH,  'W', changeDy)
	manager_add_opcode(ORTH,  'X', changeX)
	manager_add_opcode(ORTH,  'Y', changeY)
	manager_add_opcode(ORTH,  'Z', rampIfZero)
	return true;
}

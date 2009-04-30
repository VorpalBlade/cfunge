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

#include "CPLI.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"

#include <math.h>

#ifndef HAVE_sqrtl
#  define sqrtl sqrt
#endif

/// A - add
static void finger_CPLI_add(instructionPointer * ip)
{
	funge_cell ar, ai, br, bi;
	bi = stack_pop(ip->stack);
	br = stack_pop(ip->stack);
	ai = stack_pop(ip->stack);
	ar = stack_pop(ip->stack);
	stack_push(ip->stack, ar + br);
	stack_push(ip->stack, ai + bi);
}

/// D - div
static void finger_CPLI_div(instructionPointer * ip)
{
	funge_cell ar, ai, br, bi, denom;
	bi = stack_pop(ip->stack);
	br = stack_pop(ip->stack);
	ai = stack_pop(ip->stack);
	ar = stack_pop(ip->stack);
	denom = bi * bi + br * br;
	if (denom != 0) {
		stack_push(ip->stack, (ai*bi + ar*br) / denom);
		stack_push(ip->stack, (ai*br - ar*bi) / denom);
	} else {
		stack_push(ip->stack, 0);
		stack_push(ip->stack, 0);
	}
}

/// M - mul
static void finger_CPLI_mul(instructionPointer * ip)
{
	funge_cell ar, ai, br, bi;
	bi = stack_pop(ip->stack);
	br = stack_pop(ip->stack);
	ai = stack_pop(ip->stack);
	ar = stack_pop(ip->stack);
	stack_push(ip->stack, ar*br - ai*bi);
	stack_push(ip->stack, ar*bi + ai*br);
}

/// O - out
static void finger_CPLI_out(instructionPointer * ip)
{
	funge_cell r, i;
	i = stack_pop(ip->stack);
	r = stack_pop(ip->stack);
	printf("%" FUNGECELLPRI, r);
	if (i > 0)
		cf_putchar_maybe_locked('+');
	printf("%" FUNGECELLPRI "i ", i);
}

/// S - sub
static void finger_CPLI_sub(instructionPointer * ip)
{
	funge_cell ar, ai, br, bi;
	bi = stack_pop(ip->stack);
	br = stack_pop(ip->stack);
	ai = stack_pop(ip->stack);
	ar = stack_pop(ip->stack);
	stack_push(ip->stack, ar - br);
	stack_push(ip->stack, ai - bi);
}

/// V - abs
static void finger_CPLI_abs(instructionPointer * ip)
{
	funge_cell r, i;
	long double tmp;
	i = stack_pop(ip->stack);
	r = stack_pop(ip->stack);
	tmp = sqrtl((long double)(r * r + i * i));
	stack_push(ip->stack, (funge_cell)tmp);
}

bool finger_CPLI_load(instructionPointer * ip)
{
	manager_add_opcode(CPLI, 'A', add)
	manager_add_opcode(CPLI, 'D', div)
	manager_add_opcode(CPLI, 'M', mul)
	manager_add_opcode(CPLI, 'O', out)
	manager_add_opcode(CPLI, 'S', sub)
	manager_add_opcode(CPLI, 'V', abs)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

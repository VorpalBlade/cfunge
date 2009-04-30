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

#include "FPDP.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"

#include <math.h>
#include <stdlib.h> /* strtod */

// Based on how CCBI does it.

/// An union for double and two 32-bit ints.
typedef union u_doubleint {
	double d;
	struct { int32_t high; int32_t low; } i;
} doubleint;

static doubleint u;
static double d;


FUNGE_ATTR_FAST static inline void popDbl(instructionPointer * restrict ip)
{
	u.i.low = (int32_t)stack_pop(ip->stack);
	u.i.high = (int32_t)stack_pop(ip->stack);
}
FUNGE_ATTR_FAST static inline void pushDbl(instructionPointer * restrict ip)
{
	stack_push(ip->stack, u.i.high);
	stack_push(ip->stack, u.i.low);
}

// Basic arithmetics

static void finger_FPDP_add(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d += d;
	pushDbl(ip);
}

static void finger_FPDP_sub(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d -= d;
	pushDbl(ip);
}

static void finger_FPDP_mul(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d *= d;
	pushDbl(ip);
}

static void finger_FPDP_div(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d /= d;
	pushDbl(ip);
}

static void finger_FPDP_sqrt(instructionPointer * ip)
{
	popDbl(ip);
	u.d = sqrt(u.d);
	pushDbl(ip);
}


// Trigonometry

static void finger_FPDP_sin(instructionPointer * ip)
{
	popDbl(ip);
	u.d = sin(u.d);
	pushDbl(ip);
}

static void finger_FPDP_cos(instructionPointer * ip)
{
	popDbl(ip);
	u.d = cos(u.d);
	pushDbl(ip);
}

static void finger_FPDP_tan(instructionPointer * ip)
{
	popDbl(ip);
	u.d = tan(u.d);
	pushDbl(ip);
}

static void finger_FPDP_asin(instructionPointer * ip)
{
	popDbl(ip);
	u.d = asin(u.d);
	pushDbl(ip);
}

static void finger_FPDP_acos(instructionPointer * ip)
{
	popDbl(ip);
	u.d = acos(u.d);
	pushDbl(ip);
}

static void finger_FPDP_atan(instructionPointer * ip)
{
	popDbl(ip);
	u.d = atan(u.d);
	pushDbl(ip);
}


// Logarithms and exponents

static void finger_FPDP_ln(instructionPointer * ip)
{
	popDbl(ip);
	u.d = log(u.d);
	pushDbl(ip);
}

static void finger_FPDP_log10(instructionPointer * ip)
{
	popDbl(ip);
	u.d = log10(u.d);
	pushDbl(ip);
}

static void finger_FPDP_exp(instructionPointer * ip)
{
	popDbl(ip);
	u.d = exp(u.d);
	pushDbl(ip);
}


// Misc stuff

static void finger_FPDP_neg(instructionPointer * ip)
{
	popDbl(ip);
	u.d *= -1;
	pushDbl(ip);
}

static void finger_FPDP_abs(instructionPointer * ip)
{
	popDbl(ip);
	u.d = fabs(u.d);
	pushDbl(ip);
}

static void finger_FPDP_pow(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d = pow(u.d, d);
	pushDbl(ip);
}


// Conversion and standard IO

static void finger_FPDP_fromint(instructionPointer * ip)
{
	funge_cell i;
	i = stack_pop(ip->stack);
	u.d = (double)i;
	pushDbl(ip);
}

static void finger_FPDP_toint(instructionPointer * ip)
{
	popDbl(ip);
	stack_push(ip->stack, (funge_cell)u.d);
}

static void finger_FPDP_fromascii(instructionPointer * ip)
{
	char * restrict str;
	str = (char*)stack_pop_string(ip->stack, NULL);
	u.d = strtod(str, NULL);
	pushDbl(ip);
	stack_free_string(str);
}

static void finger_FPDP_print(instructionPointer * ip)
{
	popDbl(ip);
	printf("%f ", u.d);
}

bool finger_FPDP_load(instructionPointer * ip)
{
	manager_add_opcode(FPDP, 'A', add)
	manager_add_opcode(FPDP, 'B', sin)
	manager_add_opcode(FPDP, 'C', cos)
	manager_add_opcode(FPDP, 'D', div)
	manager_add_opcode(FPDP, 'E', asin)
	manager_add_opcode(FPDP, 'F', fromint)
	manager_add_opcode(FPDP, 'G', atan)
	manager_add_opcode(FPDP, 'H', acos)
	manager_add_opcode(FPDP, 'I', toint)
	manager_add_opcode(FPDP, 'K', ln)
	manager_add_opcode(FPDP, 'L', log10)
	manager_add_opcode(FPDP, 'M', mul)
	manager_add_opcode(FPDP, 'N', neg)
	manager_add_opcode(FPDP, 'P', print)
	manager_add_opcode(FPDP, 'Q', sqrt)
	manager_add_opcode(FPDP, 'R', fromascii)
	manager_add_opcode(FPDP, 'S', sub)
	manager_add_opcode(FPDP, 'T', tan)
	manager_add_opcode(FPDP, 'V', abs)
	manager_add_opcode(FPDP, 'X', exp)
	manager_add_opcode(FPDP, 'Y', pow)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

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

#include "FIXP.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"

#include <math.h>
#include <stdlib.h> /* random */

// M_PIl is a GNU extension. This value should be enough
// for 128-bit long double.
#ifndef M_PIl
#  define M_PIl 3.1415926535897932384626433832795029L
#endif

#ifndef HAVE_acosl
#  define acosl acos
#endif
#ifndef HAVE_asinl
#  define asinl asin
#endif
#ifndef HAVE_atanl
#  define atanl atan
#endif
#ifndef HAVE_cosl
#  define cosl cos
#endif
#ifndef HAVE_powl
#  define powl pow
#endif
#ifndef HAVE_roundl
#  define roundl round
#endif
#ifndef HAVE_sinl
#  define sinl sin
#endif
#ifndef HAVE_sqrtl
#  define sqrtl sqrt
#endif
#ifndef HAVE_tanl
#  define tanl tan
#endif

// For converting between degrees and radians.
#define FUNGE_PI_180 (M_PIl / 180.0)
#define FUNGE_180_PI (180.0 / M_PIl)

/// A - and
static void finger_FIXP_and(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) & stack_pop(ip->stack));
}

/// B - acos
static void finger_FIXP_acos(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * acosl((long double)n / 10000.0L) * FUNGE_180_PI);
	stack_push(ip->stack, (funge_cell)d);
}

/// C - cos
static void finger_FIXP_cos(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * cosl(((long double)n / 10000.0L) * FUNGE_PI_180));
	stack_push(ip->stack, (funge_cell)d);

}

/// D - rand
static void finger_FIXP_rand(instructionPointer * ip)
{
	funge_cell n = stack_pop(ip->stack);

	// No one said this had to be uniform, did they?
	if (n == 0)
		stack_push(ip->stack, 0);
	else
		stack_push(ip->stack, (funge_cell)(random() % n));
}

/// I - sin
static void finger_FIXP_sin(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * sinl(((long double)n / 10000.0L) * FUNGE_PI_180));
	stack_push(ip->stack, (funge_cell)d);
}

/// J - asin
static void finger_FIXP_asin(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * asinl((long double)n / 10000.0L) * FUNGE_180_PI);
	stack_push(ip->stack, (funge_cell)d);
}

/// N - neg
static void finger_FIXP_neg(instructionPointer * ip)
{
	stack_push(ip->stack, -stack_pop(ip->stack));
}

/// O - or
static void finger_FIXP_or(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) | stack_pop(ip->stack));
}

/// P - mulpi
static void finger_FIXP_mulpi(instructionPointer * ip)
{
	stack_push(ip->stack, (funge_cell)(M_PIl * stack_pop(ip->stack)));
}

/// Q - sqrt
static void finger_FIXP_sqrt(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(sqrtl((long double)n));
	stack_push(ip->stack, (funge_cell)d);
}

/// R - pow
static void finger_FIXP_pow(instructionPointer * ip)
{
	long double d;
	funge_cell a, b;
	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);
	d = roundl(powl((long double)a, (long double)b));
	stack_push(ip->stack, (funge_cell)d);
}

/// S - signbit
static void finger_FIXP_signbit(instructionPointer * ip)
{
	funge_cell n = stack_pop(ip->stack);
	stack_push(ip->stack, n > 0 ? 1 : (n < 0 ? -1 : 0));
}

/// T - tan
static void finger_FIXP_tan(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * tanl(((long double)n / 10000.0L) * FUNGE_PI_180));
	stack_push(ip->stack, (funge_cell)d);
}

/// U - atan
static void finger_FIXP_atan(instructionPointer * ip)
{
	long double d;
	funge_cell n = stack_pop(ip->stack);
	d = roundl(10000 * atanl((long double)n / 10000.0L)  * FUNGE_180_PI);
	stack_push(ip->stack, (funge_cell)d);
}

/// V - abs
static void finger_FIXP_abs(instructionPointer * ip)
{
	stack_push(ip->stack, ABS(stack_pop(ip->stack)));
}

/// X - xor
static void finger_FIXP_xor(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) ^ stack_pop(ip->stack));
}

bool finger_FIXP_load(instructionPointer * ip)
{
	manager_add_opcode(FIXP, 'A', and)
	manager_add_opcode(FIXP, 'B', acos)
	manager_add_opcode(FIXP, 'C', cos)
	manager_add_opcode(FIXP, 'D', rand)
	manager_add_opcode(FIXP, 'I', sin)
	manager_add_opcode(FIXP, 'J', asin)
	manager_add_opcode(FIXP, 'N', neg)
	manager_add_opcode(FIXP, 'O', or)
	manager_add_opcode(FIXP, 'P', mulpi)
	manager_add_opcode(FIXP, 'Q', sqrt)
	manager_add_opcode(FIXP, 'R', pow)
	manager_add_opcode(FIXP, 'S', signbit)
	manager_add_opcode(FIXP, 'T', tan)
	manager_add_opcode(FIXP, 'U', atan)
	manager_add_opcode(FIXP, 'V', abs)
	manager_add_opcode(FIXP, 'X', xor)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

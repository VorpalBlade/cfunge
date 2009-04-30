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

#include "FPSP.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"

#include <math.h>
#include <stdlib.h> /* strtof */

// Based on how CCBI does it.

/// An union for float and 32-bit int.
typedef union u_floatint {
	float f;
	int32_t i;
} floatint;


// Basic arithmetics

static void finger_FPSP_add(instructionPointer * ip)
{
	floatint a, b;
	a.i = (int32_t)stack_pop(ip->stack);
	b.i = (int32_t)stack_pop(ip->stack);
	b.f += a.f;
	stack_push(ip->stack, b.i);
}

static void finger_FPSP_sub(instructionPointer * ip)
{
	floatint a, b;
	a.i = (int32_t)stack_pop(ip->stack);
	b.i = (int32_t)stack_pop(ip->stack);
	b.f -= a.f;
	stack_push(ip->stack, b.i);
}

static void finger_FPSP_mul(instructionPointer * ip)
{
	floatint a, b;
	a.i = (int32_t)stack_pop(ip->stack);
	b.i = (int32_t)stack_pop(ip->stack);
	b.f *= a.f;
	stack_push(ip->stack, b.i);
}

static void finger_FPSP_div(instructionPointer * ip)
{
	floatint a, b;
	a.i = (int32_t)stack_pop(ip->stack);
	b.i = (int32_t)stack_pop(ip->stack);
	b.f /= a.f;
	stack_push(ip->stack, b.i);
}

static void finger_FPSP_sqrt(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = sqrtf(a.f);
	stack_push(ip->stack, a.i);
}


// Trigonometry

static void finger_FPSP_sin(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = sinf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_cos(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = cosf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_tan(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = tanf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_asin(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = asinf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_acos(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = acosf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_atan(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = atanf(a.f);
	stack_push(ip->stack, a.i);
}


// Logarithms and exponents

static void finger_FPSP_ln(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = logf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_log10(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = log10f(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_exp(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = expf(a.f);
	stack_push(ip->stack, a.i);
}


// Misc stuff

static void finger_FPSP_neg(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f *= -1;
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_abs(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	a.f = fabsf(a.f);
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_pow(instructionPointer * ip)
{
	floatint a, b;
	a.i = (int32_t)stack_pop(ip->stack);
	b.i = (int32_t)stack_pop(ip->stack);
	b.f = powf(b.f, a.f);
	stack_push(ip->stack, b.i);
}


// Conversion and standard IO

static void finger_FPSP_fromint(instructionPointer * ip)
{
	floatint a;
	funge_cell i;
	i = stack_pop(ip->stack);
	a.f = (float)i;
	stack_push(ip->stack, a.i);
}

static void finger_FPSP_toint(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	stack_push(ip->stack, (funge_cell)a.f);
}

static void finger_FPSP_fromascii(instructionPointer * ip)
{
	char * restrict str;
	floatint a;
	str = (char*)stack_pop_string(ip->stack, NULL);
	a.f = strtof(str, NULL);
	stack_push(ip->stack, a.i);
	stack_free_string(str);
}

static void finger_FPSP_print(instructionPointer * ip)
{
	floatint a;
	a.i = (int32_t)stack_pop(ip->stack);
	printf("%f ", a.f);
}

bool finger_FPSP_load(instructionPointer * ip)
{
	manager_add_opcode(FPSP, 'A', add)
	manager_add_opcode(FPSP, 'B', sin)
	manager_add_opcode(FPSP, 'C', cos)
	manager_add_opcode(FPSP, 'D', div)
	manager_add_opcode(FPSP, 'E', asin)
	manager_add_opcode(FPSP, 'F', fromint)
	manager_add_opcode(FPSP, 'G', atan)
	manager_add_opcode(FPSP, 'H', acos)
	manager_add_opcode(FPSP, 'I', toint)
	manager_add_opcode(FPSP, 'K', ln)
	manager_add_opcode(FPSP, 'L', log10)
	manager_add_opcode(FPSP, 'M', mul)
	manager_add_opcode(FPSP, 'N', neg)
	manager_add_opcode(FPSP, 'P', print)
	manager_add_opcode(FPSP, 'Q', sqrt)
	manager_add_opcode(FPSP, 'R', fromascii)
	manager_add_opcode(FPSP, 'S', sub)
	manager_add_opcode(FPSP, 'T', tan)
	manager_add_opcode(FPSP, 'V', abs)
	manager_add_opcode(FPSP, 'X', exp)
	manager_add_opcode(FPSP, 'Y', pow)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

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

#include "FPSP.h"
#include "../../stack.h"

#include <stdlib.h>
#include <math.h>

// Based on how CCBI does it.


// Lets hope float is always 32 bits...
typedef union u_floatint {
	float f;
	int32_t i;
} floatint;


// Basic arithmetics

static void FingerFPSPadd(instructionPointer * ip)
{
	floatint a, b;
	a.i = StackPop(ip->stack);
	b.i = StackPop(ip->stack);
	a.f += b.f;
	StackPush(ip->stack, a.i);
}

static void FingerFPSPsub(instructionPointer * ip)
{
	floatint a, b;
	a.i = StackPop(ip->stack);
	b.i = StackPop(ip->stack);
	b.f -= a.f;
	StackPush(ip->stack, b.i);
}

static void FingerFPSPmul(instructionPointer * ip)
{
	floatint a, b;
	a.i = StackPop(ip->stack);
	b.i = StackPop(ip->stack);
	b.f *= a.f;
	StackPush(ip->stack, b.i);
}

static void FingerFPSPdiv(instructionPointer * ip)
{
	floatint a, b;
	a.i = StackPop(ip->stack);
	b.i = StackPop(ip->stack);
	b.f /= a.f;
	StackPush(ip->stack, b.i);
}

static void FingerFPSPsqrt(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = sqrtf(a.f);
	StackPush(ip->stack, a.i);
}


// Trigonometry

static void FingerFPSPsin(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = sinf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPcos(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = cosf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPtan(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = tanf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPasin(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = asinf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPacos(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = acosf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPatan(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = atanf(a.f);
	StackPush(ip->stack, a.i);
}


// Logarithms and exponents

static void FingerFPSPln(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = logf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPlog10(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = log10f(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPexp(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = expf(a.f);
	StackPush(ip->stack, a.i);
}


// Misc stuff

static void FingerFPSPneg(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f *= -1;
	StackPush(ip->stack, a.i);
}

static void FingerFPSPabs(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	a.f = fabsf(a.f);
	StackPush(ip->stack, a.i);
}

static void FingerFPSPpow(instructionPointer * ip)
{
	floatint a, b;
	a.i = StackPop(ip->stack);
	b.i = StackPop(ip->stack);
	b.f = powf(b.f, a.f);
	StackPush(ip->stack, b.i);
}


// Conversion and standard IO

static void FingerFPSPfromint(instructionPointer * ip)
{
	floatint a;
	FUNGEDATATYPE i;
	i = StackPop(ip->stack);
	a.f = (float)i;
	StackPush(ip->stack, a.i);
}

static void FingerFPSPtoint(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	StackPush(ip->stack, (FUNGEDATATYPE)a.f);
}

static void FingerFPSPfromascii(instructionPointer * ip)
{
	char * restrict str;
	floatint a;
	str = StackPopString(ip->stack);
	a.f = strtof(str, NULL);
	StackPush(ip->stack, a.i);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

static void FingerFPSPprint(instructionPointer * ip)
{
	floatint a;
	a.i = StackPop(ip->stack);
	printf("%f ", a.f);
}

bool FingerFPSPload(instructionPointer * ip) {
	if (!OpcodeStackAdd(ip, 'A', &FingerFPSPadd))
		return false;
	if (!OpcodeStackAdd(ip, 'B', &FingerFPSPsin))
		return false;
	if (!OpcodeStackAdd(ip, 'C', &FingerFPSPcos))
		return false;
	if (!OpcodeStackAdd(ip, 'D', &FingerFPSPdiv))
		return false;
	if (!OpcodeStackAdd(ip, 'E', &FingerFPSPasin))
		return false;
	if (!OpcodeStackAdd(ip, 'F', &FingerFPSPfromint))
		return false;
	if (!OpcodeStackAdd(ip, 'G', &FingerFPSPatan))
		return false;
	if (!OpcodeStackAdd(ip, 'H', &FingerFPSPacos))
		return false;
	if (!OpcodeStackAdd(ip, 'I', &FingerFPSPtoint))
		return false;
	if (!OpcodeStackAdd(ip, 'K', &FingerFPSPln))
		return false;
	if (!OpcodeStackAdd(ip, 'L', &FingerFPSPlog10))
		return false;
	if (!OpcodeStackAdd(ip, 'M', &FingerFPSPmul))
		return false;
	if (!OpcodeStackAdd(ip, 'N', &FingerFPSPneg))
		return false;
	if (!OpcodeStackAdd(ip, 'P', &FingerFPSPprint))
		return false;
	if (!OpcodeStackAdd(ip, 'Q', &FingerFPSPsqrt))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerFPSPfromascii))
		return false;
	if (!OpcodeStackAdd(ip, 'S', &FingerFPSPsub))
		return false;
	if (!OpcodeStackAdd(ip, 'T', &FingerFPSPtan))
		return false;
	if (!OpcodeStackAdd(ip, 'V', &FingerFPSPabs))
		return false;
	if (!OpcodeStackAdd(ip, 'X', &FingerFPSPexp))
		return false;
	if (!OpcodeStackAdd(ip, 'Y', &FingerFPSPpow))
		return false;
	return true;
}

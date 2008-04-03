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

#include "FPDP.h"
#include "../../stack.h"
#include <stdlib.h>
#include <math.h>

// Based on how CCBI does it.

// Lets hope double is always 64 bits...
typedef union u_doubleint {
	double d;
	struct { int32_t high; int32_t low; } i;
} doubleint;

static doubleint u;
static double d;


FUNGE_FAST static inline void popDbl(instructionPointer * ip)
{
	u.i.low = StackPop(ip->stack);
	u.i.high = StackPop(ip->stack);
}
FUNGE_FAST static inline void pushDbl(instructionPointer * ip)
{
	StackPush(ip->stack, u.i.high);
	StackPush(ip->stack, u.i.low);
}

// Basic arithmetics

static void FingerFPDPadd(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d += d;
	pushDbl(ip);
}

static void FingerFPDPsub(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d -= d;
	pushDbl(ip);
}

static void FingerFPDPmul(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d *= d;
	pushDbl(ip);
}

static void FingerFPDPdiv(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d /= d;
	pushDbl(ip);
}

static void FingerFPDPsqrt(instructionPointer * ip)
{
	popDbl(ip);
	u.d = sqrt(u.d);
	pushDbl(ip);
}


// Trigonometry

static void FingerFPDPsin(instructionPointer * ip)
{
	popDbl(ip);
	u.d = sin(u.d);
	pushDbl(ip);
}

static void FingerFPDPcos(instructionPointer * ip)
{
	popDbl(ip);
	u.d = cos(u.d);
	pushDbl(ip);
}

static void FingerFPDPtan(instructionPointer * ip)
{
	popDbl(ip);
	u.d = tan(u.d);
	pushDbl(ip);
}

static void FingerFPDPasin(instructionPointer * ip)
{
	popDbl(ip);
	u.d = asin(u.d);
	pushDbl(ip);
}

static void FingerFPDPacos(instructionPointer * ip)
{
	popDbl(ip);
	u.d = acos(u.d);
	pushDbl(ip);
}

static void FingerFPDPatan(instructionPointer * ip)
{
	popDbl(ip);
	u.d = atan(u.d);
	pushDbl(ip);
}


// Logarithms and exponents

static void FingerFPDPln(instructionPointer * ip)
{
	popDbl(ip);
	u.d = log(u.d);
	pushDbl(ip);
}

static void FingerFPDPlog10(instructionPointer * ip)
{
	popDbl(ip);
	u.d = log10(u.d);
	pushDbl(ip);
}

static void FingerFPDPexp(instructionPointer * ip)
{
	popDbl(ip);
	u.d = exp(u.d);
	pushDbl(ip);
}


// Misc stuff

static void FingerFPDPneg(instructionPointer * ip)
{
	popDbl(ip);
	u.d *= -1;
	pushDbl(ip);
}

static void FingerFPDPabs(instructionPointer * ip)
{
	popDbl(ip);
	u.d = fabs(u.d);
	pushDbl(ip);
}

static void FingerFPDPpow(instructionPointer * ip)
{
	popDbl(ip);
	d = u.d;
	popDbl(ip);
	u.d = powf(u.d, d);
	pushDbl(ip);
}


// Conversion and standard IO

static void FingerFPDPfromint(instructionPointer * ip)
{
	FUNGEDATATYPE i;
	i = StackPop(ip->stack);
	u.d = (double)i;
	pushDbl(ip);
}

static void FingerFPDPtoint(instructionPointer * ip)
{
	popDbl(ip);
	StackPush(ip->stack, (FUNGEDATATYPE)u.d);
}

static void FingerFPDPfromascii(instructionPointer * ip)
{
	char * restrict str;
	str = StackPopString(ip->stack);
	u.d = strtod(str, NULL);
	pushDbl(ip);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

static void FingerFPDPprint(instructionPointer * ip)
{
	popDbl(ip);
	printf("%f ", u.d);
}

bool FingerFPDPload(instructionPointer * ip)
{
	if (!OpcodeStackAdd(ip, 'A', &FingerFPDPadd))
		return false;
	if (!OpcodeStackAdd(ip, 'B', &FingerFPDPsin))
		return false;
	if (!OpcodeStackAdd(ip, 'C', &FingerFPDPcos))
		return false;
	if (!OpcodeStackAdd(ip, 'D', &FingerFPDPdiv))
		return false;
	if (!OpcodeStackAdd(ip, 'E', &FingerFPDPasin))
		return false;
	if (!OpcodeStackAdd(ip, 'F', &FingerFPDPfromint))
		return false;
	if (!OpcodeStackAdd(ip, 'G', &FingerFPDPatan))
		return false;
	if (!OpcodeStackAdd(ip, 'H', &FingerFPDPacos))
		return false;
	if (!OpcodeStackAdd(ip, 'I', &FingerFPDPtoint))
		return false;
	if (!OpcodeStackAdd(ip, 'K', &FingerFPDPln))
		return false;
	if (!OpcodeStackAdd(ip, 'L', &FingerFPDPlog10))
		return false;
	if (!OpcodeStackAdd(ip, 'M', &FingerFPDPmul))
		return false;
	if (!OpcodeStackAdd(ip, 'N', &FingerFPDPneg))
		return false;
	if (!OpcodeStackAdd(ip, 'P', &FingerFPDPprint))
		return false;
	if (!OpcodeStackAdd(ip, 'Q', &FingerFPDPsqrt))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerFPDPfromascii))
		return false;
	if (!OpcodeStackAdd(ip, 'S', &FingerFPDPsub))
		return false;
	if (!OpcodeStackAdd(ip, 'T', &FingerFPDPtan))
		return false;
	if (!OpcodeStackAdd(ip, 'V', &FingerFPDPabs))
		return false;
	if (!OpcodeStackAdd(ip, 'X', &FingerFPDPexp))
		return false;
	if (!OpcodeStackAdd(ip, 'Y', &FingerFPDPpow))
		return false;
	return true;
}

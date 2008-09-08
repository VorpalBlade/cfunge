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

#include "FIXP.h"
#include "../../stack.h"

#include <stdlib.h>
#include <math.h>

// Yeah, some systems are *really* crap.
// This includes Mingw on windows when I tried.
#ifndef M_PI
#  define M_PI 3.14159265358979323846
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
#define FUNGE_PI_180 (M_PI / 180.0)
#define FUNGE_180_PI (180.0 / M_PI)

/// A - and
static void FingerFIXPand(instructionPointer * ip)
{
	StackPush(ip->stack, StackPop(ip->stack) & StackPop(ip->stack));
}

/// B - acos
static void FingerFIXPacos(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * acosl((long double)n / 10000) * FUNGE_180_PI);
	StackPush(ip->stack, (fungeCell)d);
}

/// C - cos
static void FingerFIXPcos(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * cosl(((long double)n / 10000) * FUNGE_PI_180));
	StackPush(ip->stack, (fungeCell)d);

}

/// D - rand
static void FingerFIXPrand(instructionPointer * ip)
{
	fungeCell n = StackPop(ip->stack);

	// No one said this had to be uniform, did they?
	if (n == 0)
		StackPush(ip->stack, 0);
	else
		StackPush(ip->stack, random() % n);
}

/// I - sin
static void FingerFIXPsin(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * sinl(((long double)n / 10000) * FUNGE_PI_180));
	StackPush(ip->stack, (fungeCell)d);
}

/// J - asin
static void FingerFIXPasin(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * asinl((long double)n / 10000) * FUNGE_180_PI);
	StackPush(ip->stack, (fungeCell)d);
}

/// N - neg
static void FingerFIXPneg(instructionPointer * ip)
{
	StackPush(ip->stack, -StackPop(ip->stack));
}

/// O - or
static void FingerFIXPor(instructionPointer * ip)
{
	StackPush(ip->stack, StackPop(ip->stack) | StackPop(ip->stack));
}

/// P - mulpi
static void FingerFIXPmulpi(instructionPointer * ip)
{
	StackPush(ip->stack, (fungeCell)(M_PI * StackPop(ip->stack)));
}

/// Q - sqrt
static void FingerFIXPsqrt(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(sqrtl((long double)n));
	StackPush(ip->stack, (fungeCell)d);
}

/// R - pow
static void FingerFIXPpow(instructionPointer * ip)
{
	long double d;
	fungeCell a, b;
	b = StackPop(ip->stack);
	a = StackPop(ip->stack);
	d = roundl(powl((long double)a, (long double)b));
	StackPush(ip->stack, (fungeCell)d);
}

/// S - signbit
static void FingerFIXPsignbit(instructionPointer * ip)
{
	fungeCell n = StackPop(ip->stack);
	StackPush(ip->stack, n > 0 ? 1 : (n < 0 ? -1 : 0));
}

/// T - tan
static void FingerFIXPtan(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * tanl(((long double)n / 10000) * FUNGE_PI_180));
	StackPush(ip->stack, (fungeCell)d);
}

/// U - atan
static void FingerFIXPatan(instructionPointer * ip)
{
	long double d;
	fungeCell n = StackPop(ip->stack);
	d = roundl(10000 * atanl((long double)n / 10000)  * FUNGE_180_PI);
	StackPush(ip->stack, (fungeCell)d);
}

/// V - abs
static void FingerFIXPabs(instructionPointer * ip)
{
	StackPush(ip->stack, ABS(StackPop(ip->stack)));
}

/// X - xor
static void FingerFIXPxor(instructionPointer * ip)
{
	StackPush(ip->stack, StackPop(ip->stack) ^ StackPop(ip->stack));
}

bool FingerFIXPload(instructionPointer * ip)
{
	ManagerAddOpcode(FIXP, 'A', and)
	ManagerAddOpcode(FIXP, 'B', acos)
	ManagerAddOpcode(FIXP, 'C', cos)
	ManagerAddOpcode(FIXP, 'D', rand)
	ManagerAddOpcode(FIXP, 'I', sin)
	ManagerAddOpcode(FIXP, 'J', asin)
	ManagerAddOpcode(FIXP, 'N', neg)
	ManagerAddOpcode(FIXP, 'O', or)
	ManagerAddOpcode(FIXP, 'P', mulpi)
	ManagerAddOpcode(FIXP, 'Q', sqrt)
	ManagerAddOpcode(FIXP, 'R', pow)
	ManagerAddOpcode(FIXP, 'S', signbit)
	ManagerAddOpcode(FIXP, 'T', tan)
	ManagerAddOpcode(FIXP, 'U', atan)
	ManagerAddOpcode(FIXP, 'V', abs)
	ManagerAddOpcode(FIXP, 'X', xor)
	return true;
}

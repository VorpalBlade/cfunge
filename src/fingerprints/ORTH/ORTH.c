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
	FUNGEDATATYPE x, y;
	x = StackPop(ip->stack);
	y = StackPop(ip->stack);
	StackPush(ip->stack, x & y);
}

static void FingerORTHbitOr(instructionPointer * ip)
{
	FUNGEDATATYPE x, y;
	x = StackPop(ip->stack);
	y = StackPop(ip->stack);
	StackPush(ip->stack, x | y);
}

static void FingerORTHbitXor(instructionPointer * ip)
{
	FUNGEDATATYPE x, y;
	x = StackPop(ip->stack);
	y = StackPop(ip->stack);
	StackPush(ip->stack, x ^ y);
}

// ortho get
static void FingerORTHget(instructionPointer * ip)
{
	fungeVector v = StackPopVector(ip->stack);
	StackPush(ip->stack, FungeSpaceGet(VectorCreateRef(v.y, v.x)));
}

// ortho put
static void FingerORTHput(instructionPointer * ip)
{
	fungeVector v;
	FUNGEDATATYPE c;

	v = StackPopVector(ip->stack);
	c = StackPop(ip->stack);

	FungeSpaceSet(c, VectorCreateRef(v.y, v.x));
}

// output string
static void FingerORTHoutputString(instructionPointer * ip)
{
	char * restrict str = StackPopString(ip->stack);
	// puts add newline, we therefore do fputs on stdout
	fputs(str, stdout);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

// change dx
static void FingerORTHchangeDx(instructionPointer * ip)
{
	ip->delta.x = StackPop(ip->stack);
}

// change dy
static void FingerORTHchangeDy(instructionPointer * ip)
{
	ip->delta.y = StackPop(ip->stack);
}

// change x
static void FingerORTHchangeX(instructionPointer * ip)
{
	ipSetPosition(ip, VectorCreateRef(StackPop(ip->stack), ip->position.y));
}

// change y
static void FingerORTHchangeY(instructionPointer * ip)
{
	ipSetPosition(ip, VectorCreateRef(ip->position.x, StackPop(ip->stack)));
}

// ramp if zero
static void FingerORTHrampIfZero(instructionPointer * ip)
{
	if (!StackPop(ip->stack))
		ipForward(ip, 1);
}

bool FingerORTHload(instructionPointer * ip)
{
	if (!OpcodeStackAdd(ip, 'A', &FingerORTHbitAnd))
		return false;
	if (!OpcodeStackAdd(ip, 'O', &FingerORTHbitOr))
		return false;
	if (!OpcodeStackAdd(ip, 'E', &FingerORTHbitXor))
		return false;
	if (!OpcodeStackAdd(ip, 'G', &FingerORTHget))
		return false;
	if (!OpcodeStackAdd(ip, 'P', &FingerORTHput))
		return false;
	if (!OpcodeStackAdd(ip, 'S', &FingerORTHoutputString))
		return false;
	if (!OpcodeStackAdd(ip, 'V', &FingerORTHchangeDx))
		return false;
	if (!OpcodeStackAdd(ip, 'W', &FingerORTHchangeDy))
		return false;
	if (!OpcodeStackAdd(ip, 'X', &FingerORTHchangeX))
		return false;
	if (!OpcodeStackAdd(ip, 'Y', &FingerORTHchangeY))
		return false;
	if (!OpcodeStackAdd(ip, 'Z', &FingerORTHrampIfZero))
		return false;

	return true;
}

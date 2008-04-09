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

#include "CPLI.h"
#include "../../stack.h"

#include <math.h>

// A - add
static void FingerCPLIadd(instructionPointer * ip) {
	FUNGEDATATYPE ar, ai, br, bi;
	bi = StackPop(ip->stack);
	br = StackPop(ip->stack);
	ai = StackPop(ip->stack);
	ar = StackPop(ip->stack);
	StackPush(ip->stack, ar + br);
	StackPush(ip->stack, ai + bi);
}

// D - div
static void FingerCPLIdiv(instructionPointer * ip) {
	FUNGEDATATYPE ar, ai, br, bi, denom;
	bi = StackPop(ip->stack);
	br = StackPop(ip->stack);
	ai = StackPop(ip->stack);
	ar = StackPop(ip->stack);
	denom = bi*bi + br*br;
	StackPush(ip->stack, (ai*bi + ar*br) / denom);
	StackPush(ip->stack, (ai*br - ar*bi) / denom);
}

// M - mul
static void FingerCPLImul(instructionPointer * ip) {
	FUNGEDATATYPE ar, ai, br, bi;
	bi = StackPop(ip->stack);
	br = StackPop(ip->stack);
	ai = StackPop(ip->stack);
	ar = StackPop(ip->stack);
	StackPush(ip->stack, ar*br - ai*bi);
	StackPush(ip->stack, ar*bi + ai*br);
}

// O - out
static void FingerCPLIout(instructionPointer * ip) {
	FUNGEDATATYPE r, i;
	i = StackPop(ip->stack);
	r = StackPop(ip->stack);
	printf("%" FUNGEDATAPRI, r);
	if (i > 0)
		putchar('+');
	printf("%" FUNGEDATAPRI "i ", i);
}

// S - sub
static void FingerCPLIsub(instructionPointer * ip) {
	FUNGEDATATYPE ar, ai, br, bi;
	bi = StackPop(ip->stack);
	br = StackPop(ip->stack);
	ai = StackPop(ip->stack);
	ar = StackPop(ip->stack);
	StackPush(ip->stack, ar - br);
	StackPush(ip->stack, ai - bi);
}

// V - abs
static void FingerCPLIabs(instructionPointer * ip) {
	FUNGEDATATYPE r, i;
	double tmp;
	i = StackPop(ip->stack);
	r = StackPop(ip->stack);
	tmp = sqrt((double)(r*r + i*i));
	StackPush(ip->stack, (FUNGEDATATYPE)tmp);
}

bool FingerCPLIload(instructionPointer * ip) {
	// Insert the functions in question after the &
	if (!OpcodeStackAdd(ip, 'A', &FingerCPLIadd))
		return false;
	if (!OpcodeStackAdd(ip, 'D', &FingerCPLIdiv))
		return false;
	if (!OpcodeStackAdd(ip, 'M', &FingerCPLImul))
		return false;
	if (!OpcodeStackAdd(ip, 'O', &FingerCPLIout))
		return false;
	if (!OpcodeStackAdd(ip, 'S', &FingerCPLIsub))
		return false;
	if (!OpcodeStackAdd(ip, 'V', &FingerCPLIabs))
		return false;
	return true;
}

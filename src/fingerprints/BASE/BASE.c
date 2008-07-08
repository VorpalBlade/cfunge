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

#include "BASE.h"
#include "../../stack.h"
#include "../../input.h"

#include <stdio.h>
#include <math.h>

FUNGE_ATTR_FAST static void binary(FUNGEDATATYPE number)
{
	if (number > 0) {
		binary(number >> 1);
		cf_putchar_unlocked(number & 1 ? '1' : '0');
	}
}

static void FingerBASEoutputBinary(instructionPointer * ip)
{
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	cf_flockfile(stdout);
	binary(x);
	cf_putchar_unlocked(' ');
	cf_funlockfile(stdout);
}

static void FingerBASEoutputOctal(instructionPointer * ip)
{
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	printf("%" FUNGEDATAoctPRI " ", (FUNGEunsignedDATATYPE)x);
}

static void FingerBASEoutputHex(instructionPointer * ip)
{
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	printf("%" FUNGEDATAhexPRI " ", (FUNGEunsignedDATATYPE)x);
}

#define anyLog(base, value) (log(value)/log(base))

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static void FingerBASEoutputBase(instructionPointer * ip)
{
	FUNGEDATATYPE base, val;

	base = StackPop(ip->stack);
	val = StackPop(ip->stack);
	if ((base < 1) || (base > 36) || (val < 0)) {
		ipReverse(ip);
		return;
	}

	cf_flockfile(stdout);
	if (base == 1) {
		while (val--)
			cf_putchar_unlocked('0');
		cf_putchar_unlocked(' ');
	} else if (!val) {
		cf_putchar_unlocked('0');
	} else {
		// We need at most this size of the string.
		size_t i = ceil(anyLog((double)base, (double)val) + 1);
		char * restrict result = (char*)cf_malloc_noptr(i * sizeof(char));
		for (i = 0; val > 0; val /= base)
			result[i++] = digits[val % base];
		for (; i-- > 0;)
			cf_putchar_unlocked(result[i]);
		cf_putchar_unlocked(' ');
		cf_free(result);
	}
	cf_funlockfile(stdout);
}

static void FingerBASEinputBase(instructionPointer * ip)
{
	FUNGEDATATYPE base;
	ret_getint gotint = rgi_noint;
	FUNGEDATATYPE a = 0;

	base = StackPop(ip->stack);
	if ((base < 1) || (base > 36)) {
		ipReverse(ip);
		return;
	}

	fflush(stdout);

	while (gotint == rgi_noint) {
		gotint = input_getint(&a, base);
	}
	if (gotint == rgi_success) {
		StackPush(ip->stack, a);
	} else {
		ipReverse(ip);
	}
}


bool FingerBASEload(instructionPointer * ip)
{
	ManagerAddOpcode(BASE,  'B', outputBinary)
	ManagerAddOpcode(BASE,  'H', outputHex)
	ManagerAddOpcode(BASE,  'I', inputBase)
	ManagerAddOpcode(BASE,  'N', outputBase)
	ManagerAddOpcode(BASE,  'O', outputOctal)
	return true;
}

/*
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

// FIXME: replace with non-recursive.
static void binary(FUNGEDATATYPE number) {
	if(number > 0) {
		binary(number >> 1);
		putchar(number & 1 ? '1' : '0');
	}
}

static void FingerBASEoutputBinary(instructionPointer * ip) {
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	binary(x);
	putchar(' ');
}

static void FingerBASEoutputOctal(instructionPointer * ip) {
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	printf("%" FUNGEDATAoctPRI " ", (FUNGEunsignedDATATYPE)x);
}

static void FingerBASEoutputHex(instructionPointer * ip) {
	FUNGEDATATYPE x;
	x = StackPop(ip->stack);
	printf("%" FUNGEDATAhexPRI " ", (FUNGEunsignedDATATYPE)x);
}

#define anyLog(base, value) (logf(value)/logf(base))

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static void FingerBASEoutputBase(instructionPointer * ip) {
	FUNGEDATATYPE base, val;

	base = StackPop(ip->stack);
	val = StackPop(ip->stack);
	if ((base < 1) || (base > 36) || (val < 0)) {
		ipReverse(ip);
		return;
	}

	if (base == 1) {
		while (val--)
			putchar('0');
	} else if (!val) {
		putchar('0');
	} else {
		ssize_t i = ceil(anyLog((double)base, (double)val) + 1);
		char * result = cf_calloc(i, sizeof(char));
		for (i = 0; val > 0; val /= base)
			result[i++] = digits[val % base];
		for(; i >= 0; i--)
			putchar(result[i]);
		putchar(' ');
	}
}

static void FingerBASEinputBase(instructionPointer * ip) {
	FUNGEDATATYPE base;
	FUNGEDATATYPE a = 0;

	base = StackPop(ip->stack);
	if ((base < 1) || (base > 36)) {
		ipReverse(ip);
		return;
	}

	fflush(stdout);

	while (!input_getint(&a, base));
	StackPush(a, ip->stack);
}


bool FingerBASEload(instructionPointer * ip) {
	if (!OpcodeStackAdd(ip, 'B', &FingerBASEoutputBinary))
		return false;
	if (!OpcodeStackAdd(ip, 'H', &FingerBASEoutputHex))
		return false;
	if (!OpcodeStackAdd(ip, 'I', &FingerBASEinputBase))
		return false;
	if (!OpcodeStackAdd(ip, 'N', &FingerBASEoutputBase))
		return false;
	if (!OpcodeStackAdd(ip, 'O', &FingerBASEoutputOctal))
		return false;
	return true;
}

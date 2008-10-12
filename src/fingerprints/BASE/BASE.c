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

FUNGE_ATTR_FAST static void binary(fungeCell number)
{
	if (number > 0) {
		binary(number >> 1);
		cf_putchar_unlocked(number & 1 ? '1' : '0');
	}
}

static void finger_BASE_outputBinary(instructionPointer * ip)
{
	fungeCell x;
	x = stack_pop(ip->stack);
	cf_flockfile(stdout);
	binary(x);
	cf_putchar_unlocked(' ');
	cf_funlockfile(stdout);
}

static void finger_BASE_outputOctal(instructionPointer * ip)
{
	fungeCell x;
	x = stack_pop(ip->stack);
	printf("%" FUNGECELLoctPRI " ", (fungeUnsignedCell)x);
}

static void finger_BASE_outputHex(instructionPointer * ip)
{
	fungeCell x;
	x = stack_pop(ip->stack);
	printf("%" FUNGECELLhexPRI " ", (fungeUnsignedCell)x);
}

#define anyLog(base, value) (log(value)/log(base))

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static void finger_BASE_outputBase(instructionPointer * ip)
{
	fungeCell base, val;

	base = stack_pop(ip->stack);
	val = stack_pop(ip->stack);
	if ((base < 1) || (base > 36) || (val < 0)) {
		ip_reverse(ip);
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

static void finger_BASE_inputBase(instructionPointer * ip)
{
	fungeCell base;
	ret_getint gotint = rgi_noint;
	fungeCell a = 0;

	base = stack_pop(ip->stack);
	if ((base < 1) || (base > 36)) {
		ip_reverse(ip);
		return;
	}

	fflush(stdout);

	while (gotint == rgi_noint) {
		gotint = input_getint(&a, base);
	}
	if (gotint == rgi_success) {
		stack_push(ip->stack, a);
	} else {
		ip_reverse(ip);
	}
}


bool finger_BASE_load(instructionPointer * ip)
{
	manager_add_opcode(BASE,  'B', outputBinary)
	manager_add_opcode(BASE,  'H', outputHex)
	manager_add_opcode(BASE,  'I', inputBase)
	manager_add_opcode(BASE,  'N', outputBase)
	manager_add_opcode(BASE,  'O', outputOctal)
	return true;
}

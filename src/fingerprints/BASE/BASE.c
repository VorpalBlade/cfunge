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

#include "BASE.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"
#include "../../input.h"

#include <stdio.h>
#include <math.h>

FUNGE_ATTR_FAST static void binary(funge_cell number)
{
	if (number > 0) {
		binary(number >> 1);
		cf_putchar_unlocked(number & 1 ? '1' : '0');
	}
}

static void finger_BASE_output_binary(instructionPointer * ip)
{
	funge_cell x;
	x = stack_pop(ip->stack);
	cf_flockfile(stdout);
	binary(x);
	cf_putchar_unlocked(' ');
	cf_funlockfile(stdout);
}

static void finger_BASE_output_octal(instructionPointer * ip)
{
	funge_cell x;
	x = stack_pop(ip->stack);
	printf("%" FUNGECELLoctPRI " ", (funge_unsigned_cell)x);
}

static void finger_BASE_output_hex(instructionPointer * ip)
{
	funge_cell x;
	x = stack_pop(ip->stack);
	printf("%" FUNGECELLhexPRI " ", (funge_unsigned_cell)x);
}

#define anyLog(base, value) (log(value)/log(base))

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static void finger_BASE_output_base(instructionPointer * ip)
{
	funge_cell base, val;

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
		// WARNING: VLA here. Should be safe though, since maximum size is small.
		char result[i];
		for (i = 0; val > 0; val /= base)
			result[i++] = digits[val % base];
		for (; i-- > 0;)
			cf_putchar_unlocked(result[i]);
		cf_putchar_unlocked(' ');
	}
	cf_funlockfile(stdout);
}

static void finger_BASE_input_base(instructionPointer * ip)
{
	funge_cell base;
	ret_getint gotint = rgi_noint;
	funge_cell a = 0;

	base = stack_pop(ip->stack);
	if ((base < 1) || (base > 36)) {
		ip_reverse(ip);
		return;
	}

	fflush(stdout);

	while (gotint == rgi_noint) {
		gotint = input_getint(&a, (int)base);
	}
	if (gotint == rgi_success) {
		stack_push(ip->stack, a);
	} else {
		ip_reverse(ip);
	}
}


bool finger_BASE_load(instructionPointer * ip)
{
	manager_add_opcode(BASE, 'B', output_binary)
	manager_add_opcode(BASE, 'H', output_hex)
	manager_add_opcode(BASE, 'I', input_base)
	manager_add_opcode(BASE, 'N', output_base)
	manager_add_opcode(BASE, 'O', output_octal)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

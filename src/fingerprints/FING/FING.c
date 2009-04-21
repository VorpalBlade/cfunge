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

#include "FING.h"
#include "../../stack.h"

/// Used to handle "0-25" and "A-Z"
/// @return Returns 0 for invalid, otherwise A-Z
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
static inline unsigned char pop_stack_spec(instructionPointer * ip)
{
	funge_cell n = stack_pop(ip->stack);
	if (n < 0) return 0;
	else if (n <= 25) return (unsigned char)((funge_cell)'A' + n);
	else if (n < 'A') return 0;
	else if (n <= 'Z') return (unsigned char)n;
	else return 0;
}

/// Used for pushing a reflect on stack.
static void do_reflect(instructionPointer * ip)
{
	ip_reverse(ip);
}

/// X - Swap two semantics
static void finger_FING_swap(instructionPointer * ip)
{
	unsigned char first = pop_stack_spec(ip);
	unsigned char second = pop_stack_spec(ip);
	if (first == 0 || second == 0) {
		ip_reverse(ip);
	} else {
		fingerprintOpcode op1 = opcode_stack_pop(ip, first);
		fingerprintOpcode op2 = opcode_stack_pop(ip, second);
		// Push reflect if there wasn't anything on the stack.
		if (!op1)
			op1 = &do_reflect;
		if (!op2)
			op2 = &do_reflect;
		opcode_stack_push(ip, second, op1);
		opcode_stack_push(ip, first, op2);
	}
}

/// Y - Drop semantic
static void finger_FING_drop(instructionPointer * ip)
{
	unsigned char opcode = pop_stack_spec(ip);
	if (opcode == 0) {
		ip_reverse(ip);
	} else {
		opcode_stack_pop(ip, opcode);
	}
}

/// Z - Push source semantic onto dst
static void finger_FING_push(instructionPointer * ip)
{
	unsigned char dst = pop_stack_spec(ip);
	unsigned char src = pop_stack_spec(ip);
	if (src == 0 || dst == 0) {
		ip_reverse(ip);
	} else {
		fingerprintOpcode op = opcode_stack_pop(ip, src);
		if (op == NULL) {
			op = &do_reflect;
		} else {
			if (!opcode_stack_push(ip, src, op)) {
				ip_reverse(ip);
				return;
			}
		}
		if (!opcode_stack_push(ip, dst, op))
			ip_reverse(ip);
	}
}

bool finger_FING_load(instructionPointer * ip)
{
	manager_add_opcode(FING, 'X', swap)
	manager_add_opcode(FING, 'Y', drop)
	manager_add_opcode(FING, 'Z', push)
	return true;
}

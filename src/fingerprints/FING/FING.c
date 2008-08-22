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

#include "FING.h"
#include "../../stack.h"

/// Used to handle "0-25" and "A-Z"
/// @return Returns 0 for invalid, otherwise A-Z
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
static inline char PopStackSpec(instructionPointer * ip)
{
	fungeCell n = StackPop(ip->stack);
	if (n < 0) return 0;
	else if (n <= 25) return 'A' + n;
	else if (n < 'A') return 0;
	else if (n <= 'Z') return n;
	else return 0;
}

/// Used for pushing a reflect on stack.
static void DoReflect(instructionPointer * ip)
{
	ipReverse(ip);
}

/// X - Swap two semantics
static void FingerFINGswap(instructionPointer * ip)
{
	char first = PopStackSpec(ip);
	char second = PopStackSpec(ip);
	if (first == 0 || second == 0) {
		ipReverse(ip);
	} else {
		fingerprintOpcode op1 = OpcodeStackPop(ip, first);
		fingerprintOpcode op2 = OpcodeStackPop(ip, second);

		if (!op1 || !op2) {
			// Add them back
			if (op1) OpcodeStackAdd(ip, first, op1);
			if (op2) OpcodeStackAdd(ip, second, op2);
			ipReverse(ip);
		} else {
			OpcodeStackAdd(ip, second, op1);
			OpcodeStackAdd(ip, first, op2);
		}
	}
}

/// Y - Drop semantic
static void FingerFINGdrop(instructionPointer * ip)
{
	char opcode = PopStackSpec(ip);
	if (opcode == 0) {
		ipReverse(ip);
	} else {
		if (OpcodeStackPop(ip, opcode) == NULL) {
			ipReverse(ip);
		}
	}
}

/// Z - Push source semantic onto dst
static void FingerFINGpush(instructionPointer * ip)
{
	char dst = PopStackSpec(ip);
	char src = PopStackSpec(ip);
	if (src == 0 || dst == 0) {
		ipReverse(ip);
	} else {
		fingerprintOpcode op = OpcodeStackPop(ip, src);
		if (op == NULL) {
			op = &DoReflect;
		} else {
			if (!OpcodeStackAdd(ip, src, op)) {
				ipReverse(ip);
				return;
			}
		}
		if (!OpcodeStackAdd(ip, dst, op))
			ipReverse(ip);
	}
}

bool FingerFINGload(instructionPointer * ip)
{
	ManagerAddOpcode(FING,  'X', swap)
	ManagerAddOpcode(FING,  'Y', drop)
	ManagerAddOpcode(FING,  'Z', push)
	return true;
}

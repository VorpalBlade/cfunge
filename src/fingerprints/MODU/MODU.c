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

#include "MODU.h"
#include "../../stack.h"

// This was more or less taken from CCBI
FUNGE_ATTR_CONST FUNGE_ATTR_FAST
static inline funge_cell floordiv(funge_cell x, funge_cell y)
{
	x /= y;
	if (x < 0)
		return x - 1;
	else
		return x;
}

static void finger_MODU_signed_result(instructionPointer * ip)
{
	funge_cell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		stack_push(ip->stack, x - floordiv(x, y) * y);
	} else {
		stack_push(ip->stack, 0);
	}
}

static void finger_MODU_unsigned_result(instructionPointer * ip)
{
	funge_cell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		stack_push(ip->stack, ABS(x % y));
	} else {
		stack_push(ip->stack, 0);
	}
}

// C style reminder.
static void finger_MODU_remainder(instructionPointer * ip)
{
	funge_cell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (y) {
		// Well that's easy, this *is* C.
		stack_push(ip->stack, x % y);
	} else {
		stack_push(ip->stack, 0);
	}
}


bool finger_MODU_load(instructionPointer * ip)
{
	manager_add_opcode(MODU, 'M', signed_result)
	manager_add_opcode(MODU, 'R', remainder)
	manager_add_opcode(MODU, 'U', unsigned_result)
	return true;
}

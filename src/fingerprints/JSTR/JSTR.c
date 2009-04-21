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

#include "JSTR.h"
#include "../../stack.h"

/// G - Read with delta
static void finger_JSTR_push_n(instructionPointer * ip)
{
	funge_cell n;
	funge_vector pos, delta;

	n     = stack_pop(ip->stack);
	pos   = stack_pop_vector(ip->stack);
	delta = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;

	if (n <= 0) {
		ip_reverse(ip);
		return;
	}

	stack_push(ip->stack, 0);

	while (n--) {
		stack_push(ip->stack, fungespace_get(&pos));
		pos.x += delta.x;
		pos.y += delta.y;
	}
}

/// P - Write with delta
static void finger_JSTR_pop_n(instructionPointer * ip)
{
	funge_cell n;
	funge_vector pos, delta;

	n     = stack_pop(ip->stack);
	pos   = stack_pop_vector(ip->stack);
	delta = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;

	if (n <= 0) {
		ip_reverse(ip);
		return;
	}

	while (n--) {
		fungespace_set(stack_pop(ip->stack), &pos);
		pos.x += delta.x;
		pos.y += delta.y;
	}
}

bool finger_JSTR_load(instructionPointer * ip)
{
	manager_add_opcode(JSTR, 'G', push_n)
	manager_add_opcode(JSTR, 'P', pop_n)
	return true;
}

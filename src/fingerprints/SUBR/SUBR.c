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

#include "SUBR.h"
#include "../../stack.h"
#include "../../vector.h"

#define ALLOCCHUNK 8

static const funge_vector SUBRnewDelta = { .x = 1, .y = 0 };

/// A - Change to absolute addressing
static void finger_SUBR_absolute(instructionPointer * ip)
{
	ip->fingerSUBRisRelative = false;
}

/// C - Call
static void finger_SUBR_call(instructionPointer * ip)
{
	funge_cell n;
	funge_vector pos;
	funge_stack *tmpstack;

	n = stack_pop(ip->stack);
	if (n < 0) {
		ip_reverse(ip);
		return;
	}
	// Pop vector
	pos = stack_pop_vector(ip->stack);
	// Stupid to change a fingerprint after it is published.
	if (ip->fingerSUBRisRelative) {
		pos.x += ip->storageOffset.x;
		pos.y += ip->storageOffset.y;
	}

	// FIXME: Use a faster bulk copy for stack below.
	tmpstack = stack_create();

	for (funge_cell i = 0; i < n; ++i)
		stack_push(tmpstack, stack_pop(ip->stack));

	stack_push_vector(ip->stack, &ip->position);
	stack_push_vector(ip->stack, &ip->delta);
	while (n--)
		stack_push(ip->stack, stack_pop(tmpstack));
	stack_free(tmpstack);

	ip_set_position(ip, &pos);
	ip_set_delta(ip, &SUBRnewDelta);
	ip->needMove = false;
}

/// J - Jump
static void finger_SUBR_jump(instructionPointer * ip)
{
	funge_vector pos;

	pos = stack_pop_vector(ip->stack);
	// Stupid to change a fingerprint after it is published.
	if (ip->fingerSUBRisRelative) {
		pos.x += ip->storageOffset.x;
		pos.y += ip->storageOffset.y;
	}

	ip_set_position(ip, &pos);
	ip_set_delta(ip, &SUBRnewDelta);
}

/// O - Change to relative addressing
static void finger_SUBR_relative(instructionPointer * ip)
{
	ip->fingerSUBRisRelative = true;
}

/// R - Return from call
static void finger_SUBR_return(instructionPointer * ip)
{
	funge_cell n;
	funge_vector pos;
	funge_vector vec;
	funge_stack *tmpstack;

	n = stack_pop(ip->stack);
	if (n < 0) {
		ip_reverse(ip);
		return;
	}

	// FIXME: Use a faster bulk copy for stack below.
	tmpstack = stack_create();

	for (funge_cell i = 0; i < n; ++i)
		stack_push(tmpstack, stack_pop(ip->stack));

	vec = stack_pop_vector(ip->stack);
	pos = stack_pop_vector(ip->stack);
	ip_set_position(ip, &pos);
	ip_set_delta(ip, &vec);

	while (n--)
		stack_push(ip->stack, stack_pop(tmpstack));

	stack_free(tmpstack);
}


bool finger_SUBR_load(instructionPointer * ip)
{
	manager_add_opcode(SUBR, 'A', absolute)
	manager_add_opcode(SUBR, 'C', call)
	manager_add_opcode(SUBR, 'J', jump)
	manager_add_opcode(SUBR, 'O', relative)
	// No not a keyword in this case
	manager_add_opcode(SUBR, 'R', return)
	return true;
}

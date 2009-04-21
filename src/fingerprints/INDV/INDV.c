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

#include "INDV.h"
#include "../../stack.h"

/// This simply fetches the second vector.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline funge_vector get_second_vector(instructionPointer * restrict ip)
{
	funge_vector a, b;

	a = stack_pop_vector(ip->stack);
	// Add first level of storage offset...
	a.x += ip->storageOffset.x;
	a.y += ip->storageOffset.y;
	b.x = fungespace_get(vector_create_ref(a.x + 1, a.y));
	b.y = fungespace_get(&a);
	// Add in second level of storage offset...
	b.x += ip->storageOffset.x;
	b.y += ip->storageOffset.y;
	return b;
}

/// G - Get value using indirect vector
static void finger_INDV_get_num(instructionPointer * ip)
{
	funge_vector v = get_second_vector(ip);
	stack_push(ip->stack, fungespace_get(&v));
}

/// P - Put value using indirect vector
static void finger_INDV_put_num(instructionPointer * ip)
{
	funge_vector v = get_second_vector(ip);
	fungespace_set(stack_pop(ip->stack), &v);
}

/// V - Get vector using indirect vector
static void finger_INDV_get_vec(instructionPointer * ip)
{
	funge_vector v = get_second_vector(ip);
	stack_push_vector(ip->stack,
	                  vector_create_ref(
	                      fungespace_get(vector_create_ref(v.x + 1, v.y)),
	                      fungespace_get(&v)
	                  )
	                 );
}

/// W - Write vector using indirect vector
static void finger_INDV_put_vec(instructionPointer * ip)
{
	funge_vector a, b;
	a = get_second_vector(ip);
	b = stack_pop_vector(ip->stack);
	fungespace_set(b.y, &a);
	fungespace_set(b.x, vector_create_ref(a.x + 1, a.y));
}

bool finger_INDV_load(instructionPointer * ip)
{
	manager_add_opcode(INDV, 'G', get_num)
	manager_add_opcode(INDV, 'P', put_num)
	manager_add_opcode(INDV, 'V', get_vec)
	manager_add_opcode(INDV, 'W', put_vec)
	return true;
}

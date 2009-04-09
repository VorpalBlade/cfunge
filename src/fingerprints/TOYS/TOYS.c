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

#include "TOYS.h"
#include "../../stack.h"
#include "../../ip.h"
#include "../../rect.h"
#include "../../interpreter.h"
#include "../../funge-space/funge-space.h"

#include <stdlib.h> /* random */

// Note: Function names are related to intercal names, see
// http://catseye.tc/projects/funge98/library/TOYS.html

// Also: code based on CCBI.

/// A - gable (Duplicate stack top n times)
static void finger_TOYS_gable(instructionPointer * ip)
{
	funge_cell n, c;

	n = stack_pop(ip->stack);
	c = stack_pop(ip->stack);

	if (n < 0) {
		ip_reverse(ip);
		return;
	}

	while (n--)
		stack_push(ip->stack, c);
}

/// B - pair of shoes (Butterfly operation)
static void finger_TOYS_pair_of_shoes(instructionPointer * ip)
{
	// Got no idea if this is correct.
	funge_cell x, y;

	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);

	stack_push(ip->stack, x + y);
	stack_push(ip->stack, x - y);

}

/// C - bracelet (Low order copy)
static void finger_TOYS_bracelet(instructionPointer * ip)
{
	funge_vector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (funge_cell y = 0; y < d.y; ++y)
		for (funge_cell x = 0; x < d.x; ++x)
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                      vector_create_ref(x, y), &t);
}

/// D - toilet seat (Decrement top of stack)
static void finger_TOYS_toilet_seat(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) - 1);
}

/// E - pitchfork head (Replace stack with sum of all items on stack)
static void finger_TOYS_pitchfork_head(instructionPointer * ip)
{
	funge_cell sum = 0;
	for (size_t i = 0; i < ip->stack->top; i++)
		sum += ip->stack->entries[i];
	stack_clear(ip->stack);
	stack_push(ip->stack, sum);
}

/// F - calipers (Write matrix to funge space from stack)
static void finger_TOYS_calipers(instructionPointer * ip)
{
	funge_vector t;
	funge_cell i, j;

	t = stack_pop_vector(ip->stack);

	// j's location not in spec...
	j = stack_pop(ip->stack);
	i = stack_pop(ip->stack);

	for (funge_cell y = t.y; y < t.y + j; ++y)
		for (funge_cell x = t.x; x < t.x + i; ++x)
			fungespace_set(stack_pop(ip->stack), vector_create_ref(x, y));
}

/// G - counterclockwise (Read matrix from funge space onto stack)
static void finger_TOYS_counterclockwise(instructionPointer * ip)
{
	funge_vector o;
	funge_cell i, j;

	o = stack_pop_vector(ip->stack);

	// j's location not in spec...
	j = stack_pop(ip->stack);
	i = stack_pop(ip->stack);

	for (funge_cell y = o.y + j; y-- > o.y;)
		for (funge_cell x = o.x + i; x-- > o.x;)
			stack_push(ip->stack, fungespace_get(vector_create_ref(x, y)));
}

/// H - pair of stilts (Bitshift)
static void finger_TOYS_pair_of_stilts(instructionPointer * ip)
{
	funge_cell a, b;

	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	if (b < 0)
		stack_push(ip->stack, a >> (-b));
	else
		stack_push(ip->stack, a << b);
}

/// I - doric column (Increment top of stack)
static void finger_TOYS_doric_column(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) + 1);
}

/// J - fishhook (Translate current funge space column)
static void finger_TOYS_fishhook(instructionPointer * ip)
{
	fungeRect bounds;
	funge_cell n = stack_pop(ip->stack);

	fungespace_get_bounds_rect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (funge_cell y = bounds.y; y <= (bounds.y + bounds.h); ++y)
			fungespace_set(fungespace_get(vector_create_ref(ip->position.x, y)), vector_create_ref(ip->position.x, y + n));
	} else if (n > 0) {
		for (funge_cell y = (bounds.y + bounds.h); y >= bounds.y; --y)
			fungespace_set(fungespace_get(vector_create_ref(ip->position.x, y)), vector_create_ref(ip->position.x, y + n));
	}
}

/// K - scissors (High order copy)
static void finger_TOYS_scissors(instructionPointer * ip)
{
	funge_vector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (funge_cell y = d.y; y-- > 0;)
		for (funge_cell x = d.x; x-- > 0;)
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                      vector_create_ref(x, y), &t);
}

/// L - corner (Like ' but picks up cell to left and doesn't skip)
static void finger_TOYS_corner(instructionPointer * ip)
{
	ip_turn_left(ip);
	ip_forward(ip);
	stack_push(ip->stack, fungespace_get(&ip->position));
	ip_backward(ip);
	ip_turn_right(ip);
}

/// M - kittycat (Low order move)
static void finger_TOYS_kittycat(instructionPointer * ip)
{
	funge_vector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (funge_cell y = 0; y < d.y; ++y)
		for (funge_cell x = 0; x < d.x; ++x) {
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                      vector_create_ref(x, y), &t);
			fungespace_set_offset(' ', vector_create_ref(x, y), &o);
		}
}

/// N - lightning bolt (Negate top of stack)
static void finger_TOYS_lightning_bolt(instructionPointer * ip)
{
	stack_push(ip->stack, -stack_pop(ip->stack));
}

/// O - boulder (Translate current funge space row)
static void finger_TOYS_boulder(instructionPointer * ip)
{
	fungeRect bounds;
	funge_cell n = stack_pop(ip->stack);

	fungespace_get_bounds_rect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (funge_cell x = bounds.x; x <= (bounds.x + bounds.w); ++x)
			fungespace_set(fungespace_get(vector_create_ref(x, ip->position.y)), vector_create_ref(x + n, ip->position.y));
	} else if (n > 0) {
		for (funge_cell x = (bounds.x + bounds.w); x >= bounds.x; --x)
			fungespace_set(fungespace_get(vector_create_ref(x, ip->position.y)), vector_create_ref(x + n, ip->position.y));
	}
}

/// P - mailbox (Replace stack with product of all items on stack)
static void finger_TOYS_mailbox(instructionPointer * ip)
{
	funge_cell product = 1;
	for (size_t i = 0; i < ip->stack->top; i++)
		product *= ip->stack->entries[i];
	stack_clear(ip->stack);
	stack_push(ip->stack, product);
}

/// Q - necklace (Write behind IP)
static void finger_TOYS_necklace(instructionPointer * ip)
{
	funge_cell v = stack_pop(ip->stack);

	ip_backward(ip);
	fungespace_set(v, &ip->position);
	ip_forward(ip);
}

/// R - can opener (Like L but to the right)
static void finger_TOYS_can_opener(instructionPointer * ip)
{
	ip_turn_right(ip);
	ip_forward(ip);
	stack_push(ip->stack, fungespace_get(&ip->position));
	ip_backward(ip);
	ip_turn_left(ip);
}

/// S - chicane (memset on funge space)
static void finger_TOYS_chicane(instructionPointer * ip)
{
	funge_vector d, o;
	funge_cell c;
	o = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	c = stack_pop(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (funge_cell y = o.y; y < o.y + d.y; ++y)
		for (funge_cell x = o.x; x < o.x + d.x; ++x)
			fungespace_set(c, vector_create_ref(x, y));
}

/// T - barstool (Act like _ or | depending on popped number)
static void finger_TOYS_barstool(instructionPointer * ip)
{
	switch (stack_pop(ip->stack)) {
		case 0: if_east_west(ip); break;
		case 1: if_north_south(ip); break;
		default: ip_reverse(ip); break;
	}
}

/// U - tumbler (Like ? but replaces instruction with said random choice)
static void finger_TOYS_tumbler(instructionPointer * ip)
{
	long int rnd = random() % 4;
	switch (rnd) {
		case 0: fungespace_set('^', &ip->position); ip_go_north(ip); break;
		case 1: fungespace_set('>', &ip->position); ip_go_east(ip); break;
		case 2: fungespace_set('v', &ip->position); ip_go_south(ip); break;
		case 3: fungespace_set('<', &ip->position); ip_go_west(ip); break;
	}
}

/// V - dixiecup (High order move)
static void finger_TOYS_dixiecup(instructionPointer * ip)
{
	funge_vector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (funge_cell y = d.y; y-- > 0;)
		for (funge_cell x = d.x; x-- > 0;) {
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                      vector_create_ref(x, y), &t);
			fungespace_set_offset(' ', vector_create_ref(x, y), &o);
		}
}

/// W - television antenna (Atomic g/wait and try again/reverse)
static void finger_TOYS_television_antenna(instructionPointer * ip)
{
	funge_vector vect;
	funge_cell v, c;
	vect = stack_pop_vector(ip->stack);
	v = stack_pop(ip->stack);
	c = fungespace_get(&vect);

	if (c < v) {
		stack_push(ip->stack, v);
		stack_push_vector(ip->stack, vector_create_ref(vect.x - ip->storageOffset.x, vect.y - ip->storageOffset.y));
		ip_backward(ip);
	} else if (c > v)
		ip_reverse(ip);
}

/// X - buried treasure (Increment IP's x coord)
static void finger_TOYS_buried_treasure(instructionPointer * ip)
{
	ip->position.x++;
}

/// Y - slingshot (Increment IP's y coord)
static void finger_TOYS_slingshot(instructionPointer * ip)
{
	ip->position.y++;
}

/// Z - barn door (Increment IP's z coord)
static void finger_TOYS_barn_door(instructionPointer * ip)
{
	// As this needs trefunge to work.
	ip_reverse(ip);
}

bool finger_TOYS_load(instructionPointer * ip)
{
	manager_add_opcode(TOYS, 'A', gable)
	manager_add_opcode(TOYS, 'B', pair_of_shoes)
	manager_add_opcode(TOYS, 'C', bracelet)
	manager_add_opcode(TOYS, 'D', toilet_seat)
	manager_add_opcode(TOYS, 'E', pitchfork_head)
	manager_add_opcode(TOYS, 'F', calipers)
	manager_add_opcode(TOYS, 'G', counterclockwise)
	manager_add_opcode(TOYS, 'H', pair_of_stilts)
	manager_add_opcode(TOYS, 'I', doric_column)
	manager_add_opcode(TOYS, 'J', fishhook)
	manager_add_opcode(TOYS, 'K', scissors)
	manager_add_opcode(TOYS, 'L', corner)
	manager_add_opcode(TOYS, 'M', kittycat)
	manager_add_opcode(TOYS, 'N', lightning_bolt)
	manager_add_opcode(TOYS, 'O', boulder)
	manager_add_opcode(TOYS, 'P', mailbox)
	manager_add_opcode(TOYS, 'Q', necklace)
	manager_add_opcode(TOYS, 'R', can_opener)
	manager_add_opcode(TOYS, 'S', chicane)
	manager_add_opcode(TOYS, 'T', barstool)
	manager_add_opcode(TOYS, 'U', tumbler)
	manager_add_opcode(TOYS, 'V', dixiecup)
	manager_add_opcode(TOYS, 'W', television_antenna)
	manager_add_opcode(TOYS, 'X', buried_treasure)
	manager_add_opcode(TOYS, 'Y', slingshot)
	manager_add_opcode(TOYS, 'Z', barn_door)
	return true;
}

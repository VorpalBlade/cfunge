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

#include "TOYS.h"
#include "../../stack.h"
#include "../../ip.h"
#include "../../rect.h"
#include "../../interpreter.h"
#include "../../funge-space/funge-space.h"

#include <assert.h>

// Note: Function names are related to intercal names, see
// http://catseye.tc/projects/funge98/library/TOYS.html

// Also: code based on CCBI.

/// A - gable
static void finger_TOYS_gable(instructionPointer * ip)
{
	fungeCell n, c;

	n = stack_pop(ip->stack);
	c = stack_pop(ip->stack);

	if (n < 0) {
		ip_reverse(ip);
		return;
	}

	while (n--)
		stack_push(ip->stack, c);
}

/// B - pair of shoes
static void finger_TOYS_pairOfShoes(instructionPointer * ip)
{
	// Got no idea if this is correct.
	fungeCell x, y;

	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);

	stack_push(ip->stack, x + y);
	stack_push(ip->stack, x - y);

}

/// C - bracelet
static void finger_TOYS_bracelet(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (fungeCell x = 0; x < d.x; ++x)
		for (fungeCell y = 0; y < d.y; ++y)
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                 vector_create_ref(x, y), &t);
}

/// D - toilet seat
static void finger_TOYS_toiletSeat(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) - 1);
}

/// E - pitchfork head
static void finger_TOYS_pitchforkHead(instructionPointer * ip)
{
	fungeCell sum = 0;
	for (size_t i = ip->stack->top; i-- > 0;)
		sum += ip->stack->entries[i];
	stack_clear(ip->stack);
	stack_push(ip->stack, sum);
}

/// F - calipers
static void finger_TOYS_calipers(instructionPointer * ip)
{
	fungeVector t;
	fungeCell i, j;

	t = stack_pop_vector(ip->stack);

	// j's location not in spec...
	j = stack_pop(ip->stack);
	i = stack_pop(ip->stack);

	for (fungeCell y = t.y; y < t.y + j; ++y)
		for (fungeCell x = t.x; x < t.x + i; ++x)
			fungespace_set(stack_pop(ip->stack), vector_create_ref(x, y));
}

/// G - counterclockwise
static void finger_TOYS_counterclockwise(instructionPointer * ip)
{
	fungeVector o;
	fungeCell i, j;

	o = stack_pop_vector(ip->stack);

	// j's location not in spec...
	j = stack_pop(ip->stack);
	i = stack_pop(ip->stack);

	for (fungeCell y = o.y + j; y-- > o.y;)
		for (fungeCell x = o.x + i; x-- > o.x;)
			stack_push(ip->stack, fungespace_get(vector_create_ref(x, y)));
}

/// H - pair of stilts
static void finger_TOYS_pairOfStilts(instructionPointer * ip)
{
	fungeCell a, b;

	b = stack_pop(ip->stack);
	a = stack_pop(ip->stack);

	if (b < 0)
		stack_push(ip->stack, a >> (-b));
	else
		stack_push(ip->stack, a << b);
}

/// I - doric column
static void finger_TOYS_doricColumn(instructionPointer * ip)
{
	stack_push(ip->stack, stack_pop(ip->stack) + 1);
}

/// J - fishhook
static void finger_TOYS_fishhook(instructionPointer * ip)
{
	fungeRect bounds;
	fungeCell n = stack_pop(ip->stack);

	fungespace_get_bounds_rect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (fungeCell y = bounds.y; y <= (bounds.y + bounds.h); ++y)
			fungespace_set(fungespace_get(vector_create_ref(ip->position.x, y)), vector_create_ref(ip->position.x, y + n));

	} else if (n > 0) {
		for (fungeCell y = (bounds.y + bounds.h); y >= bounds.y; --y)
			fungespace_set(fungespace_get(vector_create_ref(ip->position.x, y)), vector_create_ref(ip->position.x, y + n));
	}
}

/// K - scissors
static void finger_TOYS_scissors(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (fungeCell x = d.x; x-- > 0;)
		for (fungeCell y = d.y; y-- > 0;)
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                 vector_create_ref(x, y), &t);
}

/// L - corner
static void finger_TOYS_corner(instructionPointer * ip)
{
	ip_turn_left(ip);
	ip_forward(ip, 1);
	stack_push(ip->stack, fungespace_get(&ip->position));
	ip_forward(ip, -1);
	ip_turn_right(ip);
}

/// M - kittycat
static void finger_TOYS_kittycat(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (fungeCell x = 0; x < d.x; ++x)
		for (fungeCell y = 0; y < d.y; ++y) {
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                 vector_create_ref(x, y), &t);
			fungespace_set_offset(' ', vector_create_ref(x, y), &o);
		}
}

/// N - lightning bolt
static void finger_TOYS_lightningBolt(instructionPointer * ip)
{
	stack_push(ip->stack, -stack_pop(ip->stack));
}

/// O - boulder
static void finger_TOYS_boulder(instructionPointer * ip)
{
	fungeRect bounds;
	fungeCell n = stack_pop(ip->stack);

	fungespace_get_bounds_rect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (fungeCell x = bounds.x; x <= (bounds.x + bounds.w); ++x)
			fungespace_set(fungespace_get(vector_create_ref(x, ip->position.y)), vector_create_ref(x + n, ip->position.y));
	} else if (n > 0) {
		for (fungeCell x = (bounds.x + bounds.w); x >= bounds.x; --x)
			fungespace_set(fungespace_get(vector_create_ref(x, ip->position.y)), vector_create_ref(x + n, ip->position.y));
	}
}

/// P - mailbox
static void finger_TOYS_mailbox(instructionPointer * ip)
{
	fungeCell product = 1;
	for (size_t i = ip->stack->top; i-- > 0;)
		product *= ip->stack->entries[i];
	stack_clear(ip->stack);
	stack_push(ip->stack, product);
}

/// Q - necklace
static void finger_TOYS_necklace(instructionPointer * ip)
{
	fungeCell v = stack_pop(ip->stack);

	ip_forward(ip, -1);
	fungespace_set(v, &ip->position);
	ip_forward(ip, 1);
}

/// R - can opener
static void finger_TOYS_canOpener(instructionPointer * ip)
{
	ip_turn_right(ip);
	ip_forward(ip, 1);
	stack_push(ip->stack, fungespace_get(&ip->position));
	ip_forward(ip, -1);
	ip_turn_left(ip);
}

/// S - chicane
static void finger_TOYS_chicane(instructionPointer * ip)
{
	fungeVector d, o;
	fungeCell c;
	o = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	c = stack_pop(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (fungeCell x = o.x; x < o.x + d.x; ++x)
		for (fungeCell y = o.y; y < o.y + d.y; ++y)
			fungespace_set(c, vector_create_ref(x, y));
}

/// T - barstool
static void finger_TOYS_barstool(instructionPointer * ip)
{
	switch (stack_pop(ip->stack)) {
		case 0: if_east_west(ip); break;
		case 1: if_north_south(ip); break;
		default: ip_reverse(ip); break;
	}
}

/// U - tumbler
static void finger_TOYS_tumbler(instructionPointer * ip)
{
	long int rnd = random() % 4;
	assert((rnd >= 0) && (rnd <= 3));
	switch (rnd) {
		case 0: fungespace_set('^', &ip->position); ip_go_north(ip); break;
		case 1: fungespace_set('>', &ip->position); ip_go_east(ip); break;
		case 2: fungespace_set('v', &ip->position); ip_go_south(ip); break;
		case 3: fungespace_set('<', &ip->position); ip_go_west(ip); break;
	}
}

/// V - dixiecup
static void finger_TOYS_dixiecup(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = stack_pop_vector(ip->stack);
	d = stack_pop_vector(ip->stack);
	o = stack_pop_vector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ip_reverse(ip);
		return;
	}

	for (fungeCell x = d.x; x-- > 0;)
		for (fungeCell y = d.y; y-- > 0;) {
			fungespace_set_offset(fungespace_get_offset(vector_create_ref(x, y), &o),
			                 vector_create_ref(x, y), &t);
			fungespace_set_offset(' ', vector_create_ref(x, y), &o);
		}
}

/// W - television antenna
static void finger_TOYS_televisionAntenna(instructionPointer * ip)
{
	fungeVector vect;
	fungeCell v, c;
	vect = stack_pop_vector(ip->stack);
	v = stack_pop(ip->stack);
	c = fungespace_get(&vect);

	if (c < v) {
		stack_push(ip->stack, v);
		stack_push_vector(ip->stack, vector_create_ref(vect.x - ip->storageOffset.x, vect.y - ip->storageOffset.y));
		ip_forward(ip, -1);
	} else if (c > v)
		ip_reverse(ip);
}

/// X - buried treasure
static void finger_TOYS_buriedTreasure(instructionPointer * ip)
{
	ip->position.x++;
}

/// Y - slingshot
static void finger_TOYS_slingshot(instructionPointer * ip)
{
	ip->position.y++;
}

/// Z - barn door
static void finger_TOYS_barnDoor(instructionPointer * ip)
{
	// As this needs trefunge to work.
	ip_reverse(ip);
}

bool finger_TOYS_load(instructionPointer * ip)
{
	manager_add_opcode(TOYS, 'A', gable)
	manager_add_opcode(TOYS, 'B', pairOfShoes)
	manager_add_opcode(TOYS, 'C', bracelet)
	manager_add_opcode(TOYS, 'D', toiletSeat)
	manager_add_opcode(TOYS, 'E', pitchforkHead)
	manager_add_opcode(TOYS, 'F', calipers)
	manager_add_opcode(TOYS, 'G', counterclockwise)
	manager_add_opcode(TOYS, 'H', pairOfStilts)
	manager_add_opcode(TOYS, 'I', doricColumn)
	manager_add_opcode(TOYS, 'J', fishhook)
	manager_add_opcode(TOYS, 'K', scissors)
	manager_add_opcode(TOYS, 'L', corner)
	manager_add_opcode(TOYS, 'M', kittycat)
	manager_add_opcode(TOYS, 'N', lightningBolt)
	manager_add_opcode(TOYS, 'O', boulder)
	manager_add_opcode(TOYS, 'P', mailbox)
	manager_add_opcode(TOYS, 'Q', necklace)
	manager_add_opcode(TOYS, 'R', canOpener)
	manager_add_opcode(TOYS, 'S', chicane)
	manager_add_opcode(TOYS, 'T', barstool)
	manager_add_opcode(TOYS, 'U', tumbler)
	manager_add_opcode(TOYS, 'V', dixiecup)
	manager_add_opcode(TOYS, 'W', televisionAntenna)
	manager_add_opcode(TOYS, 'X', buriedTreasure)
	manager_add_opcode(TOYS, 'Y', slingshot)
	manager_add_opcode(TOYS, 'Z', barnDoor)
	return true;
}

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

// A - gable
static void FingerTOYSgable(instructionPointer * ip)
{
	FUNGEDATATYPE n, c;

	n = StackPop(ip->stack);
	c = StackPop(ip->stack);

	if (n < 0) {
		ipReverse(ip);
		return;
	}

	while(n--)
		StackPush(c, ip->stack);
}

// B - pair of shoes
static void FingerTOYSpairOfShoes(instructionPointer * ip)
{
	// Got no idea if this is correct.
	FUNGEDATATYPE x, y;

	y = StackPop(ip->stack);
	x = StackPop(ip->stack);

	StackPush(x+y, ip->stack);
	StackPush(x-y, ip->stack);

}

// C - bracelet
static void FingerTOYSbracelet(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = StackPopVector(ip->stack);
	d = StackPopVector(ip->stack);
	o = StackPopVector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ipReverse(ip);
		return;
	}

	for (FUNGEDATATYPE x = 0; x < d.x; ++x)
		for (FUNGEDATATYPE y = 0; y < d.y; ++y)
			FungeSpaceSetOff(FungeSpaceGetOff(VectorCreateRef(x, y), &o),
			                 VectorCreateRef(x, y), &t);
}

// D - toilet seat
static void FingerTOYStoiletSeat(instructionPointer * ip)
{
	StackPush(StackPop(ip->stack) - 1, ip->stack);
}

// E - pitchfork head
static void FingerTOYSpitchforkHead(instructionPointer * ip)
{
	FUNGEDATATYPE sum = 0;
	for(size_t i = ip->stack->top; i-- > 0;)
		sum += ip->stack->entries[i];
	StackClear(ip->stack);
	StackPush(sum, ip->stack);
}

// F - calipers
static void FingerTOYScalipers(instructionPointer * ip)
{
	fungeVector t;
	FUNGEDATATYPE i, j;

	t = StackPopVector(ip->stack);

	// j's location not in spec...
	j = StackPop(ip->stack);
	i = StackPop(ip->stack);

	for (FUNGEDATATYPE y = t.y; y < t.y + j; ++y)
		for (FUNGEDATATYPE x = t.x; x < t.x + i; ++x)
			FungeSpaceSet(StackPop(ip->stack), VectorCreateRef(x, y));
}

// G - counterclockwise
static void FingerTOYScounterclockwise(instructionPointer * ip)
{
	fungeVector o;
	FUNGEDATATYPE i, j;

	o = StackPopVector(ip->stack);

	// j's location not in spec...
	j = StackPop(ip->stack);
	i = StackPop(ip->stack);

	for (FUNGEDATATYPE y = o.y + j; y-- > o.y;)
		for (FUNGEDATATYPE x = o.x + i; x-- > o.x;)
			StackPush(FungeSpaceGet(VectorCreateRef(x, y)), ip->stack);
}

// H - pair of stilts
static void FingerTOYSpairOfStilts(instructionPointer * ip)
{
	FUNGEDATATYPE a, b;

	b = StackPop(ip->stack);
	a = StackPop(ip->stack);

	if (b < 0)
		StackPush(a >> (-b), ip->stack);
	else
		StackPush(a << b, ip->stack);
}

// I - doric column
static void FingerTOYSdoricColumn(instructionPointer * ip)
{
	StackPush(StackPop(ip->stack) + 1, ip->stack);
}

// J - fishhook
static void FingerTOYSfishhook(instructionPointer * ip)
{
	fungeRect bounds;
	FUNGEDATATYPE n = StackPop(ip->stack);

	FungeSpaceGetBoundRect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (FUNGEDATATYPE y = bounds.y; y <= (bounds.y + bounds.h); ++y)
			FungeSpaceSet(FungeSpaceGet(VectorCreateRef(ip->position.x, y)), VectorCreateRef(ip->position.x, y+n));

	} else if (n > 0) {
		for (FUNGEDATATYPE y = (bounds.y + bounds.h); y >= bounds.y; --y)
			FungeSpaceSet(FungeSpaceGet(VectorCreateRef(ip->position.x, y)), VectorCreateRef(ip->position.x, y+n));
	}
}

// K - scissors
static void FingerTOYSscissors(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = StackPopVector(ip->stack);
	d = StackPopVector(ip->stack);
	o = StackPopVector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ipReverse(ip);
		return;
	}

	for (FUNGEDATATYPE x = d.x; x-- > 0;)
		for (FUNGEDATATYPE y = d.y; y-- > 0;)
			FungeSpaceSetOff(FungeSpaceGetOff(VectorCreateRef(x, y), &o),
			                 VectorCreateRef(x, y), &t);
}

// L - corner
static void FingerTOYScorner(instructionPointer * ip)
{
	ipTurnLeft(ip);
	ipForward(1, ip);
	StackPush(FungeSpaceGet(&ip->position), ip->stack);
	ipForward(-1, ip);
	ipTurnRight(ip);
}

// M - kittycat
static void FingerTOYSkittycat(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = StackPopVector(ip->stack);
	d = StackPopVector(ip->stack);
	o = StackPopVector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ipReverse(ip);
		return;
	}

	for (FUNGEDATATYPE x = 0; x < d.x; ++x)
		for (FUNGEDATATYPE y = 0; y < d.y; ++y) {
			FungeSpaceSetOff(FungeSpaceGetOff(VectorCreateRef(x, y), &o),
			                 VectorCreateRef(x, y), &t);
			FungeSpaceSetOff(' ', VectorCreateRef(x, y), &o);
		}
}

// N - lightning bolt
static void FingerTOYSlightningBolt(instructionPointer * ip)
{
	StackPush(-StackPop(ip->stack), ip->stack);
}

// O - boulder
static void FingerTOYSboulder(instructionPointer * ip)
{
	fungeRect bounds;
	FUNGEDATATYPE n = StackPop(ip->stack);

	FungeSpaceGetBoundRect(&bounds);

	if (!n)
		return;
	else if (n < 0) {
		for (FUNGEDATATYPE x = bounds.x; x <= (bounds.x + bounds.w); ++x)
			FungeSpaceSet(FungeSpaceGet(VectorCreateRef(x, ip->position.y)), VectorCreateRef(x+n, ip->position.y));
	} else if (n > 0) {
		for (FUNGEDATATYPE x = (bounds.x + bounds.w); x >= bounds.x; --x)
			FungeSpaceSet(FungeSpaceGet(VectorCreateRef(x, ip->position.y)), VectorCreateRef(x+n, ip->position.y));
	}
}

// P - mailbox
static void FingerTOYSmailbox(instructionPointer * ip)
{
	FUNGEDATATYPE product = 1;
	for(size_t i = ip->stack->top; i-- > 0;)
		product *= ip->stack->entries[i];
	StackClear(ip->stack);
	StackPush(product, ip->stack);
}

// Q - necklace
static void FingerTOYSnecklace(instructionPointer * ip)
{
	FUNGEDATATYPE v = StackPop(ip->stack);

	ipForward(-1, ip);
	FungeSpaceSet(v, &ip->position);
	ipForward(1, ip);
}

// R - can opener
static void FingerTOYScanOpener(instructionPointer * ip)
{
	ipTurnRight(ip);
	ipForward(1, ip);
	StackPush(FungeSpaceGet(&ip->position), ip->stack);
	ipForward(-1, ip);
	ipTurnLeft(ip);
}

// S - chicane
static void FingerTOYSchicane(instructionPointer * ip)
{
	fungeVector d, o;
	FUNGEDATATYPE c;
	o = StackPopVector(ip->stack);
	d = StackPopVector(ip->stack);
	c = StackPop(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ipReverse(ip);
		return;
	}

	for (FUNGEDATATYPE x = o.x; x < o.x + d.x; ++x)
		for (FUNGEDATATYPE y = o.y; y < o.y + d.y; ++y)
			FungeSpaceSet(c, VectorCreateRef(x, y));
}

// T - barstool
static void FingerTOYSbarstool(instructionPointer * ip)
{
	switch (StackPop(ip->stack)) {
		case 0: IfEastWest(ip); break;
		case 1: IfNorthSouth(ip); break;
		default: ipReverse(ip); break;
	}
}

// U - tumbler
static void FingerTOYStumbler(instructionPointer * ip)
{
	long int rnd = random() % 4;
	assert((rnd >= 0) && (rnd <= 3));
	switch (rnd) {
		case 0: FungeSpaceSet('^', &ip->position); ipGoNorth(ip); break;
		case 1: FungeSpaceSet('>', &ip->position); ipGoEast(ip); break;
		case 2: FungeSpaceSet('v', &ip->position); ipGoSouth(ip); break;
		case 3: FungeSpaceSet('<', &ip->position); ipGoWest(ip); break;
	}
}

// V - dixiecup
static void FingerTOYSdixiecup(instructionPointer * ip)
{
	fungeVector t, d, o;
	t = StackPopVector(ip->stack);
	d = StackPopVector(ip->stack);
	o = StackPopVector(ip->stack);

	if (!d.x || !d.y)
		return;

	if (d.x < 0 || d.y < 0) {
		ipReverse(ip);
		return;
	}

	for (FUNGEDATATYPE x = d.x; x-- > 0;)
		for (FUNGEDATATYPE y = d.y; y-- > 0;) {
			FungeSpaceSetOff(FungeSpaceGetOff(VectorCreateRef(x, y), &o),
			                 VectorCreateRef(x, y), &t);
			FungeSpaceSetOff(' ', VectorCreateRef(x, y), &o);
		}
}

// W - television antenna
static void FingerTOYStelevisionAntenna(instructionPointer * ip)
{
	fungeVector vect;
	FUNGEDATATYPE v, c;
	vect = StackPopVector(ip->stack);
	v = StackPop(ip->stack);
	c = FungeSpaceGet(&vect);

	if (c < v) {
		StackPush(v, ip->stack);
		StackPushVector(VectorCreateRef(vect.x - ip->storageOffset.x, vect.y - ip->storageOffset.y), ip->stack);
		ipForward(-1, ip);
	} else if (c > v)
		ipReverse(ip);
}

// X - buried treasure
static void FingerTOYSburiedTreasure(instructionPointer * ip)
{
	ip->position.x++;
}

// Y - slingshot
static void FingerTOYSslingshot(instructionPointer * ip)
{
	ip->position.y++;
}

// Z - barn door
static void FingerTOYSbarnDoor(instructionPointer * ip)
{
	// As this needs trefunge to work.
	ipReverse(ip);
}



bool FingerTOYSload(instructionPointer * ip) {
	if (!OpcodeStackAdd(ip, 'A', &FingerTOYSgable))
		return false;
	if (!OpcodeStackAdd(ip, 'B', &FingerTOYSpairOfShoes))
		return false;
	if (!OpcodeStackAdd(ip, 'C', &FingerTOYSbracelet))
		return false;
	if (!OpcodeStackAdd(ip, 'D', &FingerTOYStoiletSeat))
		return false;
	if (!OpcodeStackAdd(ip, 'E', &FingerTOYSpitchforkHead))
		return false;
	if (!OpcodeStackAdd(ip, 'F', &FingerTOYScalipers))
		return false;
	if (!OpcodeStackAdd(ip, 'G', &FingerTOYScounterclockwise))
		return false;
	if (!OpcodeStackAdd(ip, 'H', &FingerTOYSpairOfStilts))
		return false;
	if (!OpcodeStackAdd(ip, 'I', &FingerTOYSdoricColumn))
		return false;
	if (!OpcodeStackAdd(ip, 'J', &FingerTOYSfishhook))
		return false;
	if (!OpcodeStackAdd(ip, 'K', &FingerTOYSscissors))
		return false;
	if (!OpcodeStackAdd(ip, 'L', &FingerTOYScorner))
		return false;
	if (!OpcodeStackAdd(ip, 'M', &FingerTOYSkittycat))
		return false;
	if (!OpcodeStackAdd(ip, 'N', &FingerTOYSlightningBolt))
		return false;
	if (!OpcodeStackAdd(ip, 'O', &FingerTOYSboulder))
		return false;
	if (!OpcodeStackAdd(ip, 'P', &FingerTOYSmailbox))
		return false;
	if (!OpcodeStackAdd(ip, 'Q', &FingerTOYSnecklace))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerTOYScanOpener))
		return false;
	if (!OpcodeStackAdd(ip, 'S', &FingerTOYSchicane))
		return false;
	if (!OpcodeStackAdd(ip, 'T', &FingerTOYSbarstool))
		return false;
	if (!OpcodeStackAdd(ip, 'U', &FingerTOYStumbler))
		return false;
	if (!OpcodeStackAdd(ip, 'V', &FingerTOYSdixiecup))
		return false;
	if (!OpcodeStackAdd(ip, 'W', &FingerTOYStelevisionAntenna))
		return false;
	if (!OpcodeStackAdd(ip, 'X', &FingerTOYSburiedTreasure))
		return false;
	if (!OpcodeStackAdd(ip, 'Y', &FingerTOYSslingshot))
		return false;
	if (!OpcodeStackAdd(ip, 'Z', &FingerTOYSbarnDoor))
		return false;
	return true;
}

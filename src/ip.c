/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "global.h"
#include "ip.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

instructionPointer *
ipCreate(fungeStackStack *stackstack)
{
	instructionPointer * tmp = cf_malloc(sizeof(instructionPointer));
	if (!tmp)
		return NULL;

	tmp->position.x = 0;
	tmp->position.y = 0;
	tmp->mode       = ipmCODE;
	tmp->delta.x    = 1;
	tmp->delta.y    = 0;
	tmp->stackstack = stackstack;
	return tmp;
}

void
ipFree(instructionPointer * ip)
{
	// TODO: Should we free stackstack?
	ip->stackstack = NULL;
	cf_free(ip);
}


void ipForward(int_fast64_t steps, instructionPointer * ip, fungeSpace *space)
{
	ip->position.x += ip->delta.x * steps;
	ip->position.y += ip->delta.y * steps;
	fungeSpaceWrapInPlace(space, &ip->position);
}


void
ipReverse(instructionPointer * ip)
{
	ip->delta.x = -ip->delta.x;
	ip->delta.y = -ip->delta.y;
}

void
ipTurnLeft(instructionPointer * ip)
{
	// TODO
}

void
ipTurnRight(instructionPointer * ip)
{
	// TODO
}

void
ipSetDelta(instructionPointer * ip, const ipDelta * delta)
{
	ip->delta.x = delta->x;
	ip->delta.y = delta->y;
}

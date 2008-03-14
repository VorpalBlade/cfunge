/*
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

#include "global.h"
#include "ip.h"
#include "vector.h"
#include "interpreter.h"
#include "settings.h"
#include "fingerprints/manager.h"
#include "stack.h"
#include "funge-space/funge-space.h"

instructionPointer *
ipCreate(fungeStackStack * stackstack)
{
	instructionPointer * tmp = cf_malloc(sizeof(instructionPointer));
	if (!tmp)
		return NULL;

	tmp->position.x = 0;
	tmp->position.y = 0;
	tmp->mode       = ipmCODE;
	tmp->delta.x    = 1;
	tmp->delta.y    = 0;
	tmp->storageOffset.x = 0;
	tmp->storageOffset.y = 0;
	tmp->stackstack = stackstack;
	tmp->stack      = stackstack->stacks[stackstack->current];
	if (SettingEnableFingerprints)
		ManagerInit(tmp);
	return tmp;
}

void
ipFree(instructionPointer * restrict ip)
{
	// TODO: Should we free stackstack?
	ip->stackstack = NULL;
	ip->stack      = NULL;
	cf_free(ip);
}


void ipForward(int_fast64_t steps, instructionPointer * restrict ip)
{
	ip->position.x += ip->delta.x * steps;
	ip->position.y += ip->delta.y * steps;
	fungeSpaceWrap(fspace, &ip->position, &ip->delta);
}


void
ipReverse(instructionPointer * restrict ip)
{
	ip->delta.x = -ip->delta.x;
	ip->delta.y = -ip->delta.y;
}

void
ipTurnRight(instructionPointer * restrict ip)
{
	FUNGEVECTORTYPE tmpX;
	tmpX        = ip->delta.x;
	ip->delta.x = -ip->delta.y;
	ip->delta.y = tmpX;
}

void
ipTurnLeft(instructionPointer * restrict ip)
{
	FUNGEVECTORTYPE tmpX;
	tmpX        = ip->delta.x;
	ip->delta.x = ip->delta.y;
	ip->delta.y = -tmpX;
}

void
ipSetDelta(instructionPointer * restrict ip, const ipDelta * restrict delta)
{
	ip->delta.x = delta->x;
	ip->delta.y = delta->y;
}

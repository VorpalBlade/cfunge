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

#include "../global.h"
#include "sysinfo.h"
#include "../interpreter.h"
#include "../funge-space/b93/funge-space.h"
#include "../vector.h"
#include "../stack.h"
#include "../ip.h"

// Push a single request value
static void PushRequest(FUNGEDATATYPE request, instructionPointer * ip, fungeSpace *fspace)
{
	switch (request) {
		case 1: // Flags
			//StackPush(0x20, ip->stack);
			StackPush(0x0, ip->stack);
			break;
		case 2: // Cell size
			StackPush(sizeof(FUNGEDATATYPE), ip->stack);
			break;
		case 3: // Handprint
			StackPush(FUNGEHANDPRINT, ip->stack);
			break;
		case 4: // Version
			StackPush(FUNGEVERSION, ip->stack);
			break;
		case 5: // Operating paradigm
			StackPush(0, ip->stack);
			break;
		case 6: // Path separator
			StackPush('/', ip->stack);
			break;
		case 7: // Scalars / vector
			StackPush(2, ip->stack);
			break;
		case 8: // IP ID
			StackPush(0, ip->stack);
			break;
		case 9: // TEAM ID
			StackPush(0, ip->stack);
			break;
		case 10: // Vector of current IP position
			StackPushVector(&ip->position, ip->stack);
			break;
		case 11: // Delta of current IP position
			StackPushVector(&ip->delta, ip->stack);
			break;
		case 12: // Storage offset of current IP position
			StackPushVector(&ip->storageOffset, ip->stack);
			break;
		case 13: // Least point (TODO)
			StackPushVector(& (fungePosition) { .x = 0, .y = 0 }, ip->stack);
			break;
		case 14: // Greatest point (TODO)
			StackPushVector(& (fungePosition) { .x = 0, .y = 0 }, ip->stack);
			break;
		case 15: // Time ((year - 1900) * 256 * 256) + (month * 256) + (day of month) TODO
			StackPush(0, ip->stack);
			break;
		case 16: // Time (hour * 256 * 256) + (minute * 256) + (second) TODO
			StackPush(0, ip->stack);
			break;
		case 17: // Number of stacks on stack stack
			StackPush(ip->stackstack->size, ip->stack);
			break;
		case 18: // Number of elements on all stacks (TODO)
			StackPush(ip->stack->top, ip->stack);
			break;
		case 19: // Command line arguments (TODO)
			StackPush('\0', ip->stack);
			StackPush('\0', ip->stack);
			break;
		case 20: // Environment variables (TODO)
			StackPush('\0', ip->stack);
			StackPush('\0', ip->stack);
			break;
		default:
			ipReverse(ip);
	}
}

#define HIGHESTREQUEST 20

void RunSysInfo(instructionPointer *ip, fungeSpace *fspace)
{
	FUNGEDATATYPE request = StackPop(ip->stack);
	if (request == 23)
		PushRequest(18, ip, fspace);
	else if (request > 0)
		PushRequest(request, ip, fspace);
	else
		for (int i = HIGHESTREQUEST; i > 0; i--)
			PushRequest(i, ip, fspace);
}

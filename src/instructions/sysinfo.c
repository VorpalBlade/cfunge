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

#include "../global.h"
#include <unistd.h>
#include "sysinfo.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../rect.h"
#include "../stack.h"
#include "../ip.h"
#include <time.h>
#include <string.h>

// Tmp variable used for pushing of stack size.
static size_t stackSize = 0;

#ifndef _GNU_SOURCE
extern char **environ;
#endif

// Push a single request value
// pushStack is stack to push on.
static void PushRequest(FUNGEDATATYPE request, instructionPointer * ip, fungeStack * pushStack)
{
	switch (request) {
		case 1: // Flags
			//StackPush(0x20, ip->stack);
			StackPush(0x0, pushStack);
			break;
		case 2: // Cell size
			StackPush(sizeof(FUNGEDATATYPE), pushStack);
			break;
		case 3: // Handprint
			StackPush(FUNGEHANDPRINT, pushStack);
			break;
		case 4: // Version
			StackPush(FUNGEVERSION, pushStack);
			break;
		case 5: // Operating paradigm
			StackPush(0, pushStack);
			break;
		case 6: // Path separator
			StackPush('/', pushStack);
			break;
		case 7: // Scalars / vector
			StackPush(2, pushStack);
			break;
		case 8: // IP ID
			StackPush(0, pushStack);
			break;
		case 9: // TEAM ID
			StackPush(0, pushStack);
			break;
		case 10: // Vector of current IP position
			StackPushVector(&ip->position, pushStack);
			break;
		case 11: // Delta of current IP position
			StackPushVector(&ip->delta, pushStack);
			break;
		case 12: // Storage offset of current IP position
			StackPushVector(&ip->storageOffset, pushStack);
			break;
		case 13: // Least point
			{
				fungeRect rect;
				fungeSpaceGetBoundRect(fspace, &rect);
				StackPushVector(& (fungePosition) { .x = rect.x, .y = rect.y }, pushStack);
				break;
			}
		case 14: // Greatest point
			{
				fungeRect rect;
				fungeSpaceGetBoundRect(fspace, &rect);
				StackPushVector(& (fungePosition) { .x = rect.w, .y = rect.h }, pushStack);
				break;
			}
		case 15: // Time ((year - 1900) * 256 * 256) + (month * 256) + (day of month)
			{
				time_t now;
				struct tm curTime;
				now = time(NULL);
				gmtime_r(&now, &curTime);
				StackPush((FUNGEDATATYPE)(curTime.tm_year * 256 * 256 + (curTime.tm_mon + 1) * 256 + curTime.tm_mday), pushStack);
				break;
			}
		case 16: // Time (hour * 256 * 256) + (minute * 256) + (second)
			{
				time_t now;
				struct tm curTime;
				now = time(NULL);
				gmtime_r(&now, &curTime);
				StackPush((FUNGEDATATYPE)(curTime.tm_hour * 256 * 256 + curTime.tm_min * 256 + curTime.tm_sec), pushStack);
				break;
			}
		case 17: // Number of stacks on stack stack
			StackPush(ip->stackstack->size, pushStack);
			break;
		case 18: // Number of elements on all stacks (TODO for multiple stacks)
			StackPush(stackSize, pushStack);
			break;
		case 19: // Command line arguments
			StackPush('\0', ip->stack);
			StackPush('\0', ip->stack);
			for (int i = fungeargc - 1; i >= 0; i--) {
				StackPushString(strlen(fungeargv[i]), fungeargv[i], pushStack);
			}
			break;
		case 20: // Environment variables
			{
				char * tmp;
				int i = 0;
				StackPush('\0', ip->stack);
				while (true) {
					tmp = environ[i];
					if (!tmp || *tmp == '\0')
						break;
					StackPushString(strlen(tmp), tmp, pushStack);
					i++;
				}
				break;
			}
		default:
			fprintf(stderr, "request was %" FUNGEDATAPRI "\n", request);
			ipReverse(ip);
	}
}

#define HIGHESTREQUEST 20

void RunSysInfo(instructionPointer *ip)
{
	FUNGEDATATYPE request = StackPop(ip->stack);
	stackSize = ip->stack->top;
	// Speed this one up for mycology
	if (request == 23) {
		PushRequest(18, ip, ip->stack);
	} else if (request <= 0) {
		for (int i = HIGHESTREQUEST; i > 0; i--)
			PushRequest(i, ip, ip->stack);
	// Simple to get single cell in this range
	} else if (request < 10) {
		PushRequest(request, ip, ip->stack);
	} else {
		fungeStack * tmp = StackCreate();
		for (int i = 1; i <= HIGHESTREQUEST; i++)
			PushRequest(i, ip, tmp);
		if (tmp->top > request)
			StackPush(tmp->entries[request -1], ip->stack);
		else
			StackPush(ip->stack->entries[ip->stack->top - (request - tmp->top)], ip->stack);
		StackFree(tmp);
	}
}

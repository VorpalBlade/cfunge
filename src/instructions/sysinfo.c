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

#include "../global.h"
#include "sysinfo.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../rect.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"
#include "safe_env.h"

#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Temp variable used for pushing of stack size.
static size_t TOSSSize = 0;

#ifndef _GNU_SOURCE
extern char **environ;
#endif

// Push a single request value
// pushStack is stack to push on.
static void PushRequest(FUNGEDATATYPE request, instructionPointer * restrict ip, fungeStack * restrict pushStack)
{
	switch (request) {
		case 1: // Flags
			{
				FUNGEDATATYPE tmp = 0x0;
#ifdef CONCURRENT_FUNGE
				tmp |= 0x01;
#endif
				if (!SettingSandbox) {
					// i = 0x02, o = 0x04
					tmp |= 0x06;
				}
				if (SettingCurrentStandard == stdver108)
					tmp |= 0x20;
				StackPush(pushStack, tmp);
			}
			break;
		case 2: // Cell size
			StackPush(pushStack, sizeof(FUNGEDATATYPE));
			break;
		case 3: // Handprint
			StackPush(pushStack, FUNGEHANDPRINT);
			break;
		case 4: // Version
			StackPush(pushStack, FUNGEVERSION);
			break;
		case 5: // Operating paradigm
			StackPush(pushStack, 0);
			break;
		case 6: // Path separator
			StackPush(pushStack, '/');
			break;
		case 7: // Scalars / vector
			StackPush(pushStack, 2);
			break;
		case 8: // IP ID
			StackPush(pushStack, ip->ID);
			break;
		case 9: // TEAM ID
			StackPush(pushStack, 0);
			break;
		case 10: // Vector of current IP position
			StackPushVector(pushStack, &ip->position);
			break;
		case 11: // Delta of current IP position
			StackPushVector(pushStack, &ip->delta);
			break;
		case 12: // Storage offset of current IP position
			StackPushVector(pushStack, &ip->storageOffset);
			break;
		case 13: // Least point
			{
				fungeRect rect;
				FungeSpaceGetBoundRect(&rect);
				StackPushVector(pushStack, VectorCreateRef(rect.x, rect.y));
				break;
			}
		case 14: // Greatest point
			{
				fungeRect rect;
				FungeSpaceGetBoundRect(&rect);
				StackPushVector(pushStack, VectorCreateRef(rect.x + rect.w, rect.y + rect.h));
				break;
			}
		case 15: // Time ((year - 1900) * 256 * 256) + (month * 256) + (day of month)
			{
				time_t now;
				struct tm *curTime;
				now = time(NULL);
				curTime = gmtime(&now);
				StackPush(pushStack, (FUNGEDATATYPE)(curTime->tm_year * 256 * 256 + (curTime->tm_mon + 1) * 256 + curTime->tm_mday));
				break;
			}
		case 16: // Time (hour * 256 * 256) + (minute * 256) + (second)
			{
				time_t now;
				struct tm *curTime;
				now = time(NULL);
				curTime = gmtime(&now);
				StackPush(pushStack, (FUNGEDATATYPE)(curTime->tm_hour * 256 * 256 + curTime->tm_min * 256 + curTime->tm_sec));
				break;
			}
		case 17: // Number of stacks on stack stack
			StackPush(pushStack, ip->stackstack->size);
			break;
		case 18: // Number of elements on all stacks
// I think CCBI is wrong here, but most do it this way, and so do we, but it is easy to change.
#define STACKORDER_CCBI
#ifdef STACKORDER_CCBI
			for (size_t i = 0; i < ip->stackstack->current; i++)
				StackPush(pushStack, ip->stackstack->stacks[i]->top);
			StackPush(pushStack, TOSSSize);
#else
			StackPush(pushStack, TOSSSize);
			for (size_t i = ip->stackstack->current; i-- > 0;)
				StackPush(pushStack, ip->stackstack->stacks[i]->top);
#endif
			break;
		case 19: // Command line arguments
			StackPush(pushStack, '\0');
			StackPush(pushStack, '\0');
			for (int i = fungeargc - 1; i >= 0; i--) {
				StackPushString(pushStack, fungeargv[i], strlen(fungeargv[i]));
			}
			break;
		case 20: // Environment variables
			{
				char * tmp;
				int i = 0;
				StackPush(pushStack, '\0');

				while (true) {
					tmp = environ[i];
					if (!tmp || *tmp == '\0')
						break;
					if (SettingSandbox) {
						if (!CheckEnvIsSafe(tmp)) {
							i++;
							continue;
						}
					}
					StackPushString(pushStack, tmp, strlen(tmp));
					i++;
				}

				break;
			}
		case 21: // 1 cell containing type of basic data unit used for cells (global env) (108 specific)
			// Bytes
			StackPush(pushStack, 2);
			break;
		case 22: // 1 cell containing cell size in the unit returned by request 21. (global env) (108 specific)
			StackPush(pushStack, sizeof(FUNGEDATATYPE) * CHAR_BIT);
			break;
#ifndef NDEBUG
		default:
			fprintf(stderr, "request was %" FUNGEDATAPRI "\nThis should not happen!\n", request);
			abort();
#endif
	}
}

#define HIGHESTREQUEST_98 20
#define HIGHESTREQUEST_108 22

void RunSysInfo(instructionPointer *ip)
{
	FUNGEDATATYPE request = StackPop(ip->stack);
	assert(ip != NULL);
	TOSSSize = ip->stack->top;
	// Negative: push all
	if (request <= 0) {
		for (int i = ((SettingCurrentStandard == stdver108) ? HIGHESTREQUEST_108 : HIGHESTREQUEST_98); i > 0; i--)
			PushRequest(i, ip, ip->stack);
	// Simple to get single cell in this range
	} else if (request < 10) {
		PushRequest(request, ip, ip->stack);
	} else {
		fungeStack * tmp = StackCreate();
		for (int i = ((SettingCurrentStandard == stdver108) ? HIGHESTREQUEST_108 : HIGHESTREQUEST_98); i > 0; i--)
			PushRequest(i, ip, tmp);
		if (tmp->top > (size_t)request)
			StackPush(ip->stack, tmp->entries[tmp->top - request]);
		else {
			StackPopNDiscard(ip->stack, request - tmp->top - 1);
		}
		StackFree(tmp);
	}
}

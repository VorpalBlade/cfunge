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

#ifdef __WIN32__
// Now, win32 is crap and insane, so we just fake it, much simpler.
static const char * environ[] = {
	"SYSTEM=windows crap",
	"SUPPORTS=not environ at least, get a sane system if you want this to work.",
	"REALLY=we mean it, cfunge on windows is NOT SUPPORTED."
};
#else
#  ifndef _GNU_SOURCE
extern char **environ;
#  endif
#endif

#define FUNGE_FLAGS_CONCURRENT 0x01
#define FUNGE_FLAGS_INPUT      0x02
#define FUNGE_FLAGS_OUTPUT     0x04
#define FUNGE_FLAGS_EXECUTE    0x08
#define FUNGE_FLAGS_STD109     0x20

#define FUNGE_FLAGS_NOTSANDBOX FUNGE_FLAGS_INPUT | FUNGE_FLAGS_OUTPUT | FUNGE_FLAGS_EXECUTE


typedef enum SysInfoRequests {
	si_flags = 1,
	si_cell_size,
	si_handprint98,
	si_version,
	si_operating_paradigm,
	si_path_separator,
	si_vector_size,
	si_ip_id,
	si_ip_team_id,
	si_ip_pos,
	si_ip_delta,
	si_ip_storage_offset,
	si_least_point,
	si_greatest_point,
	si_year_month_day,
	si_hour_minute_second,
	si_stack_count,
	si_stack_sizes,
	si_argc, // Funge-109
	si_argv,
	si_env_count, // Funge-109
	si_env,
	// Funge-109
	si_handprint109,
	si_basic_data_unit,
	si_cell_size_in_unit
} SysInfoRequests;

static const SysInfoRequests Funge98Requests[] = {
	si_flags,
	si_cell_size,
	si_handprint98,
	si_version,
	si_operating_paradigm,
	si_path_separator,
	si_vector_size,
	si_ip_id,
	si_ip_team_id,
	si_ip_pos,
	si_ip_delta,
	si_ip_storage_offset,
	si_least_point,
	si_greatest_point,
	si_year_month_day,
	si_hour_minute_second,
	si_stack_count,
	si_stack_sizes,
	si_argv,
	si_env
};

static const SysInfoRequests Funge109Requests[] = {
	si_flags,
	si_cell_size,
	si_handprint98,
	si_version,
	si_operating_paradigm,
	si_path_separator,
	si_vector_size,
	si_ip_id,
	si_ip_team_id,
	si_ip_pos,
	si_ip_delta,
	si_ip_storage_offset,
	si_least_point,
	si_greatest_point,
	si_year_month_day,
	si_hour_minute_second,
	si_stack_count,
	si_stack_sizes,
	si_argc,
	si_argv,
	si_env_count,
	si_env,
	si_handprint109,
	si_basic_data_unit,
	si_cell_size_in_unit
};

// We cache the result here.
static size_t environ_count = 0;

// Push a single request value.
// pushStack is stack to push on.
FUNGE_ATTR_FAST static void push_request(fungeCell request, instructionPointer * restrict ip, funge_stack * restrict pushStack)
{
	switch (request) {
		case si_flags: { // Flags
			fungeCell tmp = 0x0;
#ifdef CONCURRENT_FUNGE
			tmp |= FUNGE_FLAGS_CONCURRENT;
#endif
			if (!setting_enable_sandbox) {
				// i, o and =
				tmp |= FUNGE_FLAGS_NOTSANDBOX;
			}
			if (setting_current_standard == stdver109)
				tmp |= FUNGE_FLAGS_STD109;
			stack_push(pushStack, tmp);
			break;
		}
		case si_cell_size: // Cell size
			stack_push(pushStack, sizeof(fungeCell));
			break;
		case si_handprint98: // Handprint
			stack_push(pushStack, FUNGE_OLD_HANDPRINT);
			break;
		case si_version: // Version
			stack_push(pushStack, FUNGEVERSION);
			break;
		case si_operating_paradigm: // Operating paradigm
			if (setting_enable_sandbox) {
				stack_push(pushStack, 0);
			} else {
				// 1 = As system()
				stack_push(pushStack, 1);
			}
			break;
		case si_path_separator: // Path separator
#ifdef __WIN32__
			stack_push(pushStack, (fungeCell)'\\');
#else
			stack_push(pushStack, (fungeCell)'/');
#endif
			break;
		case si_vector_size: // Scalars / vector
			stack_push(pushStack, 2);
			break;
		case si_ip_id: // IP ID
			stack_push(pushStack, ip->ID);
			break;
		case si_ip_team_id: // TEAM ID
			stack_push(pushStack, 0);
			break;
		case si_ip_pos: // Vector of current IP position
			stack_push_vector(pushStack, &ip->position);
			break;
		case si_ip_delta: // Delta of current IP position
			stack_push_vector(pushStack, &ip->delta);
			break;
		case si_ip_storage_offset: // Storage offset of current IP position
			stack_push_vector(pushStack, &ip->storageOffset);
			break;
		case si_least_point: { // Least point
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push_vector(pushStack, vector_create_ref(rect.x, rect.y));
			break;
		}
		case si_greatest_point: { // Greatest point
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push_vector(pushStack, vector_create_ref(rect.x + rect.w, rect.y + rect.h));
			break;
		}
		case si_year_month_day: { // Time ((year - 1900) * 256 * 256) + (month * 256) + (day of month)
			time_t now;
			struct tm *curTime;
			now = time(NULL);
			curTime = gmtime(&now);
			stack_push(pushStack, (fungeCell)(curTime->tm_year * 256 * 256 + (curTime->tm_mon + 1) * 256 + curTime->tm_mday));
			break;
		}
		case si_hour_minute_second: { // Time (hour * 256 * 256) + (minute * 256) + (second)
			time_t now;
			struct tm *curTime;
			now = time(NULL);
			curTime = gmtime(&now);
			stack_push(pushStack, (fungeCell)(curTime->tm_hour * 256 * 256 + curTime->tm_min * 256 + curTime->tm_sec));
			break;
		}
		case si_stack_count: // Number of stacks on stack stack
			stack_push(pushStack, (fungeCell)ip->stackstack->size);
			break;
		case si_stack_sizes: // Number of elements on all stacks
			for (size_t i = 0; i < ip->stackstack->current; i++)
				stack_push(pushStack, (fungeCell)ip->stackstack->stacks[i]->top);
			stack_push(pushStack, (fungeCell)TOSSSize);
			break;
		case si_argc: // Command line arguments (count) (109 specific)
			stack_push(pushStack, fungeargc);
			break;
		case si_argv: // Command line arguments
			stack_push(pushStack, (fungeCell)'\0');
			stack_push(pushStack, (fungeCell)'\0');
			for (int i = fungeargc - 1; i >= 0; i--) {
				stack_push_string(pushStack, (const unsigned char*)fungeargv[i], strlen(fungeargv[i]));
			}
			break;
		case si_env_count: // Environment variables (count) (109 specific)
			// Check if result is cached.
			if (environ_count == 0) {
				size_t i = 0;
				while (true) {
					if (!environ[i] || *environ[i] == (fungeCell)'\0')
						break;
					if (setting_enable_sandbox) {
						if (!check_env_is_safe(environ[i])) {
							i++;
							continue;
						}
					}
					environ_count++;
					i++;
				}
			}
			stack_push(pushStack, (fungeCell)environ_count);
			break;
		case si_env: { // Environment variables
			size_t i = 0;
			stack_push(pushStack, (fungeCell)'\0');

			while (true) {
				if (!environ[i] || *environ[i] == (fungeCell)'\0')
					break;
				if (setting_enable_sandbox) {
					if (!check_env_is_safe(environ[i])) {
						i++;
						continue;
					}
				}
				stack_push_string(pushStack, (const unsigned char*)environ[i], strlen(environ[i]));
				i++;
			}

			break;
		}
		case si_handprint109: // 1 0"gnirts" with Funge-109 URI (global env) (109 specific)
			// Bytes
			stack_push_string(pushStack, (const unsigned char*)FUNGE_NEW_HANDPRINT, strlen(FUNGE_NEW_HANDPRINT));
			break;
		case si_basic_data_unit: // 1 cell containing type of basic data unit used for cells (global env) (109 specific)
			// Bytes
			stack_push(pushStack, 2);
			break;
		case si_cell_size_in_unit: // 1 cell containing cell size in the unit returned by request 21. (global env) (109 specific)
			stack_push(pushStack, sizeof(fungeCell) * CHAR_BIT);
			break;
#ifndef NDEBUG
		default:
			fprintf(stderr, "request was %" FUNGECELLPRI "\nThis should not happen!\n", request);
			abort();
#endif
	}
}

FUNGE_ATTR_FAST void run_sys_info(instructionPointer *ip)
{
	fungeCell request = stack_pop(ip->stack);
	assert(ip != NULL);
	TOSSSize = ip->stack->top;
	// Negative or 0: push all
	if (request <= 0) {
		if (setting_current_standard == stdver109) {
			for (int i = sizeof(Funge109Requests) / sizeof(si_flags) - 1; i >= 0; i--)
				push_request(Funge109Requests[i], ip, ip->stack);
		} else {
			for (int i = sizeof(Funge98Requests) / sizeof(si_flags) - 1; i >= 0; i--)
				push_request(Funge98Requests[i], ip, ip->stack);
		}
	// Simple to get single cell in this range
	} else if (request < 10) {
		push_request(request, ip, ip->stack);
	} else {
		funge_stack * restrict tmp = stack_create();
		if (setting_current_standard == stdver109) {
			for (int i = sizeof(Funge109Requests) / sizeof(si_flags) - 1; i >= 0; i--)
				push_request(Funge109Requests[i], ip, tmp);
		} else {
			for (int i = sizeof(Funge98Requests) / sizeof(si_flags) - 1; i >= 0; i--)
				push_request(Funge98Requests[i], ip, tmp);
		}
		// Find out if we should act as pick or not...
		if (tmp->top > (size_t)request) {
			stack_push(ip->stack, tmp->entries[tmp->top - (size_t)request]);
		} else {
			// Act as pick
			stack_push(ip->stack, stack_get_index(ip->stack, request - tmp->top));
		}
		stack_free(tmp);
	}
}

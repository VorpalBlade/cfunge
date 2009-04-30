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
#include "safe_env.h"

#include "../funge-space/funge-space.h" /* fungespace_get_bounds_rect */
#include "../ip.h"
#include "../main.h"                    /* fungeargc, fungeargv */
#include "../rect.h"
#include "../settings.h"
#include "../stack.h"
#include "../vector.h"

#include <unistd.h> /* environ (partly) */
#include <time.h>   /* gmtime, time, time_t */
#include <string.h> /* strlen */
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

#ifdef CONCURRENT_FUNGE
#  define FUNGE_FLAGS_BASIC FUNGE_FLAGS_CONCURRENT
#else
#  define FUNGE_FLAGS_BASIC 0x0
#endif

/// We cache the number of environment variables here
static size_t environ_count = 0;

/// Flags
#define PUSH_REQ_1(m_pushstack) \
	do { \
		funge_cell tmp = FUNGE_FLAGS_BASIC; \
		if (!setting_enable_sandbox) { \
			tmp |= FUNGE_FLAGS_NOTSANDBOX; \
		} \
		if (FUNGE_UNLIKELY(setting_current_standard == stdver109)) \
			tmp |= FUNGE_FLAGS_STD109; \
		stack_push((m_pushstack), tmp); \
	} while(0)
/// Cell size
#define PUSH_REQ_2(m_pushstack) \
	stack_push((m_pushstack), sizeof(funge_cell))
/// Funge-98 Handprint
#define PUSH_REQ_3(m_pushstack) \
	stack_push((m_pushstack), FUNGE_OLD_HANDPRINT)
/// Interpreter version
#define PUSH_REQ_4(m_pushstack) \
	stack_push((m_pushstack), CFUNGE_VERSION_Y)
/// Operating paradigm
#define PUSH_REQ_5(m_pushstack) \
	do { \
		if (setting_enable_sandbox) \
			stack_push((m_pushstack), 0); \
		else \
			stack_push((m_pushstack), 1); \
	} while(0)
/// Path separator
#ifdef __WIN32__
#  define PUSH_REQ_6(m_pushstack) \
	stack_push((m_pushstack), (funge_cell)'\\')
#else
#  define PUSH_REQ_6(m_pushstack) \
	stack_push((m_pushstack), (funge_cell)'/')
#endif
/// Scalars / vector
#define PUSH_REQ_7(m_pushstack) \
	stack_push((m_pushstack), 2)
/// IP ID
#define PUSH_REQ_8(m_pushstack, m_ip) \
	stack_push((m_pushstack), (m_ip)->ID)
/// Team ID
#define PUSH_REQ_9(m_pushstack) \
	stack_push((m_pushstack), 0)
/// Position
#define PUSH_REQ_10(m_pushstack, m_ip) \
	stack_push_vector((m_pushstack), &(m_ip)->position)
/// Delta
#define PUSH_REQ_11(m_pushstack, m_ip) \
	stack_push_vector((m_pushstack), &(m_ip)->delta)
/// Storage offset
#define PUSH_REQ_12(m_pushstack, m_ip) \
	stack_push_vector((m_pushstack), &(m_ip)->storageOffset)
/// Least point
#define PUSH_REQ_13(m_pushstack, m_bounds_rect) \
	stack_push_vector((m_pushstack), vector_create_ref((m_bounds_rect).x, (m_bounds_rect).y))
/// Greatest point
#define PUSH_REQ_14(m_pushstack, m_bounds_rect) \
	stack_push_vector((m_pushstack), vector_create_ref((m_bounds_rect).w, (m_bounds_rect).h))
/// Date
#define PUSH_REQ_15(m_pushstack, m_tm) \
	stack_push((m_pushstack), (funge_cell)((m_tm)->tm_year * 256 * 256 + ((m_tm)->tm_mon + 1) * 256 + (m_tm)->tm_mday))
/// Time
#define PUSH_REQ_16(m_pushstack, m_tm) \
	stack_push((m_pushstack), (funge_cell)((m_tm)->tm_hour * 256 * 256 + (m_tm)->tm_min * 256 + (m_tm)->tm_sec))
/// Stack stack count
#define PUSH_REQ_17(m_pushstack, m_ip) \
	stack_push((m_pushstack), (funge_cell)(m_ip)->stackstack->size)
/// Number of elements on stacks
#define PUSH_REQ_18(m_pushstack, m_stackstack) \
	do { \
		const size_t stack_stack_count = (m_stackstack)->current; \
		for (size_t i = 0; i < stack_stack_count; i++) \
			stack_push((m_pushstack), (funge_cell)(m_stackstack)->stacks[i]->top); \
		stack_push((m_pushstack), (funge_cell)TOSSSize); \
		break; \
	} while(0)
/// Argc (109)
#define PUSH_REQ_19a(m_pushstack) \
	stack_push((m_pushstack), fungeargc)
/// Argv
#define PUSH_REQ_19b(m_pushstack) \
	do { \
		stack_push((m_pushstack), (funge_cell)'\0');\
		stack_push((m_pushstack), (funge_cell)'\0'); \
		for (int i = fungeargc - 1; i >= 0; i--) { \
			stack_push_string((m_pushstack), (const unsigned char*)fungeargv[i], strlen(fungeargv[i])); \
		} \
	} while(0)
/// Environment variable count (109)
#define PUSH_REQ_20a(m_pushstack) \
	do { \
		if (environ_count == 0) { \
			size_t i = 0; \
			while (environ[i]) { \
				if (FUNGE_UNLIKELY(setting_enable_sandbox)) { \
					if (FUNGE_UNLIKELY(!check_env_is_safe(environ[i]))) { \
						i++; \
						continue; \
					} \
				} \
				environ_count++; \
				i++; \
			} \
		} \
		stack_push((m_pushstack), (funge_cell)environ_count); \
	} while(0)
/// Environment variables
#define PUSH_REQ_20b(m_pushstack) \
	do { \
		size_t i = 0; \
		stack_push((m_pushstack), (funge_cell)'\0'); \
		while (environ[i]) { \
			if (FUNGE_UNLIKELY(setting_enable_sandbox)) { \
				if (FUNGE_LIKELY(!check_env_is_safe(environ[i]))) { \
					i++; \
					continue; \
				} \
			} \
			stack_push_string((m_pushstack), (const unsigned char*)environ[i], strlen(environ[i])); \
			i++; \
		} \
	} while(0)
/// 109 handprint
#define PUSH_REQ_21(m_pushstack) \
	stack_push_string((m_pushstack), (const unsigned char*)FUNGE_NEW_HANDPRINT, strlen(FUNGE_NEW_HANDPRINT))

/**
 * Push all of the y values (as would be done for 0y or such).
 * @param ip IP that is calling y.
 * @param pushStack Stack to push on.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static void push_all(instructionPointer * restrict ip, funge_stack * restrict pushStack) {
	fungeRect rect;
	time_t now;
	struct tm *curTime;
	if (FUNGE_UNLIKELY(setting_current_standard == stdver109))
		PUSH_REQ_21(pushStack);
	PUSH_REQ_20b(pushStack);
	if (FUNGE_UNLIKELY(setting_current_standard == stdver109))
		PUSH_REQ_20a(pushStack);
	PUSH_REQ_19b(pushStack);
	if (FUNGE_UNLIKELY(setting_current_standard == stdver109))
		PUSH_REQ_19a(pushStack);
	PUSH_REQ_18(pushStack, ip->stackstack);
	PUSH_REQ_17(pushStack, ip);
	now = time(NULL);
	curTime = gmtime(&now);
	PUSH_REQ_16(pushStack, curTime);
	PUSH_REQ_15(pushStack, curTime);
	fungespace_get_bounds_rect(&rect);
	PUSH_REQ_14(pushStack, rect);
	PUSH_REQ_13(pushStack, rect);
	PUSH_REQ_12(pushStack, ip);
	PUSH_REQ_11(pushStack, ip);
	PUSH_REQ_10(pushStack, ip);
	PUSH_REQ_9(pushStack);
	PUSH_REQ_8(pushStack, ip);
	PUSH_REQ_7(pushStack);
	PUSH_REQ_6(pushStack);
	PUSH_REQ_5(pushStack);
	PUSH_REQ_4(pushStack);
	PUSH_REQ_3(pushStack);
	PUSH_REQ_2(pushStack);
	PUSH_REQ_1(pushStack);
}


/**
 * Push a single y value.
 * @param request Value to push (indexed as cell from top).
 * @param ip IP that is calling y.
 * @param pushStack Stack to push on.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static void push_yval(funge_cell request, instructionPointer * restrict ip, funge_stack * restrict pushStack)
{	switch (request) {
		case 1: // Flags
			PUSH_REQ_1(pushStack);
			break;
		case 2: // Cell size
			PUSH_REQ_2(pushStack);
			break;
		case 3: // Handprint
			PUSH_REQ_3(pushStack);
			break;
		case 4: // Version
			PUSH_REQ_4(pushStack);
			break;
		case 5: // Operating paradigm
			PUSH_REQ_5(pushStack);
			break;
		case 6: // Path separator
			PUSH_REQ_6(pushStack);
			break;
		case 7: // Scalars / vector
			PUSH_REQ_7(pushStack);
			break;
		case 8: // IP ID
			PUSH_REQ_8(pushStack, ip);
			break;
		case 9: // TEAM ID
			PUSH_REQ_9(pushStack);
			break;
		// Vector of current IP position (y component)
		case 10:
			stack_push(pushStack, ip->position.y);
			break;
		// Vector of current IP position (x component)
		case 11:
			stack_push(pushStack, ip->position.x);
			break;
		// Delta of current IP (y component)
		case 12:
			stack_push(pushStack, ip->delta.y);
			break;
		// Delta of current IP (x component)
		case 13:
			stack_push(pushStack, ip->delta.x);
			break;
		// Storage offset of current IP (y component)
		case 14:
			stack_push(pushStack, ip->storageOffset.y);
			break;
		// Storage offset of current IP (x component)
		case 15:
			stack_push(pushStack, ip->storageOffset.x);
			break;
		// Least point (y component)
		case 16: {
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push(pushStack, rect.y);
			break;
		}
		// Least point (x component)
		case 17: {
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push(pushStack, rect.x);
			break;
		}
		// Greatest point (y component)
		case 18: {
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push(pushStack, rect.h);
			break;
		}
		// Greatest point (x component)
		case 19: {
			fungeRect rect;
			fungespace_get_bounds_rect(&rect);
			stack_push(pushStack, rect.w);
			break;
		}
		case 20: { // Date ((year - 1900) * 256 * 256) + (month * 256) + (day of month)
			time_t now;
			struct tm *curTime;
			now = time(NULL);
			curTime = gmtime(&now);
			PUSH_REQ_15(pushStack, curTime);
			break;
		}
		case 21: { // Time (hour * 256 * 256) + (minute * 256) + (second)
			time_t now;
			struct tm *curTime;
			now = time(NULL);
			curTime = gmtime(&now);
			PUSH_REQ_16(pushStack, curTime);
			break;
		}
		case 22: // Number of stacks on stack stack
			PUSH_REQ_17(pushStack, ip);
			break;
		case 23: // Size of TOSS
			stack_push(pushStack, (funge_cell)TOSSSize);
			break;
	}
}


/// Temp stack for pushing on when needed. Faster.
static funge_stack* restrict sysinfo_tmp_stack = NULL;

FUNGE_ATTR_FAST
void run_sys_info(instructionPointer *ip)
{
	funge_cell request = stack_pop(ip->stack);
	TOSSSize = ip->stack->top;
	// Negative or 0: push all
	if (request <= 0) {
		push_all(ip, ip->stack);
	// Simple to get single cell in this range
	} else if (request < 24) {
		push_yval(request, ip, ip->stack);
	// Large positive, hard to calculate in advance, or may even be pick.
	} else {
		if (FUNGE_UNLIKELY(!sysinfo_tmp_stack))
			sysinfo_tmp_stack = stack_create();
		push_all(ip, sysinfo_tmp_stack);
		// Find out if we should act as pick or not...
		if (sysinfo_tmp_stack->top > (size_t)request) {
			stack_push(ip->stack, sysinfo_tmp_stack->entries[sysinfo_tmp_stack->top - (size_t)request]);
		} else {
			// Act as pick
			stack_push(ip->stack, stack_get_index(ip->stack, request - sysinfo_tmp_stack->top));
		}
		stack_clear(sysinfo_tmp_stack);
	}
}

#ifndef NDEBUG
/// Free some memory if debug build.
FUNGE_ATTR_FAST
void sysinfo_cleanup(void) {
	if (sysinfo_tmp_stack)
		stack_free(sysinfo_tmp_stack);
}
#endif

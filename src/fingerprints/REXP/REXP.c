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

#include "REXP.h"
#include "../../stack.h"

#include <sys/types.h> /* Regular expressions */
#include <regex.h> /* Regular expressions */
#include <unistd.h>
#include <string.h>

#if !defined(_POSIX_REGEXP) || (_POSIX_REGEXP < 1)
#  error "cfunge needs POSIX regular expressions, which this system claims it doesn't have."
#endif

#define MATCHSIZE 128

static regex_t compiled_regex;
static bool compiled_valid = false;
static bool compiled_nosub = false;
static regmatch_t matches[MATCHSIZE];

// The flags used in Funge could differ from the system ones.

// C
#define FUNGE_REG_EXTENDED 1
#define FUNGE_REG_ICASE    2
#define FUNGE_REG_NOSUB    4
#define FUNGE_REG_NEWLINE  8
// C return
#define FUNGE_REG_BADBR    1
#define FUNGE_REG_BADPAT   2
#define FUNGE_REG_BADRPT   3
#define FUNGE_REG_EBRACE   4
#define FUNGE_REG_EBRACK   5
#define FUNGE_REG_ECOLLATE 6
#define FUNGE_REG_ECTYPE   7
#define FUNGE_REG_EESCAPE  9
#define FUNGE_REG_EPAREN   10
#define FUNGE_REG_ERANGE   11
#define FUNGE_REG_ESPACE   13
#define FUNGE_REG_ESUBREG  14
// E
#define FUNGE_REG_NOTBOL 1
#define FUNGE_REG_NOTEOL 2

FUNGE_ATTR_FAST
static inline int translate_flags_C(funge_cell flags)
{
	int ret = 0;
	if (flags & FUNGE_REG_EXTENDED)
		ret |= REG_EXTENDED;
	if (flags & FUNGE_REG_ICASE)
		ret |= REG_ICASE;
	if (flags & FUNGE_REG_NOSUB)
		ret |= REG_NOSUB;
	if (flags & FUNGE_REG_NEWLINE)
		ret |= REG_NEWLINE;
	return ret;
}

FUNGE_ATTR_FAST
static inline int translate_flags_E(funge_cell flags)
{
	int ret = 0;
	if (flags & FUNGE_REG_NOTBOL)
		ret |= REG_NOTBOL;
	if (flags & FUNGE_REG_NOTEOL)
		ret |= REG_NOTEOL;
	return ret;
}

#define GenErrorCase(name) case name: return FUNGE_ ## name

FUNGE_ATTR_FAST
static inline int translate_return_C(int error)
{
	switch (error) {
		GenErrorCase(REG_BADBR);
		GenErrorCase(REG_BADPAT);
		GenErrorCase(REG_BADRPT);
		GenErrorCase(REG_EBRACE);
		GenErrorCase(REG_EBRACK);
		GenErrorCase(REG_ECOLLATE);
		GenErrorCase(REG_ECTYPE);
		GenErrorCase(REG_EESCAPE);
		GenErrorCase(REG_EPAREN);
		GenErrorCase(REG_ERANGE);
		GenErrorCase(REG_ESPACE);
		GenErrorCase(REG_ESUBREG);
	}
	// Should never be reached:
	return -1;
}

FUNGE_ATTR_FAST
static inline void push_results(instructionPointer * restrict ip,
                                char * restrict str)
{
	if (compiled_nosub) {
		stack_push(ip->stack, 0);
	} else {
		int count = 0;
		for (int i = MATCHSIZE - 1; i >= 0; i--) {
			if (matches[i].rm_so != -1) {
				count++;
				stack_push(ip->stack, 0);
				stack_push_string(ip->stack, (unsigned char*)str + matches[i].rm_so,
				                  matches[i].rm_eo - matches[i].rm_so - 1);
			}
		}
		stack_push(ip->stack, count);
	}
}

/// C - Compile a regular expression
static void finger_REXP_compile(instructionPointer * ip)
{
	char * restrict str;
	int flags;
	int compret;

	// Avoid memory leak.
	if (compiled_valid)
		regfree(&compiled_regex);

	flags = translate_flags_C(stack_pop(ip->stack));
	str = (char*)stack_pop_string(ip->stack, NULL);

	compret = regcomp(&compiled_regex, str, flags);

	if (compret != 0) {
		ip_reverse(ip);
		stack_push(ip->stack, translate_return_C(compret));
		compiled_valid = false;
	} else {
		compiled_valid = true;
		compiled_nosub = (flags & REG_NOSUB);
	}

	stack_free_string(str);
}

/// E - Execute regular expression on string
static void finger_REXP_execute(instructionPointer * ip)
{
	char * str;
	int flags;
	int execret;

	if (!compiled_valid) {
		ip_reverse(ip);
		return;
	}

	flags = translate_flags_E(stack_pop(ip->stack));
	str = (char*)stack_pop_string(ip->stack, NULL);

	execret = regexec(&compiled_regex, str, MATCHSIZE, matches, flags);
	if (execret == 0) {
		push_results(ip, str);
	} else {
		ip_reverse(ip);
	}
	stack_free_string(str);
}

/// F - Free compiled regex buffer
static void finger_REXP_free(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	if (compiled_valid) {
		regfree(&compiled_regex);
		compiled_valid = false;
	}
}

bool finger_REXP_load(instructionPointer * ip)
{
	manager_add_opcode(REXP, 'C', compile)
	manager_add_opcode(REXP, 'E', execute)
	manager_add_opcode(REXP, 'F', free)
	return true;
}

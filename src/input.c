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

#include "global.h"
#include "input.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

// We use static buffer for input to save input
// from one read to the next if there was any
// left.
static char*  lastline = NULL;
static size_t linelength = 0;
static char*  lastlineCurrent = NULL;

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool getTheLine(void)
{
	if (!lastline || !lastlineCurrent || (*lastlineCurrent == '\0')) {
		ssize_t retval = cf_getline(&lastline, &linelength, stdin);
		if (retval == -1)
			return false;
		lastlineCurrent = lastline;
	}
	return true;
}
FUNGE_ATTR_FAST static inline void discardTheLine(void)
{
	if (lastline != NULL)
		cf_free(lastline);
	lastline = NULL;
	lastlineCurrent = NULL;
}


FUNGE_ATTR_FAST bool input_getchar(FUNGEDATATYPE * chr)
{
	char tmp;
	if (!getTheLine())
		return false;
	tmp = *lastlineCurrent;
	lastlineCurrent++;
	if (lastlineCurrent && (*lastlineCurrent == '\0'))
		discardTheLine();
	*chr = (FUNGEDATATYPE)tmp;
	return true;
}

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

// Start of s as if it is in base.
// Unlike strtoll this does not clamp on overflow but stop reading just before
// a overflow would happen.
// Converted value is returned in *value.
// Return value is last index used in string.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline ptrdiff_t parseInt(const char * restrict s,
                                 FUNGEDATATYPE * value, FUNGEDATATYPE base)
{
	FUNGEDATATYPE result = 0;
	size_t i;

	assert(s != NULL);
	assert(value != NULL);

	for (i = 0; i <= strlen(s); i++) {
		// Will it overflow?
		if (result > (FUNGEDATA_MAX / base)) {
			break;
		} else {
			FUNGEDATATYPE tmp;
			char c = s[i];
			// Pointer into digits.
			const char * p = strchr(digits, c);
			// Still a digit?
			if (!p || ((p - digits) >= (ptrdiff_t)base))
				break;

			tmp = (FUNGEDATATYPE)(p - digits);
			// Break if it will overflow!
			if ((result * base) > (FUNGEDATA_MAX  - tmp))
				break;
			result = (result * base) + tmp;
		}
	}
	*value = result;
	return (ptrdiff_t)i;
}

// Note, no need to optimise really, this is user input
// bound anyway.
FUNGE_ATTR_FAST ret_getint input_getint(FUNGEDATATYPE * value, int base)
{
	bool found = false;
	char * endptr = NULL;
	assert(value != NULL);

	if (!getTheLine())
		return rgi_eof;
	// Find first char that is a number, then convert number.
	do {
		if (base == 10) {
			if (!isdigit(*lastlineCurrent))
				continue;
		} else if (base == 16) {
			if (!isxdigit(*lastlineCurrent))
				continue;
		} else {
			const char * p = strchr(digits, *lastlineCurrent);
			if (!p || ((p - digits) >= (ptrdiff_t)base))
				continue;
		}
		found = true;
		// Ok, we found it, lets convert it.
		endptr = lastlineCurrent + parseInt(lastlineCurrent, value,
		                                    (FUNGEDATATYPE)base);
		break;
	} while (*(lastlineCurrent++) != '\0');
	// Discard rest of line if it is just newline, otherwise keep it.
	if (endptr && ((*endptr == '\n') || (*endptr == '\r') || (*endptr == '\0')))
		discardTheLine();
	else
		lastlineCurrent = endptr;
	return found ? rgi_success : rgi_noint;
}

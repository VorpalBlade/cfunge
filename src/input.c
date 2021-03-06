/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

#include <assert.h>
#include <ctype.h>  /* isdigit, isxdigit */
#include <stdbool.h>
#include <stddef.h> /* ptrdiff_t */
#include <stdlib.h>

// We use static buffer for input to save input
// from one read to the next if there was any
// left.
static char*  lastline = NULL;
// Size of buffer, may be grown by cf_getline()
static size_t linesize = 0;
// Length of current string in buffer. Needed in case of \0 bytes in it
static size_t linelength = 0;
// Pointer to how far we consumed the current line.
static char*  lastline_current = NULL;

#define IS_LINE_END(ptr) ((size_t)((ptr)-lastline) >= linelength)

FUNGE_ATTR_WARN_UNUSED
static inline bool get_line(void)
{
	if (!lastline || !lastline_current || IS_LINE_END(lastline_current)) {
		ssize_t retval;
		fflush(stdout);
		retval = cf_getline(&lastline, &linesize, stdin);
		if (retval == -1)
			return false;
		lastline_current = lastline;
		linelength = retval;
	}
	return true;
}

static inline void discard_line(void)
{
	if (lastline != NULL)
		free(lastline);
	lastline = NULL;
	linelength = 0;
	linesize = 0;
	lastline_current = NULL;
}


FUNGE_ATTR_FAST bool input_getchar(funge_cell * restrict chr)
{
	unsigned char tmp;
	if (!get_line())
		return false;
	tmp = *((unsigned char*)lastline_current);
	lastline_current++;
	if (lastline_current && IS_LINE_END(lastline_current))
		discard_line();
	*chr = (funge_cell)tmp;
	return true;
}

FUNGE_ATTR_FAST bool input_getline(unsigned char ** str)
{
	if (!get_line())
		return false;
	// TODO: How to handle zero bytes?
	*str = (unsigned char*)strdup(lastline_current);
	discard_line();
	return true;
}


static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

// Start of s as if it is in base.
// Unlike strtoll this does not clamp on overflow but stop reading just before
// a overflow would happen.
// Converted value is returned in *value.
// Return value is last index used in string.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline ptrdiff_t parse_int(const char * restrict s,
                                  funge_cell * restrict value,
                                  funge_cell base,
                                  const size_t length)
{
	funge_cell result = 0;
	size_t i;

	assert(s != NULL);
	assert(value != NULL);

	for (i = 0; i <= length; i++) {
		// Will it overflow?
		if (result > (FUNGECELL_MAX / base))
			break;
		{
			funge_cell tmp;
			char c = s[i];
			// Pointer into digits.
			const char *p = strchr(digits, c);
			// Still a digit?
			if (!p || ((p - digits) >= (ptrdiff_t) base))
				break;

			tmp = (funge_cell) (p - digits);
			// Break if it will overflow!
			if ((result * base) > (FUNGECELL_MAX - tmp))
				break;
			result = (result * base) + tmp;
		}
	}
	*value = result;
	return (ptrdiff_t)i;
}

// Note, no need to optimise really, this is user input
// bound anyway.
FUNGE_ATTR_FAST ret_getint input_getint(funge_cell * restrict value, int base)
{
	bool found = false;
	char * endptr = NULL;
	assert(value != NULL);

	if (!get_line())
		return rgi_eof;
	// Find first char that is a number, then convert number.
	do {
		if (base == 10) {
			if (!isdigit(*lastline_current))
				continue;
		} else if (base == 16) {
			if (!isxdigit(*lastline_current))
				continue;
		} else {
			const char * p = strchr(digits, *lastline_current);
			if (!p || ((p - digits) >= (ptrdiff_t)base))
				continue;
		}
		found = true;
		// Ok, we found it, lets convert it.
		endptr = lastline_current
		         + parse_int(lastline_current, value, (funge_cell)base,
		                     lastline_current - lastline + linelength);
		break;
	} while ((size_t)((lastline_current++) - lastline) < linelength);
	// Discard rest of line if it is just newline, otherwise keep it.
	if (endptr && ((*endptr == '\n') || (*endptr == '\r') || IS_LINE_END(endptr)))
		discard_line();
	else
		lastline_current = endptr;
	return found ? rgi_success : rgi_noint;
}

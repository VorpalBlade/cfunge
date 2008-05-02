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
#include "support.h"

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

// This function is from Gnulib, with some changes
FUNGE_FAST size_t cf_strnlen(const char *string, size_t maxlen)
{
	const char *end = (const char*)memchr(string, '\0', maxlen);
	return end ? (size_t)(end - string) : maxlen;
}

// This function is from Gnulib, with some changes
FUNGE_FAST char * cf_strndup(const char *string, size_t n)
{
	if (!string || !*string)
		return NULL;
	// Keep gcc happy with variable decls
	{
		size_t len = cf_strnlen(string, n);
		char *newstr = (char*)cf_malloc_noptr(len + 1);

		if (newstr == NULL)
			return NULL;

		newstr[len] = '\0';
		return (char*)memcpy(newstr, string, len);
	}
}

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif
#ifndef SSIZE_MAX
# define SSIZE_MAX ((ssize_t) (SIZE_MAX / 2))
#endif

// This function is from Gnulib
__attribute__((warn_unused_result, FUNGE_IN_FAST))
static inline ssize_t
cf_getdelim(char **lineptr, size_t *n, int delimiter, FILE *fp)
{
	ssize_t result;
	size_t cur_len = 0;

	if (lineptr == NULL || n == NULL || fp == NULL) {
		errno = EINVAL;
		return -1;
	}

	cf_flockfile(fp);

	if (*lineptr == NULL || *n == 0) {
		*n = 120;
		*lineptr = (char *) cf_realloc(*lineptr, *n);
		if (*lineptr == NULL) {
			result = -1;
			goto unlock_return;
		}
	}

	for (;;) {
		int i;

		i = cf_getc_unlocked(fp);
		if (i == EOF) {
			result = -1;
			break;
		}

		/* Make enough space for len+1 (for final NUL) bytes.  */
		if (cur_len + 1 >= *n) {
			size_t needed_max =
			    SSIZE_MAX < SIZE_MAX ? (size_t) SSIZE_MAX + 1 : SIZE_MAX;
			size_t needed = 2 * *n + 1;   /* Be generous. */
			char *new_lineptr;

			if (needed_max < needed)
				needed = needed_max;
			if (cur_len + 1 >= needed) {
				result = -1;
// This is for Windows only really.
#ifdef EOVERFLOW
				errno = EOVERFLOW;
#endif
				goto unlock_return;
			}

			new_lineptr = (char *) cf_realloc(*lineptr, needed);
			if (new_lineptr == NULL) {
				result = -1;
				goto unlock_return;
			}

			*lineptr = new_lineptr;
			*n = needed;
		}

		(*lineptr)[cur_len] = i;
		cur_len++;

		if (i == delimiter)
			break;
	}
	(*lineptr)[cur_len] = '\0';
	result = cur_len ? (ssize_t)cur_len : result;

unlock_return:
	cf_funlockfile(fp);  // doesn't set errno

	return result;
}

// This function is from Gnulib
FUNGE_FAST ssize_t
cf_getline(char **lineptr, size_t *n, FILE *stream)
{
	return cf_getdelim(lineptr, n, '\n', stream);
}

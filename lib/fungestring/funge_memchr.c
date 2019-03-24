/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

#include "../../src/global.h"
#include "funge_string.h"

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE
void *funge_memchr(const void *s, funge_cell c, size_t n)
{
	const funge_cell *p = (const funge_cell *)s;
	const funge_cell *end = p + n;

	for (; p < end; p++) {
		FUNGE_WARNING_IGNORE("-Wcast-qual")
		if (*p == c)
			return (void *)p;
		FUNGE_WARNING_RESTORE()
	}

	return NULL;
}

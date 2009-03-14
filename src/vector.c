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
#include "global.h"
#include "vector.h"
#include <stdint.h>

// NOTE: This has been duplicated in funge-space.c for speed reasons.

FUNGE_ATTR_PURE FUNGE_ATTR_FAST
inline bool vector_is_cardinal(const funge_vector * restrict v)
{
	// Due to unsigned this can't overflow in the addition below.
	funge_unsigned_cell x = (funge_unsigned_cell)ABS(v->x);
	funge_unsigned_cell y = (funge_unsigned_cell)ABS(v->y);
	if ((x + y) != 1)
		return false;
	if (x && y)
		return false;
	return true;
}

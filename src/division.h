/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2015 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

/**
 * @file
 * Definition of a rectangle in Funge-Space.
 */

#ifndef FUNGE_HAD_SRC_DIVISION_H
#define FUNGE_HAD_SRC_DIVISION_H

#include "global.h"

/**
 * Performs division of funge cell values that will never cause SIGFPE
 * @param numerator Value to divide
 * @param denominator Value to divide by
 * @return Returns the value.
 */
FUNGE_ATTR_ALWAYS_INLINE FUNGE_ATTR_CONST
static inline funge_cell funge_division(funge_cell numerator, funge_cell denominator)
{
	if (denominator == 0)
		return 0;
	if (numerator == FUNGECELL_MIN && denominator == -1)
		return FUNGECELL_MIN;
	return numerator / denominator;
}

/**
 * Performs division of funge cell values that will never cause SIGFPE
 * @param numerator Value to divide
 * @param denominator Value to divide by
 * @return Returns the value.
 */
FUNGE_ATTR_ALWAYS_INLINE FUNGE_ATTR_CONST
static inline funge_cell funge_modulo(funge_cell numerator, funge_cell denominator)
{
	if (denominator == 0)
		return 0;
	if (numerator == FUNGECELL_MIN && denominator == -1)
		return 0;
	return numerator % denominator;
}
#endif




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
/**
 * @file
 * Definition of, and functions for, a Funge vector.
 */
#ifndef _HAD_SRC_VECTOR_H
#define _HAD_SRC_VECTOR_H

#include "global.h"
#include <stdbool.h>

/// A vector in funge space
typedef struct s_fungeVector {
	FUNGEVECTORTYPE x; ///< You should be able to guess what this is.
	FUNGEVECTORTYPE y; ///< You should be able to guess what this is.
} fungeVector;

/// A synomym for fungeVector.
/// @note
/// This should probably be deprecated.
typedef fungeVector fungePosition;

/// Useful to create a vector in a list of parameter for example.
/// The vector is created on the stack.
#define VectorCreateRef(a, b) (& (fungeVector) { .x = (a), .y = (b) })

/**
 * Checks if vector is cardinal (as in ^>v<).
 * @param v The vector to check.
 */
FUNGE_ATTR_PURE FUNGE_ATTR_FAST
bool VectorIsCardinal(const fungeVector * v);

#endif

/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <anmaster AT tele2 DOT se>
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
 * Random number functions for Cfunge.
 */

#ifndef FUNGE_HAD_SRC_PRNG_H
#define FUNGE_HAD_SRC_PRNG_H

#include "global.h"

/**
 * Initialize the PRNG (if it needs it)
 */
void prng_init(void);

/**
 * Generate a random number in the range [0,max_value).
 * It has protection against modulo bias.
 *
 * @param max_value Upper bound.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
funge_unsigned_cell prng_generate_unsigned(funge_unsigned_cell max_value);

#endif

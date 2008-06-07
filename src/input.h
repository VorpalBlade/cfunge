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
 * The file input.c contains functions for reading user input from STDIN,
 * buffered as needed.
 */

#ifndef _HAD_SRC_INPUT_H
#define _HAD_SRC_INPUT_H

#include "global.h"
#include <stdbool.h>

/**
 * For use in input instruction & and in some fingerprints.
 * @param value Pointer to some memory to place the read integer in.
 * @param base What base to read the number as.
 * @return Returns true if it got an integer in the given base, otherwise false.
 * @note The read integer is returned in the value parameter.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
bool input_getint(FUNGEDATATYPE * value, int base);
/**
 * For use in input instruction ~ and in some fingerprints.
 * This uses a buffer and read in one line (if the buffer is empty,
 * otherwise it reuse the values from the old buffer).
 * @return Returns next char from the buffer.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
FUNGEDATATYPE input_getchar(void);

#endif

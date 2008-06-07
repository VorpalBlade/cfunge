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

#ifndef _HAD_SRC_FUNGE_SPACE_H
#define _HAD_SRC_FUNGE_SPACE_H

#include "../global.h"
#include "../vector.h"
#include "../rect.h"
#include <stdint.h>
#include <stdbool.h>

// DO NOT CHANGE unless you are 100 sure of what you are doing!
// Yes I mean you!
typedef fungePosition fungeSpaceHashKey;

/**
 * Create a funge-space.
 */
bool FungeSpaceCreate(void) __attribute__((warn_unused_result, FUNGE_IN_FAST));
/**
 * Destroy a funge-space.
 */
void FungeSpaceFree(void) FUNGE_ATTR_FAST;
/**
 * Get a cell.
 */
FUNGEDATATYPE FungeSpaceGet(const fungePosition * restrict position) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Get a cell, with an offset.
 */
FUNGEDATATYPE FungeSpaceGetOff(const fungePosition * restrict position,
                               const fungePosition * restrict offset) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Set a cell.
 */
void FungeSpaceSet(FUNGEDATATYPE value,
                   const fungePosition * restrict position) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Set a cell, with an offset.
 */
void FungeSpaceSetOff(FUNGEDATATYPE value,
                      const fungePosition * restrict position,
                      const fungePosition * restrict offset) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Used for IP wrapping.
 */
void FungeSpaceWrap(fungePosition * restrict position,
                    const fungeVector * restrict delta) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Load a file into funge-space at 0,0. Optimised, use when possible.
 */
bool FungeSpaceLoad(const char * restrict filename) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));

/**
 * Load a file into funge space at an offset. Used for the i instruction.
 * size is an out variable.
 */
bool FungeSpaceLoadAtOffset(const char * restrict filename,
                            const fungePosition * restrict offset,
                            fungeVector * restrict size,
                            bool binary) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Write out a file from an area of funge space at an offset. Used for the o
 * instruction.
 */
bool FungeSpaceSaveToFile(const char          * restrict filename,
                          const fungePosition * restrict offset,
                          const fungeVector   * restrict size,
                          bool textfile) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));

/**
 * Get the bounding rectangle for the part of funge-space that isn't empty.
 * It won't be too small, but it may be too big.
 */
void FungeSpaceGetBoundRect(fungeRect * restrict rect) __attribute__((nonnull, FUNGE_IN_FAST));

#endif

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
 * Contains functions to access Funge-Space.
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
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
bool FungeSpaceCreate(void);
/**
 * Destroy a funge-space.
 */
FUNGE_ATTR_FAST
void FungeSpaceFree(void);
/**
 * Get a cell.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
FUNGEDATATYPE FungeSpaceGet(const fungePosition * restrict position);
/**
 * Get a cell, with an offset.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
FUNGEDATATYPE FungeSpaceGetOff(const fungePosition * restrict position,
                               const fungePosition * restrict offset);
/**
 * Set a cell.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void FungeSpaceSet(FUNGEDATATYPE value,
                   const fungePosition * restrict position);
/**
 * Set a cell, with an offset.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void FungeSpaceSetOff(FUNGEDATATYPE value,
                      const fungePosition * restrict position,
                      const fungePosition * restrict offset);
/**
 * Used for IP wrapping.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void FungeSpaceWrap(fungePosition * restrict position,
                    const fungeVector * restrict delta);
/**
 * Load a file into funge-space at 0,0. Optimised, use when possible.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool FungeSpaceLoad(const char * restrict filename);

/**
 * Load a file into funge space at an offset. Used for the i instruction.
 * size is an out variable.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool FungeSpaceLoadAtOffset(const char * restrict filename,
                            const fungePosition * restrict offset,
                            fungeVector * restrict size,
                            bool binary);
/**
 * Write out a file from an area of funge space at an offset. Used for the o
 * instruction.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool FungeSpaceSaveToFile(const char          * restrict filename,
                          const fungePosition * restrict offset,
                          const fungeVector   * restrict size,
                          bool textfile);

/**
 * Get the bounding rectangle for the part of funge-space that isn't empty.
 * It won't be too small, but it may be too big.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void FungeSpaceGetBoundRect(fungeRect * restrict rect);

#endif

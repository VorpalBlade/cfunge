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

/**
 * Create a funge-space.
 */
bool fungeSpaceCreate(void) __attribute__((warn_unused_result));
/**
 * Destroy a funge-space.
 */
void fungeSpaceFree(void);
/**
 * Get a cell.
 */
FUNGEDATATYPE fungeSpaceGet(const fungePosition * restrict position) __attribute__((nonnull,warn_unused_result));
/**
 * Get a cell, with an offset.
 */
FUNGEDATATYPE fungeSpaceGetOff(const fungePosition * restrict position,
                               const fungePosition * restrict offset) __attribute__((nonnull,warn_unused_result));
/**
 * Set a cell.
 */
void fungeSpaceSet(FUNGEDATATYPE value,
                   const fungePosition * restrict position) __attribute__((nonnull));
/**
 * Set a cell, with an offset.
 */
void fungeSpaceSetOff(FUNGEDATATYPE value,
                      const fungePosition * restrict position,
                      const fungePosition * restrict offset) __attribute__((nonnull));
/**
 * Used for IP wrapping.
 */
void fungeSpaceWrap(fungePosition * restrict position,
                    const fungeVector * restrict delta) __attribute__((nonnull));
/**
 * Load a file into funge-space at 0,0. Optimized, use when possible.
 */
bool fungeSpaceLoad(const char * restrict filename) __attribute__((nonnull,warn_unused_result));

/**
 * Load a file into funge space at an offset. Used for i instruction.
 * size is an out variable.
 */
bool fungeSpaceLoadAtOffset(const char * restrict filename,
                            const fungePosition * restrict offset,
                            fungeVector * restrict size,
                            bool binary) __attribute__((nonnull,warn_unused_result));

bool fungeSaveToFile(const char          * restrict filename,
                     const fungePosition * restrict offset,
                     const fungeVector   * restrict size,
                     bool textfile) __attribute__((nonnull,warn_unused_result));

/**
 * Get the bounding rectangle for the part of funge-space that isn't empty.
 * It won't be too small, but it may be too big.
 */
void fungeSpaceGetBoundRect(fungeRect * restrict rect) __attribute__((nonnull));

#endif

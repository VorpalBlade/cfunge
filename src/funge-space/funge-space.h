/*
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

struct _fungeSpace;
typedef struct _fungeSpace fungeSpace;

/**
 * Create a funge-space.
 */
extern bool fungeSpaceCreate(void) __attribute__((warn_unused_result));
/**
 * Destroy a funge-space.
 */
extern void fungeSpaceFree(void);
/**
 * Get a cell.
 */
extern FUNGEDATATYPE fungeSpaceGet(const fungePosition * restrict position) __attribute__((nonnull,warn_unused_result));
/**
 * Get a cell, with an offset.
 */
extern FUNGEDATATYPE fungeSpaceGetOff(const fungePosition * restrict position,
                                      const fungePosition * restrict offset) __attribute__((nonnull,warn_unused_result));
/**
 * Set a cell.
 */
extern void fungeSpaceSet(FUNGEDATATYPE value,
                          const fungePosition * restrict position) __attribute__((nonnull));
/**
 * Set a cell, with an offset.
 */
extern void fungeSpaceSetOff(FUNGEDATATYPE value,
                             const fungePosition * restrict position,
                             const fungePosition * restrict offset) __attribute__((nonnull));
/**
 * Used for IP wrapping.
 */
extern void fungeSpaceWrap(fungePosition * restrict position,
                           const fungeVector * restrict delta) __attribute__((nonnull));
/**
 * Load a file into fungespace.
 */
extern bool fungeSpaceLoad(const char * restrict filename) __attribute__((nonnull,warn_unused_result));
/**
 * Get the bounding rectangle for the part of funge-space that isn't empty.
 * It won't be too small, but it may be too big.
 */
extern void fungeSpaceGetBoundRect(fungeRect * restrict rect) __attribute__((nonnull));

#endif

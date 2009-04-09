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

/**
 * @file
 * Contains functions to access Funge-Space.
 */

#ifndef FUNGE_HAD_SRC_FUNGE_SPACE_H
#define FUNGE_HAD_SRC_FUNGE_SPACE_H

#include "../global.h"
#include "../vector.h"
#include "../rect.h"
#include <stdint.h>
#include <stdbool.h>

/// DO NOT CHANGE unless you are 100 sure of what you are doing!
/// Yes I mean you!
typedef funge_vector fungeSpaceHashKey;

/**
 * Create a Funge-space.
 * @warning Should only be called from internal setup code.
 * @return True if successful, otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
bool fungespace_create(void);
/**
 * Destroy a Funge-space.
 * @warning Should only be called from internal tear-down code.
 */
FUNGE_ATTR_FAST
void fungespace_free(void);
/**
 * Get a cell.
 * @param position The place in Funge-Space to get the value for.
 * @return The value for that position.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
funge_cell fungespace_get(const funge_vector * restrict position);
/**
 * Get a cell, with an offset. Mostly used to handle storage offset.
 * @param position The place in Funge-Space to get the value for.
 * @param offset An additional offset to add to the position.
 * @return The value for that position after adding offset.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
funge_cell fungespace_get_offset(const funge_vector * restrict position,
                                 const funge_vector * restrict offset);
/**
 * Set a cell.
 * @param value The value to set.
 * @param position The place in Funge-Space to set the value for.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void fungespace_set(funge_cell value,
                    const funge_vector * restrict position);
/**
 * Set a cell, with an offset. Mostly used to handle storage offset.
 * @param value The value to set.
 * @param position The place in Funge-Space to set the value for.
 * @param offset An additional offset to add to the position.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void fungespace_set_offset(funge_cell value,
                           const funge_vector * restrict position,
                           const funge_vector * restrict offset);
/**
 * Calculate the new position after adding a delta to a position, considering
 * any needed wrapping. Used for IP wrapping.
 * @param position Position before change, will be modified in place.
 * @param delta The delta to add to this position.
 * @note
 * This code modifies the position vector in place!
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void fungespace_wrap(funge_vector * restrict position,
                     const funge_vector * restrict delta);
/**
 * Load a file into Funge-Space at 0,0. Optimised compared to
 * fungespace_load_at_offset(). Only used for loading initial file.
 * @param filename Filename to load.
 * @return True if successful, otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool fungespace_load(const char * restrict filename);

#ifdef FUNGE_EXTERNAL_LIBRARY
/**
 * Load a string into Funge-Space at 0,0. Optimised. This code is used
 * internally by cfunge itself but is not usually exposed. It is however needed
 * for IFFI (using cfunge as a library in C-INTERCAL).
 * @param program Program to load.
 * @param length  Length of string, needed since code need to be embedded
 * null bytes, thus strlen() won't work.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void fungespace_load_string(const unsigned char * restrict program,
                            size_t length);
#endif

/**
 * Load a file into Funge-Space at an offset. Used for the i instruction.
 * @param filename Filename to load.
 * @param offset The offset to load the file at.
 * @param size This variable will be filled in by the function with the width
 * and height of the "bounding rectangle" that the file was loaded in.
 * @param binary If true newlines will be put into Funge-Space and won't
 * increment the y "counter".
 * @return True if successful, otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool fungespace_load_at_offset(const char * restrict filename,
                               const funge_vector * restrict offset,
                               funge_vector * restrict size,
                               bool binary);
/**
 * Write out a file from an area of Funge-Space at an offset. Used for the o
 * instruction.
 * @param filename Filename to write to.
 * @param offset The offset to write the file from.
 * @param size The width and height of the area to write out.
 * @param textfile If true will strip any spaces from end of lines and also
 * strip trailing newlines.
 * @return True if successful, otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool fungespace_save_to_file(const char         * restrict filename,
                             const funge_vector * restrict offset,
                             const funge_vector * restrict size,
                             bool textfile);

/**
 * Get the bounding rectangle for the part of Funge-Space that isn't empty.
 * @note It won't be too small, but it may be too big.
 * @param rect Out parameter for the bounding rectangle.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void fungespace_get_bounds_rect(fungeRect * restrict rect);

#endif

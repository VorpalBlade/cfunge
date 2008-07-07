/*
    This code was taken (and slightly modified to compile in cfunge) from:
       CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2008 Crossfire Development Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The authors can be reached via e-mail at crossfire-devel@real-time.com
*/

/** @file stringbuffer.h
 *
 * Implements a general string buffer: it builds a string by concatenating. It
 * allocates enough memory to hold the whole string; there is no upper limit
 * for the total string length.
 *
 * Usage is:
 * <code>
 * StringBuffer *sb = stringbuffer_new();
 * stringbuffer_append_string(sb, "abc");
 * stringbuffer_append_string(sb, "def");
 * ... more calls to stringbuffer_append_xxx()
 * char *str = stringbuffer_finish(sb)
 * ... use str
 * free(str);
 * </code>
 *
 */

#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include "../../src/global.h"


/**
 * The string buffer state.
 */
typedef struct StringBuffer StringBuffer;


/**
 * Create a new string buffer.
 *
 * @return The newly allocated string buffer.
 */
StringBuffer *stringbuffer_new(void);

/**
 * Deallocate the string buffer instance and return the string.
 *
 * The passed string buffer must not be accessed afterwards.
 *
 * @param sb The string buffer to deallocate.
 *
 * @return The result string; to free it, call <code>free()</code> on it.
 */
char *stringbuffer_finish(StringBuffer *sb);


/**
 * Append a string to a string buffer instance.
 *
 * @param sb The string buffer to modify.
 *
 * @param str The string to append.
 */
void stringbuffer_append_string(StringBuffer *sb, const char *str);

/**
 * Append a formatted string to a string buffer instance.
 *
 * @param sb The string buffer to modify.
 *
 * @param format The format string to append.
 */
__attribute__((format(printf, 2, 3)))
void stringbuffer_append_printf(StringBuffer *sb, const char *format, ...);

/**
 * Append the contents of a string buffer instance to another string buffer
 * instance.
 *
 * @param sb The string buffer to modify.
 *
 * @param sb2 The string buffer to append; it must be different from sb.
 */
void stringbuffer_append_stringbuffer(StringBuffer *sb, const StringBuffer *sb2);

#endif

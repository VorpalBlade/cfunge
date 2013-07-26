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

/*
 * Modifications for cfunge:
 * - Drop some crossfire specific includes and functions that depended on other
 *   parts of crossfire.
 * - Adding stringbuffer_destroy
 * - Adding GCC attributes.
 * - Convert to using Funge multibyte.
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/global.h"
#include <stdbool.h>
#include "stringbuffer.h"


struct StringBuffer {
    /**
     * The string buffer. The first {@link #pos} bytes contain the collected
     * string. It's size is at least {@link #size} bytes.
     */
    funge_cell *buf;

    /**
     * The current length of {@link #buf}. The invariant <code>pos <
     * size</code> always holds; this means there is always enough room to
     * attach a trailing \0 character.
     */
    size_t pos;

    /**
     * The allocation size of {@link #buf}.
     */
    size_t size;
};


/**
 * Make sure that at least <code>len</code> bytes are available in the passed
 * string buffer.
 *
 * @param sb The string buffer to modify.
 *
 * @param len The number of bytes to allocate.
 */
FUNGE_ATTR_FAST
static bool stringbuffer_ensure(StringBuffer *sb, size_t len);

FUNGE_ATTR_FAST
StringBuffer *stringbuffer_new(void)
{
    StringBuffer *sb;

    sb = malloc(sizeof(*sb));
    if (sb == NULL) {
        return NULL;
    }

    sb->size = 256;
    sb->buf = malloc(sb->size*sizeof(funge_cell));
    if (sb->buf == NULL) {
        free(sb);
        return NULL;
    }
    sb->pos = 0;
    return sb;
}

FUNGE_ATTR_FAST
void stringbuffer_destroy(StringBuffer *sb)
{
    free(sb->buf);
    free(sb);
}

FUNGE_ATTR_FAST
char *stringbuffer_finish(StringBuffer * restrict sb, size_t * restrict length)
{
    char *buffer = malloc(sb->pos+1);
    if (!buffer)
        return NULL;

    for (size_t i = 0; i < sb->pos; i++)
        buffer[i] = (char)sb->buf[i];

    buffer[sb->pos] = '\0';
    
    if (length)
        *length = sb->pos;
    free(sb->buf);
    free(sb);
    return buffer;
}

FUNGE_ATTR_FAST
funge_cell *stringbuffer_finish_multibyte(StringBuffer * restrict sb, size_t * restrict length)
{
    funge_cell *result;

    sb->buf[sb->pos] = '\0';
    if (length)
        *length = sb->pos;
    result = sb->buf;
    free(sb);
    return result;
}

FUNGE_ATTR_FAST
void stringbuffer_append_char(StringBuffer *sb, const char c)
{
    stringbuffer_ensure(sb, 2);
    sb->buf[sb->pos] = c;
    sb->pos += 1;
}

FUNGE_ATTR_FAST
void stringbuffer_append_cell(StringBuffer *sb, const funge_cell c)
{
    stringbuffer_ensure(sb, 2);
    sb->buf[sb->pos] = c;
    sb->pos += 1;
}

FUNGE_ATTR_FAST
void stringbuffer_append_string(StringBuffer *sb, const char *str)
{
    size_t len;

    len = strlen(str);
    stringbuffer_ensure(sb, len+1);
    for (size_t i = 0; i < len; i++)
        sb->buf[sb->pos+i] = str[i];
    sb->pos += len;
}

FUNGE_ATTR_FAST
bool stringbuffer_append_printf(StringBuffer *sb, const char *format, ...)
{
    size_t size = 100;                 /* arbitrary guess */
    char *buffer = malloc(size);
    if (!buffer)
        return false;

    for (;;) {
        int n;
        va_list arg;
        char* new_buffer = realloc(buffer, size);

        if (!new_buffer)
        {
            free(buffer);
            return false;
        }
        buffer = new_buffer;

        va_start(arg, format);
        n = vsnprintf(buffer, size, format, arg);
        va_end(arg);

        if (n > -1 && (size_t)n < size) {
            stringbuffer_append_string(sb, buffer);
            free(buffer);
            break;
        }

        if (n > -1) {
            size = (size_t)(n+1);         /* precisely what is needed */
        } else {
            size *= 2;          /* twice the old size */
        }
    }
    return true;
}

FUNGE_ATTR_FAST
bool stringbuffer_append_stringbuffer(StringBuffer * restrict sb,
                                      const StringBuffer * restrict sb2)
{
    if (!stringbuffer_ensure(sb, sb2->pos+1))
        return false;
    memcpy(sb->buf+sb->pos, sb2->buf, sb2->pos);
    sb->pos += sb2->pos;
    return true;
}

FUNGE_ATTR_FAST
static bool stringbuffer_ensure(StringBuffer *sb, size_t len)
{
    funge_cell *tmp;
    size_t new_size;

    if (sb->pos+len <= sb->size) {
        return true;
    }

    new_size = sb->pos+len+256;
    tmp = realloc(sb->buf, new_size * sizeof(funge_cell));
    if (tmp == NULL) {
        return false;
    }
    sb->buf = tmp;
    sb->size = new_size;
    return true;
}

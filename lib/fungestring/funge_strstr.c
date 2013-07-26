/*
    This code was taken (and modified to work for cfunge) from:
      EGLIBC (http://www.eglibc.org/)

   Return the offset of one string within another.
   Copyright (C) 1994-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This particular implementation was written by Eric Blake, 2008.  */

#include "../../src/global.h"
#include "funge_string.h"

#include <stdbool.h>

#define RETURN_TYPE funge_cell *
#define AVAILABLE(h, h_l, j, n_l)                        \
  (!funge_memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))        \
   && ((h_l) = (j) + (n_l)))
#define CHECK_EOL (1)
#define RET0_IF_0(a) if (!a) goto ret0
#include "funge_str-two-way.h"

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE is empty, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE
funge_cell *
funge_strstr (const funge_cell *haystack_start,
              const funge_cell *needle_start)
{
  const funge_cell *haystack = haystack_start;
  const funge_cell *needle = needle_start;
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */
  bool ok = true; /* True if NEEDLE is prefix of HAYSTACK.  */

  /* Determine length of NEEDLE, and in the process, make sure
     HAYSTACK is at least as long (no point processing all of a long
     NEEDLE if HAYSTACK is too short).  */
  while (*haystack && *needle)
    ok &= *haystack++ == *needle++;
  if (*needle)
    return NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  if (ok)
    return (funge_cell *) haystack_start;
#pragma GCC diagnostic pop

  /* Reduce the size of haystack using strchr, since it has a smaller
     linear coefficient than the Two-Way algorithm.  */
  needle_len = needle - needle_start;
  haystack = funge_strchr(haystack_start + 1, *needle_start);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  if (!haystack || FUNGE_EXPECT (needle_len == 1, 0))
    return (funge_cell *) haystack;
#pragma GCC diagnostic pop
  needle -= needle_len;
  haystack_len = (haystack > haystack_start + needle_len ? 1
                  : needle_len + haystack_start - haystack);

  /* Perform the search.  Abstract memory is considered to be an array
     of 'unsigned char' values, not an array of 'char' values.  See
     ISO C 99 section 6.2.6.1.  */
  return two_way_short_needle ((const funge_unsigned_cell *) haystack,
                               haystack_len,
                               (const funge_unsigned_cell *) needle, needle_len);
}

#undef LONG_NEEDLE_THRESHOLD

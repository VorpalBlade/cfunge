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

// Do not include directly, include global.h instead. It includes
// this file.
#ifndef _HAD_SRC_SUPPORT_H
#define _HAD_SRC_SUPPORT_H

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#ifndef DISABLE_GC

#  include <gc/gc.h>

#  define cf_malloc(x)           GC_MALLOC(x)
// Use this for strings and other stuff containing no pointers when possible.
#  define cf_malloc_noptr(x)     GC_MALLOC_ATOMIC(x)
// This memory is not collectable. Avoid using this unless you have to.
#  define cf_malloc_nocollect(x) GC_MALLOC_UNCOLLECTABLE(x)
#  define cf_free(x)             GC_FREE(x)
#  define cf_realloc(x,y)        GC_REALLOC(x, y)
#  define cf_calloc(x,y)         GC_MALLOC((x)*(y))
#  define cf_calloc_noptr(x,y)   GC_MALLOC_ATOMIC((x)*(y))

#  define gc_collect_full()      GC_gcollect()
#  define gc_collect_some()      GC_collect_a_little()

#  define cf_strdup(x)           GC_STRDUP(x)

#else

#  define cf_malloc(x)           malloc(x)
// Use this for strings and other stuff containing no pointers when possible.
#  define cf_malloc_noptr(x)     malloc(x)
// This memory is not collectable. Avoid using this unless you have to.
#  define cf_malloc_nocollect(x) malloc(x)
#  define cf_free(x)             free(x)
#  define cf_realloc(x,y)        realloc(x, y)
#  define cf_calloc(x,y)         calloc((x), (y))
#  define cf_calloc_noptr(x,y)   calloc((x), (y))

#  define gc_collect_full()      /* NO OP */
#  define gc_collect_some()      /* NO OP */

#  define cf_strdup(x)           strdup(x)

#endif

// Use these only if you have to, for example if some external library
// did the malloc
#define malloc_nogc(x)         malloc((x))
#define calloc_nogc(x,y)       calloc((x), (y))
#define realloc_nogc(x,y)      realloc((x), (y))
#define free_nogc(x)           free(x);

char * cf_strndup(char const *string, size_t n) __attribute__((warn_unused_result, FUNGE_IN_FAST));
size_t cf_strnlen(const char *string, size_t maxlen) FUNGE_FAST;

// This is glibc specific, so here is a version from gnulib.
ssize_t cf_getline(char **lineptr, size_t *n, FILE *stream) FUNGE_FAST;

// Yep, crap.
#ifdef __WIN32__
#  define random rand
#  define srandom srand
#endif

#endif

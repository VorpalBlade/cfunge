/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

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

#include <gc/gc.h>

#define cf_malloc(x)           GC_MALLOC(x)
// Use this for strings and other stuff containing no pointers when possible.
#define cf_malloc_noptr(x)     GC_MALLOC_ATOMIC(x)
// This memory is not collectable. Avoid using this unless you have to.
#define cf_malloc_nocollect(x) GC_MALLOC_UNCOLLECTABLE(x)
#define cf_free(x)             GC_FREE(x)
#define cf_realloc(x,y)        GC_REALLOC(x, y)
#define cf_calloc(x,y)         GC_MALLOC((x)*(y))
#define cf_calloc_noptr(x,y)   GC_MALLOC_ATOMIC((x)*(y))

#define gc_collect_full()      GC_gcollect()
#define gc_collect_some()      GC_collect_a_little()

// Use these only if you have to, for example if some external library
// did the malloc
#define malloc_nogc(x)         malloc(x)
#define calloc_nogc(x,y)       calloc(x, y)
#define realloc_nogc(x,y)      realloc(x, y)
#define free_nogc(x)           free(x);


#define cf_strdup(x)       GC_STRDUP(x)
char * cf_strndup(char const *string, size_t n) __attribute__((warn_unused_result));

#endif

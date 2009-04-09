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
 * Support routines to handle different systems and similar.
 * @warning
 * Never include this file directly, it is included by global.h
 * @note
 * This file contains stuff from Gnulib that we want and some
 * defines to support different platforms and configurations.
 */

#ifndef FUNGE_HAD_SRC_SUPPORT_H
#define FUNGE_HAD_SRC_SUPPORT_H

#include <sys/types.h> /* ssize_t */
#include <stdio.h>     /* FILE* (in cf_getline) */
#include <string.h>
#include <unistd.h>    /* _POSIX_THREAD_SAFE_FUNCTIONS */

// GC may cause threading to happen...
// ...but without threading we can avoid locking for some stuff.
#ifdef CFUN_USE_GC
#  define NEED_LOCKED
#else
#  undef NEED_LOCKED
#endif

/**
 * @defgroup MEMORY_ALLOC Memory allocation
 * These should be used when allocating memory
 */
/*@{*/
#ifdef CFUN_USE_GC

#  include <gc/gc.h>

/// If built with GC support this will malloc using GC.
#  define cf_malloc(x)           GC_MALLOC(x)
/// Use this for strings and other stuff containing no pointers when possible.
#  define cf_malloc_noptr(x)     GC_MALLOC_ATOMIC(x)
/// This memory is not collectable when GC is enabled. Avoid using this unless
/// you have to.
#  define cf_malloc_nocollect(x) GC_MALLOC_UNCOLLECTABLE(x)
/// Use this free when you used cf_malloc().
#  define cf_free(x)             GC_FREE(x)
/// Realloc for cf_malloc().
#  define cf_realloc(x,y)        GC_REALLOC(x, y)
/// If built with GC support this will calloc using GC. It is an alias to
/// GC_MALLOC(), because GC always zero out the memory.
#  define cf_calloc(x,y)         GC_MALLOC((x)*(y))
/// See cf_malloc_noptr for what _noptr means.
#  define cf_calloc_noptr(x,y)   GC_MALLOC_ATOMIC((x)*(y))

/// Collect memory if GC is used
#  define gc_collect_full()      GC_gcollect()
/// Use this macro instead of plain strdup, or even better use cf_strndup.
#  define cf_strdup(x)           GC_STRDUP(x)

/// Used to mark some static areas as containing no pointers.
#  define cf_mark_static_noptr(min, max) GC_exclude_static_roots(min, max)

#else

/// If built with GC support this will malloc using GC.
#  define cf_malloc(x)           malloc(x)
/// Use this for strings and other stuff containing no pointers when possible.
/// Differ from normal malloc if GC is enabled.
#  define cf_malloc_noptr(x)     malloc(x)
/// This memory is not collectable when GC is enabled. Avoid using this unless
/// you have to. Differ from normal malloc if GC is enabled.
#  define cf_malloc_nocollect(x) malloc(x)
/// Use this free when you used cf_malloc().
#  define cf_free(x)             free(x)
/// Realloc for cf_malloc().
#  define cf_realloc(x,y)        realloc(x, y)
/// If built with GC support this will calloc using GC.
#  define cf_calloc(x,y)         calloc((x), (y))
/// See cf_malloc_noptr for what _noptr means.
#  define cf_calloc_noptr(x,y)   calloc((x), (y))

/// Collect memory if GC is used
#  define gc_collect_full()      /* NO OP */
/// Use this macro instead of plain strdup, or even better use cf_strndup.
#  define cf_strdup(x)           strdup(x)

/// Used to mark some static areas as containing no pointers.
#  define cf_mark_static_noptr(min, max) /* NO OP */

#endif

/// Malloc without GC. Use this only if you have to, for example if some
/// external library need it.
#define malloc_nogc(x)         malloc((x))
/// Calloc without GC. Use this only if you have to, for example if some
/// external library need it.
#define calloc_nogc(x,y)       calloc((x), (y))
/// Realloc without GC. Use this only if you have to, for example if some
/// external library did the malloc.
#define realloc_nogc(x,y)      realloc((x), (y))
/// Free without GC. Use this only if you have to, for example if some external
/// library did the malloc.
#define free_nogc(x)           free(x);
/// Strdup without GC. Use this only if you have to, for example if some
/// external library need it.
#define strdup_nogc(x)         strdup((x))
/*@}*/

/// getline is glibc specific, so here is a version from gnulib.
FUNGE_ATTR_FAST
ssize_t cf_getline(char **lineptr, size_t *n, FILE *stream);

/**
 * @defgroup STDIO_UNLOCKED Unlocked I/O defines
 * These are useful in case of a lot of IO operations.
 */
/*@{*/
#if !defined(__CYGWIN__) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && (_POSIX_THREAD_SAFE_FUNCTIONS > 0)
#  define cf_getc_unlocked(x)              getc_unlocked((x))
#  define cf_putc_unlocked(x, y)           putc_unlocked((x), (y))
#  define cf_putchar_unlocked(x)           putchar_unlocked((x))
#  ifdef NEED_LOCKED
#    define cf_flockfile(x)                flockfile((x))
#    define cf_funlockfile(x)              funlockfile((x))
/// putchar that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#    define cf_putchar_maybe_locked(x)     putchar((x))
/// putc that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#    define cf_putc_maybe_locked(x, y)     putc((x), (y))
#  else /* NEED_LOCKED */
#    define cf_flockfile(x)                /* NO-OP */
#    define cf_funlockfile(x)              /* NO-OP */
/// putchar that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#    define cf_putchar_maybe_locked(x)     putchar_unlocked((x))
/// putc that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#    define cf_putc_maybe_locked(x, y)     putc_unlocked((x), (y))
#  endif /* NEED_LOCKED */
#else /* _POSIX_THREAD_SAFE_FUNCTIONS */
#  define cf_getc_unlocked(x)              getc((x))
#  define cf_putc_unlocked(x, y)           putc((x), (y))
#  define cf_putchar_unlocked(x)           putchar((x))
#  define cf_flockfile(x)                  /* NO-OP */
#  define cf_funlockfile(x)                /* NO-OP */
/// putchar that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#  define cf_putchar_maybe_locked(x)       putchar((x))
/// putc that will be locked only if the app could contain threads
/// (for example Boehm-GC could lead to that).
#  define cf_putc_maybe_locked(x, y)       putc((x), (y))
#endif /* _POSIX_THREAD_SAFE_FUNCTIONS */
/*@}*/

#if !defined(HAVE_random) || !defined(HAVE_srandom)
#  define random rand
#  define srandom srand
#endif

#ifdef __WIN32__
#  error "Windows is unsupported, please use POSIX. No idea if cygwin works, it is on your own risk!"
#endif

#endif

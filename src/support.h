/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2011 Arvid Norlander <anmaster AT tele2 DOT se>
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

/// getline is glibc specific, so here is a version from gnulib.
FUNGE_ATTR_FAST
ssize_t cf_getline(char **lineptr, size_t *n, FILE *stream);

#if !defined(HAVE_strlcpy)
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
size_t strlcpy(char *dst, const char *src, size_t siz);
#elif defined(STRLCPY_IN_BSD)
#  include <bsd/string.h>
#endif

/// Support macro for extra assertions.
/// Currently mostly meant for klee, but this might change in the future.
#if defined(CFUN_KLEE_TEST)
#  include <klee/klee.h>
#  define paranoid_assert(expr_) (klee_assert(expr_))
#elif defined(CFUN_PARANOIA)
#  include <assert.h>
#  define paranoid_assert(expr_) (assert(expr_))
#else
#  define paranoid_assert(expr_) ((void)(0))
#endif

/**
 * @defgroup STDIO_UNLOCKED Unlocked I/O defines
 * These are useful in case of a lot of IO operations.
 */
/*@{*/
#if !defined(__CYGWIN__) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && (_POSIX_THREAD_SAFE_FUNCTIONS > 0)
#  define cf_getc_unlocked(x)              getc_unlocked((x))
#  define cf_putc_unlocked(x, y)           putc_unlocked((x), (y))
#  define cf_putchar_unlocked(x)           putchar_unlocked((x))
#else /* _POSIX_THREAD_SAFE_FUNCTIONS */
#  define cf_getc_unlocked(x)              getc((x))
#  define cf_putc_unlocked(x, y)           putc((x), (y))
#  define cf_putchar_unlocked(x)           putchar((x))
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

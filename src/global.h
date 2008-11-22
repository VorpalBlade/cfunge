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

/**
 * @file
 * Global defines. This file should be included everywhere.
 */

#ifndef FUNGE_HAD_SRC_GLOBAL_H
#define FUNGE_HAD_SRC_GLOBAL_H

/**
 * @defgroup compat Compiler/system compatibility defines.
 * Compatibility stuff to support systems lacking some functions of features.
 */
/*@{*/
#if !defined(__GNUC__) && !defined(__INTEL_COMPILER)
#  undef __attribute__
/// Make non-GCC compilers happy.
#  define  __attribute__(x)  /* NO-OP */
#endif

#if defined(__GNUC__) && (__GNUC__ >= 3) && !defined(__INTEL_COMPILER)
/// Give compiler a hint about the most common outcome.
/// Please do not add this unless you have profiled.
#  define FUNGE_EXPECT(expr, outcome) __builtin_expect(expr,outcome)
#else
#  define FUNGE_EXPECT(expr, outcome) (expr)
#endif /* __GNUC__ */

/*@}*/


/**
 * @defgroup attributes Attribute specifications.
 * Contains attribute specifications.
 */
/*@{*/
#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
#  ifdef __i386__
/// Used to select fast calling convention on platforms that need it.
#    define  FUNGE_ATTR_FAST __attribute__((regparm(3)))
#  else
/// Used to select fast calling convention on platforms that need it.
#    define  FUNGE_ATTR_FAST /* NO-OP */
#  endif
#else
/// Used to select fast calling convention on platforms that need it.
#  define  FUNGE_ATTR_FAST /* NO-OP */
#endif

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
#  define FUNGE_ATTR_CONST         __attribute__((const))
#  define FUNGE_ATTR_ALWAYS_INLINE __attribute__((always_inline))
#  define FUNGE_ATTR_MALLOC        __attribute__((malloc))
#  define FUNGE_ATTR_NOINLINE      __attribute__((noinline))
#  define FUNGE_ATTR_NONNULL       __attribute__((nonnull))
#  define FUNGE_ATTR_NORET         __attribute__((noreturn))
#  define FUNGE_ATTR_PURE          __attribute__((pure))
#  define FUNGE_ATTR_UNUSED        __attribute__((unused))
#  define FUNGE_ATTR_WARN_UNUSED   __attribute__((warn_unused_result))
#else
#  define FUNGE_ATTR_CONST         /* NO-OP */
#  define FUNGE_ATTR_ALWAYS_INLINE /* NO-OP */
#  define FUNGE_ATTR_MALLOC        /* NO-OP */
#  define FUNGE_ATTR_NOINLINE      /* NO-OP */
#  define FUNGE_ATTR_NONNULL       /* NO-OP */
#  define FUNGE_ATTR_NORET         /* NO-OP */
#  define FUNGE_ATTR_PURE          /* NO-OP */
#  define FUNGE_ATTR_UNUSED        /* NO-OP */
#  define FUNGE_ATTR_WARN_UNUSED   /* NO-OP */
#endif

/*@}*/

#include <stdlib.h>
#include "support.h"
#include <stdint.h>
#include <inttypes.h>


/**
 * @defgroup FUNGEDATA Funge data types
 * The type/size of the data cells, and various related things
 */
/*@{*/
#if defined(USE64)
/// The type of the data cells (64-bit version).
typedef int64_t  fungeCell;
/// As fungeCell, but unsigned (64-bit version).
typedef uint64_t fungeUnsignedCell;
/// Generic printf code to use.
#  define FUNGECELLPRI PRId64
/// Printf code for octal output.
#  define FUNGECELLoctPRI PRIo64
/// Printf code for hexdecimal output.
#  define FUNGECELLhexPRI PRIx64
/// Max value for fungeCell
#  define FUNGECELL_MIN INT64_MIN
/// Min value for fungeCell
#  define FUNGECELL_MAX INT64_MAX

/**
 * Define the abs() function to use for the set data size.
 * This one is 64-bit.
 */
#  define ABS(x) llabs(x)


#elif defined(USE32)
/// The type of the data cells (32-bit version).
typedef int32_t fungeCell;
/// As fungeCell, but unsigned (32-bit version).
typedef uint32_t fungeUnsignedCell;
/// Generic printf code to use.
#  define FUNGECELLPRI PRId32
/// Printf code for octal output.
#  define FUNGECELLoctPRI PRIo32
/// Printf code for hexdecimal output.
#  define FUNGECELLhexPRI PRIx32
/// Max value for fungeCell
#  define FUNGECELL_MIN INT32_MIN
/// Min value for fungeCell
#  define FUNGECELL_MAX INT32_MAX

/**
 * Define the abs() function to use for the set data size.
 * This one is 32-bit.
 */
#  define ABS(x) abs(x)


#else
#  error "Err, you actually got to select either 32-bit or 64-bit data type. If you used the normal build system this error should not happen."
#endif
/*@}*/


// This is done in other to make integration with IFFI easier for ais523.
#ifndef FUNGE_OLD_HANDPRINT
/// Funge-98 Handprint: CFUN
#  define FUNGE_OLD_HANDPRINT 0x4346554e
#endif
#ifndef FUNGE_NEW_HANDPRINT
/// Funge-108 Handprint: An URL.
#  define FUNGE_NEW_HANDPRINT "http://kuonet.org/~anmaster/cfunge"
#endif
#ifdef FUNGEHANDPRINT
#  error "Please rename FUNGEHANDPRINT to FUNGE_OLD_HANDPRINT and add FUNGE_NEW_HANDPRINT!"
#endif
/// Version, for -V.
#define APPVERSION  "0.3.3"
/// For y instruction.
#define FUNGEVERSION 33

// Define if you use fuzz testing script.
//#define FUZZ_TESTING

#endif

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
 * Global defines. This file should be included everywhere.
 */

#ifndef FUNGE_HAD_SRC_GLOBAL_H
#define FUNGE_HAD_SRC_GLOBAL_H

/**
 * @defgroup compiler Compiler identification.
 * Defines identifying the compiler.
 *
 * CFUNGE_COMP_GCC_COMPAT    - GCC or claims to be GCC
 * CFUNGE_COMP_ICC           - Actually ICC
 * CFUNGE_COMP_CLANG         - Really clang.
 * CFUNGE_COMP_GCC           - In fact this is GCC. Or is someone pretending that
 *                             we don't know how to identify correctly.
 * CFUNGE_COMP_GCC3_COMPAT   - Claims to be GCC 3.x or later.
 * CFUNGE_COMP_GCC4_COMPAT   - Claims to be GCC 4 or later.
 * CFUNGE_COMP_GCC4_3_COMPAT - Claims to be GCC 4.3 or later.
 * CFUNGE_COMP_GCC4_4_COMPAT - Claims to be GCC 4.4 or later.
 */
/*@{*/
#ifdef __GNUC__
#  define CFUNGE_COMP_GCC_COMPAT
#  if defined(__INTEL_COMPILER)
#    define CFUNGE_COMP_ICC
#  elif defined(__clang__)
#    define CFUNGE_COMP_CLANG
#  else
#    define CFUNGE_COMP_GCC
#  endif
#  if (__GNUC__ >= 3)
#    define CFUNGE_COMP_GCC3_COMPAT
#  endif
#  if (__GNUC__ >= 4)
#    define CFUNGE_COMP_GCC4_COMPAT
#  endif
#  if (defined(CFUNGE_COMP_GCC4_COMPAT) && (__GNUC_MINOR__ >= 3)) || (__GNUC__ >= 5)
#    define CFUNGE_COMP_GCC4_3_COMPAT
#  endif
#  if (defined(CFUNGE_COMP_GCC4_COMPAT) && (__GNUC_MINOR__ >= 4)) || (__GNUC__ >= 5)
#    define CFUNGE_COMP_GCC4_4_COMPAT
#  endif
#endif
/*@}*/

/**
 * @defgroup architecture Architecture identification.
 * Defines identifying the compiler.
 *
 * CFUNGE_ARCH_X86    - 32-bit or 64-bit x86
 * CFUNGE_ARCH_X86_32 - 32-bit x86
 * CFUNGE_ARCH_X86_64 - 64-bit x86
 */
/*@{*/
#ifdef __i386__
#  define CFUNGE_ARCH_X86
#  define CFUNGE_ARCH_X86_32
#endif
#ifdef __x86_64__
#  define CFUNGE_ARCH_X86
#  define CFUNGE_ARCH_X86_64
#endif
/*@}*/

/**
 * @defgroup compat Compiler/system compatibility defines.
 * Compatibility stuff to support systems lacking some functions of features.
 */
/*@{*/
#ifdef CFUNGE_COMP_GCC_COMPAT
#  define FUNGE_ATTR(x) __attribute__(x)
#else
/// Make non-GCC compilers happy.
#  define FUNGE_ATTR(x)  /* NO-OP */
#endif

#if defined(CFUNGE_COMP_GCC3_COMPAT) && !defined(CFUNGE_COMP_ICC)
/// Give GCC a hint about the most common outcome.
/// Please do not add this unless you have profiled.
#  define FUNGE_EXPECT(expr, outcome) __builtin_expect((expr),(outcome))
#else
/// Give GCC a hint about the most common outcome.
/// Please do not add this unless you have profiled.
#  define FUNGE_EXPECT(expr, outcome) (expr)
#endif /* __GNUC__ */

/// Hints about likely outcome. Do not add unless you have profiled.
#define FUNGE_LIKELY(expr)   FUNGE_EXPECT(!!(expr), 1)
/// Hints about likely outcome. Do not add unless you have profiled.
#define FUNGE_UNLIKELY(expr) FUNGE_EXPECT(!!(expr), 0)

/*@}*/


/**
 * @defgroup attributes Attribute specifications.
 * Contains attribute specifications.
 */
/*@{*/
#ifdef CFUNGE_COMP_GCC
#  ifdef __i386__
/// Used to select fast calling convention on platforms that need it.
#    define FUNGE_ATTR_FAST FUNGE_ATTR((regparm(3)))
#  else
/// Used to select fast calling convention on platforms that need it.
#    define FUNGE_ATTR_FAST /* NO-OP */
#  endif
#else
/// Used to select fast calling convention on platforms that need it.
#  define FUNGE_ATTR_FAST /* NO-OP */
#endif

#ifdef CFUNGE_COMP_GCC
#  define FUNGE_ATTR_CONST         FUNGE_ATTR((const))
#  define FUNGE_ATTR_ALWAYS_INLINE FUNGE_ATTR((always_inline))
#  define FUNGE_ATTR_MALLOC        FUNGE_ATTR((malloc))
#  define FUNGE_ATTR_NOINLINE      FUNGE_ATTR((noinline))
#  define FUNGE_ATTR_NONNULL       FUNGE_ATTR((nonnull))
#  define FUNGE_ATTR_NORET         FUNGE_ATTR((noreturn))
#  define FUNGE_ATTR_PURE          FUNGE_ATTR((pure))
#  define FUNGE_ATTR_UNUSED        FUNGE_ATTR((unused))
#  define FUNGE_ATTR_WARN_UNUSED   FUNGE_ATTR((warn_unused_result))
#  define FUNGE_ATTR_ALIGNED(m_n)  FUNGE_ATTR((aligned(m_n)))
#  define FUNGE_ATTR_FORMAT(m_type, m_fmtidx, m_chkid) \
	FUNGE_ATTR((format(m_type, m_fmtidx, m_chkid)))
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
#  define FUNGE_ATTR_ALIGNED(m_n)  /* NO-OP */
#  define FUNGE_ATTR_FORMAT(m_type, m_fmtidx, m_chkid)  /* NO-OP */
#endif

#if defined(CFUNGE_COMP_GCC) && defined(CFUNGE_COMP_GCC4_3_COMPAT)
#  define FUNGE_ATTR_COLD FUNGE_ATTR((cold))
#  define FUNGE_ATTR_HOT  FUNGE_ATTR((hot))
#else
#  define FUNGE_ATTR_COLD /* NO-OP */
#  define FUNGE_ATTR_HOT  /* NO-OP */
#endif

/*@}*/

// I so hate the C preprocessor...
#define FUNGE_CPP_STRINGIFY_INNER(m_arg) # m_arg
/**
 * Stringifies argument, needed because the C preprocessor is so stupid.
 */
#define FUNGE_CPP_STRINGIFY(m_arg) FUNGE_CPP_STRINGIFY_INNER(m_arg)

/* stdlib.h: For ABS() below, as well as malloc/calloc/free in support.h */
#include <stdlib.h>
#include "support.h"
/* stdint.h: For int32_t/int64_t needed for funge_cell below. */
#include <stdint.h>
/* inttypes.h: For INT64_MIN and such below. */
#include <inttypes.h>


/**
 * @defgroup FUNGEDATA Funge data types
 * The type/size of the data cells, and various related things
 */
/*@{*/
#if defined(USE64)
/// The type of the data cells (64-bit version).
typedef int64_t  funge_cell;
/// As funge_cell, but unsigned (64-bit version).
typedef uint64_t funge_unsigned_cell;
/// Generic printf code to use.
#  define FUNGECELLPRI PRId64
/// Printf code for octal output.
#  define FUNGECELLoctPRI PRIo64
/// Printf code for hexdecimal output.
#  define FUNGECELLhexPRI PRIx64
/// Max value for funge_cell
#  define FUNGECELL_MIN INT64_MIN
/// Min value for funge_cell
#  define FUNGECELL_MAX INT64_MAX

/**
 * Define the abs() function to use for the set data size.
 * This one is 64-bit.
 */
#  define ABS(x) llabs(x)


#elif defined(USE32)
/// The type of the data cells (32-bit version).
typedef int32_t funge_cell;
/// As funge_cell, but unsigned (32-bit version).
typedef uint32_t funge_unsigned_cell;
/// Generic printf code to use.
#  define FUNGECELLPRI PRId32
/// Printf code for octal output.
#  define FUNGECELLoctPRI PRIo32
/// Printf code for hexdecimal output.
#  define FUNGECELLhexPRI PRIx32
/// Max value for funge_cell
#  define FUNGECELL_MIN INT32_MIN
/// Min value for funge_cell
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
/// Funge-109 Handprint: An URL.
#  define FUNGE_NEW_HANDPRINT "http://kuonet.org/~anmaster/cfunge"
#endif
#ifdef FUNGEHANDPRINT
#  error "Please rename FUNGEHANDPRINT to FUNGE_OLD_HANDPRINT and add FUNGE_NEW_HANDPRINT!"
#endif
/// Version, for -V.
#define CFUNGE_APPVERSION  "0.9.0"
/// For y instruction.
#define CFUNGE_VERSION_Y 90

/**
 * For third party code.
 * The format is 0xaabbccdd, where:
 *  - aa is major
 *  - bb is minor
 *  - cc is micro
 *  - dd is 0 for releases, and has a non-zero value for development versions
 *    after said release. Usually it will be 01, but it may be more if some
 *    major API breakage happened.
 *  This define was introduced between 0.3.3 and 0.3.4.
 */
#define CFUNGE_VERSION 0x00090000

/// Since there may be no API changes between versions we also define this.
#define CFUNGE_API_VERSION 3

// Define if you use fuzz testing script.
//#define FUZZ_TESTING

#endif

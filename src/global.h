/*
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

#ifndef _HAD_SRC_GLOBAL_H
#define _HAD_SRC_GLOBAL_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// For compatiblity with other compilers to prevent them
// failing at things like: __attribute__((noreturn))
#ifndef __GNUC__
#  define  __attribute__(x)  /* NO-OP */
#endif

#include <support.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


#ifdef USE64
// The type of the data cells
typedef int_fast64_t FUNGEDATATYPE;
#  define FUNGEDATAPRI PRIdFAST64
// And of vector values
typedef int_fast64_t FUNGEVECTORTYPE;
#  define FUNGEVECTORPRI PRIdFAST64
// FIXME: Will long long always be 64-bit?
#  define ABS(x) llabs(x)
#else
// The type of the data cells
typedef int32_t FUNGEDATATYPE;
#  define FUNGEDATAPRI PRId32
// And of vector values
typedef int32_t FUNGEVECTORTYPE;
#  define FUNGEVECTORPRI PRId32
#  define ABS(x) abs(x)
#endif

// Handprint: CFUN
#define FUNGEHANDPRINT 0x4346554e
// Version, for -V.
#define APPVERSION  "0.1.0"
// For y instruction.
#define FUNGEVERSION 10

#endif

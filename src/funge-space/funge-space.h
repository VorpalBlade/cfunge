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

#ifndef _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H
#define _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H

#include "../global.h"
#include "../vector.h"
#include <stdint.h>
#include <stdbool.h>

struct _fungeSpace;
typedef struct _fungeSpace fungeSpace;

extern fungeSpace    * fungeSpaceCreate(void) __attribute__((warn_unused_result));
extern void            fungeSpaceFree(fungeSpace * me);
extern FUNGEDATATYPE   fungeSpaceGet(fungeSpace * me, const fungePosition * position) __attribute__((warn_unused_result));
extern FUNGEDATATYPE   fungeSpaceGetOff(fungeSpace * me, const fungePosition * position, const fungePosition * offset) __attribute__((warn_unused_result));
extern void            fungeSpaceSet(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position);
extern void            fungeSpaceSetOff(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position, const fungePosition * offset);
// Used only for IP wrapping
extern void            fungeSpaceWrap(fungeSpace * me, fungePosition * restrict position, const fungeVector * restrict delta);
extern bool            fungeSpaceLoad(fungeSpace * me, const char * filename) __attribute__((warn_unused_result));

#endif

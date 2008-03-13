/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
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

#ifndef _HAD_SRC_FUNGE_SPACE_H
#define _HAD_SRC_FUNGE_SPACE_H

#include "../global.h"
#include "../vector.h"
#include "../rect.h"
#include <stdint.h>
#include <stdbool.h>

struct _fungeSpace;
typedef struct _fungeSpace fungeSpace;

extern fungeSpace    * fungeSpaceCreate(void) __attribute__((warn_unused_result));
extern void            fungeSpaceFree(fungeSpace * me);
extern FUNGEDATATYPE   fungeSpaceGet(const fungeSpace * restrict me,
                                     const fungePosition * restrict position) __attribute__((nonnull,warn_unused_result));
extern FUNGEDATATYPE   fungeSpaceGetOff(const fungeSpace * restrict me,
                                        const fungePosition * restrict position,
                                        const fungePosition * restrict offset) __attribute__((nonnull,warn_unused_result));
extern void            fungeSpaceSet(fungeSpace * restrict me,
                                     FUNGEDATATYPE value,
                                     const fungePosition * restrict position) __attribute__((nonnull));
extern void            fungeSpaceSetOff(fungeSpace * restrict me,
                                        FUNGEDATATYPE value,
                                        const fungePosition * restrict position,
                                        const fungePosition * restrict offset) __attribute__((nonnull));
// Used only for IP wrapping
extern void            fungeSpaceWrap(const fungeSpace * restrict me,
                                      fungePosition * restrict position,
                                      const fungeVector * restrict delta) __attribute__((nonnull));
extern bool            fungeSpaceLoad(fungeSpace * restrict me,
                                      const char * restrict filename) __attribute__((nonnull,warn_unused_result));
extern void            fungeSpaceGetBoundRect(const fungeSpace * restrict me,
                                              fungeRect * restrict rect) __attribute__((nonnull));

#endif

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

#ifndef _HAD_SRC_INTERPRETER_H
#define _HAD_SRC_INTERPRETER_H

#include <sys/types.h>
#include <stdint.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "ip.h"
#include "funge-space/funge-space.h"

// DO NOT MODIFY these two!
// (They are declared in main.c by the way.)
extern char **fungeargv;
extern int fungeargc;

// Functions

// Certain instructions that are also used elsewhere.
void IfEastWest(instructionPointer * restrict ip) __attribute__((nonnull,FUNGE_IN_FAST));
void IfNorthSouth(instructionPointer * restrict ip) __attribute__((nonnull,FUNGE_IN_FAST));

/**
 * Run instruction.
 */
#ifdef CONCURRENT_FUNGE
// If returns true, we executed an instruction that took 0 ticks, so call me
// again right away (in main loop)!
bool ExecuteInstruction(FUNGEDATATYPE opcode,
                        instructionPointer * restrict ip,
                        ssize_t * threadindex) __attribute__((nonnull,FUNGE_IN_FAST));
#else
void ExecuteInstruction(FUNGEDATATYPE opcode,
                        instructionPointer * restrict ip) __attribute__((nonnull,FUNGE_IN_FAST));
#endif

/**
 * Should only be called from main.c
 */
void interpreterRun(const char *filename) __attribute__((noreturn,FUNGE_IN_FAST));

#endif

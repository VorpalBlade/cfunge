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
 * Header for interpreter.c (that contains the main loop and parser).
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

/**
 * @defgroup fungearg Interpreter command line arguments
 *
 * Contains information about command line parameters
 * @note
 * DO NOT MODIFY these two!
 * (They are declared in main.c by the way.)
 */
/*@{*/
/// argv array, don't change.
extern const char **fungeargv;
/// argc, don't change.
extern int fungeargc;
/*@}*/

// Functions

// Certain instructions that are also used elsewhere.
/**
 * Run a _ instruction.
 * @param ip to IP to execute this instruction for.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void IfEastWest(instructionPointer * restrict ip);
/**
 * Run a | instruction.
 * @param ip to IP to execute this instruction for.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void IfNorthSouth(instructionPointer * restrict ip);

/**
 * Run instruction. Different prototype depending on if CONCURRENT_FUNGE
 * is defined or not.
 * @param opcode Instruction to execute
 * @param ip Instruction pointer to execute the instruction in.
 * @param threadindex The thread index (only if CONCURRENT_FUNGE is defined)
 */
#ifdef CONCURRENT_FUNGE
// If returns true, we executed an instruction that took 0 ticks, so call me
// again right away (in main loop)!
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
bool ExecuteInstruction(FUNGEDATATYPE opcode,
                        instructionPointer * restrict ip,
                        ssize_t * threadindex);
#else
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ExecuteInstruction(FUNGEDATATYPE opcode,
                        instructionPointer * restrict ip);
#endif

/**
 * Start interpreter on a specific filename.
 * @warning MUST only be called from main.c
 * @param filename Filename to operate on.
 */
FUNGE_ATTR_NORET FUNGE_ATTR_FAST
void interpreterRun(const char *filename);

#endif

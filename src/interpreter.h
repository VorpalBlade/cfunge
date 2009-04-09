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
 * Header for interpreter.c (that contains the main loop and parser).
 */

#ifndef FUNGE_HAD_SRC_INTERPRETER_H
#define FUNGE_HAD_SRC_INTERPRETER_H

#include "global.h"

#include <sys/types.h>
#include <stdint.h>

#include "ip.h"

// Certain instructions that are also used elsewhere.
/**
 * Run a _ instruction.
 * @param ip to IP to execute this instruction for.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void if_east_west(instructionPointer * restrict ip);
/**
 * Run a | instruction.
 * @param ip to IP to execute this instruction for.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void if_north_south(instructionPointer * restrict ip);

/**
 * Run instruction. Different prototype depending on if CONCURRENT_FUNGE
 * is defined or not.
 * @param opcode Instruction to execute
 * @param ip Instruction pointer to execute the instruction in.
 * @param threadindex The thread index (only if CONCURRENT_FUNGE is defined)
 * @returns
 * Return value only if CONCURRENT_FUNGE is defined. If true we executed an
 * instruction that took 0 ticks, so call me again right away (in main loop)!
 */
#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
bool execute_instruction(funge_cell opcode,
                        instructionPointer * restrict ip,
                        ssize_t * threadindex);
#else
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void execute_instruction(funge_cell opcode,
                        instructionPointer * restrict ip);
#endif

/**
 * Start interpreter on a specific filename.
 * @warning MUST only be called from main.c
 * @param filename Filename to operate on.
 */
FUNGE_ATTR_NORET FUNGE_ATTR_FAST
void interpreter_run(const char *filename);

#endif

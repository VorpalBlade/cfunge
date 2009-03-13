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
 * interate.c contains the implementation of the k instruction.
 */

#ifndef FUNGE_HAD_SRC_INSTRUCTIONS_ITERATE_H
#define FUNGE_HAD_SRC_INSTRUCTIONS_ITERATE_H

#include "../global.h"
#include "../stack.h"
#include "../ip.h"
#include "../stack.h"
#include "../funge-space/funge-space.h"

/**
 * Implements the k instruction, prototype differ depending on if
 * CONCURRENT_FUNGE is defined.
 * @param ip Instruction pointer to operate on.
 * @param IPList Pointer to IP list (only if CONCURRENT_FUNGE is defined).
 * @param threadindex What index in IPList the IP we operate on is.
 * @param isRecursive Should be false, only set to true by k itself when iterating over another k.
 * (only if CONCURRENT_FUNGE is defined).
 */
#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void run_iterate(instructionPointer * restrict ip,
                 ipList ** IPList,
                 ssize_t * restrict threadindex,
                 bool isRecursive);
#else
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void run_iterate(instructionPointer * restrict ip,
                 bool isRecursive);
#endif

#endif

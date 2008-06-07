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
 * The fingerprint manager.
 */


#ifndef _HAD_SRC_FINGERPRINTS_MANAGER_H
#define _HAD_SRC_FINGERPRINTS_MANAGER_H

#include "../global.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include "../ip.h"

typedef void (*fingerprintOpcode)(instructionPointer * ip);

typedef struct s_fungeOpcodeStack {
	// This is current size of the array entries
	size_t             size;
	// This is current top item in stack (may not be last item)
	// Note: One-indexed, as 0 = empty stack.
	size_t             top;
	fingerprintOpcode *entries;
} fungeOpcodeStack;
#define fungeOpcodeStackDefined

/**
 * Loads a fingerprint into IP. Should use ManagerAddOpcode (or OpcodeStackAdd)!
 * Returns true if successful, otherwise false.
 * Should leave things in a clean state if it fails.
 * Note: opcodes can be removed by manager at any point without prior warning.
 */
typedef bool (*fingerprintLoader)(instructionPointer * ip);
/**
 * Add func to the correct opcode in ip.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool OpcodeStackAdd(instructionPointer * restrict ip, char opcode, fingerprintOpcode func);

/**
 * Initialise stacks for IP
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerCreate(instructionPointer * restrict ip);

/**
 * Free stacks for IP
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void ManagerFree(instructionPointer * restrict ip);

#ifdef CONCURRENT_FUNGE
/**
 * Duplicate a loaded fingerprints.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerDuplicate(const instructionPointer * restrict oldip, instructionPointer * restrict newip);
#endif

/**
 * Try to load fingerprint.
 * Returns false if it failed (should reflect then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerLoad(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint);

/**
 * Try to unload fingerprint.
 * Returns false if it failed (should reflect then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerUnload(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint);

/**
 * Print out list of supported fingerprints
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NORET
void ManagerList(void);

/// For use in fingerprint loading routines ONLY
#define ManagerAddOpcode(fprint, opcode, name) \
	if (!OpcodeStackAdd(ip, (opcode), &Finger ## fprint ## name)) \
		return false;

#endif

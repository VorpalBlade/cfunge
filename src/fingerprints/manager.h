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

#ifndef _HAD_SRC_FINGERPRINTS_MANAGER_H
#define _HAD_SRC_FINGERPRINTS_MANAGER_H

#include "../global.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include "../ip.h"
#include "../interpreter.h"

typedef void (*fingerprintOpcode)(instructionPointer * ip);

typedef struct _fungeOpcodeStack {
	// This is current size of the array entries
	size_t             size;
	// This is current top item in stack (may not be last item)
	// Note: One-indexed, as 0 = empty stack.
	size_t             top;
	fingerprintOpcode *entries;
} fungeOpcodeStack;
#define fungeOpcodeStackDefined

/**
 * Loads a fingerprint into IP. Should use OpcodeStackAdd!
 * Returs true if sucessful, otherwise false.
 * MUST be a clean state if it fails.
 * Note, opcodes can be removed by manager at any point without
 * prior warning.
 */
typedef bool (*fingerprintLoader)(instructionPointer * ip);
/**
 * Add a opcode.
 */
extern bool OpcodeStackAdd(instructionPointer * ip, char opcode, fingerprintOpcode func) __attribute__((nonnull,warn_unused_result));


// A-Z
extern fungeOpcodeStack* fingerOpcodes[FINGEROPCODECOUNT];

extern void ManagerInit(instructionPointer * ip) __attribute__((nonnull));

extern bool ManagerLoad(instructionPointer * ip, FUNGEDATATYPE fingerprint) __attribute__((nonnull,warn_unused_result));
extern bool ManagerUnload(instructionPointer * ip, FUNGEDATATYPE fingerprint) __attribute__((nonnull,warn_unused_result));

#endif

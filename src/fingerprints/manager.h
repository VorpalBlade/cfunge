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
 * Should leave things in a clean state if it fails.
 * Note, opcodes can be removed by manager at any point without
 * prior warning.
 */
typedef bool (*fingerprintLoader)(instructionPointer * ip);
/**
 * Add func to the correct opcode in ip.
 */
bool OpcodeStackAdd(instructionPointer * restrict ip, char opcode, fingerprintOpcode func) __attribute__((nonnull,warn_unused_result));

/**
 * Initialize stacks for IP
 */
bool ManagerCreate(instructionPointer * restrict ip) __attribute__((nonnull,warn_unused_result));

/**
 * Free stacks for IP
 */
void ManagerFree(instructionPointer * restrict ip) __attribute__((nonnull));

#ifdef CONCURRENT_FUNGE
/**
 * Duplicate a loaded fingerprints.
 */
bool ManagerDuplicate(const instructionPointer * restrict oldip, instructionPointer * restrict newip) __attribute__((nonnull,warn_unused_result));
#endif

/**
 * Try to load fingerprint.
 * Returns false if it failed (should reflect then), otherwise true.
 */
bool ManagerLoad(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint) __attribute__((nonnull,warn_unused_result));

/**
 * Try to unload fingerprint.
 * Returns false if it failed (should reflect then), otherwise true.
 */
bool ManagerUnload(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint) __attribute__((nonnull,warn_unused_result));

/**
 * Print out list of supported fingerprints
 */
void ManagerList(void) __attribute__((noreturn));


#endif

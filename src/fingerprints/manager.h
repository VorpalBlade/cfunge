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

/// Function prototype for a fingerprint instruction.
typedef void (*fingerprintOpcode)(instructionPointer * ip);

/**
 * An opcode stack.
 * @warning
 * Fingerprints should not directly touch these, use the functions below for that.
 */
typedef struct s_fungeOpcodeStack {
	/// This is current size of the array entries
	size_t             size;
	/// This is current top item in stack (may not be last item)
	/// Note: One-indexed, as 0 = empty stack.
	size_t             top;
	/// Pointer to entries of the stack.
	fingerprintOpcode *entries;
} fungeOpcodeStack;
#define fungeOpcodeStackDefined

/**
 * Function prototype for fingerprint loader. It should load a fingerprint
 * into IP using either ManagerAddOpcode or OpcodeStackAdd! The former is
 * recommended. May also do some other setup like initialising static variables.
 * Please leave things in a clean state if it fails.
 * @return Returns true if successful, otherwise false.
 * @note Opcodes can be removed by manager at any point without prior warning.
 */
typedef bool (*fingerprintLoader)(instructionPointer * ip);
/**
 * Add func to the correct opcode (instruction) in ip.
 * If possible use ManagerAddOpcode() macro instead!
 * @param ip IP to add this opcode to.
 * @param opcode What opcode to add.
 * @param func Function pointer to routine implementing the instruction.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
bool OpcodeStackAdd(instructionPointer * restrict ip, char opcode, fingerprintOpcode func);
/**
 * Pop an opcode from an stack returning it.
 * @param ip IP to pop opcode for.
 * @param opcode What opcode to pop.
 * @return A function pointer.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
fingerprintOpcode OpcodeStackPop(instructionPointer * restrict ip, char opcode);

/**
 * Initialise opcode stacks for IP
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @return True if successful otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerCreate(instructionPointer * restrict ip);

/**
 * Free opcode stacks for IP
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @return True if successful otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void ManagerFree(instructionPointer * restrict ip);

#ifdef CONCURRENT_FUNGE
/**
 * Duplicate the loaded fingerprint stacks to another IP, used for Concurrent Funge.
 * @warning Don't call this directly from fingerprints.
 * @param oldip Old IP to copy loaded fingerprints from.
 * @param newip Target to copy to.
 * @return True if successful otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerDuplicate(const instructionPointer * restrict oldip, instructionPointer * restrict newip);
#endif

/**
 * Try to load a fingerprint.
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @param fingerprint Fingerprint to load.
 * @return Returns false if it failed (caller should reflect the IP then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerLoad(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint);

/**
 * Try to unload a fingerprint.
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @param fingerprint Fingerprint to unload.
 * @return Returns false if it failed (caller should reflect the IP then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool ManagerUnload(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint);

/**
 * Print out list of supported fingerprints
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NORET
void ManagerList(void);

/// For use in fingerprint loading routines ONLY.
/// Example code:
/// @code
/// ManagerAddOpcode(CPLI, 'V', abs)
/// @endcode
/// This will "bind" the function named FingerCPLIabs to the instruction V.
/// @param fprint Fingerprint name
/// @param opcode Instruction char (range A-Z)
/// @param name Function name. Real function name constructed from fprint and this.
#define ManagerAddOpcode(fprint, opcode, name) \
	if (!OpcodeStackAdd(ip, (opcode), &Finger ## fprint ## name)) \
		return false;

#endif

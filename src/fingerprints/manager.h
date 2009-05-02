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
 * The fingerprint manager.
 */

#ifndef FUNGE_HAD_SRC_FINGERPRINTS_MANAGER_H
#define FUNGE_HAD_SRC_FINGERPRINTS_MANAGER_H

#include "../global.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

/// Forward decl, see ../ip.h
struct s_instructionPointer;

/// Function prototype for a fingerprint instruction.
typedef void (*fingerprintOpcode)(struct s_instructionPointer * ip);

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
	/// Pointer to entries of the stack. Is initialised on demand and may thus
	/// be NULL before any fingerprint has been loaded that uses this opcode.
	/// In that case, both size and top are 0.
	fingerprintOpcode *entries;
} fungeOpcodeStack;

/**
 * Function prototype for fingerprint loader. It should load a fingerprint
 * into IP using either manager_add_opcode or opcode_stack_push! The former is
 * recommended. May also do some other setup like initialising static variables.
 * Please leave things in a clean state if it fails.
 * @return Returns true if successful, otherwise false.
 * @note Opcodes can be removed by manager at any point without prior warning.
 */
typedef bool (*fingerprintLoader)(struct s_instructionPointer * ip);

/**
 * Add func to the correct opcode (instruction) in ip.
 * If possible use manager_add_opcode() macro instead!
 * @param ip IP to add this opcode to.
 * @param opcode What opcode to add.
 * @param func Function pointer to routine implementing the instruction.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
bool opcode_stack_push(struct s_instructionPointer * restrict ip, unsigned char opcode, fingerprintOpcode func);
/**
 * Pop an opcode from an stack returning it.
 * @param ip IP to pop opcode for.
 * @param opcode What opcode to pop.
 * @return A function pointer.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
fingerprintOpcode opcode_stack_pop(struct s_instructionPointer * restrict ip, unsigned char opcode);

/**
 * Free opcode stacks for IP
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @return True if successful otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void manager_free(struct s_instructionPointer * restrict ip);

#ifdef CONCURRENT_FUNGE
/**
 * Duplicate the loaded fingerprint stacks to another IP, used for Concurrent Funge.
 * @warning Don't call this directly from fingerprints.
 * @param oldip Old IP to copy loaded fingerprints from.
 * @param newip Target to copy to.
 * @note If allocation fails it will call DIAG_OOM(). It will not return.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void manager_duplicate(const struct s_instructionPointer * restrict oldip,
                       struct s_instructionPointer * restrict newip);
#endif

/**
 * Try to load a fingerprint.
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @param fingerprint Fingerprint to load.
 * @return Returns false if it failed (caller should reflect the IP then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool manager_load(struct s_instructionPointer * restrict ip, funge_cell fingerprint);

/**
 * Try to unload a fingerprint.
 * @warning Don't call this directly from fingerprints.
 * @param ip IP to operate on.
 * @param fingerprint Fingerprint to unload.
 * @return Returns false if it failed (caller should reflect the IP then), otherwise true.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool manager_unload(struct s_instructionPointer * restrict ip, funge_cell fingerprint);

/**
 * Print out list of supported fingerprints
 */
FUNGE_ATTR_FAST FUNGE_ATTR_COLD FUNGE_ATTR_NORET
void manager_list(void);

/// For use in fingerprint loading routines ONLY.
/// Example code:
/// @code
/// manager_add_opcode(CPLI, 'V', abs)
/// @endcode
/// This will "bind" the function named finger_CPLI_abs to the instruction V.
/// @param fprint Fingerprint name
/// @param opcode Instruction char (range A-Z)
/// @param name Function name. Real function name constructed from fprint and this.
#define manager_add_opcode(fprint, opcode, name) \
	if (FUNGE_UNLIKELY(!opcode_stack_push(ip, (opcode), &finger_ ## fprint ## _ ## name))) \
		return false;

// Now include ip.h, we couldn't before.
#include "../ip.h"

#endif

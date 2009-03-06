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
 * Definition and functions for of instruction pointer and instruction
 * pointer list.
 */

#ifndef FUNGE_HAD_SRC_IP_H
#define FUNGE_HAD_SRC_IP_H

#include "global.h"

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"
#include "stack.h"
#include "funge-space/funge-space.h"

/// Type of IP delta.
typedef funge_vector ipDelta;

/// IP mode: code.
#define ipmCODE 0x0
/// IP mode: string.
#define ipmSTRING 0x1
/// Type of the ipMode entry.
typedef uint_fast8_t ipMode;


#ifndef fungeOpcodeStackDefined
struct s_fungeOpcodeStack;
#endif

/// This is for size of opcode array.
#define FINGEROPCODECOUNT 26

/// Instruction pointer.
/// @note
/// Fields of the style fingerXXXX* are for fingerprint per-IP data.
/// Please avoid such fields when possible.
typedef struct s_instructionPointer {
	funge_stack               * stack;         ///< Pointer to top stack.
	funge_vector                position;      ///< Current position.
	ipDelta                     delta;         ///< Current delta.
	funge_vector                storageOffset; ///< The storage offset for current IP.
	ipMode                      mode;          ///< String or code mode.
	// "Full" bool for very often checked flags.
	bool                        needMove;      ///< Should ip_forward be called at end of main loop. Is reset to true each time.
	bool                        stringLastWasSpace;     ///< Used in string mode for SGML style spaces.
	// Bitfield for uncommon flags
	bool                        fingerSUBRisRelative:1; ///< Data for fingerprint SUBR.
	funge_cell                  ID;                     ///< The ID of this IP.
	funge_stackstack          * stackstack;             ///< The stack stack.
	struct s_fungeOpcodeStack * fingerOpcodes[FINGEROPCODECOUNT];  ///< Array of fingerprint opcodes.
	void                      * fingerHRTItimestamp;    ///< Data for fingerprint HRTI.
	                                                    ///  We don't know what type here.
} instructionPointer;
#define ipDEFINED 1

#ifdef CONCURRENT_FUNGE
/// Instruction pointer list. For concurrent Funge.
typedef struct s_ipList {
	size_t              size;      /**< Total size */
	size_t              top;       /**< Top valid one */
	size_t              highestID; /**< Currently highest ID, they are unique. */
	/**
	 * This array is slightly complex for speed reasons.
	 * Main loop must iterate over it *backwards*, this allow easy splitting of last ip.
	 */
	instructionPointer  ips[];
} ipList;
#endif

#ifndef CONCURRENT_FUNGE
/**
 * Create a new instruction pointer.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
instructionPointer * ip_create(void);
#endif

#if !defined(CONCURRENT_FUNGE) && !defined(NDEBUG)
/**
 * Free an instruction pointer.
 */
FUNGE_ATTR_FAST
void ip_free(instructionPointer * restrict ip);
#endif

/**
 * Move the IP forward.
 * @param ip Instruction pointer to operate on.
 * @param steps Number of steps forward to take. You can also take negative
 * count of steps.
 * @note
 * steps let you take several steps at once. However if you will wrap, you
 * probably want to set a temp delta instead and take +/- one step for now.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ip_forward(instructionPointer * restrict ip, funge_cell steps);

/**
 * Mirror IP direction.
 */
#define ip_reverse(m_ip) \
	do { \
		(m_ip)->delta.x *= -1; \
		(m_ip)->delta.y *= -1; \
	} while(0)
// I don't like the do { ... } while(0) hack at all..
// but it is needed.

/// Turn the IP left as [ would do.
#define ip_turn_left(m_ip) \
	do { \
		(m_ip)->delta  = (funge_vector) { (m_ip)->delta.y, -(m_ip)->delta.x }; \
	} while(0)
/// Turn the IP right as ] would do.
#define ip_turn_right(m_ip) \
	do { \
		(m_ip)->delta  = (funge_vector) { -(m_ip)->delta.y, (m_ip)->delta.x }; \
	} while(0)

/// Set delta of an IP to a new vector.
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ip_set_delta(instructionPointer * restrict ip, const ipDelta * restrict delta);
/// Set position of an IP to a new vector. Will wrap if needed (based on current delta).
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ip_set_position(instructionPointer * restrict ip, const funge_vector * restrict position);

// To make things simpler.
/// Set IP delta to west.
#define ip_go_west(m_ip)  do { (m_ip)->delta = (funge_vector) {-1, 0}; } while(0)
/// Set IP delta to east.
#define ip_go_east(m_ip)  do { (m_ip)->delta = (funge_vector) {1, 0}; } while(0)
/// Set IP delta to north.
#define ip_go_north(m_ip) do { (m_ip)->delta = (funge_vector) {0, -1}; } while(0)
/// Set IP delta to south.
#define ip_go_south(m_ip) do { (m_ip)->delta = (funge_vector) {0, 1}; } while(0)

#ifdef CONCURRENT_FUNGE
/**
 * Create a new IP list with the single default IP in it.
 * @warning Should only be called from internal setup code.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ipList* iplist_create(void);

#ifndef NDEBUG
/**
 * Free an IP list.
 * @warning Should only be called from internal tear-down code.
 * @param me ipList to free.
 */
FUNGE_ATTR_FAST
void iplist_free(ipList* me);
#endif

/**
 * Add a new IP, one place before current one.
 * @param me ipList to operate on.
 * @param index What entry in the list to duplicate.
 * @return Returns the index of next to execute as that may have changed after
 * this call. A value of -1 = failed to create IP.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ssize_t iplist_duplicate_ip(ipList** me, size_t index);

/**
 * Terminate an ip.
 * @param me ipList to operate on.
 * @param index What entry in the list to terminate.
 * @return Returns index of next to execute as that may have changed after this call.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ssize_t iplist_terminate_ip(ipList** me, size_t index);
#endif


#endif

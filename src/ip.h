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
 * Definition and functions for of instruction pointer and instruction
 * pointer list.
 */

#ifndef _HAD_SRC_IP_H
#define _HAD_SRC_IP_H

#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/funge-space.h"

/// Type of ip delta.
typedef fungeVector ipDelta;

/// IP mode: code.
#define ipmCODE 0
/// IP mode: string.
#define ipmSTRING 1
/// Type of the ipMode entry.
typedef uint_fast8_t ipMode;


#ifndef fungeOpcodeStackDefined
struct s_fungeOpcodeStack;
#endif

/// This is for size of opcode array.
#define FINGEROPCODECOUNT 26

/// Instruction pointer.
typedef struct s_instructionPointer {
	fungeStack                * stack;    ///< Pointer to top stack.
	fungePosition               position; ///< Current position.
	ipDelta                     delta;    ///< Current delta.
	fungePosition               storageOffset; ///< The storage offset for current IP.
	ipMode                      mode;          ///< String or code mode.
	bool                        needMove:1;    ///< Should ipForward be called at end of main loop. Is reset to true each time.
	bool                        stringLastWasSpace:1; ///< Used in string mode for SGML style spaces.
	FUNGEDATATYPE               ID;                   ///< The ID of this IP.
	fungeStackStack           * stackstack;           ///< The stack stack.
	struct s_fungeOpcodeStack * fingerOpcodes[FINGEROPCODECOUNT];  ///< Array of fingerprint opcodes.
	// Here comes fingerprint per-ip data. Please avoid when possible.
	// And make the data as small as possible.
	struct timeval            * fingerHRTItimestamp; ///< Data for fingerprint HRTI.
} instructionPointer;
#define ipDEFINED 1

#ifdef CONCURRENT_FUNGE
/// Instruction pointer list. For concurrent funge.
typedef struct s_ipList {
	size_t              size; /**< Total size */
	size_t              top; /**< Top running one */
	size_t              highestID; /**< Currently highest ID, they are unique. */
	/**
	 * This array is slightly complex for speed reasons.
	 * Main loop must iterate over it *backwards*, this allow easy splitting of last ip.
	 */
	instructionPointer  ips[];
} ipList;
#endif

/**
 * Create a new instruction pointer.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
instructionPointer * ipCreate(void);
/**
 * Free an instruction pointer.
 */
FUNGE_ATTR_FAST
void ipFree(instructionPointer * restrict ip);

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
void ipForward(instructionPointer * restrict ip, int_fast64_t steps);

/**
 * Mirror IP direction.
 */
#define ipReverse(ip) \
	{ \
		(ip)->delta.x *= -1; \
		(ip)->delta.y *= -1; \
	}
/// Turn the IP left as [ would do.
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ipTurnLeft(instructionPointer * restrict ip);
/// Turn the IP right as ] would do.
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ipTurnRight(instructionPointer * restrict ip);
/// Set delta of an IP to a new vector.
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ipSetDelta(instructionPointer * restrict ip, const ipDelta * restrict delta);
/// Set position of an IP to a new vector.
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void ipSetPosition(instructionPointer * restrict ip, const fungePosition * restrict position);

// To make things simpler.
/// Set IP delta to west.
#define ipGoWest(ip)  ipSetDelta(ip, VectorCreateRef(-1,  0))
/// Set IP delta to east.
#define ipGoEast(ip)  ipSetDelta(ip, VectorCreateRef( 1,  0))
/// Set IP delta to north.
#define ipGoNorth(ip) ipSetDelta(ip, VectorCreateRef( 0, -1))
/// Set IP delta to south.
#define ipGoSouth(ip) ipSetDelta(ip, VectorCreateRef( 0,  1))

#ifdef CONCURRENT_FUNGE
/**
 * Create a new IP list with the single default IP in it.
 * @warning Should only be called from internal setup code.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ipList* ipListCreate(void);
/**
 * Free an IP list.
 * @warning Should only be called from internal tear-down code.
 * @param me ipList to free.
 */
FUNGE_ATTR_FAST
void ipListFree(ipList* me);
/**
 * Add a new IP, one place before current one.
 * @param me ipList to operate on.
 * @param index What entry in the list to duplicate.
 * @return Returns the index of next to execute as that may have changed after
 * this call. A value of -1 = failed to create IP.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ssize_t ipListDuplicateIP(ipList** me, size_t index);
/**
 * Terminate an ip.
 * @param me ipList to operate on.
 * @param index What entry in the list to terminate.
 * @return Returns index of next to execute as that may have changed after this call.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
ssize_t ipListTerminateIP(ipList** me, size_t index);
#endif


#endif

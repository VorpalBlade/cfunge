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

#ifndef _HAD_SRC_IP_H
#define _HAD_SRC_IP_H

#include <sys/types.h>
#include <stdint.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/funge-space.h"

typedef fungeVector ipDelta;

typedef enum { ipmCODE = 0, ipmSTRING } ipMode;

#ifndef fungeOpcodeStackDefined
struct _fungeOpcodeStack;
#endif

// This is for size of opcode array.
#define FINGEROPCODECOUNT 26

typedef struct _instructionPointer {
	fungePosition              position; /**< Current position. */
	ipDelta                    delta;    /**< Current delta. */
	ipMode                     mode;     /**< String or code mode. */
	bool                       StringLastWasSpace; /**< Used in string mode for SGML style spaces. */
	bool                       NeedMove; /**< Should ipForward be called at end of main loop. Is reset to true each time. */
	FUNGEDATATYPE              ID;
	fungePosition              storageOffset;
	fungeStack               * stack;                             /**< Pointer to top stack. */
	struct _fungeOpcodeStack * fingerOpcodes[FINGEROPCODECOUNT];  /**< Array of fingerprint opcodes */
	fungeStackStack          * stackstack;
} instructionPointer;
#define ipDEFINED 1

#ifdef CONCURRENT_FUNGE
// For concurrent funge
typedef struct {
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
extern instructionPointer * ipCreate(void) __attribute__((malloc,warn_unused_result));
/**
 * Free an instruction pointer.
 */
extern void ipFree(instructionPointer * restrict ip);

/**
 * steps let you take several steps at once. You can also take negative
 * count of steps.
 * However if you will wrap, you probably want to set a temp delta instead and
 * take one step for now.
 */
extern void ipForward(int_fast64_t steps, instructionPointer * restrict ip) __attribute__((nonnull));

/**
 * Mirror IP direction.
 */
#define ipReverse(ip) \
	{ \
		(ip)->delta.x *= -1; \
		(ip)->delta.y *= -1; \
	}

extern void ipTurnLeft(instructionPointer * restrict ip) __attribute__((nonnull));
extern void ipTurnRight(instructionPointer * restrict ip) __attribute__((nonnull));
extern void ipSetDelta(instructionPointer * restrict ip, const ipDelta * restrict delta) __attribute__((nonnull));
extern void ipSetPosition(instructionPointer * restrict ip, const fungePosition * restrict position) __attribute__((nonnull));

#ifdef CONCURRENT_FUNGE
/**
 * Create a new IP list with the single default IP in it.
 */
extern ipList* ipListCreate(void) __attribute__((malloc,warn_unused_result));
/**
 * Free IP list.
 */
extern void ipListFree(ipList* me);
/**
 * Add a new IP, one place before current one.
 * Returns index of next to execute as that may have changed after this call.
 * A value of -1 = failed to create IP.
 */
extern ssize_t ipListDuplicateIP(ipList** me, size_t index) __attribute__((nonnull,warn_unused_result));
/**
 * Terminate an ip.
 * Returns index of next to execute as that may have changed after this call.
 */
extern ssize_t ipListTerminateIP(ipList** me, size_t index) __attribute__((nonnull,warn_unused_result));
#endif


#endif

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
	fungePosition              storageOffset;
	fungeStack               * stack;                             /**< Pointer to top stack. */
	struct _fungeOpcodeStack * fingerOpcodes[FINGEROPCODECOUNT];  /**< Array of fingerprint opcodes */
	fungeStackStack          * stackstack;
} instructionPointer;
#define ipDEFINED 1


typedef struct {
	size_t              count;
	instructionPointer* entries;
} ipList;

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
		ip->delta.x = -ip->delta.x; \
		ip->delta.y = -ip->delta.y; \
	}

extern void ipTurnLeft(instructionPointer * restrict ip) __attribute__((nonnull));
extern void ipTurnRight(instructionPointer * restrict ip) __attribute__((nonnull));
extern void ipSetDelta(instructionPointer * restrict ip, const ipDelta * restrict delta) __attribute__((nonnull));
extern void ipSetPosition(instructionPointer * restrict ip, const fungePosition * restrict position) __attribute__((nonnull));

#endif

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

#include "../global.h"
#include "manager.h"
#include "../ip.h"
#include "../settings.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#define ALLOCCHUNKSIZE 2

#define MANAGER_INTERNAL
#include "fingerprints.h"

/// To get size of the fingerprint array
#define FPRINT_ARRAY_SIZE sizeof(ImplementedFingerprints) / sizeof(ImplementedFingerprintEntry)

/**************************
 * Opcode Stack functions *
 **************************/

/**
 * Create an opcode stack.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED
static inline fungeOpcodeStack* CreateOpcodeStack(void)
{
	fungeOpcodeStack * tmp = (fungeOpcodeStack*)cf_malloc(sizeof(fungeOpcodeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (fingerprintOpcode*)cf_malloc(ALLOCCHUNKSIZE * sizeof(fingerprintOpcode));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	tmp->entries[0] = NULL;
	return tmp;
}

/**
 * Free an opcode stack.
 */
FUNGE_ATTR_FAST
static inline void FreeOpcodeStack(fungeOpcodeStack * restrict me)
{
	if (!me)
		return;
	if (me->entries)
		cf_free(me->entries);
	cf_free(me);
}

#ifdef CONCURRENT_FUNGE
/**
 * Duplicate an opcode stack, used for split (t).
 */
FUNGE_ATTR_FAST FUNGE_ATTR_MALLOC FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline fungeOpcodeStack* DuplicateOpcodeStack(const fungeOpcodeStack * restrict old)
{
	fungeOpcodeStack * tmp;

	if (!old)
		return NULL;

	tmp = (fungeOpcodeStack*)cf_malloc(sizeof(fungeOpcodeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (fingerprintOpcode*)cf_malloc((old->top + 1) * sizeof(fingerprintOpcode));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = old->top + 1;
	tmp->top = old->top;
	for (size_t i = 0; i < tmp->top; i++)
		tmp->entries[i] = old->entries[i];
	return tmp;
}
#endif

/// Add an entry to an OP code stack.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool OpcodeStackAdd(instructionPointer * restrict ip, char opcode, fingerprintOpcode func)
{
	fungeOpcodeStack * stack = ip->fingerOpcodes[opcode - 'A'];
	// Check if we need to realloc.
	if (stack->top == stack->size) {
		stack->entries = (fingerprintOpcode*)cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(fingerprintOpcode));
		if (!stack->entries)
			return false;
		stack->entries[stack->top] = func;
		stack->top++;
		stack->size += ALLOCCHUNKSIZE;
	} else {
		stack->entries[stack->top] = func;
		stack->top++;
	}
	return true;
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
fingerprintOpcode OpcodeStackPop(instructionPointer * restrict ip, char opcode) {
	fungeOpcodeStack * stack = ip->fingerOpcodes[opcode - 'A'];
	if (stack->top == 0) {
		return NULL;
	} else {
		fingerprintOpcode tmp = stack->entries[stack->top - 1];
		stack->top--;
		return tmp;
	}
}

/**
 * Pop a function pointer from an opcode stack, discarding it.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void OpcodeStackDrop(fungeOpcodeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

/****************************
 * Opcode Manager functions *
 ****************************/

/// Set up the fingerprint stacks for an IP.
FUNGE_ATTR_FAST bool ManagerCreate(instructionPointer * restrict ip)
{
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		ip->fingerOpcodes[i] = CreateOpcodeStack();
		if (!ip->fingerOpcodes[i])
			return false;
	}
	return true;
}

/// Clean up the fingerprint stacks for an IP.
FUNGE_ATTR_FAST void ManagerFree(instructionPointer * restrict ip)
{
	if (!ip)
		return;
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		FreeOpcodeStack(ip->fingerOpcodes[i]);
	}
}

#ifdef CONCURRENT_FUNGE
/// Duplicate the opcode stacks from one ip to another, for concurrent Funge.
FUNGE_ATTR_FAST bool ManagerDuplicate(const instructionPointer * restrict oldip,
                                 instructionPointer * restrict newip)
{
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		newip->fingerOpcodes[i] = DuplicateOpcodeStack(oldip->fingerOpcodes[i]);
		if (!newip->fingerOpcodes[i]) {
			// Try to create a new one instead then...
			newip->fingerOpcodes[i] = CreateOpcodeStack();
			if (!newip->fingerOpcodes[i])
				return false;
		}
	}
	return true;
}
#endif


#define FPRINT_NOTFOUND -1
/**
 * Return value is index into ImplementedFingerprints array.
 * -1 means not found.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline ssize_t FindFingerPrint(const FUNGEDATATYPE fingerprint)
{
	for (size_t i = 0; i < FPRINT_ARRAY_SIZE; i++) {
		if (fingerprint <= ImplementedFingerprints[i].fprint) {
			// Then it is larger... as in "not found"
			if (fingerprint != ImplementedFingerprints[i].fprint)
				return -1;
			// If we run in a sandbox, can fingerprint be loaded?
			// If not: break, as we know we found the right fingerprint,
			// so no need to search more.
			if (SettingSandbox && !ImplementedFingerprints[i].safe)
				return FPRINT_NOTFOUND;
			return i;
		}
	}
	return FPRINT_NOTFOUND;
}

FUNGE_ATTR_FAST bool ManagerLoad(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint)
{
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == FPRINT_NOTFOUND) {
		return false;
	} else {
		bool gotLoaded = ImplementedFingerprints[index].loader(ip);
		if (gotLoaded) {
			StackPush(ip->stack, fingerprint);
			StackPush(ip->stack, 1);
			return true;
		} else {
			return false;
		}
	}
}

FUNGE_ATTR_FAST bool ManagerUnload(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint)
{
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == -1)
		return false;
	for (size_t i = 0; i < strlen(ImplementedFingerprints[index].opcodes); i++)
		OpcodeStackDrop(ip->fingerOpcodes[ImplementedFingerprints[index].opcodes[i] - 'A']);
	return true;
}

#if CHAR_BIT != 8
#  error "CHAR_BIT != 8, please make sure the function below the location of this error works on your system."
#endif

FUNGE_ATTR_FAST void ManagerList(void)
{
	puts("Supported fingerprints in this binary:");
	for (size_t i = 0; i < FPRINT_ARRAY_SIZE; i++) {
		// This hack is here to reconstruct the name from the fingerprint.
		// It will probably break if char isn't 8 bits.
		FUNGEDATATYPE fprint = ImplementedFingerprints[i].fprint;
		char fprintname[5] = { fprint >> 24, fprint >> 16, fprint >> 8, fprint, '\0'};

		printf("0x%x %s%s %s\n",
		       (unsigned)fprint,
		       fprintname,
		       ImplementedFingerprints[i].safe ? "" : " (not available in sandbox mode)",
		       ImplementedFingerprints[i].url ? ImplementedFingerprints[i].url : "");
	}
	exit(0);
}

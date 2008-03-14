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

#include "../global.h"
#include "manager.h"
#include "../ip.h"

#include <string.h>
#include <stdbool.h>

#include "NULL/NULL.h"
#include "ROMA/ROMA.h"
#include "MODU/MODU.h"
#include "REFC/REFC.h"

fungeOpcodeStack* fingerOpcodes[FINGEROPCODECOUNT];

#define ALLOCCHUNKSIZE 1

typedef struct {
	FUNGEDATATYPE     fprint;   /**< Fingerprint */
	fingerprintLoader loader;   /**< Loader function pointer */
	const char*       opcodes;  /**< Sorted string with all implemented opcodes */
} ImplementedFingerprintEntry;

// Implemented fingerprints
// NOTE: Keep sorted
static const ImplementedFingerprintEntry ImplementedFingerprints[] = {
	// MODU - Modulo Arithmetic
	{ .fprint = 0x4d4f4455, .loader = &FingerMODUload, .opcodes = "MRU" },
	// NULL
	{ .fprint = 0x4e554c4c, .loader = &FingerNULLload, .opcodes = "ABCDEFGHIJKLMNOPQRSTUVXYZ" },
	// REFC - Referenced Cells Extension
	{ .fprint = 0x52454643, .loader = &FingerREFCload, .opcodes = "DR" },
	// ROMA - Roman Numerals
	{ .fprint = 0x524f4d41, .loader = &FingerROMAload, .opcodes = "CDILMVX" },
	// Last should be 0
	{ .fprint = 0, .loader = NULL, .opcodes = NULL }
};


/**************************
 * Opcode Stack functions *
 **************************/

static fungeOpcodeStack* CreateOpcodeStack(void) {
	fungeOpcodeStack * tmp = cf_malloc(sizeof(fungeOpcodeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = cf_malloc_noptr(ALLOCCHUNKSIZE * sizeof(fingerprintOpcode));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	tmp->entries[0] = NULL;
	return tmp;
}

bool OpcodeStackAdd(instructionPointer * ip, char opcode, fingerprintOpcode func) {
	fungeOpcodeStack * stack = ip->fingerOpcodes[opcode - 'A'];
	// Do we need to realloc?
	if (stack->top == stack->size) {
		stack->entries = cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(fingerprintOpcode));
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

static void OpcodeStackPop(fungeOpcodeStack * stack) {
	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

/****************************
 * Opcode Manager functions *
 ****************************/

void ManagerInit(instructionPointer * ip) {
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		ip->fingerOpcodes[i] = CreateOpcodeStack();
	}
}

// Return value is index
static inline ssize_t FindFingerPrint(FUNGEDATATYPE fingerprint) {
	int i = 0;
	bool found = false;
	do {
		if (fingerprint == ImplementedFingerprints[i].fprint) {
			found = true;
			break;
		}
	} while (ImplementedFingerprints[++i].fprint != 0);
	if (!found)
		return -1;
	return i;
}

bool ManagerLoad(instructionPointer * ip, FUNGEDATATYPE fingerprint) {
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == -1)
		return false;
	{
		bool gotLoaded = ImplementedFingerprints[index].loader(ip);
		if (gotLoaded) {
			StackPush(fingerprint, ip->stack);
			StackPush(1, ip->stack);
			return true;
		} else {
			return false;
		}
	}
}

bool ManagerUnload(instructionPointer * ip, FUNGEDATATYPE fingerprint) {
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == -1)
		return false;
	for (size_t i = 0; i < strlen(ImplementedFingerprints[index].opcodes); i++)
		OpcodeStackPop(ip->fingerOpcodes[ImplementedFingerprints[index].opcodes[i] - 'A']);
	return true;
}

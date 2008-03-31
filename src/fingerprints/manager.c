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

#include "BASE/BASE.h"
#include "DIRF/DIRF.h"
#include "FPDP/FPDP.h"
#include "FPSP/FPSP.h"
#include "MODU/MODU.h"
#include "NULL/NULL.h"
#include "ORTH/ORTH.h"
#include "REFC/REFC.h"
#include "ROMA/ROMA.h"
#include "SUBR/SUBR.h"
#include "TOYS/TOYS.h"

fungeOpcodeStack* fingerOpcodes[FINGEROPCODECOUNT];

#define ALLOCCHUNKSIZE 2

typedef struct s_ImplementedFingerprintEntry {
	const FUNGEDATATYPE     fprint;   /**< Fingerprint. */
	const fingerprintLoader loader;   /**< Loader function pointer. */
	const char            * opcodes;  /**< Sorted string with all implemented opcodes. */
	const char            * url;      /**< URI, used to show links for more info about fingerprints. */
	const bool              safe:1;   /**< If true, this fingerprint is safe in sandbox mode. */
} ImplementedFingerprintEntry;

// Implemented fingerprints
// NOTE: Keep sorted (apart from ending 0 entry).
static const ImplementedFingerprintEntry ImplementedFingerprints[] = {
	// BASE - I/O for numbers in other bases
	{ .fprint = 0x42415345, .loader = &FingerBASEload, .opcodes = "BHINO", .url = "http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html", .safe = true },
	// DIRF Directory functions extension, NOT SAFE
	{ .fprint = 0x44495246, .loader = &FingerDIRFload, .opcodes = "CMR", .url = "http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html", .safe = false },
	// FPDP Double precision floating point
	{ .fprint = 0x46504450, .loader = &FingerFPDPload, .opcodes = "ABCDEFGHIKLMNPQRSTVXY", .url = "http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html", .safe = true },
	// FPSP Single precision floating point
	{ .fprint = 0x46505350, .loader = &FingerFPSPload, .opcodes = "ABCDEFGHIKLMNPQRSTVXY", .url = "http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html", .safe = true },
	// MODU - Modulo Arithmetic
	{ .fprint = 0x4d4f4455, .loader = &FingerMODUload, .opcodes = "MRU", .url = "http://catseye.tc/projects/funge98/library/MODU.html", .safe = true },
	// NULL
	{ .fprint = 0x4e554c4c, .loader = &FingerNULLload, .opcodes = "ABCDEFGHIJKLMNOPQRSTUVXYZ", .url = "http://catseye.tc/projects/funge98/library/NULL.html", .safe = true },
	// ORTH - Orthogonal Easement Library
	{ .fprint = 0x4f525448, .loader = &FingerORTHload, .opcodes = "AEGOPSVWXYZ", .url = "http://catseye.tc/projects/funge98/library/ORTH.html", .safe = true },
	// REFC - Referenced Cells Extension
	{ .fprint = 0x52454643, .loader = &FingerREFCload, .opcodes = "DR", .url = "http://catseye.tc/projects/funge98/library/REFC.html", .safe = true },
	// ROMA - Roman Numerals
	{ .fprint = 0x524f4d41, .loader = &FingerROMAload, .opcodes = "CDILMVX", .url = "http://catseye.tc/projects/funge98/library/ROMA.html", .safe = true },
	// SUBR - Subroutine extension
	{ .fprint = 0x53554252, .loader = &FingerSUBRload, .opcodes = "CJR", .url = "http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html", .safe = true },
	// TOYS - Funge-98 Standard Toys
	{ .fprint = 0x544f5953, .loader = &FingerTOYSload, .opcodes = "ABCDEFGHIJKLMNOPQRSTUVXYZ", .url = "http://catseye.tc/projects/funge98/library/TOYS.html", .safe = true },
	// Last should be 0
	{ .fprint = 0, .loader = NULL, .opcodes = NULL, .url = NULL, .safe = true }
};


/**************************
 * Opcode Stack functions *
 **************************/

/**
 * Create an opcode stack.
 */
static inline fungeOpcodeStack* CreateOpcodeStack(void) __attribute__((malloc,warn_unused_result));
/**
 * Free an opcode stack.
 */
static inline void FreeOpcodeStack(fungeOpcodeStack * restrict me) __attribute__((nonnull));
/**
 * Pop a function pointer from an opcode stack, discarding it.
 */
static inline void OpcodeStackPop(fungeOpcodeStack * restrict stack) __attribute__((nonnull));
#ifdef CONCURRENT_FUNGE
/**
 * Duplicate an opcode stack, used for split (t).
 */
static inline fungeOpcodeStack* DuplicateOpcodeStack(const fungeOpcodeStack * restrict old) __attribute__((malloc,nonnull,warn_unused_result));
#endif

static inline fungeOpcodeStack* CreateOpcodeStack(void) {
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

static inline void FreeOpcodeStack(fungeOpcodeStack * restrict me) {
	if (!me)
		return;
	if (me->entries)
		cf_free(me->entries);
	cf_free(me);
}

#ifdef CONCURRENT_FUNGE
static inline fungeOpcodeStack* DuplicateOpcodeStack(const fungeOpcodeStack * restrict old) {
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


bool OpcodeStackAdd(instructionPointer * restrict ip, char opcode, fingerprintOpcode func) {
	fungeOpcodeStack * stack = ip->fingerOpcodes[opcode - 'A'];
	// Do we need to realloc?
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

/**
 * Pop an entry from an opcode stack.
 */
static inline void OpcodeStackPop(fungeOpcodeStack * restrict stack) {
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

bool ManagerCreate(instructionPointer * restrict ip) {
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		ip->fingerOpcodes[i] = CreateOpcodeStack();
		if (!ip->fingerOpcodes[i])
			return false;
	}
	return true;
}

void ManagerFree(instructionPointer * restrict ip) {
	if (!ip)
		return;
	for (int i = 0; i < FINGEROPCODECOUNT; i++) {
		FreeOpcodeStack(ip->fingerOpcodes[i]);
	}
}

#ifdef CONCURRENT_FUNGE
bool ManagerDuplicate(const instructionPointer * restrict oldip,
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

/**
 * Return value is index into ImplementedFingerprints array.
 * -1 means not found.
 */
static inline ssize_t FindFingerPrint(FUNGEDATATYPE fingerprint)
{
	int i = 0;
	bool found = false;
	do {
		if (fingerprint == ImplementedFingerprints[i].fprint) {
			// If we run in a sandbox, can fingerprint be loaded?
			// If not: break, as we know we found the right fingerprint,
			// so no need to search more.
			if (SettingSandbox && !ImplementedFingerprints[i].safe)
				break;
			found = true;
			break;
		}
	} while (ImplementedFingerprints[++i].fprint != 0);
	if (!found)
		return -1;
	return i;
}

bool ManagerLoad(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint) {
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == -1) {
		return false;
	} else {
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

bool ManagerUnload(instructionPointer * restrict ip, FUNGEDATATYPE fingerprint) {
	ssize_t index = FindFingerPrint(fingerprint);
	if (index == -1)
		return false;
	for (size_t i = 0; i < strlen(ImplementedFingerprints[index].opcodes); i++)
		OpcodeStackPop(ip->fingerOpcodes[ImplementedFingerprints[index].opcodes[i] - 'A']);
	return true;
}

#if CHAR_BIT != 8
#  error CHAR_BIT != 8, please make sure the function below the location of this error works on your system.
#endif

void ManagerList(void) {
	int i = 0;
	puts("Supported fingerprints in this binary:");
	do {
		// This hack is here to reconstruct the name from the fingerprint.
		// It will probably break if char isn't 8 bits.
		FUNGEDATATYPE fprint = ImplementedFingerprints[i].fprint;
		char fprintname[5] = { fprint >> 24, fprint >> 16, fprint >> 8, fprint, '\0'};

		printf("0x%x %s%s %s\n",
		       (unsigned)fprint,
		       fprintname,
		       ImplementedFingerprints[i].safe ? "" : " (not available in sandbox mode)",
		       ImplementedFingerprints[i].url ? ImplementedFingerprints[i].url : "");
	} while (ImplementedFingerprints[++i].fprint != 0);
	exit(0);
}

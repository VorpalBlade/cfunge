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

#include "global.h"
#include "interpreter.h"
#include "funge-space/funge-space.h"
#include "vector.h"
#include "stack.h"
#include "ip.h"
#include "input.h"
#include "settings.h"

#include "instructions/iterate.h"
#include "instructions/sysinfo.h"
#include "instructions/io.h"
#include "instructions/execute.h"

#include "fingerprints/manager.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>

/**
 * Either the IP or the IP list.
 */
/*@{*/
#ifdef CONCURRENT_FUNGE
static ipList *IPList = NULL;
#else
static instructionPointer *IP = NULL;
#endif
/*@}*/

/**
 * Print warning on unknown instruction if such warnings are enabled.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void PrintUnknownInstrWarn(FUNGEDATATYPE opcode, instructionPointer * restrict ip)
{
	if (SettingWarnings)
		fprintf(stderr,
		        "WARN: Unknown instruction at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
		        ip->position.x, ip->position.y, (char)opcode, opcode);
}

// These two are called from elsewhere. Avoid code duplication.

FUNGE_ATTR_FAST inline void IfEastWest(instructionPointer * restrict ip)
{
	if (StackPop(ip->stack) == 0)
		ipGoEast(ip);
	else
		ipGoWest(ip);
}

FUNGE_ATTR_FAST inline void IfNorthSouth(instructionPointer * restrict ip)
{
	if (StackPop(ip->stack) == 0)
		ipGoSouth(ip);
	else
		ipGoNorth(ip);
}

#ifdef CONCURRENT_FUNGE
#  define ReturnFromExecuteInstruction(x) return (x)
#else
#  define ReturnFromExecuteInstruction(x) return
#endif


#define PUSHVAL(x, y) \
	case (x): \
		StackPush(ip->stack, (FUNGEDATATYPE)y); \
		break;

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST bool ExecuteInstruction(FUNGEDATATYPE opcode, instructionPointer * restrict ip, ssize_t * threadindex)
#else
FUNGE_ATTR_FAST void ExecuteInstruction(FUNGEDATATYPE opcode, instructionPointer * restrict ip)
#endif
{
	// First check if we are in string mode, and do special stuff then.
	if (ip->mode == ipmSTRING) {
		if (opcode == '"') {
			ip->mode = ipmCODE;
		} else if (opcode != ' ') {
			ip->stringLastWasSpace = false;
			StackPush(ip->stack, opcode);
		} else if (((opcode == ' ') && (!ip->stringLastWasSpace))
		           || (SettingCurrentStandard == stdver93)) {
			ip->stringLastWasSpace = true;
			StackPush(ip->stack, opcode);
		}
	// Next: Is this a fingerprint opcode?
	} else if ((opcode >= 'A') && (opcode <= 'Z')) {
		if (SettingDisableFingerprints) {
			PrintUnknownInstrWarn(opcode, ip);
			ipReverse(ip);
		} else {
			int_fast8_t entry = (int_fast8_t)opcode - 'A';
			if ((ip->fingerOpcodes[entry]->top > 0)
				&& ip->fingerOpcodes[entry]->entries[ip->fingerOpcodes[entry]->top - 1]) {
				// Call the fingerprint.
				ip->fingerOpcodes[entry]->entries[ip->fingerOpcodes[entry]->top - 1](ip);
			} else {
				PrintUnknownInstrWarn(opcode, ip);
				ipReverse(ip);
			}
		}
	// OK a core instruction.
	// Find what one and execute it.
	} else {
		switch (opcode) {
			case ' ': {
				do {
					ipForward(ip, 1);
				} while (FungeSpaceGet(&ip->position) == ' ');
				ip->needMove = false;
				ReturnFromExecuteInstruction(true);
			}
			case 'z':
				break;
			case ';': {
				do {
					ipForward(ip, 1);
				} while (FungeSpaceGet(&ip->position) != ';');
				ReturnFromExecuteInstruction(true);
			}
			case '^':
				ipGoNorth(ip);
				break;
			case '>':
				ipGoEast(ip);
				break;
			case 'v':
				ipGoSouth(ip);
				break;
			case '<':
				ipGoWest(ip);
				break;
			case 'j': {
				// Currently need to do it like this or wrapping
				// won't work for j.
				FUNGEDATATYPE jumps = StackPop(ip->stack);
				if (jumps != 0) {
					fungeVector tmp;
					tmp.x = ip->delta.x;
					tmp.y = ip->delta.y;
					ip->delta.y *= jumps;
					ip->delta.x *= jumps;
					ipForward(ip, 1);
					ip->delta.x = tmp.x;
					ip->delta.y = tmp.y;
				}
				break;
			}
			case '?': {
				// May not be perfectly uniform.
				// If this matters for you, contact me (with a patch).
				long int rnd = random() % 4;
				switch (rnd) {
					case 0: ipGoNorth(ip); break;
					case 1: ipGoEast(ip); break;
					case 2: ipGoSouth(ip); break;
					case 3: ipGoWest(ip); break;
				}
				break;
			}
			case 'r':
				ipReverse(ip);
				break;
			case '[':
				ipTurnLeft(ip);
				break;
			case ']':
				ipTurnRight(ip);
				break;
			case 'x': {
				fungePosition pos;
				pos = StackPopVector(ip->stack);
				ipSetDelta(ip, & pos);
				break;
			}

			PUSHVAL('0', 0)
			PUSHVAL('1', 1)
			PUSHVAL('2', 2)
			PUSHVAL('3', 3)
			PUSHVAL('4', 4)
			PUSHVAL('5', 5)
			PUSHVAL('6', 6)
			PUSHVAL('7', 7)
			PUSHVAL('8', 8)
			PUSHVAL('9', 9)
			PUSHVAL('a', 0xa)
			PUSHVAL('b', 0xb)
			PUSHVAL('c', 0xc)
			PUSHVAL('d', 0xd)
			PUSHVAL('e', 0xe)
			PUSHVAL('f', 0xf)

			case '"':
				ip->mode = ipmSTRING;
				ip->stringLastWasSpace = false;
				break;
			case ':':
				StackDupTop(ip->stack);
				break;

			case '#':
				ipForward(ip, 1);
				break;

			case '_':
				IfEastWest(ip);
				break;
			case '|':
				IfNorthSouth(ip);
				break;
			case 'w': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				if (a < b)
					ipTurnLeft(ip);
				else if (a > b)
					ipTurnRight(ip);
				break;
			}
			case 'k':
#ifdef CONCURRENT_FUNGE
				RunIterate(ip, &IPList, threadindex);
#else
				RunIterate(ip);
#endif
				break;

			case '-': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				StackPush(ip->stack, a - b);
				break;
			}
			case '+': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				StackPush(ip->stack, a + b);
				break;
			}
			case '*': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				StackPush(ip->stack, a * b);
				break;
			}
			case '/': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				if (b == 0)
					StackPush(ip->stack, 0);
				else
					StackPush(ip->stack, a / b);
				break;
			}
			case '%': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				if (b == 0)
					StackPush(ip->stack, 0);
				else
					StackPush(ip->stack, a % b);
				break;
			}

			case '!':
				StackPush(ip->stack, !StackPop(ip->stack));
				break;
			case '`': {
				FUNGEDATATYPE a, b;
				b = StackPop(ip->stack);
				a = StackPop(ip->stack);
				StackPush(ip->stack, a > b);
				break;
			}

			case 'p': {
				fungePosition pos;
				FUNGEDATATYPE a;
				pos = StackPopVector(ip->stack);
				a = StackPop(ip->stack);
				FungeSpaceSetOff(a, &pos, &ip->storageOffset);
				break;
			}
			case 'g': {
				fungePosition pos;
				FUNGEDATATYPE a;
				pos = StackPopVector(ip->stack);
				a = FungeSpaceGetOff(&pos, &ip->storageOffset);
				StackPush(ip->stack, a);
				break;
			}
			case '\'':
				ipForward(ip, 1);
				StackPush(ip->stack, FungeSpaceGet(&ip->position));
				break;
			case 's':
				ipForward(ip, 1);
				FungeSpaceSet(StackPop(ip->stack), &ip->position);
				break;


			case 'n':
				StackClear(ip->stack);
				break;
			case '$':
				StackPopDiscard(ip->stack);
				break;
			case '\\':
				StackSwapTop(ip->stack);
				break;

			case ',': {
				FUNGEDATATYPE a = StackPop(ip->stack);
				// Reverse on failed output/input
				if (cf_putchar_maybe_locked(a) != (char)a)
					ipReverse(ip);
				if (a == '\n')
					if (fflush(stdout) != 0)
						ipReverse(ip);
				break;
			}
			case '.':
				// Reverse on failed output/input
				if (printf("%" FUNGEDATAPRI " ", StackPop(ip->stack)) < 0)
					ipReverse(ip);
				break;

			case '~':
				fflush(stdout);
				StackPush(ip->stack, input_getchar());
				break;
			case '&': {
				FUNGEDATATYPE a;
				bool gotint = false;
				fflush(stdout);
				while (!gotint)
					gotint = input_getint(&a, 10);
				StackPush(ip->stack, a);
				break;
			}

			case 'y':
				RunSysInfo(ip);
				break;

			case '{': {
				FUNGEDATATYPE count;
				fungePosition pos;
				count = StackPop(ip->stack);
				ipForward(ip, 1);
				pos.x = ip->position.x;
				pos.y = ip->position.y;
				ipForward(ip, -1);
				if (!StackStackBegin(ip, &ip->stackstack, count, &pos))
					ipReverse(ip);
				break;
			}
			case '}':
				if (ip->stackstack->size == 1) {
					ipReverse(ip);
				} else {
					FUNGEDATATYPE count;
					count = StackPop(ip->stack);
					if (!StackStackEnd(ip, &ip->stackstack, count))
						ipReverse(ip);
				}
				break;
			case 'u':
				if (ip->stackstack->size == 1) {
					ipReverse(ip);
				} else {
					FUNGEDATATYPE count;
					count = StackPop(ip->stack);
					StackStackTransfer(count,
					                   ip->stackstack->stacks[ip->stackstack->current],
					                   ip->stackstack->stacks[ip->stackstack->current - 1]);
				}
				break;


			case 'i':
				RunFileInput(ip);
				break;
			case 'o':
				RunFileOutput(ip);
				break;
			case '=':
				RunSystemExecute(ip);
				break;

			case '(':
			case ')': {
				FUNGEDATATYPE fpsize = StackPop(ip->stack);
				// Check for sanity (because we won't have any fingerprints
				// outside such a range. This prevents long lockups here.
				if (fpsize < 1) {
					ipReverse(ip);
				} else if (SettingDisableFingerprints) {
					StackPopNDiscard(ip->stack, fpsize);
					ipReverse(ip);
				} else {
					FUNGEDATATYPE fprint = 0;
					if (SettingWarnings && (fpsize > 8)) {
						fprintf(stderr,
						        "WARN: %c (x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI "): count is very large(%" FUNGEDATAPRI "), probably a bug.\n",
						        (char)opcode, ip->position.x, ip->position.y, fpsize);
					}
					while (fpsize--) {
						fprint <<= 8;
						fprint += StackPop(ip->stack);
					}
					if (opcode == '(') {
						if (!ManagerLoad(ip, fprint))
							ipReverse(ip);
					} else {
						if (!ManagerUnload(ip, fprint))
							ipReverse(ip);
					}
				}
				break;
			}

#ifdef CONCURRENT_FUNGE
			case 't':
				*threadindex = ipListDuplicateIP(&IPList, *threadindex);
				break;

#endif /* CONCURRENT_FUNGE */

			case '@':
#ifdef CONCURRENT_FUNGE
				if (IPList->top == 0) {
					fflush(stdout);
					exit(0);
				} else {
					*threadindex = ipListTerminateIP(&IPList, *threadindex);
					//if (IPList->top == 0)
					IPList->ips[*threadindex].needMove = false;
				}
#else
				fflush(stdout);
				exit(0);
#endif /* CONCURRENT_FUNGE */
				break;

			case 'q':
				fflush(stdout);
// We do the wrong thing here when fuzz testing to reduce false positives.
#ifdef FUZZ_TESTING
				exit(0);
#else
				exit((int)StackPop(ip->stack));
#endif
				break;

			default:
				PrintUnknownInstrWarn(opcode, ip);
				ipReverse(ip);
		}
	}
	ReturnFromExecuteInstruction(false);
}


#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void ThreadForward(instructionPointer * ip)
{
	assert(ip != NULL);

	if (ip->needMove)
		ipForward(ip, 1);
	else
		ip->needMove = true;
}
#endif


FUNGE_ATTR_FAST FUNGE_ATTR_NORET
static inline void interpreterMainLoop(void)
{
#ifdef CONCURRENT_FUNGE
	while (true) {
		ssize_t i = IPList->top;
		while (i >= 0) {
			bool retval;
			FUNGEDATATYPE opcode;

			opcode = FungeSpaceGet(&IPList->ips[i].position);
#    ifndef DISABLE_TRACE
			if (SettingTraceLevel > 8) {
				fprintf(stderr, "tix=%zu tid=%" FUNGEDATAPRI " x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
				        i, IPList->ips[i].ID,
				        IPList->ips[i].position.x, IPList->ips[i].position.y, (char)opcode, opcode);
				PrintStackTop(IPList->ips[i].stack);
			} else if (SettingTraceLevel > 3) {
				fprintf(stderr, "tix=%zu tid=%" FUNGEDATAPRI " x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
				        i, IPList->ips[i].ID,
				        IPList->ips[i].position.x, IPList->ips[i].position.y, (char)opcode, opcode);
			} else if (SettingTraceLevel > 2)
				fprintf(stderr, "%c", (char)opcode);
#    endif /* DISABLE_TRACE */

			retval = ExecuteInstruction(opcode, &IPList->ips[i], &i);
			ThreadForward(&IPList->ips[i]);
			if (!retval)
				i--;
		}
	}
#else /* CONCURRENT_FUNGE */
	while (true) {
		FUNGEDATATYPE opcode;

		opcode = FungeSpaceGet(&IP->position);
#    ifndef DISABLE_TRACE
		if (SettingTraceLevel > 8) {
			fprintf(stderr, "x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
			        IP->position.x, IP->position.y, (char)opcode, opcode);
			PrintStackTop(IP->stack);
		} else if (SettingTraceLevel > 3) {
			fprintf(stderr, "x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
			        IP->position.x, IP->position.y, (char)opcode, opcode);
		} else if (SettingTraceLevel > 2)
			fprintf(stderr, "%c", (char)opcode);
#    endif /* DISABLE_TRACE */

		ExecuteInstruction(opcode, IP);
		if (IP->needMove)
			ipForward(IP, 1);
		else
			IP->needMove = true;
	}
#endif /* CONCURRENT_FUNGE */
}


#ifndef NDEBUG
// Used with debugging for freeing stuff at end of the program.
// Not needed, but useful to check that free functions works,
// and for detecting real memory leaks.
static void DebugFreeThings(void)
{
# ifdef CONCURRENT_FUNGE
	ipListFree(IPList);
# else
	ipFree(IP);
# endif
	FungeSpaceFree();
}
#endif

FUNGE_ATTR_FAST void interpreterRun(const char *filename)
{
	if (!FungeSpaceCreate()) {
		perror("Couldn't create funge space!?");
		exit(EXIT_FAILURE);
	}
#ifndef NDEBUG
	atexit(&DebugFreeThings);
#endif
	if (!FungeSpaceLoad(filename)) {
		fprintf(stderr, "Failed to process file \"%s\": %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
#ifdef CONCURRENT_FUNGE
	IPList = ipListCreate();
	if (IPList == NULL) {
		perror("Couldn't create instruction pointer list!?");
		exit(EXIT_FAILURE);
	}
#else
	IP = ipCreate();
	if (IP == NULL) {
		perror("Couldn't create instruction pointer!?");
		exit(EXIT_FAILURE);
	}
#endif
	{
		struct timeval tv;
		if (gettimeofday(&tv, NULL)) {
			perror("Couldn't get time of day?!");
			exit(EXIT_FAILURE);
		}
		// Set up randomness
		srandom(tv.tv_usec);
	}
	interpreterMainLoop();
}

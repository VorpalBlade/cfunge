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

#include "fingerprints/manager.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>

#ifdef CONCURRENT_FUNGE
static ipList *IPList = NULL;
#else
static instructionPointer *IP = NULL;
#endif

#define PUSHVAL(x, y) \
	case (x): \
		StackPush((FUNGEDATATYPE)y, ip->stack); \
		break;

#define GO_WEST  ipSetDelta(ip, & (fungeVector) { .x = -1, .y =  0 });
#define GO_EAST  ipSetDelta(ip, & (fungeVector) { .x =  1, .y =  0 });
#define GO_NORTH ipSetDelta(ip, & (fungeVector) { .x =  0, .y = -1 });
#define GO_SOUTH ipSetDelta(ip, & (fungeVector) { .x =  0, .y =  1 });

#ifdef CONCURRENT_FUNGE
#  define ReturnFromExecuteInstruction(x) return (x)
#else
#  define ReturnFromExecuteInstruction(x) return
#endif


/**
 * Print warning on unknown instruction if such warnings are enabled.
 */
static inline void PrintUnknownInstrWarn(FUNGEDATATYPE opcode, instructionPointer * restrict ip) __attribute__((nonnull));

static inline void PrintUnknownInstrWarn(FUNGEDATATYPE opcode, instructionPointer * restrict ip) {
	if (SettingWarnings)
		fprintf(stderr,
		        "WARN: Unknown instruction at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
		        ip->position.x, ip->position.y, (char)opcode, opcode);
}


#ifdef CONCURRENT_FUNGE
bool ExecuteInstruction(FUNGEDATATYPE opcode, instructionPointer * restrict ip, ssize_t * threadindex)
#else
void ExecuteInstruction(FUNGEDATATYPE opcode, instructionPointer * restrict ip)
#endif
{
	// First check if we are in string mode, and do special stuff then.
	if (ip->mode == ipmSTRING) {
		if (opcode == '"') {
			ip->mode = ipmCODE;
		} else if (opcode != ' ') {
			ip->StringLastWasSpace = false;
			StackPush(opcode, ip->stack);
		} else if (((opcode == ' ') && (!ip->StringLastWasSpace))
		           || (SettingCurrentStandard == stdver93)) {
			ip->StringLastWasSpace = true;
			StackPush(opcode, ip->stack);
		}
	// Next: Is this a fingerprint opcode?
	} else if ((opcode >= 'A') && (opcode <= 'Z')) {
		if (!SettingEnableFingerprints) {
			PrintUnknownInstrWarn(opcode, ip);
			ipReverse(ip);
		} else {
			int_fast8_t entry = (char)opcode - 'A';
			if ((ip->fingerOpcodes[entry]->top > 0) && ip->fingerOpcodes[entry]->entries[ip->fingerOpcodes[entry]->top - 1])
				ip->fingerOpcodes[entry]->entries[ip->fingerOpcodes[entry]->top - 1](ip);
			else {
				PrintUnknownInstrWarn(opcode, ip);
				ipReverse(ip);
			}
		}
	// OK a core instruction.
	// Find what one and execute it.
	} else {
		switch (opcode) {
			case ' ':
				{
					do {
						ipForward(1, ip);
					} while (fungeSpaceGet(&ip->position) == ' ');
					ip->NeedMove = false;
				}
				ReturnFromExecuteInstruction(true);
			case 'z':
				break;
			case ';':
				{
					do {
						ipForward(1, ip);
					} while (fungeSpaceGet(&ip->position) != ';');
					ReturnFromExecuteInstruction(true);
				}
			case '^':
				GO_NORTH
				break;
			case '>':
				GO_EAST
				break;
			case 'v':
				GO_SOUTH
				break;
			case '<':
				GO_WEST
				break;
			case 'j':
				{
					// Currently need to do it like this or wrapping
					// won't work for j.
					FUNGEDATATYPE jumps = StackPop(ip->stack);
					if (jumps != 0) {
						fungeVector tmp;
						tmp.x = ip->delta.x;
						tmp.y = ip->delta.y;
						ip->delta.y *= jumps;
						ip->delta.x *= jumps;
						ipForward(1, ip);
						ip->delta.x = tmp.x;
						ip->delta.y = tmp.y;
					}
					break;
				}
			case '?':
				{
					// May not be perfectly uniform.
					// If this matters for you, contact me (with a patch).
					long int rnd = random() % 4;
					assert((rnd >= 0) && (rnd <= 3));
					switch (rnd) {
						case 0: GO_NORTH break;
						case 1: GO_EAST break;
						case 2: GO_SOUTH break;
						case 3: GO_WEST break;
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
			case 'x':
				{
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
				ip->StringLastWasSpace = false;
				break;
			case ':':
				StackDupTop(ip->stack);
				break;

			case '#':
				ipForward(1, ip);
				break;

			case '_':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					if (a == 0)
						GO_EAST
					else
						GO_WEST
					break;
				}
			case '|':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					if (a == 0)
						GO_SOUTH
					else
						GO_NORTH
					break;
				}
			case 'w':
				{
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
				RunIterate(ip, threadindex);
#else
				RunIterate(ip);
#endif
				break;

			case '-':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					StackPush(a - b, ip->stack);
					break;
				}
			case '+':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					StackPush(a + b, ip->stack);
					break;
				}
			case '*':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					StackPush(a * b, ip->stack);
					break;
				}
			case '/':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					if (b == 0)
						StackPush(0, ip->stack);
					else
						StackPush(a / b, ip->stack);
					break;
				}
			case '%':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					if (b == 0)
						StackPush(0, ip->stack);
					else
						StackPush(a % b, ip->stack);
					break;
				}

			case '!':
				{
					FUNGEDATATYPE a;
					a = StackPop(ip->stack);
					StackPush(!a, ip->stack);
					break;
				}
			case '`':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(ip->stack);
					a = StackPop(ip->stack);
					StackPush(a > b, ip->stack);
					break;
				}

			case 'p':
				{
					fungePosition pos;
					FUNGEDATATYPE a;
					pos = StackPopVector(ip->stack);
					a = StackPop(ip->stack);
					fungeSpaceSetOff(a, &pos, &ip->storageOffset);
					break;
				}
			case 'g':
				{
					fungePosition pos;
					FUNGEDATATYPE a;
					pos = StackPopVector(ip->stack);
					a = fungeSpaceGetOff(&pos, &ip->storageOffset);
					StackPush(a, ip->stack);
					break;
				}
			case '\'':
				{
					FUNGEDATATYPE a;
					ipForward(1, ip);
					a = fungeSpaceGet(&ip->position);
					StackPush(a, ip->stack);
					break;
				}
			case 's':
				{
					FUNGEDATATYPE a;
					a = StackPop(ip->stack);
					ipForward(1, ip);
					fungeSpaceSet(a, &ip->position);
					break;
				}

			case 'n':
				StackClear(ip->stack);
				break;
			case '$':
				StackPopDiscard(ip->stack);
				break;
			case '\\':
				StackSwapTop(ip->stack);
				break;

			case ',':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					putchar((char)a);
					if (a == '\n') fflush(stdout);
					break;
				}
			case '.':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					printf("%" FUNGEDATAPRI " ", a);
					break;
				}

			case '~':
				{
					FUNGEDATATYPE a = 0;
					fflush(stdout);
					a = input_getchar();
					StackPush(a, ip->stack);
					break;
				}
			case '&':
				{
					FUNGEDATATYPE a = 0;
					bool gotint = false;
					fflush(stdout);
					while (!gotint)
						gotint = input_getint(&a, 10);
					StackPush(a, ip->stack);
					break;
				}

			case 'y':
				RunSysInfo(ip);
				break;

			case '{':
				{
					FUNGEDATATYPE count;
					fungePosition pos;
					count = StackPop(ip->stack);
					ipForward(1, ip);
					pos.x = ip->position.x;
					pos.y = ip->position.y;
					ipForward(-1, ip);
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


			case '(':
			case ')':
				{
					FUNGEDATATYPE fpsize = StackPop(ip->stack);
					// Check for sanity (because we won't have any fingerprints
					// outside such a range. This prevents long lockups here.
					if (fpsize < 1) {
						ipReverse(ip);
					} else if (!SettingEnableFingerprints) {
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
				{
					*threadindex = ipListDuplicateIP(&IPList, *threadindex);
					break;
				}

#endif /* CONCURRENT_FUNGE */

			case '@':
#ifdef CONCURRENT_FUNGE
				if (IPList->top == 0) {
					fflush(stdout);
					exit(0);
				} else
					*threadindex = ipListTerminateIP(&IPList, *threadindex);
#else
				fflush(stdout);
				exit(0);
#endif /* CONCURRENT_FUNGE */
				break;
			case 'q':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					fflush(stdout);
					exit((int)a);
					break;
				}

			default:
				PrintUnknownInstrWarn(opcode, ip);
				ipReverse(ip);
		}
	}
	ReturnFromExecuteInstruction(false);
}

static inline void ThreadForward(instructionPointer * ip)
{
	assert(ip != NULL);

	if (ip->NeedMove)
		ipForward(1, ip);
	else
		ip->NeedMove = true;
}

static inline void interpreterMainLoop(void) __attribute__((noreturn));

static inline void interpreterMainLoop(void)
{
#ifdef CONCURRENT_FUNGE
	while (true) {
		ssize_t i = IPList->top;
		while(i >= 0) {
			bool retval;
			FUNGEDATATYPE opcode;

			opcode = fungeSpaceGet(&IPList->ips[i].position);
#    ifndef DISABLE_TRACE
			if (SettingTraceLevel > 3)
				fprintf(stderr, "tix=%zu tid=%" FUNGEDATAPRI " x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
				        i, IPList->ips[i].ID,
				        IPList->ips[i].position.x, IPList->ips[i].position.y, (char)opcode, opcode);
			else if (SettingTraceLevel > 2)
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

		opcode = fungeSpaceGet(&IP->position);
#    ifndef DISABLE_TRACE
		if (SettingTraceLevel > 3)
			fprintf(stderr, "x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n",
			        IP->position.x, IP->position.y, (char)opcode, opcode);
		else if (SettingTraceLevel > 2)
			fprintf(stderr, "%c", (char)opcode);
#    endif /* DISABLE_TRACE */

		ExecuteInstruction(opcode, IP);
		if (IP->NeedMove)
			ipForward(1, IP);
		else
			IP->NeedMove = true;
	}
#endif /* CONCURRENT_FUNGE */
}


#ifndef NDEBUG
// Used with debugging for freeing stuff at end of program
// not needed, but useful to check that free functions works,
// and for detecting real memory leaks.
static void DebugFreeThings(void) {
# ifdef CONCURRENT_FUNGE
	ipListFree(IPList);
# else
	ipFree(IP);
# endif
	fungeSpaceFree();
}
#endif

void interpreterRun(const char *filename)
{
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
	if(!fungeSpaceCreate()) {
		perror("Couldn't create funge space!?");
		exit(EXIT_FAILURE);
	}
	if (!fungeSpaceLoad(filename)) {
		fprintf(stderr, "Failed to process file %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	{
		struct timeval tv;
		if (gettimeofday(&tv, NULL)) {
			perror("Couldn't get time of day?!");
			exit(EXIT_FAILURE);
		}
		// Set up randomness
		srandom(tv.tv_usec);
	}
#ifndef NDEBUG
	atexit(&DebugFreeThings);
#endif
	interpreterMainLoop();
}

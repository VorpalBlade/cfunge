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

fungeSpace *fspace = NULL;
static fungeStackStack *stackStack = NULL;
static instructionPointer *IP = NULL;

#define PUSHVAL(x, y) \
	case (x): \
		StackPush((FUNGEDATATYPE)y, ip->stack); \
		break;

#define GO_WEST ipSetDelta(ip, & (fungeVector) { .x = -1, .y = 0 });
#define GO_EAST ipSetDelta(ip, & (fungeVector) { .x = 1, .y = 0 });
#define GO_NORTH ipSetDelta(ip, & (fungeVector) { .x = 0, .y = -1 });
#define GO_SOUTH ipSetDelta(ip, & (fungeVector) { .x = 0, .y = 1 });

static inline void ExecuteInstruction(FUNGEDATATYPE opcode, instructionPointer *ip) {
	if (ip->mode == ipmSTRING) {
		if (opcode == '"') {
			ip->mode = ipmCODE;
		} else if (opcode != ' ') {
			ip->StringLastWasSpace = false;
			StackPush(opcode, ip->stack);
		} else if ((opcode == ' ') && (!ip->StringLastWasSpace)) {
			ip->StringLastWasSpace = true;
			StackPush(opcode, ip->stack);
		}
	} else if ((opcode >= 'A') && (opcode <= 'Z')) {
		if (!SettingEnableFingerprints)
			ipReverse(ip);
		else if (ip->fingerOpcodes && ip->fingerOpcodes[(char)opcode - 'A']->entries[ip->fingerOpcodes[(char)opcode - 'A']->top - 1] != NULL)
			ip->fingerOpcodes[(char)opcode - 'A']->entries[ip->fingerOpcodes[(char)opcode - 'A']->top - 1](ip);
		else
			ipReverse(ip);
	} else {
		switch (opcode) {
			case ' ':
				{
					do {
						ipForward(1, ip);
					} while (fungeSpaceGet(fspace, &ip->position) == ' ');
					ipForward(-1, ip);
				}
				return;
			case 'z':
				return;
			case ';':
				{
					do {
						ipForward(1, ip);
					} while (fungeSpaceGet(fspace, &ip->position) != ';');
					return;
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
					// FIXME: May not be uniform
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
				RunIterate(ip);
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
					fungeSpaceSetOff(fspace, a, &pos, &ip->storageOffset);
					break;
				}
			case 'g':
				{
					fungePosition pos;
					FUNGEDATATYPE a;
					pos = StackPopVector(ip->stack);
					a = fungeSpaceGetOff(fspace, &pos, &ip->storageOffset);
					StackPush(a, ip->stack);
					break;
				}
			case '\'':
				{
					FUNGEDATATYPE a;
					ipForward(1, ip);
					a = fungeSpaceGet(fspace, &ip->position);
					StackPush(a, ip->stack);
					break;
				}
			case 's':
				{
					FUNGEDATATYPE a;
					a = StackPop(ip->stack);
					ipForward(1, ip);
					fungeSpaceSet(fspace, a, &ip->position);
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
						gotint = input_getint(&a);
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


			// TODO: insert fingerprint code here.
			case '(':
			case ')':
				{
					FUNGEDATATYPE fpsize = StackPop(ip->stack);
					if (fpsize < 0) {
						ipReverse(ip);
					} else if (!SettingEnableFingerprints) {
						StackPopNDiscard(ip->stack, fpsize);
						ipReverse(ip);
					} else {
						FUNGEDATATYPE fprint = 0;
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

			case '@':
				exit(0);
				break;
			case 'q':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					exit((int)a);
					break;
				}

			default:
				if (SettingWarnings)
					fprintf(stderr, "WARN: Unknown instruction at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n", ip->position.x, ip->position.y, (char)opcode, opcode);
				ipReverse(ip);
		}
	}
}


// Wrapper, to allow real function to be inlined
void RunInstruction(FUNGEDATATYPE instruction, instructionPointer *ip)
{
	ExecuteInstruction(instruction, ip);
}

static void interpreterMainLoop(void) __attribute__((noreturn));


static void interpreterMainLoop(void)
{
	FUNGEDATATYPE opcode;

	while (true) {
		opcode = fungeSpaceGet(fspace, &IP->position);
#ifndef DISABLE_TRACE
		if (SettingTraceLevel > 3)
			fprintf(stderr, "x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ": %c (%" FUNGEDATAPRI ")\n", IP->position.x, IP->position.y, (char)opcode, opcode);
		else if (SettingTraceLevel > 2)
			fprintf(stderr, "%c", (char)opcode);
#endif

		ExecuteInstruction(opcode, IP);
		ipForward(1, IP);
	}
}

void interpreterRun(const char *filename)
{
	stackStack = StackStackCreate();
	if (stackStack == NULL)
		exit(EXIT_FAILURE);
	IP = ipCreate(stackStack);
	if (IP == NULL)
		exit(EXIT_FAILURE);
	fspace = fungeSpaceCreate();
	if (fspace == NULL)
		exit(EXIT_FAILURE);
	if (!fungeSpaceLoad(fspace, filename)) {
		fprintf(stderr, "Failed to process file %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	{
		struct timeval tv;
		if (gettimeofday(&tv, NULL)) {
			perror("Couldn't get time of day?!");
			abort();
		}
		// Set up randomness
		srandom(tv.tv_usec);
	}

	interpreterMainLoop();
}

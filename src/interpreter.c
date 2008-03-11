/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

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
#include "funge-space/b93/funge-space.h"
#include "vector.h"
#include "stack.h"
#include "ip.h"
#include "input.h"

#include "instructions/iterate.h"
#include "instructions/sysinfo.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static fungeSpace *fspace;
static fungeStackStack *stackStack;
static instructionPointer *ip;

#define PUSHVAL(x, y) \
	case (x): \
		StackPush((FUNGEDATATYPE)y, ip->stack); \
		break;

#define GO_WEST ipSetDelta(ip, & (fungeVector) { .x = -1, .y = 0 });
#define GO_EAST ipSetDelta(ip, & (fungeVector) { .x = 1, .y = 0 });
#define GO_NORTH ipSetDelta(ip, & (fungeVector) { .x = 0, .y = -1 });
#define GO_SOUTH ipSetDelta(ip, & (fungeVector) { .x = 0, .y = 1 });

static inline void ExecuteInstruction(FUNGEDATATYPE opcode) {
	if (ip->mode == ipmSTRING) {
		if (opcode == '"') {
			ip->mode = ipmCODE;
		} else {
			StackPush(opcode, ip->stack);
		}
	} else {
		switch (opcode) {
			case ' ':
				return;
			case 'z':
				return;
			case ';':
				{
					do {
						ipForward(1, ip, fspace);
					} while ((opcode = fungeSpaceGet(fspace, &ip->position)) != ';');
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
					if (jumps != 0)
						ipForward(jumps, ip, fspace);
					break;
				}
			case '?':
				{
					// FIXME: May not be uniform
					long int rnd = random() % 4;
					switch (rnd) {
						case 0: GO_NORTH break;
						case 1: GO_EAST break;
						case 2: GO_SOUTH break;
						case 3: GO_WEST break;
						default: fprintf(stderr, "Random: %ld not in range!?\n", rnd); abort(); break;
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
				break;
			case ':':
				StackDupTop(ip->stack);
				break;

			case '#':
				ipForward(1, ip, fspace);
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
				RunIterate(ip, stackStack, fspace);
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
					ipForward(1, ip, fspace);
					a = fungeSpaceGet(fspace, &ip->position);
					StackPush(a, ip->stack);
					break;
				}
			case 's':
				{
					FUNGEDATATYPE a;
					a = StackPop(ip->stack);
					ipForward(1, ip, fspace);
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
					fflush(stdout);
					break;
				}
			case '.':
				{
					FUNGEDATATYPE a = StackPop(ip->stack);
					printf("%ld ", a);
					fflush(stdout);
					break;
				}

			case '~':
				{
					FUNGEDATATYPE a = 0;
					a = input_getchar();
					StackPush(a, ip->stack);
					break;
				}
			case '&':
				{
					FUNGEDATATYPE a = 0;
					bool gotint = false;
					while (!gotint)
						gotint = input_getint(&a);
					StackPush(a, ip->stack);
					break;
				}

			case 'y':
				RunSysInfo(ip, stackStack, fspace);
				break;

			case '{':
				{
					FUNGEDATATYPE count;
					fungePosition pos;
					count = StackPop(ip->stack);
					ipForward(1, ip, fspace);
					pos.x = ip->position.x;
					pos.y = ip->position.y;
					ipForward(-1, ip, fspace);
					StackStackBegin(ip, &stackStack, count, &pos);
					break;
				}
			case '}':
				if (stackStack->size == 1) {
					ipReverse(ip);
				} else {
					FUNGEDATATYPE count;
					fungePosition pos;
					count = StackPop(ip->stack);
					StackStackEnd(ip, &stackStack, count);
				}
				break;
			case 'u':
				if (stackStack->size == 1) {
					ipReverse(ip);
				} else {
					FUNGEDATATYPE count;
					count = StackPop(ip->stack);
					StackStackTransfer(count,
					                   stackStack->stacks[stackStack->current],
					                   stackStack->stacks[stackStack->current - 1]);
				}
				break;

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
				fprintf(stderr, "Unknown instruction at x=%ld y=%ld: %c (%ld)\n", ip->position.x, ip->position.y, (char)opcode, opcode);
				ipReverse(ip);
				//exit(EXIT_FAILURE);
		}
	}
}


// Wrapper, to allow real function to be inlined
void RunInstruction(FUNGEDATATYPE instruction)
{
	ExecuteInstruction(instruction);
}

static int interpreterMainLoop(void)
{
	FUNGEDATATYPE opcode;

	while (true) {
		opcode = fungeSpaceGet(fspace, &ip->position);
		//fprintf(stderr, "x=%ld y=%ld: %c (%ld)\n", ip->position.x, ip->position.y, (char)opcode, opcode);
		//fprintf(stderr, "%c", (char)opcode);
		ExecuteInstruction(opcode);
		ipForward(1, ip, fspace);
	}

	return 0;
}

int interpreterRun(int argc, char *argv[])
{
	bool retval;

	stackStack = StackStackCreate();
	if (stackStack == NULL)
		return EXIT_FAILURE;
	ip = ipCreate(stackStack);
	if (ip == NULL)
		return EXIT_FAILURE;
	fspace = fungeSpaceCreate();
	if (fspace == NULL)
		return EXIT_FAILURE;
	retval = fungeSpaceLoad(fspace, argv[1]);
	if (!retval) {
		fprintf(stderr, "Failed to process file %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	// Set up randomness
	srandom(time(NULL));


	return interpreterMainLoop();
}

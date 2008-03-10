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
		StackPush((FUNGEDATATYPE)y, stackStack->current->stack); \
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
			StackPush(opcode, stackStack->current->stack);
		}
	} else {
		switch (opcode) {
			case ' ':
				return;
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
				StackDupTop(stackStack->current->stack);
				break;

			case '#':
				ipForward(1, ip, fspace);
				break;

			case '_':
				{
					FUNGEDATATYPE a = StackPop(stackStack->current->stack);
					if (a == 0)
						GO_EAST
					else
						GO_WEST
					break;
				}

			case '|':
				{
					FUNGEDATATYPE a = StackPop(stackStack->current->stack);
					if (a == 0)
						GO_SOUTH
					else
						GO_NORTH
					break;
				}


			case '-':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					StackPush(a - b, stackStack->current->stack);
					break;
				}
			case '+':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					StackPush(a + b, stackStack->current->stack);
					break;
				}
			case '*':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					StackPush(a * b, stackStack->current->stack);
					break;
				}
			case '/':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					if (b == 0)
						StackPush(0, stackStack->current->stack);
					else
						StackPush(a / b, stackStack->current->stack);
					break;
				}
			case '%':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					if (b == 0)
						StackPush(0, stackStack->current->stack);
					else
						StackPush(a % b, stackStack->current->stack);
					break;
				}

			case '!':
				{
					FUNGEDATATYPE a;
					a = StackPop(stackStack->current->stack);
					StackPush(!a, stackStack->current->stack);
					break;
				}
			case '`':
				{
					FUNGEDATATYPE a, b;
					b = StackPop(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					StackPush(a > b, stackStack->current->stack);
					break;
				}

			case 'p':
				{
					fungePosition pos;
					FUNGEDATATYPE a;
					pos = StackPopVector(stackStack->current->stack);
					a = StackPop(stackStack->current->stack);
					fungeSpaceSet(fspace, a, &pos);
					break;
				}
			case 'g':
				{
					fungePosition pos;
					FUNGEDATATYPE a;
					pos = StackPopVector(stackStack->current->stack);
					a = fungeSpaceGet(fspace, &pos);
					StackPush(a, stackStack->current->stack);
					break;
				}

			case '$':
				StackPopDiscard(stackStack->current->stack);
				break;
			case '\\':
				StackSwapTop(stackStack->current->stack);
				break;

			case ',':
				{
					FUNGEDATATYPE a = StackPop(stackStack->current->stack);
					putchar((char)a);
					fflush(stdout);
					break;
				}
			case '.':
				{
					FUNGEDATATYPE a = StackPop(stackStack->current->stack);
					printf("%ld ", a);
					fflush(stdout);
					break;
				}

			case '~':
				{
					FUNGEDATATYPE a = 0;
					a = getchar();
					StackPush(a, stackStack->current->stack);
					break;
				}
			case '&':
				{
					FUNGEDATATYPE a = 0;
					int retval = scanf("%li", &a);
					if (retval == 1)
						StackPush(a, stackStack->current->stack);
					else
						fprintf(stderr, "Oops, scanf in & returned %d", retval);
					break;
				}

			case '@':
				exit(0);
				break;
			case 'q':
				{
					FUNGEDATATYPE a = StackPop(stackStack->current->stack);
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

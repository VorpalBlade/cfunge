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

static fungeSpace *fspace;
static fungeStackStack *stackStack;
static instructionPointer *ip;

#define PUSHVAL(x) \
	case (x): \
		StackPush((FUNGEDATATYPE)x, stackStack->current->stack); \
		break;

static inline void ExecuteInstruction(FUNGEDATATYPE opcode) {
	switch (opcode) {
		case ' ':
			return;
		case '<':
			ipSetDelta(ip, & (fungeVector) { .x = -1, .y = 0 });
			break;
		case '>':
			ipSetDelta(ip, & (fungeVector) { .x = 1, .y = 0 });
			break;
		case '^':
			ipSetDelta(ip, & (fungeVector) { .x = 0, .y = -1 });
			break;
		case 'v':
			ipSetDelta(ip, & (fungeVector) { .x = 0, .y = 1 });
			break;
		PUSHVAL('0')
		PUSHVAL('1')
		PUSHVAL('2')
		PUSHVAL('3')
		PUSHVAL('4')
		PUSHVAL('5')
		PUSHVAL('6')
		PUSHVAL('7')
		PUSHVAL('8')
		PUSHVAL('9')
		PUSHVAL('a')
		PUSHVAL('b')
		PUSHVAL('c')
		PUSHVAL('d')
		PUSHVAL('e')
		PUSHVAL('f')

		default:
			fprintf(stderr, "Unknown instruction: %c (%ld)\n", (char)opcode, opcode);
			exit(EXIT_FAILURE);
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
		fprintf(stderr, "x=%ld y=%ld: %c (%ld)\n", ip->position.x, ip->position.y, (char)opcode, opcode);
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
		fprintf(stderr, "Failed to handle file: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return interpreterMainLoop();
}

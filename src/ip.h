#ifndef _HAD_SRC_IP_H
#define _HAD_SRC_IP_H

#include <sys/types.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

typedef fungeVector ipDelta;

typedef struct {
	fungePosition   position;
	ipDelta         delta;
	fungeStackStack stackstack;
} instructionPointer;

typedef struct {
	size_t              count;
	instructionPointer* entries;
} ipList;

// steps let you take several steps at once.
extern void ipForward(int_fast64_t steps, instructionPointer * ip, fungeSpace *space);

#endif

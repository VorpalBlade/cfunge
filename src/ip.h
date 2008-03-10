#ifndef _HAD_SRC_IP_H
#define _HAD_SRC_IP_H

#include <sys/types.h>
#include <stdint.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

typedef fungeVector ipDelta;

typedef enum { ipmCODE = 0, ipmSTRING } ipMode;

typedef struct {
	fungePosition     position;
	ipDelta           delta;
	ipMode            mode;
	fungeStackStack * stackstack;
} instructionPointer;

typedef struct {
	size_t              count;
	instructionPointer* entries;
} ipList;

extern instructionPointer * ipCreate(fungeStackStack *stackstack) __attribute__((nonnull,warn_unused_result));
extern void                 ipFree(instructionPointer * ip);

// steps let you take several steps at once.
extern void ipForward(int_fast64_t steps, instructionPointer * ip, fungeSpace *space) __attribute__((nonnull));

extern void ipReverse(instructionPointer * ip) __attribute__((nonnull));
extern void ipTurnLeft(instructionPointer * ip) __attribute__((nonnull));
extern void ipTurnRight(instructionPointer * ip) __attribute__((nonnull));
extern void ipSetDelta(instructionPointer * ip, const ipDelta * delta) __attribute__((nonnull));

#endif

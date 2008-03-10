#include "global.h"
#include "ip.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

instructionPointer *
ipCreate(fungeStackStack *stackstack)
{
	instructionPointer * tmp = cf_malloc(sizeof(instructionPointer));
	if (!tmp)
		return NULL;

	tmp->position.x = 0;
	tmp->position.y = 0;
	tmp->delta.x = 1;
	tmp->delta.y = 1;
	tmp->stackstack = stackstack;
	return tmp;
}

void
ipFree(instructionPointer * ip)
{
	// TODO: Should we free stackstack?
	ip->stackstack = NULL;
	cf_free(ip);
}


void ipForward(int_fast64_t steps, instructionPointer * ip, fungeSpace *space)
{
	ip->position.x += ip->delta.x * steps;
	ip->position.y += ip->delta.y * steps;
	fungeSpaceWrapInPlace(space, &ip->position);
}


void
ipReverse(instructionPointer * ip)
{
	ip->delta.x = -ip->delta.x;
	ip->delta.y = -ip->delta.y;
}

void
ipTurnLeft(instructionPointer * ip)
{
	// TODO
}

void
ipTurnRight(instructionPointer * ip)
{
	// TODO
}

void
ipSetDelta(instructionPointer * ip, const ipDelta * delta)
{
	ip->delta.x = delta->x;
	ip->delta.y = delta->y;
}

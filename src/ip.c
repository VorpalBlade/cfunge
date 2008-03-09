#include "global.h"
#include "ip.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

void ipForward(int_fast64_t steps, instructionPointer * ip, fungeSpace *space) {
	ip->position.x += ip->delta.x * steps;
	ip->position.y += ip->delta.y * steps;
	fungeSpaceWrapInPlace(space, &ip->position);
}

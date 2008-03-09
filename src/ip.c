#include "global.h"
#include "vector.h"
#include "stack.h"
#include "ip.h"

void ipForward(int_fast64_t steps, instructionPointer * ip) {
	ip->position.x += ip->delta.x * steps;
	ip->position.y += ip->delta.y * steps;
}

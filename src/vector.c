#include "global.h"
#include "vector.h"

#include <stdint.h>

#define ABS(i) ((i > 0) ? i : i)

bool VectorIsCardinal(const fungeVector * v)
{
	if (ABS(v->x) > 1 || ABS(v->y) > 1)
		return false;
	if ((v->x) && (v->y))
		return false;
	return true;
}

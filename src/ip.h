#ifndef _HAD_SRC_IP_H
#define _HAD_SRC_IP_H

#include <sys/types.h>

#include "vector.h"
#include "stack.h"

typedef fungeVector ipDelta;

typedef struct {
	fungePosition   position;
	ipDelta         direction;
	fungeStackStack stackstack;
} ip;

typedef struct {
	size_t count;
	ip*    entries;
} ipList;

#endif

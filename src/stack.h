#ifndef _HAD_SRC_STACK_H
#define _HAD_SRC_STACK_H

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"

typedef struct {
	size_t         count;
	size_t         current;
	fungePosition  storageOffset;
	int_fast64_t  *entries;
} fungeStack;



typedef struct {
	size_t        count;
	size_t        current;
	fungeStack   *stacks;
} fungeStackStack;


#endif

#ifndef _HAD_SRC_INTERPRETER_H
#define _HAD_SRC_INTERPRETER_H

#include <sys/types.h>
#include <stdint.h>

#include "global.h"
#include "vector.h"
#include "stack.h"
#include "funge-space/b93/funge-space.h"

extern void RunInstruction(FUNGEDATATYPE instruction);

extern int interpreterRun(int argc, char *argv[]) __attribute__((warn_unused_result));

#endif

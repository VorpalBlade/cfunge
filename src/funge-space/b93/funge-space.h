#ifndef _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H
#define _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H

#include "../../global.h"
#include "../../vector.h"
#include <stdint.h>
#include <stdbool.h>

struct _fungeSpace;
typedef struct _fungeSpace fungeSpace;

extern fungeSpace    * fungeSpaceCreate(void) __attribute__((warn_unused_result));
extern void            fungeSpaceFree(fungeSpace * me);
extern FUNGEDATATYPE   fungeSpaceGet(fungeSpace * me, const fungePosition * position) __attribute__((warn_unused_result));
extern void            fungeSpaceSet(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position);
// This mallocs with boehm-gc
extern fungePosition * fungeSpaceWrap(fungeSpace * me, const fungePosition * position) __attribute__((warn_unused_result));
// Version that modifies the variable it is passed.
extern void            fungeSpaceWrapInPlace(fungeSpace * me, fungePosition * position);
extern bool            fungeSpaceLoad(fungeSpace * me, const char * filename) __attribute__((warn_unused_result));

#endif

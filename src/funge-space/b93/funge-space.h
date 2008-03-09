#ifndef _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H
#define _HAD_SRC_FUNGE_SPACE_B93_FUNGE_SPACE_H

#include "../../global.h"
#include "../../vector.h"
#include <stdint.h>

#define FUNGESPACETYPE int_fast64_t

struct _fungeSpace;
typedef struct _fungeSpace fungeSpace;

extern fungeSpace      * fungeSpaceCreate(void) __attribute__((warn_unused_result));
extern void              fungeSpaceFree(fungeSpace * me);
extern FUNGESPACETYPE    fungeSpaceGet (fungeSpace * me, const fungePosition * position) __attribute__((warn_unused_result));
extern void              fungeSpaceSet (fungeSpace * me, const fungePosition * position, FUNGESPACETYPE value);
extern fungePosition   * fungeSpaceWrap(fungeSpace * me, const fungePosition * position) __attribute__((warn_unused_result));
extern void              fungeSpaceLoad(fungeSpace * me, const char * filename);
#endif

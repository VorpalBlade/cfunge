#include "../../global.h"
#include "funge-space.h"

#include <string.h>
#include <stdbool.h>

struct _fungeSpace {
	FUNGESPACETYPE entries[80][25];
};

static inline bool fungeSpaceInRange(const fungePosition * position) {
	if ((position->x > 79) || (position->x < 0))
		return false;
	if ((position->y > 24) || (position->y < 0))
		return false;
	return true;
}

fungeSpace*
fungeSpaceCreate(void)
{
	fungeSpace * tmp = cf_malloc(sizeof(fungeSpace));
	memset(tmp, 0, sizeof(fungeSpace));
	return tmp;
}


void
fungeSpaceFree(fungeSpace * me) {
	cf_free(me);
}


FUNGESPACETYPE
fungeSpaceGet (fungeSpace * me, const fungePosition * position) {
	if (!fungeSpaceInRange(position))
		position = fungeSpaceWrap(me, position);
	// Thanks to Zaba for suggesting this code
	return me->entries[position->x][position->y] ? me->entries[position->x][position->y] : (FUNGESPACETYPE)' ';
}


void
fungeSpaceSet (fungeSpace * me, const fungePosition * position, FUNGESPACETYPE value) {
	if (!fungeSpaceInRange(position))
		position = fungeSpaceWrap(me, position);
	me->entries[position->x][position->y] = value;
}


fungePosition *
fungeSpaceWrap(fungeSpace * me, const fungePosition * position) {
	fungePosition *tmp = cf_malloc(sizeof(position));
	// Fix this for less than -80
	if (position->x < 0)
		tmp->x = 80 - position->x;
	else
		tmp->x = position->x % 80;

	if (position->y < 0)
		tmp->y = 25 - position->y;
	else
		tmp->y = position->y % 25;

	return tmp;
}

void
fungeSpaceLoad(fungeSpace * me, const char * filename) {

}

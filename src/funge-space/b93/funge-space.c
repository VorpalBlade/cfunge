#include "../../global.h"
#include "funge-space.h"

#include <string.h>
#include <stdio.h>

struct _fungeSpace {
	FUNGEDATATYPE entries[25][80];
};

static inline bool fungeSpaceInRange(const fungePosition * position)
{
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
	memset(tmp->entries, ' ', sizeof(fungeSpace));
	return tmp;
}


void
fungeSpaceFree(fungeSpace * me)
{
	cf_free(me);
}


FUNGEDATATYPE
fungeSpaceGet(fungeSpace * me, const fungePosition * position)
{
	if (!fungeSpaceInRange(position))
		position = fungeSpaceWrap(me, position);
	// Thanks to Zaba for suggesting this code
	return me->entries[position->y][position->x];
}


void
fungeSpaceSet(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position)
{
	if (!fungeSpaceInRange(position))
		position = fungeSpaceWrap(me, position);
	me->entries[position->y][position->x] = value;
}


fungePosition *
fungeSpaceWrap(__attribute__((unused)) fungeSpace * me, const fungePosition * position)
{
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
fungeSpaceWrapInPlace(__attribute__((unused)) fungeSpace * me, fungePosition * position)
{
	// Fix this for less than -80
	if (position->x < 0)
		position->x = 80 - position->x;
	else
		position->x = position->x % 80;

	if (position->y < 0)
		position->y = 25 - position->y;
	else
		position->y = position->y % 25;
}


bool
fungeSpaceLoad(fungeSpace * me, const char * filename)
{
	FILE * file;
	char * line;
	// Row in fungespace
	int    row = 0;

	file = fopen(filename, "r");
	if (file == NULL)
		return false;

	line = cf_malloc(81 * sizeof(char));

	while ((row < 25) && (fgets(line, 81, file) != NULL)) {
		for (int i = 0; i < 80; i++) {
			// TODO: CR and CRLF are also valid (bleh)
			if ((line[i] == '\0') || (line[i] == '\n')) break;
			if (line[i] == ' ') continue;
			me->entries[row][i] = (FUNGEDATATYPE)line[i];
		}
		row++;
	}

	return true;
}

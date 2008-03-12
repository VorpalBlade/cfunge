/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../global.h"
#include "../funge-space.h"
#include "../../../lib/libghthash/ght_hash_table.h"

#include <string.h>
#include <stdio.h>

#define FUNGESPACEINITIALSIZE 200000

struct _fungeSpace {
	// These two form a rectangle for the program size
	fungePosition topLeftCorner;
	fungePosition bottomRightCorner;
	// And this is the hash table.
	ght_hash_table_t *entries;
};

static inline bool fungeSpaceInRange(const fungeSpace * restrict me, const fungePosition * restrict position)
{
	if ((position->x > me->bottomRightCorner.x) || (position->x < me->topLeftCorner.x))
		return false;
	if ((position->y > me->bottomRightCorner.y) || (position->y < me->topLeftCorner.y))
		return false;
	return true;
}

fungeSpace*
fungeSpaceCreate(void)
{
	fungeSpace * tmp = cf_malloc(sizeof(fungeSpace));
	tmp->entries = ght_create(FUNGESPACEINITIALSIZE);
	tmp->topLeftCorner.x = 0;
	tmp->topLeftCorner.y = 0;
	tmp->bottomRightCorner.x = 0;
	tmp->bottomRightCorner.y = 0;
	return tmp;
}


void
fungeSpaceFree(fungeSpace * me)
{
	ght_finalize(me->entries);
	cf_free(me);
}


FUNGEDATATYPE
fungeSpaceGet(fungeSpace * me, const fungePosition * position)
{
	FUNGEDATATYPE *tmp;
	if (!fungeSpaceInRange(me, position))
		position = fungeSpaceWrap(me, position);

	tmp = ght_get(me->entries, sizeof(fungePosition), position);
	if (!tmp)
		return ' ';
	else
		return *tmp;
}


FUNGEDATATYPE
fungeSpaceGetOff(fungeSpace * me, const fungePosition * position, const fungePosition * offset)
{
	fungePosition tmp;
	FUNGEDATATYPE *result;

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;
	fungeSpaceWrapInPlace(me, &tmp);

	result = ght_get(me->entries, sizeof(fungePosition), &tmp);
	if (!result)
		return ' ';
	else
		return *result;
}



void
fungeSpaceSet(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position)
{
	if (value == ' ')
		ght_remove(me->entries, sizeof(fungePosition), position);
	else {
		FUNGEDATATYPE *tmp = cf_malloc_noptr(sizeof(FUNGEDATATYPE));
		*tmp = value;
		if (ght_insert(me->entries, tmp, sizeof(fungePosition), position) == -1) {
			FUNGEDATATYPE *oldval = ght_replace(me->entries, tmp, sizeof(fungePosition), position);
			cf_free(oldval);
		}
	}
	if (me->bottomRightCorner.y < position->y)
		me->bottomRightCorner.y = position->y;
	if (me->bottomRightCorner.x < position->x)
		me->bottomRightCorner.x = position->x;
	if (me->topLeftCorner.y > position->y)
		me->topLeftCorner.y = position->y;
	if (me->topLeftCorner.x > position->x)
		me->topLeftCorner.x = position->x;
}

void
fungeSpaceSetOff(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position, const fungePosition * offset)
{
	fungePosition tmp;
	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	if (value == ' ')
		ght_remove(me->entries, sizeof(fungePosition), &tmp);
	else {
		FUNGEDATATYPE *data = cf_malloc_noptr(sizeof(FUNGEDATATYPE));
		*data = value;
		if (ght_insert(me->entries, data, sizeof(fungePosition), &tmp) == -1) {
			FUNGEDATATYPE *oldval = ght_replace(me->entries, data, sizeof(fungePosition), &tmp);
			cf_free(oldval);
		}
	}

	if (me->bottomRightCorner.y < position->y)
		me->bottomRightCorner.y = position->y;
	if (me->bottomRightCorner.x < position->x)
		me->bottomRightCorner.x = position->x;
	if (me->topLeftCorner.y > position->y)
		me->topLeftCorner.y = position->y;
	if (me->topLeftCorner.x > position->x)
		me->topLeftCorner.x = position->x;
}

#define ABS(i) ((i > 0) ? i : i)

fungePosition *
fungeSpaceWrap(fungeSpace * me, const fungePosition * position)
{
	fungePosition *tmp = cf_malloc(sizeof(position));

	if (position->x < me->topLeftCorner.x)
		tmp->x = me->bottomRightCorner.x - ABS(position->x);
	else
		tmp->x = position->x % me->bottomRightCorner.x;

	if (position->y < me->topLeftCorner.y)
		tmp->y = me->bottomRightCorner.y - ABS(position->y);
	else
		tmp->y = position->y % me->bottomRightCorner.y;

	return tmp;
}

void
fungeSpaceWrapInPlace(fungeSpace * me, fungePosition * position)
{
	if (position->x < me->topLeftCorner.x)
		position->x = me->bottomRightCorner.x - ABS(position->x);
	else
		position->x = position->x % me->bottomRightCorner.x;

	if (position->y < me->topLeftCorner.y)
		position->y = me->bottomRightCorner.y - ABS(position->y);
	else
		position->y = position->y % me->bottomRightCorner.y;
}


void
fungeSpaceWrapInPlaceWithDelta(fungeSpace * me, fungePosition * restrict position, const fungeVector * restrict delta)
{
// 	if (VectorIsCardinal(delta))
// 		fungeSpaceWrapInPlace(me, position);
// 	else {
		if (!fungeSpaceInRange(me, position)) {
			// SIGH!
			do {
				position->x -= delta->x;
				position->y -= delta->y;
			} while (fungeSpaceInRange(me, position));
				position->x += delta->x;
				position->y += delta->y;
		}
// 	}
}


bool
fungeSpaceLoad(fungeSpace * me, const char * filename)
{
	FILE * file;
	char * line = NULL;
	size_t linelen = 0;
	// Row in fungespace
	int    y = 0;
	int    x = 0;

	file = fopen(filename, "r");
	if (!file)
		return false;

	while (cf_getline(&line, &linelen, file) != -1) {
		for (size_t i = 0; i < (strlen(line) + 1); i++) {
			if (line[i] == '\0') {
				break;
			} else if (line[i] == '\r' && line[i+1] == '\n') {
				if (me->bottomRightCorner.x < x)
					me->bottomRightCorner.x = x;
				x = 0;
				y++;
				i++;
				continue;
			} else if (line[i] == '\n' || line[i] == '\r') {
				if (me->bottomRightCorner.x < x)
					me->bottomRightCorner.x = x;
				x = 0;
				y++;
				continue;
			}
			fungeSpaceSet(me, (FUNGEDATATYPE)line[i], & (fungePosition) { .x = x, .y = y });
			x++;
		}
	}

	if (me->bottomRightCorner.y < y)
		me->bottomRightCorner.y = y;

	return true;
}

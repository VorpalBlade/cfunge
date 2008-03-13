/*
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at the proxy's option) any later version. Arvid Norlander is a
 * proxy who can decide which future versions of the GNU General Public
 * License can be used.
 *
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
// We allocate *cells* this many at a time.
#define FUNGESPACEALLOCCHUNK 512

struct _fungeSpace {
	// These two form a rectangle for the program size
	fungePosition topLeftCorner;
	fungePosition bottomRightCorner;
	// And this is the hash table.
	ght_hash_table_t *entries;
	// Array we allocate for values as we need them, we do FUNGESPACEALLOCCHUNK at a time here.
	// We will replace it when we need to. Size MUST be FUNGESPACEALLOCCHUNK
	FUNGEDATATYPE *allocarray;
	size_t         allocarrayCurrent;
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
	//ght_set_heuristics(tmp->entries, GHT_HEURISTICS_TRANSPOSE);
	tmp->allocarray = cf_malloc_noptr(FUNGESPACEALLOCCHUNK * sizeof(FUNGEDATATYPE));
	tmp->allocarrayCurrent = 0;

	tmp->topLeftCorner.x = 0;
	tmp->topLeftCorner.y = 0;
	tmp->bottomRightCorner.x = 0;
	tmp->bottomRightCorner.y = 0;
	return tmp;
}


void
fungeSpaceFree(fungeSpace * me)
{
	if (!me)
		return;
	ght_finalize(me->entries);
	cf_free(me);
}

void
fungeSpaceGetBoundRect(const fungeSpace * restrict me, fungeRect * restrict rect) {
	rect->x = me->topLeftCorner.x;
	rect->y = me->topLeftCorner.y;
	rect->w = me->bottomRightCorner.x - me->topLeftCorner.x - 1;
	rect->h = me->bottomRightCorner.y - me->topLeftCorner.y - 1;
}


FUNGEDATATYPE
fungeSpaceGet(const fungeSpace * restrict me, const fungePosition * restrict position)
{
	FUNGEDATATYPE *tmp;

	tmp = ght_get(me->entries, sizeof(fungePosition), position);
	if (!tmp)
		return ' ';
	else
		return *tmp;
}


FUNGEDATATYPE
fungeSpaceGetOff(const fungeSpace * restrict me, const fungePosition * restrict position, const fungePosition * restrict offset)
{
	fungePosition tmp;
	FUNGEDATATYPE *result;

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	result = ght_get(me->entries, sizeof(fungePosition), &tmp);
	if (!result)
		return ' ';
	else
		return *result;
}

static FUNGEDATATYPE*
fungeSpaceInternalAlloc(fungeSpace * restrict me, FUNGEDATATYPE value) {
	if (me->allocarrayCurrent > (FUNGESPACEALLOCCHUNK - 2)) {
		// Allocate new array
		me->allocarray = cf_malloc_noptr(FUNGESPACEALLOCCHUNK * sizeof(FUNGEDATATYPE));
		me->allocarrayCurrent = 0;
	} else {
		// Allocate from array
		me->allocarrayCurrent++;
	}
	me->allocarray[me->allocarrayCurrent] = value;

	return &me->allocarray[me->allocarrayCurrent];
}


void
fungeSpaceSet(fungeSpace * restrict me, FUNGEDATATYPE value, const fungePosition * restrict position)
{
	if (value == ' ')
		ght_remove(me->entries, sizeof(fungePosition), position);
	else {
		// TODO: Reuse cell?
		FUNGEDATATYPE *tmp = fungeSpaceInternalAlloc(me, value);
		if (ght_insert(me->entries, tmp, sizeof(fungePosition), position) == -1) {
			ght_replace(me->entries, tmp, sizeof(fungePosition), position);
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
fungeSpaceSetOff(fungeSpace * restrict me, FUNGEDATATYPE value, const fungePosition * restrict position, const fungePosition * restrict offset)
{
	fungePosition tmp;
	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	fungeSpaceSet(me, value, &tmp);
}

#define ABS(i) ((i > 0) ? i : i)

#if 0
static void
fungeSpaceWrapNoDelta(fungeSpace * me, fungePosition * position)
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
#endif

void
fungeSpaceWrap(const fungeSpace * restrict me, fungePosition * restrict position, const fungeVector * restrict delta)
{
// 	if (VectorIsCardinal(delta))
// 		fungeSpaceWrapNoDelta(me, position);
// 	else {
		if (!fungeSpaceInRange(me, position)) {
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

	ght_set_rehash(me->entries, true);
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
	ght_set_rehash(me->entries, false);
	fclose(file);
	return true;
}

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

#include <string.h>
#include <stdio.h>

#define FUNGESPACEWIDTH 80
#define FUNGESPACEHEIGHT 25

struct _fungeSpace {
	FUNGEDATATYPE entries[FUNGESPACEHEIGHT][FUNGESPACEWIDTH];
};

// Forward decls.
static fungePosition *fungeSpaceWrapNotInPlace(const fungePosition * position);
static void fungeSpaceWrapNoDelta(fungePosition * position);

static inline bool fungeSpaceInRange(const fungePosition * position)
{
	if ((position->x > (FUNGESPACEWIDTH - 1)) || (position->x < 0))
		return false;
	if ((position->y > (FUNGESPACEHEIGHT - 1)) || (position->y < 0))
		return false;
	return true;
}

fungeSpace*
fungeSpaceCreate(void)
{
	fungeSpace * tmp = cf_malloc(sizeof(fungeSpace));
	for (int y = 0; y < FUNGESPACEHEIGHT; y++)
		for (int x = 0; x < FUNGESPACEWIDTH; x++)
			tmp->entries[y][x]=(FUNGEDATATYPE)' ';
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
		position = fungeSpaceWrapNotInPlace(position);
	return me->entries[position->y][position->x];
}


FUNGEDATATYPE
fungeSpaceGetOff(fungeSpace * me, const fungePosition * position, const fungePosition * offset)
{
	fungePosition tmp;
	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;
	fungeSpaceWrapNoDelta(&tmp);
	return me->entries[tmp.y][tmp.x];
}



void
fungeSpaceSet(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position)
{
	if (!fungeSpaceInRange(position))
		position = fungeSpaceWrapNotInPlace(position);
	me->entries[position->y][position->x] = value;
}

void
fungeSpaceSetOff(fungeSpace * me, FUNGEDATATYPE value, const fungePosition * position, const fungePosition * offset)
{
	fungePosition tmp;
	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;
	fungeSpaceWrapNoDelta(&tmp);
	me->entries[tmp.y][tmp.x] = value;
}


fungePosition *
fungeSpaceWrapNotInPlace(const fungePosition * position)
{
	fungePosition *tmp = cf_malloc(sizeof(position));
	// FIXME: Fix this for less than -80
	if (position->x < 0)
		tmp->x = FUNGESPACEWIDTH + position->x;
	else
		tmp->x = position->x % FUNGESPACEWIDTH;

	if (position->y < 0)
		tmp->y = FUNGESPACEHEIGHT + position->y;
	else
		tmp->y = position->y % FUNGESPACEHEIGHT;

	return tmp;
}

static void
fungeSpaceWrapNoDelta(fungePosition * position)
{
	// FIXME: Fix this for less than -80
	if (position->x < 0)
		position->x = FUNGESPACEWIDTH + position->x;
	else
		position->x = position->x % FUNGESPACEWIDTH;

	if (position->y < 0)
		position->y = FUNGESPACEHEIGHT + position->y;
	else
		position->y = position->y % FUNGESPACEHEIGHT;
}


void
fungeSpaceWrap(fungeSpace * me, fungePosition * restrict position, const fungeVector * restrict delta)
{
	if (VectorIsCardinal(delta))
		fungeSpaceWrapNoDelta(position);
	else {
		if (!fungeSpaceInRange(position)) {
			// SIGH!
			do {
				position->x -= delta->x;
				position->y -= delta->y;
			} while (fungeSpaceInRange(position));
				position->x += delta->x;
				position->y += delta->y;
		}
	}
}


bool
fungeSpaceLoad(fungeSpace * me, const char * filename)
{
	FILE * file;
	char * line;
	// Row in fungespace
	int    y = 0;
	int    x = 0;

	file = fopen(filename, "r");
	if (!file)
		return false;

	line = cf_malloc((FUNGESPACEWIDTH + 1) * sizeof(char));
	if (!line)
		return false;

	while ((y < FUNGESPACEHEIGHT) && (fgets(line, (FUNGESPACEWIDTH + 1), file) != NULL)) {
		for (size_t i = 0; i < (strlen(line) + 1); i++) {
			if (line[i] == '\0') {
				break;
			} else if (line[i] == '\r' && line[i+1] == '\n') {
				x = 0;
				y++;
				i++;
				continue;
			} else if (line[i] == '\n' || line[i] == '\r') {
				x = 0;
				y++;
				continue;
			}
			if (i < FUNGESPACEWIDTH)
				me->entries[y][x] = (FUNGEDATATYPE)line[i];
			x++;
		}
	}

	return true;
}

/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define FUNGESPACEINITIALSIZE 150000
// We allocate this many *cells* at a time.
#define FUNGESPACEALLOCCHUNK 1024

typedef struct _fungeSpace {
	// These two form a rectangle for the program size
	fungePosition topLeftCorner;
	fungePosition bottomRightCorner;
	// And this is the hash table.
	ght_hash_table_t *entries;
	// Array we allocate for values as we need them, we do FUNGESPACEALLOCCHUNK at a time here.
	// We will replace it when we need to. Size MUST be FUNGESPACEALLOCCHUNK
	FUNGEDATATYPE *allocarray;
	size_t         allocarrayCurrent;
} fungeSpace;

// Funge-space storage.
static fungeSpace *fspace = NULL;

/**
 * Check if position is in range.
 */
static inline bool fungeSpaceInRange(const fungePosition * restrict position) __attribute__((pure));

static inline bool fungeSpaceInRange(const fungePosition * restrict position)
{
	if ((position->x > fspace->bottomRightCorner.x) || (position->x < fspace->topLeftCorner.x))
		return false;
	if ((position->y > fspace->bottomRightCorner.y) || (position->y < fspace->topLeftCorner.y))
		return false;
	return true;
}

bool
fungeSpaceCreate(void)
{
	fspace = cf_malloc(sizeof(fungeSpace));
	if (!fspace)
		return false;
	fspace->entries = ght_create(FUNGESPACEINITIALSIZE);
	if (!fspace->entries)
		return false;
	ght_set_hash(fspace->entries, &ght_crc_hash);
	// Unable to determine if this helps or not.
	//ght_set_heuristics(fspace->entries, GHT_HEURISTICS_TRANSPOSE);
	ght_set_rehash(fspace->entries, true);
	fspace->allocarray = cf_malloc_noptr(FUNGESPACEALLOCCHUNK * sizeof(FUNGEDATATYPE));
	fspace->allocarrayCurrent = 0;

	fspace->topLeftCorner.x = 0;
	fspace->topLeftCorner.y = 0;
	fspace->bottomRightCorner.x = 0;
	fspace->bottomRightCorner.y = 0;
	return true;
}


void
fungeSpaceFree(void)
{
	if (!fspace)
		return;
	ght_finalize(fspace->entries);
	// Just the last block, but still.
	cf_free(fspace->allocarray);
	cf_free(fspace);
}

void
fungeSpaceGetBoundRect(fungeRect * restrict rect) {
	rect->x = fspace->topLeftCorner.x;
	rect->y = fspace->topLeftCorner.y;
	rect->w = fspace->bottomRightCorner.x - fspace->topLeftCorner.x;
	rect->h = fspace->bottomRightCorner.y - fspace->topLeftCorner.y;
}


FUNGEDATATYPE
fungeSpaceGet(const fungePosition * restrict position)
{
	FUNGEDATATYPE *tmp;

	assert(position != NULL);

	tmp = ght_get(fspace->entries, sizeof(fungePosition), position);
	if (!tmp)
		return ' ';
	else
		return *tmp;
}


FUNGEDATATYPE
fungeSpaceGetOff(const fungePosition * restrict position, const fungePosition * restrict offset)
{
	fungePosition tmp;
	FUNGEDATATYPE *result;

	assert(position != NULL);
	assert(offset != NULL);

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	result = ght_get(fspace->entries, sizeof(fungePosition), &tmp);
	if (!result)
		return ' ';
	else
		return *result;
}

/**
 * Allocate space for a cell.
 * Allocates in chunks of FUNGESPACEALLOCCHUNK.
 */
static inline FUNGEDATATYPE*
fungeSpaceInternalAlloc(FUNGEDATATYPE value)
{
	if (fspace->allocarrayCurrent > (FUNGESPACEALLOCCHUNK - 2)) {
		// Allocate new array
		fspace->allocarray = cf_malloc_noptr(FUNGESPACEALLOCCHUNK * sizeof(FUNGEDATATYPE));
		if (!fspace->allocarray) {
			perror("Out of memory, couldn't allocate cell(s) for funge space");
			abort();
		}
		fspace->allocarrayCurrent = 0;
	} else {
		// Allocate from array
		fspace->allocarrayCurrent++;
	}
	fspace->allocarray[fspace->allocarrayCurrent] = value;

	return &fspace->allocarray[fspace->allocarrayCurrent];
}


static inline void
fungeSpaceSetNoBoundUpdate(FUNGEDATATYPE value, const fungePosition * restrict position)
{
	assert(position != NULL);
	if (value == ' ') {
		ght_remove(fspace->entries, sizeof(fungePosition), position);
	} else {
		// Reuse cell if it exists
		FUNGEDATATYPE *tmp;
		if ((tmp = ght_get(fspace->entries, sizeof(fungePosition), position)) != NULL) {
			*tmp = value;
		} else {
			tmp = fungeSpaceInternalAlloc(value);
			if (ght_insert(fspace->entries, tmp, sizeof(fungePosition), position) == -1) {
				ght_replace(fspace->entries, tmp, sizeof(fungePosition), position);
			}
		}
	}
}

void
fungeSpaceSet(FUNGEDATATYPE value, const fungePosition * restrict position)
{
	assert(position != NULL);
	fungeSpaceSetNoBoundUpdate(value, position);
	if (fspace->bottomRightCorner.y < position->y)
		fspace->bottomRightCorner.y = position->y;
	if (fspace->bottomRightCorner.x < position->x)
		fspace->bottomRightCorner.x = position->x;
	if (fspace->topLeftCorner.y > position->y)
		fspace->topLeftCorner.y = position->y;
	if (fspace->topLeftCorner.x > position->x)
		fspace->topLeftCorner.x = position->x;
}

void
fungeSpaceSetOff(FUNGEDATATYPE value, const fungePosition * restrict position, const fungePosition * restrict offset)
{
	fungePosition tmp;

	assert(position != NULL);
	assert(offset != NULL);

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	fungeSpaceSet(value, &tmp);
}

#if 0
static inline void
fungeSpaceWrapNoDelta(fungePosition * restrict position)
{
	if (position->x < fspace->topLeftCorner.x)
		position->x = fspace->bottomRightCorner.x - ABS(position->x);
	else
		position->x = position->x % fspace->bottomRightCorner.x;

	if (position->y < fspace->topLeftCorner.y)
		position->y = fspace->bottomRightCorner.y - ABS(position->y);
	else
		position->y = position->y % fspace->bottomRightCorner.y;
}
#endif

void
fungeSpaceWrap(fungePosition * restrict position, const fungeVector * restrict delta)
{
#if 0
	if (VectorIsCardinal(delta))
		fungeSpaceWrapNoDelta(position);
	else {
#endif
		if (!fungeSpaceInRange(position)) {
			do {
				position->x -= delta->x;
				position->y -= delta->y;
			} while (fungeSpaceInRange(position));
				position->x += delta->x;
				position->y += delta->y;
		}
#if 0
	}
#endif
}


bool
fungeSpaceLoad(const char * restrict filename)
{
	FILE * file;
	char * line = NULL;
	size_t linelen = 0;
	// Row in fungespace
	int    y = 0;
	int    x = 0;
	assert(filename != NULL);

	file = fopen(filename, "r");
	if (!file)
		return false;

	while (cf_getline(&line, &linelen, file) != -1) {
		for (size_t i = 0; i < (linelen + 1); i++) {
			if (line[i] == '\0') {
				if (fspace->bottomRightCorner.x < x)
					fspace->bottomRightCorner.x = x;
				break;
			} else if (line[i] == '\r' && line[i+1] == '\n') {
				if (fspace->bottomRightCorner.x < x)
					fspace->bottomRightCorner.x = x;
				x = 0;
				y++;
				i++;
				continue;
			} else if (line[i] == '\n' || line[i] == '\r') {
				if (fspace->bottomRightCorner.x < x)
					fspace->bottomRightCorner.x = x;
				x = 0;
				y++;
				continue;
			}
			fungeSpaceSetNoBoundUpdate((FUNGEDATATYPE)line[i], & (fungePosition) { .x = x, .y = y });
			x++;
		}
	}
	if (fspace->bottomRightCorner.y < y)
		fspace->bottomRightCorner.y = y;
	fclose(file);
	if (line != NULL)
		cf_free(line);
	return true;
}

#ifndef NDEBUG
/*************
 * Debugging *
 *************/


// For use with call in gdb
void FungeSpaceDump(void) __attribute__((unused));

void FungeSpaceDump(void)
{
	if (!fspace)
		return;
	fprintf(stderr, "Fungespace follows:\n");
	for (FUNGEVECTORTYPE y = 0; y < fspace->bottomRightCorner.y; y++) {
		for (FUNGEVECTORTYPE x = 0; x < fspace->bottomRightCorner.x; x++)
			fprintf(stderr, "%c", (char)fungeSpaceGet(& (fungePosition) { .x = x, .y = y }));
		fprintf(stderr, "\n");
	}
	fputs("\n", stderr);
}

#endif


bool
fungeSpaceLoadAtOffset(const char          * restrict filename,
                       const fungePosition * restrict offset,
                       fungeVector         * restrict size,
                       bool binary)
{
	FILE * file;
	char * line = NULL;
	size_t linelen = 0;
	// Row in fungespace
	int    y = 0;
	int    x = 0;
	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	file = fopen(filename, "r");
	if (!file)
		return false;

	size->x = 0;
	size->y = 0;

	while (cf_getline(&line, &linelen, file) != -1) {
		for (size_t i = 0; i < (linelen + 1); i++) {
			if (line[i] == '\0') {
				break;
			} else if (!binary && (line[i] == '\r') && (line[i+1] == '\n')) {
				if (x > size->x) size->x = x;
				x = 0;
				y++;
				i++;
				continue;
			} else if (!binary && (line[i] == '\n' || line[i] == '\r')) {
				if (x > size->x) size->x = x;
				x = 0;
				y++;
				continue;
			}
			if (line[i] != ' ')
				fungeSpaceSetOff((FUNGEDATATYPE)line[i], & (fungePosition) { .x = x, .y = y }, offset);
			x++;
		}
	}
	if (x > size->x) size->x = x;
	if (y > size->y) size->y = y;
	fclose(file);
	if (line != NULL)
		cf_free(line);
	return true;
}

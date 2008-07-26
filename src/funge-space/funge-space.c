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

#include "../global.h"
#include "funge-space.h"
#include "../../lib/libghthash/ght_hash_table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO != -1)
#  include <fcntl.h>
#endif

#define FUNGESPACEINITIALSIZE 150000

typedef struct _fungeSpace {
	/// These two form a rectangle for the program size
	fungePosition     topLeftCorner;
	fungePosition     bottomRightCorner;
	/// And this is the hash table.
	ght_hash_table_t *entries;
} fungeSpace;

/// Funge-space storage.
static fungeSpace *fspace = NULL;

/**
 * Check if position is in range.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline bool FungeSpaceInRange(const fungePosition * restrict position)
{
	if ((position->x > fspace->bottomRightCorner.x) || (position->x < fspace->topLeftCorner.x))
		return false;
	if ((position->y > fspace->bottomRightCorner.y) || (position->y < fspace->topLeftCorner.y))
		return false;
	return true;
}

FUNGE_ATTR_FAST bool
FungeSpaceCreate(void)
{
	fspace = (fungeSpace*)cf_malloc(sizeof(fungeSpace));
	if (!fspace)
		return false;
	fspace->entries = ght_create(FUNGESPACEINITIALSIZE);
	if (!fspace->entries)
		return false;
	ght_set_rehash(fspace->entries, true);

	fspace->topLeftCorner.x = 0;
	fspace->topLeftCorner.y = 0;
	fspace->bottomRightCorner.x = 0;
	fspace->bottomRightCorner.y = 0;
	return true;
}


FUNGE_ATTR_FAST void
FungeSpaceFree(void)
{
	if (!fspace)
		return;
	ght_finalize(fspace->entries);
	cf_free(fspace);
}

FUNGE_ATTR_FAST void
FungeSpaceGetBoundRect(fungeRect * restrict rect)
{
	rect->x = fspace->topLeftCorner.x;
	rect->y = fspace->topLeftCorner.y;
	rect->w = fspace->bottomRightCorner.x - fspace->topLeftCorner.x;
	rect->h = fspace->bottomRightCorner.y - fspace->topLeftCorner.y;
}


FUNGE_ATTR_FAST FUNGEDATATYPE
FungeSpaceGet(const fungePosition * restrict position)
{
	FUNGEDATATYPE *tmp;

	assert(position != NULL);

	tmp = (FUNGEDATATYPE*)ght_get(fspace->entries, position);
	if (!tmp)
		return (FUNGEDATATYPE)' ';
	else
		return *tmp;
}


FUNGE_ATTR_FAST FUNGEDATATYPE
FungeSpaceGetOff(const fungePosition * restrict position, const fungePosition * restrict offset)
{
	fungePosition tmp;
	FUNGEDATATYPE *result;

	assert(position != NULL);
	assert(offset != NULL);

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	result = (FUNGEDATATYPE*)ght_get(fspace->entries, &tmp);
	if (!result)
		return (FUNGEDATATYPE)' ';
	else
		return *result;
}

FUNGE_ATTR_FAST static inline void
FungeSpaceSetNoBoundUpdate(FUNGEDATATYPE value, const fungePosition * restrict position)
{
	assert(position != NULL);
	if (value == ' ') {
		ght_remove(fspace->entries, position);
	} else {
		// Reuse cell if it exists
		FUNGEDATATYPE *tmp;
		if ((tmp = (FUNGEDATATYPE*)ght_get(fspace->entries, position)) != NULL) {
			*tmp = value;
		} else {
			if (ght_insert(fspace->entries, value, position) == -1) {
				ght_replace(fspace->entries, value, position);
			}
		}
	}
}

FUNGE_ATTR_FAST void
FungeSpaceSet(FUNGEDATATYPE value, const fungePosition * restrict position)
{
	assert(position != NULL);
	FungeSpaceSetNoBoundUpdate(value, position);
	if (fspace->bottomRightCorner.y < position->y)
		fspace->bottomRightCorner.y = position->y;
	if (fspace->bottomRightCorner.x < position->x)
		fspace->bottomRightCorner.x = position->x;
	if (fspace->topLeftCorner.y > position->y)
		fspace->topLeftCorner.y = position->y;
	if (fspace->topLeftCorner.x > position->x)
		fspace->topLeftCorner.x = position->x;
}

FUNGE_ATTR_FAST void
FungeSpaceSetOff(FUNGEDATATYPE value, const fungePosition * restrict position, const fungePosition * restrict offset)
{
	assert(position != NULL);
	assert(offset != NULL);

	FungeSpaceSet(value, VectorCreateRef(position->x + offset->x, position->y + offset->y));
}

FUNGE_ATTR_FAST void
FungeSpaceWrap(fungePosition * restrict position, const fungeVector * restrict delta)
{
	// Quick and dirty if cardinal.
	if (VectorIsCardinal(delta)) {
		if (position->x < fspace->topLeftCorner.x)
			position->x = fspace->bottomRightCorner.x;
		else if (position->x >= fspace->bottomRightCorner.x)
			position->x = fspace->topLeftCorner.x;

		if (position->y < fspace->topLeftCorner.y)
			position->y = fspace->bottomRightCorner.y;
		else if (position->y >= fspace->bottomRightCorner.y)
			position->y = fspace->topLeftCorner.y;
	} else {
		if (!FungeSpaceInRange(position)) {
			do {
				position->x -= delta->x;
				position->y -= delta->y;
			} while (FungeSpaceInRange(position));
			position->x += delta->x;
			position->y += delta->y;
		}
	}
}

#ifndef NDEBUG
/*************
 * Debugging *
 *************/


// For use with call in gdb
void FungeSpaceDump(void) FUNGE_ATTR_UNUSED;

void FungeSpaceDump(void)
{
	if (!fspace)
		return;
	fprintf(stderr, "Fungespace follows:\n");
	for (FUNGEVECTORTYPE y = 0; y <= fspace->bottomRightCorner.y; y++) {
		for (FUNGEVECTORTYPE x = 0; x <= fspace->bottomRightCorner.x; x++)
			fprintf(stderr, "%c", (char)FungeSpaceGet(VectorCreateRef(x, y)));
		fprintf(stderr, "\n");
	}
	fputs("\n", stderr);
}

#endif

FUNGE_ATTR_ALWAYS_INLINE FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline FILE * FungeSpaceOpenFile(const char * restrict filename)
{
	FILE * file;

	assert(filename != NULL);

	file = fopen(filename, "rb");
	if (!file) {
		return NULL;
	} else {
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
		// Microoptimising! Remove this if it bothers you.
		int fd = fileno(file);
		posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
		posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
		return file;
	}
}

FUNGE_ATTR_FAST bool
FungeSpaceLoad(const char * restrict filename)
{
	FILE * file;
	char buf[1024];
	size_t linelen = 0;
	bool lastwascr = false;
	bool noendingnewline = true;
	// Row in fungespace
	FUNGEVECTORTYPE y = 0;
	FUNGEVECTORTYPE x = 0;
	assert(filename != NULL);

	file = FungeSpaceOpenFile(filename);
	if (!file)
		return false;

	while ((linelen = fread(&buf, sizeof(char), sizeof(buf), file)) != 0) {
		for (size_t i = 0; i < linelen; i++) {
			switch (buf[i]) {
				// Ignore Form Feed.
				case '\f':
					break;
				case '\r':
					lastwascr = true;
					break;
				case '\n':
					if (fspace->bottomRightCorner.x < x)
						fspace->bottomRightCorner.x = x;
					x = 0;
					y++;
					lastwascr = false;
					noendingnewline = false;
					break;
				default:
					if (lastwascr) {
						if (fspace->bottomRightCorner.x < x)
							fspace->bottomRightCorner.x = x;
						lastwascr = false;
						x = 0;
						y++;
					}
					FungeSpaceSetNoBoundUpdate((FUNGEDATATYPE)buf[i], VectorCreateRef(x, y));
					x++;
					noendingnewline = true;
					break;
			}
		}
	}
	if (fspace->bottomRightCorner.x < x)
		fspace->bottomRightCorner.x = x;
	if (lastwascr) {
		noendingnewline = false;
		y++;
	}
	if (noendingnewline) y++;
	if (fspace->bottomRightCorner.y < y)
		fspace->bottomRightCorner.y = y;
	fclose(file);
	return true;
}

#ifdef FUNGE_EXTERNAL_LIBRARY
FUNGE_ATTR_FAST void
FungeSpaceLoadString(const char * restrict program)
{
	bool lastwascr = false;
	bool noendingnewline = true;
	// Row in fungespace
	FUNGEVECTORTYPE y = 0;
	FUNGEVECTORTYPE x = 0;
	size_t linelen = strlen(program);

	for (size_t i = 0; i < linelen; i++) {
		switch (program[i]) {
			// Ignore Form Feed.
			case '\f':
				break;
			case '\r':
				lastwascr = true;
				break;
			case '\n':
				if (fspace->bottomRightCorner.x < x)
					fspace->bottomRightCorner.x = x;
				x = 0;
				y++;
				lastwascr = false;
				noendingnewline = false;
				break;
			default:
				if (lastwascr) {
					if (fspace->bottomRightCorner.x < x)
						fspace->bottomRightCorner.x = x;
					lastwascr = false;
					x = 0;
					y++;
				}
				FungeSpaceSetNoBoundUpdate((FUNGEDATATYPE)program[i], VectorCreateRef(x, y));
				x++;
				noendingnewline = true;
				break;
		}
	}
	if (fspace->bottomRightCorner.x < x)
		fspace->bottomRightCorner.x = x;
	if (lastwascr) {
		noendingnewline = false;
		y++;
	}
	if (noendingnewline) y++;
	if (fspace->bottomRightCorner.y < y)
		fspace->bottomRightCorner.y = y;
}
#endif

FUNGE_ATTR_FAST bool
FungeSpaceLoadAtOffset(const char          * restrict filename,
                       const fungePosition * restrict offset,
                       fungeVector         * restrict size,
                       bool binary)
{
	FILE * file;
	char buf[1024];
	bool lastwascr = false;
	size_t linelen = 0;

	FUNGEVECTORTYPE y = 0;
	FUNGEVECTORTYPE x = 0;
	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	file = FungeSpaceOpenFile(filename);
	if (!file)
		return false;

	size->x = 0;
	size->y = 0;

	while ((linelen = fread(&buf, sizeof(char), sizeof(buf), file)) != 0) {
		for (size_t i = 0; i < linelen; i++) {
			if (binary) {
				if (buf[i] != ' ')
					FungeSpaceSetOff((FUNGEDATATYPE)buf[i], VectorCreateRef(x, y), offset);
				x++;
			} else {
				switch (buf[i]) {
					// Don't ignore Form Feed here...
					case '\r':
						lastwascr = true;
						break;
					case '\n':
						if (x > size->x) size->x = x;
						x = 0;
						y++;
						lastwascr = false;
						break;
					default:
						if (lastwascr) {
							lastwascr = false;
							if (x > size->x) size->x = x;
							x = 0;
							y++;
						}
						if (buf[i] != ' ')
							FungeSpaceSetOff((FUNGEDATATYPE)buf[i], VectorCreateRef(x, y), offset);
						x++;
						break;
				}
			}
		}
	}
	if (x > size->x) size->x = x;
	if (y > size->y) size->y = y;
	fclose(file);
	return true;
}

FUNGE_ATTR_FAST bool
FungeSpaceSaveToFile(const char          * restrict filename,
                     const fungePosition * restrict offset,
                     const fungeVector   * restrict size,
                     bool textfile)
{
	FILE * file;

	FUNGEVECTORTYPE maxy = offset->y + size->y;
	FUNGEVECTORTYPE maxx = offset->x + size->x;

	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	file = fopen(filename, "wb");
	if (!file)
		return false;

	if (!textfile) {
		FUNGEDATATYPE value;
		// Microoptimising! Remove this if it bothers you.
		// However it also makes it possible to error out early.
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
		if (posix_fallocate(fileno(file), 0, size->y * size->x) != 0) {
			fclose(file);
			return false;
		}
#endif
		cf_flockfile(file);
		for (FUNGEVECTORTYPE y = offset->y; y < maxy; y++) {
			for (FUNGEVECTORTYPE x = offset->x; x < maxx; x++) {
				value = FungeSpaceGet(VectorCreateRef(x, y));
				cf_putc_unlocked(value, file);
			}
			cf_putc_unlocked('\n', file);
		}
		cf_funlockfile(file);
	// Text mode...
	} else {
		size_t index = 0;
		// Extra size->y for adding a lot of \n...
		FUNGEDATATYPE * towrite = cf_malloc((size->x * size->y + size->y) * sizeof(FUNGEDATATYPE));
		if (!towrite) {
			fclose(file);
			return false;
		}
		// Construct each line.
		for (FUNGEVECTORTYPE y = offset->y; y < maxy; y++) {
			ssize_t lastspace = size->x;
			FUNGEDATATYPE * string = cf_malloc(size->x * sizeof(FUNGEDATATYPE));
			if (!string) {
				fclose(file);
				return false;
			}
			for (FUNGEVECTORTYPE x = offset->x; x < maxx; x++) {
				string[x-offset->x] = FungeSpaceGet(VectorCreateRef(x, y));
			}

			do {
				lastspace--;
			} while ((lastspace >= 0) && (string[lastspace] == ' '));

			if (lastspace > 0) {
				for (ssize_t i = 0; i <= lastspace; i++) {
					towrite[index] = string[i];
					index++;
				}
			}
			cf_free(string);
			towrite[index]=(FUNGEDATATYPE)'\n';
			index++;
		}
		// Remove trailing newlines.
		{
			ssize_t lastnewline = index;
			do {
				lastnewline--;
			} while ((lastnewline >= 0) && (towrite[lastnewline] == '\n'));

			cf_flockfile(file);
			if (lastnewline > 0) {
				// Why the cast? To allow GCC to optimise better, by being able to
				// check if the loop is infinite or not.
				for (size_t i = 0; i <= (size_t)lastnewline; i++) {
					cf_putc_unlocked(towrite[i], file);
				}
			}
			cf_funlockfile(file);
		}
		cf_free(towrite);
	}
	fclose(file);
	return true;
}

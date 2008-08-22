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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#if !defined(_POSIX_MAPPED_FILES) || (_POSIX_MAPPED_FILES < 1)
#  error "cfunge needs a working mmap(), which this system claims it doesn't have."
#endif


#define FUNGESPACEINITIALSIZE 150000

typedef struct fungeSpace {
	/// These two form a rectangle for the program size
	fungePosition     topLeftCorner;
	fungePosition     bottomRightCorner;
	/// And this is the hash table.
	ght_hash_table_t *entries;
} fungeSpace;

/// Funge-space storage.
static fungeSpace fspace = {
	.topLeftCorner = {0, 0},
	.bottomRightCorner = {0, 0},
	.entries = NULL
};

/**
 * Check if position is in range.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline bool FungeSpaceInRange(const fungePosition * restrict position)
{
	if ((position->x > fspace.bottomRightCorner.x) || (position->x < fspace.topLeftCorner.x))
		return false;
	if ((position->y > fspace.bottomRightCorner.y) || (position->y < fspace.topLeftCorner.y))
		return false;
	return true;
}

FUNGE_ATTR_FAST bool
FungeSpaceCreate(void)
{
	fspace.entries = ght_create(FUNGESPACEINITIALSIZE);
	if (!fspace.entries)
		return false;
	ght_set_rehash(fspace.entries, true);

	return true;
}


FUNGE_ATTR_FAST void
FungeSpaceFree(void)
{
	if (fspace.entries)
		ght_finalize(fspace.entries);
}

FUNGE_ATTR_FAST void
FungeSpaceGetBoundRect(fungeRect * restrict rect)
{
	rect->x = fspace.topLeftCorner.x;
	rect->y = fspace.topLeftCorner.y;
	rect->w = fspace.bottomRightCorner.x - fspace.topLeftCorner.x;
	rect->h = fspace.bottomRightCorner.y - fspace.topLeftCorner.y;
}


FUNGE_ATTR_FAST FUNGEDATATYPE
FungeSpaceGet(const fungePosition * restrict position)
{
	FUNGEDATATYPE *tmp;

	assert(position != NULL);

	tmp = (FUNGEDATATYPE*)ght_get(fspace.entries, position);
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

	result = (FUNGEDATATYPE*)ght_get(fspace.entries, &tmp);
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
		ght_remove(fspace.entries, position);
	} else {
		// Reuse cell if it exists
		FUNGEDATATYPE *tmp;
		if ((tmp = (FUNGEDATATYPE*)ght_get(fspace.entries, position)) != NULL) {
			*tmp = value;
		} else {
			if (ght_insert(fspace.entries, value, position) == -1) {
				ght_replace(fspace.entries, value, position);
			}
		}
	}
}

FUNGE_ATTR_FAST void
FungeSpaceSet(FUNGEDATATYPE value, const fungePosition * restrict position)
{
	assert(position != NULL);
	FungeSpaceSetNoBoundUpdate(value, position);
	if (fspace.bottomRightCorner.y < position->y)
		fspace.bottomRightCorner.y = position->y;
	if (fspace.bottomRightCorner.x < position->x)
		fspace.bottomRightCorner.x = position->x;
	if (fspace.topLeftCorner.y > position->y)
		fspace.topLeftCorner.y = position->y;
	if (fspace.topLeftCorner.x > position->x)
		fspace.topLeftCorner.x = position->x;
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
		if (position->x < fspace.topLeftCorner.x)
			position->x = fspace.bottomRightCorner.x;
		else if (position->x >= fspace.bottomRightCorner.x)
			position->x = fspace.topLeftCorner.x;

		if (position->y < fspace.topLeftCorner.y)
			position->y = fspace.bottomRightCorner.y;
		else if (position->y >= fspace.bottomRightCorner.y)
			position->y = fspace.topLeftCorner.y;
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
	if (!fspace.entries)
		return;
	fprintf(stderr, "Fungespace follows:\n");
	for (FUNGEVECTORTYPE y = 0; y <= fspace.bottomRightCorner.y; y++) {
		for (FUNGEVECTORTYPE x = 0; x <= fspace.bottomRightCorner.x; x++)
			fprintf(stderr, "%c", (char)FungeSpaceGet(VectorCreateRef(x, y)));
		fprintf(stderr, "\n");
	}
	fputs("\n", stderr);
}

#endif

// Returns fd, addr and length.
/**
 * mmap() a file.
 * @param filename Filename to mmap().
 * @param maddr Pointer to a char* where the mapping's address will be placed.
 * @param length Pointer to a size_t where the size of the mapping will be placed.
 * @return
 * Returns the file descriptor, or -1 in case of error, o
r -2 in case of empty file.
 */
FUNGE_ATTR_FAST
static inline int DoMmap(const char * restrict filename, char **maddr, size_t * restrict length) {
	char *addr = NULL;
	struct stat sb;
	int fd = -1;
	size_t len;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return -1;

	if (fstat(fd, &sb) == -1) {
		perror("fstat on file failed");
		goto error;
	}

	len = sb.st_size;
	*length = len;
	// An empty file isn't an error, but we can't mmap it.
	if (len == 0) {
		close(fd);
		return -2;
	}
	// mmap() it.
	addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		perror("mmap() on file failed");
		goto error;
	}
	*maddr = addr;
	return fd;
error:
	if (addr != NULL) {
		munmap(addr, len);
	}
	if (fd != -1) {
		close(fd);
	}
	return -1;
}

/**
 * Clean up a mapping created with DoMmap().
 * @param fd is the file descriptor to close.
 * @param addr is the address to the mmap()ed area.
 * @param length is the length of the mmap()ed area.
 */
FUNGE_ATTR_FAST
static inline void DoMmapCleanup(int fd, char *addr, size_t length) {
	if (addr != NULL) {
		munmap(addr, length);
	}
	if (fd != -1) {
		close(fd);
	}
}

/**
 * Load a string into Funge-Space at 0,0. Used for initial loading.
 * Can handle null-bytes in the string without problems.
 * @param program is the string to load.
 * @param length is the length of the string.
 */
FUNGE_ATTR_FAST
static inline void LoadString(const char * restrict program, size_t length) {
	bool lastwascr = false;
	bool noendingnewline = true;
	// Row in fungespace
	FUNGEVECTORTYPE y = 0;
	FUNGEVECTORTYPE x = 0;

	for (size_t i = 0; i < length; i++) {
		switch (program[i]) {
			// Ignore Form Feed.
			case '\f':
				break;
			case '\r':
				lastwascr = true;
				break;
			case '\n':
				if (fspace.bottomRightCorner.x < x)
					fspace.bottomRightCorner.x = x;
				x = 0;
				y++;
				lastwascr = false;
				noendingnewline = false;
				break;
			default:
				if (lastwascr) {
					if (fspace.bottomRightCorner.x < x)
						fspace.bottomRightCorner.x = x;
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

	if (fspace.bottomRightCorner.x < x)
		fspace.bottomRightCorner.x = x;
	if (lastwascr) {
		noendingnewline = false;
		y++;
	}
	if (noendingnewline) y++;
	if (fspace.bottomRightCorner.y < y)
		fspace.bottomRightCorner.y = y;
}

FUNGE_ATTR_FAST bool
FungeSpaceLoad(const char * restrict filename)
{
	char *addr;
	int fd;
	size_t length;

	assert(filename != NULL);

	fd = DoMmap(filename, &addr, &length);
	if (fd == -1)
		return false;
	// Empty file?
	if (fd == -2)
		return true;

	LoadString(addr, length);

	// Cleanup
	DoMmapCleanup(fd, addr, length);
	return true;
}


#ifdef FUNGE_EXTERNAL_LIBRARY
FUNGE_ATTR_FAST void
FungeSpaceLoadString(const char * restrict program)
{
	LoadString(program, strlen(program));
}
#endif

FUNGE_ATTR_FAST bool
FungeSpaceLoadAtOffset(const char          * restrict filename,
                       const fungePosition * restrict offset,
                       fungeVector         * restrict size,
                       bool binary)
{
	char *addr;
	int fd;
	size_t length;

	bool lastwascr = false;

	FUNGEVECTORTYPE y = 0;
	FUNGEVECTORTYPE x = 0;
	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	fd = DoMmap(filename, &addr, &length);
	if (fd == -1)
		return false;
	// Empty file?
	if (fd == -2)
		return true;

	size->x = 0;
	size->y = 0;

	for (size_t i = 0; i < length; i++) {
		if (binary) {
			if (addr[i] != ' ')
				FungeSpaceSetOff((FUNGEDATATYPE)addr[i], VectorCreateRef(x, y), offset);
			x++;
		} else {
			switch (addr[i]) {
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
					if (addr[i] != ' ')
						FungeSpaceSetOff((FUNGEDATATYPE)addr[i], VectorCreateRef(x, y), offset);
					x++;
					break;
			}
		}
	}

	if (x > size->x) size->x = x;
	if (y > size->y) size->y = y;
	DoMmapCleanup(fd, addr, length);
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

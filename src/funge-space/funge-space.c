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

/*
 * How it works:
 * * We use a static array for the commonly used funge space near (0,0).
 * * The array is slightly offset to include a bit of the negative funge space
 *   too.
 * * Outside this array we use a hash library.
 */


#include "../global.h"
#include "funge-space.h"
#include "../../lib/libghthash/ght_hash_table.h"
#include "../../lib/libghthash/cfunge_mempool.h"

#include <unistd.h>

#if !defined(_POSIX_MAPPED_FILES) || (_POSIX_MAPPED_FILES < 1)
#  error "cfunge needs a working mmap(), which this system claims it lacks."
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#define FUNGESPACEINITIALSIZE 0x40000

typedef struct fungeSpace {
	/// These two form a rectangle for the program size
	fungeVector       topLeftCorner;
	fungeVector       bottomRightCorner;
	/// And this is the hash table.
	ght_hash_table_t *entries;
	/// Used during loading to handle 0,0 not being least point.
	bool              boundsvalid;
} fungeSpace;

/// Funge-space storage.
static fungeSpace fspace = {
	.topLeftCorner = {0, 0},
	.bottomRightCorner = {0, 0},
	.entries = NULL,
	.boundsvalid = false
};

#define FUNGESPACE_STATIC_OFFSET_X 64
#define FUNGESPACE_STATIC_OFFSET_Y 64
#define FUNGESPACE_STATIC_X 512
#define FUNGESPACE_STATIC_Y 1024

#define FUNGESPACE_RANGE_CHECK(rx, ry) \
	(((rx) < FUNGESPACE_STATIC_X) && ((ry) < FUNGESPACE_STATIC_Y))
#define STATIC_COORD(rx, ry) ((rx)+(ry)*FUNGESPACE_STATIC_X)

static fungeCell static_space[FUNGESPACE_STATIC_X * FUNGESPACE_STATIC_Y];

/**
 * Check if position is in range.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline bool fungespace_in_range(const fungeVector * restrict position)
{
	if ((position->x > fspace.bottomRightCorner.x) || (position->x < fspace.topLeftCorner.x))
		return false;
	if ((position->y > fspace.bottomRightCorner.y) || (position->y < fspace.topLeftCorner.y))
		return false;
	return true;
}


FUNGE_ATTR_FAST bool
fungespace_create(void)
{
	// Fill static array with spaces.
	for (size_t i = 0; i < sizeof(static_space) / sizeof(fungeCell); i++)
		static_space[i] = ' ';
	fspace.entries = ght_create(FUNGESPACEINITIALSIZE);
	if (!fspace.entries)
		return false;
	ght_set_rehash(fspace.entries, true);
	// Set up mempool for hash library.
	return mempool_setup();
}


FUNGE_ATTR_FAST void
fungespace_free(void)
{
	if (fspace.entries)
		ght_finalize(fspace.entries);
	mempool_teardown();
}


FUNGE_ATTR_FAST void
fungespace_get_bounds_rect(fungeRect * restrict rect)
{
	rect->x = fspace.topLeftCorner.x;
	rect->y = fspace.topLeftCorner.y;
	// +1 because it is inclusive.
	rect->w = fspace.bottomRightCorner.x - fspace.topLeftCorner.x + 1;
	rect->h = fspace.bottomRightCorner.y - fspace.topLeftCorner.y + 1;
}


FUNGE_ATTR_FAST fungeCell
fungespace_get(const fungeVector * restrict position)
{
	fungeCell *tmp;
	// Offsets for static.
	fungeUnsignedCell x = (fungeUnsignedCell)position->x + FUNGESPACE_STATIC_OFFSET_X;
	fungeUnsignedCell y = (fungeUnsignedCell)position->y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x,y)) {
		return static_space[STATIC_COORD(x,y)];
	} else {
		tmp = (fungeCell*)ght_get(fspace.entries, position);
		if (!tmp)
			return (fungeCell)' ';
		else
			return *tmp;
	}
}


FUNGE_ATTR_FAST fungeCell
fungespace_get_offset(const fungeVector * restrict position,
                      const fungeVector * restrict offset)
{
	fungeVector tmp;
	fungeCell *result;
	// Offsets for static.
	fungeUnsignedCell x, y;

	assert(position != NULL);
	assert(offset != NULL);

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	x = (fungeUnsignedCell)tmp.x + FUNGESPACE_STATIC_OFFSET_X;
	y = (fungeUnsignedCell)tmp.y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x,y)) {
		return static_space[STATIC_COORD(x,y)];
	} else {
		result = (fungeCell*)ght_get(fspace.entries, &tmp);
		if (!result)
			return (fungeCell)' ';
		else
			return *result;
	}
}


FUNGE_ATTR_FAST static inline void
fungespace_set_no_bounds_update(fungeCell value,
                                const fungeVector * restrict position)
{
	// Offsets for static.
	fungeUnsignedCell x = (fungeUnsignedCell)position->x + FUNGESPACE_STATIC_OFFSET_X;
	fungeUnsignedCell y = (fungeUnsignedCell)position->y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x,y)) {
		static_space[STATIC_COORD(x,y)] = value;
	} else {
		if (value == ' ') {
			ght_remove(fspace.entries, position);
		} else {
			// Reuse cell if it exists
			fungeCell *tmp;
			if ((tmp = (fungeCell*)ght_get(fspace.entries, position)) != NULL) {
				*tmp = value;
			} else {
				if (ght_insert(fspace.entries, value, position) == -1) {
					ght_replace(fspace.entries, value, position);
				}
			}
		}
	}
}


FUNGE_ATTR_FAST void
fungespace_set(fungeCell value, const fungeVector * restrict position)
{
	assert(position != NULL);
	fungespace_set_no_bounds_update(value, position);
	if (value != ' ') {
		if (fspace.bottomRightCorner.y < position->y)
			fspace.bottomRightCorner.y = position->y;
		if (fspace.bottomRightCorner.x < position->x)
			fspace.bottomRightCorner.x = position->x;
		if (fspace.topLeftCorner.y > position->y)
			fspace.topLeftCorner.y = position->y;
		if (fspace.topLeftCorner.x > position->x)
			fspace.topLeftCorner.x = position->x;
	}
}


FUNGE_ATTR_FAST static inline void
fungespace_set_initial(fungeCell value, const fungeVector * restrict position)
{
	assert(position != NULL);
	fungespace_set_no_bounds_update(value, position);
	if (value != ' ') {
		if (!fspace.boundsvalid || fspace.bottomRightCorner.y < position->y)
			fspace.bottomRightCorner.y = position->y;
		if (!fspace.boundsvalid || fspace.bottomRightCorner.x < position->x)
			fspace.bottomRightCorner.x = position->x;
		if (!fspace.boundsvalid || fspace.topLeftCorner.y > position->y)
			fspace.topLeftCorner.y = position->y;
		if (!fspace.boundsvalid || fspace.topLeftCorner.x > position->x)
			fspace.topLeftCorner.x = position->x;
		fspace.boundsvalid = true;
	}
}


FUNGE_ATTR_FAST void
fungespace_set_offset(fungeCell value, const fungeVector * restrict position,
                      const fungeVector * restrict offset)
{
	assert(position != NULL);
	assert(offset != NULL);

	fungespace_set(value, vector_create_ref(position->x + offset->x, position->y + offset->y));
}


FUNGE_ATTR_FAST void
fungespace_wrap(fungeVector * restrict position,
                const fungeVector * restrict delta)
{
	if (!fungespace_in_range(position)) {
		// Quick and dirty if cardinal.
		if (vector_is_cardinal(delta)) {
			// FIXME, HACK: Why are the +1/-1 needed?
			if (position->x < fspace.topLeftCorner.x)
				position->x = fspace.bottomRightCorner.x + 1;
			else if (position->x > fspace.bottomRightCorner.x)
				position->x = fspace.topLeftCorner.x - 1;

			if (position->y < fspace.topLeftCorner.y)
				position->y = fspace.bottomRightCorner.y + 1;
			else if (position->y > fspace.bottomRightCorner.y)
				position->y = fspace.topLeftCorner.y - 1;
		} else {
			do {
				position->x -= delta->x;
				position->y -= delta->y;
			} while (fungespace_in_range(position));
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
void fungespace_dump(void) FUNGE_ATTR_UNUSED;

void fungespace_dump(void)
{
	if (!fspace.entries)
		return;
	fprintf(stderr, "Fungespace follows:\n");
	for (fungeCell y = 0; y <= fspace.bottomRightCorner.y; y++) {
		for (fungeCell x = 0; x <= fspace.bottomRightCorner.x; x++)
			fprintf(stderr, "%c", (char)fungespace_get(vector_create_ref(x, y)));
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
static inline int do_mmap(const char * restrict filename, unsigned char **maddr,
                          size_t * restrict length)
{
	char *addr = NULL;
	struct stat sb;
	int fd = -1;
	size_t len;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return -1;

	if (fstat(fd, &sb) == -1) {
		perror("fstat() on file failed");
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
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
	posix_madvise(addr, len, POSIX_MADV_SEQUENTIAL);
#endif
	*maddr = (unsigned char*)addr;
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
 * Clean up a mapping created with do_mmap().
 * @param fd is the file descriptor to close.
 * @param addr is the address to the mmap()ed area.
 * @param length is the length of the mmap()ed area.
 */
FUNGE_ATTR_FAST
static inline void do_mmap_cleanup(int fd, unsigned char *addr, size_t length)
{
	if (addr != NULL) {
		munmap((char*)addr, length);
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
static inline void load_string(const unsigned char * restrict program,
                               size_t length)
{
	bool lastwascr = false;
	// Row in fungespace
	fungeCell y = 0;
	fungeCell x = 0;

	for (size_t i = 0; i < length; i++) {
		switch (program[i]) {
			// We ignore Form Feed here.
			case '\f':
				break;
			case '\r':
				lastwascr = true;
				break;
			case '\n':
				x = 0;
				y++;
				lastwascr = false;
				break;
			default:
				if (lastwascr) {
					lastwascr = false;
					x = 0;
					y++;
				}
				fungespace_set_initial((fungeCell)program[i], vector_create_ref(x, y));
				x++;
				break;
		}
	}
}

FUNGE_ATTR_FAST bool
fungespace_load(const char * restrict filename)
{
	unsigned char *addr;
	int fd;
	size_t length;

	assert(filename != NULL);

	fd = do_mmap(filename, &addr, &length);
	if (fd == -1)
		return false;
	// Empty file?
	if (fd == -2)
		return true;

	load_string(addr, length);

	// Cleanup
	do_mmap_cleanup(fd, addr, length);
	return true;
}


#ifdef FUNGE_EXTERNAL_LIBRARY
FUNGE_ATTR_FAST void
fungespace_load_string(const unsigned char * restrict program)
{
	load_string(program, strlen(program));
}
#endif

FUNGE_ATTR_FAST bool
fungespace_load_at_offset(const char        * restrict filename,
                          const fungeVector * restrict offset,
                          fungeVector       * restrict size,
                          bool binary)
{
	unsigned char *addr;
	int fd;
	size_t length;

	bool lastwascr = false;

	fungeCell y = 0;
	fungeCell x = 0;
	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	fd = do_mmap(filename, &addr, &length);
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
				fungespace_set_offset((fungeCell)addr[i], vector_create_ref(x, y), offset);
			x++;
		} else {
			switch (addr[i]) {
				// We don't ignore Form Feed here...
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
						fungespace_set_offset((fungeCell)addr[i], vector_create_ref(x, y), offset);
					x++;
					break;
			}
		}
	}

	if (x > size->x) size->x = x;
	if (y > size->y) size->y = y;
	do_mmap_cleanup(fd, addr, length);
	return true;
}

FUNGE_ATTR_FAST bool
fungespace_save_to_file(const char        * restrict filename,
                        const fungeVector * restrict offset,
                        const fungeVector * restrict size,
                        bool textfile)
{
	FILE * file;

	fungeCell maxy = offset->y + size->y;
	fungeCell maxx = offset->x + size->x;

	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	file = fopen(filename, "wb");
	if (!file)
		return false;

	if (!textfile) {
		fungeCell value;
		// Microoptimising! Remove this if it bothers you.
		// However it also makes it possible to error out early.
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
		if (posix_fallocate(fileno(file), 0, size->y * size->x) != 0) {
			fclose(file);
			return false;
		}
#endif
		cf_flockfile(file);
		for (fungeCell y = offset->y; y < maxy; y++) {
			for (fungeCell x = offset->x; x < maxx; x++) {
				value = fungespace_get(vector_create_ref(x, y));
				cf_putc_unlocked(value, file);
			}
			cf_putc_unlocked('\n', file);
		}
		cf_funlockfile(file);
	// Text mode...
	} else {
		size_t index = 0;
		// Extra size->y for adding a lot of \n...
		fungeCell * restrict towrite = cf_malloc((size->x * size->y + size->y) * sizeof(fungeCell));
		if (!towrite) {
			fclose(file);
			return false;
		}
		// Construct each line.
		for (fungeCell y = offset->y; y < maxy; y++) {
			ssize_t lastspace = size->x;
			fungeCell * restrict string = cf_malloc(size->x * sizeof(fungeCell));
			if (!string) {
				fclose(file);
				return false;
			}
			for (fungeCell x = offset->x; x < maxx; x++) {
				string[x-offset->x] = fungespace_get(vector_create_ref(x, y));
			}

			do {
				lastspace--;
			} while ((lastspace >= 0) && (string[lastspace] == ' '));

			if (lastspace > 0) {
				for (ssize_t i = 0; i <= lastspace; i++) {
					towrite[index+i] = string[i];
				}
				index += lastspace + 1;
			}
			cf_free(string);
			towrite[index] = (fungeCell)'\n';
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

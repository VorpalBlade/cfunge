/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2009 Arvid Norlander <anmaster AT tele2 DOT se>
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
	funge_vector      topLeftCorner;
	funge_vector      bottomRightCorner;
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
// Note that this must be true to not break code below:
//  (FUNGESPACE_STATIC_X * FUNGESPACE_STATIC_Y * sizeof(funge_cell)) % 128 == 0
// Further cfun_static_space must be aligned on 16 byte boundary.
#define FUNGESPACE_STATIC_X 512
#define FUNGESPACE_STATIC_Y 1024

#define FUNGESPACE_RANGE_CHECK(rx, ry) \
	(((rx) < FUNGESPACE_STATIC_X) && ((ry) < FUNGESPACE_STATIC_Y))
#define STATIC_COORD(rx, ry) ((rx)+(ry)*FUNGESPACE_STATIC_X)

// We need to give it an asm name here, or the non-PIC inline asm below won't
// work properly in some cases.
static funge_cell cfun_static_space[FUNGESPACE_STATIC_X * FUNGESPACE_STATIC_Y]
#ifdef __GNUC__
__asm__("cfun_static_space")
#endif
FUNGE_ATTR((aligned(16)))
;

/**
 * Check if position is in range.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_PURE FUNGE_ATTR_WARN_UNUSED
static inline bool fungespace_in_range(const funge_vector * restrict position)
{
	if ((position->x > fspace.bottomRightCorner.x)
	    || (position->x < fspace.topLeftCorner.x)
	    || (position->y > fspace.bottomRightCorner.y)
	    || (position->y < fspace.topLeftCorner.y))
		return false;
	return true;
}

#undef FSPACE_CREATE_SSE
#undef FSPACE_SSE_ASM
#if (defined(__i386__) || defined(__x86_64__)) && defined(__GNUC__) && defined(__SSE__) && !defined(__INTEL_COMPILER)
#  define FSPACE_CREATE_SSE 1
#endif
#if defined(FSPACE_CREATE_SSE) && defined(__SSE2__) && defined(__x86_64__)
#  define FSPACE_SSE_ASM 1
#endif
#if defined(FSPACE_CREATE_SSE)
typedef int32_t v4si __attribute__((vector_size(16)));
#  ifndef FSPACE_SSE_ASM
typedef float v4sf __attribute__((vector_size(16)));
#  endif
#  if defined(USE32)
#    define FUNGESPACE_DATASIZE_STR "4"
static const v4si fspace_vector_init = {0x20, 0x20, 0x20, 0x20};
#  elif defined(USE64)
#    define FUNGESPACE_DATASIZE_STR "8"
static const v4si fspace_vector_init = {0x20, 0x0, 0x20, 0x0};
#  else
#    error "Unknown funge space data type size."
#  endif
#endif

FUNGE_ATTR_FAST bool
fungespace_create(void)
{
	// Mark the static space area as pointer-free when using Boehm-GC.
	// FIXME: Not sure the arguments are correct..
	cf_mark_static_noptr(&cfun_static_space,
	                     &cfun_static_space[FUNGESPACE_STATIC_X * FUNGESPACE_STATIC_Y]);
	// Fill static array with spaces.
	// When possible use movntps, which reduces cache pollution (because it acts
	// as if the memory was write combining).
	//
	// GCC's __builtin_ia32_movntps refuses to load thing in the fastest way, so
	// provide an inline asm version too. Also __builtin_ia32_movntps generates
	// terrible code for -O0. Worse than plain C variant with no vectorisation
	// at all.
	//
	// Further using %[space] in the movntps resulted in the invalid asm:
	// cfun_static_space(%rip)(%rax)
	// I still had to list it as output though.
	//
	// For the pure-ISO C variant, something like -ftree-vectorize (GCC) or
	// -xP (icc, generate for SSE/SSE2/SSE3) can be used to at least speed up
	// the execution a bit. Though you will still get cache pollution.
	//
	// For the PIC variant GCC's i constraint generate a $ in front of the number
	// which doesn't work here. So we do it manually with macros that expand to
	// the correct sizes.
	//
	// I'm not sure if the sfence is needed. The AMD and Intel docs are a bit
	// unclear about this. And it doesn't seem to be needed in practise. However
	// I'd rather go for the safe alternative.
#if defined(FSPACE_CREATE_SSE) && defined(FSPACE_SSE_ASM)
#  if defined(__pic__) || defined(__PIC__)
	// I hate the C preprocessor...
#    define CPP_STRINGIFY_INNER(x) # x
#    define CPP_STRINGIFY(x) CPP_STRINGIFY_INNER(x)
	__asm__ volatile ("\
	leaq    %[space],%%rax\n\
	leaq    "FUNGESPACE_DATASIZE_STR
		"*"CPP_STRINGIFY(FUNGESPACE_STATIC_X)
		"*"CPP_STRINGIFY(FUNGESPACE_STATIC_Y)"+%[space],%%rdx\n\
	movdqa  %[mask],%%xmm0\n\
	.p2align 4,,7\n\
cf_fungespace_create_init_loop:\n\
	movntps %%xmm0,(%%rax)\n\
	addq    $16,%%rax\n\
	cmpq    %%rdx,%%rax\n\
	jne     cf_fungespace_create_init_loop\n\
	sfence"
: [space] "=o"(cfun_static_space)
: [mask]  "m"(fspace_vector_init)
: "rax", "rdx", "xmm0");
#  else
	__asm__ volatile ("\
	xor     %%eax,%%eax\n\
	movdqa  %[mask],%%xmm0\n\
	.p2align 4,,7\n\
cf_fungespace_create_init_loop:\n\
	movntps %%xmm0,cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x10+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x20+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x30+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x40+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x50+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x60+cfun_static_space(%%rax)\n\
	movntps %%xmm0,0x70+cfun_static_space(%%rax)\n\
	add     $128,%%rax\n\
	cmp     %[size],%%rax\n\
	jne     cf_fungespace_create_init_loop\n\
	sfence"
	: [space] "=o"(cfun_static_space)
	: [mask]  "m"(fspace_vector_init)
	, [size]  "i"(sizeof(cfun_static_space))
	: "rax", "xmm0");
#  endif
#elif defined(FSPACE_CREATE_SSE)
	for (size_t i = 0; i < (sizeof(cfun_static_space) / 16); i++)
		// Cast to void to shut up warning about strict-aliasing rules.
		__builtin_ia32_movntps(((float*)(void*)&cfun_static_space) + i*4,
		                       *((const v4sf*)(const void*)&fspace_vector_init));
	__builtin_ia32_sfence();
#else
	for (size_t i = 0; i < sizeof(cfun_static_space) / sizeof(funge_cell); i++)
		cfun_static_space[i] = ' ';
#endif
	fspace.entries = ght_create(FUNGESPACEINITIALSIZE);
	if (FUNGE_UNLIKELY(!fspace.entries))
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


FUNGE_ATTR_FAST funge_cell
fungespace_get(const funge_vector * restrict position)
{
	funge_cell *tmp;
	// Offsets for static.
	funge_unsigned_cell x = (funge_unsigned_cell)position->x + FUNGESPACE_STATIC_OFFSET_X;
	funge_unsigned_cell y = (funge_unsigned_cell)position->y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x, y)) {
		return cfun_static_space[STATIC_COORD(x,y)];
	} else {
		tmp = (funge_cell*)ght_get(fspace.entries, position);
		if (!tmp)
			return (funge_cell)' ';
		else
			return *tmp;
	}
}


FUNGE_ATTR_FAST funge_cell
fungespace_get_offset(const funge_vector * restrict position,
                      const funge_vector * restrict offset)
{
	funge_vector tmp;
	funge_cell *result;
	// Offsets for static.
	funge_unsigned_cell x, y;

	assert(position != NULL);
	assert(offset != NULL);

	tmp.x = position->x + offset->x;
	tmp.y = position->y + offset->y;

	x = (funge_unsigned_cell)tmp.x + FUNGESPACE_STATIC_OFFSET_X;
	y = (funge_unsigned_cell)tmp.y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x, y)) {
		return cfun_static_space[STATIC_COORD(x,y)];
	} else {
		result = (funge_cell*)ght_get(fspace.entries, &tmp);
		if (!result)
			return (funge_cell)' ';
		else
			return *result;
	}
}


FUNGE_ATTR_FAST static inline void
fungespace_set_no_bounds_update(funge_cell value,
                                const funge_vector * restrict position)
{
	// Offsets for static.
	funge_unsigned_cell x = (funge_unsigned_cell)position->x + FUNGESPACE_STATIC_OFFSET_X;
	funge_unsigned_cell y = (funge_unsigned_cell)position->y + FUNGESPACE_STATIC_OFFSET_Y;

	if (FUNGESPACE_RANGE_CHECK(x, y)) {
		cfun_static_space[STATIC_COORD(x,y)] = value;
	} else {
		if (value == ' ') {
			ght_remove(fspace.entries, position);
		} else {
			// Reuse cell if it exists
			funge_cell *tmp;
			if ((tmp = (funge_cell*)ght_get(fspace.entries, position)) != NULL) {
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
fungespace_set(funge_cell value, const funge_vector * restrict position)
{
	assert(position != NULL);
	if (value != ' ') {
		// It is faster to not use else if here, because this way the code
		// translates into conditional moves (on x86 at least).
		if (fspace.bottomRightCorner.y < position->y)
			fspace.bottomRightCorner.y = position->y;
		if (fspace.topLeftCorner.y > position->y)
			fspace.topLeftCorner.y = position->y;
		if (fspace.bottomRightCorner.x < position->x)
			fspace.bottomRightCorner.x = position->x;
		if (fspace.topLeftCorner.x > position->x)
			fspace.topLeftCorner.x = position->x;
	}
	fungespace_set_no_bounds_update(value, position);
}


/**
 * Special variant of fungespace_set() to deal with initial load.
 * Needed to handle the initial bounding box properly.
 * Must NOT be called with a space for value.
 */
FUNGE_ATTR_FAST static inline void
fungespace_set_initial(funge_cell value, const funge_vector * restrict position)
{
	if (FUNGE_LIKELY(fspace.boundsvalid)) {
		// It is faster to not use else if here, because this way the code
		// translates into conditional moves (on x86 at least).
		if (fspace.bottomRightCorner.y < position->y)
			fspace.bottomRightCorner.y = position->y;
		if (fspace.topLeftCorner.y > position->y)
			fspace.topLeftCorner.y = position->y;
		if (fspace.bottomRightCorner.x < position->x)
			fspace.bottomRightCorner.x = position->x;
		if (fspace.topLeftCorner.x > position->x)
			fspace.topLeftCorner.x = position->x;
	} else {
		fspace.topLeftCorner.y = fspace.bottomRightCorner.y = position->y;
		fspace.topLeftCorner.x = fspace.bottomRightCorner.x = position->x;
		fspace.boundsvalid = true;
	}
	fungespace_set_no_bounds_update(value, position);
}


FUNGE_ATTR_FAST void
fungespace_set_offset(funge_cell value,
                      const funge_vector * restrict position,
                      const funge_vector * restrict offset)
{
	assert(position != NULL);
	assert(offset != NULL);

	fungespace_set(value, vector_create_ref(position->x + offset->x, position->y + offset->y));
}


/// Duplicated from vector.c for speed reasons (inlining).
FUNGE_ATTR_PURE FUNGE_ATTR_FAST
static inline bool fspace_vector_is_cardinal(const funge_vector * restrict v)
{
	// Due to unsigned this can't overflow in the addition below.
	funge_unsigned_cell x = (funge_unsigned_cell)ABS(v->x);
	funge_unsigned_cell y = (funge_unsigned_cell)ABS(v->y);
	if ((x + y) != 1)
		return false;
	if (x && y)
		return false;
	return true;
}


FUNGE_ATTR_FAST void
fungespace_wrap(funge_vector * restrict position,
                const funge_vector * restrict delta)
{
	if (!fungespace_in_range(position)) {
		// Quick and dirty if cardinal.
		if (FUNGE_LIKELY(fspace_vector_is_cardinal(delta))) {
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
	for (funge_cell y = 0; y <= fspace.bottomRightCorner.y; y++) {
		for (funge_cell x = 0; x <= fspace.bottomRightCorner.x; x++)
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
 * Returns the file descriptor, or -1 in case of error, or -2 in case of
 * empty file.
 */
FUNGE_ATTR_FAST
static inline int do_mmap(const char * restrict filename,
                          unsigned char **maddr,
                          size_t * restrict length)
{
	char *addr = NULL;
	struct stat sb;
	int fd = -1;
	size_t len;

	fd = open(filename, O_RDONLY);
	if (FUNGE_UNLIKELY(fd == -1))
		return -1;

	if (FUNGE_UNLIKELY(fstat(fd, &sb) == -1)) {
		perror("fstat() on file failed");
		goto error;
	}

	len = (size_t)sb.st_size;
	*length = len;
	// An empty file isn't an error, but we can't mmap it.
	if (len == 0) {
		close(fd);
		return -2;
	}
	// mmap() it.
	addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (FUNGE_UNLIKELY(addr == MAP_FAILED)) {
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
	if (FUNGE_LIKELY(addr != NULL)) {
		munmap((char*)addr, length);
	}
	if (FUNGE_LIKELY(fd != -1)) {
		close(fd);
	}
}

/// Macro for handling newlines.
#define FUNGE_INITIAL_NEWLINE \
	pos.x = 0; \
	pos.y++;

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
	// Coord in Funge-Space.
	funge_vector pos = {0, 0};

	for (size_t i = 0; i < length; i++) {
		switch (program[i]) {
			case ' ':
				pos.x++;
				break;
			case '\r':
				if (lastwascr) {
					FUNGE_INITIAL_NEWLINE
				}
				lastwascr = true;
				break;
			case '\n':
				lastwascr = false;
				FUNGE_INITIAL_NEWLINE
				break;
			// Ignore form feed. Treat it as newline is treated in Unefunge.
			case '\f':
				break;
			default:
				if (lastwascr) {
					lastwascr = false;
					FUNGE_INITIAL_NEWLINE
				}
				fungespace_set_initial((funge_cell)program[i], &pos);
				pos.x++;
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
	load_string(program, strlen((const char*)program));
}
#endif

/// Macro for handling newlines.
#define FUNGE_OFFSET_NEWLINE \
	if (pos.x > size->x) \
		size->x = pos.x; \
	pos.x = 0; \
	pos.y++;

FUNGE_ATTR_FAST bool
fungespace_load_at_offset(const char         * restrict filename,
                          const funge_vector * restrict offset,
                          funge_vector       * restrict size,
                          bool binary)
{
	unsigned char *addr;
	int fd;
	size_t length;
	funge_vector pos = {0, 0};

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

	if (binary) {
		pos.x = offset->x;
		pos.y = offset->y;
		for (size_t i = 0; i < length; i++) {
			if (addr[i] != ' ')
				fungespace_set((funge_cell)addr[i], &pos);
			pos.x++;
		}
	} else {
		bool lastwascr = false;
		for (size_t i = 0; i < length; i++) {
			switch (addr[i]) {
				// Ignore form feed. Treat it as newline is treated in Unefunge.
				case '\f':
					break;
				case '\r':
					if (lastwascr) {
						// Blergh two \r after each other.
						FUNGE_OFFSET_NEWLINE
					}
					lastwascr = true;
					break;
				case '\n':
					FUNGE_OFFSET_NEWLINE
					lastwascr = false;
					break;
				default:
					if (lastwascr) {
						lastwascr = false;
						FUNGE_OFFSET_NEWLINE
					}
					if (addr[i] != ' ')
						fungespace_set_offset((funge_cell)addr[i], &pos, offset);
					pos.x++;
					break;
			}
		}
		if (lastwascr) pos.y++;
	}
	if (pos.x > size->x) size->x = pos.x;
	if (pos.y > size->y) size->y = pos.y;
	do_mmap_cleanup(fd, addr, length);
	return true;
}

FUNGE_ATTR_FAST bool
fungespace_save_to_file(const char         * restrict filename,
                        const funge_vector * restrict offset,
                        const funge_vector * restrict size,
                        bool textfile)
{
	FILE * file;

	funge_cell maxy = offset->y + size->y;
	funge_cell maxx = offset->x + size->x;

	assert(filename != NULL);
	assert(offset != NULL);
	assert(size != NULL);

	file = fopen(filename, "wb");
	if (!file)
		return false;

	if (!textfile) {
		// Microoptimising! Remove this if it bothers you.
		// However it also makes it possible to error out early.
#if defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
		if (posix_fallocate(fileno(file), 0, (off_t)(size->y * size->x)) != 0) {
			goto error;
		}
#endif
		cf_flockfile(file);
		for (funge_cell y = offset->y; y < maxy; y++) {
			for (funge_cell x = offset->x; x < maxx; x++) {
				funge_cell value = fungespace_get(vector_create_ref(x, y));
				cf_putc_unlocked((int)value, file);
			}
			cf_putc_unlocked('\n', file);
		}
		cf_funlockfile(file);
	// Text mode...
	} else {
		size_t index = 0;
		// Extra size->y for adding a lot of \n...
		unsigned char * restrict towrite = cf_malloc_noptr((size_t)(size->x * size->y + size->y) * sizeof(unsigned char));
		if (!towrite) {
			goto error;
		}
		// Construct each line.
		for (funge_cell y = offset->y; y < maxy; y++) {
			ssize_t lastspace = (ssize_t)size->x;
			funge_cell * restrict string = cf_malloc_noptr((size_t)size->x * sizeof(funge_cell));
			if (!string) {
				cf_free(towrite);
				goto error;
			}
			for (funge_cell x = offset->x; x < maxx; x++) {
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
			towrite[index] = (funge_cell)'\n';
			index++;
		}
		// Remove trailing newlines.
		{
			ssize_t lastnewline = index;
			do {
				lastnewline--;
			} while ((lastnewline >= 0) && (towrite[lastnewline] == '\n'));

			if (lastnewline > 0) {
				size_t retval = fwrite(towrite, sizeof(unsigned char), lastnewline+1, file);
				if (retval != (size_t)lastnewline+1) {
					cf_free(towrite);
					goto error;
				}
			}
		}
		cf_free(towrite);
	}
	fclose(file);
	return true;
error:
	if (file)
		fclose(file);
	return false;
}

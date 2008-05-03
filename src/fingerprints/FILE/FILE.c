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

#include "FILE.h"
#include "../../stack.h"
#include <stdio.h>
#include <assert.h>

// Based on how CCBI does it.

typedef struct sFungeFileHandle {
	FILE      * file;
	fungeVector buffvect; // IO buffer in Funge-Space
} FungeFileHandle;

#define ALLOCCHUNK 2
// Array of pointers
static FungeFileHandle** handles = NULL;
static FUNGEDATATYPE maxHandle = 0;

FUNGE_FAST static inline FUNGEDATATYPE findNextFreeHandle(void)
{
	for (FUNGEDATATYPE i = 0; i < maxHandle; i++) {
		if (handles[i] == NULL)
			return i;
	}
	// No free one, extend array..
	{
		FungeFileHandle** newlist = (FungeFileHandle**)cf_realloc(handles, (maxHandle + ALLOCCHUNK) * sizeof(FungeFileHandle*));
		if (!newlist)
			return -1;
		handles = newlist;
		for (FUNGEDATATYPE i = maxHandle; i < (maxHandle + ALLOCCHUNK); i++)
			handles[i] = NULL;
		maxHandle += ALLOCCHUNK;
		return (maxHandle - ALLOCCHUNK);
	}
}

FUNGE_FAST static inline FUNGEDATATYPE AllocateHandle(void)
{
	FUNGEDATATYPE h;

	h = findNextFreeHandle();
	if (h < 0)
		return -1;

	handles[h] = cf_malloc(sizeof(FungeFileHandle));
	if (!handles[h])
		return -1;
	return h;
}

FUNGE_FAST static inline void FreeHandle(FUNGEDATATYPE h)
{
	if (!handles[h])
		return;
	// Should be closed first!
	if (handles[h]->file != NULL) {
		handles[h]->file = NULL;
	}
	cf_free(handles[h]);
	handles[h] = NULL;
}

FUNGE_FAST static inline bool ValidHandle(FUNGEDATATYPE h)
{
	if ((h < 0) || (h > maxHandle) || (!handles[h])) {
		return false;
	} else {
		return true;
	}
}

// C - Close a file
static void FingerFILEfclose(instructionPointer * ip)
{
	FUNGEDATATYPE h;

	h = StackPop(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	if (fclose(handles[h]->file) != 0)
		ipReverse(ip);

	FreeHandle(h);
}

FUNGE_FAST static inline void append(char * restrict * restrict str,
                                     size_t * restrict s, size_t * restrict p,
                                     int ch)
{
	if (*p >= *s) {
		*str = cf_realloc(*str, (*s) * 2);
		// TODO Error handling
		(*s) *= 2;
	}
	(*str)[*p] = (char)ch;
	(*p)++;
}

// G - Get string from file (like c fgets)
static void FingerFILEfgets(instructionPointer * ip)
{
	FUNGEDATATYPE h;
	FILE * fp;

	h = StackPop(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, h);

	fp = handles[h]->file;

	{
		char * restrict str = NULL;
		// Position in string, and size of string
		size_t p = 0;
		size_t s = 80;
		int ch;
		str = cf_malloc_noptr(80 * sizeof(char));
		if (!str) {
			ipReverse(ip);
			return;
		}

		while (true) {
			ch = fgetc(fp);
			switch (ch) {
				case '\r':
					append(&str, &s, &p, ch);
					ch = fgetc(fp);
					if (ch != '\n') {
						ungetc(ch, fp);
						goto endofloop;
					}
				// Fallthrough intentional.
				case '\n':
					append(&str, &s, &p, ch);
					goto endofloop;

				case EOF:
					if (ferror(fp)) {
						clearerr(fp);
						ipReverse(ip);
						cf_free(str);
						return;
					} else {
						goto endofloop;
					}

				default:
					append(&str, &s, &p, ch);
					break;
			}
		}
		// Yeah, can't break two levels otherwise...
	endofloop:
		str[p] = '\0';
		StackPushString(ip->stack, str, p);
		StackPush(ip->stack, (FUNGEDATATYPE)p);
		cf_free(str);
		return;
	}
}

// L - Get current location in file
static void FingerFILEftell(instructionPointer * ip)
{
	FUNGEDATATYPE h;
	long pos;

	h = StackPop(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, h);

	pos = ftell(handles[h]->file);

	if (pos == -1) {
		clearerr(handles[h]->file);
		ipReverse(ip);
		return;
	}

	StackPush(ip->stack, (FUNGEDATATYPE)pos);
}

// O - Open a file (Va = i/o buffer vector)
static void FingerFILEfopen(instructionPointer * ip)
{
	char * restrict filename;
	FUNGEDATATYPE mode;
	fungeVector vect;
	FUNGEDATATYPE h;

	filename = StackPopString(ip->stack);
	mode = StackPop(ip->stack);
	vect = StackPopVector(ip->stack);

	h = AllocateHandle();
	if (h == -1) {
		ipReverse(ip);
		goto end;
	}

	switch (mode) {
		case 0: handles[h]->file = fopen(filename, "rb");  break;
		case 1: handles[h]->file = fopen(filename, "wb");  break;
		case 2: handles[h]->file = fopen(filename, "ab");  break;
		case 3: handles[h]->file = fopen(filename, "r+b"); break;
		case 4: handles[h]->file = fopen(filename, "w+b"); break;
		case 5: handles[h]->file = fopen(filename, "a+b"); break;
		default:
			FreeHandle(h);
			ipReverse(ip);
			goto end;
	}
	if (!handles[h]->file) {
		FreeHandle(h);
		ipReverse(ip);
		goto end;
	}
	if ((mode == 2) || (mode == 5))
		rewind(handles[h]->file);

	handles[h]->buffvect = vect;
	StackPush(ip->stack, h);
// Look... The alternatives to the goto were worse...
end:
#ifdef DISABLE_GC
	cf_free(filename);
#endif
	return;
}

// P - Put string to file (like c fputs)
static void FingerFILEfputs(instructionPointer * ip)
{
	char * restrict str;
	FUNGEDATATYPE h;

	str = StackPopString(ip->stack);
	h = StackPop(ip->stack);
	if (!ValidHandle(h)) {
		ipReverse(ip);
	} else {
		StackPush(ip->stack, h);

		if (fputs(str, handles[h]->file) == EOF) {
			clearerr(handles[h]->file);
			ipReverse(ip);
		}
	}
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

// R - Read n bytes from file to i/o buffer
static void FingerFILEfread(instructionPointer * ip)
{
	FUNGEDATATYPE n, h;

	n = StackPop(ip->stack);
	h = StackPop(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, h);

	if (n <= 0) {
		ipReverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = cf_calloc_noptr(n, sizeof(char));
		if (!buf) {
			ipReverse(ip);
			return;
		}

		if (fread(buf, sizeof(unsigned char), n, fp) != (size_t)n) {
			if (ferror(fp)) {
				clearerr(fp);
				ipReverse(ip);
				cf_free(buf);
				return;
			} else {
				assert (feof(fp));
			}
		}

		for (FUNGEDATATYPE i = 0; i < n; i++) {
			FungeSpaceSet(buf[i], VectorCreateRef(handles[h]->buffvect.x + i, handles[h]->buffvect.y));
		}
		cf_free(buf);
	}
}

// S - Seek to position in file
static void FingerFILEfseek(instructionPointer * ip)
{
	FUNGEDATATYPE n, m, h;

	n = StackPop(ip->stack);
	m = StackPop(ip->stack);
	h = StackPop(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, h);

	switch (m) {
		case 0:
			if (fseek(handles[h]->file, (long)n, SEEK_SET) != 0)
				break;
			else
				return;
		case 1:
			if (fseek(handles[h]->file, (long)n, SEEK_CUR) != 0)
				break;
			else
				return;
		case 2:
			if (fseek(handles[h]->file, (long)n, SEEK_END) != 0)
				break;
			else
				return;
		default:
			break;
	}
	// An error if we got here...
	clearerr(handles[h]->file);
	ipReverse(ip);
}

// W - Write n bytes from i/o buffer to file
static void FingerFILEfwrite(instructionPointer * ip)
{
	FUNGEDATATYPE n, h;

	n = StackPop(ip->stack);
	h = StackPop(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, h);

	if (n <= 0) {
		ipReverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = cf_malloc_noptr(n * sizeof(char));

		for (FUNGEDATATYPE i = 0; i < n; i++) {
			buf[i] = FungeSpaceGet(VectorCreateRef(handles[h]->buffvect.x + i, handles[h]->buffvect.y));
		}
		if (fwrite(buf, sizeof(unsigned char), n, fp) != (size_t)n) {
			if (ferror(fp)) {
				clearerr(fp);
				ipReverse(ip);
			}
		}
		cf_free(buf);
	}
}

FUNGE_FAST static inline bool InitHandleList(void)
{
	assert(!handles);
	handles = (FungeFileHandle**)cf_calloc(ALLOCCHUNK, sizeof(FungeFileHandle*));
	if (!handles)
		return false;
	maxHandle = ALLOCCHUNK;
	return true;
}

bool FingerFILEload(instructionPointer * ip)
{
	if (!handles)
		if (!InitHandleList())
			return false;

	ManagerAddOpcode(FILE,  'C', fclose)
	ManagerAddOpcode(FILE,  'G', fgets)
	ManagerAddOpcode(FILE,  'L', ftell)
	ManagerAddOpcode(FILE,  'O', fopen)
	ManagerAddOpcode(FILE,  'P', fputs)
	ManagerAddOpcode(FILE,  'R', fread)
	ManagerAddOpcode(FILE,  'S', fseek)
	ManagerAddOpcode(FILE,  'W', fwrite)
	return true;
}

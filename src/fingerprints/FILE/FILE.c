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
#include "../../../lib/stringbuffer/stringbuffer.h"
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
static fungeCell maxHandle = 0;

/// Used by AllocateHandle() below to find next free handle.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell findNextFreeHandle(void)
{
	for (fungeCell i = 0; i < maxHandle; i++) {
		if (handles[i] == NULL)
			return i;
	}
	// No free one, extend array..
	{
		FungeFileHandle** newlist = (FungeFileHandle**)cf_realloc(handles, (maxHandle + ALLOCCHUNK) * sizeof(FungeFileHandle*));
		if (!newlist)
			return -1;
		handles = newlist;
		for (fungeCell i = maxHandle; i < (maxHandle + ALLOCCHUNK); i++)
			handles[i] = NULL;
		maxHandle += ALLOCCHUNK;
		return (maxHandle - ALLOCCHUNK);
	}
}

/// Get a new handle to use for a file, also allocates buffer for it.
/// @return Handle, or -1 on failure
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell AllocateHandle(void)
{
	fungeCell h;

	h = findNextFreeHandle();
	if (h < 0)
		return -1;

	handles[h] = cf_malloc(sizeof(FungeFileHandle));
	if (!handles[h])
		return -1;
	return h;
}

/// Free a handle. fclose() the file before calling this.
FUNGE_ATTR_FAST
static inline void FreeHandle(fungeCell h)
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

/// Checks if handle is valid.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool ValidHandle(fungeCell h)
{
	if ((h < 0) || (h >= maxHandle) || (!handles[h])) {
		return false;
	} else {
		return true;
	}
}

/// C - Close a file
static void FingerFILEfclose(instructionPointer * ip)
{
	fungeCell h;

	h = StackPop(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	if (fclose(handles[h]->file) != 0)
		ipReverse(ip);

	FreeHandle(h);
}

/// C - Delete specified file
static void FingerFILEdelete(instructionPointer * ip)
{
	char * restrict filename;

	filename = StackPopString(ip->stack);
	if(unlink(filename) != 0) {
		ipReverse(ip);
	}

	StackFreeString(filename);
	return;
}


/// G - Get string from file (like c fgets)
static void FingerFILEfgets(instructionPointer * ip)
{
	fungeCell h;
	FILE * fp;

	h = StackPeek(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	fp = handles[h]->file;

	{
		StringBuffer *sb;
		int ch;
		sb = stringbuffer_new();
		if (!sb) {
			ipReverse(ip);
			return;
		}

		while (true) {
			ch = fgetc(fp);
			switch (ch) {
				case '\r':
					stringbuffer_append_char(sb, (char)ch);
					ch = fgetc(fp);
					if (ch != '\n') {
						ungetc(ch, fp);
						goto endofloop;
					}
				// Fallthrough intentional.
				case '\n':
					stringbuffer_append_char(sb, (char)ch);
					goto endofloop;

				case EOF:
					if (ferror(fp)) {
						clearerr(fp);
						ipReverse(ip);
						stringbuffer_destroy(sb);
						return;
					} else {
						goto endofloop;
					}

				default:
					stringbuffer_append_char(sb, (char)ch);
					break;
			}
		}
		// Yeah, can't break two levels otherwise...
	endofloop:
		{
			char * str;
			size_t len;
			str = stringbuffer_finish(sb);
			len = strlen(str);
			StackPushString(ip->stack, str, len);
			StackPush(ip->stack, (fungeCell)len);
			free_nogc(str);
			return;
		}
	}
}

/// L - Get current location in file
static void FingerFILEftell(instructionPointer * ip)
{
	fungeCell h;
	long pos;

	h = StackPeek(ip->stack);
	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	pos = ftell(handles[h]->file);

	if (pos == -1) {
		clearerr(handles[h]->file);
		ipReverse(ip);
		return;
	}

	StackPush(ip->stack, (fungeCell)pos);
}

/// O - Open a file (Va = i/o buffer vector)
static void FingerFILEfopen(instructionPointer * ip)
{
	char * restrict filename;
	fungeCell mode;
	fungeVector vect;
	fungeCell h;

	filename = StackPopString(ip->stack);
	mode = StackPop(ip->stack);
	vect = StackPopVector(ip->stack);

	h = AllocateHandle();
	if (h == -1) {
		goto error;
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
			goto error;
	}
	if (!handles[h]->file) {
		FreeHandle(h);
		goto error;
	}
	if ((mode == 2) || (mode == 5))
		rewind(handles[h]->file);

	handles[h]->buffvect = vect;
	StackPush(ip->stack, h);
	goto end;
// Look... The alternatives to the goto were worse...
error:
	ipReverse(ip);
end:
	StackFreeString(filename);
}

/// P - Put string to file (like c fputs)
static void FingerFILEfputs(instructionPointer * ip)
{
	char * restrict str;
	fungeCell h;

	str = StackPopString(ip->stack);
	h = StackPeek(ip->stack);
	if (!ValidHandle(h)) {
		ipReverse(ip);
	} else {
		if (fputs(str, handles[h]->file) == EOF) {
			clearerr(handles[h]->file);
			ipReverse(ip);
		}
	}
	StackFreeString(str);
}

/// R - Read n bytes from file to i/o buffer
static void FingerFILEfread(instructionPointer * ip)
{
	fungeCell n, h;

	n = StackPop(ip->stack);
	h = StackPeek(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	if (n <= 0) {
		ipReverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = calloc_nogc(n, sizeof(char));
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

		for (fungeCell i = 0; i < n; i++) {
			FungeSpaceSet(buf[i], VectorCreateRef(handles[h]->buffvect.x + i, handles[h]->buffvect.y));
		}
		free_nogc(buf);
	}
}

/// S - Seek to position in file
static void FingerFILEfseek(instructionPointer * ip)
{
	fungeCell n, m, h;

	n = StackPop(ip->stack);
	m = StackPop(ip->stack);
	h = StackPeek(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

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

/// W - Write n bytes from i/o buffer to file
static void FingerFILEfwrite(instructionPointer * ip)
{
	fungeCell n, h;

	n = StackPop(ip->stack);
	h = StackPeek(ip->stack);

	if(!ValidHandle(h)) {
		ipReverse(ip);
		return;
	}

	if (n <= 0) {
		ipReverse(ip);
		return;
	} else {
		FILE * fp = handles[h]->file;
		unsigned char * restrict buf = cf_malloc_noptr(n * sizeof(char));

		for (fungeCell i = 0; i < n; i++) {
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

FUNGE_ATTR_FAST static inline bool InitHandleList(void)
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
	ManagerAddOpcode(FILE,  'D', delete)
	ManagerAddOpcode(FILE,  'G', fgets)
	ManagerAddOpcode(FILE,  'L', ftell)
	ManagerAddOpcode(FILE,  'O', fopen)
	ManagerAddOpcode(FILE,  'P', fputs)
	ManagerAddOpcode(FILE,  'R', fread)
	ManagerAddOpcode(FILE,  'S', fseek)
	ManagerAddOpcode(FILE,  'W', fwrite)
	return true;
}

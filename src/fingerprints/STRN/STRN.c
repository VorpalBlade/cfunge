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

#include "STRN.h"
#include "../../stack.h"

#include <string.h>
#include "../../../lib/stringbuffer/stringbuffer.h"

/// A - Append bottom string to upper string
static void FingerSTRNappend(instructionPointer * ip)
{
	char * restrict top;
	char * restrict bottom;
	char * restrict c;
	top = StackPopString(ip->stack);
	bottom = StackPopString(ip->stack);
	c = cf_calloc_noptr(strlen(top) + strlen(bottom) + 1, sizeof(char));
	strcat(c, top);
	strcat(c, bottom);
	StackPushString(ip->stack, c, strlen(c));
	StackFreeString(top);
	StackFreeString(bottom);
	cf_free(c);
}

/// C - Compare strings
static void FingerSTRNcompare(instructionPointer * ip)
{
	char * restrict a;
	char * restrict b;
	a = StackPopString(ip->stack);
	b = StackPopString(ip->stack);
	StackPush(ip->stack, strcmp(a, b));
	StackFreeString(a);
	StackFreeString(b);
}

/// D - Display a string
static void FingerSTRNdisplay(instructionPointer * ip)
{
	char * restrict s;
	s = StackPopString(ip->stack);
	fputs(s, stdout);
	StackFreeString(s);
}

/// F - Search for bottom string in upper string
static void FingerSTRNsearch(instructionPointer * ip)
{
	char * top;
	char * restrict bottom;
	char * c;
	top = StackPopString(ip->stack);
	bottom = StackPopString(ip->stack);
	c = strstr(top, bottom);
	if (c) {
		StackPushString(ip->stack, c, strlen(c));
	} else {
		StackPush(ip->stack, '\0');
	}
	StackFreeString(top);
	StackFreeString(bottom);
}

/// G - Get string from specified position
static void FingerSTRNget(instructionPointer * ip)
{
	fungeRect bounds;
	StringBuffer *sb;
	char *s;
	fungeVector pos;

	FungeSpaceGetBoundRect(&bounds);
	pos = StackPopVector(ip->stack);
	if (pos.y < bounds.y || pos.y > bounds.y + bounds.h) {
		ipReverse(ip);
		return;
	}
	sb = stringbuffer_new();
	
	while (true) {
		FUNGEDATATYPE val;
		val = FungeSpaceGet(&pos);
		stringbuffer_append_char(sb, val);
		if (pos.x < bounds.x || pos.x > bounds.x + bounds.w) {
			stringbuffer_destroy(sb);
			ipReverse(ip);
			return;
		}
		if (val == 0) break;
		pos.x += 1;
	}
	s = stringbuffer_finish(sb);
	if (!s) {
		stringbuffer_destroy(sb);
		ipReverse(ip);
		return;
	}
	StackPushString(ip->stack, s, strlen(s));
	free_nogc(s);
}

/// I - Input a string
static void FingerSTRNinput(instructionPointer * ip)
{
	char * line = NULL;
	char * newline;
	size_t len = 0;
	ssize_t retval = cf_getline(&line, &len, stdin);
	if (retval == -1) {
		ipReverse(ip);
		return;
	}
	// Discard any trailing newline.
	newline = strrchr(line, '\n');
	if (newline)
		newline[0] = '\0';
	StackPushString(ip->stack, line, strlen(line));
}

/// L - Leftmost n characters of string
static void FingerSTRNleft(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	size_t len;
	char *s;
	n = StackPop(ip->stack);
	s = StackPopString(ip->stack);
	len = strlen(s);
	if (n < 0 || len < (size_t)n) {
		StackFreeString(s);
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, '\0');
	StackPushString(ip->stack, s, n-1);
	StackFreeString(s);
}

/// M - n characters starting at position p
static void FingerSTRNslice(instructionPointer * ip)
{
	FUNGEDATATYPE n, p;
	char *s;
	n = StackPop(ip->stack);
	p = StackPop(ip->stack);
	s = StackPopString(ip->stack);
	if (p < 0 || n < 0) {
		StackFreeString(s);
		ipReverse(ip);
		return;
	}
	if (strlen(s) < (size_t)(p+n)) {
		StackFreeString(s);
		ipReverse(ip);
		return;
	}
	s[p+n] = '\0';
	StackPushString(ip->stack, s+p, strlen(s+p));
	StackFreeString(s);
}

/// N - Get length of string
static void FingerSTRNlength(instructionPointer * ip)
{
	char * restrict s;
	size_t len;
	s = StackPopString(ip->stack);
	len = strlen(s);
	StackPushString(ip->stack, s, len);
	StackPush(ip->stack, len);
	StackFreeString(s);
}

/// P - Put string at specified position
static void FingerSTRNput(instructionPointer * ip)
{
	char *s;
	fungeVector pos;
	size_t len;

	pos = StackPopVector(ip->stack);
	s   = StackPopString(ip->stack);
	len = strlen(s);
	
	for (size_t i = 0; i < len + 1; i++) {
		FungeSpaceSet(s[i], &pos);
		pos.x += 1;
	}
	StackFreeString(s);
}

/// R - Rightmost n characters of string
static void FingerSTRNright(instructionPointer * ip)
{
	FUNGEDATATYPE n;
	size_t len;
	char *s;
	n = StackPop(ip->stack);
	s = StackPopString(ip->stack);
	len = strlen(s);
	if (n < 0 || len < (size_t)n) {
		StackFreeString(s);
		ipReverse(ip);
		return;
	}
	StackPushString(ip->stack, s+(len-n), n);
	StackFreeString(s);
}

/// S - String representation of a number
static void FingerSTRNitoa(instructionPointer * ip)
{
	char *s;
	FUNGEDATATYPE n = StackPop(ip->stack);
	StringBuffer *sb = stringbuffer_new();

	stringbuffer_append_printf(sb, "%" FUNGEDATAPRI, n);
	s = stringbuffer_finish(sb);
	StackPushString(ip->stack, s, strlen(s));
	free_nogc(s);
}

/// V - Retrieve value from string
static void FingerSTRNatoi(instructionPointer * ip)
{
	char *s;
	s = StackPopString(ip->stack);
	StackPush(ip->stack, atoi(s));
	StackFreeString(s);
}

bool FingerSTRNload(instructionPointer * ip)
{
	ManagerAddOpcode(STRN,  'A', append)
	ManagerAddOpcode(STRN,  'C', compare)
	ManagerAddOpcode(STRN,  'D', display)
	ManagerAddOpcode(STRN,  'F', search)
	ManagerAddOpcode(STRN,  'G', get)
	ManagerAddOpcode(STRN,  'I', input)
	ManagerAddOpcode(STRN,  'L', left)
	ManagerAddOpcode(STRN,  'M', slice)
	ManagerAddOpcode(STRN,  'N', length)
	ManagerAddOpcode(STRN,  'P', put)
	ManagerAddOpcode(STRN,  'R', right)
	ManagerAddOpcode(STRN,  'S', itoa)
	ManagerAddOpcode(STRN,  'V', atoi)
	return true;
}

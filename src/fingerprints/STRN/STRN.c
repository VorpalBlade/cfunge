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
#include "../../input.h"

#include <string.h>
#include "../../../lib/stringbuffer/stringbuffer.h"

/// A - Append bottom string to upper string
static void FingerSTRNappend(instructionPointer * ip)
{
	char * restrict top;
	char * restrict bottom;
	char * restrict c;
	top = stack_pop_string(ip->stack);
	bottom = stack_pop_string(ip->stack);
	c = calloc_nogc(strlen(top) + strlen(bottom) + 1, sizeof(char));
	strcat(c, top);
	strcat(c, bottom);
	stack_push_string(ip->stack, c, strlen(c));
	stack_freeString(top);
	stack_freeString(bottom);
	free_nogc(c);
}

/// C - Compare strings
static void FingerSTRNcompare(instructionPointer * ip)
{
	char * restrict a;
	char * restrict b;
	a = stack_pop_string(ip->stack);
	b = stack_pop_string(ip->stack);
	stack_push(ip->stack, strcmp(a, b));
	stack_freeString(a);
	stack_freeString(b);
}

/// D - Display a string
static void FingerSTRNdisplay(instructionPointer * ip)
{
	char * restrict s;
	s = stack_pop_string(ip->stack);
	fputs(s, stdout);
	stack_freeString(s);
}

/// F - Search for bottom string in upper string
static void FingerSTRNsearch(instructionPointer * ip)
{
	char * top;
	char * restrict bottom;
	char * c;
	top = stack_pop_string(ip->stack);
	bottom = stack_pop_string(ip->stack);
	c = strstr(top, bottom);
	if (c) {
		stack_push_string(ip->stack, c, strlen(c));
	} else {
		stack_push(ip->stack, '\0');
	}
	stack_freeString(top);
	stack_freeString(bottom);
}

/// G - Get string from specified position
static void FingerSTRNget(instructionPointer * ip)
{
	fungeRect bounds;
	StringBuffer *sb;
	char *s;
	fungeVector pos;

	fungespace_get_bounds_rect(&bounds);
	pos = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;
	if (pos.y < bounds.y || pos.y > bounds.y + bounds.h) {
		ip_reverse(ip);
		return;
	}
	sb = stringbuffer_new();

	while (true) {
		fungeCell val;
		val = fungespace_get(&pos);
		stringbuffer_append_char(sb, val);
		if (pos.x < bounds.x || pos.x > bounds.x + bounds.w) {
			stringbuffer_destroy(sb);
			ip_reverse(ip);
			return;
		}
		if (val == 0) break;
		pos.x += 1;
	}
	s = stringbuffer_finish(sb);
	if (!s) {
		stringbuffer_destroy(sb);
		ip_reverse(ip);
		return;
	}
	stack_push_string(ip->stack, s, strlen(s));
	free_nogc(s);
}

/// I - Input a string
static void FingerSTRNinput(instructionPointer * ip)
{
	char * line = NULL;
	char * newline;
	bool retval = input_getline(&line);
	if (retval == false || line == NULL) {
		ip_reverse(ip);
		return;
	}
	// Discard any trailing newline.
	newline = strrchr(line, '\n');
	if (newline)
		newline[0] = '\0';
	stack_push_string(ip->stack, line, strlen(line));
	cf_free(line);
}

/// L - Leftmost n characters of string
static void FingerSTRNleft(instructionPointer * ip)
{
	fungeCell n;
	size_t len;
	char *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string(ip->stack);
	len = strlen(s);
	if (n < 0 || len < (size_t)n) {
		stack_freeString(s);
		ip_reverse(ip);
		return;
	}
	stack_push(ip->stack, '\0');
	stack_push_string(ip->stack, s, n - 1);
	stack_freeString(s);
}

/// M - n characters starting at position p
static void FingerSTRNslice(instructionPointer * ip)
{
	fungeCell n, p;
	char *s;
	n = stack_pop(ip->stack);
	p = stack_pop(ip->stack);
	s = stack_pop_string(ip->stack);
	if (p < 0 || n < 0) {
		stack_freeString(s);
		ip_reverse(ip);
		return;
	}
	if (strlen(s) < (size_t)(p + n)) {
		stack_freeString(s);
		ip_reverse(ip);
		return;
	}
	s[p+n] = '\0';
	stack_push_string(ip->stack, s + p, strlen(s + p));
	stack_freeString(s);
}

/// N - Get length of string
static void FingerSTRNlength(instructionPointer * ip)
{
	char * restrict s;
	size_t len;
	s = stack_pop_string(ip->stack);
	len = strlen(s);
	stack_push_string(ip->stack, s, len);
	stack_push(ip->stack, len);
	stack_freeString(s);
}

/// P - Put string at specified position
static void FingerSTRNput(instructionPointer * ip)
{
	char *s;
	fungeVector pos;
	size_t len;

	pos = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;
	s   = stack_pop_string(ip->stack);
	len = strlen(s);

	for (size_t i = 0; i < len + 1; i++) {
		fungespace_set(s[i], &pos);
		pos.x += 1;
	}
	stack_freeString(s);
}

/// R - Rightmost n characters of string
static void FingerSTRNright(instructionPointer * ip)
{
	fungeCell n;
	size_t len;
	char *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string(ip->stack);
	len = strlen(s);
	if (n < 0 || len < (size_t)n) {
		stack_freeString(s);
		ip_reverse(ip);
		return;
	}
	stack_push_string(ip->stack, s + (len - n), n);
	stack_freeString(s);
}

/// S - String representation of a number
static void FingerSTRNitoa(instructionPointer * ip)
{
	char *s;
	fungeCell n = stack_pop(ip->stack);
	StringBuffer *sb = stringbuffer_new();

	stringbuffer_append_printf(sb, "%" FUNGECELLPRI, n);
	s = stringbuffer_finish(sb);
	stack_push_string(ip->stack, s, strlen(s));
	free_nogc(s);
}

/// V - Retrieve value from string
static void FingerSTRNatoi(instructionPointer * ip)
{
	char *s;
	s = stack_pop_string(ip->stack);
	stack_push(ip->stack, atoi(s));
	stack_freeString(s);
}

bool FingerSTRNload(instructionPointer * ip)
{
	manager_add_opcode(STRN,  'A', append)
	manager_add_opcode(STRN,  'C', compare)
	manager_add_opcode(STRN,  'D', display)
	manager_add_opcode(STRN,  'F', search)
	manager_add_opcode(STRN,  'G', get)
	manager_add_opcode(STRN,  'I', input)
	manager_add_opcode(STRN,  'L', left)
	manager_add_opcode(STRN,  'M', slice)
	manager_add_opcode(STRN,  'N', length)
	manager_add_opcode(STRN,  'P', put)
	manager_add_opcode(STRN,  'R', right)
	manager_add_opcode(STRN,  'S', itoa)
	manager_add_opcode(STRN,  'V', atoi)
	return true;
}

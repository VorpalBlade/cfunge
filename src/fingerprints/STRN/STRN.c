/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
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

#include "STRN.h"
#include "../../stack.h"
#include "../../input.h"

#include <string.h>
#include "../../../lib/stringbuffer/stringbuffer.h"

/// A - Append bottom string to upper string
static void finger_STRN_append(instructionPointer * ip)
{
	char * top;
	char * restrict bottom;
	char * c = NULL;
	size_t top_len;
	size_t bottom_len;

	top = (char*)stack_pop_string(ip->stack);
	bottom =  (char*)stack_pop_string(ip->stack);
	top_len = strlen(top);
	bottom_len = strlen(bottom);

	c = cf_realloc(top, top_len + strlen(bottom) + 1);
	if (!c) {
		ip_reverse(ip);
		stack_free_string(top);
		stack_free_string(bottom);
		return;
	}
	memcpy(c + top_len, bottom, bottom_len);
	c[top_len + bottom_len] = '\0';

	stack_push_string(ip->stack, (unsigned char*)c, strlen(c));

	stack_free_string(bottom);
	cf_free(c);
}

/// C - Compare strings
static void finger_STRN_compare(instructionPointer * ip)
{
	unsigned char * restrict a;
	unsigned char * restrict b;
	a = stack_pop_string(ip->stack);
	b = stack_pop_string(ip->stack);
	stack_push(ip->stack, strcmp((char*)a, (char*)b));
	stack_free_string(a);
	stack_free_string(b);
}

/// D - Display a string
static void finger_STRN_display(instructionPointer * ip)
{
	unsigned char * restrict s;
	s = stack_pop_string(ip->stack);
	fputs((char*)s, stdout);
	stack_free_string(s);
}

/// F - Search for bottom string in upper string
static void finger_STRN_search(instructionPointer * ip)
{
	unsigned char * top;
	unsigned char * restrict bottom;
	unsigned char * c;
	top = stack_pop_string(ip->stack);
	bottom = stack_pop_string(ip->stack);
	c = (unsigned char*)strstr((char*)top, (char*)bottom);
	if (c) {
		stack_push_string(ip->stack, c, strlen((char*)c));
	} else {
		stack_push(ip->stack, '\0');
	}
	stack_free_string(top);
	stack_free_string(bottom);
}

/// G - Get string from specified position
static void finger_STRN_get(instructionPointer * ip)
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
	stack_push_string(ip->stack, (unsigned char*)s, strlen(s));
	free_nogc(s);
}

/// I - Input a string
static void finger_STRN_input(instructionPointer * ip)
{
	unsigned char * line = NULL;
	unsigned char * newline;
	bool retval = input_getline(&line);
	if (retval == false || line == NULL) {
		ip_reverse(ip);
		return;
	}
	// Discard any trailing newline.
	newline = (unsigned char*)strrchr((char*)line, '\n');
	if (newline)
		newline[0] = '\0';
	stack_push_string(ip->stack, line, strlen((char*)line));
	cf_free(line);
}

/// L - Leftmost n characters of string
static void finger_STRN_left(instructionPointer * ip)
{
	fungeCell n;
	size_t len;
	unsigned char *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string(ip->stack);
	len = strlen((char*)s);
	if (n <= 0 || len < (size_t)n) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	stack_push(ip->stack, '\0');
	stack_push_string(ip->stack, s, (size_t)(n - 1));
	stack_free_string(s);
}

/// M - n characters starting at position p
static void finger_STRN_slice(instructionPointer * ip)
{
	fungeCell n, p;
	char *s;
	n = stack_pop(ip->stack);
	p = stack_pop(ip->stack);
	s = (char*)stack_pop_string(ip->stack);
	if (p < 0 || n < 0) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	if (strlen(s) < (size_t)(p + n)) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	s[p+n] = '\0';
	stack_push_string(ip->stack, (unsigned char*)s + p, strlen(s + p));
	stack_free_string(s);
}

/// N - Get length of string
static void finger_STRN_length(instructionPointer * ip)
{
	unsigned char * restrict s;
	size_t len;
	s = stack_pop_string(ip->stack);
	len = strlen((char*)s);
	stack_push_string(ip->stack, s, len);
	stack_push(ip->stack, (fungeCell)len);
	stack_free_string(s);
}

/// P - Put string at specified position
static void finger_STRN_put(instructionPointer * ip)
{
	fungeCell value;
	fungeVector pos;

	pos = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;

	// This doesn't cast to char, but is faster and uses less memory.
	do {
		value = stack_pop(ip->stack);
		fungespace_set(value, &pos);
		pos.x += 1;
	} while (value != 0);
}

/// R - Rightmost n characters of string
static void finger_STRN_right(instructionPointer * ip)
{
	fungeCell n;
	size_t len;
	unsigned char *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string(ip->stack);
	len = strlen((char*)s);
	if (n < 0 || len < (size_t)n) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	stack_push_string(ip->stack, s + (len - (size_t)n), (size_t)n);
	stack_free_string(s);
}

/// S - String representation of a number
static void finger_STRN_itoa(instructionPointer * ip)
{
	char *s;
	fungeCell n = stack_pop(ip->stack);
	StringBuffer *sb = stringbuffer_new();

	stringbuffer_append_printf(sb, "%" FUNGECELLPRI, n);
	s = stringbuffer_finish(sb);
	stack_push_string(ip->stack, (unsigned char*)s, strlen(s));
	free_nogc(s);
}

/// V - Retrieve value from string
static void finger_STRN_atoi(instructionPointer * ip)
{
	unsigned char *s;
	s = stack_pop_string(ip->stack);
	stack_push(ip->stack, atoi((char*)s));
	stack_free_string(s);
}

bool finger_STRN_load(instructionPointer * ip)
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

/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

#include <stdlib.h> /* atoi */
#include <string.h>
#include <assert.h>

#include "../../../lib/stringbuffer/stringbuffer.h"
#include "../../../lib/fungestring/funge_string.h"

/// A - Append bottom string to upper string
static void finger_STRN_append(instructionPointer * ip)
{
	funge_cell * top;
	funge_cell * restrict bottom;
	funge_cell * c = NULL;
	size_t top_len;
	size_t bottom_len;

	top = stack_pop_string_multibyte(ip->stack, &top_len);
	bottom = stack_pop_string_multibyte(ip->stack, &bottom_len);
	if (FUNGE_UNLIKELY(!top || !bottom)) {
		goto error;
	}

	c = realloc(top, (top_len + bottom_len + 1) * sizeof(funge_cell));
	if (FUNGE_UNLIKELY(!c)) {
		goto error;
	}
	memcpy(c + top_len, bottom, bottom_len * sizeof(funge_cell));
	c[top_len + bottom_len] = '\0';

	stack_push_string_multibyte(ip->stack, c, bottom_len + top_len);

	stack_free_string(bottom);
	free(c);
	return;
error:
	ip_reverse(ip);
	if (top)
		stack_free_string(top);
	if (bottom)
		stack_free_string(bottom);
}

/// C - Compare strings
static void finger_STRN_compare(instructionPointer * ip)
{
	funge_cell * restrict a;
	funge_cell * restrict b;
	size_t alen = 0, blen = 0, minlen;
	funge_cell comparsion = 0;

	a = stack_pop_string_multibyte(ip->stack, &alen);
	b = stack_pop_string_multibyte(ip->stack, &blen);

	if (FUNGE_UNLIKELY(!a || !b)) {
		// Ok even if NULL.
		stack_free_string(a);
		stack_free_string(b);
		ip_reverse(ip);
		return;
	}

	minlen = (alen < blen) ? alen : blen;

	for (size_t i = 0; i < minlen + 1; i++) {
		funge_cell diff = a[i] - b[i];
		if (diff != 0) {
			comparsion = diff;
			break;
		}
	}

	stack_push(ip->stack, comparsion);
	stack_free_string(a);
	stack_free_string(b);
}

/// D - Display a string
static void finger_STRN_display(instructionPointer * ip)
{
	unsigned char * restrict s;
	s = stack_pop_string(ip->stack, NULL);
	if (FUNGE_UNLIKELY(!s)) {
		ip_reverse(ip);
		return;
	}
	fputs((char*)s, stdout);
	stack_free_string(s);
}

/// F - Search for bottom string in upper string
static void finger_STRN_search(instructionPointer * ip)
{
	funge_cell * top;
	funge_cell * restrict bottom;
	funge_cell * c;
	top = stack_pop_string_multibyte(ip->stack, NULL);
	bottom = stack_pop_string_multibyte(ip->stack, NULL);
	if (FUNGE_UNLIKELY(!top || !bottom)) {
		// Ok even if NULL.
		stack_free_string(top);
		stack_free_string(bottom);
		ip_reverse(ip);
		return;
	}
	c = funge_strstr(top, bottom);
	if (c) {
		stack_push_string_multibyte(ip->stack, c, funge_strlen(c));
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
	size_t len;
	funge_cell *s;
	funge_vector pos;

	fungespace_get_bounds_rect(&bounds);
	pos = stack_pop_vector(ip->stack);
	pos.x += ip->storageOffset.x;
	pos.y += ip->storageOffset.y;
	if (pos.y < bounds.y || pos.y > bounds.y + bounds.h) {
		ip_reverse(ip);
		return;
	}

	sb = stringbuffer_new();
	if (FUNGE_UNLIKELY(!sb)) {
		ip_reverse(ip);
		return;
	}

	while (true) {
		funge_cell val;
		val = fungespace_get(&pos);
		if (val == 0) break;
		stringbuffer_append_cell(sb, val);
		if (pos.x < bounds.x || pos.x > bounds.x + bounds.w) {
			stringbuffer_destroy(sb);
			ip_reverse(ip);
			return;
		}
		pos.x += 1;
	}
	s = stringbuffer_finish_multibyte(sb, &len);
	if (FUNGE_UNLIKELY(!s)) {
		stringbuffer_destroy(sb);
		ip_reverse(ip);
		return;
	}
	stack_push_string_multibyte(ip->stack, s, len);
	free(s);
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
	free(line);
}

/// L - Leftmost n characters of string
static void finger_STRN_left(instructionPointer * ip)
{
	funge_cell n;
	size_t len;
	funge_cell *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string_multibyte(ip->stack, &len);
	if (n < 0) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	if (n == 0) {
		stack_push(ip->stack, '\0');
		stack_free_string(s);
		return;
	}
	if (len < (size_t)n) {
		n = len;
	}
	stack_push(ip->stack, '\0');
	stack_push_string_multibyte(ip->stack, s, (size_t)(n - 1));
	stack_free_string(s);
}

/// M - n characters starting at position p
static void finger_STRN_slice(instructionPointer * ip)
{
	funge_cell n, p;
	funge_cell *s;
	size_t slen;
	n = stack_pop(ip->stack);
	p = stack_pop(ip->stack);
	s = stack_pop_string_multibyte(ip->stack, &slen);
	if (p < 0 || n < 0 || slen < (size_t)p) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	if (slen < (size_t)(p + n)) {
		n = slen - p;
	}
	s[p + n] = '\0';
	stack_push_string_multibyte(ip->stack, s + p, slen - p);
	stack_free_string(s);
}

/// N - Get length of string
static void finger_STRN_length(instructionPointer * ip)
{
	stack_push(ip->stack, (funge_cell)stack_strlen(ip->stack));
}

/// P - Put string at specified position
static void finger_STRN_put(instructionPointer * ip)
{
	funge_cell value;
	funge_vector pos;

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
	funge_cell n;
	size_t len;
	funge_cell *s;
	n = stack_pop(ip->stack);
	s = stack_pop_string_multibyte(ip->stack, &len);
	if (n < 0) {
		stack_free_string(s);
		ip_reverse(ip);
		return;
	}
	if (len < (size_t)n) {
		n = len;
	}
	stack_push_string_multibyte(ip->stack, s + (len - (size_t)n), (size_t)n);
	stack_free_string(s);
}

/// S - String representation of a number
static void finger_STRN_itoa(instructionPointer * ip)
{
	char *s;
	size_t len;
	funge_cell n = stack_pop(ip->stack);
	StringBuffer *sb = stringbuffer_new();
	if (FUNGE_UNLIKELY(!sb)) {
		ip_reverse(ip);
		return;
	}
	stringbuffer_append_printf(sb, "%" FUNGECELLPRI, n);
	s = stringbuffer_finish(sb, &len);
	assert(len == strlen(s));
	stack_push_string(ip->stack, (unsigned char*)s, len);
	free(s);
}

/// V - Retrieve value from string
static void finger_STRN_atoi(instructionPointer * ip)
{
	unsigned char *s;
	s = stack_pop_string(ip->stack, NULL);
	if (FUNGE_UNLIKELY(!s)) {
		ip_reverse(ip);
		return;
	}
	stack_push(ip->stack, FUNGE_ATOI((char*)s));
	stack_free_string(s);
}

bool finger_STRN_load(instructionPointer * ip)
{
	manager_add_opcode(STRN, 'A', append);
	manager_add_opcode(STRN, 'C', compare);
	manager_add_opcode(STRN, 'D', display);
	manager_add_opcode(STRN, 'F', search);
	manager_add_opcode(STRN, 'G', get);
	manager_add_opcode(STRN, 'I', input);
	manager_add_opcode(STRN, 'L', left);
	manager_add_opcode(STRN, 'M', slice);
	manager_add_opcode(STRN, 'N', length);
	manager_add_opcode(STRN, 'P', put);
	manager_add_opcode(STRN, 'R', right);
	manager_add_opcode(STRN, 'S', itoa);
	manager_add_opcode(STRN, 'V', atoi);
	return true;
}

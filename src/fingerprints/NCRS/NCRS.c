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

#define FUNGE_EXTENDS_NCRS
#define FUNGE_EXTENDS_TERM
#include "NCRS.h"

#if defined(HAVE_NCURSES)
#include "../../stack.h"

#include <stdio.h>

#include "../TERM/TERM.h"

#ifdef NCURSES_H_IN_NCURSES
#  include <ncurses/ncurses.h>
#else
#  include <ncurses.h>
#endif
#ifdef TERM_H_IN_NCURSES
#  include <ncurses/term.h>
#else
#  include <term.h>
#endif

#if defined(__OpenBSD__)
// OpenBSD's ncurses sucks hard, it redefines bool.
#  undef bool
#  define bool _Bool
#endif

#define NCRS_VALIDATE_STATE_CLEANUP(_c) if (!ncrs_valid_state) { ip_reverse(ip); _c; return; }
#define NCRS_VALIDATE_STATE() if (!ncrs_valid_state) { ip_reverse(ip); return; }

/// Defines if we have ever ncrs_initialised.
static bool ncrs_initialised = false;
/// Defines if we have a valid ncrs_initialised state.
static bool ncrs_valid_state = false;
static SCREEN* ncrs_screen = NULL;
static WINDOW* ncrs_window = NULL;

/// For use from TERM
FUNGE_ATTR_FAST FUNGE_ATTR_PURE
bool finger_NCRS_need_setupterm(void)
{
	return !ncrs_valid_state;
}

/// B - Beep
static void finger_NCRS_beep(instructionPointer * ip)
{
	NCRS_VALIDATE_STATE();
	if (beep() == ERR)
		ip_reverse(ip);
}

/// C - Clear all or part of the screen
static void finger_NCRS_clear(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	switch (value) {
		case 0:
			if (werase(ncrs_window) == ERR)
				ip_reverse(ip);
			return;
		case 1:
			if (wclrtoeol(ncrs_window) == ERR)
				ip_reverse(ip);
			return;
		case 2:
			if (wclrtobot(ncrs_window) == ERR)
				ip_reverse(ip);
			return;
		default:
			ip_reverse(ip);
	}
}

/// E - Set echo mode
static void finger_NCRS_toggle_echo(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	switch (value) {
		case 0:
			if (noecho() == ERR)
				ip_reverse(ip);
			return;
		case 1:
			if (echo() == ERR)
				ip_reverse(ip);
			return;
		default:
			ip_reverse(ip);
	}
}

/// G - Get character
static void finger_NCRS_get(instructionPointer * ip)
{
	funge_cell value;
	NCRS_VALIDATE_STATE();
	if ((value = wgetch(ncrs_window)) == ERR)
		ip_reverse(ip);
	else
		stack_push(ip->stack, (funge_cell)value);
}

/// I - Initialise and end curses mode
static void finger_NCRS_init(instructionPointer * ip)
{
	if (stack_pop(ip->stack) == 1) {
		// If TERM was used before, check to make sure we don't get a mem
		// leak:
		finger_TERM_fix_before_NCRS_init();
		ncrs_screen = newterm(NULL, stdout, stdin);
		if (!ncrs_screen)
			goto error;
		set_term(ncrs_screen);
		ncrs_window = newwin(0, 0, 0, 0);
		if (!ncrs_window)
			goto error;
		ncrs_initialised = true;
		ncrs_valid_state = true;
	} else {
		if (!ncrs_initialised)
			goto error;
		if (endwin() == ERR)
			goto error;
		if (delwin(ncrs_window) == ERR)
			goto error;
		delscreen(ncrs_screen);
		ncrs_screen = NULL;
		ncrs_window = NULL;
		ncrs_valid_state = false;
		// If TERM was used before, fix up the issues:
		finger_TERM_fix_after_NCRS_teardown();
	}
	return;
error:
	ip_reverse(ip);
	ncrs_valid_state = false;
}

/// K - Set keypad mode
static void finger_NCRS_toggle_keypad(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	switch (value) {
		case 0:
			if (keypad(stdscr, false) == ERR)
				ip_reverse(ip);
			return;
		case 1:
			if (keypad(stdscr, true) == ERR)
				ip_reverse(ip);
			return;
		default:
			ip_reverse(ip);
	}
}

/// M - Move cursor to x,y
static void finger_NCRS_goto_xy(instructionPointer * ip)
{
	funge_vector v = stack_pop_vector(ip->stack);
	NCRS_VALIDATE_STATE();
	if (wmove(ncrs_window, (int)v.y, (int)v.x) == ERR)
		ip_reverse(ip);
}

/// N - Toggle input mode
static void finger_NCRS_toggle_input(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	switch (value) {
		case 0:
			if (cbreak() == ERR)
				ip_reverse(ip);
			return;
		case 1:
			if (nocbreak() == ERR)
				ip_reverse(ip);
			return;
		default:
			ip_reverse(ip);
	}
}

/// P - Put the character at cursor
static void finger_NCRS_put(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	if (waddch(ncrs_window, (chtype)value) == ERR)
		ip_reverse(ip);
}

/// R - Refresh window
static void finger_NCRS_refresh(instructionPointer * ip)
{
	NCRS_VALIDATE_STATE();
	if (wrefresh(ncrs_window) == ERR)
		ip_reverse(ip);
}

/// S - Write string at cursor
static void finger_NCRS_write(instructionPointer * ip)
{
	unsigned char* str = stack_pop_string(ip->stack, NULL);
	NCRS_VALIDATE_STATE_CLEANUP(stack_free_string(str));
	if (waddstr(ncrs_window, (char*)str) == ERR)
		ip_reverse(ip);
	stack_free_string(str);
}

/// U - Unget character
static void finger_NCRS_unget(instructionPointer * ip)
{
	funge_cell value = stack_pop(ip->stack);
	NCRS_VALIDATE_STATE();
	if (ungetch((chtype)value) == ERR)
		ip_reverse(ip);
}

bool finger_NCRS_load(instructionPointer * ip)
{
	manager_add_opcode(NCRS, 'B', beep);
	manager_add_opcode(NCRS, 'C', clear);
	manager_add_opcode(NCRS, 'E', toggle_echo);
	manager_add_opcode(NCRS, 'G', get);
	manager_add_opcode(NCRS, 'I', init);
	manager_add_opcode(NCRS, 'K', toggle_keypad);
	manager_add_opcode(NCRS, 'M', goto_xy);
	manager_add_opcode(NCRS, 'N', toggle_input);
	manager_add_opcode(NCRS, 'P', put);
	manager_add_opcode(NCRS, 'R', refresh);
	manager_add_opcode(NCRS, 'S', write);
	manager_add_opcode(NCRS, 'U', unget);
	return true;
}
#endif /* defined(HAVE_NCURSES) */

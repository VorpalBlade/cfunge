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
#include "TERM.h"

#if defined(HAVE_NCURSES)

#include "../../stack.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../NCRS/NCRS.h"

#ifdef CURSES_H_IN_NCURSES
#  include <ncurses/curses.h>
#else
#  include <curses.h>
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

// Define this if you want the correct but uggly
// use of enter_ca_mode/exit_ca_mode. It makes
// everything look uggly in xterms at least!
#define TERM_CAP_CORRECT

static bool term_initialised = false;

#define valid(s) (((s) != 0) && (s) != (char *)-1)

/// C - Clear screen
static void finger_TERM_clear_screen(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clear_screen);
}

/// D - Move cursor down n lines
static void finger_TERM_go_down(instructionPointer * ip)
{
	funge_cell n = stack_pop(ip->stack);
	if (n == 0) {
		return;
	}
	if (n < 0) {
		while (n++)
			putp(cursor_up);
	} else {
		while (n--)
			putp(cursor_down);
	}
}

/// G - Goto cursor position x,y (home is 0,0)
static void finger_TERM_goto_xy(instructionPointer * ip)
{
	char *s;
	funge_cell x, y;
	x = stack_pop(ip->stack);
	y = stack_pop(ip->stack);
	s = tparm(cursor_address, (int)x, (int)y, 0, 0, 0, 0, 0, 0, 0);
	if (!valid(s)) {
		ip_reverse(ip);
		return;
	}
	putp(s);
}

/// H - Move cursor to home
static void finger_TERM_go_home(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(cursor_home);
}

/// L - Clear from cursor to end of line
static void finger_TERM_clear_to_eol(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clr_eol);
}

/// S - Clear from cursor to end of screen
static void finger_TERM_clear_to_eos(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clr_eos);
}

/// U - Move cursor up n lines
static void finger_TERM_go_up(instructionPointer * ip)
{
	funge_cell n = stack_pop(ip->stack);
	if (n == 0) {
		return;
	}
	if (n < 0) {
		while (n++)
			putp(cursor_down);
	} else {
		while (n--)
			putp(cursor_up);
	}
}

#ifdef TERM_CAP_CORRECT
static void finalise(void)
{
	int errret;
	if (!term_initialised)
		return;
	// Check that cur_term is valid, it may not be after using NCRS.
	// If it isn't valid, try to re-set it.
	if (!cur_term)
		if (setupterm(NULL, STDOUT_FILENO, &errret) != OK && errret <= 0)
			return;
	// Make some static analysers less confused.
	assert(cur_term != NULL);
	putp(exit_ca_mode);
	del_curterm(cur_term);
}
#endif

FUNGE_ATTR_FAST
static inline bool initialise(void)
{
	int errret;
	if (term_initialised)
		return true;

	if (finger_NCRS_need_setupterm()) {
		if (setupterm(NULL, STDOUT_FILENO, &errret) != OK && errret <= 0)
			return false;
	}
#ifdef TERM_CAP_CORRECT
	putp(enter_ca_mode);
	atexit(finalise);
#endif
	term_initialised = true;
	return true;
}

FUNGE_ATTR_FAST
void finger_TERM_fix_before_NCRS_init(void)
{
	if (!term_initialised)
		return;
	del_curterm(cur_term);
}

FUNGE_ATTR_FAST
void finger_TERM_fix_after_NCRS_teardown(void)
{
	int errret;
	if (!term_initialised)
		return;
	// Now terminal is potentially invalid, redo setupterm()
	if (setupterm(NULL, STDOUT_FILENO, &errret) != OK && errret <= 0)
		return;
}


bool finger_TERM_load(instructionPointer * ip)
{
	if (!initialise())
		return false;
	manager_add_opcode(TERM, 'C', clear_screen);
	manager_add_opcode(TERM, 'D', go_down);
	manager_add_opcode(TERM, 'G', goto_xy);
	manager_add_opcode(TERM, 'H', go_home);
	manager_add_opcode(TERM, 'L', clear_to_eol);
	manager_add_opcode(TERM, 'S', clear_to_eos);
	manager_add_opcode(TERM, 'U', go_up);
	return true;
}

#endif /* defined(HAVE_NCURSES) */

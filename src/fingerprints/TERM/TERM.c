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

#include "TERM.h"
#include "../../stack.h"
#include <stdlib.h>
#include <stdio.h>

#include <curses.h>
#include <term.h>

// Define this if you want the correct but uggly
// use of enter_ca_mode/exit_ca_mode. It makes
// everything look uggly in xterms at least!
#define TERM_CAP_CORRECT

static bool initialised = false;

#define valid(s) ((s != 0) && s != (char *)-1)

/// C - Clear screen
static void FingerTERMclearScreen(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clear_screen);
}

/// D - Move cursor down n lines
static void FingerTERMgoDown(instructionPointer * ip)
{
	FUNGEDATATYPE n = StackPop(ip->stack);
	if (n == 0) {
		return;
	} else if (n < 0) {
		while (n++)
			putp(cursor_up);
	} else {
		while (n--)
			putp(cursor_down);
	}
}

/// G - Goto cusor position x,y (home is 0,0)
static void FingerTERMgotoXY(instructionPointer * ip)
{
	char *s;
	FUNGEDATATYPE x, y;
	x = StackPop(ip->stack);
	y = StackPop(ip->stack);
	s = tparm(cursor_address, (int)x, (int)y, 0, 0, 0, 0, 0, 0, 0);
	if (!valid(s)) {
		ipReverse(ip);
		return;
	}
	putp(s);
}

/// H - Move cursor to home
static void FingerTERMgoHome(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(cursor_home);
}

/// L - Clear from cursor to end of line
static void FingerTERMclearToEOL(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clr_eol);
}

/// S - Clear from cursor to end of screen
static void FingerTERMclearToEOS(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	putp(clr_eos);
}

/// U - Move cursor up n lines
static void FingerTERMgoUp(instructionPointer * ip)
{
	FUNGEDATATYPE n = StackPop(ip->stack);
	if (n == 0) {
		return;
	} else if (n < 0) {
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
	if (!initialised)
		return;
	putp(exit_ca_mode);
}
#endif

static bool initialise(void)
{
	int errret;
	if (initialised)
		return true;

	if (setupterm(NULL, STDOUT_FILENO, &errret) != OK && errret <= 0)
		return false;

#ifdef TERM_CAP_CORRECT
	putp(enter_ca_mode);
	atexit(finalise);
#endif
	initialised = true;
	return true;
}

bool FingerTERMload(instructionPointer * ip)
{
	if (!initialise())
		return false;
	ManagerAddOpcode(TERM,  'C', clearScreen)
	ManagerAddOpcode(TERM,  'D', goDown)
	ManagerAddOpcode(TERM,  'G', gotoXY)
	ManagerAddOpcode(TERM,  'H', goHome)
	ManagerAddOpcode(TERM,  'L', clearToEOL)
	ManagerAddOpcode(TERM,  'S', clearToEOS)
	ManagerAddOpcode(TERM,  'U', goUp)
	return true;
}

/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "input.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

static char*  lastline = NULL;
static size_t linelength = 0;
static char*  lastlineCurrent = NULL;

static void getTheLine(void)
{
	if (!lastline || !lastlineCurrent || (lastlineCurrent == '\0')) {
		cf_getline(&lastline, &linelength, stdin);
		lastlineCurrent = lastline;
	}
}
static void discardTheLine(void)
{
	lastline = NULL;
	lastlineCurrent = NULL;
}


FUNGEDATATYPE input_getchar(void)
{
	char tmp;
	getTheLine();
	tmp = *lastlineCurrent;
	lastlineCurrent++;
	if (lastlineCurrent && (*lastlineCurrent == '\0'))
		discardTheLine();
	return tmp;
}

bool input_getint(FUNGEDATATYPE * value)
{
	long long int result;
	bool found = false;
	char* endptr = NULL;

	getTheLine();
	// Find first char that is a number, then convert number.
	do {
		if (!isdigit(*lastlineCurrent))
			continue;
		found = true;
		result = strtoll(lastlineCurrent, &endptr, 10);
		break;
	} while (*(lastlineCurrent++) != '\0');
	// Discard rest of line if it is just newline, otherwise keep it.
	if (endptr && ((*endptr == '\n') || (*endptr == '\r') || (*endptr == '\0')))
		discardTheLine();
	else
		lastlineCurrent = endptr;
	*value = result;
	return found;
}

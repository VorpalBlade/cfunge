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

#include "TIME.h"
#include "../../stack.h"


#include <time.h>

static bool TIMEuseUTC = false;

#define GetTheTime \
	time_t now; \
	struct tm *curTime; \
	now = time(NULL); \
	if (TIMEuseUTC) \
		curTime = gmtime(&now); \
	else \
		curTime = localtime(&now);

/// D - Get day of month.
static void FingerTIMEday(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_mday);
}

/// F - Get day of year.
static void FingerTIMEdayOfYear(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_yday);
}

/// G - Set to use UTC.
static void FingerTIMEuseGMT(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	TIMEuseUTC = true;
}

/// H - Get hour.
static void FingerTIMEhour(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_hour);
}

/// L - Set to use local time.
static void FingerTIMEuseLocal(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	TIMEuseUTC = false;
}

/// M - Get minute.
static void FingerTIMEminute(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_min);
}

/// O - Get month.
static void FingerTIMEmonth(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_mon + 1);
}

/// S - Get second.
static void FingerTIMEsecond(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_sec);
}

/// W - Get day of week.
static void FingerTIMEdayOfWeek(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_wday + 1);
}

/// Y - Get year.
static void FingerTIMEyear(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, 1900 + curTime->tm_year);
}

bool FingerTIMEload(instructionPointer * ip)
{
	manager_add_opcode(TIME,  'D', day)
	manager_add_opcode(TIME,  'F', dayOfYear)
	manager_add_opcode(TIME,  'G', useGMT)
	manager_add_opcode(TIME,  'H', hour)
	manager_add_opcode(TIME,  'L', useLocal)
	manager_add_opcode(TIME,  'M', minute)
	manager_add_opcode(TIME,  'O', month)
	manager_add_opcode(TIME,  'S', second)
	manager_add_opcode(TIME,  'W', dayOfWeek)
	manager_add_opcode(TIME,  'Y', year)
	return true;
}

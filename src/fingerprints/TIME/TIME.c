/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
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
static void finger_TIME_day(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_mday);
}

/// F - Get day of year.
static void finger_TIME_day_of_year(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_yday);
}

/// G - Set to use UTC.
static void finger_TIME_use_gmt(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	TIMEuseUTC = true;
}

/// H - Get hour.
static void finger_TIME_hour(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_hour);
}

/// L - Set to use local time.
static void finger_TIME_use_local(FUNGE_ATTR_UNUSED instructionPointer * ip)
{
	TIMEuseUTC = false;
}

/// M - Get minute.
static void finger_TIME_minute(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_min);
}

/// O - Get month.
static void finger_TIME_month(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_mon + 1);
}

/// S - Get second.
static void finger_TIME_second(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_sec);
}

/// W - Get day of week.
static void finger_TIME_day_of_week(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, curTime->tm_wday + 1);
}

/// Y - Get year.
static void finger_TIME_year(instructionPointer * ip)
{
	GetTheTime
	stack_push(ip->stack, 1900 + curTime->tm_year);
}

bool finger_TIME_load(instructionPointer * ip)
{
	manager_add_opcode(TIME, 'D', day)
	manager_add_opcode(TIME, 'F', day_of_year)
	manager_add_opcode(TIME, 'G', use_gmt)
	manager_add_opcode(TIME, 'H', hour)
	manager_add_opcode(TIME, 'L', use_local)
	manager_add_opcode(TIME, 'M', minute)
	manager_add_opcode(TIME, 'O', month)
	manager_add_opcode(TIME, 'S', second)
	manager_add_opcode(TIME, 'W', day_of_week)
	manager_add_opcode(TIME, 'Y', year)
	return true;
}

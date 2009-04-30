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

#include "DATE.h"

#if !defined(CFUN_NO_FLOATS)
#include "../../stack.h"

#include <math.h>

#ifndef HAVE_floorl
#  define floorl floor
#endif
#ifndef HAVE_roundl
#  define roundl round
#endif

typedef int_least64_t fprint_DATE_int;

typedef struct ymd {
	fprint_DATE_int year;
	fprint_DATE_int month;
	fprint_DATE_int day;
} ymd;

FUNGE_ATTR_FAST
static int month_length(const ymd * restrict date);

/// Pop a struct ymd.
FUNGE_ATTR_FAST
static bool pop_ymd(instructionPointer * restrict ip, ymd * restrict result)
{
	result->day = stack_pop(ip->stack);
	result->month = stack_pop(ip->stack);
	result->year = stack_pop(ip->stack);

	if (result->day <= 0 || result->month <= 0 || result->month > 12
	    || result->year == 0) {
		ip_reverse(ip);
		return false;
	}
	if (result->day > month_length(result)) {
		ip_reverse(ip);
		return false;
	}
	return true;
}

/// Push a struct ymd.
FUNGE_ATTR_FAST
static void push_ymd(instructionPointer * restrict ip, const ymd * restrict result)
{
	stack_push(ip->stack, (funge_cell)result->year);
	stack_push(ip->stack, (funge_cell)result->month);
	stack_push(ip->stack, (funge_cell)result->day);

}

/// Check if year is a leap year.
FUNGE_ATTR_FAST
static bool is_leap_year(fprint_DATE_int y)
{
	// Handle year 0
	if (y < 0) y++;
	if (y % 4 == 0) {
		if (y % 400 == 0)
			return true;
		if (y % 100 == 0)
			return false;
		return true;
	}
	return false;
}

/// Compute Length of the month for the given year.
/// @note This funtion ignores day
FUNGE_ATTR_FAST
static int month_length(const ymd * restrict date)
{
	switch (date->month) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return 31;
		case 2:
			if (is_leap_year(date->year))
				return 29;
			return 28;
		default:
			return 30;
	}
}

FUNGE_ATTR_FAST
static fprint_DATE_int ymd_to_julian(const ymd * restrict date)
{
	// Based on: http://en.wikipedia.org/wiki/Julian_day#Calculation
	fprint_DATE_int Y = date->year;
	// Handle year 0
	if (Y < 0)
		Y++;
	{
		fprint_DATE_int a = floorl((14 - date->month) / 12.0L);
		fprint_DATE_int y = Y + 4800 - a;
		fprint_DATE_int m = date->month + 12 * a - 3;
		long double jdn = date->day + floorl((153 * m + 2)/5.0L) + 365 * y
		                + floorl(y/4.0L) - floorl(y/100.0L) + floorl(y/400.0L) - 32045;
		return roundl(jdn);
	}
}

FUNGE_ATTR_FAST
static void julian_to_ymd(ymd * restrict result, fprint_DATE_int date)
{
	// Based on: http://www.hermetic.ch/cal_stud/jdn.htm#comp
	// We need floorl() here, not integer rounding, since we want to round
	// downwards all the time, not towards 0.
	fprint_DATE_int l, n, i, j;
	l = date + 68569;
	n = floorl((4*l)/146097.0L);
	l -= floorl((146097*n+3)/4.0L);
	i = floorl((4000*(l+1))/1461001.0L);
	l = (int64_t)(l - floorl((1461*i)/4.0L)+31);
	j = floorl((80*l)/2447.0L);
	result->day = (fprint_DATE_int)(l - floorl((2447*j)/80.0L));
	l = floorl(j/11.0L);
	result->month = j + 2 - 12 * l;
	result->year = 100 * (n - 49) + i + l;
	// Handle year 0
	if (result->year <= 0) result->year--;
}

/// A - Add days to date
static void finger_DATE_add_days(instructionPointer * ip)
{
	fprint_DATE_int days = stack_pop(ip->stack);
	ymd date;
	if (!pop_ymd(ip, &date))
		return;
	{
		fprint_DATE_int jdn = ymd_to_julian(&date);
		jdn += days;
		julian_to_ymd(&date, jdn);
		push_ymd(ip, &date);
	}
}

/// C - Convert Julian day to calendar date
static void finger_DATE_jdn_to_ymd(instructionPointer * ip)
{
	fprint_DATE_int date = stack_pop(ip->stack);
	ymd result;
	julian_to_ymd(&result, date);
	push_ymd(ip, &result);
}

/// D - Days between dates
static void finger_DATE_day_diff(instructionPointer * ip)
{
	ymd a, b;
	if (!pop_ymd(ip, &a))
		return;
	if (!pop_ymd(ip, &b))
		return;
	{
		fprint_DATE_int a_days = ymd_to_julian(&a);
		fprint_DATE_int b_days = ymd_to_julian(&b);
		stack_push(ip->stack, (funge_cell)(b_days - a_days));
	}
}

/// J - Calendar date to Julian day
static void finger_DATE_ymd_to_jdn(instructionPointer * ip)
{
	ymd date;
	if (!pop_ymd(ip, &date))
		return;
	stack_push(ip->stack, (funge_cell)ymd_to_julian(&date));
}

/// T - Year/day-of-year to full date
static void finger_DATE_year_day_to_full(instructionPointer * ip)
{
	funge_cell doy  = stack_pop(ip->stack) + 1;
	funge_cell year = stack_pop(ip->stack);
	if (doy > (is_leap_year(year) ? 366 : 365)) {
		ip_reverse(ip);
		return;
	}
	{
		funge_cell dom = doy;
		funge_cell month = 0;
		// Iterate though months, break when less than a month.
		for (int i = 0; i < 12; i++) {
			int mlength =  month_length(&(ymd){ .year = year, .month = i + 1 });
			if ((dom - mlength) > 0) {
				dom -= mlength;
				month++;
			} else
				break;
		}
		{
			ymd date = { .year = year, .month = month + 1, .day = dom };
			push_ymd(ip, &date);
		}
	}
}

/// W - Day of week (0=Monday)
static void finger_DATE_week_day(instructionPointer * ip)
{
	ymd date;
	if (!pop_ymd(ip, &date))
		return;
	{
		fprint_DATE_int jdn = ymd_to_julian(&date);
		stack_push(ip->stack, (funge_cell)(jdn % 7));
	}
}

/// Y - Day of year (0=Jan 1)
static void finger_DATE_year_day(instructionPointer * ip)
{
	ymd date;
	if (!pop_ymd(ip, &date))
		return;
	{
		fprint_DATE_int jdn = ymd_to_julian(&date);
		fprint_DATE_int jdn_start = ymd_to_julian(&(ymd){ .year = date.year, .month = 1, .day = 1 });
		stack_push(ip->stack, (funge_cell)(jdn - jdn_start));
	}
}

bool finger_DATE_load(instructionPointer * ip)
{
	manager_add_opcode(DATE, 'A', add_days)
	manager_add_opcode(DATE, 'C', jdn_to_ymd)
	manager_add_opcode(DATE, 'D', day_diff)
	manager_add_opcode(DATE, 'J', ymd_to_jdn)
	manager_add_opcode(DATE, 'T', year_day_to_full)
	manager_add_opcode(DATE, 'W', week_day)
	manager_add_opcode(DATE, 'Y', year_day)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

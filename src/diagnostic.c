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
#include "global.h"
#include "diagnostic.h"

#include <stdlib.h> /* abort, exit */
#include <stdio.h>

FUNGE_ATTR_FAST
void diag_fatal(const char* message)
{
	fprintf(stderr, "FATAL: %s\n", message);
	exit(1);
}

FUNGE_ATTR_FAST
void diag_error(const char* message)
{
	if (FUNGE_UNLIKELY(setting_enable_errors))
		fprintf(stderr, "ERROR: %s\n", message);
}

FUNGE_ATTR_FAST
void diag_warn(const char* message)
{
	if (FUNGE_UNLIKELY(setting_enable_warnings))
		fprintf(stderr, "WARN: %s\n", message);
}


FUNGE_ATTR_FAST
void diag_fatal_format(const char* format, ...)
{
	va_list ap;
	fputs("FATAL: ", stderr);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

FUNGE_ATTR_FAST
void diag_error_format(const char* format, ...)
{
	if (FUNGE_UNLIKELY(setting_enable_errors)) {
		va_list ap;
		fputs("ERROR: ", stderr);
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

FUNGE_ATTR_FAST
void diag_warn_format(const char* format, ...)
{
	if (FUNGE_UNLIKELY(setting_enable_warnings)) {
		va_list ap;
		fputs("WARN: ", stderr);
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

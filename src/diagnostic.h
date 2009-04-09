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

/**
 * @file
 * Diagnostics handling.
 */

#ifndef FUNGE_HAD_SRC_DIAGNOSTIC_H
#define FUNGE_HAD_SRC_DIAGNOSTIC_H

#include "global.h"
#include "settings.h"

#include <stdarg.h>
#include <stdlib.h> /* abort, exit */

/**
 * @defgroup diagnostics Diagnostics
 * Various functions and macros for error reporting.
 */
/*@{*/
/// For internal use in this header only.
#define DIAG_SOURCELOC "[" __FILE__ ":" FUNGE_CPP_STRINGIFY(__LINE__) "]"

/// Print location in code, out of memory, message and abort().
#define DIAG_OOM(m_reason) \
	do { \
		fputs("FATAL: Out of memory at " DIAG_SOURCELOC ":\n " m_reason "\n", stderr); \
		abort(); \
	} while(0)

/**
 * Like DIAG_FATAL_LOC() but abort()s instead, thus hopefully producing core
 * dump. Meant for internal errors that should never happen.
 */
#define DIAG_CRIT_LOC(m_message) \
	do { \
		fputs("CRITICAL ERROR " DIAG_SOURCELOC ": " m_message "\n", stderr); \
		abort(); \
	} while(0)


/**
 * Like diag_fatal() but includes file and line number, meant for errors that
 * should never happen (not for user typoing something).
 */
#define DIAG_FATAL_LOC(m_message) \
	do { \
		fputs("FATAL " DIAG_SOURCELOC ": " m_message "\n", stderr); \
		exit(EXIT_FAILURE); \
	} while(0)

/**
 * Like diag_error() but includes file and line number, meant for errors that
 * should never happen (not for user typoing something).
 */
#define DIAG_ERROR_LOC(m_message) \
	do { \
		if (FUNGE_UNLIKELY(setting_enable_errors)) \
			fputs("ERROR " DIAG_SOURCELOC ": " m_message "\n", stderr); \
	} while(0)

/**
 * Like diag_warn() but includes file and line number, meant for errors that
 * should never happen (not for user typoing something).
 */
#define DIAG_WARN_LOC(m_fmt) \
	do { \
		if (FUNGE_UNLIKELY(setting_enable_warnings)) \
			fputs("WARN " DIAG_SOURCELOC ": " m_message "\n", stderr); \
	} while(0)

/**
 * Like diag_fatal_format() but includes file and line number, meant for errors
 * that should never happen (not for user typoing something).
 */
#define DIAG_FATAL_FORMAT_LOC(m_fmt, ...) \
	do { \
		fprintf(stderr, "FATAL " DIAG_SOURCELOC ": " m_fmt "\n", __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while(0)

/**
 * Like diag_error_format() but includes file and line number, meant for errors
 * that should never happen (not for user typoing something).
 */
#define DIAG_ERROR_FORMAT_LOC(m_fmt, ...) \
	do { \
		if (FUNGE_UNLIKELY(setting_enable_errors)) \
			fprintf(stderr, "ERROR " DIAG_SOURCELOC ": " m_fmt "\n", __VA_ARGS__); \
	} while(0)

/**
 * Like diag_warn_format() but includes file and line number, meant for errors
 * that should never happen (not for user typoing something).
 */
#define DIAG_WARN_FORMAT_LOC(m_fmt, ...) \
	do { \
		if (FUNGE_UNLIKELY(setting_enable_warnings)) \
			fprintf(stderr, "WARN " DIAG_SOURCELOC ": " m_fmt "\n", __VA_ARGS__); \
	} while(0)



/**
 * Prints "FATAL: message" and exits.
 * @param message Message to print
 */
FUNGE_ATTR_FAST FUNGE_ATTR_COLD FUNGE_ATTR_NONNULL FUNGE_ATTR_NORET
void diag_fatal(const char* message);

/**
 * Prints "ERROR: message" and returns.
 * @param message Message to print
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void diag_error(const char* message);

/**
 * Prints "WARN: message" and returns.
 * @param message Message to print
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void diag_warn(const char* message);

/**
 * Prints "FATAL: formatted message" and exits.
 * @param format Format string to use.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_COLD FUNGE_ATTR_NONNULL FUNGE_ATTR_NORET
FUNGE_ATTR_FORMAT(printf,1,2)
void diag_fatal_format(const char* format, ...);

/**
 * Prints "ERROR: formatted message" and returns.
 * @param format Format string to use.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_FORMAT(printf,1,2)
void diag_error_format(const char* format, ...);

/**
 * Prints "WARN: formatted message" and returns.
 * @param format Format string to use.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_FORMAT(printf,1,2)
void diag_warn_format(const char* format, ...);

/*@}*/

#endif

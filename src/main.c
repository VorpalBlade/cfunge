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
 * Contains command line parsing and such.
 */
#include "global.h"
#include "main.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "interpreter.h"
#include "settings.h"
#include "diagnostic.h"
#include "fingerprints/manager.h"

const char **fungeargv = NULL;
int fungeargc = 0;
// Use a larger buffer for stdout in fully buffered mode.
static char cfun_iobuf[BUFSIZ*4];

// These are NOT worth inlineing, even though only called once.
FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_features(void)
{
	puts("Features compiled into this binary:");
#ifdef CONCURRENT_FUNGE
	puts(" + Concurrency using t instruction is enabled.");
#else
	puts(" - Concurrency using t instruction is disabled.");
#endif

#ifndef DISABLE_TRACE
	puts(" + Tracing using -t <level> option is enabled.");
#else
	puts(" - Tracing using -t <level> option is disabled.");
#endif

#ifdef CFUN_USE_GC
	puts(" * This binary uses Boehm GC.");
#else
	puts(" * This binary does not use Boehm GC.");
#endif

#ifdef DEBUG
	puts(" * This binary is a debug build.");
#endif

#ifndef NDEBUG
	puts(" * This binary is compiled with asserts.");
#endif

#ifdef ENABLE_VALGRIND
	puts(" * This binary is compiled with valgrind debugging annotations.");
#endif

#if defined(USE64)
	puts(" * Cell size is 64 bits (8 bytes).");
#elif defined(USE32)
	puts(" * Cell size is 32 bits (4 bytes).");
#else
	puts(" * Err, this shouldn't happen, it seems cell size is not known...");
#endif

#ifdef FUZZ_TESTING
	// We use this to warn users and to do sanity checking in the fuzz testing
	// script.
	puts(" ! This is a fuzz testing build and thus not standard-conforming.");
#endif

	cf_putchar_maybe_locked('\n');
	// This call does not return.
	manager_list();
}

FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_help(void)
{
	puts("Usage: cfunge [OPTIONS] [FILE] [PROGRAM OPTIONS]");
	puts("A fast Befunge interpreter in C\n");
	puts(" -b           Use fully buffered output (default is system default for stdout).");
	puts(" -E           Show non-fatal error messages, fatal ones are always shown.");
	puts(" -F           Disable all fingerprints.");
	puts(" -f           Show list of features and fingerprints supported in this binary.");
	puts(" -h           Show this help and exit.");
	puts(" -S           Enable sandbox mode (see README for details).");
	puts(" -s standard  Use the given standard (one of 93, 98 [default] and 109).");
	puts(" -t level     Use given trace level. Default 0.");
	puts(" -V           Show version information and exit.");
	puts(" -W           Show warnings.");

#ifdef DISABLE_TRACE
	puts("\nNote that someone disabled trace in this binary, so -t will have no effect.");
#endif
	exit(EXIT_SUCCESS);
}

FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_version(void)
{
	puts("cfunge " CFUNGE_APPVERSION);
	puts("Copyright (C) 2008-2009 Arvid Norlander.");
	puts("This is free software.  You may redistribute copies of it under the terms of");
	puts("the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.");
	puts("There is NO WARRANTY, to the extent permitted by law.\n");

	puts("Written by Arvid Norlander.");

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int opt;

#ifdef CFUN_USE_GC
//	GC_find_leak = 1;
	GC_all_interior_pointers = 1;
	GC_INIT();
//	atexit(&GC_gcollect);
#endif
#ifdef FUZZ_TESTING
	alarm(3);
#endif

	// We detect socket issues in other ways.
	signal(SIGPIPE, SIG_IGN);

	while ((opt = getopt(argc, argv, "+bEFfhSs:t:VW")) != -1) {
		switch (opt) {
			case 'b':
				setvbuf(stdout, cfun_iobuf, _IOFBF, sizeof(cfun_iobuf));
				break;
			case 'E':
				setting_enable_errors = true;
				break;
			case 'F':
				setting_disable_fingerprints = true;
				break;
			case 'f':
				print_features();
				break;
			case 'h':
				print_help();
				break;
			case 'S':
				setting_enable_sandbox = true;
				break;
			case 's':
				if (strncmp(optarg, "93", 2) == 0)
					setting_current_standard = stdver93;
				else if (strncmp(optarg, "98", 2) == 0)
					setting_current_standard = stdver98;
				else if (strncmp(optarg, "109", 3) == 0)
					setting_current_standard = stdver109;
				else {
					diag_fatal_format("%s is not valid for -s.\n", optarg);
				}
				break;
			case 't':
				setting_trace_level = (uint_fast16_t)atoi(optarg);
				break;
			case 'V':
				print_version();
				break;
			case 'W':
				setting_enable_warnings = true;
				break;
			default:
				fprintf(stderr, "For help see: %s -h\n", argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (FUNGE_UNLIKELY(optind >= argc)) {
		diag_fatal("No file provided.");
	} else {
		// Copy a argument count and a pointer to argv[optind] for later reuse
		// by the y instruction.
		fungeargc = argc - optind;
		fungeargv = (const char**)&argv[optind];
		// Run the actual interpreter.
		interpreter_run(argv[optind]);
	}
}

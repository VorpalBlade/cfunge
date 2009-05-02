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

#include <stdio.h>  /* fprintf, puts */
#include <stdlib.h> /* exit */
#include <signal.h> /* signal */
#include <string.h> /* strncmp */
#include <unistd.h> /* getopt */
#include <limits.h> /* CHAR_BIT */

#include "diagnostic.h"
#include "interpreter.h"
#include "settings.h"
#include "fingerprints/manager.h"

const char **fungeargv = NULL;
int fungeargc = 0;

// Exclude some code if we are building in IFFI.
#ifndef CFUN_IS_IFFI
// Use a larger buffer for stdout in fully buffered mode.
static char cfun_iobuf[BUFSIZ*4];

// These are NOT worth inlineing, even though only called once.
FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_features(void)
{
	puts("Features compiled into this binary:\n"
#ifdef CONCURRENT_FUNGE
	     " + Concurrency using t instruction is enabled.\n"
#else
	     " - Concurrency using t instruction is disabled.\n"
#endif

#ifndef DISABLE_TRACE
	     " + Tracing using -t <level> option is enabled.\n"
#else
	     " - Tracing using -t <level> option is disabled.\n"
#endif

#ifdef CFUN_EXACT_BOUNDS
	     " + This binary uses exact bounds in y.\n"
#else
	     " - This binary does not use exact bounds in y.\n"
#endif

#ifdef CFUN_USE_GC
	     " * This binary uses Boehm GC.\n"
#else
	     " * This binary does not use Boehm GC.\n"
#endif

#ifdef DEBUG
	     " * This binary is a debug build.\n"
#endif

#ifndef NDEBUG
	     " * This binary is compiled with asserts.\n"
#endif

#ifdef ENABLE_VALGRIND
	     " * This binary is compiled with valgrind debugging annotations.\n"
#endif

#if defined(USE64)
	     " * Cell size is 64 bits (8 bytes).\n"
#elif defined(USE32)
	     " * Cell size is 32 bits (4 bytes).\n"
#else
#  error "Unknown cell size."
#endif

	// Features with ! are stuff most users doesn't want.
#ifdef CFUN_NO_FLOATS
	     " ! This binary is compiled without any floating point fingerprints.\n"
#endif

#ifdef FUZZ_TESTING
	// We use this to warn users and to do sanity checking in the fuzz testing
	// script.
	     " ! This is a fuzz testing build and thus not standard-conforming.\n"
#endif

	); /* End of puts() call */

	// This call does not return.
	manager_list();
}

FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_help(void)
{
	puts("Usage: cfunge [OPTIONS] [FILE] [PROGRAM OPTIONS]\n"
	     "A fast Befunge interpreter in C\n\n"
	     " -b           Use fully buffered output (default is system default for stdout).\n"
	     " -E           Show non-fatal error messages, fatal ones are always shown.\n"
	     " -F           Disable all fingerprints.\n"
	     " -f           Show list of features and fingerprints supported in this binary.\n"
	     " -h           Show this help and exit.\n"
	     " -S           Enable sandbox mode (see README for details).\n"
	     " -s standard  Use the given standard (one of 93, 98 [default] and 109).\n"
	     " -t level     Use given trace level. Default 0.\n"
	     " -V           Show version and copyright info and exit.\n"
	     " -v           Show version and build info and exit.\n"
	     " -W           Show warnings."
#ifdef DISABLE_TRACE
	     "\nNote that someone disabled trace in this binary, so -t will have no effect."
#endif
	     );
	exit(EXIT_SUCCESS);
}

FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_build_info(void) {
	printf("cfunge " CFUNGE_APPVERSION " ["
#ifdef CONCURRENT_FUNGE
	       "+con "
#else
	       "-con "
#endif
#ifndef DISABLE_TRACE
	       "+trace "
#else
	       "-trace "
#endif
#ifdef CFUN_EXACT_BOUNDS
	       "+exact-bounds "
#else
	       "-exact-bounds "
#endif
#ifdef HAVE_NCURSES
	       "+ncurses "
#else
	       "-ncurses "
#endif
#ifdef CFUN_USE_GC
	       "gc "
#endif
#ifdef _FORTIFY_SOURCE
	       "hardened "
#endif
#ifdef DEBUG
	       "debug "
#endif
#ifndef NDEBUG
	       "asserts "
#endif
#ifdef ENABLE_VALGRIND
	       "valgrind "
#endif
#ifdef _MUDFLAP
	       "mud "
#endif
#ifdef CFUN_NO_FLOATS
	       "nofloat "
#endif
#ifdef FUZZ_TESTING
	       "fuzz "
#endif
	// Pointer size and Cell size
	       "p:%zu c:%zu]\n\n"
           "Platform:      " CFUN_TARGET_PLATFORM "\n"
	       "OS:            " CFUN_TARGET_OS "\n"
	       "Compiler path: " CFUN_COMPILER "\n"
#ifdef CFUNGE_COMP_CLANG
	       "Compiler:      clang (unknown version)\n"
#elif defined(CFUNGE_COMP_ICC)
	       "Compiler:      ICC " FUNGE_CPP_STRINGIFY(__INTEL_COMPILER) "\n"
#elif defined(CFUNGE_COMP_GCC)
	       "Compiler:      GCC " FUNGE_CPP_STRINGIFY(__GNUC__)
	       "." FUNGE_CPP_STRINGIFY(__GNUC_MINOR__)
	       "." FUNGE_CPP_STRINGIFY(__GNUC_PATCHLEVEL__)
	       " (or compatible)\n"
#else
	       "Compiler:      Unknown.\n"
#endif
	       "Build type:    " CFUN_BUILD_TYPE "\n"
	       "Compiled on:   " CFUN_COMPILED_ON "\n\n"
	       "CFLAGS=\"" CFUN_USER_CFLAGS "\"\n"
	       "LDFLAGS=\"" CFUN_USER_LDFLAGS "\"\n",
	       sizeof(void*) * CHAR_BIT, sizeof(funge_cell) * CHAR_BIT);
	exit(EXIT_SUCCESS);
}

FUNGE_ATTR_FAST FUNGE_ATTR_NOINLINE FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void print_version(void)
{
	puts("cfunge " CFUNGE_APPVERSION "\n"
	     "Copyright (C) 2008-2009 Arvid Norlander.\n"
	     "This is free software.  You may redistribute copies of it under the terms of\n"
	     "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
	     "There is NO WARRANTY, to the extent permitted by law.\n\n"
	     "Written by Arvid Norlander.");

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

	while ((opt = getopt(argc, argv, "+bEFfhSs:t:VvW")) != -1) {
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
			case 'v':
				print_build_info();
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
		// Run the actual interpreter (never returns).
		interpreter_run(argv[optind]);
	}
	// NEVER REACHED.
}
#endif /* ! CFUN_IS_IFFI */

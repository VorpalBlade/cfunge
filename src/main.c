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

#include "global.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "interpreter.h"
#include "settings.h"
#include "fingerprints/manager.h"

char **fungeargv = NULL;
int fungeargc = 0;

// These are NOT worth inlineing
static void printHelp(void) __attribute__((noreturn,noinline));
static void printVersion(void) __attribute__((noreturn,noinline));
static void printFeatures(void) __attribute__((noreturn,noinline));


static void printFeatures(void) {
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

#ifndef DISABLE_GC
	puts(" + This binary uses Boehm GC.");
#else
	puts(" - This binary does NOT use Boehm GC!");
	puts("   You are on your own if you disable Boehm GC!");
#endif

#if defined(USE64)
	puts(" * Cell size is 64 bits (8 bytes).");
#elif defined(USE32)
	puts(" * Cell size is 32 bits (4 bytes).");
#else
	puts(" * Err, this shouldn't happen, it seems cell size is not known...");
#endif

	putchar('\n');
	// This call does not return.
	ManagerList();
}

static void printHelp(void) {
	puts("Usage: cfunge [OPTIONS] [FILE] [SCRIPT OPTIONS]");
	puts("A fast Befunge interpreter in C\n");
	puts(" -F           Disable all fingerprints.");
	puts(" -f           Show list of features and fingerprints supported in this binary.");
	puts(" -h           Show this help and exit.");
	puts(" -S           Enable sandbox mode (see README for details).");
	puts(" -s standard  Use the given standard (one of 93, 98 [default] and 108).");
	puts(" -t level     Use given trace level. Default 0.");
	puts(" -V           Show version information and exit.");
	puts(" -W           Show warnings.");

#ifdef DISABLE_TRACE
	puts("\nNote that someone disabled trace in this binary, so -t will have no effect.");
#endif
	exit(EXIT_SUCCESS);
}

static void printVersion(void) {
	printf("cfunge %s\n", APPVERSION);
	puts("Copyright (C) 2008 Arvid Norlander.");
	puts("This is free software.  You may redistribute copies of it under the terms of");
	puts("the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.");
	puts("There is NO WARRANTY, to the extent permitted by law.\n");

	puts("Written by Arvid Norlander.");

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int opt;

#ifndef DISABLE_GC
//	GC_find_leak = 1;
//	atexit(&GC_gcollect);
	GC_all_interior_pointers = 1;
	GC_INIT();
#endif

	while ((opt = getopt(argc, argv, "+FfhSs:t:VW")) != -1) {
		switch (opt) {
			case 'F':
				SettingEnableFingerprints = false;
				break;
			case 'f':
				printFeatures();
				break;
			case 'h':
				printHelp();
				break;
			case 'S':
				SettingSandbox = true;
				break;
			case 's':
				if (strncmp(optarg, "93", 2) == 0)
					SettingCurrentStandard = stdver93;
				else if (strncmp(optarg, "98", 2) == 0)
					SettingCurrentStandard = stdver98;
				else if (strncmp(optarg, "108", 3) == 0)
					SettingCurrentStandard = stdver108;
				else {
					fprintf(stderr, "%s is not valid for -s.\n", optarg);
					return EXIT_FAILURE;
				}
				break;
			case 't':
				SettingTraceLevel = atoi(optarg);
				break;
			case 'V':
				printVersion();
				break;
			case 'W':
				SettingWarnings = true;
				break;
			default:
				fprintf(stderr, "For help see: %s -h\n", argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind >= argc) {
		fputs("No file provided\n", stderr);
		return EXIT_FAILURE;
	} else {
		// Copy the rest to the variables in interpreter.c/interpreter.h
		// for later reuse by y instruction.
		if (argc > 1) {
			fungeargc = argc - optind;
			fungeargv = cf_malloc(fungeargc * sizeof(char*));
			for (int i = optind; i < argc; i++) {
				fungeargv[i - optind] = cf_strdup(argv[i]);
				if (fungeargv[i - optind] == NULL) {
					perror("Couldn't store arguments in array and this even before file was loaded!\n");
					abort();
				}
			}
		}
		interpreterRun(argv[optind]);
	}
}

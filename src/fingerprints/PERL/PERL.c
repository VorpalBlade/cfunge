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

#include "PERL.h"
#include "../../stack.h"
#include "../../settings.h"

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdio.h>

#include <assert.h>
#include <errno.h>

#include <string.h>
#include <limits.h>

// Used when reading back the result
#define STRINGALLOCCHUNK 1024

// There are two ways to interpret the docs.
#define PERL_AS_CCBI_DOES_IT

// S - Is Perl loaded? Or will it be shelled?
static void FingerPERLshelled(instructionPointer * ip)
{
	StackPush(ip->stack, 1); // Not Perl, at least last I checked this was C.
}

// The A stuff is because otherwise we get an error (from read) in case of no
// other output.
FUNGE_FAST
static inline char * FindData(char * output, size_t length) {
	const char * p;

#ifdef PERL_AS_CCBI_DOES_IT
	p = strrchr(output, 'A');
#else
	p = strchr(output, 'A');
#endif
	if (p == NULL) {
		return output;
	} else {
		char * restrict newstr;
		size_t maxlen = length - (p - output);
		newstr = cf_strndup(++p, maxlen);
		cf_free(output);
		return newstr;
	}

}

// Builds the argument with the perl code.
FUNGE_FAST
static inline char * BuildArgument(const char * restrict perlcode) {
	size_t argsize;
	char * restrict argument;
	// +1 for \0
	argsize = strlen("print 'A',eval()") + strlen(perlcode) + 1;
	argument = malloc_nogc(argsize * sizeof(char));
	if (!argument) {
		return NULL;
	}
#ifdef PERL_AS_CCBI_DOES_IT
	snprintf(argument, argsize, "print 'A',eval(%s)", perlcode);
#else
	snprintf(argument, argsize, "print 'A';eval(%s)", perlcode);
#endif
	return argument;
}

#define CLEANUP_PIPE \
	{ \
		close(fds[0]); \
		close(fds[1]); \
	}

// Yes... This is a mess...
FUNGE_FAST
static char * RunPerl(const char * restrict perlcode)
{
	pid_t pid;
	int fds[2]; // For pipe to child.
	int status;

	if (perlcode == NULL)
		return NULL;

	if (pipe(fds) == -1)
		return NULL;
	if (fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1) {
		if (SettingWarnings)
			perror("RunPerl, fcntl failed");
		CLEANUP_PIPE
		return NULL;
	}

	pid = fork();
	switch (pid) {
		case -1: // Parent, error
			if (SettingWarnings)
				perror("RunPerl, could not fork");
			// Clean up
			CLEANUP_PIPE
			return NULL;
			break;

		case 0: {
			// Child

			// Build argument list.
			// Strdup to avoid the read only string warning.
			// No need to free in child.
			char * arguments[] = { strdup_nogc("perl"), strdup_nogc("-e"), NULL, NULL };
			arguments[2] = BuildArgument(perlcode);
			if (!arguments[2]) {
				_Exit(2);
			}

			// Do the FD stuff.
			dup2(fds[1], 1);
			dup2(fds[1], 2);

			// Execute
			if (execvp("perl", arguments) == -1) {
				// If we got here...
				// Message is followed by ": error"
				perror("Failed to run perl");
			}
			_Exit(2);
			break;
		}

		default: {
			// Parent, sucess
			if (waitpid(pid, &status, 0) != -1) {
				if (WIFEXITED(status)) {
					// Ok... get output :)
					ssize_t n;
					size_t bufsize = STRINGALLOCCHUNK;
					char * returnedData;
					char * p;
					int readErrno;

					returnedData = cf_calloc_noptr(bufsize, sizeof(char));
					if (!returnedData)
						return NULL;
					p = returnedData;

					// Read the result
					// Yes this is very messy. Needs cleaning up.
					while (true) {
						n = read(fds[0], p, STRINGALLOCCHUNK);
						readErrno = errno;
						if (n == -1) {
							CLEANUP_PIPE
							if (readErrno == EAGAIN) {
								return FindData(returnedData, bufsize - STRINGALLOCCHUNK);
							} else {
								if (SettingWarnings)
									perror("RunPerl, read failed");
								cf_free(returnedData);
								return NULL;
							}
						} else if (n > 0) {
							if ((n + 1) >= STRINGALLOCCHUNK) {
								char * reallocRes;
								bufsize += STRINGALLOCCHUNK;
								reallocRes = cf_realloc(returnedData, bufsize * sizeof(char));
								if (!reallocRes) {
									if (SettingWarnings)
										perror("RunPerl, realloc for returnedData failed");
									CLEANUP_PIPE
									return FindData(returnedData,  bufsize - STRINGALLOCCHUNK);
								} else {
									returnedData = reallocRes;
									p = returnedData + (bufsize - 2 * STRINGALLOCCHUNK) + n;
									memset(p, '\0', STRINGALLOCCHUNK);
								}
							} else {
								CLEANUP_PIPE
								return FindData(returnedData, bufsize - STRINGALLOCCHUNK + n);
							}
						}
					}

				} else /* WIFEXITED */ {
					// Error
					CLEANUP_PIPE
					return NULL;
				}
			} else /* waitpid */ {
				// Error
				CLEANUP_PIPE
				return NULL;
			}
			break;
		} // default
	} // switch
}

// E - Evaluate 0gnits
static void FingerPERLeval(instructionPointer * ip)
{
	char * restrict result;
	char * restrict perlcode = StackPopString(ip->stack);
	result = RunPerl(perlcode);
	if (result == NULL) {
		ipReverse(ip);
	} else {
		StackPush(ip->stack, '\0');
		StackPushString(ip->stack, result, strlen(result));
	}
#ifdef DISABLE_GC
	cf_free(perlcode);
#endif
	cf_free(result);
}

// I - As E but cast to integer.
static void FingerPERLintEval(instructionPointer * ip)
{
	char * restrict result;
	char * restrict perlcode = StackPopString(ip->stack);
	result = RunPerl(perlcode);
	if (result == NULL) {
		ipReverse(ip);
	} else {
		long int i = strtol(result, NULL, 10);
		if ((i == LONG_MIN) || (i == LONG_MAX)) {
			if (errno == ERANGE)
				StackPush(ip->stack, -1);
		} else {
			StackPush(ip->stack, (FUNGEDATATYPE)i);
		}
	}
#ifdef DISABLE_GC
	cf_free(perlcode);
#endif
	cf_free(result);
}

bool FingerPERLload(instructionPointer * ip)
{
	ManagerAddOpcode(PERL,  'E', eval)
	ManagerAddOpcode(PERL,  'I', intEval)
	ManagerAddOpcode(PERL,  'S', shelled)
	return true;
}

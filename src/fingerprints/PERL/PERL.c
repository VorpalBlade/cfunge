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

// Thanks to Heikki Kallasjoki for help with the perl side of this.
// I would not have managed it without his help.

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

#include <errno.h>

#include <string.h>
#include <limits.h>

// Used when reading back the result
#define STRINGALLOCCHUNK 1024

// S - Is Perl loaded? Or will it be shelled?
static void FingerPERLshelled(instructionPointer * ip)
{
	StackPush(ip->stack, 1); // Not Perl, at least last I checked this was C.
}

// Yes... This is a mess...
FUNGE_FAST
static char * RunPerl(const char * restrict perlcode)
{
	pid_t pid;
	int outfds[2]; // For pipe of stderr.

	if (perlcode == NULL)
		return NULL;

	if (pipe(outfds) == -1)
		return NULL;

	// Non-blocking to prevent locking up in read() in parent
	if (fcntl(outfds[0], F_SETFL, O_NONBLOCK) == -1) {
		if (SettingWarnings)
			perror("RunPerl, fcntl on outfds failed");
		close(outfds[0]);
		close(outfds[1]);
		return NULL;
	}

	pid = fork();
	switch (pid) {
		case -1: // Parent, error
			if (SettingWarnings)
				perror("RunPerl, could not fork");
			// Clean up
			close(outfds[0]);
			close(outfds[1]);
			return NULL;
			break;

		case 0: {
			// Child

			// Build argument list.
			// Strdup to avoid the read only string warning.
			// No need to free in child.
			// TODO: Check that the allocations worked.
			char * arguments[] = {
				strdup_nogc("perl"),
				strdup_nogc("-e"),
				strdup_nogc("open(CFUNGE_REALERR, \">&STDERR\"); open(STDERR, \">&STDOUT\"); print CFUNGE_REALERR eval($ARGV[0])"),
				strdup_nogc(perlcode),
				NULL
			};

			// Do the FD stuff.
			// Close unused end
			close(outfds[0]);
			// Dup the FD
			dup2(outfds[1], 2);

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
			// Parent, success
			int status;
			// Close unused end
			close(outfds[1]);
			// Wait...
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
						n = read(outfds[0], p, STRINGALLOCCHUNK);
						readErrno = errno;
						if (n == -1) {
							close(outfds[0]);
							if (readErrno == EAGAIN) {
								return returnedData;
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
									close(outfds[0]);
									return returnedData;
								} else {
									returnedData = reallocRes;
									p = returnedData + (bufsize - 2 * STRINGALLOCCHUNK) + n;
									memset(p, '\0', STRINGALLOCCHUNK);
								}
							} else {
								close(outfds[0]);
								return returnedData;
							}
						}
					}

				} else /* WIFEXITED */ {
					// Error
					close(outfds[0]);
					return NULL;
				}
			} else /* waitpid */ {
				// Error
				close(outfds[0]);
				return NULL;
			}
			break;
		} // default
	} // switch
}

// E - Evaluate 0gnirts
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

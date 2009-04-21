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

// Thanks to Heikki Kallasjoki for help with the Perl side of this.
// I would not have managed it without his help.

#include "PERL.h"
#include "../../stack.h"
#include "../../diagnostic.h"
#include "../../settings.h"
#include "../../../lib/stringbuffer/stringbuffer.h"

#include <unistd.h> /* close, dup2, execvp, fcntl, fork, pipe, read */
#include <fcntl.h> /* fcntl */

#include <sys/types.h> /* waitpid */
#include <sys/wait.h> /* waitpid */

#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> /* _Exit, strtol */
#include <string.h> /* strerror */

// Used when reading back the result
#define STRINGALLOCCHUNK 1024

/// S - Is Perl loaded? Or will it be shelled?
static void finger_PERL_shelled(instructionPointer * ip)
{
	stack_push(ip->stack, 1); // Not Perl, at least last I checked this was C.
}

// Yes... This is a mess...
FUNGE_ATTR_FAST
static char * run_perl(const char * restrict perlcode, size_t * restrict retlength)
{
	pid_t pid;
	int outfds[2]; // For pipe of stderr.

	if (perlcode == NULL)
		return NULL;

	if (pipe(outfds) == -1)
		return NULL;

	// Non-blocking to prevent locking up in read() in parent
	if (fcntl(outfds[0], F_SETFL, O_NONBLOCK) == -1) {
		DIAG_ERROR_FORMAT_LOC("fcntl() on outfds failed in run_perl: %s", strerror(errno));
		close(outfds[0]);
		close(outfds[1]);
		return NULL;
	}

	pid = fork();
	switch (pid) {
		case -1: // Parent, error
			DIAG_ERROR_FORMAT_LOC("Could not fork in run_perl: %s", strerror(errno));
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
			char * const arguments[] = {
				strdup_nogc("perl"),
				strdup_nogc("-e"),
				strdup_nogc("open(CFUNGE_REALERR, \">&STDERR\"); open(STDERR, \">&STDOUT\"); print CFUNGE_REALERR eval($ARGV[0])"),
				strdup_nogc(perlcode),
				NULL
			};
			if (!arguments[0] || !arguments[1] || !arguments[2]
			    || !arguments[3]) {
				_Exit(2);
			}

			// Do the FD stuff.
			// Close unused end
			close(outfds[0]);
			// Dup the FD
			dup2(outfds[1], 2);
			// Close unused copy
			close(outfds[1]);

			// Execute
			if (execvp("perl", arguments) == -1) {
				// If we got here...
				// Message is followed by ": error"
				DIAG_ERROR_FORMAT_LOC("Failed to run perl: %s", strerror(errno));
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
					StringBuffer * sb;
					char * buf;
					int readErrno;

					sb = stringbuffer_new();
					if (!sb)
						return NULL;
					buf = malloc_nogc((STRINGALLOCCHUNK + 1) * sizeof(char));
					if (!buf) {
						stringbuffer_destroy(sb);
						return NULL;
					}

					// Read the result
					while (true) {
						n = read(outfds[0], buf, STRINGALLOCCHUNK);
						readErrno = errno;
						if (n == -1) {
							close(outfds[0]);
							if (readErrno == EAGAIN) {
								free_nogc(buf);
								return stringbuffer_finish(sb, retlength);
							} else {
								DIAG_ERROR_FORMAT_LOC("Read failed in run_perl: %s", strerror(readErrno));
								free_nogc(buf);
								stringbuffer_destroy(sb);
								return NULL;
							}
						} else if (n > 0) {
							buf[n] = '\0';
							stringbuffer_append_string(sb, buf);
							if (n < STRINGALLOCCHUNK) {
								close(outfds[0]);
								free_nogc(buf);
								return stringbuffer_finish(sb, retlength);
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
		} // default
	} // switch
	// This is never reached, just avoid ICC warnings.
	// GCC can detect this is not reached.
	return NULL;
}

/// E - Evaluate 0gnirts
static void finger_PERL_eval(instructionPointer * ip)
{
	size_t length;
	char * restrict result;
	char * restrict perlcode = (char*)stack_pop_string(ip->stack, NULL);
	result = run_perl(perlcode, &length);
	if (result == NULL) {
		ip_reverse(ip);
	} else {
		stack_push_string(ip->stack, (unsigned char*)result, length);
	}
	stack_free_string(perlcode);
	free_nogc(result);
}

/// I - As E but cast to integer.
static void finger_PERL_int_eval(instructionPointer * ip)
{
	char * restrict result;
	char * restrict perlcode = (char*)stack_pop_string(ip->stack, NULL);
	result = run_perl(perlcode, NULL);
	if (result == NULL) {
		ip_reverse(ip);
	} else {
		long int i;
		errno = 0;
		i = strtol(result, NULL, 10);
		if (errno == ERANGE)
			stack_push(ip->stack, -1);
		else
			stack_push(ip->stack, (funge_cell)i);
	}
	stack_free_string(perlcode);
	free_nogc(result);
}

bool finger_PERL_load(instructionPointer * ip)
{
	manager_add_opcode(PERL, 'E', eval)
	manager_add_opcode(PERL, 'I', int_eval)
	manager_add_opcode(PERL, 'S', shelled)
	return true;
}

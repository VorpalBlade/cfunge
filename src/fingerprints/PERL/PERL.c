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


#define STRINGALLOCCHUNK 1024

// S - Is Perl loaded? Or will it be shelled?
static void FingerPERLshelled(instructionPointer * ip)
{
	StackPush(ip->stack, 1); // Not Perl, at least last I checked this was C.
}

FUNGE_FAST
static char * FindData(char * output, size_t length) {
	const char * p;

	output[length] = '\0';
	p = strchr(output, 'A');
	if (p == NULL) {
		return output;
	} else {
		char * restrict newstr;
		newstr = strdup(++p);
		cf_free(output);
		return newstr;
	}

}

FUNGE_FAST
static char * RunPerl(const char * perlcode)
{
	volatile pid_t pid;
	int fds[2];
	int status;

	size_t argsize;
	int snprintfretval;
	char * argument = NULL;

	if (perlcode == NULL)
		return NULL;

	// We fork here as system() got quoting issues.
	if (pipe(fds) == -1)
		return NULL;
	if (fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1) {
		if (SettingWarnings)
			perror("RunPerl, fcntl failed");
		return NULL;
	}



	// +1 for \0
	argsize = strlen("print 'A';eval()") + strlen(perlcode) + 1;
	argument = cf_calloc(argsize, sizeof(char));
	if (!argument)
		_Exit(1);

	snprintfretval = snprintf(argument, argsize, "print 'A';eval(%s)", perlcode);
	assert(snprintfretval < (long)argsize);

	pid = fork();
	switch (pid) {
		case -1: // Parent, error
			if (SettingWarnings)
				perror("RunPerl, could not fork");
			return NULL;

		case 0: {
			// Child

			// Do the FD stuff.
			dup2(fds[1], 1);
			dup2(fds[1], 2);

			// Execute
			if (execlp("perl", "perl", "-e", argument) == -1) {
				// If we got here...
				// Message is followed by ": error"
				perror("RunPerl, execvp failed! This is bad... Cause");
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

					returnedData = cf_malloc(bufsize * sizeof(char));
					if (!returnedData)
						return NULL;
					p = returnedData;

					do {
						n = read(fds[0], p, bufsize - 1);
						readErrno = errno;
						if (n == -1)
							if (SettingWarnings)
								perror("RunPerl, read failed");
						if ((n > 0) && (n >= (ssize_t)bufsize)) {
							char * reallocRes;
							bufsize += STRINGALLOCCHUNK;
							reallocRes = cf_realloc(returnedData, bufsize * sizeof(char));
							if (!reallocRes) {
								if (SettingWarnings)
									perror("RunPerl, realloc for returnedData failed");
								return FindData(returnedData,  bufsize - STRINGALLOCCHUNK);
							} else {
								returnedData = reallocRes;
								p = returnedData + (bufsize - STRINGALLOCCHUNK) + n;
							}
						}
					} while ((n > 0) && (n >= (ssize_t)bufsize));

					if (n == -1) {
						if (readErrno == EAGAIN) { // OK one
							if (n > 0) {
								return FindData(returnedData, bufsize - STRINGALLOCCHUNK + n);
							} else {
								return FindData(returnedData, bufsize - STRINGALLOCCHUNK);
							}
						} else {
							cf_free(returnedData);
							return NULL;
						}
					}
					if (n > 0) {
						return FindData(returnedData, bufsize - STRINGALLOCCHUNK + n);
					} else {
						return FindData(returnedData, bufsize - STRINGALLOCCHUNK);
					}
				} else {
					// Error
					return NULL;
				}
			} else {
				// Error
				return NULL;
			}
			break;
		}
	}
}

// E - Evaluate 0gnits
static void FingerPERLeval(instructionPointer * ip)
{
	char * restrict result;
	char * restrict perlcode = StackPopString(ip->stack);
	result = RunPerl(perlcode);
	if (result == NULL) {
		ipReverse(ip);
		return;
	}
	StackPush(ip->stack, '\0');
	StackPushString(ip->stack, result, strlen(result));
}

// I - As E but cast to integer.
static void FingerPERLintEval(instructionPointer * ip)
{
	char * restrict result;
	char * restrict perlcode = StackPopString(ip->stack);
	result = RunPerl(perlcode);
	if (result == NULL) {
		ipReverse(ip);
		return;
	}
	{
		long int i = strtol(result, NULL, 10);
		if ((i == LONG_MIN) || (i == LONG_MAX)) {
			if (errno == ERANGE)
				StackPush(ip->stack, 0);
		} else {
			StackPush(ip->stack, (FUNGEDATATYPE)i);
		}
	}
}

bool FingerPERLload(instructionPointer * ip)
{
	ManagerAddOpcode(PERL,  'E', eval)
	ManagerAddOpcode(PERL,  'I', intEval)
	ManagerAddOpcode(PERL,  'S', shelled)
	return true;
}

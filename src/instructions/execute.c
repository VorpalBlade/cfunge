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

#include "../global.h"
#include "execute.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

// On empty string
#define FUNGE_NOCOMMAND -2

FUNGE_ATTR_FAST void run_system_execute(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (setting_enable_sandbox) {
		ip_reverse(ip);
		return;
	}

	{
		char * restrict command;
		int retval;
		// Pop stuff.
		command = (char*)stack_pop_string(ip->stack, NULL);

		// Sanity test!
		if (*command == '\0') {
			stack_free_string(command);
			stack_push(ip->stack, FUNGE_NOCOMMAND);
			return;
		}

		retval = system(command);
		// POSIX says we may only use WEXITSTATUS if WIFEXITED returns true...
		if (WIFEXITED(retval)) {
			stack_push(ip->stack, (funge_cell)WEXITSTATUS(retval));
		} else {
			stack_push(ip->stack, (funge_cell)retval);
		}
		stack_free_string(command);
	}
}

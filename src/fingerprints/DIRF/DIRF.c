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

#include "DIRF.h"
#include "../../stack.h"

#include <unistd.h> /* chdir, rmdir */
#include <sys/stat.h> /* mkdir */
#include <sys/types.h> /* mkdir (not required by POSIX?) */

static void finger_DIRF_chdir(instructionPointer * ip)
{
	size_t len;
	char * restrict str = (char*)stack_pop_string(ip->stack, &len);
	if (!str || (len < 1)) {
		ip_reverse(ip);
	} else if (chdir(str) != 0) {
		ip_reverse(ip);
	}
	stack_free_string(str);
}

static void finger_DIRF_mkdir(instructionPointer * ip)
{
	size_t len;
	char * restrict str = (char*)stack_pop_string(ip->stack, &len);
	if (!str || (len < 1)) {
		ip_reverse(ip);
	} else if (mkdir(str, S_IRWXU) != 0) {
		ip_reverse(ip);
	}
	stack_free_string(str);
}

static void finger_DIRF_rmdir(instructionPointer * ip)
{
	size_t len;
	char * restrict str = (char*)stack_pop_string(ip->stack, &len);
	if (!str || (len < 1)) {
		ip_reverse(ip);
	} else if (rmdir(str) != 0) {
		ip_reverse(ip);
	}
	stack_free_string(str);
}


bool finger_DIRF_load(instructionPointer * ip)
{
	manager_add_opcode(DIRF, 'C', chdir)
	manager_add_opcode(DIRF, 'M', mkdir)
	manager_add_opcode(DIRF, 'R', rmdir)
	return true;
}

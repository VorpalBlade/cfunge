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

#include "DIRF.h"
#include "../../stack.h"

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void FingerDIRFchdir(instructionPointer * ip)
{
	char * restrict str;
	str = StackPopString(ip->stack);
	if (strlen(str) < 1) {
		ipReverse(ip);
		return;
	}
	if (chdir(str) != 0)
		ipReverse(ip);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

static void FingerDIRFmkdir(instructionPointer * ip)
{
	char * restrict str;
	str = StackPopString(ip->stack);
	if (strlen(str) < 1) {
		ipReverse(ip);
		return;
	}
	if (mkdir(str, S_IRWXU) != 0)
		ipReverse(ip);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}

static void FingerDIRFrmdir(instructionPointer * ip)
{
	char * restrict str;
	str = StackPopString(ip->stack);
	if (strlen(str) < 1) {
		ipReverse(ip);
		return;
	}
	if (rmdir(str) != 0)
		ipReverse(ip);
#ifdef DISABLE_GC
	cf_free(str);
#endif
}


bool FingerDIRFload(instructionPointer * ip)
{
	if (!OpcodeStackAdd(ip, 'C', &FingerDIRFchdir))
		return false;
	if (!OpcodeStackAdd(ip, 'M', &FingerDIRFmkdir))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &FingerDIRFrmdir))
		return false;
	return true;
}

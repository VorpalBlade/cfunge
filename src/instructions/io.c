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

#include "../global.h"
#include "io.h"
#include "../interpreter.h"
#include "../funge-space/funge-space.h"
#include "../vector.h"
#include "../rect.h"
#include "../stack.h"
#include "../ip.h"
#include "../settings.h"

#include <assert.h>
#include <stdbool.h>

void RunFileInput(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (SettingSandbox) {
		ipReverse(ip);
		return;
	}

	{
		char * filename;
		bool binary;
		fungePosition offset;
		fungeVector size;

		// Pop stuff.
		filename = StackPopString(ip->stack);

		// Sanity test!
		if (*filename == '\0') {
			ipReverse(ip);
			return;
		}

		binary = (bool)(StackPop(ip->stack) & 1);
		offset = StackPopVector(ip->stack);

		if (!fungeSpaceLoadAtOffset(filename,
		                            &(fungePosition){ .x = offset.x + ip->storageOffset.x,  .y = offset.y + ip->storageOffset.y },
		                            &size, binary))
			ipReverse(ip);

		StackPushVector(&size, ip->stack);
		StackPushVector(&offset, ip->stack);
	}
}

void RunFileOutput(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (SettingSandbox) {
		ipReverse(ip);
		return;
	}

	{
		char * filename;
		bool textfile;
		fungePosition offset;
		fungeVector size;

		// Pop stuff.
		filename = StackPopString(ip->stack);
		textfile = (bool)(StackPop(ip->stack) & 1);
		offset = StackPopVector(ip->stack);
		size = StackPopVector(ip->stack);

		// Sanity test!
		if (*filename == '\0' || size.x < 1 || size.y < 1) {
			ipReverse(ip);
			return;
		}

		if (!fungeSaveToFile(filename,
		                     &(fungePosition){ .x = offset.x + ip->storageOffset.x,  .y = offset.y + ip->storageOffset.y },
		                     &size, textfile))
			ipReverse(ip);
	}

}

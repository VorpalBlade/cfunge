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

#include "SCKE.h"
#include "../../stack.h"

#define FUNGE_EXTENDS_SOCK
#include "../SOCK/SOCK.h"

#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>


// TODO: Add code to template functions

/// H - Get address by hostname
static void FingerSCKEgetHostByName(instructionPointer * ip)
{
	char * restrict str;
	union {
		char bytes[4];
		int i;
	} resCell;
	struct hostent *result;
	str = StackPopString(ip->stack);

	result = gethostbyname(str);
	if (!result)
		goto error;
	// We can't handle IPv6 here...
	if (result->h_addrtype != AF_INET)
		goto error;
	// Should we use result->h_length here? Not sure...
	for (size_t i = 0; i < 4; i++)
		resCell.bytes[i] = result->h_addr_list[0][i];

	StackPush(ip->stack, resCell.i);

	goto end;
error:
	ipReverse(ip);
end:
	StackFreeString(str);
}

/// P - Peek for incoming data
static void FingerSCKEPeek(instructionPointer * ip)
{
	fungeCell s = StackPop(ip->stack);
	FungeSocketHandle* handle = FingerSOCKLookupHandle(s);
	if (!handle)
		goto error;

	{
		struct pollfd fds;
		int retval;
		fds.fd = handle->fd;
		fds.events = POLLIN;

		retval = poll(&fds, 1, 0);

		if (retval == -1)
			goto error;
		StackPush(ip->stack, retval);
	}

	return;
error:
	ipReverse(ip);
}

bool FingerSCKEload(instructionPointer * ip)
{
	ManagerAddOpcode(SCKE,  'H', getHostByName)
	ManagerAddOpcode(SCKE,  'P', Peek)
	return true;
}

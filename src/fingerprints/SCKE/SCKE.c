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

#include "SCKE.h"
#include "../../stack.h"

#define FUNGE_EXTENDS_SOCK
#include "../SOCK/SOCK.h"

#include <string.h> /* memset */

#include <sys/types.h>   /* getaddrinfo */
#include <sys/socket.h>  /* getaddrinfo */
#include <netinet/in.h>
#include <netdb.h>       /* getaddrinfo */
#include <poll.h>        /* poll */

/* Handle systems missing this. */
#ifndef AI_ADDRCONFIG
#  define AI_ADDRCONFIG 0
#endif

/// H - Get address by hostname
static void finger_SCKE_gethostbyname(instructionPointer * ip)
{
	char * restrict str = NULL;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	int retval;

	str = (char*)stack_pop_string(ip->stack, NULL);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_ADDRCONFIG;

	retval = getaddrinfo(str, NULL, &hints, &result);

	if (retval != 0)
		goto error;
	// We can't handle IPv6 here...
	if (result->ai_addr->sa_family != AF_INET)
		goto error;

	{
		struct sockaddr_in *addr = (struct sockaddr_in*)result->ai_addr;
		stack_push(ip->stack, (funge_cell)addr->sin_addr.s_addr);
	}
	goto end;
error:
	ip_reverse(ip);
end:
	stack_free_string(str);
	if (result)
		freeaddrinfo(result);
}

/// P - Peek for incoming data
static void finger_SCKE_peek(instructionPointer * ip)
{
	funge_cell s = stack_pop(ip->stack);
	FungeSocketHandle* handle = finger_SOCK_LookupHandle(s);
	if (!handle)
		goto error;

	{
		struct pollfd fds;
		int retval;
		fds.fd = handle->fd;
		fds.events = POLLIN;

		retval = poll(&fds, 1, 0);

		if (retval == -1) {
			goto error;
		} else if (retval == 0) {
			stack_push(ip->stack, 0);
		} else {
			if ((fds.revents & POLLIN) != 0)
				stack_push(ip->stack, 1);
			else
				goto error;
		}
	}

	return;
error:
	ip_reverse(ip);
}

bool finger_SCKE_load(instructionPointer * ip)
{
	manager_add_opcode(SCKE, 'H', gethostbyname)
	manager_add_opcode(SCKE, 'P', peek)
	return true;
}

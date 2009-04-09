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

// It need to get it's own typedefs from the header.
#define FUNGE_EXTENDS_SOCK

#include "SOCK.h"
#include "../../stack.h"

#include <unistd.h> /* close, fcntl */
#include <fcntl.h>  /* fcntl */

#include <sys/types.h>  /* accept, connect, socket, ... */
#include <sys/socket.h> /* accept, connect, socket, ... */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>  /* htons */
#include <netdb.h>

// Based on how CCBI does it.

typedef union {
	struct sockaddr_in in;
	struct sockaddr    gen;
} FungeSockAddr;

#define ALLOCCHUNK 2
// Array of pointers
static FungeSocketHandle** sockets = NULL;
static size_t maxHandle = 0;

/// Used by allocate_handle() below to find next free handle.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline funge_cell findNextfree_handle(void)
{
	for (size_t i = 0; i < maxHandle; i++) {
		if (sockets[i] == NULL)
			return (funge_cell)i;
	}
	// No free one, extend array..
	{
		FungeSocketHandle** newlist = (FungeSocketHandle**)cf_realloc(sockets, (maxHandle + ALLOCCHUNK) * sizeof(FungeSocketHandle*));
		if (!newlist)
			return -1;
		sockets = newlist;
		for (size_t i = maxHandle; i < (maxHandle + ALLOCCHUNK); i++)
			sockets[i] = NULL;
		maxHandle += ALLOCCHUNK;
		return (funge_cell)(maxHandle - ALLOCCHUNK);
	}
}

/// Get a new handle to use for a file, also allocates buffer for it.
/// @return Handle, or -1 on failure
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline funge_cell allocate_handle(void)
{
	funge_cell h;

	h = findNextfree_handle();
	if (h < 0)
		return -1;

	sockets[h] = cf_malloc(sizeof(FungeSocketHandle));
	if (!sockets[h])
		return -1;
	return h;
}

/// Free a handle. close() the file before calling this.
FUNGE_ATTR_FAST
static inline void free_handle(funge_cell h)
{
	if (!sockets[h])
		return;
	cf_free(sockets[h]);
	sockets[h] = NULL;
}

/// Checks if handle is valid.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool valid_handle(funge_cell h)
{
	if ((h < 0) || ((size_t)h >= maxHandle) || (!sockets[h])) {
		return false;
	} else {
		return true;
	}
}

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
FungeSocketHandle* finger_SOCK_LookupHandle(funge_cell h)
{
	if (!valid_handle(h))
		return NULL;
	return sockets[h];
}


static inline int popFam(instructionPointer * ip)
{
	switch (stack_pop(ip->stack)) {
		case 1:  return AF_UNIX;
		case 2:  return AF_INET;
		default: return AF_UNSPEC;
	}
}


/// A - Accept a connection
static void finger_SOCK_accept(instructionPointer * ip)
{
	funge_cell s = stack_pop(ip->stack);

	if (!valid_handle(s))
		goto error;

	{
		FungeSockAddr addr;
		socklen_t addrlen = sizeof(addr.in);
		int as;
		funge_cell i;

		addr.in.sin_addr.s_addr = 0;
		addr.in.sin_port = 0;
		addr.in.sin_family = AF_INET;

		as = accept(sockets[s]->fd, &addr.gen, &addrlen);
		if (as == -1)
			goto error;

		fcntl(as, F_SETFD, FD_CLOEXEC, 1);

		i = allocate_handle();
		if (i == -1)
			goto error;
		sockets[i]->fd = as;
		sockets[i]->family = sockets[s]->family;

		stack_push(ip->stack, addr.in.sin_port);
		stack_push(ip->stack, (funge_cell)addr.in.sin_addr.s_addr);
		stack_push(ip->stack, i);
	}
	return;
error:
	ip_reverse(ip);
}

/// B - Bind a socket
static void finger_SOCK_bind(instructionPointer * ip)
{
	uint32_t  address = (uint32_t)stack_pop(ip->stack);
	uint16_t  port    = (uint16_t)stack_pop(ip->stack);
	int       fam     = popFam(ip);
	funge_cell s       = stack_pop(ip->stack);
	FungeSockAddr addr;

	if (!valid_handle(s))
		goto error;

	switch (fam) {
		case AF_INET: {
			int retval;

			addr.in.sin_family = AF_INET;
			addr.in.sin_addr.s_addr = address;
			addr.in.sin_port = htons(port);

			retval = bind(sockets[s]->fd, &addr.gen, sizeof(addr.in));
			if (retval == -1)
				goto error;
			break;
		}
		default: goto error;
	}
	return;
error:
	ip_reverse(ip);
}

/// C - Open a connection
static void finger_SOCK_open(instructionPointer * ip)
{
	uint32_t  address = (uint32_t)stack_pop(ip->stack);
	uint16_t  port    = (uint16_t)stack_pop(ip->stack);
	int       fam     = popFam(ip);
	funge_cell s       = stack_pop(ip->stack);
	FungeSockAddr addr;

	if (!valid_handle(s))
		goto error;

	switch (fam) {
		case AF_INET: {
			int retval;

			addr.in.sin_family = AF_INET;
			addr.in.sin_addr.s_addr = address;
			addr.in.sin_port = htons(port);

			retval = connect(sockets[s]->fd, &addr.gen, sizeof(addr.in));
			if (retval == -1)
				goto error;

			break;
		}
		default: goto error;
	}
	return;
error:
	ip_reverse(ip);
}

/// I - Convert an ASCII IP address to a 32 bit address
static void finger_SOCK_fromascii(instructionPointer * ip)
{
	char * restrict str;
	struct in_addr addr;

	str = (char*)stack_pop_string(ip->stack, NULL);
	if (inet_pton(AF_INET, str, &addr) != 1) {
		ip_reverse(ip);
	} else {
		stack_push(ip->stack, (funge_cell)addr.s_addr);
	}
	stack_free_string(str);

}

/// K - Kill a connection
static void finger_SOCK_kill(instructionPointer * ip)
{
	funge_cell s       = stack_pop(ip->stack);
	if (!valid_handle(s))
		goto invalid;
	shutdown(sockets[s]->fd, SHUT_RDWR);
	if (close(sockets[s]->fd) == -1) {
		goto error;
	}
	free_handle(s);
	return;
error:
	free_handle(s);
invalid:
	ip_reverse(ip);
}

/// L - Set a socket to listening mode (n=backlog size)
static void finger_SOCK_listen(instructionPointer * ip)
{
	funge_cell s = stack_pop(ip->stack);
	int n = (int)stack_pop(ip->stack);

	if (!valid_handle(s))
		goto error;

	if (listen(sockets[s]->fd, n) == -1)
		goto error;
	return;
error:
	ip_reverse(ip);
}

/// O - Set socket option
static void finger_SOCK_setopt(instructionPointer * ip)
{
	int val;
	int o;

	funge_cell s = stack_pop(ip->stack);
	funge_cell t = stack_pop(ip->stack);

	val = (int)stack_pop(ip->stack);

	if (!valid_handle(s))
		goto error;

	switch (t) {
		case 1: o = SO_DEBUG;     break;
		case 2: o = SO_REUSEADDR; break;
		case 3: o = SO_KEEPALIVE; break;
		case 4: o = SO_DONTROUTE; break;
		case 5: o = SO_BROADCAST; break;
		case 6: o = SO_OOBINLINE; break;
		default: goto error;
	}

	{
		int retval;
		retval = setsockopt(sockets[s]->fd, SOL_SOCKET, o, &val, sizeof(int));
		if (retval == -1)
			goto error;
	}
	return;
error:
	ip_reverse(ip);
}

/// R - Receive from a socket
static void finger_SOCK_receive(instructionPointer * ip)
{
	unsigned char *buffer = NULL;
	ssize_t got;
	funge_cell s   = stack_pop(ip->stack);
	funge_cell len = stack_pop(ip->stack);

	funge_vector v = stack_pop_vector(ip->stack);

	if (len < 0)
		goto error;

	if (!valid_handle(s))
		goto error;

	v.x += ip->storageOffset.x;
	v.y += ip->storageOffset.y;

	buffer = cf_malloc_noptr((size_t)len * sizeof(unsigned char));

	got = recv(sockets[s]->fd, buffer, (size_t)len, 0);

	stack_push(ip->stack, (funge_cell)got);

	if (got == -1)
		goto error;

	for (ssize_t i = 0; i < got; ++i)
		fungespace_set(buffer[i], vector_create_ref(v.x + i, v.y));

	goto end;
error:
	ip_reverse(ip);
end:
	if (buffer)
		cf_free(buffer);
}

/// S - Create a socket
static void finger_SOCK_create(instructionPointer * ip)
{
	int type;
	int fam;
	// Protocol.
	stack_discard(ip->stack, 1);

	switch (stack_pop(ip->stack)) {
		case 1: type = SOCK_DGRAM ; break;
		case 2: type = SOCK_STREAM ; break;
		default: ip_reverse(ip); return;
	}

	fam = popFam(ip);

	if (fam == AF_UNSPEC)
		goto error;

	{
		funge_cell h = allocate_handle();
		if (h == -1) {
			goto error;
		}

		sockets[h]->fd = socket(fam, type, 0);
		if (sockets[h]->fd == -1)
			goto error;

		fcntl(sockets[h]->fd, F_SETFD, FD_CLOEXEC, 1);

		sockets[h]->family = fam;

		stack_push(ip->stack, h);
	}
	return;
error:
	ip_reverse(ip);
}

/// W - Write to a socket
static void finger_SOCK_write(instructionPointer * ip)
{
	unsigned char *buffer = NULL;
	ssize_t sent;
	funge_cell s   = stack_pop(ip->stack);
	funge_cell len = stack_pop(ip->stack);

	funge_vector v = stack_pop_vector(ip->stack);

	if (len < 0)
		goto error;

	if (!valid_handle(s))
		goto error;

	v.x += ip->storageOffset.x;
	v.y += ip->storageOffset.y;

	buffer = cf_malloc_noptr((size_t)len * sizeof(unsigned char));

	for (size_t i = 0; i < (size_t)len; ++i)
		buffer[i] = (unsigned char)fungespace_get(vector_create_ref(v.x + (funge_cell)i, v.y));

	sent = send(sockets[s]->fd, buffer, (size_t)len, 0);

	stack_push(ip->stack, (funge_cell)sent);

	if (sent == -1)
		goto error;

	goto end;
error:
	ip_reverse(ip);
end:
	if (buffer)
		cf_free(buffer);
}

FUNGE_ATTR_FAST static inline bool init_handle_list(void)
{
	sockets = (FungeSocketHandle**)cf_calloc(ALLOCCHUNK, sizeof(FungeSocketHandle*));
	if (!sockets)
		return false;
	maxHandle = ALLOCCHUNK;
	return true;
}

bool finger_SOCK_load(instructionPointer * ip)
{
	if (!sockets)
		if (!init_handle_list())
			return false;

	manager_add_opcode(SOCK, 'A', accept)
	manager_add_opcode(SOCK, 'B', bind)
	manager_add_opcode(SOCK, 'C', open)
	manager_add_opcode(SOCK, 'I', fromascii)
	manager_add_opcode(SOCK, 'K', kill)
	manager_add_opcode(SOCK, 'L', listen)
	manager_add_opcode(SOCK, 'O', setopt)
	manager_add_opcode(SOCK, 'R', receive)
	manager_add_opcode(SOCK, 'S', create)
	manager_add_opcode(SOCK, 'W', write)
	return true;
}

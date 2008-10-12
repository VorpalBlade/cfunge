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

// It need to get it's own typedefs from the header.
#define FUNGE_EXTENDS_SOCK

#include "SOCK.h"
#include "../../stack.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
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

/// Used by AllocateHandle() below to find next free handle.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell findNextFreeHandle(void)
{
	for (size_t i = 0; i < maxHandle; i++) {
		if (sockets[i] == NULL)
			return i;
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
		return (maxHandle - ALLOCCHUNK);
	}
}

/// Get a new handle to use for a file, also allocates buffer for it.
/// @return Handle, or -1 on failure
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline fungeCell AllocateHandle(void)
{
	fungeCell h;

	h = findNextFreeHandle();
	if (h < 0)
		return -1;

	sockets[h] = cf_malloc(sizeof(FungeSocketHandle));
	if (!sockets[h])
		return -1;
	return h;
}

/// Free a handle. fclose() the file before calling this.
FUNGE_ATTR_FAST
static inline void FreeHandle(fungeCell h)
{
	if (!sockets[h])
		return;
	cf_free(sockets[h]);
	sockets[h] = NULL;
}

/// Checks if handle is valid.
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool ValidHandle(fungeCell h)
{
	if ((h < 0) || ((size_t)h >= maxHandle) || (!sockets[h])) {
		return false;
	} else {
		return true;
	}
}

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
FungeSocketHandle* FingerSOCKLookupHandle(fungeCell h)
{
	if (!ValidHandle(h))
		return NULL;
	return sockets[h];
}


static inline int popFam(instructionPointer * ip) {
	switch (stack_pop(ip->stack)) {
		case 1:  return AF_UNIX;
		case 2:  return AF_INET;
		default: return AF_UNSPEC;
	}
}


/// A - Accept a connection
static void FingerSOCKaccept(instructionPointer * ip)
{
	fungeCell s = stack_pop(ip->stack);

	if (!ValidHandle(s))
		goto error;

	{
		FungeSockAddr addr;
		socklen_t addrlen = sizeof(addr.in);
		int as, i;

		addr.in.sin_addr.s_addr=0;
		addr.in.sin_port=0;
		addr.in.sin_family=AF_INET;

		as = accept(sockets[s]->fd, &addr.gen, &addrlen);

		i = AllocateHandle();
		if (i == -1)
			goto error;
		sockets[i]->fd = as;
		sockets[i]->family = sockets[s]->family;

		stack_push(ip->stack, addr.in.sin_port);
		stack_push(ip->stack, addr.in.sin_addr.s_addr);
		stack_push(ip->stack, i);
	}
	return;
error:
	ip_reverse(ip);
}

/// B - Bind a socket
static void FingerSOCKbind(instructionPointer * ip)
{
	uint32_t  address = (uint32_t)stack_pop(ip->stack);
	uint16_t  port    = (uint16_t)stack_pop(ip->stack);
	int       fam     = popFam(ip);
	fungeCell s       = stack_pop(ip->stack);
	FungeSockAddr addr;

	if (!ValidHandle(s))
		goto error;

	switch (fam) {
		case AF_INET: {
			int retval;

			addr.in.sin_family = AF_INET;
			addr.in.sin_addr.s_addr = address;
			addr.in.sin_port=htons(port);

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
static void FingerSOCKopen(instructionPointer * ip)
{
	uint32_t  address = (uint32_t)stack_pop(ip->stack);
	uint16_t  port    = (uint16_t)stack_pop(ip->stack);
	int       fam     = popFam(ip);
	fungeCell s       = stack_pop(ip->stack);
	FungeSockAddr addr;

	if (!ValidHandle(s))
		goto error;

	switch (fam) {
		case AF_INET: {
			int retval;

			addr.in.sin_family = AF_INET;
			addr.in.sin_addr.s_addr = address;
			addr.in.sin_port=htons(port);

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
static void FingerSOCKfromascii(instructionPointer * ip)
{
	char * restrict str;
	struct in_addr addr;

	str = stack_pop_string(ip->stack);
	if (inet_pton(AF_INET, str, &addr) != 1) {
		ip_reverse(ip);
	} else {
		stack_push(ip->stack, addr.s_addr);
	}
	stack_freeString(str);

}

/// K - Kill a connection
static void FingerSOCKkill(instructionPointer * ip)
{
	fungeCell s       = stack_pop(ip->stack);
	if (!ValidHandle(s))
		goto invalid;
	shutdown(sockets[s]->fd, SHUT_RDWR);
	if (close(sockets[s]->fd) == -1) {
		goto error;
	}
	FreeHandle(s);
	return;
error:
	FreeHandle(s);
invalid:
	ip_reverse(ip);
}

/// L - Set a socket to listening mode (n=backlog size)
static void FingerSOCKlisten(instructionPointer * ip)
{
	fungeCell s = stack_pop(ip->stack);
	int n = stack_pop(ip->stack);

	if (!ValidHandle(s))
		goto error;

	if (listen(sockets[s]->fd, n) == -1)
		goto error;
	return;
error:
	ip_reverse(ip);
}

/// O - Set socket option
static void FingerSOCKsetopt(instructionPointer * ip)
{
	int val;
	int o;

	fungeCell s = stack_pop(ip->stack);
	fungeCell t = stack_pop(ip->stack);

	val = stack_pop(ip->stack);

	if (!ValidHandle(s))
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
static void FingerSOCKreceive(instructionPointer * ip)
{
	unsigned char *buffer = NULL;
	ssize_t got;
	fungeCell s   = stack_pop(ip->stack);
	size_t    len = stack_pop(ip->stack);

	fungeVector v = stack_pop_vector(ip->stack);
	v.x += ip->storageOffset.x;
	v.y += ip->storageOffset.y;

	if (!ValidHandle(s))
		goto error;

	buffer = cf_malloc_noptr(len * sizeof(char));

	got = recv(sockets[s]->fd, buffer, len, 0);

	stack_push(ip->stack, got);

	if (got == -1)
		goto error;

	for (ssize_t i = 0; i < got; ++i)
		fungespace_set(buffer[i], vector_create_ref(v.x+i, v.y));

	goto end;
error:
	ip_reverse(ip);
end:
	if (buffer)
		cf_free(buffer);
}

/// S - Create a socket
static void FingerSOCKcreate(instructionPointer * ip)
{
	int type;
	int fam;
	// Protocol.
	stack_pop_discard(ip->stack);

	switch (stack_pop(ip->stack)) {
		case 1: type = SOCK_DGRAM ; break;
		case 2: type = SOCK_STREAM ; break;
		default: ip_reverse(ip); return;
	}

	fam = popFam(ip);

	if (fam == AF_UNSPEC)
		goto error;

	{
		fungeCell h = AllocateHandle();
		if (h == -1) {
			goto error;
		}

		sockets[h]->fd = socket(fam, type, 0);
		if (sockets[h]->fd == -1)
			goto error;

		sockets[h]->family = fam;

		stack_push(ip->stack, h);
	}
	return;
error:
	ip_reverse(ip);
}

/// W - Write to a socket
static void FingerSOCKwrite(instructionPointer * ip)
{
	unsigned char *buffer = NULL;
	ssize_t sent;
	fungeCell s   = stack_pop(ip->stack);
	size_t    len = stack_pop(ip->stack);

	fungeVector v = stack_pop_vector(ip->stack);
	v.x += ip->storageOffset.x;
	v.y += ip->storageOffset.y;

	if (!ValidHandle(s))
		goto error;

	buffer = cf_malloc_noptr(len * sizeof(char));

	for (size_t i = 0; i < len; ++i)
		buffer[i] = fungespace_get(vector_create_ref(v.x+i, v.y));

	sent = send(sockets[s]->fd, buffer, len, 0);

	stack_push(ip->stack, sent);

	if (sent == -1)
		goto error;

	goto end;
error:
	ip_reverse(ip);
end:
	if (buffer)
		cf_free(buffer);
}

FUNGE_ATTR_FAST static inline bool InitHandleList(void)
{
	sockets = (FungeSocketHandle**)cf_calloc(ALLOCCHUNK, sizeof(FungeSocketHandle*));
	if (!sockets)
		return false;
	maxHandle = ALLOCCHUNK;
	return true;
}

bool FingerSOCKload(instructionPointer * ip)
{
	if (!sockets)
		if (!InitHandleList())
			return false;

	manager_add_opcode(SOCK,  'A', accept)
	manager_add_opcode(SOCK,  'B', bind)
	manager_add_opcode(SOCK,  'C', open)
	manager_add_opcode(SOCK,  'I', fromascii)
	manager_add_opcode(SOCK,  'K', kill)
	manager_add_opcode(SOCK,  'L', listen)
	manager_add_opcode(SOCK,  'O', setopt)
	manager_add_opcode(SOCK,  'R', receive)
	manager_add_opcode(SOCK,  'S', create)
	manager_add_opcode(SOCK,  'W', write)
	return true;
}

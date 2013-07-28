/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <anmaster AT tele2 DOT se>
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


#include "global.h"
#include "ip.h"

#ifdef LARGE_IPLIST
#  define CFUNGE_MEMPOOL_IPS
#  include "../lib/mempool/cfunge_mempool.h"
#endif

#include "diagnostic.h"
#include "interpreter.h"
#include "settings.h"
#include "stack.h"
#include "vector.h"

#include "fingerprints/manager.h"
#include "funge-space/funge-space.h"

#include <assert.h>
#include <string.h> /* memcpy */

/// For concurrent funge: how many new IPs to allocate in one go?
#ifdef LARGE_IPLIST
#  define ALLOCCHUNKSIZE 256
#else
#  define ALLOCCHUNKSIZE 32
#endif

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline bool ip_create_in_place(instructionPointer *me)
{
	assert(me != NULL);
	me->position.x           = 0;
	me->position.y           = 0;
	me->delta.x              = 1;
	me->delta.y              = 0;
	me->storageOffset.x      = 0;
	me->storageOffset.y      = 0;
	me->mode                 = ipmCODE;
	me->needMove             = true;
	me->stringLastWasSpace   = false;
	me->fingerSUBRisRelative = false;
	me->stackstack           = stackstack_create();
	if (FUNGE_UNLIKELY(!me->stackstack))
		return false;
	me->stack                = me->stackstack->stacks[me->stackstack->current];
	me->ID                   = 0;
	// Zero the opcode stacks if needed.
	if (FUNGE_LIKELY(!setting_disable_fingerprints)) {
		memset(me->fingerOpcodes, 0, sizeof(fungeOpcodeStack) * FINGEROPCODECOUNT);
	}
	me->fingerHRTItimestamp  = NULL;
	return true;
}

#ifndef CONCURRENT_FUNGE
instructionPointer * ip_create(void)
{
	instructionPointer * tmp = (instructionPointer*)malloc(sizeof(instructionPointer));
	if (FUNGE_UNLIKELY(!tmp))
		return NULL;
	if (FUNGE_UNLIKELY(!ip_create_in_place(tmp))) {
		free(tmp);
		return NULL;
	}
	return tmp;
}
#endif

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline bool ip_duplicate_in_place(const instructionPointer * restrict old, instructionPointer * restrict new)
{
	assert(old != NULL);
	assert(new != NULL);
	memcpy(new, old, sizeof(instructionPointer));

	new->stackstack = stackstack_duplicate(old->stackstack);
	if (FUNGE_UNLIKELY(!new->stackstack)) {
		// We need to clear out pointers in the IP to avoid double free when we exit
		memset(new, 0, sizeof(instructionPointer));
		return false;
	}

	new->stack = new->stackstack->stacks[new->stackstack->current];
	if (FUNGE_LIKELY(!setting_disable_fingerprints)) {
		manager_duplicate(old, new);
	}
	new->fingerHRTItimestamp  = NULL;
	return true;
}
#endif

#if defined(CONCURRENT_FUNGE) || !defined(NDEBUG)
FUNGE_ATTR_FAST static inline void ip_free_resources(instructionPointer * ip)
{
	if (FUNGE_UNLIKELY(!ip))
		return;
	if (FUNGE_LIKELY(ip->stackstack)) {
		stackstack_free(ip->stackstack);
		ip->stackstack = NULL;
	}
	ip->stack = NULL;
	if (FUNGE_LIKELY(!setting_disable_fingerprints)) {
		manager_free(ip);
	}
	if (ip->fingerHRTItimestamp) {
		free(ip->fingerHRTItimestamp);
		ip->fingerHRTItimestamp = NULL;
	}
#  ifdef LARGE_IPLIST
	cf_mempool_ip_free(ip);
#  endif
}
#endif

#if !defined(CONCURRENT_FUNGE) && !defined(NDEBUG)
FUNGE_ATTR_FAST void ip_free(instructionPointer * restrict ip)
{
	if (!ip)
		return;
	ip_free_resources(ip);
	free(ip);
}
#endif

FUNGE_ATTR_FAST inline void ip_set_position(instructionPointer * restrict ip, const funge_vector * restrict position)
{
	assert(ip != NULL);
	assert(position != NULL);
	ip->position.x = position->x;
	ip->position.y = position->y;
	fungespace_wrap(&ip->position, &ip->delta);
}


/***********
 * IP list *
 ***********/

#ifdef CONCURRENT_FUNGE
ipList* iplist_create(void)
{
	ipList *list;

#ifdef LARGE_IPLIST
	list = malloc(sizeof(ipList) + sizeof(instructionPointer*) * ALLOCCHUNKSIZE);
	if (FUNGE_UNLIKELY(!list))
		return NULL;

	if (FUNGE_UNLIKELY(!cf_mempool_ip_setup()))
		return NULL;

	list->ips[0] = cf_mempool_ip_alloc();
	if (FUNGE_UNLIKELY(!list->ips[0]))
		return NULL;

	if (FUNGE_UNLIKELY(!ip_create_in_place(list->ips[0])))
		return NULL;
#else
	list = malloc(sizeof(ipList) + sizeof(instructionPointer) * ALLOCCHUNKSIZE);
	if (FUNGE_UNLIKELY(!list))
		return NULL;
	if (FUNGE_UNLIKELY(!ip_create_in_place(&list->ips[0])))
		return NULL;
#endif
	list->size = ALLOCCHUNKSIZE;
	list->top = 0;
	list->highestID = 0;
	return list;
}

#ifndef NDEBUG
FUNGE_ATTR_FAST void iplist_free(ipList* me)
{
	if (FUNGE_UNLIKELY(!me))
		return;
	for (size_t i = 0; i <= me->top; i++) {
#  ifdef LARGE_IPLIST
		ip_free_resources(me->ips[i]);
#  else
		ip_free_resources(&me->ips[i]);
#  endif
	}
	free(me);
#  ifdef LARGE_IPLIST
	cf_mempool_ip_teardown();
#  endif
}
#endif

FUNGE_ATTR_FAST ssize_t iplist_duplicate_ip(ipList** me, size_t index)
{
	ipList *list;

	assert(me != NULL);
	assert(*me != NULL);
	assert(index <= (*me)->top);

	list = *me;

	// Grow if needed
	if (list->size <= (list->top + 1)) {
#ifdef LARGE_IPLIST
		list = (ipList*)realloc(*me, sizeof(ipList) + sizeof(instructionPointer*) * ((*me)->size + ALLOCCHUNKSIZE));
#else
		list = (ipList*)realloc(*me, sizeof(ipList) + sizeof(instructionPointer) * ((*me)->size + ALLOCCHUNKSIZE));
#endif
		if (FUNGE_UNLIKELY(!list))
			return -1;
		*me = list;
		list->size += ALLOCCHUNKSIZE;
	}
	/*
	 *  Splitting examples.
	 *
	 *  Thread index 3 splits (to 3a)
	 *  0  | 1  | 2  | 3  | 4   | 5  | 6
	 *  ---------------------------------
	 *  t0 | t1 | t3 | t3 | t4  | t5 |
	 *  t0 | t1 | t2 | t3 | t3a | t4 | t5
	 *
	 */
	// Do we need to move any upwards?
	if (index != list->top) {
		/* Move upwards:
		 * t0 | t1 | t2 |
		 * t10|    | t1 | t2
		 */
		for (size_t i = list->top + 1; i > index; i--) {
			list->ips[i] = list->ips[i - 1];
		}
	}
	/* Duplicate:
	 * t0 | t1  | t2 |
	 * t0 | t0a | t1 | t2
	 */
#ifdef LARGE_IPLIST
	list->ips[index + 1] = cf_mempool_ip_alloc();
	if (FUNGE_UNLIKELY(!list->ips[index + 1])) {
		// We are in trouble
		DIAG_OOM("Could not allocate IP resources.");
	}
	if (FUNGE_UNLIKELY(!ip_duplicate_in_place(list->ips[index], list->ips[index + 1]))) {
		// We are in trouble
		DIAG_OOM("Could not duplicate IP resources.");
	}
#else
	if (FUNGE_UNLIKELY(!ip_duplicate_in_place(&list->ips[index], &list->ips[index + 1]))) {
		// We are in trouble
		DIAG_OOM("Could not duplicate IP resources.");
	}
#endif

	// Here we mirror new IP and do ID changes.
	index++;
#ifdef LARGE_IPLIST
	ip_reverse(list->ips[index]);
	ip_forward(list->ips[index]);
	list->ips[index]->ID = ++list->highestID;
#else
	ip_reverse(&list->ips[index]);
	ip_forward(&list->ips[index]);
	list->ips[index].ID = ++list->highestID;
#endif
	list->top++;
	return index - 1;
}


FUNGE_ATTR_FAST ssize_t iplist_terminate_ip(ipList** me, size_t index)
{
	ipList *list;

	assert(me != NULL);
	assert(*me != NULL);

	list = *me;

	/*
	 *  Terminate examples.
	 *
	 *  Thread index 3 dies
	 *  0  | 1  | 2  | 3  | 4  | 5
	 *  ---------------------------
	 *  t0 | t1 | t2 | t3 | t4 | t5
	 *  t0 | t1 | t3 | t4 | t5 |
	 *
	 */
#ifdef LARGE_IPLIST
	ip_free_resources(list->ips[index]);
#else
	ip_free_resources(&list->ips[index]);
#endif
	// Do we need to move downwards?
	if (index != list->top) {
		/* Move downwards:
		 * t0 |    | t2 | t3
		 * t0 | t2 | t3 |
		 */
		for (size_t i = index; i < list->top; i++) {
			list->ips[i] = list->ips[i + 1];
		}
	}
	// Set stack to be invalid in the top one. This should help catch any bugs
	// related to this. For large model we instead sets the top IP to NULL.
#ifdef LARGE_IPLIST
	list->ips[list->top] = NULL;
#else
	list->ips[list->top].stackstack = NULL;
	list->ips[list->top].stack = NULL;
#endif
	list->top--;
	// TODO: Shrink if difference is large
#if 0
	if ((list->size - ALLOCCHUNKSIZE) > list->top) {
		ipList *tmp;
		tmp = (ipList*)realloc(list, sizeof(ipList) + (list->size - ALLOCCHUNKSIZE) * sizeof(instructionPointer));
		if (tmp) {
			*me = tmp;
			tmp->size - ALLOCCHUNKSIZE;
		}
	}
#endif
	return (index > 0) ? index - 1 : 0;
}

#endif

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

#include "global.h"
#include "stack.h"
#include "vector.h"
#include "ip.h"
#include "settings.h"
#include <assert.h>
#include <string.h>
#ifndef DISABLE_GC
#  include <gc/cord.h>
#  include <gc/ec.h>
#endif

// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 32

/******************************
 * Constructor and destructor *
 ******************************/

FUNGE_FAST fungeStack * StackCreate(void)
{
	fungeStack * tmp = (fungeStack*)cf_malloc(sizeof(fungeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (FUNGEDATATYPE*)cf_malloc_noptr(ALLOCCHUNKSIZE * sizeof(FUNGEDATATYPE));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	return tmp;
}

FUNGE_FAST void StackFree(fungeStack * stack)
{
	if (!stack)
		return;
	if (stack->entries) {
		cf_free(stack->entries);
		stack->entries = NULL;
	}
	cf_free(stack);
}

#ifdef CONCURRENT_FUNGE
// Used for concurrency
__attribute__((malloc, nonnull, warn_unused_result, FUNGE_IN_FAST))
FUNGE_FAST static inline fungeStack * StackDuplicate(const fungeStack * old)
{
	fungeStack * tmp = (fungeStack*)cf_malloc(sizeof(fungeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (FUNGEDATATYPE*)cf_malloc_noptr((old->top + 1) * sizeof(FUNGEDATATYPE));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = old->top + 1;
	tmp->top = old->top;
	for (size_t i = 0; i < tmp->top; i++)
		tmp->entries[i] = old->entries[i];
	return tmp;
}
#endif


/************************
 * Basic push/pop/peeks *
 ************************/

FUNGE_FAST void StackPush(fungeStack * restrict stack, FUNGEDATATYPE value)
{
	assert(stack != NULL);
	assert(stack->top <= stack->size);

	// Do we need to realloc?
	if (stack->top == stack->size) {
		stack->entries = (FUNGEDATATYPE*)cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(FUNGEDATATYPE));
		if (!stack->entries) {
			perror("Emergency! Failed to allocate enough memory for new stack items");
			abort();
		}
		stack->entries[stack->top] = value;
		stack->top++;
		stack->size += ALLOCCHUNKSIZE;
	} else {
		stack->entries[stack->top] = value;
		stack->top++;
	}
}

FUNGE_FAST inline FUNGEDATATYPE StackPop(fungeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else {
		FUNGEDATATYPE tmp = stack->entries[stack->top - 1];
		stack->top--;
		return tmp;
	}
}

FUNGE_FAST void StackPopDiscard(fungeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

FUNGE_FAST void StackPopNDiscard(fungeStack * restrict stack, size_t n)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return;
	} else {
		if (stack->top > n)
			stack->top -= n;
		else
			stack->top = 0;
	}
}


FUNGE_FAST FUNGEDATATYPE StackPeek(const fungeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else {
		return stack->entries[stack->top - 1];
	}
}


/********************************
 * Push and pop for data types. *
 ********************************/

FUNGE_FAST void StackPushVector(fungeStack * restrict stack, const fungeVector * restrict value)
{
	// TODO: Optimize
	StackPush(stack, value->x);
	StackPush(stack, value->y);
}

FUNGE_FAST fungeVector StackPopVector(fungeStack * restrict stack)
{
	// TODO Optimize
	FUNGEVECTORTYPE x, y;
	y = StackPop(stack);
	x = StackPop(stack);
	return (fungeVector) { .x = x, .y = y };
}

FUNGE_FAST void StackPushString(fungeStack * restrict stack, const char * restrict str, size_t len)
{
	assert(str != NULL);
	assert(stack != NULL);

	for (ssize_t i = len; i >= 0; i--)
		StackPush(stack, str[i]);
}

FUNGE_FAST char *StackPopString(fungeStack * restrict stack)
{
	FUNGEDATATYPE c;
#ifndef DISABLE_GC
	CORD_ec x;

	CORD_ec_init(x);
	while ((c = StackPop(stack)) != '\0')
		CORD_ec_append(x, (char)c);
	CORD_ec_append(x, '\0');

	return CORD_to_char_star(CORD_ec_to_cord(x));
#else
	size_t index = 0;
	// This may very likely be more than is needed. But this is only used in
	// case GC is disabled, and that is unsupported anyway.
	char * x = (char*)cf_malloc_noptr((stack->top + 1) * sizeof(char));

	while ((c = StackPop(stack)) != '\0') {
		x[index] = (char)c;
		index++;
	}
	x[index] = '\0';
	return x;
#endif
}

FUNGE_FAST char *StackPopSizedString(fungeStack * restrict stack, size_t len)
{
	char * x = (char*)cf_malloc_noptr((len + 1) * sizeof(char));

	for (size_t i = 0; i < len; i++) {
		x[i] = StackPop(stack);
	}
	x[len + 1] = '\0';
	return x;
}


/***************
 * Other stuff *
 ***************/
FUNGE_FAST void StackDupTop(fungeStack * restrict stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE tmp;

	tmp = StackPeek(stack);
	StackPush(stack, tmp);
	if (stack->top == 1)
		StackPush(stack, 0);
}

FUNGE_FAST void StackSwapTop(fungeStack * restrict stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE a, b;
	a = StackPop(stack);
	b = StackPop(stack);
	StackPush(stack, a);
	StackPush(stack, b);
}


#ifndef NDEBUG
/*************
 * Debugging *
 *************/


// For use with call in gdb
void StackDump(const fungeStack * stack) __attribute__((unused));

void StackDump(const fungeStack * stack)
{
	if (!stack)
		return;
	fprintf(stderr, "%zu elements:\n", stack->top);
	for (size_t i = 0; i < stack->top; i++)
		fprintf(stderr, "%" FUNGEDATAPRI " ", stack->entries[i]);
	fputs("\n", stderr);
}

#endif


/****************
 * Stack-stacks *
 ****************/

FUNGE_FAST fungeStackStack * StackStackCreate(void)
{
	fungeStackStack * stackStack;
	fungeStack      * stack;

	stackStack = (fungeStackStack*)cf_malloc(sizeof(fungeStackStack) + sizeof(fungeStack*));
	if (!stackStack)
		return NULL;

	stack = StackCreate();
	if (!stack)
		return NULL;

	stackStack->size = 1;
	stackStack->current = 0;
	stackStack->stacks[0] = stack;
	return stackStack;
}

FUNGE_FAST void StackStackFree(fungeStackStack * me)
{
	if (!me)
		return;

	for (size_t i = 0; i < me->size; i++)
		StackFree(me->stacks[i]);

	cf_free(me);
}

#ifdef CONCURRENT_FUNGE
FUNGE_FAST fungeStackStack * StackStackDuplicate(const fungeStackStack * restrict old)
{
	fungeStackStack * stackStack;

	assert(old != NULL);

	stackStack = (fungeStackStack*)cf_malloc(sizeof(fungeStackStack) + old->size * sizeof(fungeStack*));
	if (!stackStack)
		return NULL;

	for (size_t i = 0; i <= old->current; i++) {
		stackStack->stacks[i] = StackDuplicate(old->stacks[i]);
		if (!stackStack->stacks[i])
			return NULL;
	}

	stackStack->size = old->size;
	stackStack->current = old->current;
	return stackStack;
}
#endif


FUNGE_FAST static void OOMstackStack(const instructionPointer * restrict ip)
{
	if (SettingWarnings) {
		fprintf(stderr,
		        "WARN: Out of memory in stack-stack routine at x=%" FUNGEVECTORPRI " y=%" FUNGEVECTORPRI ". Reflecting.\n",
		        ip->position.x, ip->position.y);
	}
	// Lets hope.
#ifndef DISABLE_GC
	gc_collect_full();
#endif
}

FUNGE_FAST bool StackStackBegin(instructionPointer * restrict ip, fungeStackStack ** me, FUNGEDATATYPE count, const fungePosition * restrict storageOffset)
{
	fungeStackStack *stackStack;
	fungeStack      *TOSS, *SOSS;
	FUNGEDATATYPE * entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);
	assert(storageOffset != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc(sizeof(FUNGEDATATYPE) * (count + 1));
		// Reflect on out of memory, do it here before we mess up stuff.
		if (!entriesCopy) {
			OOMstackStack(ip);
			return false;
		}
	}

	// Set up variables
	stackStack = *me;

	TOSS = StackCreate();
	if (!TOSS) {
		if (entriesCopy)
			cf_free(entriesCopy);
		return false;
	}

	// Extend by one
	stackStack = (fungeStackStack*)cf_realloc(*me, sizeof(fungeStackStack) + ((*me)->size + 1) * sizeof(fungeStack*));
	if (!stackStack) {
		if (entriesCopy)
			cf_free(entriesCopy);
		return false;
	}
	*me = stackStack;
	SOSS = stackStack->stacks[stackStack->current];

	stackStack->size++;
	stackStack->current++;
	stackStack->stacks[stackStack->current] = TOSS;

	if (count > 0) {
		FUNGEDATATYPE i = count;
		while (i--)
			entriesCopy[i] = StackPop(SOSS);
		for (i = 0; i < count; i++)
			StackPush(TOSS, entriesCopy[i]);
	} else if (count < 0) {
		while (count++)
			StackPush(SOSS, 0);
	}
	StackPushVector(SOSS, &ip->storageOffset);
	ip->storageOffset.x = storageOffset->x;
	ip->storageOffset.y = storageOffset->y;
	ip->stack = TOSS;
	ip->stackstack = stackStack;
	if (entriesCopy)
		cf_free(entriesCopy);
	return true;
}


FUNGE_FAST bool StackStackEnd(instructionPointer * restrict ip, fungeStackStack ** me, FUNGEDATATYPE count)
{
	fungeStack      *TOSS, *SOSS;
	fungeStackStack *stackStack;
	fungePosition    storageOffset;
	FUNGEDATATYPE * entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc(sizeof(FUNGEDATATYPE) * (count + 1));
		// Reflect on out of memory, do it here before we mess up stuff.
		if (!entriesCopy) {
			OOMstackStack(ip);
			return false;
		}
	}

	// Set up variables
	stackStack = *me;
	TOSS = stackStack->stacks[stackStack->current];
	SOSS = stackStack->stacks[stackStack->current - 1];
	storageOffset = StackPopVector(SOSS);
	// TODO: Transfer data back here
	if (count > 0) {
		// Dynamic array (C99) to copy elements into, because order must be preserved.
		FUNGEDATATYPE i = count;
		while (i--)
			entriesCopy[i] = StackPop(TOSS);
		for (i = 0; i < count; i++)
			StackPush(SOSS, entriesCopy[i]);
	} else if (count < 0) {
		while (count++)
			StackPopDiscard(SOSS);
	}
	ip->storageOffset.x = storageOffset.x;
	ip->storageOffset.y = storageOffset.y;

	ip->stack = SOSS;
	// Make it one smaller
	stackStack = (fungeStackStack*)cf_realloc(*me, sizeof(fungeStackStack) + ((*me)->size - 1) * sizeof(fungeStack*));
	if (!stackStack) {
		if (entriesCopy)
			cf_free(entriesCopy);
		return false;
	}
	stackStack->size--;
	stackStack->current--;
	*me = stackStack;
	ip->stackstack = stackStack;
	StackFree(TOSS);
	if (entriesCopy)
		cf_free(entriesCopy);
	return true;
}


FUNGE_FAST void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS)
{
	assert(TOSS != NULL);
	assert(SOSS != NULL);
	assert(TOSS != SOSS);

	if (count > 0) {
		while (count--) {
			StackPush(TOSS, StackPop(SOSS));
		}
	} else if (count < 0) {
		while (count++) {
			StackPush(SOSS, StackPop(TOSS));
		}
	}
}

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

/// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 4096

/******************************
 * Constructor and destructor *
 ******************************/

FUNGE_ATTR_FAST fungeStack * StackCreate(void)
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

FUNGE_ATTR_FAST void StackFree(fungeStack * stack)
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
FUNGE_ATTR_FAST FUNGE_ATTR_MALLOC FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline fungeStack * StackDuplicate(const fungeStack * old)
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

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_NORET
static void StackOOM(void)
{
	perror("Emergency! Failed to allocate enough memory for new stack items");
	abort();
}

/*************************************
 * Basic push/pop/peeks and prealloc *
 *************************************/

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void StackPreallocSpace(fungeStack * restrict stack, size_t minfree)
{
	if ((stack->top + minfree) >= stack->size) {
		size_t newsize = stack->size + minfree;
		// Round upwards to whole ALLOCCHUNKSIZEed blocks.
		newsize += ALLOCCHUNKSIZE - (newsize % ALLOCCHUNKSIZE);
		stack->entries = (FUNGEDATATYPE*)cf_realloc(stack->entries, newsize * sizeof(FUNGEDATATYPE));
		if (!stack->entries) {
			StackOOM();
		}
		stack->size = newsize;
	}
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void StackPushNoCheck(fungeStack * restrict stack, FUNGEDATATYPE value)
{
	// This should only be used if that is true...
	assert(stack->top < stack->size);
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST void StackPush(fungeStack * restrict stack, FUNGEDATATYPE value)
{
	assert(stack != NULL);
	assert(stack->top <= stack->size);

	// Do we need to realloc?
	if (stack->top == stack->size) {
		stack->entries = (FUNGEDATATYPE*)cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(FUNGEDATATYPE));
		if (!stack->entries) {
			StackOOM();
		}
		stack->size += ALLOCCHUNKSIZE;
	}
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST inline FUNGEDATATYPE StackPop(fungeStack * restrict stack)
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

FUNGE_ATTR_FAST void StackPopDiscard(fungeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

FUNGE_ATTR_FAST void StackPopNDiscard(fungeStack * restrict stack, size_t n)
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


FUNGE_ATTR_FAST inline FUNGEDATATYPE StackPeek(const fungeStack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else {
		return stack->entries[stack->top - 1];
	}
}


FUNGE_ATTR_FAST inline FUNGEDATATYPE StackGetIndex(const fungeStack * restrict stack, size_t index)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else if (stack->top <= index) {
		return 0;
	} else {
		return stack->entries[index - 1];
	}
}


/********************************
 * Push and pop for data types. *
 ********************************/

FUNGE_ATTR_FAST void StackPushVector(fungeStack * restrict stack, const fungeVector * restrict value)
{
	// TODO: Optimise
	StackPush(stack, value->x);
	StackPush(stack, value->y);
}

FUNGE_ATTR_FAST fungeVector StackPopVector(fungeStack * restrict stack)
{
	// TODO: Optimise
	FUNGEVECTORTYPE x, y;
	y = StackPop(stack);
	x = StackPop(stack);
	return (fungeVector) { .x = x, .y = y };
}

FUNGE_ATTR_FAST void StackPushString(fungeStack * restrict stack, const char * restrict str, size_t len)
{
	assert(str != NULL);
	assert(stack != NULL);
	// Increment it once or it won't work
	len++;
	StackPreallocSpace(stack, len);
	while (len-- > 0)
		StackPushNoCheck(stack, str[len]);
}

FUNGE_ATTR_FAST char *StackPopString(fungeStack * restrict stack)
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

FUNGE_ATTR_FAST char *StackPopSizedString(fungeStack * restrict stack, size_t len)
{
	char * restrict x = (char*)cf_malloc_noptr((len + 1) * sizeof(char));

	for (size_t i = 0; i < len; i++) {
		x[i] = StackPop(stack);
	}
	x[len + 1] = '\0';
	return x;
}


/***************
 * Other stuff *
 ***************/
FUNGE_ATTR_FAST void StackDupTop(fungeStack * restrict stack)
{
	// TODO: Optimise instead of doing it this way
	FUNGEDATATYPE tmp;

	tmp = StackPeek(stack);
	StackPush(stack, tmp);
	// If it was empty, push a second zero.
	if (stack->top == 1)
		StackPush(stack, 0);
}

FUNGE_ATTR_FAST void StackSwapTop(fungeStack * restrict stack)
{
	// TODO: Optimise instead of doing it this way
	FUNGEDATATYPE a, b;
	// Well this have to work logically...
	a = StackPop(stack);
	b = StackPop(stack);
	StackPreallocSpace(stack, 2);
	StackPushNoCheck(stack, a);
	StackPushNoCheck(stack, b);
}


#ifndef NDEBUG
/*************
 * Debugging *
 *************/


// For use with call in gdb
void StackDump(const fungeStack * stack) FUNGE_ATTR_UNUSED;

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

// This is for tracing
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void PrintStackTop(const fungeStack * stack)
{
	if (!stack)
		return;
	if (stack->top == 0) {
		fputs("\tStack is empty.\n", stderr);
	} else {
		fprintf(stderr, "\tStack has %zu elements, top 5 (or less) elements:\n\t\t", stack->top);
		for (ssize_t i = stack->top; (i > 0) && (i > ((ssize_t)stack->top - 5)); i--)
			fprintf(stderr, "%" FUNGEDATAPRI " ", stack->entries[i-1]);
		fputs("\n", stderr);
	}
}


/****************
 * Stack-stacks *
 ****************/

FUNGE_ATTR_FAST fungeStackStack * StackStackCreate(void)
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

FUNGE_ATTR_FAST void StackStackFree(fungeStackStack * me)
{
	if (!me)
		return;

	for (size_t i = 0; i < me->size; i++)
		StackFree(me->stacks[i]);

	cf_free(me);
}

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST fungeStackStack * StackStackDuplicate(const fungeStackStack * restrict old)
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


FUNGE_ATTR_FAST static void OOMstackStack(const instructionPointer * restrict ip)
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

FUNGE_ATTR_FAST bool StackStackBegin(instructionPointer * restrict ip, fungeStackStack ** me, FUNGEDATATYPE count, const fungePosition * restrict storageOffset)
{
	fungeStackStack *stackStack;
	fungeStack      *TOSS, *SOSS;
	FUNGEDATATYPE * entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);
	assert(storageOffset != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc_noptr(sizeof(FUNGEDATATYPE) * (count + 1));
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


FUNGE_ATTR_FAST bool StackStackEnd(instructionPointer * restrict ip, fungeStackStack ** me, FUNGEDATATYPE count)
{
	fungeStack      *TOSS, *SOSS;
	fungeStackStack *stackStack;
	fungePosition    storageOffset;
	FUNGEDATATYPE * entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc_noptr(sizeof(FUNGEDATATYPE) * (count + 1));
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
	if (count > 0) {
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


FUNGE_ATTR_FAST void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS)
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

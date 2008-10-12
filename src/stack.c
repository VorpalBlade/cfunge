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

/// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 4096

/******************************
 * Constructor and destructor *
 ******************************/

FUNGE_ATTR_FAST funge_stack * stack_create(void)
{
	funge_stack * tmp = (funge_stack*)cf_malloc(sizeof(funge_stack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (fungeCell*)cf_malloc_noptr(ALLOCCHUNKSIZE * sizeof(fungeCell));
	if (tmp->entries == NULL) {
		cf_free(tmp);
		return NULL;
	}
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	return tmp;
}

FUNGE_ATTR_FAST void stack_free(funge_stack * stack)
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
static inline funge_stack * stack_duplicate(const funge_stack * old)
{
	funge_stack * tmp = (funge_stack*)cf_malloc(sizeof(funge_stack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = (fungeCell*)cf_malloc_noptr((old->top + 1) * sizeof(fungeCell));
	if (tmp->entries == NULL) {
		cf_free(tmp);
		return NULL;
	}
	tmp->size = old->top + 1;
	tmp->top = old->top;
	// Not sure if memcpy() on 0 is well defined, so lets be careful.
	if (tmp->top != 0)
		memcpy(tmp->entries, old->entries, sizeof(fungeCell) * tmp->top);
	return tmp;
}
#endif

FUNGE_ATTR_FAST FUNGE_ATTR_NORET
static void stack_oom(void)
{
	perror("Emergency! Failed to allocate enough memory for new stack items");
	abort();
}

/*************************************
 * Basic push/pop/peeks and prealloc *
 *************************************/

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void stack_prealloc_space(funge_stack * restrict stack, size_t minfree)
{
	if ((stack->top + minfree) >= stack->size) {
		size_t newsize = stack->size + minfree;
		// Round upwards to whole ALLOCCHUNKSIZEed blocks.
		newsize += ALLOCCHUNKSIZE - (newsize % ALLOCCHUNKSIZE);
		stack->entries = (fungeCell*)cf_realloc(stack->entries, newsize * sizeof(fungeCell));
		if (!stack->entries) {
			stack_oom();
		}
		stack->size = newsize;
	}
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void stack_push_no_check(funge_stack * restrict stack, fungeCell value)
{
	// This should only be used if that is true...
	assert(stack->top < stack->size);
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST void stack_push(funge_stack * restrict stack, fungeCell value)
{
	assert(stack != NULL);
	assert(stack->top <= stack->size);

	// Do we need to realloc?
	if (stack->top == stack->size) {
		stack->entries = (fungeCell*)cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(fungeCell));
		if (!stack->entries) {
			stack_oom();
		}
		stack->size += ALLOCCHUNKSIZE;
	}
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST inline fungeCell stack_pop(funge_stack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else {
		fungeCell tmp = stack->entries[stack->top - 1];
		stack->top--;
		return tmp;
	}
}

FUNGE_ATTR_FAST void stack_pop_discard(funge_stack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

FUNGE_ATTR_FAST void stack_pop_n_discard(funge_stack * restrict stack, size_t n)
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


FUNGE_ATTR_FAST inline fungeCell stack_peek(const funge_stack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0) {
		return 0;
	} else {
		return stack->entries[stack->top - 1];
	}
}


FUNGE_ATTR_FAST inline fungeCell stack_get_index(const funge_stack * restrict stack, size_t index)
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

FUNGE_ATTR_FAST void stack_push_vector(funge_stack * restrict stack, const fungeVector * restrict value)
{
	// TODO: Optimise
	stack_push(stack, value->x);
	stack_push(stack, value->y);
}

FUNGE_ATTR_FAST fungeVector stack_pop_vector(funge_stack * restrict stack)
{
	// TODO: Optimise
	fungeCell x, y;
	y = stack_pop(stack);
	x = stack_pop(stack);
	return (fungeVector) { .x = x, .y = y };
}

FUNGE_ATTR_FAST void stack_push_string(funge_stack * restrict stack, const char * restrict str, size_t len)
{
	assert(str != NULL);
	assert(stack != NULL);
	// Increment it once or it won't work
	len++;
	stack_prealloc_space(stack, len);
	while (len-- > 0)
		stack_push_no_check(stack, str[len]);
}

FUNGE_ATTR_FAST char *stack_pop_string(funge_stack * restrict stack)
{
	fungeCell c;
	size_t index = 0;
	// FIXME: This may very likely be more than is needed.
	char * buf = (char*)cf_malloc_noptr((stack->top + 1) * sizeof(char));
	if (!buf)
		return NULL;

	while ((c = stack_pop(stack)) != '\0') {
		buf[index] = (char)c;
		index++;
	}
	buf[index] = '\0';
	return buf;
}

FUNGE_ATTR_FAST char *stack_pop_sized_string(funge_stack * restrict stack, size_t len)
{
	char * restrict x = (char*)cf_malloc_noptr((len + 1) * sizeof(char));
	if (!x)
		return NULL;

	for (size_t i = 0; i < len; i++) {
		x[i] = stack_pop(stack);
	}
	x[len + 1] = '\0';
	return x;
}


/***************
 * Other stuff *
 ***************/
FUNGE_ATTR_FAST void stack_dup_top(funge_stack * restrict stack)
{
	// TODO: Optimise instead of doing it this way
	fungeCell tmp;

	tmp = stack_peek(stack);
	stack_push(stack, tmp);
	// If it was empty, push a second zero.
	if (stack->top == 1)
		stack_push(stack, 0);
}

FUNGE_ATTR_FAST void stack_swap_top(funge_stack * restrict stack)
{
	// TODO: Optimise instead of doing it this way
	fungeCell a, b;
	// Well this have to work logically...
	a = stack_pop(stack);
	b = stack_pop(stack);
	stack_prealloc_space(stack, 2);
	stack_push_no_check(stack, a);
	stack_push_no_check(stack, b);
}


#ifndef NDEBUG
/*************
 * Debugging *
 *************/


// For use with call in gdb
void stack_dump(const funge_stack * stack) FUNGE_ATTR_UNUSED;

void stack_dump(const funge_stack * stack)
{
	if (!stack)
		return;
	fprintf(stderr, "%zu elements:\n", stack->top);
	for (size_t i = 0; i < stack->top; i++)
		fprintf(stderr, "%" FUNGECELLPRI " ", stack->entries[i]);
	fputs("\n", stderr);
}

#endif

// This is for tracing
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void stack_print_top(const funge_stack * stack)
{
	if (!stack)
		return;
	if (stack->top == 0) {
		fputs("\tStack is empty.\n", stderr);
	} else {
		fprintf(stderr, "\tStack has %zu elements, top 5 (or less) elements:\n\t\t", stack->top);
		for (ssize_t i = stack->top; (i > 0) && (i > ((ssize_t)stack->top - 5)); i--)
			fprintf(stderr, "%" FUNGECELLPRI " ", stack->entries[i-1]);
		fputs("\n", stderr);
	}
}


/****************
 * Stack-stacks *
 ****************/

FUNGE_ATTR_FAST funge_stackstack * stackstack_create(void)
{
	funge_stackstack * stackStack;
	funge_stack      * stack;

	stackStack = (funge_stackstack*)cf_malloc(sizeof(funge_stackstack) + sizeof(funge_stack*));
	if (!stackStack)
		return NULL;

	stack = stack_create();
	if (!stack) {
		cf_free(stackStack);
		return NULL;
	}

	stackStack->size = 1;
	stackStack->current = 0;
	stackStack->stacks[0] = stack;
	return stackStack;
}

FUNGE_ATTR_FAST void stackstack_free(funge_stackstack * me)
{
	if (!me)
		return;

	for (size_t i = 0; i < me->size; i++)
		stack_free(me->stacks[i]);

	cf_free(me);
}

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST funge_stackstack * stackstack_duplicate(const funge_stackstack * restrict old)
{
	funge_stackstack * stackStack;

	assert(old != NULL);

	stackStack = (funge_stackstack*)cf_malloc(sizeof(funge_stackstack) + old->size * sizeof(funge_stack*));
	if (!stackStack)
		return NULL;

	for (size_t i = 0; i <= old->current; i++) {
		stackStack->stacks[i] = stack_duplicate(old->stacks[i]);
		if (!stackStack->stacks[i])
			return NULL;
	}

	stackStack->size = old->size;
	stackStack->current = old->current;
	return stackStack;
}
#endif


FUNGE_ATTR_FAST static void oom_stackstack(const instructionPointer * restrict ip)
{
	if (setting_enable_warnings) {
		fprintf(stderr,
		        "WARN: Out of memory in stack-stack routine at x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ". Reflecting.\n",
		        ip->position.x, ip->position.y);
	}
	// Lets hope.
#ifndef DISABLE_GC
	gc_collect_full();
#endif
}

FUNGE_ATTR_FAST bool stackstack_begin(instructionPointer * restrict ip, funge_stackstack ** me, fungeCell count, const fungeVector * restrict storageOffset)
{
	funge_stackstack *stackStack;
	funge_stack      *TOSS, *SOSS;
	fungeCell       * restrict entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);
	assert(storageOffset != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc_noptr(sizeof(fungeCell) * (count + 1));
		// Reflect on out of memory, do it here before we mess up stuff.
		if (!entriesCopy) {
			oom_stackstack(ip);
			return false;
		}
	}

	// Set up variables
	stackStack = *me;

	TOSS = stack_create();
	if (!TOSS) {
		if (entriesCopy)
			cf_free(entriesCopy);
		return false;
	}

	// Extend by one
	stackStack = (funge_stackstack*)cf_realloc(*me, sizeof(funge_stackstack) + ((*me)->size + 1) * sizeof(funge_stack*));
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
		fungeCell i = count;
		while (i--)
			entriesCopy[i] = stack_pop(SOSS);
		for (i = 0; i < count; i++)
			stack_push(TOSS, entriesCopy[i]);
	} else if (count < 0) {
		while (count++)
			stack_push(SOSS, 0);
	}
	stack_push_vector(SOSS, &ip->storageOffset);
	ip->storageOffset.x = storageOffset->x;
	ip->storageOffset.y = storageOffset->y;
	ip->stack = TOSS;
	ip->stackstack = stackStack;
	if (entriesCopy)
		cf_free(entriesCopy);
	return true;
}


FUNGE_ATTR_FAST bool stackstack_end(instructionPointer * restrict ip, funge_stackstack ** me, fungeCell count)
{
	funge_stack      *TOSS, *SOSS;
	funge_stackstack *stackStack;
	fungeVector      storageOffset;
	fungeCell       * restrict entriesCopy = NULL;

	assert(ip != NULL);
	assert(me != NULL);

	if (count > 0) {
		entriesCopy = cf_malloc_noptr(sizeof(fungeCell) * (count + 1));
		// Reflect on out of memory, do it here before we mess up stuff.
		if (!entriesCopy) {
			oom_stackstack(ip);
			return false;
		}
	}

	// Set up variables
	stackStack = *me;
	TOSS = stackStack->stacks[stackStack->current];
	SOSS = stackStack->stacks[stackStack->current - 1];
	storageOffset = stack_pop_vector(SOSS);
	if (count > 0) {
		fungeCell i = count;
		while (i--)
			entriesCopy[i] = stack_pop(TOSS);
		for (i = 0; i < count; i++)
			stack_push(SOSS, entriesCopy[i]);
	} else if (count < 0) {
		while (count++)
			stack_pop_discard(SOSS);
	}
	ip->storageOffset.x = storageOffset.x;
	ip->storageOffset.y = storageOffset.y;

	ip->stack = SOSS;
	// Make it one smaller
	stackStack = (funge_stackstack*)cf_realloc(*me, sizeof(funge_stackstack) + ((*me)->size - 1) * sizeof(funge_stack*));
	if (!stackStack) {
		if (entriesCopy)
			cf_free(entriesCopy);
		return false;
	}
	stackStack->size--;
	stackStack->current--;
	*me = stackStack;
	ip->stackstack = stackStack;
	stack_free(TOSS);
	if (entriesCopy)
		cf_free(entriesCopy);
	return true;
}


FUNGE_ATTR_FAST void stackstack_transfer(fungeCell count, funge_stack * restrict TOSS, funge_stack * restrict SOSS)
{
	assert(TOSS != NULL);
	assert(SOSS != NULL);
	assert(TOSS != SOSS);

	if (count > 0) {
		while (count--) {
			stack_push(TOSS, stack_pop(SOSS));
		}
	} else if (count < 0) {
		while (count++) {
			stack_push(SOSS, stack_pop(TOSS));
		}
	}
}

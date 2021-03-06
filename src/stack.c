/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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
#include "diagnostic.h"

#include <assert.h>
#include <string.h> /* memcpy, memset */

/// How many new items to allocate in one go?
#define ALLOCSIZE_STACK 4096
/// How many stack pointers to allocate for the stack stack in one go.
#define ALLOCSIZE_STACKSTACK 32


/******************************
 * Constructor and destructor *
 ******************************/

funge_stack * stack_create(void)
{
	funge_stack * tmp = (funge_stack*)malloc(sizeof(funge_stack));
	if (FUNGE_UNLIKELY(!tmp))
		return NULL;
	tmp->entries = (funge_cell*)malloc(ALLOCSIZE_STACK * sizeof(funge_cell));
	if (FUNGE_UNLIKELY(!tmp->entries)) {
		free(tmp);
		return NULL;
	}
	tmp->size = ALLOCSIZE_STACK;
	tmp->top = 0;
	return tmp;
}

FUNGE_ATTR_FAST void stack_free(funge_stack * stack)
{
	if (FUNGE_UNLIKELY(!stack))
		return;
	if (FUNGE_LIKELY(stack->entries != NULL)) {
		free(stack->entries);
		stack->entries = NULL;
	}
	free(stack);
}

#ifdef CONCURRENT_FUNGE
// Used for concurrency
FUNGE_ATTR_FAST FUNGE_ATTR_MALLOC FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline funge_stack * stack_duplicate(const funge_stack * old)
{
	funge_stack * tmp = (funge_stack*)malloc(sizeof(funge_stack));
	if (FUNGE_UNLIKELY(!tmp))
		return NULL;
	tmp->entries = (funge_cell*)malloc((old->top + 1) * sizeof(funge_cell));
	if (FUNGE_UNLIKELY(!tmp->entries)) {
		free(tmp);
		return NULL;
	}
	tmp->size = old->top + 1;
	tmp->top = old->top;
	// Not sure if memcpy() on 0 is well defined, so lets be careful.
	if (tmp->top != 0)
		memcpy(tmp->entries, old->entries, sizeof(funge_cell) * tmp->top);
	return tmp;
}
#endif

FUNGE_ATTR_COLD FUNGE_ATTR_NORET
static void stack_oom(void)
{
	DIAG_OOM("Failed to allocate enough memory for new stack items");
}

/*************************************
 * Basic push/pop/peeks and prealloc *
 *************************************/

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void stack_prealloc_space(funge_stack * restrict stack, size_t minfree)
{
	if ((stack->top + minfree) >= stack->size) {
		size_t newsize = stack->size + minfree;
		size_t allocation_size;
		// Round upwards to whole ALLOCSIZE_STACK sized blocks.
		newsize += ALLOCSIZE_STACK - (newsize % ALLOCSIZE_STACK);

		// Guard against overflow.
		allocation_size = newsize * sizeof(funge_cell);
		if (FUNGE_UNLIKELY(allocation_size < newsize))
		{
			stack_oom();
		}
		stack->entries = (funge_cell*)realloc(stack->entries, allocation_size);
		if (FUNGE_UNLIKELY(!stack->entries)) {
			stack_oom();
		}
		stack->size = newsize;
	}
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void stack_push_no_check(funge_stack * restrict stack, funge_cell value)
{
	// This should only be used if that is true...
	assert(stack->top < stack->size);
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST void stack_push(funge_stack * restrict stack, funge_cell value)
{
	assert(stack != NULL);
	assert(stack->top <= stack->size);

	// Do we need to realloc?
	if (FUNGE_UNLIKELY(stack->top == stack->size)) {
		funge_cell* new_entries = (funge_cell*)realloc(stack->entries, (stack->size + ALLOCSIZE_STACK) * sizeof(funge_cell));
		if (FUNGE_UNLIKELY(!new_entries)) {
			stack_oom();
		}
		stack->entries = new_entries;
		stack->size += ALLOCSIZE_STACK;
	}
	stack->entries[stack->top] = value;
	stack->top++;
}

FUNGE_ATTR_FAST inline funge_cell stack_pop(funge_stack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0)
		return 0;

	return stack->entries[--stack->top];
}

FUNGE_ATTR_FAST void stack_discard(funge_stack * restrict stack, size_t n)
{
	assert(stack != NULL);

	if (stack->top > n) {
		stack->top -= n;
	} else {
		stack->top = 0;
	}
}


FUNGE_ATTR_FAST inline funge_cell stack_peek(const funge_stack * restrict stack)
{
	assert(stack != NULL);

	if (stack->top == 0)
		return 0;
	return stack->entries[stack->top - 1];
}


FUNGE_ATTR_FAST inline funge_cell stack_get_index(const funge_stack * restrict stack, size_t index)
{
	assert(stack != NULL);
	assert(index > 0);

	if (stack->top == 0)
		return 0;
	if (stack->top < index)
		return 0;
	return stack->entries[index - 1];
}

FUNGE_ATTR_FAST inline size_t stack_strlen(const funge_stack * restrict stack)
{
	paranoid_assert(stack != NULL);
	// TODO: Maybe scan two cells at once if we are using 32-bit cells on a
	// 64-bit system?
	for (size_t i = stack->top; i > 0; i--) {
		if (stack->entries[i - 1] == 0)
			return stack->top - i;
	}
	return stack->top;
}


/********************************
 * Push and pop for data types. *
 ********************************/

FUNGE_ATTR_FAST void stack_push_vector(funge_stack * restrict stack, const funge_vector * restrict value)
{
	// TODO: Optimise
	paranoid_assert(stack != NULL);
	paranoid_assert(value != NULL);
	stack_push(stack, value->x);
	stack_push(stack, value->y);
}

FUNGE_ATTR_FAST funge_vector stack_pop_vector(funge_stack * restrict stack)
{
	// TODO: Optimise
	funge_cell x, y;
	paranoid_assert(stack != NULL);
	y = stack_pop(stack);
	x = stack_pop(stack);
	return (funge_vector) { .x = x, .y = y };
}

FUNGE_ATTR_FAST void stack_push_string(funge_stack * restrict stack, const unsigned char * restrict str, size_t len)
{
	assert(str != NULL);
	assert(stack != NULL);
	// Increment it once or it won't work
	stack_prealloc_space(stack, len + 1);
	{
		const size_t top = stack->top + len;
		for (ssize_t i = (ssize_t)len; i >= 0; i--)
			stack->entries[top - (size_t)i] = str[i];
		stack->top += len + 1;
	}
}

FUNGE_ATTR_FAST unsigned char *stack_pop_string(funge_stack * restrict stack, size_t * restrict len)
{
	funge_cell c;
	size_t index = 0;
	unsigned char *buf;
	paranoid_assert(stack != NULL);
	// FIXME: This may very likely be more than is needed.
	buf = (unsigned char*)malloc((stack->top + 1) * sizeof(unsigned char));
	if (FUNGE_UNLIKELY(!buf)) {
		if (len)
			*len = 0;
		return NULL;
	}

	while ((c = stack_pop(stack)) != '\0') {
		buf[index++] = (unsigned char)c;
	}
	buf[index] = '\0';
	if (len)
		*len = index;
	return buf;
}

FUNGE_ATTR_FAST void stack_push_string_multibyte(funge_stack * restrict stack, const funge_cell * restrict str, size_t len)
{
	assert(str != NULL);
	assert(stack != NULL);
	// Increment it once or it won't work
	stack_prealloc_space(stack, len + 1);
	{
		const size_t top = stack->top + len;
		for (ssize_t i = (ssize_t)len; i >= 0; i--)
			stack->entries[top - (size_t)i] = str[i];
		stack->top += len + 1;
	}
}

FUNGE_ATTR_FAST funge_cell *stack_pop_string_multibyte(funge_stack * restrict stack, size_t * restrict len)
{
	funge_cell c;
	size_t index = 0;
	funge_cell *buf;
	paranoid_assert(stack != NULL);
	// FIXME: This may very likely be more than is needed.
	buf = (funge_cell*)malloc((stack->top + 1) * sizeof(funge_cell));
	if (FUNGE_UNLIKELY(!buf)) {
		if (len)
			*len = 0;
		return NULL;
	}

	while ((c = stack_pop(stack)) != '\0') {
		buf[index++] = c;
	}
	buf[index] = '\0';
	if (len)
		*len = index;
	return buf;
}

#ifdef UNUSED
FUNGE_ATTR_FAST unsigned char *stack_pop_sized_string(funge_stack * restrict stack, size_t len)
{
	unsigned char * restrict x = (unsigned char*)malloc((len + 1) * sizeof(char));
	if (FUNGE_UNLIKELY(!x))
		return NULL;

	for (size_t i = 0; i < len; i++) {
		x[i] = (unsigned char)stack_pop(stack);
	}
	x[len + 1] = '\0';
	return x;
}
#endif


/***************
 * Other stuff *
 ***************/
FUNGE_ATTR_FAST void stack_dup_top(funge_stack * restrict stack)
{
	funge_cell tmp;
	paranoid_assert(stack != NULL);
	// TODO: Optimise instead of doing it this way
	tmp = stack_peek(stack);
	stack_push(stack, tmp);
	// If it was empty, push a second zero.
	if (stack->top == 1)
		stack_push(stack, 0);
}

FUNGE_ATTR_FAST void stack_swap_top(funge_stack * restrict stack)
{
	// TODO: Optimise instead of doing it this way
	funge_cell a, b;
	paranoid_assert(stack != NULL);
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
void stack_dump(const funge_stack * stack) FUNGE_ATTR_UNUSED FUNGE_ATTR_COLD;

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

#ifndef DISABLE_TRACE
// This is for tracing
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void stack_print_top(const funge_stack * stack)
{
	assert(stack != NULL);
	if (stack->top == 0) {
		fputs("\tStack is empty.\n", stderr);
	} else {
		fprintf(stderr, "\tStack has %zu elements, top 15 (or less) elements:\n\t\t", stack->top);
		for (ssize_t i = (ssize_t)stack->top; (i > 0) && (i > ((ssize_t)stack->top - 15)); i--)
			fprintf(stderr, "%" FUNGECELLPRI " ", stack->entries[i - 1]);
		fputs("\n", stderr);
	}
}
#endif

/****************
 * Stack-stacks *
 ****************/

funge_stackstack * stackstack_create(void)
{
	funge_stackstack * stackStack;
	funge_stack      * stack;

	stackStack = (funge_stackstack*)malloc(sizeof(funge_stackstack) + sizeof(funge_stack*) * ALLOCSIZE_STACKSTACK);
	if (FUNGE_UNLIKELY(!stackStack))
		return NULL;

	stack = stack_create();
	if (FUNGE_UNLIKELY(!stack)) {
		free(stackStack);
		return NULL;
	}

	stackStack->size = ALLOCSIZE_STACKSTACK;
	stackStack->current = 0;
	stackStack->stacks[0] = stack;
	return stackStack;
}

FUNGE_ATTR_FAST void stackstack_free(funge_stackstack * me)
{
	if (FUNGE_UNLIKELY(!me))
		return;

	for (size_t i = 0; i <= me->current; i++)
		stack_free(me->stacks[i]);

	free(me);
}

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST funge_stackstack * stackstack_duplicate(const funge_stackstack * restrict old)
{
	funge_stackstack * stackStack;

	assert(old != NULL);

	stackStack = (funge_stackstack*)malloc(sizeof(funge_stackstack) + old->size * sizeof(funge_stack*));
	if (FUNGE_UNLIKELY(!stackStack))
		return NULL;

	for (size_t i = 0; i <= old->current; i++) {
		stackStack->stacks[i] = stack_duplicate(old->stacks[i]);
		if (FUNGE_UNLIKELY(!stackStack->stacks[i])) {
			// Clean up the partially created stack-stack.
			for (size_t j = 0; j < i; j++) {
				stack_free(stackStack->stacks[j]);
			}
			free(stackStack);
			return NULL;
		}
	}

	stackStack->size = old->size;
	stackStack->current = old->current;
	return stackStack;
}
#endif


FUNGE_ATTR_FAST static void oom_stackstack(const instructionPointer * restrict ip)
{
	diag_warn_format("Out of memory in stack-stack routine at x=%"
	                 FUNGECELLPRI " y=%" FUNGECELLPRI ". Reflecting.",
	                 ip->position.x, ip->position.y);
	// Lets hope.
}


/**
 * Like stack_prealloc_space() above, except it returns false instead of exiting
 * on OOM. Needed for stack stack begin/end.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline bool stack_prealloc_space_non_fatal(funge_stack * restrict stack, size_t minfree)
{
	paranoid_assert(stack != NULL);
	if ((stack->top + minfree) >= stack->size) {
		size_t newsize = stack->size + minfree;
		size_t allocation_size;
		funge_cell* newentries;
		// Round upwards to whole ALLOCSIZE_STACKed blocks.
		newsize += ALLOCSIZE_STACK - (newsize % ALLOCSIZE_STACK);

		// Guard against overflow.
		allocation_size = newsize * sizeof(funge_cell);
		if (FUNGE_UNLIKELY(allocation_size < newsize))
		{
			return false;
		}
		newentries = (funge_cell*)realloc(stack->entries, allocation_size);
		if (FUNGE_UNLIKELY(!newentries)) {
			return false;
		}
		stack->entries = newentries;
		stack->size = newsize;
	}
	return true;
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void stack_zero_fill(funge_stack * restrict stack, size_t count)
{
	paranoid_assert(stack != NULL);
	stack_prealloc_space(stack, count);
	memset(&stack->entries[stack->top], 0, count * sizeof(funge_cell));
	stack->top += count;
}

// This does an in-order bulk copy of count elements between two stacks.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
inline void stack_bulk_copy(funge_stack * restrict dest,
                            const funge_stack * restrict src, size_t count)
{
	paranoid_assert(dest != NULL);
	paranoid_assert(src != NULL);

	stack_prealloc_space(dest, count);

	// Figure out if we were asked to copy more items than actually exists:
	if (count > src->top) {
		// Push some initial zeros then.
		size_t zero_count = count - src->top;
		count -= zero_count;
		stack_zero_fill(dest, zero_count);
	}

	// memcpy the rest.
	memcpy(&dest->entries[dest->top], &src->entries[src->top - count], count * sizeof(funge_cell));
	dest->top += count;
}

FUNGE_ATTR_FAST
bool stackstack_begin(instructionPointer * ip, funge_cell count, const funge_vector * restrict storageOffset)
{
	funge_stackstack *stackStack;
	funge_stack      *TOSS, *SOSS;

	assert(ip != NULL);
	assert(storageOffset != NULL);

	// Set up variables
	stackStack = ip->stackstack;

	TOSS = stack_create();
	if (FUNGE_UNLIKELY(!TOSS)) {
		oom_stackstack(ip);
		return false;
	}
	// Allocate enough space on the TOSS and reflect if not.
	// This is count + 2 (storage offset)
	if (FUNGE_UNLIKELY(!stack_prealloc_space_non_fatal(TOSS, ABS(count) + 2))) {
		stack_free(TOSS);
		oom_stackstack(ip);
		return false;
	}

	// Extend the stack stack allocation if required:
	if ((stackStack->size - 1) == stackStack->current) {
		stackStack = realloc(stackStack, sizeof(funge_stackstack) + (stackStack->size + ALLOCSIZE_STACKSTACK) * sizeof(funge_stack*));
		if (FUNGE_UNLIKELY(!stackStack)) {
			stack_free(TOSS);
			oom_stackstack(ip);
			return false;
		}
		stackStack->size += ALLOCSIZE_STACKSTACK;
	}

	SOSS = stackStack->stacks[stackStack->current];

	stackStack->current++;
	stackStack->stacks[stackStack->current] = TOSS;

	if (count > 0) {
		stack_bulk_copy(TOSS, SOSS, (size_t)count);
		// Make it into a move.
		if ((size_t)count > SOSS->top)
			SOSS->top = 0;
		else
			SOSS->top -= (size_t)count;
	} else if (count < 0) {
		stack_zero_fill(SOSS, (size_t)(-count));
	}
	stack_push_vector(SOSS, &ip->storageOffset);
	ip->storageOffset.x = storageOffset->x;
	ip->storageOffset.y = storageOffset->y;
	ip->stack = TOSS;
	ip->stackstack = stackStack;
	return true;
}


FUNGE_ATTR_FAST
bool stackstack_end(instructionPointer * ip, funge_cell count)
{
	funge_stack      *TOSS, *SOSS;
	funge_stackstack *stackStack;
	funge_vector      storageOffset;

	assert(ip != NULL);

	// Set up variables
	stackStack = ip->stackstack;
	TOSS = stackStack->stacks[stackStack->current];
	SOSS = stackStack->stacks[stackStack->current - 1];
	storageOffset = stack_pop_vector(SOSS);
	if (count > 0) {
		// Since TOSS is discarded there is no need to update it's top pointer.
		stack_bulk_copy(SOSS, TOSS, (size_t)count);
	} else if (count < 0) {
		stack_discard(SOSS, (size_t)(-count));
	}
	ip->storageOffset.x = storageOffset.x;
	ip->storageOffset.y = storageOffset.y;

	ip->stack = SOSS;
	stackStack->stacks[stackStack->current] = NULL;
	// TODO: Should we shrink stack stack allocation if difference is large?
	// Needs testing to figure out.
	stackStack->current--;
	stack_free(TOSS);
	return true;
}


FUNGE_ATTR_FAST
void stackstack_transfer(funge_cell count, funge_stack * restrict TOSS, funge_stack * restrict SOSS)
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

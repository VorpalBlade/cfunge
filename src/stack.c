/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
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
#include "vector.h"
#include "stack.h"
#include "ip.h"
#include <assert.h>
#include <string.h>
#include <gc/cord.h>
#include <gc/ec.h>

// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 16

/******************************
 * Constructor and destructor *
 ******************************/

fungeStack * StackCreate(void)
{
	fungeStack * tmp = cf_malloc(sizeof(fungeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = cf_malloc_noptr(ALLOCCHUNKSIZE * sizeof(FUNGEDATATYPE));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	return tmp;
}

void StackFree(fungeStack * stack)
{
	if (!stack || !stack->entries)
		return;
	cf_free(stack->entries);
	stack->entries = NULL;
	cf_free(stack);
}



/************************
 * Basic push/pop/peeks *
 ************************/

void StackPush(FUNGEDATATYPE value, fungeStack * stack)
{
	// Do we need to realloc?
	if (stack->top == stack->size) {
		stack->entries = cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(FUNGEDATATYPE));
		stack->entries[stack->top] = value;
		stack->top++;
		stack->size += ALLOCCHUNKSIZE;
	} else {
		stack->entries[stack->top] = value;
		stack->top++;
	}
}

FUNGEDATATYPE StackPop(fungeStack * stack)
{
	if (stack->top == 0) {
		return 0;
	} else {
		FUNGEDATATYPE tmp = stack->entries[stack->top - 1];
		stack->top--;
		return tmp;
	}
}

void StackPopDiscard(fungeStack * stack)
{
	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

void StackPopNDiscard(fungeStack * stack, size_t n)
{
	if (stack->top == 0) {
		return;
	} else {
		if ((stack->top - n) > 0)
			stack->top -= n;
		else
			stack->top = 0;
	}
}



FUNGEDATATYPE StackPeek(fungeStack * stack)
{
	if (stack->top == 0) {
		return 0;
	} else {
		return stack->entries[stack->top - 1];
	}
}



/********************************
 * Push and pop for data types. *
 ********************************/

void StackPushVector(const fungeVector * restrict value, fungeStack * restrict stack)
{
	// TODO: Optimize
	StackPush(value->x, stack);
	StackPush(value->y, stack);
}

fungeVector StackPopVector(fungeStack * stack)
{
	// TODO Optimize
	FUNGEVECTORTYPE x, y;
	y = StackPop(stack);
	x = StackPop(stack);
	return (fungeVector) { .x = x, .y = y };
}

void StackPushString(size_t len, const char * restrict str, fungeStack * restrict stack)
{
	for (ssize_t i = len; i >= 0; i--)
		StackPush(str[i], stack);
}

char *StackPopString(fungeStack * stack)
{
	CORD_ec x;
	FUNGEDATATYPE c;

	CORD_ec_init(x);
	while ((c = StackPop(stack)) != '\0')
		CORD_ec_append(x, (char)c);

	return CORD_to_char_star(CORD_ec_to_cord(x));
}

/***************
 * Other stuff *
 ***************/
void StackClear(fungeStack * stack)
{
	stack->top = 0;
}

void StackDupTop(fungeStack * stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE tmp;

	tmp = StackPeek(stack);
	StackPush(tmp, stack);
	if (stack->top == 1)
		StackPush(0, stack);
}

void StackSwapTop(fungeStack * stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE a, b;
	a = StackPop(stack);
	b = StackPop(stack);
	StackPush(a, stack);
	StackPush(b, stack);
}



/*************
 * Debugging *
 *************/

// For use with call in gdb
void StackDump(fungeStack * stack) __attribute__((unused));

void StackDump(fungeStack * stack)
{
	for (size_t i = 0; i < stack->top; i++)
		fprintf(stderr, "%zu=%" FUNGEDATAPRI " ", i, stack->entries[i]);
	fputs("%\n", stderr);
}



/****************
 * Stack-stacks *
 ****************/

fungeStackStack * StackStackCreate(void)
{
	fungeStackStack * stackStack;
	fungeStack      * stack;

	stackStack = cf_malloc(sizeof(fungeStackStack) + sizeof(fungeStack*));
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

bool StackStackBegin(instructionPointer * restrict ip, fungeStackStack ** restrict me, FUNGEDATATYPE count, const fungePosition * restrict storageOffset)
{
	fungeStackStack *stackStack;
	fungeStack      *TOSS, *SOSS;

	// Set up variables
	stackStack = *me;

	TOSS = StackCreate();
	if (!TOSS)
		return false;

	// Extend by one
	stackStack = cf_realloc(*me, sizeof(fungeStackStack) + ((*me)->size + 1) * sizeof(fungeStack*));
	if (!stackStack)
		return false;
	*me = stackStack;
	SOSS = stackStack->stacks[stackStack->current];

	stackStack->size++;
	stackStack->current++;
	stackStack->stacks[stackStack->current] = TOSS;

	if (count > 0) {
		// Dynamic array (C99) to copy elements into, because order must be preserved.
		FUNGEDATATYPE entriesCopy[count + 1];
		FUNGEDATATYPE i = count;
		while (i--)
			entriesCopy[i] = StackPop(SOSS);
		for (i = 0; i < count; i++)
			StackPush(entriesCopy[i], TOSS);
	} else if (count < 0) {
		while (count++)
			StackPush(0, SOSS);
	}
	StackPushVector(&ip->storageOffset, SOSS);
	ip->storageOffset.x = storageOffset->x;
	ip->storageOffset.y = storageOffset->y;
	ip->stack = TOSS;
	ip->stackstack = stackStack;

	return true;
}


bool StackStackEnd(instructionPointer * restrict ip, fungeStackStack ** restrict me, FUNGEDATATYPE count)
{
	fungeStack      *TOSS, *SOSS;
	fungeStackStack *stackStack;
	fungePosition    storageOffset;
	// Set up variables
	stackStack = *me;
	TOSS = stackStack->stacks[stackStack->current];
	SOSS = stackStack->stacks[stackStack->current - 1];
	storageOffset = StackPopVector(SOSS);
	// TODO: Transfer data back here
	if (count > 0) {
		// Dynamic array (C99) to copy elements into, because order must be preserved.
		FUNGEDATATYPE entriesCopy[count + 1];
		FUNGEDATATYPE i = count;
		while (i--)
			entriesCopy[i] = StackPop(TOSS);
		for (i = 0; i < count; i++)
			StackPush(entriesCopy[i], SOSS);
	} else if (count < 0) {
		while (count++)
			StackPopDiscard(SOSS);
	}
	ip->storageOffset.x = storageOffset.x;
	ip->storageOffset.y = storageOffset.y;

	ip->stack = SOSS;
	// Make it one smaller
	stackStack = cf_realloc(*me, sizeof(fungeStackStack) + ((*me)->size - 1) * sizeof(fungeStack*));
	if (!stackStack)
		return false;
	stackStack->size--;
	stackStack->current--;
	*me = stackStack;
	ip->stackstack = stackStack;
	return true;
}


void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS)
{
	if (count > 0) {
		while (count--) {
			StackPush(StackPop(SOSS), TOSS);
		}
	} else if (count < 0) {
		while (count++) {
			StackPush(StackPop(TOSS), SOSS);
		}
	}
}

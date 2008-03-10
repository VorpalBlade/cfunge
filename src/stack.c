/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

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

// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 10

fungeStack *
StackCreate(void)
{
	fungeStack * tmp = cf_malloc(sizeof(fungeStack));
	if (tmp == NULL)
		return NULL;
	tmp->entries = cf_malloc(ALLOCCHUNKSIZE * sizeof(FUNGEDATATYPE));
	if (tmp->entries == NULL)
		return NULL;
	tmp->size = ALLOCCHUNKSIZE;
	tmp->top = 0;
	tmp->storageOffset.x = 0;
	tmp->storageOffset.y = 0;
	return tmp;
}

void
StackFree(fungeStack * stack)
{
	if (!stack || !stack->entries)
		return;
	cf_free(stack->entries);
	stack->entries = NULL;
	cf_free(stack);
}


void
StackPush(FUNGEDATATYPE value, fungeStack * stack)
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


FUNGEDATATYPE
StackPop(fungeStack * stack)
{
	if (stack->top == 0) {
		return 0;
	} else {
		FUNGEDATATYPE tmp = stack->entries[stack->top - 1];
		stack->top--;
		return tmp;
	}
}

void
StackPopDiscard(fungeStack * stack)
{
	if (stack->top == 0) {
		return;
	} else {
		stack->top--;
	}
}

fungeVector
StackPopVector (fungeStack * stack) {
	FUNGEVECTORTYPE x, y;
	y = StackPop(stack);
	x = StackPop(stack);
	return (fungeVector) { .x = x, .y = y };
}

FUNGEDATATYPE
StackPeek(fungeStack * stack)
{
	if (stack->top == 0) {
		return 0;
	} else {
		return stack->entries[stack->top - 1];
	}
}


void
StackClear(fungeStack * stack)
{
	stack->top = 0;
}


void
StackDupTop(fungeStack * stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE tmp;

	tmp = StackPeek(stack);
	StackPush(tmp, stack);
}


void
StackSwapTop(fungeStack * stack)
{
	// TODO: Optimize instead of doing it this way
	FUNGEDATATYPE a, b;
	a = StackPop(stack);
	b = StackPop(stack);
	StackPush(a, stack);
	StackPush(b, stack);
}



static fungeStackEntry* StackEntryCreate(void)
{
	fungeStack *newStack;
	fungeStackEntry *newStackE;

	newStackE = cf_malloc(sizeof(fungeStackEntry));
	if (!newStackE)
		return NULL;
	newStack = StackCreate();
	if (!newStack)
		return NULL;

	newStackE->stack = newStack;
	newStackE->previous = NULL;
	newStackE->next = NULL;

	return newStackE;
}

fungeStackStack *
StackStackCreate(void)
{
	fungeStackStack * stackStack;
	fungeStackEntry * stackEntry;

	stackStack = cf_malloc(sizeof(fungeStackStack));
	if (!stackStack)
		return NULL;

	stackEntry = StackEntryCreate();
	if (!stackEntry)
		return NULL;

	stackStack->base    = stackEntry;
	stackStack->current = stackEntry;
	stackStack->top     = stackEntry;
	stackStack->count   = 1;

	return stackStack;
}

fungeStack *
StackStackBegin(fungeStackStack * stackStack, fungePosition * storageOffset, size_t count)
{
	// TODO: Do the copying of values between the stacks.
	fungeStackEntry *newStackE, *top;

	newStackE = StackEntryCreate();
	if (!newStackE)
		return NULL;

	stackStack->count++;
	top = stackStack->top;
	top->next = newStackE;
	newStackE->previous = top;

	stackStack->top = newStackE;
	stackStack->current = newStackE;

	newStackE->stack->storageOffset.x = storageOffset->x;
	newStackE->stack->storageOffset.y = storageOffset->y;

	return newStackE->stack;
}


fungeStack *
StackStackEnd(fungeStackStack * stackStack)
{
	// TODO
}


fungeStack *
StackStackUnder(fungeStackStack * stackStack, size_t count)
{
	// TODO

}

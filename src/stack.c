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
		cf_realloc(stack->entries, (stack->size + ALLOCCHUNKSIZE) * sizeof(FUNGEDATATYPE));
		stack->entries[stack->top] = value;
		stack->top++;
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
	newStack = cf_malloc(sizeof(fungeStackEntry));
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

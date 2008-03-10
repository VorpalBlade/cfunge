#include "global.h"
#include "vector.h"
#include "stack.h"

// How many new items to allocate in one go?
#define ALLOCCHUNKSIZE 10

fungeStack *
StackCreate(void)
{
	fungeStack * tmp = cf_malloc(sizeof(fungeStack));
	tmp->entries = cf_malloc(ALLOCCHUNKSIZE * sizeof(FUNGEDATATYPE));
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

}


void
StackSwapTop(fungeStack * stack)
{

}



fungeStack *
StackStackBegin(fungeStackStack * stackStack, fungePosition * storageOffset, size_t count)
{

}


fungeStack *
StackStackEnd(fungeStackStack * stackStack)
{

}


fungeStack *
StackStackUnder(fungeStackStack * stackStack, size_t count)
{

}

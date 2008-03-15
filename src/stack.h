/*
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

#ifndef _HAD_SRC_STACK_H
#define _HAD_SRC_STACK_H

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"
#include "funge-space/funge-space.h"


#ifndef ipDEFINED
struct _instructionPointer;
#endif

typedef struct {
	size_t         size; /**< This is current size of the array entries */
	size_t         top;  /**< This is current top item in stack (may not be last item).
	                          Note: One-indexed, as 0 = empty stack. */
	FUNGEDATATYPE *entries;
} fungeStack;


typedef struct {
	size_t         size;     /**< This is number of elements in stacks. */
	size_t         current;  /**< Top stack and current stack */
	fungeStack   * stacks[]; /**< Array of pointers to stacks */
} fungeStackStack;

/**
 * Create a new stack.
 */
extern fungeStack * StackCreate(void) __attribute__((malloc,warn_unused_result));
/**
 * Destroy a stack.
 */
extern void StackFree(fungeStack * stack);

/**
 * Push a item on the stack
 */
extern void StackPush(FUNGEDATATYPE value, fungeStack * stack) __attribute__((nonnull));
/**
 * Pop item from stack.
 */
extern FUNGEDATATYPE StackPop(fungeStack * stack) __attribute__((nonnull, warn_unused_result));
/**
 * Pop one item and discard it.
 */
extern void StackPopDiscard(fungeStack * stack) __attribute__((nonnull));
/**
 * Pop a number of items and discard them.
 */
extern void StackPopNDiscard(fungeStack * stack, size_t n) __attribute__((nonnull));
/**
 * Stack peek.
 */
extern FUNGEDATATYPE StackPeek(fungeStack * stack) __attribute__((nonnull, warn_unused_result));

/**
 * Push a vector.
 */
extern void StackPushVector(const fungeVector * restrict value, fungeStack * restrict stack) __attribute__((nonnull));
/**
 * Pop a vector.
 */
extern fungeVector StackPopVector(fungeStack * stack) __attribute__((nonnull, warn_unused_result));
/**
 * Push a null-terminated string to a 0"gnirts".
 */
extern void StackPushString(size_t len, const char * restrict str, fungeStack * restrict stack) __attribute__((nonnull));
/**
 * Pop a 0"gnirts" and return a null-terminated string
 */
extern char * StackPopString(fungeStack * stack) __attribute__((nonnull, warn_unused_result));
/**
 * Pop a fixed number of chars. Return as null-terminated string.
 */
extern char * StackPopSizedString(size_t len, fungeStack * stack) __attribute__((nonnull, warn_unused_result));
#define StackClear(stack) { stack->top = 0; }
/**
 * Duplicate top element of the stack.
 */
extern void StackDupTop(fungeStack * stack) __attribute__((nonnull));
/**
 * Swap the top two elements of the stack.
 */
extern void StackSwapTop(fungeStack * stack) __attribute__((nonnull));

//
// Stack-stack functions
//

/**
 * Create a new stack-stack
 */
extern fungeStackStack * StackStackCreate(void) __attribute__((malloc,warn_unused_result));
/**
 * Free a stack-stack and any stacks it contain.
 */
extern void StackStackFree(fungeStackStack * me);


/**
 * Begin a new stack on the stack stack.
 * count is how many arguments to copy over.
 */
extern bool StackStackBegin(struct _instructionPointer * restrict ip,
                            fungeStackStack ** restrict me,
                            FUNGEDATATYPE count,
                            const fungePosition * restrict storageOffset) __attribute__((nonnull, warn_unused_result));
/**
 * End a stack on the stack-stack
 * count is how many items to copy over.
 */
extern bool StackStackEnd(struct _instructionPointer * restrict ip,
                          fungeStackStack ** restrict me,
                          FUNGEDATATYPE count) __attribute__((nonnull, warn_unused_result));
/**
 * Transfer items from one stack to another (not in order).
 */
extern void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS)  __attribute__((nonnull));

#endif

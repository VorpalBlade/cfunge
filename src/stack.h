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

#ifndef _HAD_SRC_STACK_H
#define _HAD_SRC_STACK_H

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"
#include "funge-space/funge-space.h"


#ifndef ipDEFINED
struct s_instructionPointer;
#endif

typedef struct s_fungeStack {
	size_t         size; /**< This is current size of the array entries */
	size_t         top;  /**< This is current top item in stack (may not be last item).
	                          Note: One-indexed, as 0 = empty stack. */
	FUNGEDATATYPE *entries;
} fungeStack;


typedef struct s_fungeStackStack {
	size_t         size;     /**< This is number of elements in stacks. */
	size_t         current;  /**< Top stack and current stack */
	fungeStack   * stacks[]; /**< Array of pointers to stacks */
} fungeStackStack;

/**
 * Create a new stack.
 */
fungeStack * StackCreate(void) __attribute__((malloc, warn_unused_result, FUNGE_IN_FAST));
/**
 * Destroy a stack.
 */
void StackFree(fungeStack * stack) FUNGE_FAST;

/**
 * Push a item on the stack
 */
void StackPush(fungeStack * restrict stack, FUNGEDATATYPE value) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Pop item from stack.
 */
FUNGEDATATYPE StackPop(fungeStack * restrict stack) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Pop one item and discard it.
 */
void StackPopDiscard(fungeStack * restrict stack) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Pop a number of items and discard them.
 */
void StackPopNDiscard(fungeStack * restrict stack, size_t n) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Stack peek.
 */
FUNGEDATATYPE StackPeek(const fungeStack * restrict stack) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));

/**
 * Push a vector.
 */
void StackPushVector(fungeStack * restrict stack, const fungeVector * restrict value) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Pop a vector.
 */
fungeVector StackPopVector(fungeStack * restrict stack) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Push a null-terminated string to a 0"gnirts".
 */
void StackPushString(fungeStack * restrict stack, const char * restrict str, size_t len) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Pop a 0"gnirts" and return a null-terminated string
 */
char * StackPopString(fungeStack * restrict stack) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Pop a fixed number of chars. Return as null-terminated string.
 */
char * StackPopSizedString(fungeStack * restrict stack, size_t len) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
#define StackClear(stack) { stack->top = 0; }
/**
 * Duplicate top element of the stack.
 */
void StackDupTop(fungeStack * restrict stack) __attribute__((nonnull, FUNGE_IN_FAST));
/**
 * Swap the top two elements of the stack.
 */
void StackSwapTop(fungeStack * restrict stack) __attribute__((nonnull, FUNGE_IN_FAST));

//
// Stack-stack functions
//

/**
 * Create a new stack-stack
 */
fungeStackStack * StackStackCreate(void) __attribute__((malloc,warn_unused_result, FUNGE_IN_FAST));
/**
 * Free a stack-stack and any stacks it contain.
 */
void StackStackFree(fungeStackStack * me) FUNGE_FAST;

#ifdef CONCURRENT_FUNGE
/**
 * Deep copy a stack-stack, used for concurrency
 */
fungeStackStack * StackStackDuplicate(const fungeStackStack * restrict old) __attribute__((malloc,nonnull,warn_unused_result, FUNGE_IN_FAST));
#endif

/**
 * Begin a new stack on the stack stack.
 * count is how many arguments to copy over.
 */
bool StackStackBegin(struct s_instructionPointer * restrict ip,
                     fungeStackStack ** me,
                     FUNGEDATATYPE count,
                     const fungePosition * restrict storageOffset) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * End a stack on the stack-stack
 * count is how many items to copy over.
 */
bool StackStackEnd(struct s_instructionPointer * restrict ip,
                   fungeStackStack ** me,
                   FUNGEDATATYPE count) __attribute__((nonnull, warn_unused_result, FUNGE_IN_FAST));
/**
 * Transfer items from one stack to another (not in order).
 */
void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS)  __attribute__((nonnull, FUNGE_IN_FAST));

#endif

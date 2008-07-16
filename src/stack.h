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

/**
 * @file
 * Definition of, and functions for, a Funge stack.
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

/// A Funge stack.
/// @warning Don't access directly, use functions and macros below.
typedef struct s_fungeStack {
	size_t         size; ///< This is current size of the array entries.
	size_t         top;  /**< This is current top item in stack (may not be last item).
	                          Note: One-indexed, as 0 = empty stack. */
	FUNGEDATATYPE *entries; ///< Pointer to entries.
} fungeStack;

/// A Funge stack-stack.
typedef struct s_fungeStackStack {
	size_t         size;     ///< This is number of elements in stacks.
	size_t         current;  ///< Top stack and current stack.
	fungeStack   * stacks[]; ///< Array of pointers to stacks.
} fungeStackStack;

/**
 * Create a new stack.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
fungeStack * StackCreate(void);
/**
 * Destroy a stack.
 * @param stack Pointer to stack to free.
 */
FUNGE_ATTR_FAST
void StackFree(fungeStack * stack);

/**
 * Push a item on the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackPush(fungeStack * restrict stack, FUNGEDATATYPE value);
/**
 * Pop item from stack.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
FUNGEDATATYPE StackPop(fungeStack * restrict stack);
/**
 * Pop one item and discard it.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackPopDiscard(fungeStack * restrict stack);
/**
 * Pop a number of items and discard them.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackPopNDiscard(fungeStack * restrict stack, size_t n);
/**
 * Stack peek.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
FUNGEDATATYPE StackPeek(const fungeStack * restrict stack);
/**
 * Get an element from a specific position (counting from stack base).
 * Will return 0 if element isn't valid.
 * @note Index is one-based.
 * @param stack Pointer to stack to operate on.
 * @param index What index to operate on.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
FUNGEDATATYPE StackGetIndex(const fungeStack * restrict stack, size_t index);



/**
 * Push a vector.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackPushVector(fungeStack * restrict stack, const fungeVector * restrict value);
/**
 * Pop a vector.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
fungeVector StackPopVector(fungeStack * restrict stack);
/**
 * Push a null-terminated string to a 0"gnirts".
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackPushString(fungeStack * restrict stack, const char * restrict str, size_t len);
/**
 * Pop a 0"gnirts" and return a null-terminated string.
 * Use StackFreeString() to free the string.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
char * StackPopString(fungeStack * restrict stack);

#ifdef DISABLE_GC
/**
 * Free a 0"gnirts" that was popped with StackPopString().
 * Do NOT use for StackPopSizedString().
 */
#  define StackFreeString(string) cf_free(string)
#else
/**
 * Free a 0"gnirts" that was popped with StackPopString().
 * Do NOT use for StackPopSizedString().
 */
#  define StackFreeString(string) /* NO-OP */
#endif

/**
 * Pop a fixed number of chars from a stack.
 * @param stack A pointer to the stack in question.
 * @param len The number of chars to pop.
 * @return Result returned as null-terminated string.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
char * StackPopSizedString(fungeStack * restrict stack, size_t len);
/// Clear all items from a stack.
#define StackClear(stack) { stack->top = 0; }
/**
 * Duplicate top element of the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackDupTop(fungeStack * restrict stack);
/**
 * Swap the top two elements of the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackSwapTop(fungeStack * restrict stack);

/**
 * Print some tracing info.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void PrintStackTop(const fungeStack * stack);

//
// Stack-stack functions
//

/**
 * Create a new stack-stack.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
fungeStackStack * StackStackCreate(void);
/**
 * Free a stack-stack and any stacks it contain.
 */
FUNGE_ATTR_FAST
void StackStackFree(fungeStackStack * me);

#ifdef CONCURRENT_FUNGE
/**
 * Deep copy a stack-stack, used for concurrency.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
fungeStackStack * StackStackDuplicate(const fungeStackStack * restrict old);
#endif

/**
 * Begin a new stack on the stack-stack.
 * @param ip Instruction pointer this is for.
 * @param me What stack-stack to operate on.
 * @param count How many items to copy over.
 * @param storageOffset New storage offset.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
bool StackStackBegin(struct s_instructionPointer * restrict ip,
                     fungeStackStack ** me,
                     FUNGEDATATYPE count,
                     const fungePosition * restrict storageOffset);
/**
 * End a stack on the stack-stack.
 * @param ip Instruction pointer this is for.
 * @param me What stack-stack to operate on.
 * @param count How many items to copy over.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
bool StackStackEnd(struct s_instructionPointer * restrict ip,
                   fungeStackStack ** me,
                   FUNGEDATATYPE count);
/**
 * Transfer items from one stack to another (not in order).
 * Used for u instruction.
 * @param count How many items to copy over.
 * @param TOSS Pointer to top stack on the stack-stack.
 * @param SOSS Pointer to second stack on the stack-stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void StackStackTransfer(FUNGEDATATYPE count, fungeStack * restrict TOSS, fungeStack * restrict SOSS);

#endif

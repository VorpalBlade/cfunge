/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2009 Arvid Norlander <anmaster AT tele2 DOT se>
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

#ifndef FUNGE_HAD_SRC_STACK_H
#define FUNGE_HAD_SRC_STACK_H

#include "global.h"

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"

/// Forward decl, see ip.h
struct s_instructionPointer;

/// A Funge stack.
/// @warning Don't access directly, use functions and macros below.
typedef struct funge_stack {
	size_t      size;    ///< This is current size of the array entries.
	size_t      top;     /**< This is current top item in stack (may not be last item).
	                          Note: One-indexed, as 0 = empty stack. */
	funge_cell *entries; ///< Pointer to entries.
} funge_stack;

/// A Funge stack-stack.
typedef struct funge_stackstack {
	size_t         size;     ///< This is number of elements in stacks.
	size_t         current;  ///< Top stack and current stack.
	funge_stack  * stacks[]; ///< Array of pointers to stacks.
} funge_stackstack;

/**
 * Create a new stack.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
funge_stack * stack_create(void);
/**
 * Destroy a stack.
 * @param stack Pointer to stack to free.
 */
FUNGE_ATTR_FAST
void stack_free(funge_stack * stack);

/**
 * Push a item on the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_push(funge_stack * restrict stack, funge_cell value);
/**
 * Pop item from stack.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
funge_cell stack_pop(funge_stack * restrict stack);
/**
 * Pop a number of items and discard them.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_discard(funge_stack * restrict stack, size_t n);
/**
 * Stack peek.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
funge_cell stack_peek(const funge_stack * restrict stack);
/**
 * Get an element from a specific position (counting from stack base).
 * Will return 0 if element isn't valid.
 * @note Index is one-based.
 * @param stack Pointer to stack to operate on.
 * @param index What index to operate on.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
funge_cell stack_get_index(const funge_stack * restrict stack, size_t index);

/**
 * Find length of string on stack. Scans for first 0 from the top. Returns
 * length excluding the 0.
 * @param stack Pointer to stack to operate on.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST FUNGE_ATTR_PURE
size_t stack_strlen(const funge_stack * restrict stack);



/**
 * Push a vector.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_push_vector(funge_stack * restrict stack,
                       const funge_vector * restrict value);
/**
 * Pop a vector.
 */
FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
funge_vector stack_pop_vector(funge_stack * restrict stack);
/**
 * Push a null-terminated string to a 0"gnirts".
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_push_string(funge_stack * restrict stack,
                       const unsigned char * restrict str, size_t len);
/**
 * Pop a 0"gnirts" and return a null-terminated string.
 * Use stack_free_string() to free the string. This is due to that a different
 * allocation function may be used for these strings.
 * @param stack A pointer to the stack in question.
 * @param len If non-NULL, the string length is stored in this variable.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR((nonnull(1))) FUNGE_ATTR_FAST
unsigned char * stack_pop_string(funge_stack * restrict stack,
                                 size_t * restrict len);

/**
 * Free a 0"gnirts" that was popped with stack_pop_string().
 * Do NOT use for stack_pop_sized_string().
 */
#define stack_free_string(string) cf_free(string)

#ifdef UNUSED
/**
 * Pop a fixed number of chars from a stack.
 * @param stack A pointer to the stack in question.
 * @param len The number of chars to pop.
 * @return Result returned as null-terminated string.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
unsigned char * stack_pop_sized_string(funge_stack * restrict stack,
                                       size_t len);
#endif

/// Clear all items from a stack.
#define stack_clear(stack) { stack->top = 0; }
/**
 * Duplicate top element of the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_dup_top(funge_stack * restrict stack);
/**
 * Swap the top two elements of the stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stack_swap_top(funge_stack * restrict stack);

#ifndef DISABLE_TRACE
/**
 * Print some tracing info.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
void stack_print_top(const funge_stack * stack);
#endif

//
// Stack-stack functions
//

/**
 * Create a new stack-stack.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
funge_stackstack * stackstack_create(void);
/**
 * Free a stack-stack and any stacks it contain.
 */
FUNGE_ATTR_FAST
void stackstack_free(funge_stackstack * me);

#ifdef CONCURRENT_FUNGE
/**
 * Deep copy a stack-stack, used for concurrency.
 */
FUNGE_ATTR_MALLOC FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
funge_stackstack * stackstack_duplicate(const funge_stackstack * restrict old);
#endif

/**
 * Begin a new stack on the stack-stack.
 * @param ip Instruction pointer (will operate on it's stack stack).
 * @param count How many items to copy over.
 * @param storageOffset New storage offset.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
bool stackstack_begin(struct s_instructionPointer * restrict ip,
                      funge_cell count,
                      const funge_vector * restrict storageOffset);
/**
 * End a stack on the stack-stack.
 * @param ip Instruction pointer (will operate on it's stack stack).
 * @param count How many items to copy over.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_FAST
bool stackstack_end(struct s_instructionPointer * restrict ip,
                    funge_cell count);
/**
 * Transfer items from one stack to another (not in order).
 * Used for u instruction.
 * @param count How many items to copy over.
 * @param TOSS Pointer to top stack on the stack-stack.
 * @param SOSS Pointer to second stack on the stack-stack.
 */
FUNGE_ATTR_NONNULL FUNGE_ATTR_FAST
void stackstack_transfer(funge_cell count,
                         funge_stack * restrict TOSS,
                         funge_stack * restrict SOSS);

#endif

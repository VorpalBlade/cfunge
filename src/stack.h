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

#ifndef _HAD_SRC_STACK_H
#define _HAD_SRC_STACK_H

#include <sys/types.h>
#include <stdint.h>

#include "vector.h"
#include "funge-space/b93/funge-space.h"

typedef struct {
	// This is current size of the array entries
	size_t         size;
	// This is current top item in stack (may not be last item)
	// Note: One-indexed, as 0 = empty stack.
	size_t         top;
	fungePosition  storageOffset;
	FUNGEDATATYPE *entries;
} fungeStack;


typedef struct fungeStackEntry_ {
	fungeStack              * stack;
	struct fungeStackEntry_ * next;
	struct fungeStackEntry_ * previous;
} fungeStackEntry;


typedef struct {
	size_t            count;
	fungeStackEntry * current;
	fungeStackEntry * base;
	fungeStackEntry * top;
} fungeStackStack;


extern fungeStack  * StackCreate (void) __attribute__((warn_unused_result));
extern void          StackFree   (fungeStack * stack);

extern void          StackPush      (FUNGEDATATYPE value, fungeStack * stack) __attribute__((nonnull));
extern FUNGEDATATYPE StackPop       (fungeStack * stack) __attribute__((nonnull,warn_unused_result));
extern void          StackPopDiscard(fungeStack * stack) __attribute__((nonnull));
extern fungeVector   StackPopVector (fungeStack * stack) __attribute__((nonnull,warn_unused_result));
extern FUNGEDATATYPE StackPeek      (fungeStack * stack) __attribute__((nonnull,warn_unused_result));
extern void          StackClear     (fungeStack * stack) __attribute__((nonnull));
extern void          StackDupTop    (fungeStack * stack) __attribute__((nonnull));
extern void          StackSwapTop   (fungeStack * stack) __attribute__((nonnull));

extern fungeStackStack * StackStackCreate(void) __attribute__((warn_unused_result));

extern fungeStack      * StackStackBegin(fungeStackStack * stackStack, fungePosition * storageOffset, size_t count) __attribute__((nonnull,warn_unused_result));
extern fungeStack      * StackStackEnd(fungeStackStack * stackStack) __attribute__((nonnull,warn_unused_result));
extern fungeStack      * StackStackUnder(fungeStackStack * stackStack, size_t count) __attribute__((nonnull,warn_unused_result));

#endif

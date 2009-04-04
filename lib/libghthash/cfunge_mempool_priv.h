/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
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

// This file does not come from libght, however this code is used in the
// modified libght found in cfunge exclusively.

// This file is included several times in cfunge_mempool.c to create different
// implementations.


/*
The basic idea:
 * Pools consist of a header and a data block.
 * We allocate a huge array, of fixed size.
 * The pool headers are stored in an array, should one get filled we realloc
   that array and start a new pool. We never grow a pool itself.
Allocation strategy:
 * If first_free is not NULL we allocate from that linked list.
 * If first_free is NULL we try to allocate the next object after the last
   currently allocated block.
 * If that is not possible we check the next memory pool (if any) or create a
   new memory pool.

Notes:
 * For valgrind purposes we treat the whole thing as one pool.

*/

// This is either a memory block, or a pointer in the free list.
typedef union memory_block {
	// This should be first... Or mempool_free() won't work
	CF_MEMPOOL_DATATYPE data;
	// If NULL: end of list. If not in free list the value is undefined.
	union memory_block *next_free;
} memory_block;


typedef struct pool_header {
	memory_block  *base;
	// Pointer to first free block for mallocing.
	memory_block  *first_free;
} pool_header;

// This points to an array of pools.
static pool_header *pools = NULL;
// Size of pools array
static size_t       pools_size = 0;
// The free list
static memory_block *free_list = NULL;

// Forward decls:
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool CF_MEMPOOL_FUNC(initialise_mempool, CF_MEMPOOL_VARIANT)(pool_header *pool);

FUNGE_ATTR_FAST
static inline void CF_MEMPOOL_FUNC(clear_mempool, CF_MEMPOOL_VARIANT)(pool_header *pool);

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline bool CF_MEMPOOL_FUNC(add_mempool, CF_MEMPOOL_VARIANT)(void);

FUNGE_ATTR_FAST
static inline void CF_MEMPOOL_FUNC(freelist_add, CF_MEMPOOL_VARIANT)(memory_block *memblock);

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_MALLOC
static inline memory_block *CF_MEMPOOL_FUNC(freelist_get, CF_MEMPOOL_VARIANT)(void);

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_MALLOC
static inline memory_block *CF_MEMPOOL_FUNC(get_next_free, CF_MEMPOOL_VARIANT)(void);


FUNGE_ATTR_FAST
bool CF_MEMPOOL_FUNC(setup, CF_MEMPOOL_VARIANT)(void)
{
	assert(pools == NULL);
	pools = calloc_nogc(1, sizeof(pool_header));
	if (!pools)
		return false;
	// No it isn't really initialise_mempoold.
	// However valgrind will give false errors for freelist otherwise.
	VALGRIND_CREATE_MEMPOOL(pools, 0, true);
	pools_size = 1;
	return CF_MEMPOOL_FUNC(initialise_mempool, CF_MEMPOOL_VARIANT)(pools);
}


FUNGE_ATTR_FAST
void CF_MEMPOOL_FUNC(teardown, CF_MEMPOOL_VARIANT)(void)
{
	if (!pools)
		return;
	for (size_t i = 0; i < pools_size; i++) {
		CF_MEMPOOL_FUNC(clear_mempool, CF_MEMPOOL_VARIANT)(&pools[i]);
	}
	VALGRIND_DESTROY_MEMPOOL(pools);
	free_nogc(pools);
}


FUNGE_ATTR_FAST
CF_MEMPOOL_DATATYPE * CF_MEMPOOL_FUNC(alloc, CF_MEMPOOL_VARIANT)(void)
{
	memory_block *block = CF_MEMPOOL_FUNC(freelist_get, CF_MEMPOOL_VARIANT)();
	if (!block)
		block = CF_MEMPOOL_FUNC(get_next_free, CF_MEMPOOL_VARIANT)();
	if (block)
		return &block->data;
	return NULL;
}


FUNGE_ATTR_FAST
void CF_MEMPOOL_FUNC(free, CF_MEMPOOL_VARIANT)(CF_MEMPOOL_DATATYPE *ptr)
{
	CF_MEMPOOL_FUNC(freelist_add, CF_MEMPOOL_VARIANT)((memory_block*)ptr);
}


// Private functions

/// Setup a mempool, allocating it's block.
FUNGE_ATTR_FAST
static inline bool CF_MEMPOOL_FUNC(initialise_mempool, CF_MEMPOOL_VARIANT)(pool_header *pool)
{
	pool->base = malloc_nogc(sizeof(memory_block) * (POOL_ARRAY_COUNT + 1));
	if (!pool->base)
		return false;
	pool->first_free = pool->base;
	VALGRIND_MAKE_MEM_NOACCESS(pool->base, sizeof(memory_block) * POOL_ARRAY_COUNT);
	return true;
}

/// Tear down a mempool, freeing it's block.
FUNGE_ATTR_FAST
static inline void CF_MEMPOOL_FUNC(clear_mempool, CF_MEMPOOL_VARIANT)(pool_header *pool)
{
	if (!pool)
		return;
	if (pool->base)
		free_nogc(pool->base);
	pool->first_free = NULL;
}

/// add_mempools a new memory pool at the end of the array.
FUNGE_ATTR_FAST
static inline bool CF_MEMPOOL_FUNC(add_mempool, CF_MEMPOOL_VARIANT)(void)
{
	pool_header *pools_new = realloc_nogc(pools, sizeof(pool_header) * (pools_size + 1));
	if (!pools_new)
		return false;
	VALGRIND_MOVE_MEMPOOL(pools, pools_new);
	pools = pools_new;

	if (!CF_MEMPOOL_FUNC(initialise_mempool, CF_MEMPOOL_VARIANT)(&pools[pools_size])) {
		// Blergh...
		return false;
	}
	pools_size++;
	return true;
}

/// add_mempool a block to the list of free blocks.
FUNGE_ATTR_FAST
static inline void CF_MEMPOOL_FUNC(freelist_add, CF_MEMPOOL_VARIANT)(memory_block *memblock)
{
	memblock->next_free = free_list;
	free_list = memblock;
	VALGRIND_MEMPOOL_FREE(pools, memblock);
}

/// Try to get a block from the free block list.
FUNGE_ATTR_FAST
static inline memory_block *CF_MEMPOOL_FUNC(freelist_get, CF_MEMPOOL_VARIANT)(void)
{
	if (!free_list) {
		return NULL;
	} else {
		memory_block *block = free_list;
		VALGRIND_MEMPOOL_ALLOC(pools, block, sizeof(memory_block));
		free_list = free_list->next_free;
		return block;
	}
}

/// Get memory from the first mempool with blocks free at the end.
/// Will call add_mempool_mempool() if no blocks are free in the last mempool.
FUNGE_ATTR_FAST
static inline memory_block *CF_MEMPOOL_FUNC(get_next_free, CF_MEMPOOL_VARIANT)(void)
{
	pool_header* pool = &pools[pools_size-1];

	if ((pool->first_free - pool->base) >= (intptr_t)(POOL_ARRAY_COUNT - 1)) {
		if (!CF_MEMPOOL_FUNC(add_mempool, CF_MEMPOOL_VARIANT)())
			return NULL;
		pool = &pools[pools_size-1];
	}
	{
		memory_block* block = pool->first_free;
		pool->first_free++;
		VALGRIND_MEMPOOL_ALLOC(pools, block, sizeof(memory_block));
		return block;
	}
}

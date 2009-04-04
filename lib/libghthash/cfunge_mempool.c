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

// This file does not come from libght, however this code is for libght
// exclusively.


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

#include "cfunge_mempool.h"

#include <assert.h>

// Number of items in a pool.
#define POOL_ARRAY_COUNT 4096

#ifdef ENABLE_VALGRIND
#  include <valgrind/valgrind.h>
#  include <valgrind/memcheck.h>
#else
#  define VALGRIND_MAKE_MEM_NOACCESS(x, y) /* NO-OP */
#  define VALGRIND_CREATE_MEMPOOL(x, y, z) /* NO-OP */
#  define VALGRIND_DESTROY_MEMPOOL(x)      /* NO-OP */
#  define VALGRIND_MEMPOOL_ALLOC(x, y, z)  /* NO-OP */
#  define VALGRIND_MEMPOOL_FREE(x, y)      /* NO-OP */
#  define VALGRIND_MOVE_MEMPOOL(x, y)      /* NO-OP */
#endif


// I hate the preprocessor
#define CF_MEMPOOL_FUNC_INTERN(m_funcname, m_variant) \
	cf_mempool_ ## m_variant ## _ ## m_funcname

#define CF_MEMPOOL_FUNC(m_funcname, m_variant) \
	CF_MEMPOOL_FUNC_INTERN(m_funcname, m_variant)

#define CF_MEMPOOL_VARIANT  fspace
#define CF_MEMPOOL_DATATYPE struct s_fspace_hash_entry
#define memory_block        memory_block_fspace
#define pool_header         pool_fspace_header
#define pools               fspace_pools
#define pools_size          fspace_pools_size
#define free_list           fspace_free_list
#include "cfunge_mempool_priv.h"

#undef CF_MEMPOOL_VARIANT
#undef CF_MEMPOOL_DATATYPE
#undef memory_block
#undef pool_header
#undef pools
#undef pools_size
#undef free_list

#ifdef CFUN_EXACT_BOUNDS
#  define CF_MEMPOOL_VARIANT  fspacecount
#  define CF_MEMPOOL_DATATYPE struct s_fspacecount_hash_entry
#  define memory_block        memory_block_fspacecount
#  define pool_header         pool_fspacecount_header
#  define pools               fspacecount_pools
#  define pools_size          fspacecount_pools_size
#  define free_list           fspacecount_free_list
#  include "cfunge_mempool_priv.h"
#endif

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


/**
 * @file
 * Mempools are used for allocating Funge-space s_hash_entry.
 * Since cfunge is single-threaded they are static, and have no locking.
 *
 * For implementation details see comments in cfunge_mempool.c.
 */

#ifndef FUNGE_HAD_LIB_CFUNGE_MEMPOOL_H
#define FUNGE_HAD_LIB_CFUNGE_MEMPOOL_H

#include "../../src/global.h"
#include "../../src/funge-space/funge-space.h"

#include "ght_hash_table.h"

#define CF_MEMPOOL_FUNCPROT(m_variant, m_rettype, m_funcname, m_args, m_attrs) \
	m_attrs m_rettype cf_mempool_ ## m_variant ## _ ## m_funcname m_args

#define CF_MEMPOOL_DECLARE_FUNCS(m_variant, m_datatype) \
	CF_MEMPOOL_FUNCPROT(m_variant, bool,         setup,    (void), FUNGE_ATTR_FAST); \
	CF_MEMPOOL_FUNCPROT(m_variant, void,         teardown, (void), FUNGE_ATTR_FAST); \
	CF_MEMPOOL_FUNCPROT(m_variant, m_datatype *, alloc,    (void), FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_MALLOC); \
	CF_MEMPOOL_FUNCPROT(m_variant, void,         free,     (m_datatype *ptr), FUNGE_ATTR_FAST);


#include <stdbool.h>

#if 0
typedef struct s_hash_entry memorypool_data;

/**
 * Set up the memory pools. Used at program start.
 * @return True if successful, otherwise false.
 */
FUNGE_ATTR_FAST
bool mempool_setup(void);

/**
 * Tear down the memory pools. Used at program exit.
 */
FUNGE_ATTR_FAST
void mempool_teardown(void);

/**
 * Get a memorypool_data block from the memory pool.
 * @return A pointer to a memorypool_data, or NULL if allocation failed.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED FUNGE_ATTR_MALLOC
memorypool_data *mempool_alloc(void);

/**
 * Free a block, returning it to the memory pool.
 * @param ptr Pointer to memory block allocated with mempool_alloc.
 */
FUNGE_ATTR_FAST
void mempool_free(memorypool_data *ptr);
#endif

CF_MEMPOOL_DECLARE_FUNCS(fspace, struct s_fspace_hash_entry)
#ifdef CFUN_EXACT_BOUNDS
CF_MEMPOOL_DECLARE_FUNCS(fspacecount, struct s_fspacecount_hash_entry)
#endif

#undef CF_MEMPOOL_FUNCPROT
#undef CF_MEMPOOL_DECLARE_FUNCS

#ifdef CF_GHT_INTERNAL
#  define CF_MEMPOOL_FUNC_INTERN(m_variant, m_funcname) \
	cf_mempool_ ## m_variant ## _ ## m_funcname
#  define CF_MEMPOOL_FUNC(m_variant, m_funcname) \
	CF_MEMPOOL_FUNC_INTERN(m_variant, m_funcname)
#endif

#endif

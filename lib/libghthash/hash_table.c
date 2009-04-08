/*********************************************************************
 *
 * Copyright (C) 2001-2005,  Simon Kagstrom
 *
 * Filename:      hash_table.c
 * Description:   The implementation of the hash table (MK2).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * $Id: hash_table.c 15761 2007-07-15 06:08:52Z ska $
 *
 ********************************************************************/

#include <stdlib.h> /* malloc */
#include <stdio.h>  /* perror */
#include <errno.h>  /* errno */
#include <string.h> /* memcmp */
#include <stdbool.h>
#include <assert.h> /* assert */

#define CF_GHT_INTERNAL
#include "ght_hash_table.h"

#include "../../src/global.h"
#include "../../src/diagnostic.h"

#include "cfunge_mempool.h"

/* Flags for the elements. This is currently unused. */
#define FLAGS_NONE     0 /* No flags */
#define FLAGS_NORMAL   0 /* Normal item. All user-inserted stuff is normal */
#define FLAGS_INTERNAL 1 /* The item is internal to the hash table */

#define CF_GHT_VAR fspace
#define CF_GHT_KEY fungeSpaceHashKey
#define CF_GHT_DATA funge_cell
#define CF_GHT_COMPAREKEYS(m_a, m_b) (((m_a)->p_key.x == (m_b)->p_key.x) && ((m_a)->p_key.y == (m_b)->p_key.y))
#define CF_GHT_COPYKEY(m_target, m_source) \
	do { \
		(m_target).x = (m_source)->x; \
		(m_target).y = (m_source)->y; \
	} while (0)

#include "hash_table_priv.h"

#undef CF_GHT_VAR
#undef CF_GHT_KEY
#undef CF_GHT_DATA
#undef CF_GHT_COMPAREKEYS
#undef CF_GHT_COPYKEY

#ifdef CFUN_EXACT_BOUNDS
#  define CF_GHT_VAR fspacecount
#  define CF_GHT_KEY funge_cell
#  define CF_GHT_DATA funge_unsigned_cell
#  define CF_GHT_COMPAREKEYS(m_a, m_b) ((m_a)->p_key == (m_b)->p_key)
#  define CF_GHT_COPYKEY(m_target, m_source) \
	do { (m_target) = *(m_source); } while (0)

#  include "hash_table_priv.h"
#endif

/*-*-c-*- ************************************************************
 * Copyright (C) 2001-2005,  Simon Kagstrom
 *
 * Filename:      ght_hash_table.h.in
 * Description:   The definitions used in the hash table.
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
 * $Id: ght_hash_table.h.in 15761 2007-07-15 06:08:52Z ska $
 *
 ********************************************************************/

/**
 * @file
 * libghthash is a generic hash table used for storing arbitrary
 * data.
 *
 * Libghthash really stores pointers to data - the hash
 * table knows nothing about the actual type of the data.
 *
 * A simple example to get started can be found in the
 * <TT>example/simple.c</TT> file found in the distribution.
 * <TT>hash_test.c</TT> provides a more complete example.
 *
 * Some basic properties of the hash table are:
 *
 * - Both the data stored and the keys are of void type, which
 *   means that you can store any kind of data.
 *
 * - The only functions you probably will need to start is:
 *   - ght_create(), which creates a new hash table.
 *   - ght_insert(), which inserts a new entry into a table.
 *   - ght_get(), which searches for an entry.
 *   - ght_remove(), which removes and entry.
 *   - ght_finalize(), which destroys a hash table.
 *
 * - Inserting entries is done without first creating a key,
 *   i.e. you insert with the data, the datasize, the key and the
 *   key size directly.
 *
 * - The hash table copies the key data when inserting new
 *   entries. This means that you should <I>not</I> malloc() the key
 *   before inserting a new entry.
 *
 */
#ifndef GHT_HASH_TABLE_H
#define GHT_HASH_TABLE_H

#include "../../src/global.h"
#include "../../src/funge-space/funge-space.h"
#include <stdlib.h>                    /* size_t */
#include <stdint.h>
#include <stdbool.h>

#define GHT_HEURISTICS_NONE          0
#define GHT_HEURISTICS_TRANSPOSE     1
#define GHT_HEURISTICS_MOVE_TO_FRONT 2
#define GHT_AUTOMATIC_REHASH         4

#ifndef TRUE
#  define TRUE true
#endif

#ifndef FALSE
#  define FALSE false
#endif

// Use macros for some stuff.
#define GHT_USE_MACROS
// What checksum to use, one of:
//  one_at_a_time_hash
//  crc_hash
//  murmur_hash
// Be sure to change to #if 1 for the relevant function in hash_functions.c
#define GHT_HASH_NAME crc_hash
// Define to one of the GHT_HEURISTICS_* above
#define GHT_HEURISTICS GHT_HEURISTICS_NONE

/** unsigned 32 bit integer. */
typedef uint32_t ght_uint32_t;

#ifdef GHT_USE_MACROS
#  define ght_size(p_ht) (p_ht->i_items)
#  define ght_table_size(p_ht) (p_ht->i_size)
#endif

// I hate the preprocessor
#define CF_GHT_NAME_INTERN(m_prefix, m_variant, m_name) \
	m_prefix ## m_variant ## _ ## m_name

#define CF_GHT_NAME(m_variant, m_name) \
	CF_GHT_NAME_INTERN(ght_, m_variant, m_name)

#define CF_GHT_STRUCT(m_variant, m_name) \
	CF_GHT_NAME_INTERN(s_, m_variant, m_name)

/*
 * CF_GHT_VAR - Variant name
 * CF_GHT_KEY - Type of key
 * CF_GHT_DATA - Type of data
 */

// Create funge space storage.
#define CF_GHT_VAR fspace
#define CF_GHT_KEY fungeSpaceHashKey
#define CF_GHT_DATA funge_cell

#include "ght_hash_table_priv.h"

#undef CF_GHT_VAR
#undef CF_GHT_KEY
#undef CF_GHT_DATA

// Create column/row count variant.
#ifdef CFUN_EXACT_BOUNDS
#  define CF_GHT_VAR fspacecount
#  define CF_GHT_KEY funge_cell
#  define CF_GHT_DATA funge_unsigned_cell

#  include "ght_hash_table_priv.h"

#  undef CF_GHT_VAR
#  undef CF_GHT_KEY
#  undef CF_GHT_DATA
#endif

#ifndef CF_GHT_INTERNAL
#  undef CF_GHT_NAME_INTERN
#  undef CF_GHT_NAME
#  undef CF_GHT_STRUCT
#endif

#endif /* GHT_HASH_TABLE_H */

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
 * The structure for hash keys. You should not care about this
 * structure unless you plan to write your own hash functions.
 */
typedef struct CF_GHT_STRUCT(CF_GHT_VAR, hash_key) {
	CF_GHT_KEY p_key;    /**< The key. */
} CF_GHT_NAME(CF_GHT_VAR, hash_key_t);

/**
 * The structure for hash entries.
 */
typedef struct CF_GHT_STRUCT(CF_GHT_VAR, hash_entry) {
	CF_GHT_DATA p_data;

	struct CF_GHT_STRUCT(CF_GHT_VAR, hash_entry) *p_next;
	struct CF_GHT_STRUCT(CF_GHT_VAR, hash_entry) *p_prev;
	struct CF_GHT_STRUCT(CF_GHT_VAR, hash_entry) *p_older;
	struct CF_GHT_STRUCT(CF_GHT_VAR, hash_entry) *p_newer;
	CF_GHT_NAME(CF_GHT_VAR, hash_key_t) key;

} CF_GHT_NAME(CF_GHT_VAR, hash_entry_t);

/**
 * The structure used in iterations. You should not care about the
 * contents of this, it will be filled and updated by ght_first() and
 * ght_next().
 */
typedef struct {
	CF_GHT_NAME(CF_GHT_VAR, hash_entry_t) *p_entry; /* The current entry */
	CF_GHT_NAME(CF_GHT_VAR, hash_entry_t) *p_next;  /* The next entry */
} CF_GHT_NAME(CF_GHT_VAR, iterator_t);

/**
 * Definition of the hash function pointers. @c ght_fn_hash_t should be
 * used when implementing new hash functions. Look at the supplied
 * hash functions, like @c ght_one_at_a_time_hash(), for examples of hash
 * functions.
 *
 * @param p_key the key to calculate the hash value for.
 *
 * @return a 32 bit hash value.
 *
 * @see @c ght_one_at_a_time_hash(), @c ght_rotating_hash(),
 *      @c ght_crc_hash()
 */
// Not used as a typedef any longer.
//typedef ght_uint32_t (*ght_fn_hash_t)(const CF_GHT_NAME(CF_GHT_VAR, hash_key_t) *p_key) FUNGE_ATTR_FAST;

/**
 * The hash table structure.
 */
typedef struct CF_GHT_STRUCT(CF_GHT_VAR, hash_table) {
	size_t i_items;                    /**< The current number of items in the table */
	size_t i_size;                     /**< The number of buckets */
	bool i_automatic_rehash;           /**< TRUE if automatic rehashing is used */

	/* private: */
	int i_size_mask;                   /* The number of bits used in the size */
	CF_GHT_NAME(CF_GHT_VAR, hash_entry_t) **pp_entries;
	int *p_nr;                         /* The number of entries in each bucket */

	CF_GHT_NAME(CF_GHT_VAR, hash_entry_t) *p_oldest;        /* The entry inserted the earliest. */
	CF_GHT_NAME(CF_GHT_VAR, hash_entry_t) *p_newest;        /* The entry inserted the latest. */
} CF_GHT_NAME(CF_GHT_VAR, hash_table_t);

/**
 * Create a new hash table. The number of buckets should be about as
 * big as the number of elements you wish to store in the table for
 * good performance. The number of buckets is rounded to the next
 * higher power of two.
 *
 * The hash table is created with @c ght_one_at_a_time_hash() as hash
 * function, automatic rehashing disabled, @c cf_malloc() as the memory
 * allocator and no heuristics.
 *
 * @param i_size the number of buckets in the hash table. Giving a
 *        non-power of two here will round the size up to the next
 *        power of two.
 *
 * @see ght_set_hash(), ght_set_heuristics(), ght_set_rehash(),
 * @see ght_set_alloc()
 *
 * @return a pointer to the hash table or NULL upon error.
 */
FUNGE_ATTR_FAST
CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *CF_GHT_NAME(CF_GHT_VAR, create)(size_t i_size);

/**
 * Enable or disable automatic rehashing.
 *
 * With automatic rehashing, the table will rehash itself when the
 * number of elements in the table are twice as many as the number of
 * buckets. You should note that automatic rehashing will cause your
 * application to be really slow when the table is rehashing (which
 * might happen at times when you need speed), you should therefore be
 * careful with this in time-constrainted applications.
 *
 * @param p_ht the hash table to set rehashing for.
 * @param b_rehash TRUE if rehashing should be used or FALSE if it
 *        should not be used.
 */
FUNGE_ATTR_FAST
void CF_GHT_NAME(CF_GHT_VAR, set_rehash)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht,
                                         bool b_rehash);

#ifndef GHT_USE_MACROS
/**
 * Get the size (the number of items) of the hash table.
 *
 * @param p_ht the hash table to get the size for.
 *
 * @return the number of items in the hash table.
 */
size_t CF_GHT_NAME(CF_GHT_VAR, size)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht);

/**
 * Get the table size (the number of buckets) of the hash table.
 *
 * @param p_ht the hash table to get the table size for.
 *
 * @return the number of buckets in the hash table.
 */
size_t CF_GHT_NAME(CF_GHT_VAR, table_size)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht);
#endif

/**
 * Insert an entry into the hash table. Prior to inserting anything,
 * make sure that the table is created with ght_create(). If an
 * element with the same key as this one already exists in the table,
 * the insertion will fail and -1 is returned.
 *
 * A typical example is shown below, where the string "blabla"
 * (including the '\0'-terminator) is used as a key for the integer
 * 15.
 *
 * <PRE>
 * CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_table;
 * char *p_key_data;
 * int *p_data;
 * int ret;
 *
 * [Create p_table etc...]
 * p_data = malloc(sizeof(int));
 * p_key_data = "blabla";
 * *p_data = 15;
 *
 * ret = ght_insert(p_table,
 *                  p_data,
 *                  sizeof(char)*(strlen(p_key_data)+1), p_key_data);
 * </PRE>
 *
 * @param p_ht the hash table to insert into.
 * @param p_entry_data the data to insert.
 * @param i_key_size the size of the key to associate the data with (in bytes).
 * @param p_key_data the key to use. The value will be copied, and it
 *                   is therefore OK to use a stack-allocated entry here.
 *
 * @return 0 if the element could be inserted, -1 otherwise.
 */
FUNGE_ATTR_FAST
int CF_GHT_NAME(CF_GHT_VAR, insert)(
        CF_GHT_NAME(CF_GHT_VAR, hash_table_t) * restrict p_ht,
        CF_GHT_DATA p_entry_data,
        const CF_GHT_KEY * restrict p_key_data);

/**
 * Replace an entry in the hash table. This function will return an
 * error if the entry to be replaced does not exist, i.e. it cannot be
 * used to insert new entries. Replacing an entry does not affect its
 * iteration order.
 *
 * @param p_ht the hash table to search in.
 * @param p_entry_data the new data for the key.
 * @param i_key_size the size of the key to search with (in bytes).
 * @param p_key_data the key to search for.
 *
 * @return a pointer to the <I>old</I> value or NULL if the operation failed.
 */
FUNGE_ATTR_FAST
CF_GHT_DATA CF_GHT_NAME(CF_GHT_VAR, replace)(
        CF_GHT_NAME(CF_GHT_VAR, hash_table_t) * restrict p_ht,
        CF_GHT_DATA p_entry_data,
        const CF_GHT_KEY * restrict p_key_data);


/**
 * Lookup an entry in the hash table. The entry is <I>not</I> removed from
 * the table.
 *
 * @param p_ht the hash table to search in.
 * @param i_key_size the size of the key to search with (in bytes).
 * @param p_key_data the key to search for.
 *
 * @return a pointer to the found entry or NULL if no entry could be found.
 */
FUNGE_ATTR_FAST
CF_GHT_DATA *CF_GHT_NAME(CF_GHT_VAR, get)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) * restrict p_ht,
        const CF_GHT_KEY * restrict p_key_data);

/**
 * Remove an entry from the hash table. The entry is removed from the
 * table, but not freed (that is, the data stored is not freed).
 *
 * @param p_ht the hash table to use.
 * @param i_key_size the size of the key to search with (in bytes).
 * @param p_key_data the key to search for.
 *
 * @return a pointer to the removed entry or NULL if the entry could be found.
 */
FUNGE_ATTR_FAST
CF_GHT_DATA CF_GHT_NAME(CF_GHT_VAR, remove)(
        CF_GHT_NAME(CF_GHT_VAR, hash_table_t) * restrict p_ht,
        const CF_GHT_KEY * restrict p_key_data);

/**
 * Return the first entry in the hash table. This function should be
 * used for iteration and is used together with ght_next(). The order
 * of the entries will be from the oldest inserted entry to the newest
 * inserted entry. If an entry is inserted during an iteration, the entry
 * might or might not occur in the iteration. Note that removal during
 * an iteration is only safe for the <I>current</I> entry or an entry
 * which has <I>already been iterated over</I>.
 *
 * The use of the ght_foo_iterator_t allows for several concurrent
 * iterations, where you would use one ght_foo_iterator_t for each
 * iteration. In threaded environments, you should still lock access
 * to the hash table for insertion and removal.
 *
 * A typical example might look as follows:
 * <PRE>
 * ght_foo_hash_table_t *p_table;
 * ght_foo_iterator_t iterator;
 * void *p_key;
 * void *p_e;
 *
 * [Create table etc...]
 * for(p_e = ght_first(p_table, &iterator, &p_key); p_e; p_e = ght_next(p_table, &iterator, &p_key))
 *   {
 *      [Do something with the current entry p_e and it's key p_key]
 *   }
 * </PRE>
 *
 * @param p_ht the hash table to iterate through.
 *
 * @param p_iterator the iterator to use. The value of the structure
 * is filled in by this function and may be stack allocated.
 *
 * @param pp_key a pointer to the pointer of the key (NULL if none).
 *
 * @return a pointer to the first entry in the table or NULL if there
 * are no entries.
 *
 *
 * @see ght_next()
 */
FUNGE_ATTR_FAST
void *CF_GHT_NAME(CF_GHT_VAR, first)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht,
                                     CF_GHT_NAME(CF_GHT_VAR, iterator_t) *p_iterator,
                                     const CF_GHT_KEY **pp_key);

/**
 * Return the next entry in the hash table. This function should be
 * used for iteration, and must be called after ght_first().
 *
 * @warning calling this without first having called ght_first will
 * give undefined results (probably a crash), since p_iterator isn't
 * filled correctly.
 *
 * @param p_iterator the iterator to use.
 *
 * @param pp_key a pointer to the pointer of the key (NULL if none).
 *
 * @return a pointer to the next entry in the table or NULL if there
 * are no more entries in the table.
 *
 * @see ght_first()
 */
FUNGE_ATTR_FAST
void *CF_GHT_NAME(CF_GHT_VAR, next)(
        CF_GHT_NAME(CF_GHT_VAR, iterator_t) *p_iterator,
        const CF_GHT_KEY **pp_key);

/**
 * Rehash the hash table.
 *
 * Rehashing will change the size of the hash table, retaining all
 * elements. This is very costly and should be avoided unless really
 * needed. If <TT>GHT_AUTOMATIC_REHASH</TT> is specified in the flag
 * parameter when ght_create() is called, the hash table is
 * automatically rehashed when the number of stored elements exceeds
 * two times the number of buckets in the table (making calls to this
 * function unnecessary).
 *
 * @param p_ht the hash table to rehash.
 * @param i_size the new size of the table.
 *
 * @see ght_create()
 */
FUNGE_ATTR_FAST
void CF_GHT_NAME(CF_GHT_VAR, rehash)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht,
                                     size_t i_size);

/**
 * Free the hash table. ght_finalize() should typically be called
 * at the end of the program. Note that only the metadata and the keys
 * of the table is freed, not the entries. If you want to free the
 * entries when removing the table, the entries will have to be
 * manually freed before ght_finalize() is called like:
 *
 * <PRE>
 * CF_GHT_NAME(CF_GHT_VAR, iterator_t) iterator;
 * void *p_key;
 * void *p_e;
 *
 * for(p_e = ght_first(p_table, &iterator, &p_key); p_e; p_e = ght_next(p_table, &iterator, &p_key))
 *   {
 *     free(p_e);
 *   }
 *
 * ght_finalize(p_table);
 * </PRE>
 *
 * @param p_ht the table to remove.
 */
FUNGE_ATTR_FAST
void CF_GHT_NAME(CF_GHT_VAR, finalize)(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht);

/* exported hash functions */

/**
 * One-at-a-time-hash. One-at-a-time-hash is a good hash function, and
 * is the default when ght_create() is called with NULL as the
 * fn_hash parameter. This was found in a DrDobbs article, see
 * http://burtleburtle.net/bob/hash/doobs.html
 *
 * @warning Don't call this function directly, it is only meant to be
 * used as a callback for the hash table.
 *
 * @see ght_fn_hash_t
 * @see ght_rotating_hash(), ght_crc_hash()
 */
FUNGE_ATTR_FAST
ght_uint32_t CF_GHT_NAME(CF_GHT_VAR, one_at_a_time_hash)(const CF_GHT_NAME(CF_GHT_VAR, hash_key_t) *p_key);

/**
 * CRC32 hash. CRC32 hash is a good hash function. This came from Dru
 * Lemley <spambait@lemley.net>.
 *
 * @warning Don't call this function directly, it is only meant to be
 * used as a callback for the hash table.
 *
 * @see ght_fn_hash_t
 * @see ght_one_at_a_time_hash(), ght_rotating_hash()
 */
FUNGE_ATTR_FAST
ght_uint32_t CF_GHT_NAME(CF_GHT_VAR, crc_hash)(const CF_GHT_NAME(CF_GHT_VAR, hash_key_t) *p_key);

// Fast hash, sometimes better than CRC, sometimes worse.
FUNGE_ATTR_FAST
ght_uint32_t CF_GHT_NAME(CF_GHT_VAR, murmur_hash)(const CF_GHT_NAME(CF_GHT_VAR, hash_key_t) *p_key);

#ifdef USE_PROFILING
/**
 * Print some statistics about the table. Only available if the
 * library was compiled with <TT>USE_PROFILING</TT> defined.
 */
void ght_print(CF_GHT_NAME(CF_GHT_VAR, hash_table_t) *p_ht);
#endif

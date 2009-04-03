/*********************************************************************
 *
 * Copyright (C) 2001-2002,  Simon Kagstrom
 *
 * Filename:      hash_functions.c
 * Description:   Hash functions
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
 * $Id: hash_functions.c 2174 2005-03-18 07:00:30Z ska $
 *
 ********************************************************************/

#if 0
/* One-at-a-time hash (found in a web article from ddj), this is the
 * standard hash function.
 *
 * See http://burtleburtle.net/bob/hash/doobs.html
 * for the hash functions used here.
 */
FUNGE_ATTR_FAST ght_uint32_t ght_one_at_a_time_hash(const ght_hash_key_t *p_key)
{
	ght_uint32_t i_hash = 0;
	size_t i;

	assert(p_key != NULL);

	for (i = 0; i < sizeof(fungeSpaceHashKey); ++i) {
		i_hash += ((const unsigned char*)&(p_key->p_key))[i];
		i_hash += (i_hash << 10);
		i_hash ^= (i_hash >> 6);
	}
	i_hash += (i_hash << 3);
	i_hash ^= (i_hash >> 11);
	i_hash += (i_hash << 15);

	return i_hash;
}
#endif

#if 1
/* CRC32 hash based on code from comp.compression FAQ.
 * Added by Dru Lemley <spambait@lemley.net>
 */
FUNGE_ATTR_FAST ght_uint32_t CF_GHT_NAME(CF_GHT_VAR, crc_hash)(const CF_GHT_NAME(CF_GHT_VAR, hash_key_t) *p_key)
{
	const unsigned char *p;
	ght_uint32_t  crc;

	assert(p_key != NULL);

	crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */
	p = (const unsigned char *)&(p_key->p_key);

	for (size_t i = 0; i < sizeof(CF_GHT_KEY); i++)
		crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ (p[i])];

	return ~crc;            /* transmit complement, per CRC-32 spec */
}
#endif

#if 0
#ifdef USE64
FUNGE_ATTR_FAST
static inline ght_uint32_t MurmurHash2(const fungeSpaceHashKey * key)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const ght_uint32_t m = 0x5bd1e995;
	const int32_t r = 24;

	// Initialise the hash to a 'random' value
	size_t len = sizeof(fungeSpaceHashKey);
	ght_uint32_t h = 0x7fd652ad ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		ght_uint32_t k = *(const ght_uint32_t *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Not needed the way we use it.
#if 0
	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};
#endif

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
#endif

/* CRC32 hash based on code from comp.compression FAQ.
 * Added by Dru Lemley <spambait@lemley.net>
 */
FUNGE_ATTR_FAST ght_uint32_t murmur_hash(const ght_hash_key_t *p_key)
{
#ifdef USE32
	const ght_uint32_t m = 0xc6a4a793;

	ght_uint32_t h = 0x7fd652ad ^ (8 * m), k;

	k = p_key->p_key.x; k *= m; k ^= k >> 16; k *= m; h += k; h *= m;
	k = p_key->p_key.y; k *= m; k ^= k >> 16; k *= m; h += k; h *= m;

	h *= m; h ^= h >> 10;
	h *= m; h ^= h >> 17;

	return h;
#else
	return MurmurHash2(&p_key->p_key);
#endif
}
#endif

/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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


#include "global.h"
#include "prng.h"

#include "diagnostic.h"

#if defined(CFUN_KLEE_TEST) || defined(AFL_FUZZ_TESTING)
#  undef HAVE_arc4random_buf
#endif

#ifdef HAVE_arc4random_buf
#  define HAVE_ARC4RANDOM
#  ifndef ARC4RANDOM_IN_BSD
#    include <stdlib.h>
#  else
// Workaround for broken header
typedef unsigned char u_char;
#    include <bsd/stdlib.h>
#  endif
#endif

#ifndef HAVE_ARC4RANDOM
#  include <errno.h>
#  include <stdlib.h>
#  include <string.h> /* strerror */

#  ifdef HAVE_clock_gettime
#    include <time.h>
#  else
#    include <sys/time.h>
#  endif

#  if !defined(HAVE_random) || !defined(HAVE_srandom)
#    define random rand
#    define srandom srand
#  endif

#endif

#ifndef HAVE_ARC4RANDOM
static void setup_libc_random(void)
{
#if defined(CFUN_KLEE_TEST) || defined(AFL_FUZZ_TESTING)
	// Make klee tests deterministic.
	srandom(4);
#elif defined(HAVE_clock_gettime)
	struct timespec tv;
	if (FUNGE_UNLIKELY(clock_gettime(CLOCK_REALTIME, &tv))) {
		diag_fatal_format("clock_gettime() failed (needed for random seed): %s", strerror(errno));
	}
	// Set up randomness
	srandom((unsigned int)tv.tv_nsec);
#else
	struct timeval tv;
	if (FUNGE_UNLIKELY(gettimeofday(&tv, NULL))) {
		diag_fatal_format("gettimeofday() failed (needed for random seed): %s", strerror(errno));
	}
	// Set up randomness
	srandom((unsigned int)tv.tv_usec);
#endif
}
#endif

#ifdef HAVE_ARC4RANDOM
// This is arc4random_uniform wholesale (BSD license),
// but adapted to work with more than 32 bits.
/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound). This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline funge_unsigned_cell cfun_arc4random_range(funge_unsigned_cell upper_bound)
{
	funge_unsigned_cell r, min;

	if (upper_bound < 2)
		return 0;

	/* 2**n % x == (2**n - x) % x */
	min = -upper_bound % upper_bound;
	/*
	* This could theoretically loop forever but each retry has
	* p > 0.5 (worst case, usually far better) of selecting a
	* number inside the range we need, so it should rarely need
	* to re-roll.
	*/
	for (;;) {
		arc4random_buf(&r, sizeof(funge_unsigned_cell));
		if (r >= min)
			break;
	}

	return r % upper_bound;
}
#endif

// Sets up random seed from time.
void prng_init(void)
{
#ifdef HAVE_ARC4RANDOM
	arc4random_stir();
#else
	setup_libc_random();
#endif
}

FUNGE_ATTR_FAST
funge_unsigned_cell prng_generate_unsigned(funge_unsigned_cell max_value)
{
#if defined(HAVE_ARC4RANDOM)
	return cfun_arc4random_range(max_value);
#else
	return random() % max_value;
#endif
}

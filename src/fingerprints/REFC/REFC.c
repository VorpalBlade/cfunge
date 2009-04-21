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

#include "REFC.h"
#include "../../stack.h"
#include "../../vector.h"

#include <assert.h>

#define ALLOCCHUNK 5
// Array holding references.
static funge_vector *references = NULL;
// Top index used in array.
static size_t referencesTop = 0;
// Size of array (including allocated but not yet used elements).
static size_t referencesSize = 0;

static void finger_REFC_reference(instructionPointer * ip)
{
	funge_cell x, y;
	y = stack_pop(ip->stack);
	x = stack_pop(ip->stack);
	if (referencesSize == referencesTop + 1) {
		funge_vector * newrefs = (funge_vector*)cf_realloc(references, (referencesSize + ALLOCCHUNK) * sizeof(funge_vector));
		if (newrefs == NULL) {
			ip_reverse(ip);
			return;
		} else {
			references = newrefs;
			referencesSize += ALLOCCHUNK;
		}
	}
	// TODO: Return same value for the same cell each time!
	// Yes cell 0 will never be used, but that is a hack to prevent having
	// errors on someone doing 0D before they do any R.
	referencesTop++;
	references[referencesTop].x = x;
	references[referencesTop].y = y;
	stack_push(ip->stack, (funge_cell)referencesTop);
}

static void finger_REFC_dereference(instructionPointer * ip)
{
	funge_cell ref;
	ref = stack_pop(ip->stack);
	if ((ref <= 0) || ((size_t)ref > referencesTop)) {
		ip_reverse(ip);
		return;
	}
	stack_push_vector(ip->stack, &references[ref]);
}

FUNGE_ATTR_FAST static inline bool init_references(void)
{
	assert(!references);
	references = (funge_vector*)cf_malloc_noptr(ALLOCCHUNK * sizeof(funge_vector));
	if (!references)
		return false;
	referencesSize = ALLOCCHUNK;
	return true;
}


bool finger_REFC_load(instructionPointer * ip)
{
	if (!references)
		if (!init_references())
			return false;
	manager_add_opcode(REFC, 'D', dereference)
	manager_add_opcode(REFC, 'R', reference)
	return true;
}

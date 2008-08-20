/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
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

#include "INDV.h"
#include "../../stack.h"

/// This simply fetches the second vector.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
static inline fungeVector GetSecondVector(instructionPointer * restrict ip)
{
	fungeVector a, b;

	a = StackPopVector(ip->stack);
	// Add first level of storage offset...
	a.x += ip->storageOffset.x;
	a.y += ip->storageOffset.y;
	b.x = FungeSpaceGet(VectorCreateRef(a.x+1, a.y));
	b.y = FungeSpaceGet(&a);
	// Add in second level of storage offset...
	b.x += ip->storageOffset.x;
	b.y += ip->storageOffset.y;
	return b;
}

/// G - Get value using indirect vector
static void FingerINDVgetNum(instructionPointer * ip)
{
	fungeVector v = GetSecondVector(ip);
	StackPush(ip->stack, FungeSpaceGet(&v));
}

/// P - Put value using indirect vector
static void FingerINDVputNum(instructionPointer * ip)
{
	fungeVector v = GetSecondVector(ip);
	FungeSpaceSet(StackPop(ip->stack), &v);
}

/// V - Get vector using indirect vector
static void FingerINDVgetVec(instructionPointer * ip)
{
	fungeVector v = GetSecondVector(ip);
	StackPushVector(ip->stack,
		VectorCreateRef(
			FungeSpaceGet(&v),
			FungeSpaceGet(VectorCreateRef(v.x+1, v.y))
		)
	);
}

/// W - Write vector using indirect vector
static void FingerINDVputVec(instructionPointer * ip)
{
	fungeVector a, b;
	a = GetSecondVector(ip);
	b = StackPopVector(ip->stack);
	FungeSpaceSet(b.y, &a);
	FungeSpaceSet(b.x, VectorCreateRef(a.x+1, a.y));
}

bool FingerINDVload(instructionPointer * ip)
{
	ManagerAddOpcode(INDV,  'G', getNum)
	ManagerAddOpcode(INDV,  'P', putNum)
	ManagerAddOpcode(INDV,  'V', getVec)
	ManagerAddOpcode(INDV,  'W', putVec)
	return true;
}

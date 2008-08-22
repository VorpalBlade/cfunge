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

#include "3DSP.h"
#include "../../stack.h"

#include <math.h>

// Yeah, some systems are *really* crap.
// This includes Mingw on windows when I tried.
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

// This is based on the CCBI 3DSP fingerprint code.

/// An union for float and 32-bit int.
typedef union u_floatint {
	float f;
	int32_t i;
} floatint;


/********************
 * Helper functions *
 ********************/

FUNGE_ATTR_FAST
static inline float PopFloat(instructionPointer * restrict ip)
{
	floatint u;
	u.i = StackPop(ip->stack);
	return u.f;
}

FUNGE_ATTR_FAST
static inline void PushFloat(instructionPointer * restrict ip, float f)
{
	floatint u;
	u.f = f;
	StackPush(ip->stack, u.i);
}


FUNGE_ATTR_FAST
static inline void PopVecF(instructionPointer * restrict ip, float vec[3])
{
	vec[2] = PopFloat(ip);
	vec[1] = PopFloat(ip);
	vec[0] = PopFloat(ip);
}

FUNGE_ATTR_FAST
static inline void PushVecF(instructionPointer * restrict ip, const float vec[3])
{
	PushFloat(ip, vec[0]);
	PushFloat(ip, vec[1]);
	PushFloat(ip, vec[2]);
}

FUNGE_ATTR_FAST
static inline void PopVec(instructionPointer * restrict ip, double vec[3])
{
	vec[2] = PopFloat(ip);
	vec[1] = PopFloat(ip);
	vec[0] = PopFloat(ip);
}

FUNGE_ATTR_FAST
static inline void PushVec(instructionPointer * restrict ip, const double vec[3])
{
	PushFloat(ip, vec[0]);
	PushFloat(ip, vec[1]);
	PushFloat(ip, vec[2]);
}

FUNGE_ATTR_FAST
static inline double VectorLength(const double vec[3])
{
	return sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
}

FUNGE_ATTR_FAST
static inline void writeMatrix(const instructionPointer * restrict ip,
                               const fungeVector * restrict fV, const double m[16])
{
	for (fungeCell i = 0; i < 4; ++i) {
		for (fungeCell j = 0; j < 4; ++j) {
			floatint u;
			u.f = m[4*j + i];
			FungeSpaceSetOff(u.i, VectorCreateRef(fV->x+i, fV->y+j), &ip->storageOffset);
		}
	}
}

FUNGE_ATTR_FAST
static inline void readMatrix(const instructionPointer * restrict ip,
                              const fungeVector * restrict fV, double m[16])
{
	for (fungeCell x = 0; x < 4; ++x) {
		for (fungeCell y = 0; y < 4; ++y) {
			floatint u;
			u.i = FungeSpaceGetOff(VectorCreateRef(fV->x+x, fV->y+y), &ip->storageOffset);
			m[y*4 + x] = u.f;
		}
	}
}

FUNGE_ATTR_FAST
static inline void mulMatrixVector(const double m[16], const double v[4], double r[4])
{
	for (size_t i = 0; i < 4; ++i) {
		double n = 0;
		for (size_t k = 0; k < 4; ++k)
			n += m[i*4 + k] * v[k];
		r[i] = n;
	}
}

FUNGE_ATTR_FAST
static inline void mulMatrices(const double a[16], const double b[16], double r[16])
{
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			double n = 0;
			for (size_t k = 0; k < 4; ++k)
				n += a[i*4 + k] * b[k*4 + j];
			r[i*4 + j] = n;
		}
	}
}

/****************************
 * Fingerprint instructions *
 ****************************/

/// A - Add two 3d vectors
static void Finger3DSPadd(instructionPointer * ip)
{
	double a[3], b[3];
	PopVec(ip, b);
	PopVec(ip, a);

	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
	PushVec(ip, a);
}

/// B - Subtract two 3d vectors
static void Finger3DSPsub(instructionPointer * ip)
{
	double a[3], b[3];
	PopVec(ip, b);
	PopVec(ip, a);

	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
	PushVec(ip, a);
}

/// C - Cross porduct of two vectors
static void Finger3DSPcross(instructionPointer * ip)
{
	double a[3], b[3], c[3];
	PopVec(ip, b);
	PopVec(ip, a);

	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];

	PushVec(ip, c);
}

/// D - Dot product of two vector
static void Finger3DSPdot(instructionPointer * ip)
{
	double a[3], b[3];
	PopVec(ip, b);
	PopVec(ip, a);

	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];

	PushFloat(ip, a[0] + a[1] + a[2]);
}

/// L - Length of vector
static void Finger3DSPlength(instructionPointer * ip)
{
	double a[3];
	PopVec(ip, a);
	PushFloat(ip, VectorLength(a));
}

/// M - Multiply two 3d vectors
static void Finger3DSPmul(instructionPointer * ip)
{
	double a[3], b[3];
	PopVec(ip, b);
	PopVec(ip, a);

	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];

	PushVec(ip, a);
}

/// N - Normalize vector (sets length to 1)
static void Finger3DSPnormalise(instructionPointer * ip)
{
	double a[3];
	double len;

	PopVec(ip, a);
	len = VectorLength(a);

	a[0] /= len;
	a[1] /= len;
	a[2] /= len;

	PushVec(ip, a);
}

/// P - Copy a matrix
static void Finger3DSPmatrixCopy(instructionPointer * ip)
{
	fungeVector fs, ft;

	fs = StackPopVector(ip->stack);
	ft = StackPopVector(ip->stack);
	// Add in storage offset
	fs.x += ip->storageOffset.x;
	fs.y += ip->storageOffset.y;
	ft.x += ip->storageOffset.x;
	ft.y += ip->storageOffset.y;


	for (fungeCell x = 0; x < 4; ++x)
		for (fungeCell y = 0; y < 4; ++y) {
			FungeSpaceSet(FungeSpaceGet(VectorCreateRef(fs.x+x, fs.y+y)),
			              VectorCreateRef(ft.x+x, ft.y+y));
		}
}

/// R - Generate a rotation matrix
static void Finger3DSPmatrixRotate(instructionPointer * ip)
{
	double s, c;
	double angle = PopFloat(ip);
	fungeCell axis = StackPop(ip->stack);
	fungeVector fV = StackPopVector(ip->stack);

	if (!(axis >= 1 && axis <= 3)) {
		ipReverse(ip);
		return;
	}

	angle *= M_PI/180;

	s = sin(angle);
	c = cos(angle);

	switch (axis) {
		case 1: {
			double m[16] = { 1, 0, 0, 0
			               , 0, c,-s, 0
			               , 0, s, c, 0
			               , 0, 0, 0, 1};
			writeMatrix(ip, &fV, m);
			break;
		}
		case 2: {
			double m[16] = { c, 0, s, 0
			               , 0, 1, 0, 0
			               ,-s, 0, c, 0
			               , 0, 0, 0, 1};
			writeMatrix(ip, &fV, m);
			break;
		}
		case 3: {
			double m[16] = { c,-s, 0, 0
			               , s, c, 0, 0
			               , 0, 0, 1, 0
			               , 0, 0, 0, 1};
			writeMatrix(ip, &fV, m);
			break;
		}
	}
}

/// S - Generate a scale matrix
static void Finger3DSPmatrixScale(instructionPointer * ip)
{
	double v[3];
	fungeVector fV;
	PopVec(ip, v);
	fV = StackPopVector(ip->stack);
	{
		double matrix[16] = {v[0],   0,   0,   0
		                    ,   0,v[1],   0,   0
		                    ,   0,   0,v[2],   0
		                    ,   0,   0,   0,   1};
		writeMatrix(ip, &fV, matrix);
	}
}

/// T - Generate a translation matrix
static void Finger3DSPmatrixTranslate(instructionPointer * ip)
{
	double v[3];
	fungeVector fV;
	PopVec(ip, v);
	fV = StackPopVector(ip->stack);

	{
		double matrix[16] = {   1,   0,   0,   v[0]
		                    ,   0,   1,   0,   v[1]
		                    ,   0,   0,   1,   v[2]
		                    ,   0,   0,   0,   1};
		writeMatrix(ip, &fV, matrix);
	}
}

/// U - Duplicate vector on top of stack
static void Finger3DSPduplicate(instructionPointer * ip)
{
	float a[3];
	PopVecF(ip, a);
	PushVecF(ip, a);
	PushVecF(ip, a);
}

/// V - Map 3d point to 2d view
static void Finger3DSPmap(instructionPointer * ip)
{
	double v[3];

	PopVec(ip, v);
	// Use fpclassify() to avoid:
	// "warning: comparing floating point with == or != is unsafe".
	if (fpclassify(v[2]) != FP_ZERO) {
		v[0] /= v[2];
		v[1] /= v[2];
		v[2] /= v[2];
	}
	PushFloat(ip, v[0]);
	PushFloat(ip, v[1]);
}

/// X - Transform a vector using transformation matrix
static void Finger3DSPtransform(instructionPointer * ip)
{
	fungeVector fm;
	double v[4];
	double m[16];
	double r[4];

	fm = StackPopVector(ip->stack);

	PopVec(ip, v);
	v[3] = 1;

	readMatrix(ip, &fm, m);

	mulMatrixVector(m, v, r);
	PushVec(ip, r);
}

/// Y - Multiply two matrices
static void Finger3DSPmatrixMul(instructionPointer * ip)
{
	fungeVector ft, fa, fb;
	double a[16], b[16], r[16];

	fb = StackPopVector(ip->stack);
	fa = StackPopVector(ip->stack);
	ft = StackPopVector(ip->stack);

	readMatrix(ip, &fb, b);
	readMatrix(ip, &fa, a);

	mulMatrices(b, a, r);

	for (fungeCell x = 0; x < 4; ++x)
		for (fungeCell y = 0; y < 4; ++y) {
			floatint u;
			u.f = r[y*4 + x];
			FungeSpaceSetOff(u.i, VectorCreateRef(ft.x+x, ft.y+y), &ip->storageOffset);
		}
}

/// Z - Scale a vector
static void Finger3DSPscale(instructionPointer * ip)
{
	double a[3];
	double n;

	PopVec(ip, a);
	n = PopFloat(ip);

	a[0] *= n;
	a[1] *= n;
	a[2] *= n;

	PushVec(ip, a);
}

bool Finger3DSPload(instructionPointer * ip)
{
	ManagerAddOpcode(3DSP,  'A', add)
	ManagerAddOpcode(3DSP,  'B', sub)
	ManagerAddOpcode(3DSP,  'C', cross)
	ManagerAddOpcode(3DSP,  'D', dot)
	ManagerAddOpcode(3DSP,  'L', length)
	ManagerAddOpcode(3DSP,  'M', mul)
	ManagerAddOpcode(3DSP,  'N', normalise)
	ManagerAddOpcode(3DSP,  'P', matrixCopy)
	ManagerAddOpcode(3DSP,  'R', matrixRotate)
	ManagerAddOpcode(3DSP,  'S', matrixScale)
	ManagerAddOpcode(3DSP,  'T', matrixTranslate)
	ManagerAddOpcode(3DSP,  'U', duplicate)
	ManagerAddOpcode(3DSP,  'V', map)
	ManagerAddOpcode(3DSP,  'X', transform)
	ManagerAddOpcode(3DSP,  'Y', matrixMul)
	ManagerAddOpcode(3DSP,  'Z', scale)
	return true;
}

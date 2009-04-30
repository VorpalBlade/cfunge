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

#include "3DSP.h"

#if !defined(CFUN_NO_FLOATS)
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
static inline float pop_float(instructionPointer * restrict ip)
{
	floatint u;
	u.i = (int32_t)stack_pop(ip->stack);
	return u.f;
}

FUNGE_ATTR_FAST
static inline void push_float(instructionPointer * restrict ip, float f)
{
	floatint u;
	u.f = f;
	stack_push(ip->stack, u.i);
}


FUNGE_ATTR_FAST
static inline void pop_vec_float(instructionPointer * restrict ip, float vec[restrict 3])
{
	vec[2] = pop_float(ip);
	vec[1] = pop_float(ip);
	vec[0] = pop_float(ip);
}

FUNGE_ATTR_FAST
static inline void push_vec_float(instructionPointer * restrict ip, const float vec[restrict 3])
{
	push_float(ip, vec[0]);
	push_float(ip, vec[1]);
	push_float(ip, vec[2]);
}

FUNGE_ATTR_FAST
static inline void pop_vec(instructionPointer * restrict ip, double vec[restrict 3])
{
	vec[2] = pop_float(ip);
	vec[1] = pop_float(ip);
	vec[0] = pop_float(ip);
}

FUNGE_ATTR_FAST
static inline void push_vec(instructionPointer * restrict ip, const double vec[restrict 3])
{
	push_float(ip, (float)vec[0]);
	push_float(ip, (float)vec[1]);
	push_float(ip, (float)vec[2]);
}

FUNGE_ATTR_FAST
static inline double vector_length(const double vec[restrict 3])
{
	return sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
}

FUNGE_ATTR_FAST
static inline void writeMatrix(const instructionPointer * restrict ip,
                               const funge_vector * restrict fV, const double m[restrict 16])
{
	const funge_cell basex = fV->x + ip->storageOffset.x;
	const funge_cell basey = fV->y + ip->storageOffset.y;
	for (funge_cell y = 0; y < 4; ++y) {
		for (funge_cell x = 0; x < 4; ++x) {
			floatint u;
			u.f = (float)m[4*y + x];
			fungespace_set(u.i, vector_create_ref(basex + x, basey + y));
		}
	}
}

FUNGE_ATTR_FAST
static inline void readMatrix(const instructionPointer * restrict ip,
                              const funge_vector * restrict fV, double m[restrict 16])
{
	const funge_cell basex = fV->x + ip->storageOffset.x;
	const funge_cell basey = fV->y + ip->storageOffset.y;
	for (funge_cell y = 0; y < 4; ++y) {
		for (funge_cell x = 0; x < 4; ++x) {
			floatint u;
			u.i = (int32_t)fungespace_get(vector_create_ref(basex + x, basey + y));
			m[y*4 + x] = u.f;
		}
	}
}

FUNGE_ATTR_FAST
static inline void mulMatrixVector(const double m[restrict 16], const double v[restrict 4], double r[restrict 4])
{
	for (size_t i = 0; i < 4; ++i) {
		double n = 0;
		for (size_t k = 0; k < 4; ++k)
			n += m[i*4 + k] * v[k];
		r[i] = n;
	}
}

FUNGE_ATTR_FAST
static inline void mulMatrices(const double a[restrict 16], const double b[restrict 16], double r[restrict 16])
{
	for (size_t y = 0; y < 4; ++y) {
		for (size_t x = 0; x < 4; ++x) {
			double n = 0;
			for (size_t k = 0; k < 4; ++k)
				n += a[y*4 + k] * b[k*4 + x];
			r[y*4 + x] = n;
		}
	}
}

/****************************
 * Fingerprint instructions *
 ****************************/

/// A - Add two 3d vectors
static void finger_3DSP_add(instructionPointer * ip)
{
	double a[3], b[3];
	pop_vec(ip, b);
	pop_vec(ip, a);

	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
	push_vec(ip, a);
}

/// B - Subtract two 3d vectors
static void finger_3DSP_sub(instructionPointer * ip)
{
	double a[3], b[3];
	pop_vec(ip, b);
	pop_vec(ip, a);

	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
	push_vec(ip, a);
}

/// C - Cross porduct of two vectors
static void finger_3DSP_cross(instructionPointer * ip)
{
	double a[3], b[3], c[3];
	pop_vec(ip, b);
	pop_vec(ip, a);

	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];

	push_vec(ip, c);
}

/// D - Dot product of two vector
static void finger_3DSP_dot(instructionPointer * ip)
{
	double a[3], b[3];
	pop_vec(ip, b);
	pop_vec(ip, a);

	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];

	push_float(ip, (float)(a[0] + a[1] + a[2]));
}

/// L - Length of vector
static void finger_3DSP_length(instructionPointer * ip)
{
	double a[3];
	pop_vec(ip, a);
	push_float(ip, (float)vector_length(a));
}

/// M - Multiply two 3d vectors
static void finger_3DSP_mul(instructionPointer * ip)
{
	double a[3], b[3];
	pop_vec(ip, b);
	pop_vec(ip, a);

	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];

	push_vec(ip, a);
}

/// N - Normalize vector (sets length to 1)
static void finger_3DSP_normalise(instructionPointer * ip)
{
	double a[3];
	double len;

	pop_vec(ip, a);
	len = vector_length(a);

	a[0] /= len;
	a[1] /= len;
	a[2] /= len;

	push_vec(ip, a);
}

/// P - Copy a matrix
static void finger_3DSP_matrix_copy(instructionPointer * ip)
{
	funge_vector fs, ft;

	fs = stack_pop_vector(ip->stack);
	ft = stack_pop_vector(ip->stack);
	// Add in storage offset
	fs.x += ip->storageOffset.x;
	fs.y += ip->storageOffset.y;
	ft.x += ip->storageOffset.x;
	ft.y += ip->storageOffset.y;


	for (funge_cell y = 0; y < 4; ++y)
		for (funge_cell x = 0; x < 4; ++x) {
			fungespace_set(fungespace_get(vector_create_ref(fs.x + x, fs.y + y)),
			               vector_create_ref(ft.x + x, ft.y + y));
		}
}

/// R - Generate a rotation matrix
static void finger_3DSP_matrix_rotate(instructionPointer * ip)
{
	double s, c;
	double angle = pop_float(ip);
	funge_cell axis = stack_pop(ip->stack);
	funge_vector fV = stack_pop_vector(ip->stack);

	if (!(axis >= 1 && axis <= 3)) {
		ip_reverse(ip);
		return;
	}

	angle *= M_PI / 180;

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
static void finger_3DSP_matrix_scale(instructionPointer * ip)
{
	double v[3];
	funge_vector fV;
	pop_vec(ip, v);
	fV = stack_pop_vector(ip->stack);
	{
		double matrix[16] = {v[0],   0,   0,   0
		                    ,   0,v[1],   0,   0
		                    ,   0,   0,v[2],   0
		                    ,   0,   0,   0,   1};
		writeMatrix(ip, &fV, matrix);
	}
}

/// T - Generate a translation matrix
static void finger_3DSP_matrix_translate(instructionPointer * ip)
{
	double v[3];
	funge_vector fV;
	pop_vec(ip, v);
	fV = stack_pop_vector(ip->stack);

	{
		double matrix[16] = {   1,   0,   0,   v[0]
		                    ,   0,   1,   0,   v[1]
		                    ,   0,   0,   1,   v[2]
		                    ,   0,   0,   0,   1};
		writeMatrix(ip, &fV, matrix);
	}
}

/// U - Duplicate vector on top of stack
static void finger_3DSP_duplicate(instructionPointer * ip)
{
	float a[3];
	pop_vec_float(ip, a);
	push_vec_float(ip, a);
	push_vec_float(ip, a);
}

/// V - Map 3d point to 2d view
static void finger_3DSP_map(instructionPointer * ip)
{
	double v[3];

	pop_vec(ip, v);
	// Use fpclassify() to avoid:
	// "warning: comparing floating point with == or != is unsafe".
#ifdef FP_ZERO
	if (fpclassify(v[2]) != FP_ZERO)
#else
	if (v[2] != 0)
#endif
	{
		v[0] /= v[2];
		v[1] /= v[2];
		v[2] /= v[2];
	}
	push_float(ip, (float)v[0]);
	push_float(ip, (float)v[1]);
}

/// X - Transform a vector using transformation matrix
static void finger_3DSP_transform(instructionPointer * ip)
{
	funge_vector fm;
	double v[4];
	double m[16];
	double r[4];

	fm = stack_pop_vector(ip->stack);

	pop_vec(ip, v);
	v[3] = 1;

	readMatrix(ip, &fm, m);

	mulMatrixVector(m, v, r);
	push_vec(ip, r);
}

/// Y - Multiply two matrices
static void finger_3DSP_matrix_mul(instructionPointer * ip)
{
	funge_vector ft, fa, fb;
	double a[16], b[16], r[16];

	fb = stack_pop_vector(ip->stack);
	fa = stack_pop_vector(ip->stack);
	ft = stack_pop_vector(ip->stack);

	readMatrix(ip, &fb, b);
	readMatrix(ip, &fa, a);

	mulMatrices(b, a, r);

	writeMatrix(ip, &ft, r);
}

/// Z - Scale a vector
static void finger_3DSP_scale(instructionPointer * ip)
{
	double a[3];
	double n;

	pop_vec(ip, a);
	n = pop_float(ip);

	a[0] *= n;
	a[1] *= n;
	a[2] *= n;

	push_vec(ip, a);
}

bool finger_3DSP_load(instructionPointer * ip)
{
	manager_add_opcode(3DSP, 'A', add)
	manager_add_opcode(3DSP, 'B', sub)
	manager_add_opcode(3DSP, 'C', cross)
	manager_add_opcode(3DSP, 'D', dot)
	manager_add_opcode(3DSP, 'L', length)
	manager_add_opcode(3DSP, 'M', mul)
	manager_add_opcode(3DSP, 'N', normalise)
	manager_add_opcode(3DSP, 'P', matrix_copy)
	manager_add_opcode(3DSP, 'R', matrix_rotate)
	manager_add_opcode(3DSP, 'S', matrix_scale)
	manager_add_opcode(3DSP, 'T', matrix_translate)
	manager_add_opcode(3DSP, 'U', duplicate)
	manager_add_opcode(3DSP, 'V', map)
	manager_add_opcode(3DSP, 'X', transform)
	manager_add_opcode(3DSP, 'Y', matrix_mul)
	manager_add_opcode(3DSP, 'Z', scale)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) */

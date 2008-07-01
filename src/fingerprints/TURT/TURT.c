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

#include "TURT.h"
#include "../../stack.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// This fingerprint is basically a translation from D to C of the TURT
// fingerprint of CCBI, but with a lot of bug fixes.

#define DEFAULT_FILENAME "cfunge_TURT.svg"
static const char* filename = NULL;

// fixed point with 5 decimals
// tc meaning "turtle coordinate"
typedef int32_t tc;

// SVGT limits all numbers to -32767.9999 - 32767.9999, not -32768 - 32767
// that limits our width to 32767.9999, hence the min and max values
// we add a PADDING value to get nice viewBoxes for small drawings
#define TURT_PADDING 1
#define TURT_MIN -163839999 + TURT_PADDING
#define TURT_MAX  163839999 - TURT_PADDING

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline int getInt(tc c)
{
	return (c < 0 ? -c : c) / 1000;
}

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline unsigned int getDec(tc c)
{
	return abs(c) % 1000;
}

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline double getDouble(tc c)
{
	return (double)c / 1000;
}

typedef struct Point {
	tc x, y;
} Point;

typedef struct Turtle {
	Point p;
	Point min;
	Point max;
	double heading, sin, cos;
	uint32_t colour;
	bool penDown:1;
	bool movedWithoutDraw:1;
} Turtle;

typedef struct Dot {
	Point p;
	uint32_t colour;
} Dot;

typedef struct Path {
	struct Path* next;
	Dot d;
	bool penDown:1;
} Path;

typedef struct Drawing {
	size_t       dots_size;
	Dot*         dots;
	Path*        pathBeg;
	Path*        path;
	uint32_t bgColour;
} Drawing;

static Turtle turt;
static Drawing pic;

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline Path* CreatePath(Point a, bool b, uint32_t c)
{
	Path* p = malloc(sizeof(Path));
	if (!p)
		return NULL;
	p->next = NULL;
	p->d.p = a;
	p->d.colour = c;
	p->penDown = b;
	return p;
}

FUNGE_ATTR_FAST
static inline void addPath(Point pt, bool penDown, uint32_t colour)
{
	Path* p = CreatePath(pt, penDown, colour);

	if (pic.pathBeg == NULL)
		pic.pathBeg = p;
	else
		pic.path->next = p;
	pic.path = p;
}

FUNGE_ATTR_FAST
static inline void normalize(void)
{
	while (turt.heading > 2*M_PI)
		turt.heading -= 2 * M_PI;
	while (turt.heading < 0)
		turt.heading += 2 * M_PI;
	turt.sin = sin(turt.heading);
	turt.cos = cos(turt.heading);
}

FUNGE_ATTR_FAST
static inline void newDraw(void)
{
	if (turt.p.x < turt.min.x)
		turt.min.x = turt.p.x;
	if (turt.p.x > turt.max.x)
		turt.max.x = turt.p.x;

	if (turt.p.y < turt.min.y)
		turt.min.y = turt.p.y;
	if (turt.p.y > turt.max.y)
		turt.max.y = turt.p.y;
}

FUNGE_ATTR_FAST
static inline void move(tc distance)
{
	// have to check for under-/overflow...

	tc dx, dy;
	int64_t nx, ny;
	double tmp;

	tmp = round(turt.cos * distance);
	dx = (tc)tmp;
	tmp = round(turt.sin * distance);
	dy = (tc)tmp;

	nx = turt.p.x + dx;
	if (nx > TURT_MAX)
		nx = TURT_MAX;
	else if (nx < TURT_MIN)
		nx = TURT_MIN;
	turt.p.x = nx;

	ny = turt.p.y + dy;
	if (ny > TURT_MAX)
		ny = TURT_MAX;
	else if (ny < TURT_MIN)
		ny = TURT_MIN;
	turt.p.y = ny;

	// a -> ... -> z is equivalent to a -> z if not drawing
	if (turt.penDown || (pic.path && pic.path->penDown)) {
		addPath(turt.p, turt.penDown, turt.colour);
		newDraw();
		turt.movedWithoutDraw = false;
	} else
		turt.movedWithoutDraw = true;
}


// helpers...
FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline double toRad(FUNGEDATATYPE c)
{
	return (M_PI / 180.0) * c;
}
FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline FUNGEDATATYPE toDeg(double r)
{
	double d = round((180.0 / M_PI) * r);
	return (FUNGEDATATYPE)d;
}

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline uint32_t toRGB(FUNGEDATATYPE c)
{
	return (uint32_t)(c & ((1 << 24) - 1));
}

FUNGE_ATTR_FAST
static inline void addPoint(void)
{

	for (size_t i = 0; i < pic.dots_size; i++) {
		Dot* dot = &pic.dots[i];
		if ((dot->p.x == turt.p.x) && (dot->p.y == turt.p.y)) {
			if (dot->colour != turt.colour)
				dot->colour = turt.colour;
			return;
		}
	}

	pic.dots_size++;
	pic.dots = realloc(pic.dots, pic.dots_size * sizeof(Dot));
	pic.dots[pic.dots_size - 1].p = turt.p;
	pic.dots[pic.dots_size - 1].colour = turt.colour;
	newDraw();
}

// if we've moved to a location with the pen up, and the pen is now down, it
// may be that we'll move to another location with the pen down so there's no
// need to add a point unless the pen is lifted up or we need to look at the drawing
FUNGE_ATTR_FAST
static inline void tryAddPoint(void)
{
	if (turt.movedWithoutDraw && turt.penDown)
		addPoint();
}

// Uses a static buffer, not reentrant!
FUNGE_ATTR_FAST
static inline const char* toCSSColour(uint32_t c)
{
	static char s[8];
	size_t i;
	i = snprintf(s, sizeof(s), "#%02x%02x%02x", c >> 16 & 0xff, c >> 8 & 0xff, c & 0xff);
	return s;
}

FUNGE_ATTR_FAST
static inline void freeResources(void)
{
	Path* p = pic.pathBeg;
	if (p) {
		Path* next;
		while (p) {
			next = p->next;
			free(p);
			p = next;
		}
		free(p);
	}
	pic.pathBeg = NULL;
	pic.path = NULL;
	free(pic.dots);
	pic.dots = NULL;
	pic.dots_size = 0;
}

FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void GenerateViewBox(FILE * f) {
	double minx, miny, w, h;
	minx = getDouble(turt.min.x - TURT_PADDING);
	miny = getDouble(turt.min.y - TURT_PADDING);
	w = getDouble(turt.max.x - turt.min.x + TURT_PADDING);
	h = getDouble(turt.max.y - turt.min.y + TURT_PADDING);
	fputs("viewBox=\"", f);
	fprintf(f, "%f %f %f %f", minx, miny, w, h);
	fputs("\"", f);
}

/*
 * The actual fingerprint functions
 */

// A - Query Position (x, y coordinates)
static void FingerTURTqueryHeading(instructionPointer * ip)
{
	StackPush(ip->stack, toDeg(turt.heading));
}

// B - Back (distance in pixles)
static void FingerTURTback(instructionPointer * ip)
{
	move(-StackPop(ip->stack));
}

// C - Pen Colour (24-bit RGB)
static void FingerTURTpenColour(instructionPointer * ip)
{
	tryAddPoint();
	turt.colour = toRGB(StackPop(ip->stack));
}

// D - Show Display (0 = no, 1 = yes)
static void FingerTURTshowDisplay(instructionPointer * ip)
{
	// What display? We don't have one as far as I know?
	FUNGEDATATYPE a;
	a = StackPop(ip->stack);
	switch (a) {
		case 0:  break;
		case 1:  tryAddPoint(); break;
		default: ipReverse(ip); break;
	}
}

// E - Query Pen (0 = up, 1 = down)
static void FingerTURTqueryPen(instructionPointer * ip)
{
	StackPush(ip->stack, turt.penDown);
}

// F - Forward (distance in pixels)
static void FingerTURTforward(instructionPointer * ip)
{
	move(StackPop(ip->stack));
}

// H - Set Heading (angle in degrees, relative to 0deg, east)
static void FingerTURTsetHeading(instructionPointer * ip)
{
	turt.heading = toRad(StackPop(ip->stack)); normalize();
}

#define PATH_START_STRING "\n<path style=\"fill:none;fill-opacity:0.75;fill-rule:evenodd;stroke:%s;stroke-width:0.00005px;stroke-linecap:round;stroke-linejoin:miter;stroke-opacity:1\" d=\""
#define PATH_END_STRING   "\n\"/>"
// SVG suggests a maximum line length of 255
#define NODES_PER_LINE 10


static inline void PrintPoint(FILE * f, char prefix, tc x, tc y) {
	fprintf(f, "%c%s%d.%.4u,%s%d.%.4u ", prefix,
	        (x < 0) ? "-" : "", getInt(x), getDec(x),
	        (y < 0) ? "-" : "", getInt(y), getDec(y)
	       );
}

// I - Print current Drawing (if possible)
static void FingerTURTprintDrawing(instructionPointer * ip)
{
	FILE * file;
	Path* p;

	tryAddPoint();

	file = fopen(filename, "wb");
	if (!file) {
		ipReverse(ip);
		return;
	}

	// if we need more size (unlikely), baseProfile="full" below
	// static assert (MAX - MIN <= 32767_9999);

	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", file);
	fputs("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n", file);
	fputs("<svg version=\"1.1\" baseProfile=\"tiny\" xmlns=\"http://www.w3.org/2000/svg\" ", file);
	GenerateViewBox(file);
	fputs(">", file);

	p = pic.pathBeg;

	if (p) {
		Path* prev;
		uint8_t i;
		bool openpath = true;
		fprintf(file, PATH_START_STRING, toCSSColour(p->d.colour));

		fputs("\n\t", file);
		// need to move to the start if we draw immediately
		if (p->penDown)
			fputs("M0,0 ", file);

		// SVG suggests a maximum line length of 255
		i = 0;

		prev = p;
		while (p) {
			if (p->penDown) {
				if (!openpath) {
					fprintf(file, PATH_START_STRING "\n\t", toCSSColour(p->d.colour));
					PrintPoint(file, 'M', prev->d.p.x, prev->d.p.y);
				}
				PrintPoint(file, 'L', p->d.p.x, p->d.p.y);
				// start a new path if the colour changes
				if (p->next && (p->d.colour != p->next->d.colour)) {
					fputs(PATH_END_STRING, file);
					openpath = false;

				}
			// if the last one doesn't draw anything, skip it, it's useless
			} else if (p != pic.path) {
				PrintPoint(file, 'M', p->d.p.x, p->d.p.y);
			}

			if (++i >= NODES_PER_LINE) {
				fputs("\n\t", file);
				i = 0;
			}
			prev = p;
			p = p->next;
		}
		fputs(PATH_END_STRING, file);
	}

	for (size_t i = 0; i < pic.dots_size; i++) {
		Dot* dot = &pic.dots[i];
		fprintf(file, "\n<circle cx=\"%s%d.%.4u\" cy=\"%s%d.%.4u\" r=\"0.00005\" fill=\"%s\" />",
		        (dot->p.x < 0) ? "-" : "", getInt(dot->p.x), getDec(dot->p.x),
		        (dot->p.y < 0) ? "-" : "", getInt(dot->p.y), getDec(dot->p.y),
		        toCSSColour(dot->colour)
		       );
	}

	fputs("\n</svg>", file);

	fclose(file);
}

// L - Turn Left (angle in degrees)
static void FingerTURTturnLeft(instructionPointer * ip)
{
	turt.heading -= toRad(StackPop(ip->stack)); normalize();
}

// N - Clear Paper with Colour (24-bit RGB)
static void FingerTURTclearPaper(instructionPointer * ip)
{
	pic.bgColour = toRGB(StackPop(ip->stack));
	freeResources();
}

// P - Pen Position (0 = up, 1 = down)
static void FingerTURTpenPosition(instructionPointer * ip)
{
	FUNGEDATATYPE a;
	a = StackPop(ip->stack);
	switch (a) {
		case 0:
			tryAddPoint();
			turt.penDown = false;
			break;
		case 1:  turt.penDown = true; break;
		default: ipReverse(ip); break;
	}
}

// Q - Query Position (x, y coordinates)
static void FingerTURTqueryPosition(instructionPointer * ip)
{
	StackPush(ip->stack, turt.p.x);
	StackPush(ip->stack, turt.p.y);
}

// R - Turn Right (angle in degrees)
static void FingerTURTturnRight(instructionPointer * ip)
{
	turt.heading += toRad(StackPop(ip->stack)); normalize();
}

// T - Teleport (x, y coords relative to origin; 00T = home)
static void FingerTURTteleport(instructionPointer * ip)
{
	tryAddPoint();

	turt.p.y = StackPop(ip->stack);
	turt.p.x = StackPop(ip->stack);

	turt.movedWithoutDraw = true;
}

// U - Query Bounds (two pairs of x, y coordinates)
static void FingerTURTqueryBounds(instructionPointer * ip)
{
	StackPush(ip->stack, TURT_MIN);
	StackPush(ip->stack, TURT_MIN);
	StackPush(ip->stack, TURT_MAX);
	StackPush(ip->stack, TURT_MAX);
}

static bool initialized = false;

static void inititalize(void)
{
	if (!initialized) {
		initialized = true;
		filename = DEFAULT_FILENAME;
		turt.movedWithoutDraw = true;
		// To set up turt.sin/turt.cos.
		normalize();
	}
}

bool FingerTURTload(instructionPointer * ip)
{
	inititalize();
#ifndef NDEBUG
	atexit(&freeResources);
#endif
	ManagerAddOpcode(TURT,  'A', queryHeading)
	ManagerAddOpcode(TURT,  'B', back)
	ManagerAddOpcode(TURT,  'C', penColour)
	ManagerAddOpcode(TURT,  'D', showDisplay)
	ManagerAddOpcode(TURT,  'E', queryPen)
	ManagerAddOpcode(TURT,  'F', forward)
	ManagerAddOpcode(TURT,  'H', setHeading)
	ManagerAddOpcode(TURT,  'I', printDrawing)
	ManagerAddOpcode(TURT,  'L', turnLeft)
	ManagerAddOpcode(TURT,  'N', clearPaper)
	ManagerAddOpcode(TURT,  'P', penPosition)
	ManagerAddOpcode(TURT,  'Q', queryPosition)
	ManagerAddOpcode(TURT,  'R', turnRight)
	ManagerAddOpcode(TURT,  'T', teleport)
	ManagerAddOpcode(TURT,  'U', queryBounds)
	return true;
}

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

#include "TURT.h"

#if !defined(CFUN_NO_FLOATS) && !defined(CFUN_NO_TURT)
#include "../../stack.h"
#include "../../../lib/genx/genx.h"
#include "../../../lib/stringbuffer/stringbuffer.h"

#include <math.h>    /* cosl, roundl, sinl */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>   /* fputs, snprintf */
#include <stdlib.h>  /* abs */

// M_PIl is a GNU extension. This value should be enough
// for 128-bit long double.
#ifndef M_PIl
#  define M_PIl 3.1415926535897932384626433832795029L
#endif

#ifndef HAVE_cosl
#  define cosl cos
#endif
#ifndef HAVE_roundl
#  define roundl round
#endif
#ifndef HAVE_sinl
#  define sinl sin
#endif

// This fingerprint is basically a translation from D to C of the TURT
// fingerprint of CCBI, but with a lot of bug fixes.

#define DEFAULT_FILENAME "cfunge_TURT.svg"
static const char* filename = NULL;

/// fixed point with 5 decimals
/// tc meaning "turtle coordinate"
typedef int32_t tc;

// SVGT limits all numbers to -32767.9999 - 32767.9999, not -32768 - 32767
// that limits our width to 32767.9999, hence the min and max values
// we add a PADDING value to get nice viewBoxes for small drawings
#define TURT_PADDING 1
#define TURT_MIN -163839999 + TURT_PADDING
#define TURT_MAX  163839999 - TURT_PADDING

#define FIXEDFMT   "%s%d.%04u"
#define PRINTFIXED(n) ((n) < 0) ? "-" : "", getInt(n), getDec(n)

// For use with genx:
static constUtf8 gns = NULL;

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline int getInt(tc c)
{
	return (c < 0 ? -c : c) / 10000;
}

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline unsigned int getDec(tc c)
{
	return (unsigned int)(abs(c) % 10000);
}

typedef struct Point {
	tc x, y;
} Point;

typedef struct Turtle {
	/// We use radians here.
	long double heading, sin, cos;
	Point p;
	Point min;
	Point max;
	uint32_t colour;
	bool penDown;
	bool movedWithoutDraw;
} Turtle;

typedef struct Dot {
	Point p;
	uint32_t colour;
} Dot;

typedef struct Path {
	struct Path* next;
	Dot d;
	bool penDown;
} Path;

typedef struct Drawing {
	size_t       dots_size;
	Dot*         dots;
	Path*        pathBeg;
	Path*        path;
	uint32_t     bgColour;
	/**
	 * When possible prefer transparency.
	 * This can be done because TURT specs doesn't say what default is.
	 */
	bool         bgSet;
} Drawing;

static Turtle turt;
static Drawing pic;

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
static inline Path* create_path(Point a, bool b, uint32_t c)
{
	Path* p = cf_malloc(sizeof(Path));
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
	// FIXME: Handle OOM
	Path* p = create_path(pt, penDown, colour);

	if (pic.pathBeg == NULL)
		pic.pathBeg = p;
	else
		pic.path->next = p;
	pic.path = p;
}

FUNGE_ATTR_FAST
static inline void normalise(void)
{
	while (turt.heading > 2*M_PIl)
		turt.heading -= 2 * M_PIl;
	while (turt.heading < 0)
		turt.heading += 2 * M_PIl;
	turt.sin = sinl(turt.heading);
	turt.cos = cosl(turt.heading);
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
	tc dx, dy;
	int64_t nx, ny;
	long double tmp;

	if (turt.penDown && turt.movedWithoutDraw)
		addPath(turt.p, false, 0);

	tmp = roundl(turt.cos * distance);
	dx = (tc)tmp;
	tmp = roundl(turt.sin * distance);
	dy = (tc)tmp;

	// have to check for under-/overflow...
	nx = turt.p.x + dx;
	if (nx > TURT_MAX)
		nx = TURT_MAX;
	else if (nx < TURT_MIN)
		nx = TURT_MIN;
	turt.p.x = (tc)nx;

	ny = turt.p.y + dy;
	if (ny > TURT_MAX)
		ny = TURT_MAX;
	else if (ny < TURT_MIN)
		ny = TURT_MIN;
	turt.p.y = (tc)ny;

	// a -> ... -> z is equivalent to a -> z if not drawing
	if (turt.penDown) {
		addPath(turt.p, turt.penDown, turt.colour);
		newDraw();
		turt.movedWithoutDraw = false;
	} else
		turt.movedWithoutDraw = true;
}


// helpers...
FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline long double toRad(funge_cell c)
{
	return (M_PI / 180.0) * c;
}
FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline funge_cell toDeg(long double r)
{
	long double d = roundl((180.0 / M_PI) * r);
	return (funge_cell)d;
}

FUNGE_ATTR_FAST FUNGE_ATTR_CONST FUNGE_ATTR_WARN_UNUSED
static inline uint32_t toRGB(funge_cell c)
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
	// FIXME: Handle OOM.
	pic.dots = cf_realloc(pic.dots, pic.dots_size * sizeof(Dot));
	pic.dots[pic.dots_size - 1].p = turt.p;
	pic.dots[pic.dots_size - 1].colour = turt.colour;
	newDraw();
}

/// If we've moved to a location with the pen up, and the pen is now down, it
/// may be that we'll move to another location with the pen down so there's no
/// need to add a point unless the pen is lifted up or we need to look at the drawing
FUNGE_ATTR_FAST
static inline void tryAddPoint(void)
{
	if (turt.movedWithoutDraw && turt.penDown)
		addPoint();
}

/// Generates a CSS colour
/// Uses a static buffer, not reentrant!
FUNGE_ATTR_FAST
static inline const char* toCSSColour(uint32_t c)
{
	static char s[8];
	snprintf(s, sizeof(s), "#%02x%02x%02x", c >> 16 & 0xff, c >> 8 & 0xff, c & 0xff);
	return s;
}

static inline void freeResources(void)
{
	Path* p = pic.pathBeg;
	if (p) {
		while (p) {
			Path* next = p->next;
			cf_free(p);
			p = next;
		}
		cf_free(p);
	}
	pic.pathBeg = NULL;
	pic.path = NULL;
	cf_free(pic.dots);
	pic.dots = NULL;
	pic.dots_size = 0;
}

/// Print the "header" of the SVG file
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void print_header(genxWriter gw)
{
	tc minx, miny, w, h;

	minx = turt.min.x - TURT_PADDING;
	miny = turt.min.y - TURT_PADDING;
	w = turt.max.x - turt.min.x + 2 * TURT_PADDING;
	h = turt.max.y - turt.min.y + 2 * TURT_PADDING;

	genxStartElementLiteral(gw, gns, (constUtf8)"svg");
	genxAddAttributeLiteral(gw, gns, (constUtf8)"version", (constUtf8)"1.1");
	genxAddAttributeLiteral(gw, gns, (constUtf8)"baseProfile", (constUtf8)"full");
	genxAddAttributeLiteral(gw, gns, (constUtf8)"xmlns", (constUtf8)"http://www.w3.org/2000/svg");
	{
		char sviewbox[256];
		snprintf(sviewbox, sizeof(sviewbox), FIXEDFMT " " FIXEDFMT " " FIXEDFMT " " FIXEDFMT,
		         PRINTFIXED(minx), PRINTFIXED(miny), PRINTFIXED(w), PRINTFIXED(h));
		genxAddAttributeLiteral(gw, gns, (constUtf8)"viewBox", (constUtf8)sviewbox);
	}
	genxStartElementLiteral(gw, gns, (constUtf8)"defs");
	genxStartElementLiteral(gw, gns, (constUtf8)"style");
	genxAddAttributeLiteral(gw, gns, (constUtf8)"type", (constUtf8)"text/css");
	genxAddText(gw,
	            (constUtf8)"path{fill:none;stroke-width:0.00005px;stroke-linecap:round;stroke-linejoin:miter}");
	genxEndElement(gw);
	genxEndElement(gw);
	// This check is because we want transparency if possible.
	if (pic.bgSet) {
		char sminx[64];
		char sminy[64];
		char sw[64];
		char sh[64];
		char scss[sizeof("fill:#112233;stroke:none")];
		snprintf(sminx, sizeof(sminx), FIXEDFMT, PRINTFIXED(minx));
		snprintf(sminy, sizeof(sminy), FIXEDFMT, PRINTFIXED(miny));
		snprintf(sw, sizeof(sw), FIXEDFMT, PRINTFIXED(w));
		snprintf(sh, sizeof(sh), FIXEDFMT, PRINTFIXED(h));
		snprintf(scss, sizeof(scss), "fill:%s;stroke:none", toCSSColour(pic.bgColour));
		genxStartElementLiteral(gw, gns, (constUtf8)"rect");
		genxAddAttributeLiteral(gw, gns, (constUtf8)"style", (constUtf8)scss);
		genxAddAttributeLiteral(gw, gns, (constUtf8)"x", (constUtf8)sminx);
		genxAddAttributeLiteral(gw, gns, (constUtf8)"y", (constUtf8)sminy);
		genxAddAttributeLiteral(gw, gns, (constUtf8)"width", (constUtf8)sw);
		genxAddAttributeLiteral(gw, gns, (constUtf8)"height", (constUtf8)sh);
		genxEndElement(gw);
	}
}

/// Used to print a point in a path element.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void print_point(StringBuffer * sb, char prefix, tc x, tc y)
{
	stringbuffer_append_printf(sb, "%c" FIXEDFMT "," FIXEDFMT " ", prefix, PRINTFIXED(x), PRINTFIXED(y));
}


/*
 * The actual fingerprint functions
 */

/// A - Query Position (x, y coordinates)
static void finger_TURT_query_heading(instructionPointer * ip)
{
	stack_push(ip->stack, toDeg(turt.heading));
}

/// B - Back (distance in pixels)
static void finger_TURT_back(instructionPointer * ip)
{
	move((tc) - stack_pop(ip->stack));
}

/// C - Pen Colour (24-bit RGB)
static void finger_TURT_pen_colour(instructionPointer * ip)
{
	tryAddPoint();
	turt.colour = toRGB(stack_pop(ip->stack));
}

/// D - Show Display (0 = no, 1 = yes)
static void finger_TURT_show_display(instructionPointer * ip)
{
	// What display? We don't have one as far as I know?
	funge_cell a;
	a = stack_pop(ip->stack);
	switch (a) {
		case 0:  break;
		case 1:  tryAddPoint(); break;
		default: ip_reverse(ip); break;
	}
}

/// E - Query Pen (0 = up, 1 = down)
static void finger_TURT_query_pen(instructionPointer * ip)
{
	stack_push(ip->stack, turt.penDown);
}

/// F - Forward (distance in pixels)
static void finger_TURT_forward(instructionPointer * ip)
{
	move((tc)stack_pop(ip->stack));
}

/// H - Set Heading (angle in degrees, relative to 0deg, east)
static void finger_TURT_set_heading(instructionPointer * ip)
{
	turt.heading = toRad(stack_pop(ip->stack)); normalise();
}


static inline bool generate_path(genxWriter gw, uint32_t colour, const char * path,
                                 genxElement g_path, genxAttribute g_style, genxAttribute g_d)
{
	char sstyle[sizeof("stroke:#112233")];
	snprintf(sstyle, sizeof(sstyle), "stroke:%s", toCSSColour(colour));

	genxStartElement(g_path);
	genxAddAttribute(g_style, (constUtf8)sstyle);
	genxAddAttribute(g_d, (constUtf8)path);
	genxEndElement(gw);
	return true;
}

static inline bool generate_paths(genxWriter gw)
{
	genxElement g_path;
	genxAttribute g_style, g_d;
	genxStatus status;
	Path *p, *prev = NULL;
	StringBuffer * sb;
	char * path_data;
	size_t path_data_length;

	p = pic.pathBeg;
	if (!p)
		return false;

	sb = stringbuffer_new();
	if (!sb)
		return false;
	// Create elements.
	g_path  = genxDeclareElement(gw, NULL, (constUtf8)"path", &status);
	g_style = genxDeclareAttribute(gw, NULL, (constUtf8)"style", &status);
	g_d     = genxDeclareAttribute(gw, NULL, (constUtf8)"d", &status);

	if (p->penDown)
		stringbuffer_append_string(sb, "M0,0 ");

	while (p) {
		// Time to create a new one?
		if (!sb) {
			sb = stringbuffer_new();
			if (!sb)
				return false;
			if (p->penDown)
				print_point(sb, 'M', prev->d.p.x, prev->d.p.y);
		}
		if (p->penDown) {
			print_point(sb, 'L', p->d.p.x, p->d.p.y);
		} else if (p != pic.path) {
			print_point(sb, 'M', p->d.p.x, p->d.p.y);
		}
		if (p->next && (p->d.colour != p->next->d.colour)) {
			path_data = stringbuffer_finish(sb, NULL);
			sb = NULL;
			generate_path(gw, p->d.colour, path_data, g_path, g_style, g_d);
			// TODO: Should we free?
			if (path_data)
				free_nogc(path_data);
			path_data = NULL;
		}
		prev = p;
		p = p->next;
	}
	// Final printout:
	path_data = stringbuffer_finish(sb, &path_data_length);
	if (path_data_length > 0) {
		generate_path(gw, prev->d.colour, path_data, g_path, g_style, g_d);
	}
	if (path_data) free_nogc(path_data);
	return true;
}

static inline bool generate_circle(genxWriter gw, Dot* dot,
                                   genxElement g_circle, genxAttribute g_cx, genxAttribute g_cy,
                                   genxAttribute g_r, genxAttribute g_fill)
{
	char buf[64];
	genxStartElement(g_circle);
	snprintf(buf, sizeof(buf), FIXEDFMT, PRINTFIXED(dot->p.x));
	genxAddAttribute(g_cx, (constUtf8)buf);
	snprintf(buf, sizeof(buf), FIXEDFMT, PRINTFIXED(dot->p.y));
	genxAddAttribute(g_cy, (constUtf8)buf);
	genxAddAttribute(g_r, (constUtf8)"0.000025");
	genxAddAttribute(g_fill, (constUtf8)toCSSColour(dot->colour));
	genxEndElement(gw);
	return true;
}

static inline bool generate_circles(genxWriter gw)
{
	genxStatus status;
	genxElement g_circle;
	genxAttribute g_cx, g_cy, g_r, g_fill;

	// Create elements.
	g_circle = genxDeclareElement(gw, NULL, (constUtf8)"circle", &status);
	g_cx   = genxDeclareAttribute(gw, NULL, (constUtf8)"cx", &status);
	g_cy   = genxDeclareAttribute(gw, NULL, (constUtf8)"cy", &status);
	g_r    = genxDeclareAttribute(gw, NULL, (constUtf8)"r", &status);
	g_fill = genxDeclareAttribute(gw, NULL, (constUtf8)"fill", &status);

	for (size_t i = 0; i < pic.dots_size; i++) {
		if (!generate_circle(gw, &pic.dots[i], g_circle, g_cx, g_cy, g_r, g_fill))
			return false;
	}
	return true;
}

/// I - Print current Drawing (if possible)
static void finger_TURT_print_drawing(instructionPointer * ip)
{
	FILE * file;
	genxWriter gw = NULL;

	tryAddPoint();

	file = fopen(filename, "wb");
	if (!file) {
		goto error;
	}

	gw = genxNew();
	if (!gw) {
		goto error;
	}

	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n", file);
	fputs("<!-- Created with cfunge (http://kuonet.org/~anmaster/cfunge/) -->\n", file);
	fputs("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n", file);

	if (genxStartDocFile(gw, file) != GENX_SUCCESS) {
		goto error;
	}

	print_header(gw);

	generate_paths(gw);
	generate_circles(gw);
	// End <svg>
	genxEndElement(gw);
	if (genxEndDocument(gw) != GENX_SUCCESS) {
		goto error;
	}
	goto exit;

error:
	ip_reverse(ip);
exit:
	if (file)
		fclose(file);
	if (gw)
		genxDispose(gw);
}

/// L - Turn Left (angle in degrees)
static void finger_TURT_turn_left(instructionPointer * ip)
{
	turt.heading -= toRad(stack_pop(ip->stack)); normalise();
}

/// N - Clear Paper with Colour (24-bit RGB)
static void finger_TURT_clear_paper(instructionPointer * ip)
{
	pic.bgColour = toRGB(stack_pop(ip->stack));
	pic.bgSet = true;
	turt.min.x = 0;
	turt.max.x = 0;
	turt.min.y = 0;
	turt.max.y = 0;
	freeResources();
}

/// P - Pen Position (0 = up, 1 = down)
static void finger_TURT_pen_position(instructionPointer * ip)
{
	funge_cell a;
	a = stack_pop(ip->stack);
	switch (a) {
		case 0:
			tryAddPoint();
			turt.penDown = false;
			break;
		case 1:  turt.penDown = true; break;
		default: ip_reverse(ip); break;
	}
}

/// Q - Query Position (x, y coordinates)
static void finger_TURT_query_position(instructionPointer * ip)
{
	stack_push(ip->stack, turt.p.x);
	stack_push(ip->stack, turt.p.y);
}

/// R - Turn Right (angle in degrees)
static void finger_TURT_turn_right(instructionPointer * ip)
{
	turt.heading += toRad(stack_pop(ip->stack)); normalise();
}

/// T - Teleport (x, y coords relative to origin; 00T = home)
static void finger_TURT_teleport(instructionPointer * ip)
{
	tryAddPoint();

	turt.p.y = (tc)stack_pop(ip->stack);
	turt.p.x = (tc)stack_pop(ip->stack);

	turt.movedWithoutDraw = true;
}

/// U - Query Bounds (two pairs of x, y coordinates)
static void finger_TURT_query_bounds(instructionPointer * ip)
{
	stack_push(ip->stack, TURT_MIN);
	stack_push(ip->stack, TURT_MIN);
	stack_push(ip->stack, TURT_MAX);
	stack_push(ip->stack, TURT_MAX);
}

static bool turt_initialised = false;

static void initialise(void)
{
	if (!turt_initialised) {
		turt_initialised = true;
		filename = DEFAULT_FILENAME;
		turt.movedWithoutDraw = true;
		// To set up turt.sin/turt.cos.
		normalise();
	}
}

bool finger_TURT_load(instructionPointer * ip)
{
	initialise();
#ifndef NDEBUG
	atexit(&freeResources);
#endif
	manager_add_opcode(TURT, 'A', query_heading)
	manager_add_opcode(TURT, 'B', back)
	manager_add_opcode(TURT, 'C', pen_colour)
	manager_add_opcode(TURT, 'D', show_display)
	manager_add_opcode(TURT, 'E', query_pen)
	manager_add_opcode(TURT, 'F', forward)
	manager_add_opcode(TURT, 'H', set_heading)
	manager_add_opcode(TURT, 'I', print_drawing)
	manager_add_opcode(TURT, 'L', turn_left)
	manager_add_opcode(TURT, 'N', clear_paper)
	manager_add_opcode(TURT, 'P', pen_position)
	manager_add_opcode(TURT, 'Q', query_position)
	manager_add_opcode(TURT, 'R', turn_right)
	manager_add_opcode(TURT, 'T', teleport)
	manager_add_opcode(TURT, 'U', query_bounds)
	return true;
}
#endif /* !defined(CFUN_NO_FLOATS) && !defined(CFUN_NO_TURT) */

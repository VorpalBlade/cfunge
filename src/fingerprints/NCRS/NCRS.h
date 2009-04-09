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

#ifndef FUNGE_HAD_SRC_FINGERPRINTS_NCRS_H
#define FUNGE_HAD_SRC_FINGERPRINTS_NCRS_H

#include "../../global.h"
#include "../manager.h"

#if defined(HAVE_NCURSES)

bool finger_NCRS_load(instructionPointer * ip);

// Used by TERM to be specific to check coordinate ncurses state changes
#ifdef FUNGE_EXTENDS_NCRS
FUNGE_ATTR_FAST
bool finger_NCRS_need_setupterm(void);
#endif

#endif /* defined(HAVE_NCURSES) */

#endif

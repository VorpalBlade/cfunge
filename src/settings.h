/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
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


// This file contains global runtime settings
#ifndef _HAD_SRC_SETTINGS_H
#define _HAD_SRC_SETTINGS_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>


// Out of order to initialize to 0 :D
// (A few bytes smaller binary that way)
// (The standard one should be 0 always
typedef enum { stdver98 = 0, stdver93, stdver08 } standardVersion;

// What version we should simulate.
// Affects space processing.
extern standardVersion SettingCurrentStandard;

extern uint_fast16_t SettingTraceLevel;
extern bool SettingWarnings;

#endif

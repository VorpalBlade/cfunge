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

/**
 * @file
 * Definition of variables holding settings for current session.
 */

// This file contains global runtime settings
#ifndef _HAD_SRC_SETTINGS_H
#define _HAD_SRC_SETTINGS_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

/// What version of the Funge standard we are executing.
/// @note
/// Out of order to make default initialise to 0.
/// (A few bytes smaller binary that way, The standard one should always be 0)
typedef enum { stdver98 = 0, stdver93 = 1, stdver108 = 2 } standardVersion;

/// What version we should simulate.
/// Affects space processing.
extern standardVersion SettingCurrentStandard;

/// Level of trace output
extern uint_fast16_t SettingTraceLevel;
/// Should we enable warnings
extern bool SettingWarnings;

/// Should fingerprints be enabled
extern bool SettingDisableFingerprints;

/// Sandbox, prevent bad programs affecting system.
/// If true:
/// - Any file, filesystem or network IO is forbidden.
/// - The program can not access network using network fingerprints.
/// - The environment variables it can see are restricted.
///
/// Summary:
/// - In core opcodes: =, o and i are forbidden and certain environment
///   variables are hidden.
/// - In fingerprints: Non-safe fingerprints are not loaded.
extern bool SettingSandbox;

#endif

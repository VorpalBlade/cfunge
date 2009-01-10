/*
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

/**
 * @file
 * safe_env.c is a perfect hash generated with gperf used to check
 * if a certain environment variable should be visible in sandbox mode.
 */

#ifndef FUNGE_HAD_SRC_INSTRUCTIONS_SAFE_ENV_H
#define FUNGE_HAD_SRC_INSTRUCTIONS_SAFE_ENV_H

#include "../global.h"
#include <stdbool.h>

/**
 * Check if an environment variable is safe for sandbox.
 * @param envvar The string to check, this must be a full line like:
 * foo=bar.
 * @return True if safe, otherwise false.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL FUNGE_ATTR_WARN_UNUSED
bool check_env_is_safe(const char *envvar);

#endif

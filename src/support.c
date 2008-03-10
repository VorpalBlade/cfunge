/*
 * cfunge08 - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "global.h"
#include "support.h"

#include <string.h>

char * cf_strndup(char const *string, size_t n)
{
	if (!string || !*string)
		return NULL;
	// Keep gcc happy with variable decls
	{
		size_t len = strnlen(string, n);
		char *newstr = cf_malloc_noptr(len + 1);

		if (newstr == NULL)
			return NULL;

		newstr[len] = '\0';
		return memcpy(newstr, string, len);
	}
}

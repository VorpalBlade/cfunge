# - Check if a given include file exists and error out if not.
# CFUNGE_REQUIRE_INCLUDE(_files _name)
#
#  _files - List of includes file to check for.
#  _name  - Used for cache variable name.
#
# The same variables as for CheckIncludeFile affect this macro.
#
# Will set CFUNGE_CHECKMULTIINCLUDES_<name> (as CheckIncludeFiles out variable).
#

# cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
# Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at the proxy's option) any later version. Arvid Norlander is a
# proxy who can decide which future versions of the GNU General Public
# License can be used.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include(CheckIncludeFiles)

macro(CFUNGE_REQUIRE_MULTIPLE_INCLUDES _files _name)
	CHECK_INCLUDE_FILES("${_files}" "CFUNGE_CHECKMULTIINCLUDES_${_name}")
	if (NOT "CFUNGE_CHECKMULTIINCLUDES_${_name}")
		message(FATAL_ERROR
		        "Your system seems to be missing one or several of the headers \"${_files}\" which are required by cfunge.")
	endif (NOT "CFUNGE_CHECKMULTIINCLUDES_${_name}")
endmacro(CFUNGE_REQUIRE_MULTIPLE_INCLUDES)

# - Check if a given include file exists and error out if not.
# CFUNGE_REQUIRE_INCLUDE(_name)
#
#  _name - Include file to check for.
#
# The same variables as for CheckIncludeFile affect this macro.
#
# Will set CFUNGE_CHECKINCLUDE_<name> (as CheckIncludeFile out variable).
#

# cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
# Copyright (C) 2008 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

include(CheckIncludeFile)

macro(CFUNGE_REQUIRE_INCLUDE _name)
	CHECK_INCLUDE_FILE(${_name} "CFUNGE_CHECKINCLUDE_${_name}")
	if (NOT CFUNGE_CHECKINCLUDE_${_name})
		message(FATAL_ERROR
		        "Your system seems to be missing the header \"${_name}\" which is required by cfunge.")
	endif ()
endmacro(CFUNGE_REQUIRE_INCLUDE)

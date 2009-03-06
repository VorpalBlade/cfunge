# - Check if the Linker supports a specific flag and if yes add it.
# CFUNGE_CHECK_LINKER_FLAG(_target _name _flag)
#
#  _target - Target to add to.
#  _name   - Name to use for cache variable.
#  _flag   - Flag to check for
#
# The same variables as for CheckCSourceCompiles affect this macro.
# Exception: SAFE_CMAKE_REQUIRED_FLAGS is overridden.
#
# Will set CFUNGE_CHECKLINKERFLAG_$<name> (as CheckCCompilerFlag out variable).
# Will use MACRO_ADD_LINK_FLAGS() to add the <flag> if supported.
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

include(CheckCSourceCompiles)
include(MacroAddLinkFlags)

macro(CFUNGE_CHECK_LINKER_FLAG _target _name _flag)
	SET(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
	SET(CMAKE_REQUIRED_FLAGS "${_flag}")
	CHECK_C_SOURCE_COMPILES("int main() { return 0;}" CFUNGE_CHECKLINKERFLAG_${_name})
	if (CFUNGE_CHECKLINKERFLAG_${_name})
		MACRO_ADD_LINK_FLAGS(${_target} ${_flag})
	endif (CFUNGE_CHECKLINKERFLAG_${_name})
	SET(CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
endmacro(CFUNGE_CHECK_LINKER_FLAG)

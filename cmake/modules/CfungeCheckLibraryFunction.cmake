# - Check if a given library exists and have a specific function.
# CFUNGE_CHECK_LIBRARY_FUNCTION(_target _library _function _location)
#
#  _target   - Build target to add the library for to link.
#  _library  - Library to check for and then check for function in.
#  _function - Function name to check for.
#  _location - Location where the library should be found.
#
# The same variables as for CheckLibraryExists affect this macro.
#
# Will set CFUNGE_CHECKLIBFUNC_<library>_<function> (as CheckLibraryExists out variable).
# Will #define HAVE_<function> if found.
# Will set the variable CFUNGE_HAVE_<library>_<function> if found.
# Will add the library to target_link_libraries() if found.
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

include(CheckLibraryExists)

macro(CFUNGE_CHECK_LIBRARY_FUNCTION _target _library _function _location)
	CHECK_LIBRARY_EXISTS(${_library} ${_function} ${_location} CFUNGE_CHECKLIBFUNC_${_library}_${_function})
	if (CFUNGE_CHECKLIBFUNC_${_library}_${_function})
		set(CFUNGE_HAVE_${_library}_${_function} true PARENT_SCOPE)
		add_definitions(-DHAVE_${_function})
		target_link_libraries(${_target} ${_library})
	endif (CFUNGE_CHECKLIBFUNC_${_library}_${_function})
endmacro(CFUNGE_CHECK_LIBRARY_FUNCTION)

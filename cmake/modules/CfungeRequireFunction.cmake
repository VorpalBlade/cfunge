# - Check if a given function exists and error out if not.
# CFUNGE_REQUIRE_FUNCTION(_name)
#
#  _name - Function name to check for.
#
# The same variables as for CheckFunctionExists affect this macro.
#
# Will set CFUNGE_CHECKFUNC_<name> (as CheckFunctionExists out variable).
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


include(CheckFunctionExists)

macro(CFUNGE_REQUIRE_FUNCTION _name)
	CHECK_FUNCTION_EXISTS(${_name} CFUNGE_CHECKFUNC_${_name})
	if (NOT CFUNGE_CHECKFUNC_${_name})
		message(FATAL_ERROR
		        "Your system seems to be missing the function \"${_name}\" which is required by cfunge.")
	endif (NOT CFUNGE_CHECKFUNC_${_name})
endmacro(CFUNGE_REQUIRE_FUNCTION)

# - Check if the C compiler supports a specific flag and if yes add it.
# CFUNGE_CHECK_CFLAG(_name _flag)
#
#  _name - Name to use for cache variable.
#  _flag - Flag to check for
#
# The same variables as for CheckCCompilerFlag affect this macro.
#
# Will set CFUNGE_CHECKFLAG_<name> (as CheckCCompilerFlag out variable).
# Will use add_definitions() to add the <flag> if supported.
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

include(CheckCCompilerFlag)

macro(CFUNGE_CHECK_CFLAG _name _flag)
	CHECK_C_COMPILER_FLAG(${_flag} CFUNGE_CHECKFLAG_${_name})
	if (CFUNGE_CHECKFLAG_${_name})
		add_definitions(${_flag})
	endif (CFUNGE_CHECKFLAG_${_name})
endmacro(CFUNGE_CHECK_CFLAG)

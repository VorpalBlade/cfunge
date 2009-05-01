# - Set build info defines for use in -v output.
# CFUNGE_SET_BUILD_INFO_FLAGS()
#
# The same variables as for CheckCCompilerFlag affect this macro.
#
# NOTE: Set CMAKE_BUILD_TYPE_UPPERCASE first!
#
# Will use add_definitions() to add the the relevant defines.
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

macro(CFUNGE_SET_BUILD_INFO_FLAGS)
	add_definitions("-DCFUN_TARGET_PLATFORM=\"${CMAKE_SYSTEM_PROCESSOR}\"")
	add_definitions("-DCFUN_TARGET_OS=\"${CMAKE_SYSTEM_NAME}\"")
	add_definitions("-DCFUN_COMPILED_ON=\"${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} (${CMAKE_SYSTEM_PROCESSOR})\"")
	add_definitions("-DCFUN_COMPILER=\"${CMAKE_C_COMPILER}\"")
	add_definitions("-DCFUN_BUILD_TYPE=\"${CMAKE_BUILD_TYPE}\"")
	add_definitions("-DCFUN_USER_CFLAGS=\"${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPERCASE}}\"")
	add_definitions("-DCFUN_USER_LDFLAGS=\"${CMAKE_EXE_LINKER_FLAGS}\"")
endmacro(CFUNGE_SET_BUILD_INFO_FLAGS)

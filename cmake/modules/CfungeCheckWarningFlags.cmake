# - Check if the C compiler supports a set of warning flags.
# CFUNGE_CHECK_WARNING_FLAGS()
#
# Will use add_definitions() to add the flags if they are supported.
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

include(CfungeCheckCflag)

macro(CFUNGE_CHECK_WARNING_FLAGS)
	if (CMAKE_COMPILER_IS_GNUCC)
		# Not all GCC versions support all of these flags.
		CFUNGE_CHECK_CFLAG(Wall                         -Wall)
		CFUNGE_CHECK_CFLAG(Wextra                       -Wextra)
		CFUNGE_CHECK_CFLAG(pedantic                     -pedantic)
	
		CFUNGE_CHECK_CFLAG(Wwrite-strings               -Wwrite-strings)
	
		CFUNGE_CHECK_CFLAG(Wcast-align                  -Wcast-align)
		CFUNGE_CHECK_CFLAG(Wcast-qual                   -Wcast-qual)
		CFUNGE_CHECK_CFLAG(Wbad-function-cast           -Wbad-function-cast)
	
		CFUNGE_CHECK_CFLAG(Wstrict-prototypes           -Wstrict-prototypes)
		CFUNGE_CHECK_CFLAG(Wmissing-prototypes          -Wmissing-prototypes)
		CFUNGE_CHECK_CFLAG(Wmissing-declarations        -Wmissing-declarations)
		CFUNGE_CHECK_CFLAG(Wold-style-definition        -Wold-style-definition)
	
		CFUNGE_CHECK_CFLAG(Wredundant-decls             -Wredundant-decls)
		CFUNGE_CHECK_CFLAG(Wnested-externs              -Wnested-externs)
		CFUNGE_CHECK_CFLAG(Wdeclaration-after-statement -Wdeclaration-after-statement)
	
		CFUNGE_CHECK_CFLAG(Wshadow                      -Wshadow)
		CFUNGE_CHECK_CFLAG(Wundef                       -Wundef)
		CFUNGE_CHECK_CFLAG(Wpacked                      -Wpacked)
		CFUNGE_CHECK_CFLAG(Wfloat-equal                 -Wfloat-equal)
	
		CFUNGE_CHECK_CFLAG(Wstrict-aliasing_2           -Wstrict-aliasing=2)
		#CFUNGE_CHECK_CFLAG(Wstrict-overflow_5          -Wstrict-overflow=5)
		CFUNGE_CHECK_CFLAG(Wformat_2                    -Wformat=2)
	
		CFUNGE_CHECK_CFLAG(Wmissing-noreturn            -Wmissing-noreturn)
		CFUNGE_CHECK_CFLAG(Wmissing-format-attribute    -Wmissing-format-attribute)
	
		CFUNGE_CHECK_CFLAG(Winit-self                   -Winit-self)
		# GCC 4.1(?) or later
		CFUNGE_CHECK_CFLAG(Wunsafe-loop-optimizations   -Wunsafe-loop-optimizations)
		CFUNGE_CHECK_CFLAG(Wmissing-include-dirs        -Wmissing-include-dirs)
	
		CFUNGE_CHECK_CFLAG(Wunused-parameter            -Wunused-parameter)
		# These are part of -Wall in GCC 4.3, but not in some older versions.
		CFUNGE_CHECK_CFLAG(Wunused-function             -Wunused-function)
		CFUNGE_CHECK_CFLAG(Wunused-label                -Wunused-label)
		CFUNGE_CHECK_CFLAG(Wunused-value                -Wunused-value)
		CFUNGE_CHECK_CFLAG(Wunused-variable             -Wunused-variable)
		CFUNGE_CHECK_CFLAG(Wimplicit                    -Wimplicit)
		CFUNGE_CHECK_CFLAG(Wparentheses                 -Wparentheses)
		# Part of -pedantic in 4.3
		CFUNGE_CHECK_CFLAG(Wpointer-arith               -Wpointer-arith)
	
		# Maintainer flags. Gives lots of false positives.
		# add_definitions(-Wunreachable-code -fno-inline
		#                 -fno-inline-functions-called-once -fno-inline-functions
		#                 -fkeep-inline-functions -Wno-inline)
		# Even more false positives
		#add_definitions(-Wunreachable-code)
		# Only with this meaning in GCC 4.3 (and false positives):
		# -Wconversion
	endif (CMAKE_COMPILER_IS_GNUCC)
endmacro(CFUNGE_CHECK_WARNING_FLAGS)

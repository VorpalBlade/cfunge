# - Check if the C compiler supports a set of warning flags.
# CFUNGE_CHECK_WARNING_FLAGS()
#
# Will use add_definitions() to add the flags if they are supported.
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

include(CfungeCheckCflag)
include(CMakePushCheckState)

macro(CFUNGE_CHECK_WARNING_FLAGS)
	if (CMAKE_COMPILER_IS_GNUCC)
		# Use -Werror so we error out on "warning: unknown flag" and such.
		# However since the test file cmake uses uses int main() instead of
		# int main(void) we need to test two of the flags after all other ones
		set(CMAKE_REQUIRED_FLAGS "-Werror")
		# Not all GCC versions support all of these flags.
		CFUNGE_CHECK_CFLAG(Wall                         -Wall)
		CFUNGE_CHECK_CFLAG(Wextra                       -Wextra)
		CFUNGE_CHECK_CFLAG(pedantic                     -pedantic)

		CFUNGE_CHECK_CFLAG(Wwrite_strings               -Wwrite-strings)

		CFUNGE_CHECK_CFLAG(Wcast_align                  -Wcast-align)
		CFUNGE_CHECK_CFLAG(Wcast_qual                   -Wcast-qual)
		CFUNGE_CHECK_CFLAG(Wbad_function_cast           -Wbad-function-cast)

		CFUNGE_CHECK_CFLAG(Wmissing_prototypes          -Wmissing-prototypes)
		CFUNGE_CHECK_CFLAG(Wmissing_declarations        -Wmissing-declarations)

		CFUNGE_CHECK_CFLAG(Wredundant_decls             -Wredundant-decls)
		CFUNGE_CHECK_CFLAG(Wnested_externs              -Wnested-externs)
		CFUNGE_CHECK_CFLAG(Wdeclaration_after_statement -Wdeclaration-after-statement)

		CFUNGE_CHECK_CFLAG(Wshadow                      -Wshadow)
		CFUNGE_CHECK_CFLAG(Wundef                       -Wundef)
		CFUNGE_CHECK_CFLAG(Wpacked                      -Wpacked)
		CFUNGE_CHECK_CFLAG(Wfloat_equal                 -Wfloat-equal)

		CFUNGE_CHECK_CFLAG(Wstrict_aliasing_2           -Wstrict-aliasing=2)
		CFUNGE_CHECK_CFLAG(Wformat_2                    -Wformat=2)

		CFUNGE_CHECK_CFLAG(Wmissing_noreturn            -Wmissing-noreturn)
		CFUNGE_CHECK_CFLAG(Wmissing_format_attribute    -Wmissing-format-attribute)
		# Newer names
		CFUNGE_CHECK_CFLAG(Wsuggest_attribute_const     -Wsuggest-attribute=const)
		CFUNGE_CHECK_CFLAG(Wsuggest_attribute_noreturn  -Wsuggest-attribute=noreturn)
		CFUNGE_CHECK_CFLAG(Wsuggest_attribute_format    -Wsuggest-attribute=format)

		CFUNGE_CHECK_CFLAG(Winit_self                   -Winit-self)
		# GCC 4.1(?) or later
		CFUNGE_CHECK_CFLAG(Wunsafe_loop_optimizations   -Wunsafe-loop-optimizations)
		CFUNGE_CHECK_CFLAG(Wmissing_include_dirs        -Wmissing-include-dirs)

		CFUNGE_CHECK_CFLAG(Wunused_parameter            -Wunused-parameter)
		# These are part of -Wall in GCC 4.3, but not in some older versions.
		CFUNGE_CHECK_CFLAG(Wunused_function             -Wunused-function)
		CFUNGE_CHECK_CFLAG(Wunused_label                -Wunused-label)
		CFUNGE_CHECK_CFLAG(Wunused_value                -Wunused-value)
		CFUNGE_CHECK_CFLAG(Wunused_variable             -Wunused-variable)
		CFUNGE_CHECK_CFLAG(Wimplicit                    -Wimplicit)
		CFUNGE_CHECK_CFLAG(Wparentheses                 -Wparentheses)
		# Part of -pedantic in 4.3
		CFUNGE_CHECK_CFLAG(Wpointer_arith               -Wpointer-arith)

		# Various
		CFUNGE_CHECK_CFLAG(Wswitch_enum                 -Wswitch-enum)
		CFUNGE_CHECK_CFLAG(Wunused_but_set_parameter    -Wunused-but-set-parameter)
		CFUNGE_CHECK_CFLAG(Wunused_but_set_variable     -Wunused-but-set-variable)
		CFUNGE_CHECK_CFLAG(Wunused_local_typedefs       -Wunused-local-typedefs)
		CFUNGE_CHECK_CFLAG(Wunused                      -Wunused)
		CFUNGE_CHECK_CFLAG(Wuninitialized               -Wuninitialized)
		CFUNGE_CHECK_CFLAG(Wmaybe_uninitialized         -Wmaybe-uninitialized)
		CFUNGE_CHECK_CFLAG(Wtrampolines                 -Wtrampolines)
		CFUNGE_CHECK_CFLAG(Wjump_misses_init            -Wjump-misses-init)
		CFUNGE_CHECK_CFLAG(Wlogical_op                  -Wlogical-op)

		# Maintainer flags. Gives lots of false positives.
		#CFUNGE_CHECK_CFLAG(Wsuggest_attribute_pure      -Wsuggest-attribute=pure)
		# add_definitions(-Wunreachable-code -fno-inline
		#                 -fno-inline-functions-called-once -fno-inline-functions
		#                 -fkeep-inline-functions -Wno-inline)
		# Even more false positives
		#add_definitions(-Wunreachable-code)
		#
		#CFUNGE_CHECK_CFLAG(Wswitch_default              -Wswitch-default)
		#
		# Only with this meaning in GCC 4.3+ (and false positives):
		#CFUNGE_CHECK_CFLAG(Wconversion                  -Wconversion)
		#CFUNGE_CHECK_CFLAG(Wno_sign_conversion          -Wno-sign-conversion)
		cmake_push_check_state()
		unset(CMAKE_REQUIRED_FLAGS)
		# Test those where GCC gives warning on the test file cmake uses.
		CFUNGE_CHECK_CFLAG(Wstrict_prototypes           -Wstrict-prototypes)
		CFUNGE_CHECK_CFLAG(Wold_style_definition        -Wold-style-definition)
		#CFUNGE_CHECK_CFLAG(Wstrict-overflow_5           -Wstrict-overflow=5)
		cmake_pop_check_state()
	endif (CMAKE_COMPILER_IS_GNUCC)
endmacro(CFUNGE_CHECK_WARNING_FLAGS)

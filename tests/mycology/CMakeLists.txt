# cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
# Copyright (C) 2017 Arvid Norlander <code AT vorpal DOT se>
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

# If the mycology sub module is checked out, run it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/mycology.b98")
	add_test(
		NAME mycology
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mycology_runner.sh
		        ${CMAKE_CURRENT_SOURCE_DIR}/../test_runner.py
		        $<TARGET_FILE:cfunge>
		        ${CMAKE_CURRENT_SOURCE_DIR})
else()
	message(WARNING "Mycology sub module missing, can not run mycology test.")
endif()

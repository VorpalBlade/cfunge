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

function(cfunge_test test_name)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_dir)
	add_test(
		NAME ${test_name}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${test_name}_dir
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../test_runner.py $<TARGET_FILE:cfunge> ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}_dir)
endfunction()

cfunge_test(bool-test.b98)
cfunge_test(bounds.b98)
cfunge_test(concurrent-issues.b98)
cfunge_test(dirf-errors.b98)
cfunge_test(file-errors.b98)
cfunge_test(frth-test.b98)
cfunge_test(io-errors.b98)
cfunge_test(iterate-exit.b98)
cfunge_test(iterate-fetchchar.b98)
cfunge_test(iterate-iterate.b109)
cfunge_test(iterate-iterate2.b109)
cfunge_test(iterate-jump.b109)
cfunge_test(iterate-space.b109)
cfunge_test(iterate-zero.b98)
cfunge_test(multi-file.b98)
cfunge_test(perl.b98)
cfunge_test(refc-force-resize.b98)
cfunge_test(refc-invalid-deref.b98)
cfunge_test(s-nowrap.b98)
cfunge_test(sigfpe.b98)
cfunge_test(split-in-iterate.b98)
cfunge_test(strn-A.b98)
cfunge_test(strn-F.b98)
cfunge_test(strn-G.b98)
cfunge_test(subr-test.b98)
cfunge_test(sysexec.b98)
cfunge_test(sysinfo-pick.b98)
cfunge_test(test-formfeed.b98)
cfunge_test(toys-errors.b98)
cfunge_test(turt.b98)
cfunge_test(turt2.b98)
cfunge_test(wrap.b98)

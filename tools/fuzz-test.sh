#!/bin/bash
# -*- coding: utf-8 -*-
###########################################################################
#                                                                         #
#  cfunge - a conformant Befunge93/98/08 interpreter in C                 #
#  Copyright (C) 2008  Arvid Norlander                                    #
#                                                                         #
#  This program is free software: you can redistribute it and/or modify   #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation, either version 3 of the License, or      #
#  (at the proxy's option) any later version. Arvid Norlander is a        #
#  proxy who can decide which future versions of the GNU General Public   #
#  License can be used.                                                   #
#                                                                         #
#  This program is distributed in the hope that it will be useful,        #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          #
#  GNU General Public License for more details.                           #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.  #
#                                                                         #
###########################################################################
#
# This script is used for simple fuzz testing, to use it:
# 1) uncomment the line:
#      #define FUZZ_TESTING
#    in global.h
# 2) Enable LEAK_MODE in cmake, or valgrind will fail.
# 4) make
# 5) Run script from top source directory
#    To test a specific fingerprint, run script with that fingerprint as
#    the parameter. Otherwise no fingerprints are tested.
# Note that this script has only been tried on Gentoo Linux 2.6.24 x86_64!
# I got no idea if it works elsewhere.
#


die() {
	echo "$1" >&2
	exit 1
}

checkerror() {
	# Most likely Ctrl-C, or q instruction
	if [[ $1 -eq 130 ]]; then
		echo " * Exit code was $1, ctrl c or q"
		return
	# ulimit or q
	elif [[ $1 -eq 137 ]]; then
		echo " * Exit code was $1, ulimit or q"
		return
	# SIGALARM or q
	elif [[ $1 -eq 142 ]]; then
		echo " * Exit code was $1, alarm"
		return
	# Ok, definitly
	elif [[ $1 -eq 0 ]]; then
		echo " * Exit code was $1, ok"
		return
	# abort(), or maybe just "*** glibc detected ***" ones
	elif [[ $1 -eq 134 ]]; then
		die  " * Exit code was 134, probably abort!"
	# SIGSEGV or q
	elif [[ $1 -eq 139 ]]; then
		die  " * Exit code was 139, probably segfault!"
	# Unknown, but in not likely bad range.
	elif [[ $1 -lt 127 ]]; then
		echo " * Exit code was $1, probably ok"
	else
		die  " * Exit code was $1, probably issues there!"
		return
	fi
}

# List of additional fingerprint instructions to test.
FPRINTINSTRS=""
FPRINT=""

# First parameter is fingerprint name
createfingerprint() {
	echo -n "\"${1:3:1}${1:2:1}${1:1:1}${1:0:1}\"4( "
}

if [[ $1 ]]; then
	echo "Will test fingerprint ${1}."
	FPRINT=$1
	FPRINTINSTRS=$(grep Finger${1}load src/fingerprints/manager.c | grep -Eo '"[A-Z]+"' | tr -d '"')
fi

# This does not test fingerprints loaded randomly for the simple reason that it is very unlikely any will load.
# Therefore it does instead provide a way to select a fingerprint to test as first parameter.
while true; do
	# Clean file
	> fuzz.tmp
	# Should we load a fingerprint?
	if [[ $FPRINT ]]; then
		createfingerprint "$FPRINT" >> fuzz.tmp
	fi
	echo " * Generating random program"
	cat /dev/urandom | tr -Cd -- '-[:lower:][:digit:]\n\\/ ;",.+*[]{}^<>@`_|?:%$#!'\'"${FPRINTINSTRS}" | tr -d 'mhlior' | head -n 100 >> fuzz.tmp

	echo " * Running free standing"
	(./cfunge -S fuzz.tmp); checkerror "$?"

	echo " * Running under valgrind"
	(valgrind --leak-check=no ./cfunge -S fuzz.tmp) 2> valgnd.output; checkerror "$?"
	grep -Fq "ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 5 from 1)" valgnd.output || die "Valgrind issues!"
done

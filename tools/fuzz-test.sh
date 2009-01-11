#!/usr/bin/env bash
# -*- coding: utf-8 -*-
###########################################################################
#                                                                         #
#  cfunge - A standard-conforming Befunge93/98/109 interpreter in C.      #
#  Copyright (C) 2008-2009  Arvid Norlander                               #
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
# 2) Disable USE_GC in cmake, or valgrind will fail.
# 4) make
# 5) Run script from top source directory
#    To test a specific fingerprint, run script with that fingerprint as
#    the parameter. Otherwise no fingerprints are tested (just core).
# Note that this script has only been tried on Gentoo Linux 2.6.27 x86_64!
# I got no idea if it works elsewhere.
#
# Note that input/output isn't tested (this script is meant to be run
# non-interactively) and sandbox mode is used (for safety). You can modify the
# script to test in non-sandbox mode if you run fuzz testing in a chroot. How
# is left as an exercise for the reader.
#
# Another note: this script doesn't do much sanity checking on command line
# parameters
#

# Check bash version. We need at least 3.1
# Lets not use anything like =~ here because
# that may not work on old bash versions.
if [[ "${BASH_VERSINFO[0]}${BASH_VERSINFO[1]}" -lt 31 ]]; then
	echo "Sorry your bash version is too old!"
	echo "You need at least version 3.1 of bash."
	echo "Please install a newer version:"
	echo " * Either use your distro's packages."
	echo " * Or see http://www.gnu.org/software/bash/"
	exit 2
fi

die() {
	echo "$1" >&2
	exit 1
}

error() {
	die "ERROR: $1"
}

# Return exit code for signal name
#  $1 Out variable
#  $2 Signal name.
get_exit_code()
{
	local signum retval
	signum=$(kill -l "$2")
	retval=$(( 128 + signum ))
	printf -v "$1" '%s' "$retval"
}

# Set up exit code info, to be used in checkerror() later.
RET_ABRT=
RET_ALRM=
RET_BUS=
RET_INT=
RET_KILL=
RET_SEGV=

get_exit_code RET_ABRT SIGABRT
get_exit_code RET_ALRM SIGALRM
get_exit_code RET_BUS SIGBUS
get_exit_code RET_INT SIGINT
get_exit_code RET_KILL SIGKILL
get_exit_code RET_SEGV SIGSEGV

# Check return code and decide if we should end the script or continue.
checkerror() {
	# Ok
	if [[ $1 -eq 0 ]]; then
		echo " * Exit code was $1, ok"
		return
	# Most likely Ctrl-C
	elif [[ $1 -eq $RET_INT ]]; then
		die " * Exit code was $1 (SIGINT), most likely ctrl-c"
	# abort(), or maybe just "*** glibc detected ***" ones
	elif [[ $1 -eq $RET_BUS ]]; then
		die " * Exit code was $1 (SIGBUS). This is very bad."
	elif [[ $1 -eq $RET_ABRT ]]; then
		die " * Exit code was $1 (SIGABRT). This is usually bad."
	# Ok: ulimit
	elif [[ $1 -eq $RET_KILL ]]; then
		echo -e "\a * Exit code was $1 (SIGKILL): Hopefully due to ulimits. It"
		echo -e '\a   could also be caused by something else. Will continue after'
		echo -e '\a   waiting for 5 seconds to allow you to abort script if you'
		echo -e '\a   see anything indicating it is not due to ulimits.'
		sleep 5
		return
	# SIGSEGV or q
	elif [[ $1 -eq $RET_SEGV ]]; then
		die  " * Exit code was $1 (SIGSEGV). This is very bad."
	# Ok: SIGALRM
	elif [[ $1 -eq $RET_ALRM ]]; then
		echo " * Exit code was $1 (SIGALRM). This is OK."
		return
	# Unknown
	else
		die  " * Exit code was $1 (unknown), probably issues there!"
	fi
}

if [[ ! -d src/fingerprints ]]; then
	error "Run from top source directory please."
fi
if [[ ! -f ./cfunge ]]; then
	error "There must be a copy of the binary in the top source directory."
fi
if ./cfunge -f | grep -q 'This binary uses Boehm GC'; then
	error "This script doesn't work if cfunge was built against Bohem-GC. Please disable USE_GC in cmake and recompile."
fi
if ! ./cfunge -f | grep -q 'This is a fuzz testing build and thus not standard-conforming.'; then
	error "cfunge was built without fuzz testing support. Please see instructions at the top of this script for how to fix."
fi

HAS_VALGRIND=
if hash valgrind >/dev/null 2>&1; then
	HAS_VALGRIND=1
else
	echo -e '\aWarning: You lack valgrind, if possible please install it.'
	echo -e '\aFor now, will continue without valgrind.'
	sleep 2
fi

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
	# TODO: This should use *.spec parsing really... Parsing fingerprints.h is
	# legacy code.
	FPRINTINSTRS=$(grep finger_${1}_load src/fingerprints/fingerprints.h | grep -Eo '"[A-Z]+"' | tr -d '"')
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
	# We skip ? because that make things harder to debug, we also skip i, o and = for security reasons.
	cat /dev/urandom | tr -Cd -- '-[:lower:][:digit:]\n\\/ ;",.+*[]{}^<>@`_|:%$#!'\'"${FPRINTINSTRS}" | tr -d 'mhlior' | head -n 100 >> fuzz.tmp

	echo " * Running free standing"
	(./cfunge -S fuzz.tmp); checkerror "$?"

	if [[ $HAS_VALGRIND ]]; then
		echo " * Running under valgrind"
		(valgrind --leak-check=no ./cfunge -S fuzz.tmp) 2> valgnd.output; checkerror "$?"
		grep -Eq "ERROR SUMMARY: 0 errors from 0 contexts \(suppressed: [0-9]+ from 1\)" valgnd.output || die "Valgrind detected issues!"
	else
		echo " * Skipping run under valgrind due to valgrind not being found"
	fi

	echo
done

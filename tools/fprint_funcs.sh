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

# This script contains utility functions for the fingerprint scripts. It is
# not meant for stand alone use.

# Clean up locale settings to prevent strange bugs:
unset LANG LC_ALL LC_COLLATE LC_CTYPE LC_NUMERIC
export LANG=C
export LC_ALL=C

# Print error message to stderr and exit. Each parameter is printed on it's own
# line prefixed by "ERROR: ".
die() {
	local emsg
	for emsg in "$@"; do
		echo "ERROR: $emsg" >&2
		echo -ne '\a' >&2
	done
	exit 1
}

warn() {
	local emsg
	for emsg in "$@"; do
		echo "WARNING: $emsg" >&2
		echo -ne '\a' >&2
	done
	echo -ne '\a' >&2
	sleep 5
}

progress() {
	echo " * ${1}..."
}
progresslvl2() {
	echo "   * ${1}..."
}

status() {
	echo "   ${1}"
}
statuslvl2() {
	echo "     ${1}"
}

# Char to decimal (ASCII value)
# $1 Name of output variable
# $2 Char to convert.
ord() {
	printf -v "$1" '%d' "'$2"
}

# $1 is fingerprint name
# Returns if ok, otherwise it dies.
checkfprint() {
	local FPRINT="$1"
	if [[ $FPRINT =~ ^[A-Z0-9]{4}$ ]]; then
		status "Fingerprint name $FPRINT ok style."
	elif [[ $FPRINT =~ ^[^\ /\\]{4}$ ]]; then
		status "Fingerprint name $FPRINT probably ok (but not common style)."
		status "Make sure each char is in the ASCII range 0-254."
		status "Note that alphanumeric (upper case only) fingerprint names are strongly preferred."
	else
		die "Not valid format for fingerprint name."
	fi
}

# Parse a spec file. Will make use of FD 4.
# $1 Fingerprint name.
#    There must be a file named ${1}.spec in the current directory
#    The caller should make sure it exists. The caller should also have
#    called checkfprint to validate it.
# This will set/change these globals:
#   Strings:
#     fp_URL              Fingerprint URL
#     fp_F109_URI         URI for Funge-109
#     fp_CONDITION        #if condition for compilation.
#     fp_SAFE             Safe for sandbox mode (true/false)
#     fp_OPCODES          Defined opcodes.
#     fp_DESCRIPTION      Description
#   Arrays:
#     fp_ALIASES          An array with aliases for the fingerprint.
#                         (text format, not hex)
#     fp_OPCODE_NAMES     Opcode names (index: opcode's ASCII value).
#     fp_OPCODE_DESC      Opcode descriptions (index: opcode's ASCII value).
#
# On return IFS is unset.
# This function calls die in case of errors.
parse_spec() {
	local FPRINT="$1"
	# Variables
	fp_URL=""
	fp_F109_URI="NULL"
	fp_CONDITION=""
	fp_SAFE=""
	fp_OPCODES=""
	fp_DESCRIPTION=""
	fp_ALIASES=()
	fp_OPCODE_NAMES=()
	fp_OPCODE_DESC=()

	if [[ ! -f "${FPRINT}.spec" ]]; then
		die "${FPRINT}.spec not found (or not a normal file)."
	fi

	progresslvl2 "Opening spec file"
	IFS=$'\n'
	local line type data instr name desc number
	# First line is %fingerprint-spec 1.[234]
	# (1.2 is still supported).
	exec 4<"${FPRINT}.spec" || die "Couldn't open spec file for reading on FD 4."
	statuslvl2 "Success."
	progresslvl2 "Parsing spec file"
	read -ru 4 line
	if ! [[ "$line" =~ ^%fingerprint-spec\ 1\.[234]$ ]]; then
		die "Either the spec file is not a fingerprint spec, or it is not a supported version (1.2, 1.3 and 1.4 are currently supported)."
	fi

	# 0: pre-"begin instrs"
	# 1: "begin-instrs"
	local parsestate=0

	while read -ru 4 line; do
		if [[ "$line" =~ ^# ]]; then
			continue
		fi
		if [[ $parsestate == 0 ]]; then
			IFS=':' read -rd $'\n' type data <<< "$line" || true
			case $type in
				"%fprint")
					if [[ "$FPRINT" != "$data" ]]; then
						die "%fprint and spec file name doesn't match."
					fi
					;;
				"%url")
					fp_URL="$data"
					;;
				"%f108-uri")
					warn "%f108-uri is deprecated. Replace with %f109-uri and update spec format to 1.4."
					fp_F109_URI="\"$data\""
					;;
				"%f109-uri")
					fp_F109_URI="\"$data\""
					;;
				"%condition")
					fp_CONDITION="$data"
					;;
				"%alias")
					fp_ALIASES+=( "$data" )
					;;
				"%desc")
					fp_DESCRIPTION="$data"
					;;
				"%safe")
					fp_SAFE="$data"
					;;
				"%begin-instrs")
					parsestate=1
					;;
				"#"*)
					# A comment, ignore
					;;
				*)
					die "Unknown entry $type found in ${FPRINT}."
					;;
			esac
		else
			if [[ "$line" == "%end" ]]; then
				break
			fi
			# Parse instruction lines.
			IFS=$' \t' read -rd $'\n' instr name desc <<< "$line"

			fp_OPCODES+="$instr"
			ord number "${instr:0:1}"
			fp_OPCODE_NAMES[$number]="$name"
			fp_OPCODE_DESC[$number]="$desc"
		fi
	done

	unset IFS

	statuslvl2 "Done parsing."

	exec 4<&-

	progresslvl2 "Validating the parsed data"

	if [[ "$fp_URL" ]]; then
		statuslvl2 "%url: Good, not empty"
	else
		die "%url is not given or is empty."
	fi

	if [[ "$fp_DESCRIPTION" ]]; then
		statuslvl2 "%desc: Good, not empty"
	else
		die "%desc is not given or is empty."
	fi

	if [[ ( "$fp_SAFE" == "true" ) || ( "$fp_SAFE" == "false" ) ]]; then
		statuslvl2 "%safe: OK"
	else
		die "%safe must be either true or false."
	fi

	if [[ "$fp_OPCODES" =~ ^[A-Z]+$ ]]; then
		# Check that they are sorted.
		local previousnr=0
		for (( i = 0; i < ${#fp_OPCODES}; i++ )); do
			ord number "${fp_OPCODES:$i:1}"
			if [[ $previousnr -ge $number ]]; then
				die "Instructions not sorted or there are duplicates"
			else
				previousnr=$number
			fi
		done
		statuslvl2 "Instructions: OK"
	else
		die "The opcodes are not valid. The must be in the range A-Z"
	fi
	return 0
}

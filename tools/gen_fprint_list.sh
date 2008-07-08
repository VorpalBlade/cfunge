#!/usr/bin/env bash
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

# Generate the fingerprint list

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

set -e

# This must be run from top source directory.

die() {
	echo "ERROR: $1" >&2
	exit 1
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


# Char to decimal
ord() {
	printf -v "$1" '%d' "'$2"
}


if [[ ! -d src/fingerprints ]]; then
	die "Run from top source directory please."
fi

addtolist() {
	echo "$1" >> "fingerprints.h"
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

# This is to allow sorted list even with aliases...
# I hate aliases...
ENTRIES=()
ENTRYL1=()
ENTRYL2=()
ENTRYL3=()

# $1 = fprint name
genfprintinfo() {
	local FPRINT="$1"

	# Variables
	local URL=""
	local F108_URI="NULL"
	local SAFE=""
	local OPCODES=""
	local DESCRIPTION=""
	local MYALIASES=()

	local OPCODES=""

	progress "Processing $FPRINT"

	checkfprint "$FPRINT"

	if [[ ! -e $FPRINT ]]; then
		die "A fingerprint called $FPRINT is not yet generated!"
	fi

	progresslvl2 "Looking for spec file"

	if [[ -f "${FPRINT}.spec" ]]; then
		statuslvl2 "Good, spec file found."
	else
		die "Sorry you need a spec file for the fingerprint. It should be placed at src/fingerprints/${FPRINT}.spec"
	fi


	progresslvl2 "Parsing spec file"
	IFS=$'\n'
	local line type data instr name desc number
	# First line is %fingerprint-spec 1.2
	exec 4<"${FPRINT}.spec"
	read -ru 4 line
	if [[ "$line" != "%fingerprint-spec 1.2" ]]; then
		die "Either the spec file is not a fingerprint spec, or it is not version 1.2 of the format."
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
						die "fprint is spec file doesn't match."
					fi
					;;
				"%url")
					URL="$data"
					;;
				"%f108-uri")
					F108_URI="\"$data\""
					;;
				"%alias")
					MYALIASES+=( "$data" )
					;;
				"%desc")
					DESCRIPTION="$data"
					;;
				"%safe")
					SAFE="$data"
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
			IFS=$'\t' read -rd $'\n' instr name desc <<< "$line"

			OPCODES+="$instr"
		fi
	done

	unset IFS

	statuslvl2 "Done parsing."

	exec 4<&-

	progresslvl2 "Validating the parsed data"

	if [[ "$URL" ]]; then
		statuslvl2 "%url: Good, not empty"
	else
		die "%url is not given or is empty."
	fi

	if [[ "$DESCRIPTION" ]]; then
		statuslvl2 "%desc: Good, not empty"
	else
		die "%desc is not given or is empty."
	fi

	if [[ ( "$SAFE" == "true" ) || ( "$SAFE" == "false" ) ]]; then
		statuslvl2 "%safe: OK"
	else
		die "%safe must be either true or false."
	fi

	if [[ "$OPCODES" =~ ^[A-Z]+$ ]]; then
		# Check that they are sorted.
		local previousnr=0
		for (( i = 0; i < ${#OPCODES}; i++ )); do
			ord number "${OPCODES:$i:1}"
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

	progresslvl2 "Generating data for list"
	local FPRINTHEX='0x'
	for (( i = 0; i < ${#FPRINT}; i++ )); do
		printf -v hex '%x' "'${FPRINT:$i:1}"
		FPRINTHEX+="$hex"
	done
	ENTRIES+=( "$FPRINTHEX" )
	ENTRYL1[$FPRINTHEX]="	// ${FPRINT} - ${DESCRIPTION}"
	ENTRYL2[$FPRINTHEX]="	{ .fprint = ${FPRINTHEX}, .uri = ${F108_URI}, .loader = &Finger${FPRINT}load, .opcodes = \"${OPCODES}\","
	ENTRYL3[$FPRINTHEX]="	  .url = \"${URL}\", .safe = ${SAFE} },"
	statuslvl2 "Done"
	if [[ ${!MYALIASES[*]} ]]; then
		progresslvl2 "Generating aliases..."
		local myalias
		for myalias in "${MYALIASES[@]}"; do
			checkfprint "$myalias"
			local ALIASHEX='0x'
			for (( i = 0; i < ${#myalias}; i++ )); do
				printf -v hex '%x' "'${myalias:$i:1}"
				ALIASHEX+="$hex"
			done
			ENTRIES+=("$ALIASHEX")
			ENTRYL1[$ALIASHEX]="	// ${myalias} - Alias for ${FPRINT} - ${DESCRIPTION}"
			ENTRYL2[$ALIASHEX]="	{ .fprint = ${ALIASHEX}, .uri = ${F108_URI}, .loader = &Finger${myalias}load, .opcodes = \"${OPCODES}\","
			ENTRYL3[$ALIASHEX]="	  .url = \"${URL}\", .safe = ${SAFE} },"
		done
	fi
}

cd "src/fingerprints/" || die "change directory failed"

progress "Finding fingerprint list"
SPECS=( *.spec )

FPRINTS=()

for spec in "${SPECS[@]}"; do
	FPRINTS+=( ${spec%.spec} )
done

progress "Creating list file"
cat > "fingerprints.h" << EOF
/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at the proxy's option) any later version. Arvid Norlander is a
 * proxy who can decide which future versions of the GNU General Public
 * License can be used.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// WARNING THIS FILE IS AUTO GENERATED DON'T CHANGE
// It was generated by tools/gen_fprint_list.sh
// Rerun the tool to update this list
#ifndef MANAGER_INTERNAL
#  error "This is for use in manager.c only"
#endif

EOF


addtolist "#ifndef _HAD_SRC_FINGERPRINTS_FINGERPRINTS_H"
addtolist "#define _HAD_SRC_FINGERPRINTS_FINGERPRINTS_H"

cat >> "fingerprints.h" << EOF

#include "../global.h"

#include <stdbool.h>
#include <stdint.h>

EOF

for fprint in "${FPRINTS[@]}"; do
	addtolist "#include \"${fprint}/${fprint}.h\""
done

cat >> "fingerprints.h" << EOF

typedef struct s_ImplementedFingerprintEntry {
	const FUNGEDATATYPE     fprint;   /**< Fingerprint. */
	const char            * uri;      /**< URI, used for Funge-108. */
	const fingerprintLoader loader;   /**< Loader function pointer. */
	const char            * opcodes;  /**< Sorted string with all implemented opcodes. */
	const char            * url;      /**< URL, used to show links for more info about fingerprints. */
	const bool              safe:1;   /**< If true, this fingerprint is safe in sandbox mode. */
} ImplementedFingerprintEntry;

// Implemented fingerprints
// NOTE: Keep sorted (apart from ending 0 entry).
// Also note that this table is processed by scripts, so keep the .loader and
// .opcodes entries on the same line! As well as in current format.
static const ImplementedFingerprintEntry ImplementedFingerprints[] = {
EOF

for fprint in "${FPRINTS[@]}"; do
	genfprintinfo "$fprint"
done
# I really hate aliases...
SORTEDENTRIES=( $(IFS=$'\n'; echo "${ENTRIES[*]}" | sort -n) )
for entry in "${SORTEDENTRIES[@]}"; do
	addtolist "${ENTRYL1[$entry]}"
	addtolist "${ENTRYL2[$entry]}"
	addtolist "${ENTRYL3[$entry]}"
done


cat >> "fingerprints.h" << EOF
	// Last should be 0
	{ .fprint = 0, .uri = NULL, .loader = NULL, .opcodes = NULL, .url = NULL, .safe = true }
};

#endif
EOF

status "Done"

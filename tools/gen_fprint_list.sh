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

# Generate the fingerprint list
# This must be run from top source directory.

# Error to fail with for old bash.
fail_old_bash() {
	echo "Sorry your bash version is too old!"
	echo "You need at least version 3.2.10 of bash"
	echo "Please install a newer version:"
	echo " * Either use your distro's packages"
	echo " * Or see http://www.gnu.org/software/bash/"
	exit 2
}

# Check bash version. We need at least 3.2.10
# Lets not use anything like =~ here because
# that may not work on old bash versions.
if [[ "${BASH_VERSINFO[0]}${BASH_VERSINFO[1]}" -lt 32 ]]; then
	fail_old_bash
elif [[ "${BASH_VERSINFO[0]}${BASH_VERSINFO[1]}" -eq 32 && "${BASH_VERSINFO[2]}" -lt 10 ]]; then
	fail_old_bash
fi

if [[ ! -d src/fingerprints ]]; then
	echo "ERROR: Run from top source directory please." >&2
	exit 1
fi

set -e

if [[ ! -f tools/fprint_funcs.sh ]]; then
	echo "ERROR: Couldn't find tools/fprint_funcs.sh." >&2
	exit 1
fi
source tools/fprint_funcs.sh
if [[ $? -ne 0 ]]; then
	echo "ERROR: Couldn't load tools/fprint_funcs.sh." >&2
	exit 1
fi

addtolist() {
	echo "$1" >> "fingerprints.h"
}
addtoman() {
	echo "$1" >> "doc/cfunge-man-fingerprints.in"
}

GENERATE_MAN=

# This is used internally to generate a list for man page.
if [[ $# -gt 0 ]]; then
	case $1 in
		"-man")
			GENERATE_MAN=yes
			;;
		*)
			die "Invalid cmd line parameter, normally don't use any."
			;;
	esac
fi

# This is to allow sorted list even with aliases...
# I hate aliases...
ENTRIES=()
if [[ $GENERATE_MAN ]]; then
	MANENTRY1=()
	MANENTRY2=()
else
	ENTRYL1_cond=()
	ENTRYL2=()
	ENTRYL3=()
	ENTRYL4=()
	ENTRYL5_cond=()
fi

# $1 = fprint name
genfprintinfo() {
	local FPRINT="$1"

	# Variables
	local fp_URL fp_SAFE fp_OPCODES fp_DESCRIPTION
	local fp_F109_URI fp_CONDITION
	local fp_ALIASES
	local fp_OPCODE_NAMES fp_OPCODE_DESC

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
	parse_spec "${FPRINT}"

	progresslvl2 "Generating data for list"
	local FPRINTHEX='0x'
	for (( i = 0; i < ${#FPRINT}; i++ )); do
		printf -v hex '%x' "'${FPRINT:$i:1}"
		FPRINTHEX+="$hex"
	done
	ENTRIES+=( "$FPRINTHEX" )

	if [[ $GENERATE_MAN ]]; then
		MANENTRY1[$FPRINTHEX]="${FPRINT}"
		# Fix this if we need to escape anything else.
		MANENTRY2[$FPRINTHEX]="${fp_DESCRIPTION}"
		if [[ "${fp_SAFE}" == "false" ]]; then
			MANENTRY2[$FPRINTHEX]+=" (not available in sandbox mode)"
		fi
	else
		[[ "$fp_CONDITION" ]] && ENTRYL1_cond[$FPRINTHEX]="#if $fp_CONDITION"
		ENTRYL2[$FPRINTHEX]="	// ${FPRINT} - ${fp_DESCRIPTION}"
		ENTRYL3[$FPRINTHEX]="	{ .fprint = ${FPRINTHEX}, .uri = ${fp_F109_URI}, .loader = &finger_${FPRINT}_load, .opcodes = \"${fp_OPCODES}\","
		ENTRYL4[$FPRINTHEX]="	  .url = \"${fp_URL}\", .safe = ${fp_SAFE} },"
		[[ "$fp_CONDITION" ]] && ENTRYL5_cond[$FPRINTHEX]="#endif"
		statuslvl2 "Done"
		if [[ ${!fp_ALIASES[*]} ]]; then
			progresslvl2 "Generating aliases..."
			local myalias
			for myalias in "${fp_ALIASES[@]}"; do
				checkfprint "$myalias"
				local ALIASHEX='0x'
				for (( i = 0; i < ${#myalias}; i++ )); do
					printf -v hex '%x' "'${myalias:$i:1}"
					ALIASHEX+="$hex"
				done
				ENTRIES+=("$ALIASHEX")
				[[ "$fp_CONDITION" ]] && ENTRYL1_cond[$ALIASHEX]="#if $fp_CONDITION"
				ENTRYL2[$ALIASHEX]="	// ${myalias} - Alias for ${FPRINT} - ${fp_DESCRIPTION}"
				ENTRYL3[$ALIASHEX]="	{ .fprint = ${ALIASHEX}, .uri = ${fp_F109_URI}, .loader = &finger_${myalias}_load, .opcodes = \"${fp_OPCODES}\","
				ENTRYL4[$ALIASHEX]="	  .url = \"${fp_URL}\", .safe = ${fp_SAFE} },"
				[[ "$fp_CONDITION" ]] && ENTRYL5_cond[$ALIASHEX]="#endif"
			done
		fi
	fi
}

cd "src/fingerprints/" || die "change directory failed"

progress "Finding fingerprint list"
SPECS=( *.spec )

FPRINTS=()

for spec in "${SPECS[@]}"; do
	FPRINTS+=( ${spec%.spec} )
done

if [[ $GENERATE_MAN ]]; then
	for fprint in "${FPRINTS[@]}"; do
		genfprintinfo "$fprint"
	done
	cd ../.. || die "change directory to top src dir failed."
	echo "[FINGERPRINTS]" > doc/cfunge-man-fingerprints.in
	addtoman "Short descriptions of implemented fingerprints:"
	# I really hate aliases...
	SORTEDENTRIES=( $(IFS=$'\n'; echo "${ENTRIES[*]}" | sort -n) )
	for entry in "${SORTEDENTRIES[@]}"; do
		addtoman ".TP"
		addtoman "${MANENTRY1[$entry]}"
		addtoman "${MANENTRY2[$entry]}"
	done
	addtoman ".LP"
	addtoman "For more details please see the specs for each fingerprint."
	addtoman "In cases of undefined behaviour in fingerprints, cfunge mostly tries to do the"
	addtoman "same thing as CCBI."

	status "Man page generated"
	exit 0
fi

progress "Creating list file"
cat > "fingerprints.h" << EOF
/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2009 Arvid Norlander <anmaster AT tele2 DOT se>
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

#ifndef FUNGE_HAD_SRC_FINGERPRINTS_FINGERPRINTS_H
#define FUNGE_HAD_SRC_FINGERPRINTS_FINGERPRINTS_H

#include "../global.h"

#include <stdbool.h>
#include <stdint.h>

EOF

for fprint in "${FPRINTS[@]}"; do
	addtolist "#include \"${fprint}/${fprint}.h\""
done

cat >> "fingerprints.h" << EOF

typedef struct s_ImplementedFingerprintEntry {
	const funge_cell         fprint;   /**< Fingerprint. */
	const char            * uri;      /**< URI, used for Funge-109. */
	const fingerprintLoader loader;   /**< Loader function pointer. */
	const char            * opcodes;  /**< Sorted string with all implemented opcodes. */
	const char            * url;      /**< URL, used to show links for more info about fingerprints. */
	const bool              safe;     /**< If true, this fingerprint is safe in sandbox mode. */
} ImplementedFingerprintEntry;

// Implemented fingerprints
// NOTE: Keep sorted!
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
	[[ ${ENTRYL1_cond[$entry]} ]] && addtolist "${ENTRYL1_cond[$entry]}"
	addtolist "${ENTRYL2[$entry]}"
	addtolist "${ENTRYL3[$entry]}"
	addtolist "${ENTRYL4[$entry]}"
	[[ ${ENTRYL5_cond[$entry]} ]] && addtolist "${ENTRYL5_cond[$entry]}"
done


cat >> "fingerprints.h" << EOF
};

#endif
EOF

status "Done"

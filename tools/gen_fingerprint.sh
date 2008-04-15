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

# Generate a fingerprint template.

set -e

# Variables
FPRINT=""
URL=""
SAFE=""
OPCODES=""
DESCRIPTION=""

OPCODES=""
OPCODE_NAMES=()
OPCODE_DESC=()

# This must be run from top source directory.

die() {
	echo "ERROR: $1" >&2
	exit 1
}

progress() {
	echo " * ${1}..."
}
status() {
	echo "   ${1}"
}


# Char to decimal
ord() {
	printf -v "$1" '%d' "'$2"
}

if [[ -z $1 ]]; then
	echo "ERROR: Please provide finger print name!" >&2
	echo "Usage: $0 FingerprintName opcodes" >&2
	exit 1
else
	FPRINT="$1"
fi

progress "Sanity checking parameters"
if [[ $FPRINT =~ ^[A-Z0-9]{4}$ ]]; then
	status "Fingerprint name $FPRINT ok style."
# Yes those (space, / and \) break stuff...
# You got to create stuff on your own if you need those, and not include that
# in any function names or filenames.
elif [[ $FPRINT =~ ^[^\ /\\]{4}$ ]]; then
	status "Fingerprint name $FPRINT probably ok (but not common style)."
	status "Make sure each char is in the ASCII range 0-254."
	status "Note that alphanumeric (upper case only) fingerprint names are strongly prefered."
else
	die "Not valid format for fingerprint name."
fi

if [[ ! -d src/fingerprints ]]; then
	die "Run from top source directory please."
fi

if [[ -e src/fingerprints/$FPRINT ]]; then
	die "A fingerprint with that name already exists"
fi

progress "Looking for spec file"

if [[ -f "src/fingerprints/${FPRINT}.spec" ]]; then
	status "Good, spec file found."
else
	die "Sorry you need a spec file for the fingerprint. It should be placed at src/fingerprints/${FPRINT}.spec"
fi


progress "Parsing spec file"
IFS=$'\n'

# First line is %fingerprint-spec 1.0
exec 4<"src/fingerprints/${FPRINT}.spec"
read -ru 4 line
if [[ "$line" != "%fingerprint-spec 1.0" ]]; then
	die "Either the spec file is not a fingerprint spec, or it is not version 1.0 of the format."
fi

# 0: pre-"begin instrs"
# 1: "begin-instrs"
parsestate=0


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
			"%desc")
				DESCRIPTION="$data"
				;;
			"%safe")
				SAFE="$data"
				;;
			"%begin-instrs")
				parsestate=1
				;;
		esac
	else
		if [[ "$line" == "%end" ]]; then
			break
		fi
		# Parse instruction lines.
		IFS=$'\t' read -rd $'\n' instr name desc <<< "$line"

		OPCODES+="$instr"
		ord number "${instr:0:1}"
		OPCODE_NAMES[$number]="$name"
		OPCODE_DESC[$number]="$desc"
	fi
done

unset IFS

status "Done parsing."

exec 4<&-

progress "Validating the parsed data"

if [[ "$URL" ]]; then
	status "%url: Good, not empty"
else
	die "%url is not given or is empty."
fi

if [[ "$DESCRIPTION" ]]; then
	status "%desc: Good, not empty"
else
	die "%desc is not given or is empty."
fi

if [[ ( "$SAFE" == "true" ) || ( "$SAFE" == "false" ) ]]; then
	status "%safe: OK"
else
	die "%safe must be either true or false."
fi

if [[ "$OPCODES" =~ ^[A-Z]+$ ]]; then
	# Check that they are sorted.
	previousnr=0
	for (( i = 0; i < ${#OPCODES}; i++ )); do
		ord number "${OPCODES:$i:1}"
		if [[ $previousnr -ge $number ]]; then
			die "Instructions not sorted or there are duplicates"
		else
			previousnr=$number
		fi
	done
	status "Instructions: OK"
else
	die "The opcodes are not valid. The must be in the range A-Z"
fi

addtoh() {
	echo "$1" >> "${FPRINT}.h"
}
addtoc() {
	echo "$1" >> "${FPRINT}.c"
}



progress "Creating directory"
mkdir "src/fingerprints/$FPRINT" || die "mkdir failed"
cd "src/fingerprints/$FPRINT"

progress "Creating header file"
cat > "${FPRINT}.h" << EOF
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

EOF


addtoh "#ifndef _HAD_SRC_FINGERPRINTS_${FPRINT}_H"
addtoh "#define _HAD_SRC_FINGERPRINTS_${FPRINT}_H"

cat >> "${FPRINT}.h" << EOF

#include "../../global.h"
#include "../manager.h"

EOF

addtoh "bool Finger${FPRINT}load(instructionPointer * ip);"

addtoh ""
addtoh "#endif"

##############
# Now for .c #
##############

progress "Creating source file"
cat > "${FPRINT}.c" << EOF
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

EOF
addtoc "#include \"${FPRINT}.h\""
cat >> "${FPRINT}.c" << EOF
#include "../../stack.h"

// TODO: Add code to template functions

EOF

for (( i = 0; i < ${#OPCODES}; i++ )); do
	ord number "${OPCODES:$i:1}"
	addtoc "// ${OPCODES:$i:1} - ${OPCODE_DESC[$number]}"
	addtoc "static void Finger${FPRINT}${OPCODE_NAMES[$number]}(instructionPointer * ip)"
	addtoc '{'
	addtoc '}'
	addtoc ''
done



addtoc "bool Finger${FPRINT}load(instructionPointer * ip)"
addtoc '{'
for (( i = 0; i < ${#OPCODES}; i++ )); do
	ord number "${OPCODES:$i:1}"
	addtoc "	ManagerAddOpcode(${FPRINT},  '${OPCODES:$i:1}', ${OPCODE_NAMES[$number]})"
done

cat >> "${FPRINT}.c" << EOF
	return true;
}
EOF

status "File creation done"
echo
echo "To make cfunge aware of the new fingerprint run tools/gen_fprint_list.sh"
echo "You may need to run cmake or similar to make the build system aware as well."
echo
echo "All done! However make sure the copyright in the files is correct. Oh, and another thing: implement the fingerprint :)"

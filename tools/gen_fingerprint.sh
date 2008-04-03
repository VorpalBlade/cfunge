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

# This must be run from top source directory.

die() {
	echo "ERROR: $1" >&2
	exit 1
}

if [[ -z $1 ]]; then
	echo "ERROR: Please provide finger print name!" >&2
	echo "Usage: $0 FingerprintName opcodes" >&2
	exit 1
else
	FPRINT="$1"
fi

if [[ -z $2 ]]; then
	echo "ERROR: Please provide as second parameter a *sorted* list of implemented opcodes!" >&2
	echo "Usage: $0 FingerprintName opcodes" >&2
	exit 1
else
	if [[ $2 =~ ^[A-Z]+$ ]]; then
		OPCODES="$2"
	else
		die "The opcodes are not valid. The must be in the range A-Z"
	fi
fi

addtoh() {
	echo "$1" >> "${FPRINT}.h"
}
addtoc() {
	echo "$1" >> "${FPRINT}.c"
}


if [[ $FPRINT =~ ^[A-Z0-9]{4}$ ]]; then
	echo "Fingerprint name $FPRINT ok style."
# Yes those (space, / and \) break stuff...
# You got to create stuff on your own if you need those, and not include that
# in any function names or filenames.
elif [[ $FPRINT =~ ^[^\ /\\]{4}$ ]]; then
	echo "Fingerprint name $FPRINT probably ok (but not common style)."
	echo "Make sure each char is in the ASCII range 0-254."
	echo "Note that alphanumeric (upper case only) fingerprint names are strongly prefered."
else
	die "Not valid format for fingerprint name."
fi

if [[ ! -d src/fingerprints ]]; then
	die "Run from top source directory please."
fi

if [[ -e src/fingerprints/$FPRINT ]]; then
	die "A fingerprint with that name already exists"
fi

mkdir "src/fingerprints/$FPRINT" || die "mkdir failed"
cd "src/fingerprints/$FPRINT"

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

// Template functions, rename them.

EOF

for (( i = 0; i < ${#OPCODES}; i++ )); do
	addtoc "// ${OPCODES:$i:1} - "
	addtoc "static void Finger${FPRINT}function(instructionPointer * ip) {"
	addtoc '}'
	addtoc ''
done



addtoc "bool Finger${FPRINT}load(instructionPointer * ip) {"
addtoc "	// Insert the functions in question after the &"
for (( i = 0; i < ${#OPCODES}; i++ )); do
	addtoc "	if (!OpcodeStackAdd(ip, '${OPCODES:$i:1}', &))"
	addtoc "		return false;"
done

cat >> "${FPRINT}.c" << EOF
	return true;
}
EOF

echo "If the opcode list isn't sorted you may want to delete the result and rerun with it sorted."
echo "All done! However make sure the copyright in the files is correct. Oh, and another thing: implement the fingerprint :)"

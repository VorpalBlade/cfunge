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
	die "Please provide finger print name!"
else
	FPRINT="$1"
fi

if [[ $FPRINT =~ ^[A-Z0-9]{4}$ ]]; then
	echo "Fingerprint name $FPRINT ok style."
elif [[ $FPRINT =~ ^[^\ ]{4}$ ]]; then
	echo "Fingerprint name $FPRINT probably ok (but not common style)."
else
	die "Not valid format for fingerprint."
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
/*
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


echo "#ifndef _HAD_SRC_FINGERPRINTS_${FPRINT}_H" >> "${FPRINT}.h"
echo "#define _HAD_SRC_FINGERPRINTS_${FPRINT}_H" >> "${FPRINT}.h"

cat >> "${FPRINT}.h" << EOF

#include "../../global.h"
#include "../manager.h"

EOF

echo "bool Finger${FPRINT}load(instructionPointer * ip);" >> "${FPRINT}.h"

echo >> "${FPRINT}.h"
echo "#endif" >> "${FPRINT}.h"

##############
# Now for .c #
##############


cat > "${FPRINT}.c" << EOF
/*
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
echo "#include \"${FPRINT}.h\"" >> "${FPRINT}.c"
cat >> "${FPRINT}.c" << EOF
#include "../../stack.h"

// Template function
EOF
echo "static void Finger${FPRINT}function(instructionPointer * ip)" >> "${FPRINT}.c"

cat >> "${FPRINT}.c" << EOF
{
}

EOF

echo "bool Finger${FPRINT}load(instructionPointer * ip) {" >> "${FPRINT}.c"

echo "	if (!OpcodeStackAdd(ip, 'A', &Finger${FPRINT}function))"  >> "${FPRINT}.c"

cat >> "${FPRINT}.c" << EOF
		return false;
	if (!OpcodeStackAdd(ip, 'B', &...))
		return false;
	if (!OpcodeStackAdd(ip, 'R', &...))
		return false;
	return true;
}
EOF

echo "All done!"

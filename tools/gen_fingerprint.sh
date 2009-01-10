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

# Generate a fingerprint template.
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

# Variables
FPRINT=""
fp_URL=""
fp_SAFE=""
fp_CONDITION=""
fp_OPCODES=""
fp_DESCRIPTION=""

fp_OPCODES=""
fp_OPCODE_NAMES=()
fp_OPCODE_DESC=()

if [[ -z $1 ]]; then
	echo "ERROR: Please provide finger print name!" >&2
	echo "Usage: $0 FingerprintName opcodes" >&2
	exit 1
else
	FPRINT="$1"
fi

progress "Sanity checking parameters"
checkfprint "$FPRINT"

if [[ -e src/fingerprints/$FPRINT ]]; then
	die "A fingerprint with that name already exists"
fi

progress "Looking for spec file"

if [[ -f "src/fingerprints/${FPRINT}.spec" ]]; then
	status "Good, spec file found."
else
	die "Sorry, but you need a spec file for the fingerprint." \
	    "Either you misspelled the parameter to this script, or you misspelled" \
	    "the spec file. Or you forgot to create a spec file." \
	    "If you didn't typo the name on the command line to this script then" \
	    "the spec file should be placed at src/fingerprints/${FPRINT}.spec"
fi

cd "src/fingerprints" || die "Couldn't change directory to src/fingerprints"

progress "Parsing spec file"
parse_spec "${FPRINT}"

addtoh() {
	echo "$1" >> "${FPRINT}.h"
}
addtoc() {
	echo "$1" >> "${FPRINT}.c"
}



progress "Creating directory"
mkdir "$FPRINT" || die "mkdir failed"
cd "$FPRINT" || die "cd to src/fingerprints/$FPRINT failed"

progress "Creating header file"
cat > "${FPRINT}.h" << EOF
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

EOF

addtoh "#ifndef FUNGE_HAD_SRC_FINGERPRINTS_${FPRINT}_H"
addtoh "#define FUNGE_HAD_SRC_FINGERPRINTS_${FPRINT}_H"


cat >> "${FPRINT}.h" << EOF

#include "../../global.h"
#include "../manager.h"

EOF

if [[ "$fp_CONDITION" ]]; then
	addtoh "#if $fp_CONDITION"
fi

addtoh "bool finger_${FPRINT}_load(instructionPointer * ip);"

if [[ "$fp_CONDITION" ]]; then
	addtoh "#endif /* $fp_CONDITION */"
fi

addtoh ""
addtoh "#endif"

##############
# Now for .c #
##############

progress "Creating source file"
cat > "${FPRINT}.c" << EOF
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

EOF
addtoc "#include \"${FPRINT}.h\""

if [[ "$fp_CONDITION" ]]; then
	addtoc "#if $fp_CONDITION"
fi

cat >> "${FPRINT}.c" << EOF
#include "../../stack.h"

// TODO: Add code to template functions

EOF

for (( i = 0; i < ${#fp_OPCODES}; i++ )); do
	ord number "${fp_OPCODES:$i:1}"
	addtoc "/// ${fp_OPCODES:$i:1} - ${fp_OPCODE_DESC[$number]}"
	addtoc "static void finger_${FPRINT}_${fp_OPCODE_NAMES[$number]}(instructionPointer * ip)"
	addtoc '{'
	addtoc '}'
	addtoc ''
done



addtoc "bool finger_${FPRINT}_load(instructionPointer * ip)"
addtoc '{'
for (( i = 0; i < ${#fp_OPCODES}; i++ )); do
	ord number "${fp_OPCODES:$i:1}"
	addtoc "	manager_add_opcode(${FPRINT}, '${fp_OPCODES:$i:1}', ${fp_OPCODE_NAMES[$number]})"
done

cat >> "${FPRINT}.c" << EOF
	return true;
}
EOF

if [[ "$fp_CONDITION" ]]; then
	addtoc "#endif /* $fp_CONDITION */"
fi

status "File creation done"
echo
echo "To make cfunge aware of the new fingerprint run tools/gen_fprint_list.sh"
echo "You may need to run cmake or similar to make the build system aware as well."
echo
echo "All done! However make sure the copyright in the files is correct. Oh, and another thing: implement the fingerprint :)"

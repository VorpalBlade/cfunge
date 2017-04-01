#!/bin/bash
# Runner for mycology
# $1 Path to python runner
# $2 Path to cfunge
# $3 Path to mycology directory (where this file resides)

cp "$3"/src/*.b98 "$3"/src/*.bf "$3"/*.expected .

"$1" "$2" mycology.b98 "$3/mycology_output_filter.sh" --exit-code 15 || exit 1

echo -e "1\nx\n7\n16\nabc\n" | "$1" "$2" mycouser.b98 || exit 1

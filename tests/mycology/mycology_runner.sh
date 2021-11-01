#!/bin/bash
# Runner for mycology
# $1 Path to Python
# $2 Path to python runner
# $3 Path to cfunge
# $4 Path to mycology directory (where this file resides)

PYTHON="$1"
RUNNER="$2"
CFUNGE="$3"
MYCOLOGY="$4"

cp "$MYCOLOGY"/src/*.b98 "$MYCOLOGY"/src/*.bf "$MYCOLOGY"/*.expected .

"$PYTHON" "$RUNNER" "$CFUNGE" mycology.b98 "$MYCOLOGY/mycology_output_filter.sh" --exit-code 15 || exit 1

echo -e "1\nx\n7\n16\nabc\n" | "$PYTHON" "$RUNNER" "$CFUNGE" mycouser.b98 || exit 1

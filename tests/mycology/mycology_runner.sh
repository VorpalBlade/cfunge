#!/bin/bash
# Runner for mycology
# $1 Path to python runner
# $2 Path to cfunge
# $3 Path to mycology directory (where this file resides)

for i in "$3"/src/*.b98 "$3"/src/*.bf "$3/mycology.expected" "$3"/*.expected; do
	cp "$i" .
done

exec "$1" "$2" mycology.b98 "$3/mycology_output_filter.sh" --exit-code 15

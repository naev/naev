#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")

tmp=$(mktemp)
trap 'rm -f $tmp' EXIT

grep -H -m 1 '^<[a-zA-Z]* name="' $(find $DST -name "*.xml") |
sed 's/^\(.*\)\/\([^/]*\)\.xml:.*name=\"\([^"]*\)\".*$/\1\/ \2 \3/' |
# from now on, we have lines: <path> <filename> <name>
tee $tmp | cut "-d " -f3- | tr '[:upper:]' '[:lower:]' | $SCRIPT_DIR/xml_name.sed |
# rebuild lines with modified names
paste - $tmp '-d ' | cut '-d ' -f-3 | sed 's/^\([^ ]*\)\ \([^ ]*\)\ \([^ ]*\)/\2 \3 \1/' |
# keep only lines where <filename> != <name>
grep -v '^[^ ]*\(\ [^ ]*\)\1$' |
sed 's/^\([^ ]*\)\ \([^ ]*\)\ \([^ ]*\)$/\1\{\2 => \3\}.xml/'

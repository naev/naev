#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DEC="$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../map_decorator")"

grep -r '<\(x\|y\)' "$DEC"/*.xml |
sed -e 's/^.*\/\(.*\).xml:/\1/' -e 's/^.*<y>\(.*\)<\/y>/\1/' |
sed -e 's/<x>//' -e 's/<\/x>/X/' -e 's/$/X/' -e 's/XX/ /' |
tr -d '\n' | tr 'X' '\n' |
while read -ra line ; do
   echo -n "${line[0]} = "
   "$SCRIPT_DIR"/ssysmap2graph.sh | "$SCRIPT_DIR"/closest.py "${line[@]:1:2}"
done

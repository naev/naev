#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&
TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

for i in *.xml; do
   tac "$i" | sed -e '0,/<opos x="/{//d}' >"$TMP"
   tac "$TMP" > "$i"
done

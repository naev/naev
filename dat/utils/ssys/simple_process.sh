#!/usr/bin/env bash

trap 'exit 0' SIGINT
DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DAT=$(realpath --relative-to="$PWD" "${DIR}/../../")
DST="$DAT/ssys"
POVF=()
POVO='-q'

"$DIR"/ssysmap2graph.sh       |
"$DIR"/graph_vaux.py -e -c -n |
grep -v '^$'                  |
sort -d                       |
# shellcheck disable=SC2002
tee >("$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_ini') |
"$DIR"/graphmod_center.py |
tee >("$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_fin') >("$DIR"/decorators.py) |
"$DIR"/graph2ssysmap.py

#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")

git checkout "$DST"

echo "freeze" >&2
$SCRIPT_DIR/sys_freeze.py $DST/*.xml > /dev/null
echo "gen before graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml -k | neato -n2 -Tpng 2>/dev/null > before.png
echo "gen after graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml | neato 2>/dev/null | tee after.dot | neato -n2 -Tpng 2>/dev/null > after.png
echo "apply after graph" >&2
$SCRIPT_DIR/dot2sys.py < after.dot
echo "gen final graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml -k | neato -n2 -Tpng 2>/dev/null > final.png
#echo "relax" >&2
#$SCRIPT_DIR/sys_relax.py $DST/*.xml > /dev/null
#$SCRIPT_DIR/sys_unfreeze.sh $DST/*.xml > /dev/null

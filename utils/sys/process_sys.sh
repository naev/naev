#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAS=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat")
DST="$BAS/ssys"

git checkout "$BAS/spob" "$DST"

echo "freeze non-empty:" >&2
$SCRIPT_DIR/sys_empty.py -r $DST/*.xml | $SCRIPT_DIR/sys_freeze.py -f | wc -l
echo "gen before graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml -k | neato -n2 -Tpng 2>/dev/null > before.png
echo "gen after graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml | neato 2>/dev/null | tee after.dot | neato -n2 -Tpng 2>/dev/null > after.png
echo "apply after graph" >&2
$SCRIPT_DIR/dot2sys.py < after.dot
echo "gen final graph" >&2
$SCRIPT_DIR/sys2dot.py $DST/*.xml -k | neato -n2 -Tpng 2>/dev/null > final.png
#echo "relax" >&2
#$SCRIPT_DIR/sys_relax.py $DST/*.xml | wc -l

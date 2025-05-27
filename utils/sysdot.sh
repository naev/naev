#!/usr/bin/bash

git checkout dat/
echo "gen before graph" >&2
./utils/sys2dot.py dat/ssys/*.xml -k | neato -n2 -Tpng 2>/dev/null > before.png
echo "gen after graph" >&2
./utils/sys2dot.py dat/ssys/*.xml | neato 2>/dev/null | tee after.dot | neato -Tpng 2>/dev/null > after.png
echo "apply after graph" >&2
 ./utils/dot2sys.py < after.dot
echo "gen final graph" >&2
./utils/sys2dot.py dat/ssys/*.xml -k | neato -n2 -Tpng 2>/dev/null > final.png


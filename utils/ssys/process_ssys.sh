#!/usr/bin/bash

repos_systems=(doowa flok firk)
repos_systems2=(terminus)

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Applies the whole remap process. See the script content." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAS=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat")
DST="$BAS/ssys"

COL=-C

git checkout "$BAS/spob" "$DST"

echo -n "gen colored sys map... " >&2
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) && $cmd 2>/dev/null && mv -v out.png map_bef.png
echo -n "freeze non-empty: " >&2
"$SCRIPT_DIR"/ssys_empty.py -r "$DST"/*.xml | "$SCRIPT_DIR"/ssys_freeze.py -f | wc -l
echo -n "gen before graph " >&2
"$SCRIPT_DIR"/ssys2dot.py $COL "$DST"/*.xml -k | neato -n2 -Tpng 2>/dev/null > before.png
echo -n -e "\ngen after graph " >&2
"$SCRIPT_DIR"/ssys2dot.py "$DST"/*.xml | tee before.dot | neato 2>/dev/null |
tee after.dot | neato -n2 -Tpng 2>/dev/null > after.png
echo -n -e "\napply after graph " >&2
"$SCRIPT_DIR"/dot2graph.py < after.dot |
"$SCRIPT_DIR"/pp_graph.py |
"$SCRIPT_DIR"/graph_repos_virt.py |
"$SCRIPT_DIR"/ssys_graph.py -w
echo -n "gen final graph " >&2
"$SCRIPT_DIR"/ssys2dot.py $COL "$DST"/*.xml -k | neato -n2 -Tpng 2>/dev/null > final.png
echo -e -n "\ngen colored sys map... " >&2
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) && $cmd 2>/dev/null && mv -v out.png map_fin.png
echo -e -n "\nselect Sirius systems " >&2
read -ra SIRIUS <<< "$(./utils/ssys/ssys_graph.py | grep 'teal' | cut '-d ' -f1)"
echo -e -n "\nreposition " >&2
"$SCRIPT_DIR"/repos.sh -C
("$SCRIPT_DIR"/repos.sh 5 -i -q "${SIRIUS[@]}" && "$SCRIPT_DIR"/ssys_edges.sh) |
"$SCRIPT_DIR"/reposition -e -w0 "${repos_systems[@]}" |
"$SCRIPT_DIR"/reposition -w0 "${repos_systems2[@]}" |
"$SCRIPT_DIR"/ssys_graph.py -w
echo -n "\nposition virtual sys + smooth tradelane " >&2
"$SCRIPT_DIR"/ssys_pos.sh |
"$SCRIPT_DIR"/graph_repos_virt.py |
"$SCRIPT_DIR"/ssys_graph.py -w
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) && $cmd 2>/dev/null && mv -v out.png map_repos.png
echo -n "apply gravity -> colored sys map... " >&2
cmd=$( "$SCRIPT_DIR"/apply_pot.sh -g | "$SCRIPT_DIR"/ssys2pov.py -g -C "$DST"/*.xml) &&
$cmd 2>/dev/null && mv -v out.png map_fin_g.png
echo "relax ssys.." >&2
echo "total relaxed : $("$SCRIPT_DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)" >&2

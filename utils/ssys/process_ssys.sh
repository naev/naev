#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Applies the whole remap process. See the script content." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAS=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat")
DST="$BAS/ssys"

COL=-C

msg() {
   echo -e "$1" | while IFS=$'\n' read -ra line; do
      if [ -z "$line" ] ; then
         echo ""
      else
         echo -e -n "\e[36m$line\e[0m "
      fi
   done >&2
}

git checkout "$BAS/spob" "$DST"

msg "gen colored sys map..."
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) && $cmd 2>/dev/null && mv -v out.png map_bef.png

msg "freeze non-nempty:"
echo -e "\e[32m$("$SCRIPT_DIR"/ssys_empty.py -r "$DST"/*.xml | "$SCRIPT_DIR"/ssys_freeze.py -f | wc -l)" >&2

msg "gen before graph"
"$SCRIPT_DIR"/ssys2dot.py $COL "$DST"/*.xml -k | neato -n2 -Tpng 2>/dev/null > before.png

msg "\ngen after graph"
"$SCRIPT_DIR"/ssys2dot.py "$DST"/*.xml | tee before.dot | neato 2>/dev/null |
tee after.dot | neato -n2 -Tpng 2>/dev/null > after.png
("$SCRIPT_DIR"/dot2graph.py < after.dot && "$SCRIPT_DIR"/ssys_graph.sh -e) |
"$SCRIPT_DIR"/graphmod_repos_virt.py |
"$SCRIPT_DIR"/ssys_graph.py -w
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) &&
$cmd 2>/dev/null && mv -v out.png map_dot.png

msg "pprocess"
"$SCRIPT_DIR"/ssys_graph.sh |
"$SCRIPT_DIR"/graphmod_pp.py |
"$SCRIPT_DIR"/graphmod_repos_virt.py |
"$SCRIPT_DIR"/ssys_graph.py -w
cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) &&
$cmd 2>/dev/null && mv -v out.png map_fin.png


msg "gen final graph"
"$SCRIPT_DIR"/ssys2dot.py $COL "$DST"/*.xml -k | neato -n2 -Tpng 2>/dev/null > final.png

#msg "\nselect Sirius systems " >&2
#read -ra SIRIUS <<< "$(./utils/ssys/ssys_graph.py -v | grep 'teal' | cut '-d ' -f1)"

#s=(doowa flok firk)
#repos_systems2=(terminus)
#PROTERON=(leporis hystera korifa apik telika mida ekta akra)
#TWINS=(carnis_minor carnis_major gliese gliese_minor kruger krugers_pocket)
N=4
msg "(reposition sys + smooth tradelane)x$N  +  position virtual"
SPIR=(syndania nirtos sagittarius hopa scholzs_star veses alpha_centauri padonia urillian baitas protera tasopa)
ABH=(anubis_black_hole ngc11935 ngc5483 ngc7078 ngc7533 octavian copernicus ngc13674 ngc1562 ngc2601)
read -ra ALMOST_ALL <<< "$("$SCRIPT_DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}")"
"$SCRIPT_DIR"/repos.sh -C
for i in $(seq "$N"); do
   "$SCRIPT_DIR"/ssys_graph.sh |
   "$SCRIPT_DIR"/reposition -q -e -i "${ALMOST_ALL[@]}" |
   "$SCRIPT_DIR"/graphmod_smooth_tl.py |
   "$SCRIPT_DIR"/ssys_graph.py -w
   msg "\e[32m$i"
done
"$SCRIPT_DIR"/ssys_graph.sh |
"$SCRIPT_DIR"/graphmod_repos_virt.py |
"$SCRIPT_DIR"/ssys_graph.py -w
msg ""

cmd=$("$SCRIPT_DIR"/ssys2pov.py -C "$DST"/*.xml) &&
$cmd 2>/dev/null && mv -v out.png map_repos.png

msg "apply gravity -> colored sys map..."
cmd=$( "$SCRIPT_DIR"/apply_pot.sh -g |
"$SCRIPT_DIR"/ssys2pov.py -g -C "$DST"/*.xml) &&
$cmd 2>/dev/null && mv -v out.png map_fin_g.png

msg "relax ssys..\n"
msg "total relaxed: \e[32m$("$SCRIPT_DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"

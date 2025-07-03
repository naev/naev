#!/usr/bin/bash


if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Applies the whole remap process. See the script content." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAS=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat")
DST="$BAS/ssys"

msg() {
   echo -e "$1" | while read -r line; do
      if [ -z "$line" ] ; then
         echo ""
      else
         echo -e -n "\e[36m$line\e[0m "
      fi
   done >&2
}

pmsg() {
   cat
   msg "$@"
}

N_ITER=5
#msg "\nselect Sirius systems " >&2
#read -ra SIRIUS <<< "$("$SCRIPT_DIR"/ssys_graph.sh -v |
# "$SCRIPT_DIR"/graph_vaux.py | grep 'sirius' | cut '-d ' -f1)"
#s=(doowa flok firk)
#repos_systems2=(terminus)
#PROTERON=(leporis hystera korifa apik telika mida ekta akra)
#TWINS=(carnis_minor carnis_major gliese gliese_minor kruger krugers_pocket)
SPIR=(syndania nirtos sagittarius hopa scholzs_star veses alpha_centauri padonia urillian baitas protera tasopa)
ABH=(anubis_black_hole ngc11935 ngc5483 ngc7078 ngc7533 octavian copernicus ngc13674 ngc1562 ngc2601)
read -ra ALMOST_ALL <<< "$("$SCRIPT_DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}")"
"$SCRIPT_DIR"/repos.sh -C || exit 1
"$SCRIPT_DIR"/apply_pot.sh -C || exit 1


# Ok, let's go!
git checkout "$BAS/spob" "$DST"

msg "freeze non-nempty"
echo -e "\e[32m$("$SCRIPT_DIR"/ssys_empty.py -r "$DST"/*.xml |
"$SCRIPT_DIR"/ssys_freeze.py -f | wc -l)" >&2

msg "gen crt sys map"
"$SCRIPT_DIR"/ssys2graph.sh |
"$SCRIPT_DIR"/graph_vaux.py -c -n |
tee >("$SCRIPT_DIR"/graph2pov.py -c -q'map_bef.png') |
pmsg "gen before graph" |
"$SCRIPT_DIR"/graphmod_prep.py |
tee >(
   "$SCRIPT_DIR"/graph2dot.py -c -k |
   neato -n2 -Tpng 2>/dev/null > before.png
) |
pmsg "\napply neato" |
"$SCRIPT_DIR"/graph2dot.py -c | tee before.dot | neato 2>/dev/null |
("$SCRIPT_DIR"/dot2graph.py && "$SCRIPT_DIR"/ssys2graph.sh -e) |
("$SCRIPT_DIR"/graphmod_repos_virt.py && echo 'zied 0 0') |
"$SCRIPT_DIR"/graph_vaux.py -c -n |
tee >("$SCRIPT_DIR"/graph2pov.py -c -q'map_dot.png') |
pmsg "pprocess" |
"$SCRIPT_DIR"/graphmod_postp.py |
"$SCRIPT_DIR"/graphmod_repos_virt.py |
tee >("$SCRIPT_DIR"/graph2pov.py -c -q'map_post.png') |
pmsg "${N_ITER} x (reposition sys + smooth tradelane) + position virtual" |
"$SCRIPT_DIR"/repeat.sh "$N_ITER" "$SCRIPT_DIR"/graphmod_repos.sh "$SCRIPT_DIR" "${ALMOST_ALL[@]}" |
"$SCRIPT_DIR"/graphmod_repos_virt.py |
pmsg "" |
tee >("$SCRIPT_DIR"/graph2pov.py -c -q'map_repos.png') |
pmsg "apply gravity" |
"$SCRIPT_DIR"/apply_pot.sh -g |
"$SCRIPT_DIR"/graphmod_repos.sh "$SCRIPT_DIR"|
tee >("$SCRIPT_DIR"/graph2pov.py -c -q'map_grav.png') |
pmsg "gen final graph" |
tee >("$SCRIPT_DIR"/graph2ssys.py) |
"$SCRIPT_DIR"/graph2dot.py -c -k |
neato -n2 -Tpng 2>/dev/null > final.png

msg "\nrelax ssys..\n"
msg "total relaxed: \e[32m$("$SCRIPT_DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"

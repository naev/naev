#!/usr/bin/bash


if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") [-f]"
      "  Applies the whole remap process. See the script content."
      "  Unless -f is set, does not save anything."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAS=$(realpath --relative-to="$PWD" "${DIR}/../../dat")
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

N_ITER=6
#msg "\nselect Sirius systems " >&2
#read -ra SIRIUS <<< "$("$DIR"/ssys_graph.sh -v |
# "$DIR"/graph_vaux.py | grep 'sirius' | cut '-d ' -f1)"
#s=(doowa flok firk)
#repos_systems2=(terminus)
#PROTERON=(leporis hystera korifa apik telika mida ekta akra)
#TWINS=(carnis_minor carnis_major gliese gliese_minor kruger krugers_pocket)
SPIR=(syndania nirtos sagittarius hopa scholzs_star veses alpha_centauri padonia urillian baitas protera tasopa)
ABH=(anubis_black_hole ngc11935 ngc5483 ngc7078 ngc7533 octavian copernicus ngc13674 ngc1562 ngc2601)
read -ra ALMOST_ALL <<< "$("$DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}")"
"$DIR"/repos.sh -C || exit 1
"$DIR"/apply_pot.sh -C || exit 1
"$DIR"/gen_decorators.sh -C || exit 1

# handy debug line to insert anywhere:
#tee >(cat >&2) | # DEBUG OUTPUT

# Ok, let's go!
if [ "$1" = '-f' ] ; then
   #git checkout "$BAS/spob" "$DST"
   msg "freeze non-nempty"
   echo -e "\e[32m$(
      "$DIR"/ssys_empty.py -r "$DST"/*.xml |
      "$DIR"/ssys_freeze.py -f | wc -l)\e[0m" >&2
fi

msg "gen before graph"
"$DIR"/ssys2graph.sh                                                          |
"$DIR"/graph_vaux.py -e -c -n                                                 |
"$DIR"/graphmod_prep.py                                                       |
tee >("$DIR"/graph2pov.py -c -q'map_bef.png')                                 |
tee                                                                        >(
   "$DIR"/graph2dot.py -c -k |
   neato -n2 -Tpng 2>/dev/null > before.png                                 ) |
pmsg "apply neato"                                                            |
tee >("$DIR"/graph2dot.py -c | neato 2>/dev/null)                             |
"$DIR"/dot2graph.py                                                           |
"$DIR"/graphmod_virtual.py                                                    |
tee >("$DIR"/graph2pov.py -c -q'map_dot.png')                                 |
pmsg "pprocess"                                                               |
"$DIR"/graphmod_postp.py                                                      |
"$DIR"/graphmod_virtual.py                                                    |
tee >("$DIR"/graph2pov.py -c -q'map_post.png')                                |
pmsg "${N_ITER} x (repos sys + smooth tradelane) + virtual"                   |
"$DIR"/repeat.sh "$N_ITER" "$DIR"/graphmod_repos.sh "$DIR" "${ALMOST_ALL[@]}" |
"$DIR"/graphmod_virtual.py                                                    |
pmsg ""                                                                       |
tee >("$DIR"/graph2pov.py -c -q'map_repos.png')                               |
if [ "$1" = "-f" ] ; then tee >("$DIR"/graph2ssys.py) ; else cat ; fi         |
pmsg "apply gravity"                                                          |
"$DIR"/apply_pot.sh -g                                                        |
"$DIR"/graphmod_stretch_north.py                                              |
tee >("$DIR"/graph2pov.py -c -q'map_grav.png')                                |
"$DIR"/graphmod_abh.py                                                        |
pmsg "gen final graph"                                                        |
"$DIR"/graphmod_repos.sh "$DIR"                                               |
"$DIR"/graphmod_abh.py                                                        |
"$DIR"/graphmod_final.py                                                      |
tee >("$DIR"/graph2pov.py -c -q'map_final.png')                               |
"$DIR"/graph2dot.py -c -k | neato -n2 -Tpng 2>/dev/null > after.png

msg ""
if [ "$1" = '-f' ] ; then
   msg "relax ssys..\n"
   msg "total relaxed: \e[32m$("$DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"
fi

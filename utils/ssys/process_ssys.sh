#!/usr/bin/bash

trap 'exit 0' SIGINT
DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") [-H] [-f] [-v]"
      "  Applies the whole remap process. See the script content."
      "  Unless -f is set, does not save anything."
      "  If -H is set, pov outputs are 1080p."
      "  If -v is set, povray output is displayed."
      "  If -n is set, no povray preview."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

POVF=()
POVO='-q'
for i in "$@" ; do
   if [ "$i" = "-f" ] ; then
      FORCE=1
   elif [ "$i" = "-H" ] ; then
      POVF+=("-H")
   elif [ "$i" = "-n" ] ; then
      POVF+=("-n")
   elif [ "$i" = "-v" ] ; then
      POVO='-p'
   else
      echo "Ignored: \"$i\"" >&2
   fi
done

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
if [ -n "$FORCE" ] ; then
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
tee >("$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_bef')                   |
tee                                                                        >(
   "$DIR"/graph2dot.py -c -k |
   neato -n2 -Tpng 2>/dev/null > before.png                                 ) |
pmsg "apply neato"                                                            |
tee >("$DIR"/graph2dot.py -c | neato 2>/dev/null)                             |
"$DIR"/dot2graph.py                                                           |
grep -v ' virtual$'                                                           |
"$DIR"/graphmod_virtual_ssys.py                                               |
tee >("$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_dot')                      |
pmsg "pprocess"                                                               |
"$DIR"/graphmod_postp.py                                                      |
"$DIR"/graphmod_virtual_ssys.py                                               |
tee >("$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_post')                     |
pmsg "${N_ITER} x (repos sys + smooth tradelane) + virtual"                   |
"$DIR"/repeat.sh "$N_ITER" "$DIR"/graphmod_repos.sh "$DIR" "${ALMOST_ALL[@]}" |
pmsg ""                                                                       |
"$DIR"/graphmod_virtual_ssys.py                                               |
tee >("$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_repos')                    |
if [ -n "$FORCE" ] ; then tee >("$DIR"/graph2ssys.py) ; else cat ; fi         |
pmsg "apply gravity"                                                          |
"$DIR"/apply_pot.sh -g                                                        |
"$DIR"/graphmod_stretch_north.py                                              |
"$DIR"/graphmod_virtual_ssys.py                                               |
tee >("$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_grav')                     |
"$DIR"/graphmod_abh.py                                                        |
pmsg "gen final graph"                                                        |
"$DIR"/graphmod_repos.sh "$DIR" "${ALMOST_ALL[@]}"                            |
"$DIR"/graphmod_abh.py                                                        |
"$DIR"/graphmod_final.py                                                      |
"$DIR"/graphmod_virtual_ssys.py                                               |
tee >("$DIR"/graph2dot.py -c -k | neato -n2 -Tpng 2>/dev/null > after.png)    |
"$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_aft'                          |

msg ""
if [ -n "$FORCE" ] ; then
   msg "relax ssys..\n"
   msg "total relaxed: \e[32m$("$DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"
fi

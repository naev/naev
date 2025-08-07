#!/bin/bash

N_ITER=5

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
      "  If -N is set, no picture generated."
      "  If -S is set, no spoilers on pictures."
      "  If -E is set, early game map."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

POVF=()
POVO='-q'
SPOIL_FILTER="cat"
E_FILTER="$DIR"/graph_earlygame.py
S_FILTER="$DIR"/graph_unspoil.sh
for i in "$@" ; do
   if [ "$i" = "-f" ] ; then
      FORCE=1
   elif [ "$i" = "-H" ] ; then
      POVF+=("-H")
   elif [ "$i" = "-N" ] ; then
      NOPIC=1
   elif [ "$i" = "-n" ] ; then
      POVF+=("-n")
   elif [ "$i" = "-S" ] ; then
      if [ "SPOIL_FILTER" = "$E_FILTER" ] ; then
         echo "warning: -S overwrites previous -E." >&2
      fi
      SPOIL_FILTER="$S_FILTER"
   elif [ "$i" = "-E" ] ; then
      if [ "SPOIL_FILTER" = "$S_FILTER" ] ; then
         echo "warning: -E overwrites previous -S." >&2
      fi
      SPOIL_FILTER="$E_FILTER"
   elif [ "$i" = "-v" ] ; then
      POVO='-p'
   else
      echo "Ignored: \"$i\"" >&2
   fi
done

BAS=$(realpath --relative-to="$PWD" "${DIR}/../../dat")
DST="$BAS/ssys"

"$DIR"/repos.sh -C || exit 1
"$DIR"/apply_pot.sh -C || exit 1
"$DIR"/gen_decorators.sh -C || exit 1


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
   cat; msg "$@"
}

IGN() {
   cat >/dev/null
}

DEBUG() {
   tee >(cat >&2)
}


#msg "\nselect Sirius systems " >&2
#read -ra SIRIUS <<< "$("$DIR"/ssys_graph.sh -v |
# "$DIR"/graph_vaux.py | grep 'sirius' | cut '-d ' -f1)"

msg "extract input graph"
TMP=$(mktemp)
trap 'rm -f $TMP' EXIT
"$DIR"/ssysmap2graph.sh | "$DIR"/graph_vaux.py -e -c -n > "$TMP"

#PROTERON=(leporis hystera korifa apik telika mida ekta akra)
SPIR=(syndania nirtos sagittarius hopa scholzs_star veses alpha_centauri padonia urillian baitas protera tasopa)
ABH=(anubis_black_hole ngc11935 ngc5483 ngc7078 ngc7533 octavian copernicus ngc13674 ngc1562 ngc2601)
read -ra ALMOST_ALL <<< "$("$DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}")"
read -ra TERM_SSYS <<< "$("$DIR"/terminal_ssys.py < "$TMP")"
read -ra SMOOTH_SSYS <<< "$("$DIR"/graphmod_smooth.py -L < "$TMP")"
read -ra ALMOST_ALMOST_ALL <<< "$("$DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}" "${TERM_SSYS[@]}" "${SMOOTH_SSYS[@]}" )"
read -ra SWR <<< "$("$DIR"/graphmod_stellarwind_road.py -L < "$TMP")"

msg ""
# Ok, let's go!
if [ -n "$FORCE" ] ; then
   #git checkout "$BAS/spob" "$DST"
   msg "freeze non-nempty"
   echo -e "\e[32m$(
      "$DIR"/ssys_empty.py -r "$DST"/*.xml |
      "$DIR"/ssys_freeze.py -f | wc -l)\e[0m" >&2
fi

msg "gen before graph"
# shellcheck disable=SC2002
cat "$TMP"                                                                    |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >(
      $SPOIL_FILTER |
      tee >("$DIR"/graph2dot.py -c -k | neato -n2 -Tpng 2>/dev/null > before.png) |
      "$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_ini'
   )
else cat ;                                                                  fi |
"$DIR"/graphmod_prep.py                                                        |
"$DIR"/graphmod_vedges.py                                                      |
if [ -z "$NOPIC" ] ;                                                      then
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_bef')
else pmsg "" ;                                                              fi |
pmsg "apply neato"                                                             |
tee >("$DIR"/graph2dot.py -c | neato 2>/dev/null)                              |
"$DIR"/dot2graph.py                                                            |
grep -v ' virtual$'                                                            |
"$DIR"/graphmod_vedges.py                                                      |
"$DIR"/graphmod_virtual_ssys.py                                                |
if [ -z "$NOPIC" ] ;                                                      then
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_dot')
else cat ;                                                                  fi |
pmsg "pprocess"                                                                |
"$DIR"/graphmod_postp.py                                                       |
"$DIR"/graphmod_virtual_ssys.py                                                |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_post')
else pmsg "" ;                                                              fi |
pmsg "${N_ITER} x (repos sys + smooth/round lanes) + virtual"                  |
"$DIR"/repeat.sh "$N_ITER" "$DIR"/graphmod_repos.sh "$DIR" "${ALMOST_ALL[@]}"  |
pmsg ""                                                                        |
"$DIR"/graphmod_virtual_ssys.py                                                |
if [ -z "$NOPIC" ] ;                                                      then
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_repos')
else cat ;                                                                  fi |
pmsg "apply gravity"                                                           |
"$DIR"/apply_pot.sh -g                                                         |
"$DIR"/graphmod_stretch_north.py                                               |
"$DIR"/graphmod_virtual_ssys.py                                                |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_grav')
else pmsg "" ;                                                              fi |
pmsg "post-process "                                                           |
"$DIR"/repeat.sh 3 "$DIR"/graphmod_repos.sh "$DIR" "${ALMOST_ALMOST_ALL[@]}"   |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_aft')
else pmsg "" ;                                                              fi |
"$DIR"/graphmod_virtual_ssys.py                                                |
grep -v ' virtual$'                                                            |
pmsg "stellarwind road"                                                        |
"$DIR"/graphmod_stellarwind_road.py                                            |
"$DIR"/repeat.sh 0 "$DIR"/reposition -e -q "${SWR[@]}"                         |
"$DIR"/reposition -e -q "yarn" "griffin"                                       |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_swr')
else pmsg "";                                                               fi |
pmsg "finally"                                                                 |
if [ -n "$FORCE" ] ;                                                      then
   tee >("$DIR"/graph2ssysmap.py) >("$DIR"/decorators.py)
else cat ;                                                                  fi |
if [ -z "$NOPIC" ] ; then
   pmsg "" |
   $SPOIL_FILTER |
   tee >("$DIR"/graph2dot.py -c -k | neato -n2 -Tpng 2>/dev/null > after.png) |
   "$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_fin'
else pmsg "" >/dev/null ; fi

if [ -n "$FORCE" ] ; then
   msg "relax ssys..\n"
   msg "total relaxed: \e[32m$("$DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"
fi

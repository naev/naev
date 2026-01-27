#!/usr/bin/env bash

N_ITER=5

trap 'exit 0' SIGINT
DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DAT=$(realpath --relative-to="$PWD" "${DIR}/../../")

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") [OPTIONS] [-n] [-f]"
      "  Applies the whole remap process. See the script content."
      "  Unless -f is set, does not save anything."
      "  If -n is set, no povray preview."
      "options:"
      "  If -N is set, no picture generated."
      "  If -F is set, only final pov is generated; other povs are disabled."
      "  If -H is set, pov outputs are 1080p."
      "  If -v is set, povray output is printed."
      "  If -S is set, no spoilers on pictures."
      "  If -E is set, early game map."
      "  If --clean-ssys is set, cleans up data/ dir before starting."
      "    This will REMOVE anything git does not know in SSYS"
      "    and will RESET everything in SSYS, SPOB, and MAP_DECORATOR"
      "  If --just-clean-ssys is set, applies the previous one and stops."
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
   elif [ "$i" = "--clean-ssys" ] || [ "$i" == "--just-clean-ssys" ] ; then
      git clean -fd "$DAT/ssys"
      git checkout "$DAT/ssys" "$DAT/map_decorator" "$DAT/spob"
      if [ "$i" == "--just-clean-ssys" ] ; then
         exit 0
      fi
   elif [ "$i" = "-F" ] ; then
      NOPIC=2
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

DST="$DAT/ssys"
"$DIR"/repos.sh -C || exit 1
"$DIR"/apply_pot.sh -C || exit 1
if [ ! "$NOPIC" = "1" ] ; then
   "$DIR"/gen_decorators.sh -C || exit 1
fi

msg() {
   echo -e "$1" | while read -r line; do
      if [ -z "$line" ] ; then
         echo
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
"$DIR"/ssysmap2graph.sh       |
"$DIR"/graph_vaux.py -e -c -n |
grep -v '^$'                  |
sort -d                       > "$TMP"
echo >> "$TMP"

#PROTERON=(leporis hystera korifa apik telika mida ekta akra)
SPIR=(syndania nirtos sagittarius hopa scholzs_star veses alpha_centauri padonia urillian baitas protera tasopa)
ABH=(anubis_black_hole ngc11935 ngc5483 ngc7078 ngc7533 octavian copernicus ngc13674 ngc1562 ngc2601)
read -ra ALMOST_ALL <<< "$("$DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}")"
read -ra TERM_SSYS <<< "$("$DIR"/terminal_ssys.py < "$TMP")"
read -ra SMOOTH_SSYS <<< "$("$DIR"/graphmod_smooth.py -L < "$TMP")"
read -ra ALMOST_ALMOST_ALL <<< "$("$DIR"/all_ssys_but.sh "${SPIR[@]}" "${ABH[@]}" "${TERM_SSYS[@]}" "${SMOOTH_SSYS[@]}" )"
#read -ra SWR <<< "$("$DIR"/graphmod_stellarwind_road.py -L < "$TMP")"

msg ""
# Ok, let's go!
if [ -n "$FORCE" ] ; then
   #git checkout "$DAT/spob" "$DST"
   msg "freeze non-nempty"
   echo -e "\e[32m$(
      "$DIR"/ssys_empty.py -r "$DST"/*.xml |
      "$DIR"/ssys_freeze.py -f | wc -l)\e[0m" >&2
fi

msg "gen before graph"
# shellcheck disable=SC2002
cat "$TMP"                                                                    |
"$DIR"/graphmod_stats.py 'init'                                               |
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
"$DIR"/graphmod_stats.py 'repos'                                               |
pmsg "apply gravity"                                                           |
"$DIR"/apply_pot.sh -g                                                         |
"$DIR"/graphmod_stretch_north.py                                               |
"$DIR"/graphmod_virtual_ssys.py                                                |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_grav')
else pmsg "" ;                                                              fi |
"$DIR"/graphmod_stats.py 'grav '                                               |
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
"$DIR"/reposition -e -q -w0 "yarn" "griffin" "ngc8338"                         |
if [ -z "$NOPIC" ] ;                                                      then
   pmsg "" |
   tee >($SPOIL_FILTER | "$DIR"/graph2pov.py "${POVF[@]}" "$POVO"'map_swr')
else pmsg "";                                                               fi |
pmsg "finally"                                                                 |
if [ -n "$FORCE" ] ;                                                      then
   tee >("$DIR"/graph2ssysmap.py) >("$DIR"/decorators.py)
else cat ;                                                                  fi |
if [ ! "$NOPIC" = "1" ] ; then
   pmsg "" |
   $SPOIL_FILTER |
   tee >("$DIR"/graph2dot.py -c -k | neato -n2 -Tpng 2>/dev/null > after.png) |
   "$DIR"/graph2pov.py "${POVF[@]}" -d "$POVO"'map_fin'
else pmsg "" >/dev/null ; fi

if [ -n "$FORCE" ] ; then
   msg "relax ssys..\n"
   msg "total relaxed: \e[32m$("$DIR"/ssys_relax.py -j 4 "$DST"/*.xml | wc -l)\n"
fi

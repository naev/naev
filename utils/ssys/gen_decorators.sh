#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PIC="$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../artwork/gfx/map")"
#MAP="$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/map_decorator")"

if [ -f "decorators.inc" ] ; then
   echo '"decorators.inc" already exists!'
   exit 0
fi

for i in "$PIC"/*.webp; do
   res=$(identify -verbose "$i" | grep -m 1 'geometry:')
   if [ "$res" != "" ] ; then
      #xml="$MAP/$(basename "${i%.webp}")".xml
      bas="$(basename "${i%.webp}")"
      # shellcheck disable=SC2001
      geom=$(sed "s/^.*geometry: \([0-9]*x[0-9]*\).*$/\1/" <<< "$res")
      W="$(echo "$geom" | cut -dx -f1)"
      H="$(echo "$geom" | cut -dx -f2)"
      echo "$bas.png $W $H" >&2
      if [ ! -f "$bas".png ] || [ "$i" -nt "$bas.png" ] ; then
         convert "$i" "$bas".png
      fi
      echo -e "#declare $bas = box{\n\t<0,0,0>\n\t<1,1,-1>"
      echo -e "\tpigment{image_map{\"$bas.png\"}}"
      echo -e "\ttranslate <-0.5,-0.5,0>\n\tscale <-$W,-$H,1>\n}"
   fi
done > "decorators.inc"



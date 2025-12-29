#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PIC="$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../assets/gfx/map")"
MAP="$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/map_decorator")"

if [ -f "decorators.inc" ] ; then
   echo '"decorators.inc" already exists!' 1>&2
   exit 0
fi

mkdir -p "decorators"

grep "<image>" "$MAP"/*.xml | sed 's/^.*<image>\(.*\)<\/image>.*$/\1/' |
while read -r picnam ; do
   pic="$PIC"/"$picnam"
   bas="${picnam%.webp}"
   OUT=decorators/$bas.png
   res=$(identify -verbose "$pic" | grep -m 1 'geometry:')
   if [ "$res" != "" ] ; then
      # shellcheck disable=SC2001
      geom=$(sed "s/^.*geometry: \([0-9]*x[0-9]*\).*$/\1/" <<< "$res")
      W="$(echo "$geom" | cut -dx -f1)"
      H="$(echo "$geom" | cut -dx -f2)"
      echo -n "$OUT $W $H" >&2
      if [ ! -f "$OUT" ] || [ "$pic" -nt "$OUT" ] ; then
         convert "$pic" "$OUT"
      else
         echo -n " [was already here]" >&2
      fi
      echo >&2
      echo -e "#declare $bas = box{\n\t<0,0,0>\n\t<1,1,-1>"
      echo -e "\tpigment{image_map{\"$OUT\"}}"
      echo -e "\ttranslate <-0.5,-0.5,0>\n\tscale <-$W,-$H,1>\n}"
   fi
done > "decorators.inc"

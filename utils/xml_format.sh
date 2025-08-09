#!/bin/bash
#shellcheck disable=SC2207

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

res=0
IFS=$'\n'
ARGS=($(cat <<< "$*"))
NON_COMMENTED=($(grep -L "<!--" "${ARGS[@]}"))
NON_OUTFITS=($(grep -v '\<outfits/' <<< "${NON_COMMENTED[*]}"))
NON_SSYS=($(grep -v '\<ssys/' <<< "${NON_OUTFITS[*]}"))

{
   "$SCRIPT_DIR"/naev_xml.py -i "${NON_SSYS[@]}"
   echo -e "\tformat_xml: ${#NON_SSYS[@]} others" >&2
} & {
   OUTFITS=($(grep '\<outfits/' <<< "${NON_COMMENTED[*]}"))
   "$SCRIPT_DIR"/outfits/outfit.py -i "${OUTFITS[@]}"
   echo -e "\tformat_xml: ${#OUTFITS[@]} outfits" >&2
   SSYS=($(grep '\<ssys/' <<< "${NON_OUTFITS[*]}"))
   "$SCRIPT_DIR"/ssys/ssys.py -i "${SSYS[@]}"
   echo -e "\tformat_xml: ${#SSYS[@]} ssys" >&2
} & {
   COMMENTED=($(grep -l "<!--" "${ARGS[@]}"))
   unset IFS

   TMP="$(mktemp -d)"
   PARS="$TMP"/parsed.xml
   UNCO="$TMP"/no_commment.xml
   RES="$TMP"/res.xml
   for i in "${COMMENTED[@]}" ; do
      if grep -q '\<outfits/' <<< "$i" ; then
         "$SCRIPT_DIR"/outfits/outfit.py "$i" > "$PARS"
      elif grep -q '\<ssys/' <<< "$i" ; then
         "$SCRIPT_DIR"/ssys/ssys.py "$i" > "$PARS"
      else
         "$SCRIPT_DIR"/naev_xml.py "$i" > "$PARS"
      fi
      "$SCRIPT_DIR"/uncomment_xml.py < "$i" | sed '/^[[:space:]]*$/d' > "$UNCO"
      diff3 -m "$PARS" "$UNCO" "$i" | sed -z 's/|\{7\}.*\(=\{7\}\)/\1/' > "$RES"
      if ! grep -q  '<<<<<<<' "$RES" ; then
         mv "$RES" "$i"
      else
         mv "$RES" "$i.patch"
         echo -e "$i: \e[31mmerge conflict\e[0m, see $i.patch"
         res=1
      fi
   done
   rm -fr "$TMP"
   echo -e "\tformat_xml: ${#COMMENTED[@]} files with comments." >&2
}
wait
exit "$res"

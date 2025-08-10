#!/bin/bash

if [ "$1" = "-st" ] ; then
   ST=1
   shift
fi

#TODO: optimize the commented part (much can be done without repeated invocations)
if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ -z "$*" ]; then
   DOC=(
      "usage:  $(basename "$0") [-st] <xml_file>.."
      "  Formats the xml file provided in input."
      "  If -st is set, uses a single thread (e.g. for using with pre-commit)."
      "  Based on its path, a xml file is considered either:"
      "   - outfit: uses the outfits/outfit.py formatter."
      "     This one expects that the file defines an outfit."
      "     It understands lua_inline and in particular multicores."
      "   - ssys: uses the ssys/ssys.py formatter."
      "     This one expects that the file defines a ssys."
      "     This one expects that the file defines a ssys."
      "     It will add empty jump/spob lists (see MANDATORY_FIELDS in ssys.py)"
      "     if no such list is present."
      "   - other: uses the generic naev_xml formater."
      "     Like all the previous ones, indents the xml and fold empty tags."
      "  The 2 first formatters rely on the third one, that relies on elementTree."
      "  So far, this module does not manages comments, and they are therefore lost."
      ""
      "  We managed a workaround using diff3 and presenting the input file"
      "  as a new version (a) of its commentless version (o) and the formatted version"
      "  as a another new version (b) of (o). diff3 allows to merge."
      "  In case of conflict, the file with annotated conflicting parts of (b) and (a)"
      "  is written as <filename.xml.patch>, and a non-zero value is returned."
      ""
      "  However, this is terribly slow compared to the no-comment case, so only"
      "  commented files are processed this way."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

res=0
ARGS=("$@")
IFS=$'\n'
readarray -t NON_COMMENTED <<< "$(grep -L "<!--" "${ARGS[@]}")"
readarray -t NON_OUTFITS <<< "$(grep -v '\<outfits/' <<< "${NON_COMMENTED[*]}")"
readarray -t NON_SSYS <<< "$(grep -v '\<ssys/' <<< "${NON_OUTFITS[*]}")"

(
   readarray -t OUTFITS <<< "$(grep '\<outfits/' <<< "${NON_COMMENTED[*]}")"
   if [ -n "${OUTFITS[*]}" ] ; then
      "$SCRIPT_DIR"/outfits/outfit.py -i "${OUTFITS[@]}"
      echo -n "[${#OUTFITS[@]} outfits] " >&2
   fi

   readarray -t SSYS <<< "$(grep '\<ssys/' <<< "${NON_OUTFITS[*]}")"
   if [ -n "${SSYS[*]}" ] ; then
      "$SCRIPT_DIR"/ssys/ssys.py -i "${SSYS[@]}"
      echo -n "[${#SSYS[@]} ssys] " >&2
   fi

   if [ -n "${NON_SSYS[*]}" ] ; then
      "$SCRIPT_DIR"/naev_xml.py -i "${NON_SSYS[@]}"
      echo -n "[${#NON_SSYS[@]} others] " >&2
   fi
) & if [ -n "$ST" ] ; then wait ; fi ; (
   readarray -t COMMENTED <<< "$(grep -l "<!--" "${ARGS[@]}")"
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
         echo -e "\n$i: \e[31mmerge conflict\e[0m, see $i.patch" >&2
         res=1
      fi
   done
   rm -fr "$TMP"
   echo -n "[${#COMMENTED[@]} commented] " >&2
)
wait
echo >&2
exit "$res"

#!/usr/bin/env bash

if [ "$1" = "-st" ] ; then
   ST=1
   shift
fi

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
      "   - other: uses the generic naev_content formater."
      "     Like all the previous ones, indents the xml and fold empty tags."
      "  The 2 first formatters rely on the third one, that relies on elementTree."
      "  So far, this module does not manages comments, and they are therefore lost."
      ""
      "  We managed a workaround using diff3 and presenting the input file"
      "  as a new version (a) of its commentless version (o) and the formatted version"
      "  as a another new version (b) of (o). diff3 allows to merge."
      "  In case of conflict, the file with annotated conflicting parts of (b) and (a)"
      "  is written as <filename.xml.diff>, and a non-zero value is returned."
      ""
      "  However, this is terribly slow compared to the no-comment case, so only"
      "  commented files are processed this way."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

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
) & if [ -n "$ST" ] ; then wait ; fi ; (
   if [ -n "${NON_SSYS[*]}" ] ; then
      "$SCRIPT_DIR"/naev_content.py -i "${NON_SSYS[@]}"
      echo -n "[${#NON_SSYS[@]} others] " >&2
   fi
) & if [ -n "$ST" ] ; then wait ; fi ; (
   UNCOM_DIR="$(mktemp -d)"
   PARSE_DIR="$(mktemp -d)"
   RES="$(mktemp)"
   trap 'rm -fr "$RES" "$UNCOM_DIR" "$PARSE_DIR"' EXIT

   readarray -t COMMENTED <<< "$(grep -l "<!--" "${ARGS[@]}")"
   sed -i 's/^\([[:space:]]*\)\(.*[^[:space:]]\)[[:space:]]*\(<!--.*-->\)[[:space:]]*\(.*\)$/\1\3\n\1\2\4/' "${COMMENTED[@]}"

   readarray -t UNCOM_RES <<< "$("$SCRIPT_DIR"/xml_uncomment.py "$PWD" "$UNCOM_DIR" "${COMMENTED[@]}")"
   sed '/^[[:space:]]*$/d' -i "${UNCOM_RES[@]}"

   readarray -t OUTFITS <<< "$(grep '\<outfits/' <<< "${COMMENTED[*]}")"
   if [ -n "${OUTFITS[*]}" ] ; then
      "$SCRIPT_DIR"/outfits/outfit.py -c "$PWD" "$PARSE_DIR" "${OUTFITS[@]}"
      res1="$?"
      if [ "$res1" = 0 ] ; then
         echo -n "[${#OUTFITS[@]} comm. outfits] " >&2
      fi
   fi

   readarray -t NON_OUTFITS <<< "$(grep -v '\<outfits/' <<< "${COMMENTED[*]}")"
   readarray -t SSYS <<< "$(grep '\<ssys/' <<< "${NON_OUTFITS[*]}")"
   if [ -n "${SSYS[*]}" ] ; then
      "$SCRIPT_DIR"/ssys/ssys.py -c "$PWD" "$PARSE_DIR" "${SSYS[@]}"
      res2="$?"
      if [ "$res2" = 0 ] ; then
         echo -n "[${#SSYS[@]} comm. ssys] " >&2
      fi
   fi

   readarray -t NON_SSYS <<< "$(grep -v '\<ssys/' <<< "${NON_OUTFITS[*]}")"
   if [ -n "${NON_SSYS[*]}" ] ; then
      "$SCRIPT_DIR"/naev_content.py -c "$PWD" "$PARSE_DIR" "${NON_SSYS[@]}"
      res3="$?"
      if [ "$res3" = 0 ] ; then
         echo -n "[${#NON_SSYS[@]} comm. others] " >&2
      fi
   fi

   if [ ! "$res1" = 0 ] || [ ! "$res2" = 0 ] || [ ! "$res3" = 0 ] ; then
      exit 1
   fi
   conflicts=0
   for i in "${COMMENTED[@]}" ; do
      P="$(realpath --relative-to="$PWD" "$i")"
      UNCO="$UNCOM_DIR/""$P"
      PARS="$PARSE_DIR/""$P"
      diff3 -m "$PARS" "$UNCO" "$i" | sed -z 's/|\{7\}.*\(=\{7\}\)/\1/' > "$RES"
      if ! grep -q  '<<<<<<<' "$RES" ; then
         mv "$RES" "$i"
      else
         mv "$RES" "$i.diff"
         echo -e "\n$i: \e[31mmerge conflict\e[0m, see $i.diff" >&2
         conflicts=$((conflicts + 1))
      fi
   done
   if [ ! "$conflicts" = 0 ] ; then
      echo -e "\n[\e[31m$conflicts conflict(s)]\e[0m "
   fi
)
wait
echo >&2

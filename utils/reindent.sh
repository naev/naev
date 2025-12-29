#!/usr/bin/env bash

# shellcheck disable=SC2016
# warns that the doc examples won't expand in single quotes...

# Note. In Naev:
#  - not requiring indentation: sed/ini/toml/6/wrap/po/pot/cls/codespellignore/desktopeditorconfig/spec/.gitmodules/config.ld
#  - requiring 1-space indentation: xml/template
#     - xml checked by naev_xml.py
#     - translation.loc and entitlements.plist are xml -> naev_xml.py again
#  - requiring 3-spaces indentation but managed by specific programs:
#     - rs files are managed by rustfmt.
#     - c/h files are checked by clang-format.
#     - src/glue_macos.m is O-c, can be clang-format'ed if we pretend it is c.
#     - maybe clang-format understands frag/vert/glsl ?
#  - everything else is 3 space-indented (checked by this script)
#     -> pre-commit calls us with:
#           build clang-format dot frag glsl gltf js lua luacheckrc
#           markdown md nsi py pov rb scm sh tex vdf vert yaml yml
#     -> pre-commit does not call (but could)
#        css scss
# Ignored:
#  - .in files do not refer to a specific type
#  - .txt are of misc origin
#  - .html/.its/ldoc.ltp are indented in various ways. xml_format can't format them.
#
# Unmanaged so far:
#  - md/markdown/yaml/yml are a bit weird, but basically 3-s indented,
#    if we consider the space before an item part of it.
#    However, that would require a specific processing.

#TODO: manage "*.md" "*.markdown" "*.yaml" "*.yml" "*.clang-format"


if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ -z "$*" ]; then
   DOC=(
      "usage  $(basename "$0") [ -l | -b | -a ] <file>.."
      '  Checks its arguments are 3-space-indented.'
      '  Binary files are silently ignored.'
      '   - If not, checks is they are k-space indented with k>3.'
      '     k is guessed based on the number of occurences of small indents found in the file.'
      '      - If so, replaces blocks of k spaces with blocks of 3 spaces.'
      '      - If not, complains angrily (in red) and returns nonzero.'
      '  In the whole process:'
      '   - lines beginning with "<" are ignored (to exclude embedded xml).'
      '   - lines beginning with "-" are ignored (to exclude lists in comments'
      '     and minus sign in aligned number lists.)'
      '   - lines beginning with "*" are ignored (to ignore some comments in frag files)'
      '   - lines beginning with "#" are ignored (because)'
      ''
      '  If -b is set, show 3 erroneous lines, not 10.'
      '  If -a is set, show all erroneous lines.'
      ''
      '  If -l is set, only list files that require some fixing.'
      'example'
      ' > readarray -t FILES <<< "$(git ls-files -- "**.py")"'
      ' > ./utils/reindent.sh "${FILES[@]}'
      ' > vim $(./utils/reindent.sh -l "${FILES[@]})'
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

GREPL=('grep' '-m10')
while true ; do
   if [ "$1" = "-l" ] ; then
      LIST_ONLY='y'
      LIST=()
   elif [ "$1" = "-a" ] ; then
      GREPL=('grep')
   elif [ "$1" = "-b" ] ; then
      GREPL=('grep' '-m3')
   else
      break
   fi
   shift
done

readarray -t FILES <<< "$( grep -I -vl -e '^$' -e '^ *\(<\|-\|\*\|#\)' -e '^\(   \)*[^ ]' "$@" )"

if [ -z "${FILES[*]}" ] ; then exit 0 ; fi

TMP=$(mktemp)
trap 'rm "$TMP"' EXIT
ret=0
for f in "${FILES[@]}" ; do
   IFS=$'\t'
   MAX=0
   PAT="$(
      sed -e "/^ *$/d" -e "/^[^ ]/d" -e '/^ *\(<\|-\|\*\|\#\)/d' -e 's/^\( *\)[^ ].*$/\1/' "$f" |
      sort | tee "$TMP" | uniq -c | sed 's/^ *\([1-9][0-9]*\) /\1\t/' |
      while read -r NOCC BPAT ; do
         if [ "$MAX" = "0" ] ; then
            LIM="$BPAT""$BPAT"
         elif [ "${#BPAT}" -ge "${#LIM}" ] ; then
            break
         fi
         if [ "$NOCC" -gt "$MAX" ] ; then
            MAX="$NOCC"
            echo "$BPAT"
         fi
      done | tail -n1
   )"
   N=${#PAT}

   if [ "$N" -le 2 ] && grep -q -e $'\t' -e "[^ ]$PAT" "$f" ; then
      if [ -n "${PAT}" ] ; then
         if [ -z "$LIST_ONLY" ] ; then
            ret=1
            echo -e '\e[31m"'"$f"'":\e[0m minimal indent too small ('"\e[36m$N\e[0m"')' >&2

            export GREP_COLORS="ms=41"
            "${GREPL[@]}" -n --color=always -e '^'"$PAT"'[^ <*#-]' "$f" >&2
         else
            LIST+=("$f")
         fi
      fi
   elif grep -q -v '^\('"$PAT"'\)*$' "$TMP" ; then
      if [ -z "$LIST_ONLY" ] ; then
         ret=1
         echo -e '\e[31m"'"$f"'":\e[0m not properly indented (be consistent) (size '"\e[36m$N\e[0m"')' >&2
         DOTS='.'
         for (( i=1; i<N ; i++ )); do
            DOTS="$DOTS"'.\?'
         done
         export GREP_COLORS="ms=31"
         "${GREPL[@]}" --color=always -n -v -e '^$' -e '^ *\(<\|-\|\*\)' -e '^\('"$PAT"'\)*[^ ]' "$f" |
         sed 's/^\([^ ]*\('"$PAT"'\)*\)\('"$DOTS"'\)\(.*\)$/\1'$'\e''\[44m\3'$'\e''\[0m\4/' >&2
      else
         LIST+=("$f")
      fi
   else
      if [ -z "$LIST_ONLY" ] ; then
         echo -e '\e[33m"'"$f"'":\e[0m'" tab len $N -> 3" >&2
         sed 's/'"$PAT"'/   /g' -i "$f"
      fi
   fi
done
if [ -n "$LIST_ONLY" ] ; then
   echo "${LIST[@]}"
fi
exit "$ret"

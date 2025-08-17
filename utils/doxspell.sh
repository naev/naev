#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PERS="$SCRIPT_DIR/naev.aspell.pws"
PERS_U="$SCRIPT_DIR/naev_ugly.aspell.pws"

filter() {
   grep -v -e '@file\>' -e '@luamod\>' -e '@luafunc\>' -e '@\(lua\)\?note\>' -e '@\(lua\)\?see\>' -e '@struct\>' -e '@sa\>' -e '@endcode\>' |
   sed 's/"[^ ]*"//g' |
   sed 's/\(@usage\)\(\([^-]\)\|\(-[^-]\)\)*\(--.*\)\?/\5/' |
   sed 's/@param\(\[[^]]*\]\)\? *[^ ]* *//' |
   sed 's/@luaparam\(\[[^]]*\]\)\? [^ ]* *//' |
   sed 's/@luatparam\(\[[^]]*\]\)\? [^ ]* \(\w\||\)* *//' |
   sed 's/\\\(see\|ref\|p\) [^ ]*//' |
   sed 's/@luatreturn\(\[[^]]*\]\)\?\( [^ ]*\)\? *//' |
   sed 's/@return\(\[[^]]*\]\)\? \(\w\||\)* *//' |
   sed 's/@luareturn\(\[[^]]*\]\)\? *//'
}

readarray -t WORDS <<< "$(
   grep -h '^ *\* *@' "$@"                                           |
   filter                                                            |
   aspell list -l en_US --personal "$PERS" --extra-dicts "$PERS_U"   |
    sort -u
   #sed 's/\([a-z]\)\([A-Z]\)/\1 \2/g'
)"

echo "${WORDS[@]}" >&2
MARK="33;44;1"
E=$'\e'

EXPR="$(sed 's/ /\\)\\|\\(/g' <<< "${WORDS[*]}")"
if [ -z "${EXPR[*]}" ] ; then
   exit 0
fi
EXPR='\(\(\<\|[^a-zA-Z]\)\(\('"$EXPR"'\)\)\(\>\|[^a-zA-Z]\)\)'

readarray -t FILES <<< "$(grep -l '^ *\* *@.*'"$EXPR"'' "$@")"

if [ -z "${FILES[*]}" ] ; then
   exit 0
fi
echo "$# -> ${#FILES[@]}" >&2

(
   export GREP_COLORS="ms="
   grep --color=always -H -n '^ *\* *@' "${FILES[@]}"
) |
sed 's/^\([^ ]*\) *\* *\(.*\)$/\1 \2/'       |
filter                                       |
sed 's/\([^ ]*\) *@\w* /\1 /'                |
(
   export GREP_COLORS="ms=$MARK"
   grep --color=always "$EXPR" |
   sed 's/^\([^:]*\)'"$E"'\['"$MARK"'m'"$E"'\[K\([^'"$E"']*\)'"$E"'\[m/\1\2/' |
   if grep "$E\[$MARK"'m' ; then
      exit 1
   fi
)

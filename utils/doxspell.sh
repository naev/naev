#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PERS="$SCRIPT_DIR/naev.aspell.pws"
PERS_U="$SCRIPT_DIR/naev_ugly.aspell.pws"

if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ -z "$*" ]; then
   DOC=(
      "usage  $(basename "$0") <file.c>.."
      "Apply aspell to the doxygen strings contained in c files provided in input."
      ""
      "When you get a warning, you have 4 options:"
      " - fix the spelling if possible."
      " - add the word to the legitimate list '/utils/naev.aspell.pws' if it is a legitimate English form or a name belonging to the Naev universe."
      " - protect your word with double-quotes, (similar in meaning to backquotes in md: for quoting code). This currently works only for a non-space sequences."
      " - add the word to the semi-legitimate list '/utils/naev.aspell_ugly.pws' in the remaining cases."
      ""
      "Beware the capitals. 'Naev' is legitimate, 'naev' is not."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

# !!! don't give it more than 9 args.
# (because sed groups go from \1 to \9)
mk_expr() {
   SUFF=$1
   OFF=$2
   shift 2
   ARGS="$*"
   ARG_EXPR='s/\(\('"${ARGS// /\\)\\|\\(}"'\)\)'
   #shellcheck disable=SC2028
   echo "$ARG_EXPR""$SUFF"' \([^ ]*\)/\1 `'"\\$(($# + 2 + OFF))"'`/g'
}

PROTECT_ARG_1="$(mk_expr '' 0 '@luafunc' '@file' '@struct' '@sa' '@luamod' '@typedef' '@luasee' )"
PROTECT_ARG_2="$(mk_expr '' 0 '\\see' '\\ref' '\\p' '@c' '@see' )"
SUB='\(\[[^]]*\]\)\?'
PARAM_ARG="$(mk_expr "$SUB" 1 '@param' '@luaparam' '@luatreturn')"
TYP='\(\w\||\)*'
filter() {
   sed                                                      \
   -e "$PROTECT_ARG_1" -e "$PROTECT_ARG_2"                  \
   -e "$PARAM_ARG"                                          \
   `#TODO: protect the args instead of killing everything.` \
   -e 's/\(@usage\)\(\([^-]\)\|\(-[^-]\)\)*\(--.*\)\?/\5/'  \
   -e 's/@luatparam'"$SUB"' [^ ]* '"$TYP"' *//'             \
   -e 's/@return'"$SUB"' '"$TYP"' *//'
}

SEP=" ?,;.:/!:"
NSEPNW="[^a-zA-Z$SEP]"
DOXTRACT="$("$SCRIPT_DIR"/get_doxtractor.sh)"
readarray -t WORDS <<< "$(
   "$DOXTRACT" "$@" | cut '-d ' -f 3-                                |
   filter                                                            |
   sed                     \
    -e "s/\"[^\"]*\"//g"   \
    -e "s/\`[^\`]*\`//g"   \
    -e 's/@[^ ]*//g'       \
    -e 's/\w*\('"$NSEPNW"'\)\w*/\1/g'                                |
   aspell list -l en_US --personal "$PERS" --extra-dicts "$PERS_U"   |
   sort -u
   #sed 's/\([a-z]\)\([A-Z]\)/\1 \2/g'
)"

MARK="33;44;1"
E=$'\e'

WS="${WORDS[*]}"
EXPR="${WS// /\\)\\|\\(}"

if [ -z "${EXPR[*]}" ] ; then
   exit 0
fi

echo "${WORDS[@]}" >&2

SEPE="[$SEP]"
EXPR='\(\(^\|'"$SEPE"'\)\(\('"$EXPR"'\)\)\($\|'"$SEPE"'\)\)'

readarray -t FILES <<< "$(grep -l '^ *\* *.*'"$EXPR"'' "$@")"

if [ -z "${FILES[*]}" ] ; then
   exit 0
fi
echo "$# -> ${#FILES[@]}" >&2

TMP=$(mktemp -u)
mkfifo "$TMP"
trap 'rm "$TMP"' EXIT

#shellcheck disable=SC2016
"$DOXTRACT" "${FILES[@]}"                          |
grep -v -e '^$' -e '^[^:]*:$'                      |
tee >(cut '-d:' -f1 > "$TMP")                      |
cut '-d:' -f2-                                     |
filter                                             |
#substitute ' ' with '_' inside a `...` / "..." expr
sed ':loop; s/\([^`]*\)\(`[^ `]*\) \([^`]*`\)/\1\2_\3/; t loop' |
sed ':loop; s/\([^"]*\)\("[^ "]*\) \([^"]*"\)/\1\2_\3/; t loop' |
sed 's/'"$EXPR"'/'"$E"'['"$MARK"'m\1'"$E"'[0m/g'   |
paste '-d:' "$TMP" -                               |
grep "$E"                                          |
# Mimic grep's colors
sed 's/^\([^ ]*\) \([^:]*\):/'"$E"'[35m\1 '"$E"'[32m\2'"$E"'[36m:'"$E"'[m/' >&2
exit 1

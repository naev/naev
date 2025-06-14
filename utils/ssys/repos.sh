#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0") [#iterations] [reposition args]" >&2
   echo "  Applies reposition repeatedly (or once if not provided)" >&2
   exit 0
fi

#FLAGS=("-pg" "-O1")
FLAGS=("-O3")

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
   N="$1"
   shift
else
   N=1
fi
echo "Apply reposition $N time(s)." >&2

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
read -ra FLG <<< "$(pkg-config glib-2.0 --cflags)"
read -ra LIB <<< "$(pkg-config glib-2.0 --libs)"

if [ ! -f "$SCRIPT_DIR"/reposition ] || [ ! "$SCRIPT_DIR"/reposition -nt "$SCRIPT_DIR"/reposition.c ] || [ ! "$SCRIPT_DIR"/reposition -nt "$SCRIPT_DIR"/repos.sh ] ; then
   echo -n 'Compile.. ' >&2
   gcc -Wall -Wextra -Werror -Og "${FLAGS[@]}" "$SCRIPT_DIR"/reposition.c -o "$SCRIPT_DIR"/reposition\
      -I/usr/include/glib-2.0 \
      "${FLG[@]}"             \
      -lm "${LIB[@]}"         \
   || exit 1
   echo >&2
fi

TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

repeat() {
   local -i n="$1";
   n="$((n-1))"
   shift;
   cat - "$TMP" | "$SCRIPT_DIR"/reposition "$@" |
   (
      if (( n )); then
         repeat "$n" "$@"
      else
         cat
      fi
   )
}

"$SCRIPT_DIR"/ssys_pos.sh |
(
   if [ "$N" -gt "0" ]; then
      cat - <("$SCRIPT_DIR"/ssys_edges.sh | tee "$TMP") |
      "$SCRIPT_DIR"/reposition "$@" |
      (if (( N-1 )); then repeat "$((N-1))" "$@"; else cat; fi)
   else
      cat
   fi
)

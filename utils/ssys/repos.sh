#!/usr/bin/bash

#FLAGS=("-pg" "-O1")
FLAGS=("-Ofast")

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0") [<#iterations>|-C] [reposition args]" >&2
   echo "  If -C is set, just compile reposition." >&2
   echo "  Applies reposition <#iterations> times (or once if not provided)" >&2
   echo "  <#iterations> might be 0 (just output current ssys positions).">&2
   echo "  Use reposition -h to get info on reposition args." >&2
   echo "  reposition args -e and -o are managed even if <#iterations> is 0." >&2
   exit 0
fi

if [ -n "$1" ] && [ "$1" -eq "$1" ] 2>/dev/null; then
   N="$1"
   shift
else
   N=1
   for j in "$@"; do
      if [ "$j" = "-C" ]; then
         N="C";
         break
      fi
   done
fi

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

if [ "$N" = "C" ] ; then
   exit 0
fi
echo "Apply reposition $N time(s)." >&2

TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

repeat() {
   local -i n="$1";
   n="$((n-1))"
   shift;
   cat - "$TMP" | "$SCRIPT_DIR"/reposition -q "$@" |
   (
      if (( n )); then
         repeat "$n" "$@"
      else
         cat
      fi
   )
}

"$SCRIPT_DIR"/ssys_graph.sh -v |
(
   if [ "$N" -gt "0" ]; then
      cat - <("$SCRIPT_DIR"/ssys_graph.sh -e | tee "$TMP") |
      "$SCRIPT_DIR"/reposition -q "$@" |
      (if (( N-1 )); then repeat "$((N-1))" "$@"; else cat; fi)
   elif [ ! "$1" = "-o" ] ; then
      cat
      if [ "$1" = "-e" ] ; then
         "$SCRIPT_DIR"/ssys_graph.sh -e
      fi
   fi
)

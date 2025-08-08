#!/bin/bash

#FLAGS=("-pg" "-O1")
FLAGS=("-Ofast")

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") [<#iterations>|-C] [reposition args]"
      "  If -C is set, just compile reposition."
      "  Applies reposition <#iterations> times (or once if not provided)"
      "  <#iterations> might be 0 (just output current ssys positions)."
      "  Use reposition -h to get info on reposition args."
      "  reposition arg -o is managed even if <#iterations> is 0."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
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
if [ ! -f "$SCRIPT_DIR"/reposition ] || [ ! "$SCRIPT_DIR"/reposition -nt "$SCRIPT_DIR"/reposition.c ] || [ ! "$SCRIPT_DIR"/reposition -nt "$SCRIPT_DIR"/repos.sh ] ; then
   echo -n 'compile reposition.. ' >&2

   read -ra FLG <<< "$(pkg-config glib-2.0 --cflags)"
   read -ra LIB <<< "$(pkg-config glib-2.0 --libs)"
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

ARGS="$*"
"$SCRIPT_DIR"/repeat.sh "$N" "$SCRIPT_DIR"/det_reposition.sh -e -q "$ARGS"

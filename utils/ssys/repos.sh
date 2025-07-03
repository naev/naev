#!/usr/bin/bash

#FLAGS=("-pg" "-O1")
FLAGS=("-Ofast")

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   (
      echo "usage:  $(basename "$0") [<#iterations>|-C] [reposition args]"
      echo "  If -C is set, just compile reposition."
      echo "  Applies reposition <#iterations> times (or once if not provided)"
      echo "  <#iterations> might be 0 (just output current ssys positions)."
      echo "  Use reposition -h to get info on reposition args."
      echo "  reposition arg -o is managed even if <#iterations> is 0."
   ) >&2
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

repeat() {
   local -i n="$1";
   shift;
   if [ ! "$n" = "0" ] ; then
      n="$((n - 1))"
      "$SCRIPT_DIR"/reposition "$@" | repeat "$n" "$@"
   elif [ ! "$1" = "-o" ] ; then
      cat
   fi
}

repeat "$N" -e -q "$@"

#!/usr/bin/bash

OPS=("-g" "-E1" "-E2" "-w" "-C")


for i in "$@" ; do
   if [ "$i" = "--help" ] || [ "$i" = "-h" ] ; then
      HELP=1
   else
      for j in "${OPS[@]}" ; do
         if [ "$i" = "$j" ] ; then
            if [ -n "$OP" ] ; then
               echo "incompatible $OP and $i." >&2
               OP="ERR"
               break
            else
               OP="$i"
            fi
         fi
      done
      if [ -z "$OP" ] ; then
         echo "Ignored: $i"
      fi
      if [ "$OP" = "ERR" ] ; then
         break
      fi
   fi
done

if [ -z "$OP" ] ; then
   echo "  Either -C -E1, -E2, -g, or -w must be selected." >&2
   OP="ERR"
fi

if [ "$OP" = "-g" ] || [ "$OP" = "ERR" ] ; then
   N=20
   RESCALE=1.8
   POST_RESCALE=1.1
elif [ "$OP" = "-E2" ] ; then
   N=20
   RESCALE=1.5
   POST_RESCALE=1.1
elif [ "$OP" = "-E1" ] ; then
   N=20
   RESCALE=0.95
   POST_RESCALE=1.5
elif [ "$OP" = "-w" ] ; then
   N=20
   RESCALE=1.0
   POST_RESCALE=1.0
elif [ ! "$1" = "-C" ] ; then
   exit 1
fi

if [ -n "$HELP" ] ; then
   DOC=(
      "usage:  $(basename "$0") -C |  -E(1|2) | -g | -w"
      "  If -C is set, just compile potential."
      "  Else, applies potential $N times and rescaling x$RESCALE"
      "  to input graph with current flags."
      "  Output the positions of systems in the same form as ssys2graph.sh."
      ""
      "  -g stands for gravity; -w for waves. (-E<n> for experimental grav.)"
      "  See potential -h for more information."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" )>&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ ! -f "$SCRIPT_DIR"/potential ] || [ ! "$SCRIPT_DIR"/potential -nt "$SCRIPT_DIR"/potential.c ] ; then
   echo -n 'compile potential.. ' >&2
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/potential.c -o "$SCRIPT_DIR"/potential -lm || exit 1
   echo >&2
fi

for j in "$@"; do
   if [ "$j" = "-C" ]; then exit 0; fi
done

"$SCRIPT_DIR"/graph_scale.py "$RESCALE" |
"$SCRIPT_DIR"/repeat.sh "$N" "$SCRIPT_DIR"/potential -a "$OP" |
"$SCRIPT_DIR"/graph_scale.py "$POST_RESCALE"

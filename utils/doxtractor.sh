#!/bin/bash

if [ -z "$*" ]; then args=( "-" ); else args=( "$@" ); fi

sed -s -n -f <(cat <<EOF
:init
   s/^\( *\)\/\*@/\1\n/;t enter
   b

:enter
   h; x; s/\n.*//; x
   s/ *\n//
   t ok
:ok
   s/*\//\n/
   /^ *[^ \n]/{F; =; P}
   t end
:cont
   n; G
   # <indent>*/
   /^\( *\) \*\/\([^\n]*\)\n\1$/{b end}
   # <indent>*
   s/^\( *\) \*\( \([^\n]*\)\)\?\n\1$/\3/; t ok
   # else
   s/\n.*$//
   s/^/invalid line: /
   w /dev/stderr
   s/^invalid line: //
   t ok

:end
   i
   D
EOF
) "${args[@]}" |
sed "/^$/b; N; s/\n/ /; N; s/\n/: /"

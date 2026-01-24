#!/usr/bin/env bash


if [ "$1" = "" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") <names>.."
      "  Output all ssys names but the ones in argument."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../ssys")" &&
for i in *.xml; do
   echo "$i"
done | grep -v -F -f <( for i in "$@"; do echo "$i"; done ) | sed 's/.xml$//' | tr '\n' ' '

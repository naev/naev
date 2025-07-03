#!/usr/bin/bash


if [ "$1" = "" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0") <names>.." >&2
   echo "  Output all ssys names but the ones in argument." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&
for i in *.xml; do
   echo "$i"
done | grep -v -F -f <( for i in "$@"; do echo "$i"; done ) | sed 's/.xml$//' | tr '\n' ' '

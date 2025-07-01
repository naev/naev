#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  like apply_pot.sh" >&2
   echo "  The only diff is that output is applied to ssys." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

"$SCRIPT_DIR"/apply_pot.sh "$1" | "$SCRIPT_DIR"/graph2ssys.py

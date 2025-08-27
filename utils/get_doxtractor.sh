#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR/../subprojects/doxtractor" || exit
if [ ! -f builddir/doxtractor ] ; then
   mkdir -p builddir
   gcc -O1 -Wall -Wextra -o builddir/doxtractor doxtractor.c
fi
if [ ! -f builddir/doxtractor ] ; then
   (
      meson setup builddir .
      cd builddir || exit
      meson compile
   )
fi
echo "$(pwd)/builddir/doxtractor"

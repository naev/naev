#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR/../subprojects/doxtractor" || exit
if [ ! -f builddir/doxtractor ] ; then
   mkdir -p builddir
   gcc -O1 -Wall -Wextra -o builddir/doxtractor doxtractor.c
fi
if [ ! -f builddir/doxtractor ] ; then
   (
      meson setup builddir . >/dev/null
      cd builddir || exit
      meson compile >/dev/null
   )
fi
if [ ! -f builddir/doxtractor ] ; then
   echo ":"
   exit 1
else
   echo "$(pwd)/builddir/doxtractor"
fi

#!/bin/sh

#clang-format -i "$@" *should work*, but does not.
for i in "$@" ; do
   clang-format -i "$i"
done

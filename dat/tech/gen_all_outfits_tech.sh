#!/bin/bash

ARG=$1
shift
(
   echo  '<tech name="All Outfits">'
   grep -m1 -ho '<outfit name=\"[^"]*\"' "$@" |
   sed 's/^<outfit name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$ARG"

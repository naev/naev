#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ -z "$*" ] ; then
   ARGS=( "-r" "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")" )
else
   ARGS=( "$@" )
fi

readarray -t FILES <<< "$(grep -L '^<?xml version' "${ARGS[@]}" --include "*.xml")"
if [ -n "${FILES[*]}" ] ; then
   sed -i '1 i\<?xml version="1.0" encoding="UTF-8"?>' "${FILES[@]}"
fi

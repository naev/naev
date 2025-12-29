#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ -z "$*" ] ; then
   DAT=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")
   ARGS=( "-r" "$DAT/spob" "$DAT/ssys" )
else
   ARGS=( "$@" )
   IFS=$'\n'
fi

readarray -t FILES <<< "$(grep -l '^<?xml version' "${ARGS[@]}" --include "*.xml")"
if [ -n "${FILES[*]}" ] ; then
   sed '0,/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)\(\(utf\)\|\(UTF\)\)-\?8\2?>$/{//d}' -i "${FILES[@]}"
fi

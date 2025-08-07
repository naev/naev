#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

#shellcheck disable=SC2207
NON_COMMENTED=($(grep -L "<!--" "$@"))

#shellcheck disable=SC2207
OUTFITS=($(grep -l '^[[:space:]]*<outfit ' "${NON_COMMENTED[@]}"))
"$SCRIPT_DIR"/outfits/outfit.py "${OUTFITS[@]}"

#shellcheck disable=SC2207
NON_OUTFITS=($(grep -L '^<outfit ' "${NON_COMMENTED[@]}"))
"$SCRIPT_DIR"/naev_xml.py "${NON_OUTFITS[@]}"

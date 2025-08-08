#!/bin/bash
#shellcheck disable=SC2207

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

NON_COMMENTED=($(grep -L "<!--" "$@"))

OUTFITS=($(grep -l '^[[:space:]]*<outfit ' "${NON_COMMENTED[@]}"))
"$SCRIPT_DIR"/outfits/outfit.py "${OUTFITS[@]}"

NON_OUTFITS=($(grep -L '^<outfit ' "${NON_COMMENTED[@]}"))
SSYS=($(grep -l '^[[:space:]]*<ssys ' "${NON_OUTFITS[@]}"))
"$SCRIPT_DIR"/ssys/ssys.py "${SSYS[@]}"

NON_SSYS=($(grep -L '^[[:space:]]*<ssys ' "${NON_OUTFITS[@]}"))
"$SCRIPT_DIR"/naev_xml.py "${NON_SSYS[@]}"

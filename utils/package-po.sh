#!/bin/bash
# PACKAGING SCRIPT SUBROUTINE: Copy the outputs of Meson's "naev-gmo" build target to the given staging area's
# Pass in -b <BUILDPATH> (Sets location of build directory) -o <STAGING> (Parent directory of dat/)

set -e

# Defaults
BUILDPATH="$(pwd)/build"
STAGING="$(pwd)/dist"

while getopts dns:b:o: OPTION "$@"; do
    case $OPTION in
    b)
        BUILDPATH="${OPTARG}"
        ;;
    o)
        STAGING="${OPTARG}"
        ;;
    esac
done

echo "Checking for Meson-built translations"
if compgen -G "${BUILDPATH}/po/*.gmo" > /dev/null; then
    for MO_PATH in "${BUILDPATH}"/po/*.gmo; do
        MO_NAME="$(basename "${MO_PATH}")"
        LANG=${MO_NAME%.gmo}
        mkdir -p "$STAGING"/dat/gettext/$LANG/LC_MESSAGES
        cp -v "${MO_PATH}" "$STAGING"/dat/gettext/$LANG/LC_MESSAGES/naev.mo
    done
    grep -q '%<PRI' "${BUILDPATH}/po/*.gmo" && echo "***WARNING: 'sysdep' strings like %<PRIu64> cannot be translated. Try %.0f?"
fi

#!/bin/bash
# RELEASE SCRIPT FOR NAEV
#
# This script attempts to compile and build different parts of Naev
# automatically in order to prepare for a new release. Files will  be written
# to the "dist/" directory.
#
# Steam sdk should be unpacked, set up, and named "steam/" in the naev directory.

if [[ ! -f "naev.6" ]]; then
   echo "Please run from Naev root directory."
   exit -1
fi

NAEVDIR="$(pwd)"
OUTPUTDIR="${NAEVDIR}/dist/"
STEAMPATH="${NAEVDIR}/steam/tools/linux/"
CFLAGS="-j5"

function get_version {
   VERSION="$(cat ${NAEVDIR}/VERSION)"
   # Get version, negative minors mean betas
   if [[ -n $(echo "${VERSION}" | grep "-") ]]; then
      BASEVER=$(echo "${VERSION}" | sed 's/\.-.*//')
      BETAVER=$(echo "${VERSION}" | sed 's/.*-//')
      VERSION="${BASEVER}.0-beta${BETAVER}"
   fi
}

function make_generic {
   make distclean
   ./autogen.sh
   ./configure $1
   make ${CFLAGS}
   get_version
   mv src/naev "${OUTPUTDIR}/naev-${VERSION}-$2"
}

function make_linux_64 {
   make_generic "--enable-lua=internal" "linux-x86-64"
}

function make_linux_steam_64 {
   ${STEAMPATH}/shell-amd64.sh "${NAEVDIR}/extras/steam-compile.sh"
   get_version
   mv src/naev "${OUTPUTDIR}/naev-${VERSION}-linux-steam-x86-64"
}

# Create output dirdectory if necessary
test -d "$OUTPUTDIR" || mkdir "$OUTPUTDIR"

# Source
./autogen.sh
./configure
make distclean
make dist-bzip2
get_version
mv "naev-${VERSION}.tar.bz2" "dist/"

# Compile shit
make_linux_64
make_linux_steam_64

# Ndata
get_version
make "ndata.zip"
mv "ndata.zip" "${OUTPUTDIR}/ndata-${VERSION}.zip"



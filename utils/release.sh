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
LOGFILE="release.log"
THREADS="-j$(nproc --all)"

COMPILED=""
FAILED=""
SKIPPED=""

function log {
   echo
   echo
   echo "====================================="
   echo "$1"
   echo "====================================="
   return 0
}

function get_version {
   VERSION="$(cat ${NAEVDIR}/VERSION)"
   # Get version, negative minors mean betas
   if [[ -n $(echo "${VERSION}" | grep "-") ]]; then
      BASEVER=$(echo "${VERSION}" | sed 's/\.-.*//')
      BETAVER=$(echo "${VERSION}" | sed 's/.*-//')
      VERSION="${BASEVER}.0-beta${BETAVER}"
   fi
   return 0
}

function make_generic {
   log "Compiling $2"
   make distclean
   ./autogen.sh
   ./configure $1
   make ${THREADS}
   get_version
   if [[ -f src/naev ]]; then
      mv src/naev "${OUTPUTDIR}/naev-${VERSION}-$2"
      COMPILED="$COMPILED $2"
      return 0
   else
      FAILED="$FAILED $2"
      return 1
   fi
}

function make_win32 {
   # Openal isabled due to issues while compiling.. not sure what is up.
   make_generic "--host=i686-w64-mingw32.static --enable-lua=internal --with-openal=no" "win32"
}

function make_win64 {
   make_generic "--host=x86_64-w64-mingw32.static --enable-lua=internal" "win64"
}

function make_linux_64 {
   make_generic "--enable-lua=internal" "linux-x86-64"
}

function make_linux_steam_64 {
   if [[ ! -d "${STEAMPATH}" ]]; then
      log "Skipping linux-steam-x86-64"
      SKIPPED+=( "linux-steam-x86-64" )
      return 2
   fi
   log "Compiling linux-steam-x86-64"
   TMPPATH="/tmp/naev_steam_compile.sh"
   echo "#!/bin/bash" > "${TMPPATH}"
   echo "make distclean" >> "${TMPPATH}"
   echo "./autogen.sh" >> "${TMPPATH}"
   echo "./configure --enable-lua=internal --without-libzip CFLAGS=-std=gnu99" >> "${TMPPATH}"
   echo "make ${THREADS}" >> "${TMPPATH}"
   chmod +x "${TMPPATH}"
   ${STEAMPATH}/shell-amd64.sh "${TMPPATH}"
   get_version
   if [[ -f src/naev ]]; then
      mv src/naev "${OUTPUTDIR}/naev-${VERSION}-linux-steam-x86-64"
      COMPILED="$COMPILED linux-steam-x86-64"
      return 0
   else
      FAILED="$FAILED linux-steam-x86-64"
      return 1
   fi
}

function make_source {
   log "Making source bzip2"
   VERSIONRAW="$(cat ${NAEVDIR}/VERSION)"
   make dist-bzip2
   if [[ -f "naev-${VERSIONRAW}.tar.bz2" ]]; then
      get_version
      mv "naev-${VERSIONRAW}.tar.bz2" "dist/naev-${VERSION}-source.tar.bz2"
      COMPILED="$COMPILED source"
      return 0
   else
      FAILED="$FAILED source"
      return 1
   fi
}

function make_ndata {
   log "Making ndata"
   get_version
   make "ndata.zip"
   if [[ -f "ndata.zip" ]]; then
      mv "ndata.zip" "${OUTPUTDIR}/ndata-${VERSION}.zip"
      COMPILED="$COMPILED ndata"
      return 0
   else
      FAILED="$FAILED ndata"
      return 1
   fi
}

# Create output dirdectory if necessary
test -d "${OUTPUTDIR}" || mkdir "${OUTPUTDIR}"

# Set up log
rm -f "${LOGFILE}"
touch "${LOGFILE}"

# Preparation
make distclean
./autogen.sh
./configure --enable-lua=internal
make VERSION

# Make stuff
make_source          >> "${LOGFILE}" 2>&1
make_ndata           >> "${LOGFILE}" 2>&1
make_win32           >> "${LOGFILE}" 2>&1
make_win64           >> "${LOGFILE}" 2>&1
make_linux_64        >> "${LOGFILE}" 2>&1
make_linux_steam_64  >> "${LOGFILE}" 2>&1

log "COMPILED"
for i in ${COMPILED[@]}; do echo "$i"; done

log "SKIPPED"
for i in ${SKIPPED[@]}; do echo "$i"; done

log "FAILED"
for i in ${FAILED[@]}; do echo "$i"; done

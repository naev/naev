#!/bin/bash
# Run the *.o filetype detection logic from libbacktrace/configure.ac. Meson passes the compiler command line as arguments.
set -euo pipefail  # Let's not do a https://github.com/valvesoftware/steam-for-linux/issues/3671
TEMPDIR="$(mktemp -d)"
echo 'void x(){}' > "${TEMPDIR}/conftest.c"
"$@" -c -o "${TEMPDIR}/conftest.o" "${TEMPDIR}/conftest.c"
LC_ALL=C awk -f "${0/filetype.sh/filetype.awk}" "${TEMPDIR}/conftest.o"
rm -rf "${TEMPDIR}"

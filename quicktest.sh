#!/bin/sh
# Very quickly compile with meson, launch for testing, and return. For
# use with git bisect. (Should be used only for Naev specifically.)

cd builddir
meson compile
cd ..
./builddir/naev

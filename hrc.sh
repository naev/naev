#!/bin/sh
# Hard recompile - completely clean everything up and re-build. For use
# with git bisect.

git clean -fdx
meson setup builddir .
cd builddir
meson compile

![Nightly Release](https://github.com/naev/naev/workflows/Nightly%20Release/badge.svg) ![CI](https://github.com/naev/naev/workflows/CI/badge.svg)
# NAEV README

Naev is a 2D space trading and combat game, taking inspiration from the Escape
Velocity series, among others.

You pilot a space ship from a top-down perspective, and are more or less free
to do what you want. As the genre name implies, you’re able to trade and engage
in combat at will. Beyond that, there’s an ever-growing number of storyline
missions, equipment, and ships; Even the galaxy itself grows larger with each
release. For the literarily-inclined, there are large amounts of lore
accompanying everything from planets to equipment.

## DEPENDENCIES

Naev's dependencies are intended to be relatively common. In addition to
an OpenGL-capable graphics card and driver, Naev requires the following:
* SDL 2
* libxml2
* freetype2
* libpng
* OpenAL
* libvorbis
* binutils
* intltool


### Ubuntu

Install compile-time dependencies on Ubuntu 16.04 (and hopefully later) with:

```
apt-get install build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev \
libgl1-mesa-dev libxml2-dev libfreetype6-dev libpng-dev libopenal-dev \
libvorbis-dev binutils-dev libiberty-dev autopoint intltool libfontconfig-dev \
python3-pip
pip3 install meson ninja
```

### Other \*nix

See https://github.com/naev/naev/wiki/Compiling-on-*nix for package lists for several
distributions.

## COMPILING

Run:

```
bash
meson setup builddir .
cd builddir
meson compile
./naev.sh
```

If you need special settings you can run `meson configure` in your build
directory to see a list of all available options.

## INSTALLATION

Naev currently supports `meson install` which will install everything that
is needed.

If you wish to create a .desktop for your desktop environment, logos
from 16x16 to 256x256 can be found in `extras/logos/`.

## WINDOWS

See https://github.com/naev/naev/wiki/Compiling-on-Windows for how to compile on windows.

## UPDATING PO FILES

If you are a developer, you may need to update translation files as
text is modified. You can update all translation files with the
following commands:

```
meson compile potfiles # only necessary if files have been added or removed
meson compile naev-pot
meson compile naev-update-po
```

Again, you will only ever need to do this if you are a developer.

## CRASHES & PROBLEMS

Please take a look at the FAQ (linked below) before submitting a new
bug report, as it covers a number of common gameplay questions and
common issues.

If Naev is crashing during gameplay, please file a bug report after
reading https://github.com/naev/naev/wiki/Bugs


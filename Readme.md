[![Nightly Release Status](https://github.com/naev/naev/workflows/Nightly%20Release/badge.svg)](https://github.com/naev/naev/actions?query=workflow%3A%22Nightly+Release%22) [![CI Status](https://github.com/naev/naev/workflows/CI/badge.svg)](https://github.com/naev/naev/actions?query=workflow%3ACI) [![Packaging status](https://repology.org/badge/tiny-repos/naev.svg)](https://repology.org/project/naev/versions) [![Translation Status](https://hosted.weblate.org/widgets/naev/-/naev/svg-badge.svg)](https://hosted.weblate.org/projects/naev/)
# NAEV README

![Naev Logo](https://naev.org/imgs/naev.png)

**Naev (/nɑ.ɛv/)** is an open source 2D space trading and combat game, taking
inspiration from the [Escape Velocity
series](https://en.wikipedia.org/wiki/Escape_Velocity_(video_game)), among
others.

You pilot a space ship from a top-down perspective, and are more or less free
to do what you want. As the genre name implies, you’re able to trade and engage
in combat at will. Beyond that, there’s an ever-growing number of storyline
missions, equipment, and ships; Even the galaxy itself grows larger with each
release. For the literarily-inclined, there are large amounts of lore
accompanying everything from planets to equipment.

## DEPENDENCIES

Naev's dependencies are intended to be widely available. In addition to a
graphics card and driver supporting at least OpenGL 3.2, Naev requires:
* SDL 2
* libxml2
* freetype2
* GLPK
* libpng
* libwebp
* OpenAL
* OpenBLAS
* libvorbis
* intltool
* libunibreak (included)
* pyyaml

### DETAILS FOR YOUR OS

The Naev wiki has more detailed compilation steps, and lists of packages to install, for several operating systems and Linux distros:
* [Linux/\*nix](https://github.com/naev/naev/wiki/Compiling-on-*nix)
* [Windows](https://github.com/naev/naev/wiki/Compiling-on-Windows)
* [macOS](https://github.com/naev/naev/wiki/Compiling-on-macOS)

## COMPILING NAEV

### CLONING AND SUBMODULES

Naev requires the artwork submodule to run from git. You can check out the
submodules from the cloned repository with:

```bash
git submodule init
git submodule update
```

Note that `git submodule update` has to be run every time you `git pull` to
stay up to date. This can also be done automatically (highly recommended) by
setting the following configuration:

```bash
git config submodule.recurse true
```

### COMPILATION

Run:

```bash
meson setup builddir .
cd builddir
meson compile
./naev.sh
```

If you need special settings you can run `meson configure` in your build
directory to see a list of all available options.

**For installation**, try: `meson configure --buildtype=release -Db_lto=true`

**For Building a Windows Installer**, try adding: `--bindir=. -Dndata_path=. -Dinstaller=true`. Check the `dist` folder in your build directory

**For Building a macOS DMG**, try adding: `--prefix="$(pwd)"/build/dist/Naev.app --bindir=Contents/MacOS -Dndata_path=Contents/Resources -Dinstaller=true`. Check the `dist` folder in your build directory

**For normal development**, try adding: `--buildtype=debug -Db_sanitize=address` (adding `-Db_lundef=false` if compiling with Clang, substituting `-Ddebug_arrays=true` for `-Db_sanitize=...` on Windows if you can't use Clang).
(If your system supports debuginfod, also add `set debuginfod enabled on` to a file named `.gdbinit` in your home directory!)

**For faster debug builds** (but harder to trace with gdb/lldb), try `--buildtype=debugoptimized -Db_lto=true -Db_lto_mode=thin` in place of the corresponding values above.

### RUNNING NAEV

You can run Naev directly from the git repository using the `naev.sh` script
which will be generated in the build directory. This script will automatically
set up all the data paths for running Naev. Make sure the art assets are
checked out and up to date as mentioned in the Updating Art Assets section
below.

### INSTALLATION

Naev currently supports `meson install` which will install everything that
is needed.

If you wish to create a .desktop for your desktop environment, logos
from 16x16 to 256x256 can be found in `extras/logos/`.

## UPDATING ART ASSETS

Art assets are partially stored in the
[naev-artwork-production](https://github.com/naev/naev-artwork-production)
repository and sometimes are updated. For that reason, it is recommended to
periodically update the submodules with the following command.

```bash
git submodule update
```

You can also set this to be done automatically on git pull with the following
command:

```bash
git config submodule.recurse true
```

Afterwards, every time you perform a `git pull`, it will also update the
artwork submodule.

## CONTRIBUTING

To get in touch, you can visit [naev.org](https://naev.org/) which links to the project's Discord chat and Wiki.
There are also Lua API docs there.

Before committing, it's advisable to install [pre-commit](https://pre-commit.com/) 2.17 or newer, and run `pre-commit install`.
The dev team is teaching `pre-commit` to handle various fussy and forgettable steps.

### ONLINE TRANSLATION

Naev is incorporated into [Weblate](https://weblate.org/). You can easily
translate directly with a web interface to your chosen language from Naev's
[project page](https://hosted.weblate.org/projects/naev/naev/). New languages have to be added
manually, please open an issue if you want to translate Naev to a new language.

### TRANSLATION FOR DEVELOPERS

Naev's translation is handled with gettext. (It's custom, but C and Lua code can use the conventional `_()` for gettext and
`N_()` for gettext-noop, as well as `n_()` for ngettext.)

When content like missions is updated, new translatable text must be made available to Weblate.
The key manual step is to regenerate the `po/naev.pot` file (`meson compile naev-pot` in the build dir) and commit it.

Under the hood: `po/POTFILES.in` is a catalog of files that may have translatable text.
We keep it synced using pre-commit hooks (or manually: `meson compile potfiles`).
The `naev-pot` Meson target is built using standard `xgettext`, plus additional rules.
(Rules for `AUTHORS` and `intro` are in `po/update-po.sh`. Rules for XML data files are in `po/its/translation.its`.)
Individual translations can be updated via `meson compile naev-update-po`, but _don't do this_ without a good reason, because Weblate does the same job more carefully.

## CRASHES AND PROBLEMS

Please take a look at the [FAQ](https://github.com/naev/naev/wiki/FAQ) before submitting a new
bug report, as it covers a number of common gameplay questions and
common issues.

If Naev is crashing during gameplay, please file a bug report after
reading https://github.com/naev/naev/wiki/Bugs

## LICENSE

Naev is open source software compatible with the [Debian Free Software
Guidelines](https://www.debian.org/social_contract#guidelines) licensed under
the [GNU General Public License version 3 or
later](https://www.gnu.org/licenses/gpl-3.0.en.html), with some exceptions.
Please refer to the [LICENSE](LICENSE) file for more in-depth licensing
details.

 [![Nightly Release Status](https://codeberg.org/naev/naev/badges/workflows/naev_nightly.yml/badge.svg?branch=main)](https://codeberg.org/naev/naev/actions/?workflow=naev_nightly.yml)
 [![CI Status](https://codeberg.org/naev/naev/badges/workflows/naev_ci.yml/badge.svg?branch=main)](https://codeberg.org/naev/naev/actions/?workflow=naev_ci.yml)
 [![Packaging status](https://repology.org/badge/tiny-repos/naev.svg)](https://repology.org/project/naev/versions)
 [![Translation Status](https://translate.codeberg.org/widget/naev/naev/svg-badge.svg)](https://translate.codeberg.org/projects/naev/)





NAEV README
==========================================================================



 ![Naev Logo](https://naev.org/imgs/naev.png)

 **Naev (/nɑ.ɛv/)** is an open source 2D space trading and combat game,
 taking inspiration from the
 [Escape Velocity series](https://en.wikipedia.org/wiki/Escape_Velocity_(video_game)),
 among others.

 You pilot a spaceship from a top-down perspective, and are more or less
 free to do what you want. As the genre name implies, you’re able to trade
 and engage in combat at will. Beyond that, there’s an ever-growing number
 of storyline missions, equipment, and ships; Even the galaxy itself grows
 larger with each release. For the literarily-inclined, there are large
 amounts of lore accompanying everything from planets to equipment.

 Please note that Naev is still actively under development and not
 complete yet. Although there are a lot of things to do in the game, you
 will find incomplete or work in progress content as you progress.



Getting Naev
------------

 Naev is on steam, itch.io, flathub, many linux distributions and more! If
 you don't feel up to the task of compiling it yourself, please see the
 [Naev website](https://naev.org/downloads/) for different ways to get
 started playing Naev!



Plugins
-------

 Plugins are supported since version 0.10.0, and a plugin manager is built
 into Naev since 0.13.0 which is accessible from the main menu. If you
 want to get started making your own, please take a look at the
 [Naev Development Manual](https://naev.org/devmanual/) (WIP).



Dependencies
------------

 Naev's dependencies are intended to be widely available. In addition to a
 graphics card and driver supporting at least OpenGL 3.3, Naev requires:
   * rust 1.88 or later
   * bindgen 0.72 or later
   * SDL3`*`
   * libxml2
   * freetype2
   * OpenAL
   * OpenBLAS
   * libvorbis
   * intltool
   * SuiteSparse`*`
   * enet`*`
   * physfs`*`
   * lua 5.1 / luajit`*`
   * pcre2`*`
   * GLPK`*`
   * libunibreak`*`
   * cmark`*`
   * pyyaml (compilation only)

 Dependencies marked with `*` will use subprojects if not found in the
 host system.


### Details for your OS

 The Naev wiki has more detailed compilation steps, and lists of packages
 to install, for several operating systems and Linux distros:
   * [Linux/\*nix](https://codeberg.org/naev/naev/wiki/Compiling-on-*nix)
   * [Windows](https://codeberg.org/naev/naev/wiki/Compiling-on-Windows)
   * [macOS](https://codeberg.org/naev/naev/wiki/Compiling-on-macOS)



Compiling Naev
--------------

### Cloning and Submodules

 Naev requires the artwork submodule to run from git. You can check out
 the submodules from the cloned repository with:

 ```bash
 git submodule update --init --recursive
 ```

 Note that `git submodule update` has to be run every time you `git pull`
 to stay up to date. This can also be done automatically (highly
 recommended) by setting the following configuration:

 ```bash
 git config submodule.recurse true
 ```


### Compilation

 Run:

 ```bash
 meson setup builddir .
 cd builddir
 meson compile
 ./naev.py
 ```

 If you need special settings you can run `meson configure` in your build
 directory to see a list of all available options.

 **For installation**, try:
 `meson configure --buildtype=release -Db_lto=true`

 **For Building a Windows Installer**, try adding:
 `--prefix="$(pwd)"/build/windows` `--bindir=.` `-Dndata_path=.`
 `-Dinstaller=true`. Check the `dist` folder in your build directory

 **For Building a macOS DMG**, try adding:
 `--prefix="$(pwd)"/build/dist/Naev.app` `--bindir=Contents/MacOS`
 `-Dndata_path=Contents/Resources` `-Dinstaller=true`. Check the `dist`
 folder in your build directory

 **For normal development**, try adding:
 `--buildtype=debug -Db_sanitize=address` (adding `-Db_lundef=false` if
 compiling with Clang, substituting `-Ddebug_arrays=true` for
 `-Db_sanitize=...` on Windows if you can't use Clang). (If your system
 supports debuginfod, also add `set debuginfod enabled on` to a file named
 `.gdbinit` in your home directory!)

 **For faster debug builds** (but harder to trace with gdb/lldb), try
 `--buildtype=debugoptimized` `-Db_lto=true` `-Db_lto_mode=thin` in place
 of the corresponding values above.

 #### For up-to-date build instructions, check out the compilation page in our [wiki](https://codeberg.org/naev/naev/wiki/Compiling)


### Running Naev

 You can run Naev directly from the git repository using the `naev.py`
 script which will be generated in the build directory. This script will
 automatically set up all the data paths for running Naev. Make sure the
 art assets are checked out and up to date as mentioned in the Updating
 Art Assets section below.


### Installation

 Naev currently supports `meson install` which will install everything
 that is needed.

 If you wish to create a `.desktop` for your desktop environment, logos
 from 16x16 to 256x256 can be found in `extras/logos/`.



Updating Art Assets
-------------------

 Art assets are partially stored in the
 [naev-artwork](https://codeberg.org/naev/naev-artwork) repository and
 sometimes are updated. For that reason, it is recommended to periodically
 update the submodules with the following command.

 ```bash
 git submodule update --init --recursive
 ```

 You can also set this to be done automatically on git pull with the
 following command:

 ```bash
 git config submodule.recurse true
 ```

 Afterwards, every time you perform a `git pull`, it will also update the
 artwork submodule.



Contributing
------------

 To get in touch, you can visit [naev.org](https://naev.org/) which links
 to the project's Discord chat and Wiki. There are also Lua API docs
 there.

 Before committing, it's advisable to install
 [pre-commit](https://pre-commit.com/) 3.2 or newer, and run
 `pre-commit install` from the Naev git directory root. pre-commit will
 run automatically when commiting files, but can also be run manually with
 `pre-commit run -a`. The dev team is teaching `pre-commit` to handle
 various fussy and forgettable steps.

 Naev uses
 [Oxford Spelling](https://en.wikipedia.org/wiki/Oxford_spelling) for all
 text in the game.


### Online Translation

 Naev is incorporated into Weblate on Codeberg. You can easily translate
 directly with a web interface to your chosen language from Naev's
 [project page](https://translate.codeberg.org/projects/naev/). New
 languages have to be added manually, please open an issue if you want to
 translate Naev to a new language.

 Some translation notes:
   * Do not translate formatting strings. Example: `Hello {player}!` would
     be translated to Spanish as `Hola {player}!`
   * Do not translate the link part in markdown links. Example:
     `[mechanics](mechanics)` should be translated as
     `[TRANSLATION](mechanics)`
   * Use phonetical translations for names of places. Example: the space
     object `Dust` should be translated phonetically and not semantically.
     Exceptions to this rule are compound names such as stations, for
     example `Violin Monastery` should have `Violin` translated
     phonetically and `Monastery` translated semantically.


### Translation for Developers

 Naev's translation is handled with gettext. (It's custom, but C and Lua
 code can use the conventional `_()` for gettext and `N_()` for
 gettext-noop, as well as `n_()` for ngettext. Rust uses `gettext()`,
 `ngettext()`, and the likes instead.)

 When content like missions is updated, new translatable text must be made
 available to Weblate. The key manual step is to regenerate the
 `po/naev.pot` file (`meson compile naev-pot` in the build dir) and commit
 it. To avoid merge conflicts, it is recommended to not include updated
 `po/naev.pot` in a pull request that isn't exclusively about translation.

 Under the hood: `po/POTFILES.in` is a catalogue of files that may have
 translatable text. We keep it synced using pre-commit hooks (or manually:
 `meson compile potfiles`). The `naev-pot` Meson target is built using
 standard `xgettext`, plus additional rules. (Rules for `AUTHORS` are in
 `po/update-po.sh`. Rules for XML data files are in
 `po/its/translation.its`.) Individual translations can be updated via
 `meson compile naev-update-po`, but _don't do this_ without a good
 reason, because Weblate does the same job more carefully.



Crashes and Problems
--------------------

 Please take a look at the [FAQ](https://codeberg.org/naev/naev/wiki/FAQ)
 before submitting a new bug report, as it covers a number of common
 gameplay questions and common issues.

 If Naev is crashing during gameplay, please file a bug report after
 reading [this page](https://codeberg.org/naev/naev/wiki/Bugs).



Licence
-------

 Naev is open source software compatible with the
 [Debian Free Software Guidelines](https://www.debian.org/social_contract#guidelines)
 licenced under the
 [GNU General Public Licence version 3 or later](https://www.gnu.org/licenses/gpl-3.0.en.html),
 with some exceptions. Please refer to the [LICENSE](LICENSE) file for
 more in-depth licencing details.

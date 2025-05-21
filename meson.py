#!/usr/bin/env python3
#
# This script will download and wrap meson if the current installed version
# is not at least version 1.4.0 (which meson.build currently requires)
# if you have a newer version of meson in PATH, this wrapper isn't needed,
# but it will pass commands through all the same.
#
# To force using the downloaded Meson version, run the script with the --force-download flag:
# e.g. meson.py --force-download setup builddir
#
# Keep in mind that this script does not account for ninja-build being installed (yet)
#

import os
import subprocess
import tarfile
import urllib.request
import shutil
import sys
import argparse

MESONDIR = "meson-bin"
VERSION = "1.4.0"
PACKAGE = f"meson-{VERSION}.tar.gz"
MESON = os.path.join(MESONDIR, "meson.py")

def grab_meson():
    if not os.path.exists(MESONDIR):
        os.mkdir(MESONDIR)

    if not os.path.exists(MESON):
        print("Grabbing from online")
        urllib.request.urlretrieve(
            f"https://github.com/mesonbuild/meson/releases/download/{VERSION}/{PACKAGE}",
            PACKAGE
        )
        with tarfile.open(PACKAGE, "r:gz") as tar:
            top_level_dir = tar.getnames()[0].split('/')[0]
            members = [member for member in tar.getmembers() if member.name.startswith(top_level_dir + "/")]
            for member in members:
                member.name = member.name[len(top_level_dir) + 1:]
                if member.name:
                    tar.extract(member, MESONDIR)
        os.remove(PACKAGE)
    else:
        print(f"Using cached version {VERSION}")

def run_meson(*args):
    if MESONDIR in MESON:
        subprocess.run([sys.executable, MESON] + list(args))
    else:
        subprocess.run([MESON] + list(args))

def main():
    parser = argparse.ArgumentParser(description="Wraps meson to ensure the required version is used.")
    parser.add_argument('--force-download', action='store_true', help="Force using the downloaded Meson version")
    args, meson_args = parser.parse_known_args()

    if args.force_download:
        print("Forcing use of downloaded Meson version")
        grab_meson()
    else:
        meson_path = shutil.which("meson")
        if meson_path is None:
            print("You don't have Meson in PATH")
            grab_meson()
        else:
            currentver = subprocess.check_output(["meson", "--version"]).decode().strip()
            requiredver = "1.4.0"
            if tuple(map(int, currentver.split("."))) >= tuple(map(int, requiredver.split("."))):
                print(f"Meson version is greater than or equal to {requiredver}")
                global MESON
                MESON = "meson"
            else:
                print(f"Meson version is lower than {requiredver}")
                grab_meson()

    run_meson(*meson_args)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import os
import sys
import shutil
import subprocess

def usage():
    print(f"usage: {os.path.basename(sys.argv[0])} [-v] (Verbose output)")
    print("NSIS installer compilation script for Naev")
    print("This script is called by 'meson install' when building on Windows, and with the 'installer' option selected.")
    print("It builds an NSIS installer for Naev")
    print("Requires 'makensis' to be installed and available on PATH.")
    print(f"usage: {os.path.basename(sys.argv[0])} [-v] (Verbose output)")
    sys.exit(1)

verbose = False

# Parse command-line arguments
args = sys.argv[1:]
while args:
    arg = args.pop(0)
    if arg == '-v':
        verbose = True
    else:
        usage()

# Check if makensis is available
if not shutil.which('makensis'):
    print("You don't have makensis in PATH")
    sys.exit(1)

# Set paths
WORKPATH = os.path.join(os.getenv('MESON_BUILD_ROOT'), 'installer_staging')
BIN_DIR = os.path.join(WORKPATH, 'bin')
DIST_DIR = os.path.join(os.getenv('MESON_BUILD_ROOT'), 'dist')

# Check if WORKPATH already exists, if not create it
if not os.path.exists(WORKPATH):
    os.makedirs(WORKPATH)

# Make temp directory
os.makedirs(BIN_DIR, exist_ok=True)

# Copy installer assets to staging area
shutil.copytree(os.path.join(os.getenv('MESON_SOURCE_ROOT'), 'extras/windows/installer_assets'), WORKPATH, dirs_exist_ok=True)
shutil.copy(os.path.join(os.getenv('MESON_SOURCE_ROOT'), 'LICENSE'), os.path.join(WORKPATH, 'legal/naev-license.txt'))
shutil.copy(os.path.join(os.getenv('MESON_INSTALL_DESTDIR_PREFIX'), 'dat/VERSION'), os.path.join(WORKPATH, 'VERSION'))

# Copy Windows DLLs, binary, and dat files to staging area
shutil.copytree(os.getenv('MESON_INSTALL_DESTDIR_PREFIX'), BIN_DIR)

# Set VERSION
with open(os.path.join(WORKPATH, 'VERSION'), 'r') as version_file:
    VERSION = version_file.read().strip()

# Compile NSIS installer
makensis_args = ["-XOutFile", os.path.join(DIST_DIR, 'naev-installer.exe'), "-DVERSION=" + VERSION, os.path.join(WORKPATH, 'naev.nsi')]
subprocess.run(['makensis'] + makensis_args)

# Clean up
shutil.rmtree(WORKPATH)

if verbose:
    print(f"Created NSIS installer: {os.path.join(DIST_DIR, 'naev-installer.exe')}")

#!/usr/bin/env python3

import os
import subprocess
import shutil
import sys
import re

source_root = os.environ['MESON_SOURCE_ROOT']

def usage():
    print(f"usage: {os.path.basename(sys.argv[0])} [-d] (Verbose output)")
    print("DLL Bundler for Windows")
    print("This script is called by 'meson install' if building for Windows.")
    print("The intention is for this wrapper to behave in a similar manner to the bundle.py script in extras/macos.")
    print(f"usage: {os.path.basename(sys.argv[0])} [-d] (Verbose output)")
    sys.exit(1)

# Yoink helper function from mingw-bundledlls to help find DLL in given search paths
def find_full_path(filename, path_prefixes):
    for path_prefix in path_prefixes:
        path = os.path.join(path_prefix, filename)
        path_low = os.path.join(path_prefix, filename.lower())
        if os.path.exists(path):
            return path
        if os.path.exists(path_low):
            return path_low
    else:
        raise RuntimeError(
            "Can't find " + filename + ". If it is an inbuilt Windows DLL, "
            "please add it to the blacklist variable in the script and send "
            "a pull request!"
        )

verbose = False

# Parse command-line arguments
args = sys.argv[1:]
while args:
    arg = args.pop(0)
    if arg == '-d':
        verbose = True
    else:
        usage()

# Get DLL search path from meson
devenv = subprocess.run([sys.executable, os.path.join(source_root, "meson.py"), "devenv", "--dump"], capture_output=True, check=True)

# WINEPATH is set in cross compilation enviornments. Check it first
devenvPath = re.search(f"WINEPATH=\"(.*);\\$WINEPATH\"",devenv.stdout.decode())
if devenvPath is not None:
    os.environ['MINGW_BUNDLEDLLS_SEARCH_PATH'] = (devenvPath.group(1) + ";" + os.environ.get('WINEPATH', default="")).replace(";", os.pathsep)

# If WINEPATH wasn't set, this is a native windows build, and we should use PATH
else:
    devenvPath = re.search(f"PATH=\"(.*){os.pathsep}\\$PATH\"",devenv.stdout.decode())
    os.environ['MINGW_BUNDLEDLLS_SEARCH_PATH'] = devenvPath.group(1) + os.pathsep + os.environ.get('PATH', default="")

# Run mingw-bundledlls to get DLL list
dll_list_cmd = [sys.executable, os.path.join(os.getenv('MESON_SOURCE_ROOT'), 'extras/windows/mingw-bundledlls/mingw-bundledlls'),
                os.path.join(os.getenv('MESON_BUILD_ROOT'), 'naev.exe')]

if verbose:
    print("Executing command:", dll_list_cmd)
    print("Working directory:", os.getcwd())

dll_list_proc = subprocess.run(dll_list_cmd, capture_output=True)
dll_list_out = dll_list_proc.stdout
dll_list_err = dll_list_proc.stderr

if verbose:
    print(dll_list_out.decode())
if verbose or dll_list_proc.returncode != 0:
    print(dll_list_err.decode())

dll_list_proc.check_returncode()

# HACK: Add SDL3.dll if it exists
# This is a workaround for the fact that objdump doesnm't show SDL3.dll as a dependency for SDL2-compat
# So lets include it in the list of DLLs to copy

dll_list = dll_list_out.decode().splitlines()
search_paths = os.environ['MINGW_BUNDLEDLLS_SEARCH_PATH'].split(os.pathsep)
try:
    sdl3_path = find_full_path('SDL3.dll', search_paths)
    dll_list.append(sdl3_path)
    if verbose:
        print(f"Found SDL3.dll at {sdl3_path} and added to dll_list")
except RuntimeError:
    if verbose:
        print("SDL3.dll not found. Skipping.")

# Copy DLLs to installation directory
for dll in dll_list:
    dll_path = os.path.join(os.getenv('MESON_INSTALL_DESTDIR_PREFIX'), os.path.basename(dll))
    shutil.copy(dll, dll_path)

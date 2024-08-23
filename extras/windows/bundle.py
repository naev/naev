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
os.environ['MINGW_BUNDLEDLLS_SEARCH_PATH'] = re.search(f"PATH=\"(.*){os.pathsep}\\$PATH\"",devenv.stdout.decode()).group(1) + os.pathsep + os.environ['PATH']

# Run mingw-bundledlls to get DLL list
dll_list_cmd = [sys.executable, os.path.join(os.getenv('MESON_SOURCE_ROOT'), 'extras/windows/mingw-bundledlls/mingw-bundledlls'),
                os.path.join(os.getenv('MESON_BUILD_ROOT'), 'naev.exe')]

if verbose:
    print("Executing command:", dll_list_cmd)
    print("Working directory:", os.getcwd())

dll_list_proc = subprocess.Popen(dll_list_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
dll_list_out, dll_list_err = dll_list_proc.communicate()

if verbose:
    print(dll_list_out.decode())
    print(dll_list_err.decode())

# Copy DLLs to installation directory
dll_list = dll_list_out.decode().splitlines()
for dll in dll_list:
    dll_path = os.path.join(os.getenv('MESON_INSTALL_DESTDIR_PREFIX'), os.path.basename(dll))
    shutil.copy(dll, dll_path)

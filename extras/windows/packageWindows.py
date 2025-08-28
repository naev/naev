#!/usr/bin/env python3

import os
import sys
import tarfile

def usage():
   print(f"usage: {os.path.basename(sys.argv[0])} [-v] (Verbose output)")
   print("Windows release packaging script for Naev")
   print("This script is called by 'meson install' if building for Windows with the release feature set to 'true'.")
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

# Create dist folder
os.makedirs(os.path.join(os.getenv('MESON_BUILD_ROOT'), 'dist'), exist_ok=True)

# Package steam windows tarball
source_dir = os.getenv('MESON_INSTALL_DESTDIR_PREFIX')
output_tarball = os.path.join(os.getenv('MESON_BUILD_ROOT'), 'dist', 'naev-windows.tar.xz')

with tarfile.open(output_tarball, 'w:xz') as tar:
   for root, dirs, files in os.walk(source_dir):
      for file in files:
         if file.endswith(('.dll', '.exe')):
            tar.add(os.path.join(root, file), arcname=os.path.relpath(os.path.join(root, file), source_dir))

if verbose:
   print(f"Created tarball: {output_tarball}")

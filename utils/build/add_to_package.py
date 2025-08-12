#!/usr/bin/env python3

import os
import sys
import shutil
import logging

# Configure logging to stderr and stdout
logging.basicConfig(level=logging.INFO, stream=sys.stderr)

def copy_file(source_file, dist_file):
   try:
      os.makedirs(os.path.dirname(dist_file), exist_ok=True)
      shutil.copyfile(source_file, dist_file)
      logging.info(f"File '{source_file}' copied to '{dist_file}'.")
   except Exception as e:
      logging.error(f"Error copying file: {e}")
      sys.exit(1)

if __name__ == "__main__":
   if len(sys.argv) != 2:
      logging.error("Usage: python3 add_to_package.py <filename>")
      sys.exit(1)

   source_root = os.getenv("MESON_SOURCE_ROOT")
   dist_root = os.getenv("MESON_DIST_ROOT")

   if not source_root or not dist_root:
      logging.error("Error: MESON_SOURCE_ROOT or MESON_DIST_ROOT environment variables not set.")
      sys.exit(1)

   source_file = os.path.join(source_root, sys.argv[1])
   dist_file = os.path.join(dist_root, sys.argv[1])

   copy_file(source_file, dist_file)

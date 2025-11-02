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
   if len(sys.argv) != 3:
      logging.error("Usage: python3 add_to_package.py <INFILE> <OUTFILE>")
      sys.exit(1)

   dist_root = os.getenv("MESON_DIST_ROOT")

   if not dist_root:
      logging.error("Error: MESON_DIST_ROOT environment variable not set.")
      sys.exit(1)

   source_file = sys.argv[1]
   dist_file = os.path.join(dist_root, sys.argv[2])

   copy_file(source_file, dist_file)

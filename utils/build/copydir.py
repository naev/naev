#!/usr/bin/env python3
import os
import sys
import shutil
import logging

# Configure logging to stderr and stdout
logging.basicConfig(level=logging.INFO, stream=sys.stderr)

def copy_file(source_file, dest_dir):
   try:
      os.makedirs(dest_dir, exist_ok=True)
      shutil.copy(source_file, os.path.join(dest_dir, os.path.basename(source_file)))
      logging.info(f"File '{source_file}' copied to '{dest_dir}'.")
   except Exception as e:
      logging.error(f"Error copying file: {e}")
      sys.exit(1)

if __name__ == "__main__":
   if len(sys.argv) < 3:
      logging.error("Usage: python3 copydir.py <destination_directory> <file1> <file2> ...")
      sys.exit(1)

   outdir = sys.argv[1]
   for f in sys.argv[2:]:
      copy_file(f, outdir)

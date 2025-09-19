#!/usr/bin/env python3

import os
import subprocess
import sys
import logging

# Configure logging to output to stderr
logging.basicConfig(level=logging.INFO, stream=sys.stderr)


def get_commit_sha(source_root: str) -> str | None:
   """Return the short git commit SHA, or None if unavailable."""
   try:
      proc = subprocess.run(
         ["git", "-C", source_root, "rev-parse", "--short", "HEAD"],
         capture_output=True,
         text=True,
         check=True,
      )
      return proc.stdout.strip()
   except Exception:
      return None


def is_dirty(source_root: str) -> bool:
   """Return True if the git repo has uncommitted changes."""
   try:
      proc = subprocess.run(
         ["git", "-C", source_root, "status", "--porcelain"],
         capture_output=True,
         text=True,
         check=True,
      )
      return bool(proc.stdout.strip())
   except Exception:
      return False


def get_version(source_root: str, base_version: str) -> str:
   """
   Determine the project version:
      1. Use base_version + git SHA [+dirty] if .git exists.
      2. Otherwise, read existing dat/VERSION if present.
      3. Otherwise, just base_version.
      Always write the result back to dat/VERSION.
   """
   version_file = os.path.join(source_root, "dat", "VERSION")

   version = None

   git_dir = os.path.join(source_root, ".git")
   if os.path.isdir(git_dir):
      version = base_version
      sha = get_commit_sha(source_root)
      if sha:
         version += f"+g{sha}"
         if is_dirty(source_root):
            version += ".dirty"
   elif os.path.isfile(version_file):
      # No git, but VERSION file exists (tarball or source package)
      try:
         with open(version_file, "r") as f:
            version = f.read().strip()
      except Exception as e:
         logging.warning(f"Could not read existing VERSION file: {e}")

   if not version:
      # Fallback: just use the base project version
      version = f"{base_version}+dev"

   # Write the version to VERSION file
   try:
      os.makedirs(os.path.dirname(version_file), exist_ok=True)
      with open(version_file, "w") as f:
         f.write(version)
   except Exception as e:
      logging.warning(f"Could not write VERSION file: {e}")

   return version

if __name__ == "__main__":
   source_root = os.getenv("MESON_SOURCE_ROOT")
   if not source_root:
      logging.error("This script is intended to be run by Meson.")
      sys.exit(1)
   if len(sys.argv) < 2:
      logging.error("Usage: gen_version.py <project_version>")
      sys.exit(1)

   base_version = sys.argv[1]
   version = get_version(source_root, base_version)
   logging.info(f"Version retrieved: {version}")
   print(version)

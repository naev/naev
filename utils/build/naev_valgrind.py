#!/usr/bin/env python3
# Standalone Valgrind server for Naev.
#
# Usage:
#   1. Run this script in Terminal A:         ./naev_valgrind.py
#   2. Attach the debugger in Terminal B:     WITHVALGRIND=true ./naev.py
#      (Alternatively, use 'vgdb' or 'gdb' directly as instructed by this script).
#
# Environment Variables:
#   VG_TRACE_CHILDREN=true  Enable --trace-children=yes in Valgrind.
#   VG_LOGFILE=filename     Specify a log file for Valgrind output.
#
# Automated Settings (Applied by script):
#   ALSOFT_LOGLEVEL      Set to 2 (debug) or 3 (paranoid) to help with OpenAL debugging.
#   ALSOFT_TRAP_AL_ERROR Set to 1 (paranoid only) to catch OpenAL errors immediately.
#   RUST_BACKTRACE       Always set to 1 to provide detailed Rust error reports.
#   ASAN_OPTIONS         Always set to halt_on_error=1 for precise memory checking.

import os
import signal
import subprocess
import shutil
import sys
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def env_bool(name: str, default: bool) -> bool:
   v = os.getenv(name)
   if v is None:
      return default
   v = v.strip().lower()
   return v in ("1", "true", "yes", "y", "on")

# Optional VG knobs
VG_TRACE_CHILDREN = env_bool("VG_TRACE_CHILDREN", False)
VG_LOGFILE = os.getenv("VG_LOGFILE", "")

source_root = "@source_root@"
build_root = "@build_root@"
debug_paranoid = "@debug_paranoid@"
debug = "@debug@"
naev_bin = "@naev_bin@"
zip_overlay = "@zip_overlay@"

MESON = [sys.executable, os.path.join(source_root, "meson.py")]

def wrapper(*args):
   if not os.path.exists(os.path.join(source_root, "meson.py")):
      logger.error(f"Error: meson.py not found at {source_root}. Check your source tree.")
      return

   if debug_paranoid == "True":
      os.environ["ALSOFT_LOGLEVEL"] = "3"
      os.environ["ALSOFT_TRAP_AL_ERROR"] = "1"
   elif debug == "True":
      os.environ["ALSOFT_LOGLEVEL"] = "2"
   os.environ["RUST_BACKTRACE"] = "1"
   os.environ["ASAN_OPTIONS"] = "halt_on_error=1"

   logger.info("Valgrind Server Mode enabled (10x-50x slower).")
   
   if not shutil.which("valgrind"):
      logger.error("Error: valgrind is not installed or not in PATH.")
      return

   logger.info("Waiting for GDB connection via vgdb...")

   valgrind_command = [
      "valgrind",
      "--leak-check=full",
      "--show-leak-kinds=all",
      "--track-origins=yes",
      "--num-callers=100",
      "--error-limit=no",
      "--vex-guest-max-insns=25", # Workaround for temporary storage exhaustion
      "--vgdb=yes",
      "--vgdb-error=0"
   ]

   if VG_TRACE_CHILDREN:
      valgrind_command += ["--trace-children=yes"]
   if VG_LOGFILE:
      valgrind_command += [f"--log-file={VG_LOGFILE}"]

   full_command = MESON + ["devenv", "-C", build_root] + valgrind_command + list(args)

   try:
      subprocess.run(full_command)
   except KeyboardInterrupt:
      pass

# Build steps
subprocess.run(MESON + ["compile", "-C", build_root, "naev-gmo"], check=False)
os.makedirs(os.path.join(build_root, "dat/gettext"), exist_ok=True)
po_root = os.path.join(build_root, "po")
if os.path.isdir(po_root):
   for mo_name in os.listdir(po_root):
      mo_path = os.path.join(po_root, mo_name)
      if os.path.isdir(mo_path):
         dest_dir = os.path.join(build_root, "dat/gettext", mo_name)
         shutil.copytree(mo_path, dest_dir, dirs_exist_ok=True)

wrapper(
   naev_bin,
   "-d", zip_overlay,
   "-d", os.path.join(source_root, "dat"),
   "-d", os.path.join(source_root, "assets"),
   "-d", os.path.join(build_root, "dat"),
   "-d", source_root,
   *sys.argv[1:]
)

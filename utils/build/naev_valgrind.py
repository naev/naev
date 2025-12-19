#!/usr/bin/env python3
# Standalone Valgrind server for Naev.
#
# Usage:
#   1. Run this script in Terminal A:         ./naev_valgrind.py
#   2. Attach the debugger in Terminal B:     WITHVALGRIND=true ./naev.py
#      (Alternatively, use 'vgdb' or 'gdb' directly as instructed by this script).
#
#   Pro-tip: To see game stdout in the debugger terminal, run `tty` in that
#            terminal to get its device path (e.g. /dev/pts/1), then start
#            this server with: ./naev_valgrind.py > /dev/pts/1 2>&1
#
# Environment Variables:
#   VG_TRACE_CHILDREN=true  Enable --trace-children=yes in Valgrind.
#   VG_LOGFILE=filename     Specify a log file for Valgrind output (default: build_root/naev_valgrind.log).
#   WITHDEBUGGER=true       Enable Valgrind server mode (vgdb) for GDB attachment.
#   VG_SUPPRESSIONS=file    Path to a Valgrind suppression (.supp) file (default: @naev_supp@).
#   VG_ARGS="--flags"       Additional raw arguments to pass to Valgrind.
#
# Automated Settings (Applied by script):
#   ALSOFT_LOGLEVEL      Set to 2 (debug) or 3 (paranoid) to help with OpenAL debugging.
#   ALSOFT_TRAP_AL_ERROR Set to 1 (paranoid only) to catch OpenAL errors immediately.
#   RUST_BACKTRACE       Always set to 1 to provide detailed Rust error reports.
#   ASAN_OPTIONS         Always set to halt_on_error=1 for precise memory checking.

import os
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

source_root = "@source_root@"
build_root = "@build_root@"
debug_paranoid = "@debug_paranoid@"
debug = "@debug@"
naev_bin = "@naev_bin@"
zip_overlay = "@zip_overlay@"

# Optional VG knobs
VG_TRACE_CHILDREN = env_bool("VG_TRACE_CHILDREN", False)
VG_LOGFILE = os.getenv("VG_LOGFILE", os.path.join(build_root, "naev_valgrind.log"))
WITHDEBUGGER = env_bool("WITHDEBUGGER", False)

# Default suppressions to naev.supp in the utils/build directory
naev_supp = os.path.join(source_root, "utils", "build", "naev.supp")
VG_SUPPRESSIONS = [naev_supp]
if os.getenv("VG_SUPPRESSIONS"):
   VG_SUPPRESSIONS.extend(os.getenv("VG_SUPPRESSIONS").split(','))

VG_ARGS = os.getenv("VG_ARGS", "")

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

   if not shutil.which("valgrind"):
      logger.error("Error: valgrind is not installed or not in PATH.")
      return

   if not WITHDEBUGGER:
      logger.info("Valgrind Standalone Mode: Running game directly under Valgrind...")
   else:
      logger.info("Valgrind Server Mode: Waiting for GDB connection via vgdb...")

   # Define an isolated pipe prefix in the build directory to ensure client/server find each other.
   vgdb_prefix = os.path.join(build_root, ".vgdb-pipe")

   valgrind_command = [
      "valgrind",
      "--leak-check=full",
      "--show-leak-kinds=all",
      "--track-origins=yes",
      "--num-callers=100",
      "--error-limit=no",
      "--vex-guest-max-insns=25", # Workaround for temporary storage exhaustion
   ]

   if WITHDEBUGGER:
      valgrind_command += [
         "--vgdb=yes",
         "--vgdb-error=0",
         f"--vgdb-prefix={vgdb_prefix}",
      ]

   for suppression in VG_SUPPRESSIONS:
      if os.path.exists(suppression):
         valgrind_command += [f"--suppressions={suppression}"]
      else:
         logger.warning(f"Suppression file not found: {suppression}")

   if VG_ARGS:
      import shlex
      valgrind_command += shlex.split(VG_ARGS)

   if VG_TRACE_CHILDREN:
      valgrind_command += ["--trace-children=yes"]
   if VG_LOGFILE:
      valgrind_command += [f"--log-file={VG_LOGFILE}"]

   full_command = MESON + ["devenv", "-C", build_root] + valgrind_command + list(args)

   try:
      p = subprocess.Popen(full_command)
      p.wait()
   except KeyboardInterrupt:
      logger.info("\nCtrl+C detected, shutting down Valgrind...")
      p.terminate()
      try:
         p.wait(timeout=5)
      except subprocess.TimeoutExpired:
         p.kill()
   except Exception as e:
      logger.error(f"Failed to execute Valgrind: {e}")
   finally:
      if sys.platform != "win32":
         os.system("stty sane 2>/dev/null")

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
   *sys.argv[1:],
)

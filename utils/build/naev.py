#!/usr/bin/env python3
# Helper script to run Naev with a debugger (GDB/LLDB) or as a Valgrind client.
#
# Standard Usage:
#   ./naev.py
#
# Valgrind Client Usage:
#   1. Start the Valgrind server in Terminal A: ./naev_valgrind.py
#   2. Run this script in Terminal B:         WITHVALGRIND=true ./naev.py
#   3. When you are done, you can kill the Valgrind server in Terminal A with Ctrl+C
#      Or you can type `kill` and then `quit` in the debugger in Terminal B.
#
#   Pro-tip: To see game stdout in Terminal B (debugger), run `tty` in B to get
#            your TTY (e.g. /dev/pts/1), then start the server in A with:
#            ./naev_valgrind.py > /dev/pts/1 2>&1
#
# Environment Variables:
#   WITHDEBUGGER=false  Disable launching debuggers (default: true).
#   PREFERLLDB=true      Use LLDB instead of GDB if both are available.
#   WITHVALGRIND=true    Enable Valgrind client mode (attaches to server).
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
import time
from typing import Optional, List

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def env_bool(name: str, default: bool) -> bool:
   v = os.getenv(name)
   if v is None:
      return default
   v = v.strip().lower()
   return v in ("1", "true", "yes", "y", "on")

# User-config knobs
PREFERLLDB   = env_bool("PREFERLLDB", False)
WITHDEBUGGER = env_bool("WITHDEBUGGER", True)
WITHVALGRIND = env_bool("WITHVALGRIND", False)

source_root = "@source_root@"
build_root = "@build_root@"
debug_paranoid = "@debug_paranoid@"
debug = "@debug@"
naev_bin = "@naev_bin@"
zip_overlay = "@zip_overlay@"

MESON = [sys.executable, os.path.join(source_root, "meson.py")]

# Build debugger command
def get_debugger() -> Optional[str]:
   if not WITHDEBUGGER:
      logger.info("Debugging disabled (WITHDEBUGGER=false).")
      return None

   preferred = "lldb" if PREFERLLDB else "gdb"
   if shutil.which(preferred):
      return preferred
   if shutil.which("gdb"):
      return "gdb"
   if shutil.which("lldb"):
      return "lldb"

   logger.warning("Neither gdb nor lldb found; running without debugger.")
   return None


def wrapper(*args: str) -> int:
   if not os.path.exists(os.path.join(source_root, "meson.py")):
      logger.error(f"Error: meson.py not found at {source_root}. Check your source tree.")
      return 1

   # Environment setup
   if debug_paranoid == "True":
      os.environ["ALSOFT_LOGLEVEL"] = "3"
      os.environ["ALSOFT_TRAP_AL_ERROR"] = "1"
   elif debug == "True":
      os.environ["ALSOFT_LOGLEVEL"] = "2"

   os.environ["RUST_BACKTRACE"] = "1"
   os.environ["ASAN_OPTIONS"] = "halt_on_error=1"

   debugger = get_debugger()

   if WITHVALGRIND:
       if debugger == "gdb":
           vgdb_prefix = os.path.join(build_root, ".vgdb-pipe")
           if not shutil.which("vgdb"):
               logger.error("Error: 'vgdb' utility not found. It is usually part of the valgrind package.")
               return 1
           logger.info("Valgrind Client Mode: Attaching to remote vgdb server...")
           cmd = [
              "gdb",
              "--nx",
              "-x", os.path.join(build_root, ".gdbinit"),
              "-ex", f"target remote | vgdb --vgdb-prefix={vgdb_prefix}",
              "-ex", "continue",
              "--args",
           ] + list(args)
       else:
           logger.error("Valgrind Client Mode requires GDB. LLDB does not support vgdb.")
           return 1
   elif debugger == "gdb":
       cmd = [
          "gdb",
          "--nx",
          "-x", os.path.join(build_root, ".gdbinit"),
          "-ex", "run",
          "--args",
       ] + list(args)
   elif debugger == "lldb":
       cmd = [
          "lldb",
          "--one-line", f"command script import {os.path.join(build_root, 'lldbinit.py')}",
          "--",
       ] + list(args)
   else:
       cmd = list(args)

   full_command = MESON + ["devenv", "-C", build_root] + cmd

   # Ignore SIGINT in the parent so the child (debugger/game) handles it.
   old_handler = signal.signal(signal.SIGINT, signal.SIG_IGN)

   try:
      # Run the command and wait for it to complete.
      p = subprocess.Popen(full_command)
      return p.wait()
   except Exception as e:
      logger.error(f"Failed to execute command: {e}")
      return 1
   finally:
      # Restore SIGINT handler and terminal state
      signal.signal(signal.SIGINT, old_handler)
      if sys.platform != "win32":
         os.system("stty sane 2>/dev/null")

# Meson >= 0.60
if not WITHVALGRIND:
   subprocess.run(MESON + ["compile", "-C", build_root, "naev-gmo"], check=False)
   os.makedirs(os.path.join(build_root, "dat/gettext"), exist_ok=True)
   po_root = os.path.join(build_root, "po")
   if os.path.isdir(po_root):
      for mo_name in os.listdir(po_root):
         mo_path = os.path.join(po_root, mo_name)
         if os.path.isdir(mo_path):
            dest_dir = os.path.join(build_root, "dat/gettext", mo_name)
            shutil.copytree(mo_path, dest_dir, dirs_exist_ok=True)
            logger.info("Copied directory %s -> %s", mo_path, dest_dir)

# Run target
rc = wrapper(
   naev_bin,
   "-d", zip_overlay,
   "-d", os.path.join(source_root, "dat"),
   "-d", os.path.join(source_root, "assets"),
   "-d", os.path.join(build_root, "dat"),
   "-d", source_root,
   *sys.argv[1:],
)
raise SystemExit(rc)

#!/usr/bin/env python3
import os
import subprocess
import shutil
import sys
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Prefers GDB over LLDB if both are installed, you can choose your preference
# by exporting PREFERLLDB=true
# Set WITHDEBUGGER=false to avoid using debuggers where it is a hinderance.
PREFERLLDB = os.getenv("PREFERLLDB", "false")
WITHDEBUGGER = os.getenv("WITHDEBUGGER", "true")

def get_debugger():
    preferred_debugger = "gdb"

    if WITHDEBUGGER == "false":
        logger.info("Debugging disabled.")
        return

    if PREFERLLDB == "true":
        preferred_debugger = "lldb"

    if shutil.which(preferred_debugger):
        return preferred_debugger
    elif shutil.which("lldb"):
        return "lldb"
    elif shutil.which("gdb"):
        return "gdb"
    else:
        logger.error("Error: Neither lldb nor gdb is installed. Debugging disabled.")

source_root = "@source_root@"
build_root = "@build_root@"
debug_paranoid = "@debug_paranoid@"
debug = "@debug@"
naev_bin = "@naev_bin@"
zip_overlay = "@zip_overlay@"

def wrapper(*args):
    if debug_paranoid == "True":
        os.environ["ALSOFT_LOGLEVEL"] = "3"
        os.environ["ALSOFT_TRAP_AL_ERROR"] = "1"
    elif debug == "True":
        os.environ["ALSOFT_LOGLEVEL"] = "2"

    os.environ["ASAN_OPTIONS"] = "halt_on_error=1"
    debugger = get_debugger()

    if "gdb" in debugger:
        subprocess.run([debugger, "-x", os.path.join(build_root, ".gdbinit"), "--args"] + list(args))
    elif "lldb" in debugger:
        # TODO: Implement LLDB initialization
        subprocess.run([debugger, "-o", "run", "--"] + list(args))
    else:
        subprocess.run(list(args))

subprocess.run([os.path.join(source_root, "meson.sh"), "compile", "-C", build_root, "naev-gmo"])
os.makedirs(os.path.join(build_root, "dat/gettext"), exist_ok=True)

# Meson <= 0.59
for mo_path in os.listdir(os.path.join(build_root, "po")):
    mo_path = os.path.join(build_root, "po", mo_path)
    if os.path.isfile(mo_path):
        mo_name = os.path.basename(mo_path)
        lang = mo_name.replace(".gmo", "")
        lang_dir = os.path.join(build_root, "dat/gettext", lang, "LC_MESSAGES")
        os.makedirs(lang_dir, exist_ok=True)
        shutil.copy(mo_path, os.path.join(lang_dir, "naev.mo"))
        logger.info(f"Copied {mo_path} to {os.path.join(lang_dir, 'naev.mo')}")

# Meson >= 0.60
for mo_path in os.listdir(os.path.join(build_root, "po")):
    mo_path = os.path.join(build_root, "po", mo_path)
    if os.path.isdir(mo_path):
        dest_dir = os.path.join(build_root, "dat/gettext")
        shutil.copytree(mo_path, dest_dir, dirs_exist_ok=True)
        logger.info(f"Copied directory {mo_path} to {dest_dir}")

wrapper(naev_bin, "-d", zip_overlay, "-d", os.path.join(source_root, "dat"), "-d", os.path.join(source_root, "artwork"), "-d", os.path.join(build_root, "dat"), "-d", source_root, *sys.argv[1:])

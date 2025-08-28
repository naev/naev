#!/usr/bin/env python3

# MACOS INSTALL SCRIPT FOR NAEV
# This script is called by "meson install" if building for macOS.
# It assumes a dependency must be bundled (with priority over any installations the end user might have) if either
# | it's in a known Homebrew or osxcross path (LOCAL_LIB_ROOTS below)
# | it's relative to the rpath (Meson subproject / embedded lib)

import os
import shutil
import subprocess
import sys


LOCAL_LIB_ROOTS = ('/opt/local', '/usr/lib/osxcross', '/usr/local')


def main():
   app_path = os.environ['MESON_INSTALL_DESTDIR_PREFIX']

   # Create a directory for dynamic libraries.
   # Docs allege Contents/Resources/lib is more idiomatic, but bundle.sh chose here.
   trace(shutil.rmtree, f'{app_path}/Contents/Frameworks', ignore_errors=True)
   trace(os.makedirs, f'{app_path}/Contents/Frameworks')

   # Gather Naev and dependencies.
   trace(copy_with_deps, f'{os.environ["MESON_BUILD_ROOT"]}/naev', app_path, dest='Contents/MacOS')
   if '-d' in sys.argv:
      trace(subprocess.check_call, ['dsymutil', f'{app_path}/Contents/MacOS/naev'])

   print(f'Successfully created {app_path}')


def trace(f, *args, **kwargs):
   """ Run the given function on the given arguments. In debug mode, print the function call. """

   if os.getenv('MESON_INSTALL_QUIET') is None:
      # HACK: when tracing a function call, use unittest.mock to construct a pretty-printable version.
      # HACK: when tracing subprocess shell commands, format them because it's unreadable otherwise.
      if f.__module__ == 'subprocess' and len(args) == 1:
         from subprocess import list2cmdline
         print('$', list2cmdline(args[0]))
      else:
         from unittest import mock
         call = getattr(mock.call, f.__name__)(*args, **kwargs)
         print(str(call).replace('call.', '', 1))
   return f(*args, **kwargs)


def copy_with_deps(bin_src, app_path, dest='Contents/Frameworks'):
   """ Copy the binary into the bundle destination, while resolving and rewriting dependencies. """

   loader_path = os.path.dirname(bin_src)
   bin_name = os.path.basename(bin_src)
   bin_dst = os.path.join(app_path, dest, bin_name)
   rpaths_to_delete = set()

   os.makedirs(os.path.dirname(bin_dst), exist_ok=True)
   trace(shutil.copy, bin_src, bin_dst)

   dylibs, rpaths = otool_results(bin_src)
   # Now we have a "dylibs" list, where some entries may be "@rpath/" relative,
   # and we have a "rpaths" list, where some entries may be "@loader_path/" relative.

   change_cmd = [host_program('install_name_tool')]
   if bin_dst.endswith('/naev'):
      change_cmd.extend(['-add_rpath', '@executable_path/../Frameworks'])
   if bin_dst.endswith('.dylib'):
      change_cmd.extend(['-id', f'@rpath/{bin_name}'])

   for dylib in dylibs:
      dylib_name = os.path.basename(dylib)
      dylib_path, matching_rpath = find_dylib_dependency(dylib, rpaths, loader_path)
      if dylib_path:
         if not os.path.exists(os.path.join(app_path, dest, dylib_name)):
            trace(copy_with_deps, dylib_path, app_path)
         change_cmd.extend(['-change', dylib, f'@rpath/{dylib_name}'])
      if matching_rpath:
         rpaths_to_delete.add(matching_rpath)

   for rpath in rpaths_to_delete:
      change_cmd.extend(['-delete_rpath', rpath])

   change_cmd.append(bin_dst)
   trace(subprocess.check_call, change_cmd)


def otool_results(bin_src):
   """ Look at a host binary's load instructions using "otool", and return
   (0) the list of dylibs it says to load,
   (1) the list of rpaths it says to search. """

   operands = {'LC_LOAD_DYLIB': [], 'LC_RPATH': []}
   operand_list = unwanted = []
   for line in subprocess.check_output([host_program('otool'), '-l', bin_src]).decode().splitlines():
      line = line.lstrip()
      if line.startswith('cmd '):
         operand_list = operands.get(line[4:], unwanted)
      elif line.startswith(('name ', 'path ')):
         operand_list.append(line[5:].rsplit(' ', 2)[0]) # "name /some/path (offset 42)" -> "/some/path"
   return operands['LC_LOAD_DYLIB'], operands['LC_RPATH']


def find_dylib_dependency(dylib, rpaths, loader_path):
   """ Return the path to the dependency "dylib", and the value from "rpaths" that found it.
      Also search the build directory for subproject or local dylibs.
   """
   build_root = os.environ.get('MESON_BUILD_ROOT', '')

   if dylib.startswith(LOCAL_LIB_ROOTS):
      lib_dir = os.path.realpath(os.path.dirname(dylib))
      for rpath in list(rpaths):
         if os.path.realpath(rpath) == lib_dir:
            return dylib, rpath
      return dylib, None
   elif dylib.startswith('@rpath/'):
      lib_base = dylib.replace('@rpath/', '', 1)
      # Try rpaths
      for rpath in list(rpaths):
         trial_path = os.path.join(rpath.replace('@loader_path', loader_path), lib_base)
         if os.path.exists(trial_path):
            return trial_path, rpath
      # Try build directory (for subprojects or local builds)
      if build_root:
         for root, dirs, files in os.walk(build_root):
            if lib_base in files:
               return os.path.join(root, lib_base), None
   elif not os.path.isabs(dylib):
      # Relative path: search build directory
      if build_root:
         for root, dirs, files in os.walk(build_root):
            if os.path.basename(dylib) in files:
               return os.path.join(root, os.path.basename(dylib)), None
   else:
      # Absolute path: check if it's in build directory
      if build_root and dylib.startswith(build_root):
         return dylib, None
   return None, None


def host_program(name):
   """ Turn a program name like 'gcc' into one like 'x86_64-apple-darwin17-gcc'. """

   try:
      return os.environ['HOST'] + '-' + name
   except KeyError:
      return name


if __name__ == '__main__':
   main()

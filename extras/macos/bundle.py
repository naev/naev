#!/usr/bin/env python3

# MACOS PACKAGING SCRIPT FOR NAEV
# This script should be run after compiling Naev.

# This script assumes a dependency must be bundled (with priority over any installations the end user might have) if either
# | it's in /usr/local (Homebrew?)
# | it's relative to the rpath (Meson subproject / embedded lib)

import argparse
import os
import shutil
import subprocess


LOCAL_LIB_ROOTS = ('/usr/local',)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--debug", help="Trace the script's actions.", action="store_true")
    parser.add_argument("-n", "--nightly", help="Set this for nightly builds.", action="store_true")
    parser.add_argument("-s", "--sourceroot", help="Set this to the repo location.", default=os.path.abspath('.'))
    parser.add_argument("-b", "--buildroot", help="Set this to the build location.", default=os.path.abspath('build'))
    parser.add_argument("-o", "--buildoutput", help="Set location for dist output.", default=os.path.abspath('dist'))
    args = parser.parse_args()

    trace.enabled = args.debug
    app_path = os.path.join(args.buildoutput, 'Naev.app')

    #Clean previous build.
    trace(shutil.rmtree, app_path, ignore_errors=True)

    # Build basic structure.
    for subdir in 'MacOS', 'Resources', 'Frameworks':
        trace(os.makedirs, os.path.join(app_path, 'Contents', subdir), exist_ok=True)
    trace(shutil.copy, os.path.join(args.buildroot, 'Info.plist'), os.path.join(app_path, 'Contents'))
    trace(shutil.copy, os.path.join(args.sourceroot, 'extras/macos/naev.icns'), os.path.join(app_path, 'Contents/Resources'))

    # Gather Naev and dependencies.
    trace(copy_with_deps, os.path.join(args.buildroot, 'naev'), app_path, dest='Contents/MacOS')

    # Strip headers, especially from the SDL2 framework.
    trace(subprocess.check_call, ['find', app_path, '-name', 'Headers', '-prune', '-exec', 'rm', '-r', '{}', '+'])

    print(f'Successfully created {app_path}')
    print(f'*** NOTE: This script expects other steps to create {app_path}/Contents/Resources/dat')


def trace(f, *args, **kwargs):
    """ Run the given function on the given arguments. In debug mode, print the function call.
        (Indent if we trace while running a traced function.) """

    trace.depth += 1
    try:
        if trace.enabled:
            # HACK: when tracing a function call, use unittest.mock to construct a pretty-printable version.
            # HACK: when tracing subprocess shell commands, format them because it's unreadable otherwise.
            if f.__module__ == 'subprocess' and len(args) == 1:
                from subprocess import list2cmdline
                print(f'[{trace.depth}]', '$', list2cmdline(args[0]))
            else:
                from unittest import mock
                call = getattr(mock.call, f.__name__)(*args, **kwargs)
                pretty = str(call).replace('call.', '', 1)
                print(f'[{trace.depth}]', pretty)
        return f(*args, **kwargs)
    finally:
        trace.depth -= 1
trace.depth = 0
trace.enabled = False


def copy_with_deps(bin_src, app_path, dest='Contents/Frameworks', exe_rpaths=None):
    loader_path = os.path.dirname(bin_src)
    bin_name = os.path.basename(bin_src)
    bin_dst = os.path.join(app_path, dest, bin_name)

    trace(shutil.copy, bin_src, bin_dst)

    dylibs, rpaths = otool_results(bin_src)
    # Now we have a "dylibs" list, where some entries may be "@rpath/" relative,
    # and we have a "rpaths" list, where some entries may be "@loader_path/" relative.
    # The "rpaths" list only matters when it comes from the Naev executable, so record that result
    if exe_rpaths is None:
        exe_rpaths = list(rpaths)

    change_cmd = ['install_name_tool']
    if bin_dst.endswith('/naev'):
        change_cmd.extend(['-add_rpath', '@executable_path/../Frameworks'])
    if bin_dst.endswith('.dylib'):
        change_cmd.extend(['-id', f'@rpath/{bin_name}'])

    for dylib in dylibs:
        dylib_name = os.path.basename(dylib)
        dylib_path = search_and_pop_rpath(dylib, exe_rpaths, loader_path)
        if dylib_path:
            if not os.path.exists(os.path.join(app_path, dest, dylib_name)):
                trace(copy_with_deps, dylib_path, app_path)
            change_cmd.extend(['-change', dylib, f'@rpath/{dylib_name}'])

    for rpath in set(rpaths).difference(exe_rpaths):
        change_cmd.extend(['-delete_rpath', rpath])

    change_cmd.append(bin_dst)
    trace(subprocess.check_call, change_cmd)


def otool_results(bin_src):
    operands = {'LC_LOAD_DYLIB': [], 'LC_RPATH': []}
    operand_list = unwanted = []
    for line in subprocess.check_output(['otool', '-l', bin_src]).decode().splitlines():
        line = line.lstrip()
        if line.startswith('cmd '):
            operand_list = operands.get(line[4:], unwanted)
        elif line.startswith(('name ', 'path ')):
            operand_list.append(line[5:].rsplit(' ', 2)[0]) # "name /some/path (offset 42)" -> "/some/path"
    return operands['LC_LOAD_DYLIB'], operands['LC_RPATH']


def search_and_pop_rpath(dylib, rpaths, loader_path):
    dylib_path = None
    if dylib.startswith(LOCAL_LIB_ROOTS):
        dylib_path = dylib
        lib_dir = os.path.realpath(os.path.dirname(dylib))
        for rpath in list(rpaths):
            if os.path.realpath(rpath) == lib_dir:
                rpaths.remove(rpath)
    elif dylib.startswith('@rpath/'):
        lib_base = dylib.replace('@rpath/', '', 1)
        for rpath in list(rpaths):
            trial_path = os.path.join(rpath.replace('@loader_path', loader_path), lib_base)
            if os.path.exists(trial_path):
                dylib_path = dylib_path or trial_path  # We should bundle the first matching lib.
                rpaths.remove(rpath)
    return dylib_path


if __name__ == '__main__':
    main()

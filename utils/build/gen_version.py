#!/usr/bin/env python3

import os
import subprocess
import sys

def get_version(source_root):
    version_file = os.path.join(source_root, 'dat', 'VERSION')
    if os.path.isdir(os.path.join(source_root, '.git')):
        # In the git repo. Build the tag from git info
        try:
            git_describe = subprocess.run(['git', '-C', source_root, 'describe', '--tags', '--match', 'v*', '--dirty'], capture_output=True, text=True)
            git_output = git_describe.stdout.strip()
            version_parts = git_output[1:].split('-')
            version = version_parts[0] + '-' + version_parts[1].replace('.', '.') + '+' + '.'.join(version_parts[2:]).replace('-', '+').replace('.dirty', '+dirty')
            with open(version_file, 'w') as f:
                f.write(version)
        except subprocess.CalledProcessError:
            version = ''
    elif os.path.isfile(version_file):
        # In a source package. Version file should exist.
        with open(version_file, 'r') as f:
            version = f.read().strip()
    else:
        # Couldn't find git repo or a packaged VERSION file
        # Did you download the zip'd repo instead of a release?
        version = f"{sys.argv[1]}+dev"
        with open(version_file, 'w') as f:
            f.write(version)
    return version

if __name__ == "__main__":
    source_root = os.getenv('MESON_SOURCE_ROOT')
    if source_root:
        version = get_version(source_root)
        print(version)
    else:
        print("Error: MESON_SOURCE_ROOT environment variable not set.")

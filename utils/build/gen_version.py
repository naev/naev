#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys
import logging

# Configure logging to output to stderr
logging.basicConfig(level=logging.INFO, stream=sys.stderr)

def parse_version(version_str):
    try:
        version_parts = version_str.split('-')
        if len(version_parts) >= 2:
            version = version_parts[0] + '-' + version_parts[1].replace('.', '.') + '+' + '.'.join(version_parts[2:]).replace('-', '+')
        else:
            version = version_str
        return version
    except Exception as e:
        logging.error(f"Error parsing version string: {e}")
        return ''

def get_version(source_root, version_match):
    version_file = os.path.join(source_root, 'dat', 'VERSION')
    version = ""

    if os.path.isdir(os.path.join(source_root, '.git')):
        # In the git repo. Build the tag from git info
        try:
            git_describe = subprocess.run(['git', '-C', source_root, 'describe', '--tags', '--match', version_match, '--dirty'], capture_output=True, text=True)
            git_output = git_describe.stdout.strip()
            version = parse_version(git_output[1:])
            with open(version_file, 'w') as f:
                f.write(version)
        except subprocess.CalledProcessError:
            pass

    if not version and os.path.isfile(version_file):
        # In a source package. Version file should exist.
        with open(version_file, 'r') as f:
            version = f.read().strip()

    if not version:
        # Couldn't find git repo or a packaged VERSION file
        # Did you clone the repo with --depth=1?
        # Did you download the zip'd repo instead of a release?
        if len(sys.argv) > 1:
            version = f"{version_match}+dev"
        else:
            version = "0.0.0+dev"
        with open(version_file, 'w') as f:
            f.write(version)

    return version

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument('-v', '--version-match', type=str, default='v*', help="Tags to match when generating description based on git.")
    args = ap.parse_args()

    source_root = os.getenv('MESON_SOURCE_ROOT')
    if source_root:
        version = get_version( source_root, args.version_match )
        logging.info(f"Version retrieved: {version}")
        print(version)
    else:
        logging.error("Error: MESON_SOURCE_ROOT environment variable not set.")

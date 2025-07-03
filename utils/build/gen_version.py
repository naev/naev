#!/usr/bin/env python3

import os
import subprocess
import sys
import logging

# Configure logging to output to stderr
logging.basicConfig(level=logging.INFO, stream=sys.stderr)

def parse_version(version_str):
    """
    Parses a git describe version string into a standardized version format.
    Handles tags with additional commit/dirtiness info.
    """
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

def get_matching_tag(source_root, version_prefix):
    """
    Returns the latest tag that matches the given version prefix (e.g., 'v0.13').
    Looks for tags in the git repository that start with the specified prefix.
    """
    try:
        tags_proc = subprocess.run(
            ['git', '-C', source_root, 'tag', '--list', f'v{version_prefix}*', '--sort=-v:refname'],
            capture_output=True, text=True, check=True
        )
        tags = tags_proc.stdout.strip().splitlines()
        if tags:
            return tags[0]
    except Exception as e:
        logging.warning(f"Could not find matching tag for version {version_prefix}: {e}")
    return None

def get_version(source_root):
    """
    Determines the project version using git tags or a VERSION file.
    Prefers a git tag matching the provided version argument, if available.
    Falls back to the VERSION file or a default dev version if necessary.
    """
    version_file = os.path.join(source_root, 'dat', 'VERSION')
    version = ""

    if os.path.isdir(os.path.join(source_root, '.git')):
        # In the git repo. Build the tag from git info
        try:
            version_arg = sys.argv[1] if len(sys.argv) > 1 else None
            tag_to_match = None
            if version_arg:
                tag_to_match = get_matching_tag(source_root, version_arg)
            if tag_to_match:
                git_describe = subprocess.run(
                    ['git', '-C', source_root, 'describe', '--tags', '--match', tag_to_match, '--dirty'],
                    capture_output=True, text=True, check=True
                )
                git_output = git_describe.stdout.strip()
                version = parse_version(git_output[1:])
            else:
                git_describe = subprocess.run(
                    ['git', '-C', source_root, 'describe', '--tags', '--match', 'v*', '--dirty'],
                    capture_output=True, text=True, check=True
                )
                git_output = git_describe.stdout.strip()
                version = parse_version(git_output[1:])
            if version:
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
            version = f"{sys.argv[1]}+dev"
        else:
            version = "0.0.0+dev"
        with open(version_file, 'w') as f:
            f.write(version)

    return version

if __name__ == "__main__":
    source_root = os.getenv('MESON_SOURCE_ROOT')
    if source_root:
        version = get_version(source_root)
        logging.info(f"Version retrieved: {version}")
        print(version)
    else:
        logging.error("Error: MESON_SOURCE_ROOT environment variable not set.")

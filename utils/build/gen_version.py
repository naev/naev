#!/usr/bin/env python3

import os
import subprocess
import sys

def get_version(source_root):
    version = ""
    if os.path.isdir(os.path.join(source_root, ".git")):
        # In the git repo. Build the tag from git info
        try:
            git_describe = subprocess.run(
                ["git", "-C", source_root, "describe", "--tags", "--match", "v*", "--dirty"],
                capture_output=True,
                text=True,
                check=True,
            ).stdout.strip()
            version = git_describe.replace("v", "").replace("-dirty", ".dirty")
        except subprocess.CalledProcessError:
            pass
        else:
            with open(os.path.join(source_root, "dat", "VERSION"), "w") as version_file:
                version_file.write(version)
    if not version:
        version_file_path = os.path.join(source_root, "dat", "VERSION")
        if os.path.isfile(version_file_path):
            # In a source package. Version file should exist.
            with open(version_file_path, "r") as version_file:
                version = version_file.read().strip()
        else:
            # Couldn't find git repo or a packaged VERSION file
            # Did you download the zip'd repo instead of a release?
            version = f"{sys.argv[1]}+dev"
            with open(version_file_path, "w") as version_file:
                version_file.write(version)
    return version

if __name__ == "__main__":
    SOURCE_ROOT = os.getenv("MESON_SOURCE_ROOT")
    if SOURCE_ROOT:
        version = get_version(SOURCE_ROOT)
        print(version)
    else:
        print("Error: MESON_SOURCE_ROOT environment variable not set.")

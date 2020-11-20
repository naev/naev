#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
import pefile
import shutil


DLL_ROOTS = ["/mingw64/bin", "/usr/x86_64-w64-mingw32/bin"]


def extract(exepath, outdir):
    os.makedirs(outdir, exist_ok=True)
    for path in dll_paths(exepath):
        print("Copying", path, "to", outdir)
        shutil.copy(path, outdir)


def dll_paths(path):
    paths = set()

    def search(path):
        if path not in paths and os.path.exists(path):
            paths.add(path)
            for imp in pefile.PE(path).DIRECTORY_ENTRY_IMPORT:
                for root in DLL_ROOTS:
                    search(os.path.join(root, imp.dll.decode()))

    search(path)
    return paths


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("exepath")
    parser.add_argument("outdir")
    args = parser.parse_args()
    extract(args.exepath, args.outdir)

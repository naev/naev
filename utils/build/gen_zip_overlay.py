#!/usr/bin/env python3

import argparse
import os
import sys
import zipfile

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit(f'Usage: {sys.argv[0]} outfile.zip [infile] [--cd dat_subdir] ...')

    output = sys.argv[1]
    inputs = iter(sys.argv[2:])
    cwd = ''

    with zipfile.ZipFile(output, 'w') as zout:
        for path in inputs:
            if path == '--cd':
                cwd = next(inputs, '')
                continue
            zout.write(path, os.path.join(cwd, os.path.basename(path)))

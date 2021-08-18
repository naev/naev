#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later

import glob
import os
import sys

from obj_view.window import Window
from obj_view.parse import parse_obj

ROOT = '../../artwork/gfx/ship/3d'

window = Window(0)

stdout = sys.stdout
sys.stdout = open(os.devnull, 'w')

for path in glob.glob(f'{ROOT}/*/*/*.obj'):
    try:
        ship = parse_obj(path)
    except Exception as e:
        stdout.write(f"Error loading '{path}': {e!r}\n")

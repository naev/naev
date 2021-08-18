#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later

import os
import sys

from obj_view.window import Window
from obj_view.parse import parse_obj

window = Window(0)

stdout = sys.stdout
sys.stdout = open(os.devnull, 'w')

for i in os.listdir('3d'):
    if i.startswith('.') or i.startswith('viper'):
        continue

    try:
        ship = parse_obj(f"3d/{i}/{i}.obj")
    except Exception as e:
        stdout.write(f"Error loading '{i}.obj': {e}\n")

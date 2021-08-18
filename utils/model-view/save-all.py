#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later

import glob
import os
import math
import shutil

from OpenGL.GL import *

from obj_view.window import Window
from obj_view.parse import parse_obj
from obj_view.fb import Framebuffer
from obj_view.render import ObjProgram, RenderObject

shutil.rmtree('png-exports', ignore_errors=True)
os.mkdir('png-exports')

ROOT = '../../artwork/gfx/ship/3d'
RES = 2048

window = Window(0)
fb = Framebuffer(RES, RES)
glViewport(0, 0, RES, RES)

program = ObjProgram()
program.use()

for path in glob.glob(f'{ROOT}/*/*/*.obj'):
    glClearColor(0., 0., 0., 0.)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    try:
        print(f"Rendering '{path}'...")
        ship = parse_obj(path)
    except Exception as e:
        print(f"Error parsing '{path}': {e!r}")
        continue

    for obj in ship.values():
        obj = RenderObject(program, obj)
        program.draw(obj, RES, RES, rot=0.)
    fb.save(os.path.join('png-exports', os.path.basename(path)+'.png'))

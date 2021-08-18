#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later

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

RES = 2048

window = Window(0)
fb = Framebuffer(RES, RES)
glViewport(0, 0, RES, RES)

program = ObjProgram()
program.use()

for i in os.listdir('3d'):
    if i.startswith('.') or i.startswith('viper'):
        continue

    glClearColor(0., 0., 0., 0.)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    try:
        print(f"Rendering '{i}.obj'...")
        ship = parse_obj(f"3d/{i}/{i}.obj")
    except Exception as e:
        print(f"Error parsing '{i}.obj': {e}")
        continue

    for obj in ship.values():
        obj = RenderObject(program, obj)
        program.draw(obj, RES, RES, math.pi / 2)
    fb.save(f"png-exports/{i}.png")

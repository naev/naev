#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later

# TODO: antialias edges of model
# TODO: diffuse
# TODO: file format: interleaved, extendable
# TODO: uniform buffer object
# TODO: https://stackoverflow.com/questions/50806126/why-are-textures-displayed-incorrectly-when-using-indexed-rendering-gldraweleme
# TODO: handle material switches within one object (combine into atlas texture? texture array? multiple draws?)
# TODO: bump map
# TODO: specral, emit, l (used in peacemaker.obj?)
# https://people.cs.clemson.edu/~dhouse/courses/405/docs/brief-mtl-file-format.html
# illum 2 is Blinn–Phong reflection model
# map files in a .mtl are square power of two
# TODO: make sure bump mapping is correct

import math
import argparse

from OpenGL.GL import *

from obj_view.parse import parse_obj
from obj_view.window import Window
from obj_view.fb import Framebuffer
from obj_view.render import ObjProgram, RenderObject


parser = argparse.ArgumentParser()
parser.add_argument('obj')
parser.add_argument('--rot', type=int, default=0)
parser.add_argument('--res', type=int, default=256)
parser.add_argument('--save')
parser.add_argument('--exit', action='store_true')
args = parser.parse_args()

rot = args.rot * math.pi / 180

window = Window(rot)
ship = parse_obj(args.obj)

if args.save is not None:
    fb = Framebuffer(args.res, args.res)

    glViewport(0, 0, args.res, args.res)

    program = ObjProgram()
    program.use()

    glClearColor(0., 0., 0., 0.)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    for obj in ship.values():
        obj = RenderObject(program, obj)
        program.draw(obj, args.res, args.res, rot)

    fb.save(args.save)
elif not args.exit:
    window.main_loop(ship)

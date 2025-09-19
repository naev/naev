# SPDX-Licence-Identifier: GPL-3.0-or-later

import math

from OpenGL.GLUT import *
from OpenGL.GL import *

from obj_view.render import ObjProgram, RenderObject

# Rotations per second
ROT_RATE = 0.5

class Window:
   prev_time = 0
   rot_dir = 0
   rot_z_dir = 0
   width = 1280
   height = 720
   objs = None

   def __init__(self, rot):
      glutInit("")
      glutInitContextVersion(3, 2)
      glutInitContextProfile(GLUT_CORE_PROFILE)
      glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE)
      glutInitWindowSize(self.width, self.height)
      glutInitWindowPosition(0, 0)
      self.window = glutCreateWindow("Obj Viewer")
      self.rot = rot
      self.rot_z = 0

      glEnable( GL_FRAMEBUFFER_SRGB )

   def main_loop(self, objs):
      self.program = ObjProgram()
      self.program.use()
      self.objs = [RenderObject(self.program, i) for i in objs.values()]

      glutDisplayFunc(self.display_cb)
      glutReshapeFunc(self.reshape_cb)
      glutSpecialFunc(self.keyboard_special_down)
      glutSpecialUpFunc(self.keyboard_special_up)
      glutKeyboardFunc(self.keyboard_down_cb)
      glutKeyboardUpFunc(self.keyboard_up_cb)
      glutMainLoop()

   def display_cb(self):
      cur_time = glutGet(GLUT_ELAPSED_TIME)
      dt = cur_time - self.prev_time
      self.prev_time = cur_time

      self.rot   += self.rot_dir * dt * (2. * math.pi) * ROT_RATE / 1e3
      self.rot_z  += self.rot_z_dir * dt * (2. * math.pi) * ROT_RATE / 1e3

      glClearColor(0.1, 0.1, 0.1, 1.)
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

      for obj in self.objs:
         self.program.draw( obj, self.width, self.height, self.rot, self.rot_z )

      glutSwapBuffers()
      glutPostRedisplay()

   def reshape_cb(self, w, h):
      self.width = w
      self.height = h
      glViewport(0, 0, w, h)

   def keyboard_special_down(self, key, x, y):
      if key == GLUT_KEY_LEFT:
         self.rot_dir = 1
      elif key == GLUT_KEY_RIGHT:
         self.rot_dir = -1
      elif key == GLUT_KEY_UP:
         self.rot_z_dir = 1
      elif key == GLUT_KEY_DOWN:
         self.rot_z_dir = -1

   def keyboard_special_up(self, key, x, y):
      if key == GLUT_KEY_LEFT or key == GLUT_KEY_RIGHT:
         self.rot_dir = 0
      elif key == GLUT_KEY_UP or key == GLUT_KEY_DOWN:
         self.rot_z_dir = 0

   def keyboard_down_cb(self, key, x, y):
      if key == b'\x1b' or key == b'q':
         glutDestroyWindow(self.window)
      elif key == b'a':
         self.rot_dir = 1
      elif key == b'd':
         self.rot_dir = -1
      elif key == b'w':
         self.rot_z_dir = 1
      elif key == b's':
         self.rot_z_dir = -1

   def keyboard_up_cb(self, key, x, y):
      if key in b'ad':
         self.rot_dir = 0
      elif key in b'ws':
         self.rot_z_dir = 0

# SPDX-License-Identifier: GPL-3.0-or-later

from OpenGL.GL import *
from PIL import Image

class Framebuffer:
    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.fb = glGenFramebuffers(1)

        self.bind()

        color_buffer = glGenRenderbuffers(1)
        glBindRenderbuffer(GL_RENDERBUFFER, color_buffer)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer)

        depth_buffer = glGenRenderbuffers(1)
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer)

    def bind(self):
        glBindFramebuffer(GL_FRAMEBUFFER, self.fb)

    def save(self, path):
        self.bind()
        data = glReadPixels(0, 0, self.w, self.h, GL_RGBA, GL_UNSIGNED_BYTE)
        image = Image.frombytes('RGBA', (self.w, self.h), data)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        image.save(path)

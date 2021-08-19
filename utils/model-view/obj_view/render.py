# SPDX-License-Identifier: GPL-3.0-or-later

from ctypes import *

from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram, compileShader
from OpenGL.arrays.vbo import VBO
import glm
import math

base_path = "./"

vert = '#version 150\n\n' + open(base_path+'dat/glsl/material.vert').read()
frag = '#version 150\n\n' + open(base_path+'dat/glsl/material.frag').read()

class RenderObject:
    def __init__(self, program, obj):
        self.mtl_list = obj.mtl_list
        self.vao = glGenVertexArrays(1)

        glBindVertexArray(self.vao)

        vertices = (GLfloat * len(obj.vertices))(*obj.vertices);
        self.vbo = VBO(vertices, GL_STATIC_DRAW, GL_ARRAY_BUFFER)
        self.vbo.bind()

        vertex_attrib = program.attribs["vertex"]
        glEnableVertexAttribArray(vertex_attrib)
        glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(0))

        vertex_tex_attrib = program.attribs["vertex_tex"]
        glEnableVertexAttribArray(vertex_tex_attrib)
        glVertexAttribPointer(vertex_tex_attrib, 2, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(3 * 4))

        vertex_normal_attrib = program.attribs["vertex_normal"]
        glEnableVertexAttribArray(vertex_normal_attrib)
        glVertexAttribPointer(vertex_normal_attrib, 2, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(5 * 4))


class ObjProgram:
    def __init__(self):
        vertex_shader = compileShader(vert, GL_VERTEX_SHADER)
        fragment_shader = compileShader(frag, GL_FRAGMENT_SHADER)
        self.program = compileProgram(vertex_shader, fragment_shader)

        uniform_count = glGetProgramiv(self.program, GL_ACTIVE_UNIFORMS)
        self.uniforms = {glGetActiveUniform(self.program, i)[0].decode() : i for i in range(uniform_count)}

        attrib_count = glGetProgramiv(self.program, GL_ACTIVE_ATTRIBUTES)
        self.attribs = {glGetActiveAttrib(self.program, i)[0].decode() : i for i in range(attrib_count)}

    def use(self):
        glUseProgram(self.program)

    def draw(self, obj, width, height, rot):
        glBindVertexArray(obj.vao)

        scale = 5  # matching "camobj" setup in naev-artwork/3D/render.sh

        if width < height:
            scale_w = scale
            scale_h = scale_w * (height / width)
        else:
            scale_h = scale
            scale_w = scale_h * (width / height)

        projection = glm.ortho(-scale_w, scale_w, -scale_h, scale_h, -9*math.sqrt(2), 9*math.sqrt(2))
        projection = glm.rotate(projection, math.pi / 4, glm.vec3(1, 0, 0))
        projection = glm.rotate(projection, rot + math.pi / 2, glm.vec3(0, 1, 0))
        glUniformMatrix4fv(self.uniforms["projection"], 1, GL_FALSE, projection.to_list())

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LESS)

        glUniform1i(self.uniforms["map_Kd"], 0)
        glUniform1i(self.uniforms["map_Bump"], 1)
        
        for (mtl, start, count) in obj.mtl_list:
            glUniform3f(self.uniforms["Kd"], *mtl.Kd)
            glUniform3f(self.uniforms["Ka"], *mtl.Ka)
            glUniform1f(self.uniforms["d"], mtl.d)
            glUniform1f(self.uniforms["bm"], mtl.bm)

            glActiveTexture(GL_TEXTURE0)
            glBindTexture(GL_TEXTURE_2D, mtl.map_Kd)

            glActiveTexture(GL_TEXTURE1)
            glBindTexture(GL_TEXTURE_2D, mtl.map_Bump)

            glDrawArrays(GL_TRIANGLES, start, count)

        self.gl_checkErr()

    def gl_checkErr(self):
        err = glGetError()
        if err != 0:
            print(gluErrorString(err))

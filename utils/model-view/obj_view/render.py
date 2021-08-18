# SPDX-License-Identifier: GPL-3.0-or-later

from ctypes import *

from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram, compileShader
from OpenGL.arrays.vbo import VBO
import glm
import math

vert = """
#version 150

uniform mat4 trans;

in vec4 vertex;
in vec3 normal;
in vec2 tex;
out vec2 tex_out;
out vec3 normal_out;

void main(void) {
   tex_out = tex;
   normal_out = normal;
   gl_Position = trans * vertex;
}
"""


frag = """
#version 150

uniform mat4 trans;

uniform sampler2D map_Kd, map_Bump;

uniform vec3 Ka, Kd;
uniform float d, bm;

in vec2 tex_out;
in vec3 normal_out;
out vec4 color_out;

const vec3 lightDir = vec3(0, 0, -1);

void main(void) {
   float normal_ratio = step(.01, bm);
   vec3 norm = (1. - normal_ratio) * normal_out;
   norm += normal_ratio * bm * texture(map_Bump, tex_out).xyz;
   norm = normalize((trans * vec4(norm, 1.)).xyz);

   vec3 ambient = Ka;

   vec3 diffuse = Kd * max(dot(norm, lightDir), 0.0);

   color_out = texture(map_Kd, tex_out);
   color_out.rgb *= .4 * ambient + .7 * diffuse;
   color_out.a = d;
}
"""


class RenderObject:
    def __init__(self, program, obj):
        self.mtl_list = obj.mtl_list
        self.radius = obj.radius
        self.vao = glGenVertexArrays(1)

        glBindVertexArray(self.vao)

        vertices = (GLfloat * len(obj.vertices))(*obj.vertices);
        self.vbo = VBO(vertices, GL_STATIC_DRAW, GL_ARRAY_BUFFER)
        self.vbo.bind()

        vertex_attrib = program.attribs["vertex"]
        glEnableVertexAttribArray(vertex_attrib)
        glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(0))

        tex_attrib = program.attribs["tex"]
        glEnableVertexAttribArray(tex_attrib)
        glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(3 * 4))

        normal_attrib = program.attribs["normal"]
        glEnableVertexAttribArray(normal_attrib)
        glVertexAttribPointer(normal_attrib, 2, GL_FLOAT, GL_FALSE, 8 * 4, c_void_p(5 * 4))


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

        scale = obj.radius

        if width < height:
            scale_w = scale
            scale_h = scale_w * (height / width)
        else:
            scale_h = scale
            scale_w = scale_h * (width / height)

        trans = glm.ortho(-scale_w, scale_w, -scale_h, scale_h, -scale, scale)
        trans = glm.rotate(trans, math.pi / 4, glm.vec3(1, 0, 0))
        trans = glm.rotate(trans, rot, glm.vec3(0, 1, 0))
        glUniformMatrix4fv(self.uniforms["trans"], 1, GL_FALSE, trans.to_list())

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

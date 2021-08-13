#!/usr/bin/env python3

import collections

Shader = collections.namedtuple("Shader", "name vs_path fs_path attributes uniforms subroutines")

class Shader:
    def __init__(self, name, vs_path, fs_path, attributes, uniforms, subroutines ):
        self.name       = name
        self.vs_path    = vs_path
        self.fs_path    = fs_path
        self.attributes = attributes
        self.uniforms   = uniforms
        self.subroutines= subroutines

    def write_header(self, f):
        f.write("   struct {\n")
        f.write("      GLuint program;\n")
        for attribute in self.attributes:
            f.write(f"      GLuint {attribute};\n")
        for uniform in self.uniforms:
            f.write(f"      GLuint {uniform};\n")
        for subroutine, routines in self.subroutines.items():
            f.write("      struct {\n")
            f.write("         GLuint uniform;\n")
            for r in routines:
                f.write(f"         GLuint {r};\n")
            f.write(f"      }} {subroutine};\n")
        f.write(f"   }} {self.name};\n")

    def write_source(self, f):
        f.write(f"   shaders.{self.name}.program = gl_program_vert_frag(\"{self.vs_path}\", \"{self.fs_path}\");\n")
        for attribute in self.attributes:
            f.write(f"   shaders.{self.name}.{attribute} = glGetAttribLocation(shaders.{self.name}.program, \"{attribute}\");\n")
        for uniform in self.uniforms:
            f.write(f"   shaders.{self.name}.{uniform} = glGetUniformLocation(shaders.{self.name}.program, \"{uniform}\");\n")
        if len(self.subroutines) > 0:
            f.write("   if (gl_has( OPENGL_SUBROUTINES )) {\n")
            for subroutine, routines in self.subroutines.items():
                f.write(f"      shaders.{self.name}.{subroutine}.uniform = glGetSubroutineUniformLocation( shaders.{self.name}.program, GL_FRAGMENT_SHADER, \"{subroutine}\" );\n")
                for r in routines:
                    f.write(f"      shaders.{self.name}.{subroutine}.{r} = glGetSubroutineIndex( shaders.{self.name}.program, GL_FRAGMENT_SHADER, \"{r}\" );\n")
            f.write("   }\n");


class SimpleShader(Shader):
    def __init__(self, name, fs_path):
        super().__init__( name=name, vs_path="project_pos.vert", fs_path=fs_path, attributes=["vertex"], uniforms=["projection","color","dimensions"], subroutines={} )
    def write_header(self, f):
        f.write(f"   SimpleShader {self.name};\n")


SHADERS = [
   Shader(
      name = "circle",
      vs_path = "circle.vert",
      fs_path = "circle.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "radius"],
      subroutines = {},
   ),
   Shader(
      name = "circle_filled",
      vs_path = "circle.vert",
      fs_path = "circle_filled.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "radius"],
      subroutines = {},
   ),
   Shader(
      name = "circle_partial",
      vs_path = "circle.vert",
      fs_path = "circle_partial.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "radius", "angle1", "angle2"],
      subroutines = {},
   ),
   Shader(
      name = "solid",
      vs_path = "project.vert",
      fs_path = "solid.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color"],
      subroutines = {},
   ),
   Shader(
      name = "trail",
      vs_path = "project_pos.vert",
      fs_path = "trail.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "c1", "c2", "t1", "t2", "dt", "pos1", "pos2", "r", "nebu_col" ],
      subroutines = {
        "trail_func" : [
            "trail_default",
            "trail_pulse",
            "trail_wave",
            "trail_flame",
            "trail_nebula",
            "trail_arc",
            "trail_bubbles",
        ]
      }
   ),
   Shader(
      name = "smooth",
      vs_path = "smooth.vert",
      fs_path = "smooth.frag",
      attributes = ["vertex", "vertex_color"],
      uniforms = ["projection"],
      subroutines = {},
   ),
   Shader(
      name = "texture",
      vs_path = "texture.vert",
      fs_path = "texture.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "tex_mat"],
      subroutines = {},
   ),
   Shader(
      name = "texture_interpolate",
      vs_path = "texture.vert",
      fs_path = "texture_interpolate.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "tex_mat", "sampler1", "sampler2", "inter"],
      subroutines = {},
   ),
   Shader(
      name = "nebula",
      vs_path = "nebula.vert",
      fs_path = "nebula_overlay.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "brightness", "horizon", "eddy_scale", "time"],
      subroutines = {},
   ),
   Shader(
      name = "nebula_background",
      vs_path = "nebula.vert",
      fs_path = "nebula_background.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "brightness", "eddy_scale", "time"],
      subroutines = {},
   ),
   Shader(
      name = "nebula_map",
      vs_path = "nebula_map.vert",
      fs_path = "nebula_map.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "eddy_scale", "time", "globalpos", "alpha"],
      subroutines = {},
   ),
   Shader(
      name = "stars",
      vs_path = "stars.vert",
      fs_path = "stars.frag",
      attributes = ["vertex", "brightness"],
      uniforms = ["projection", "star_xy", "wh", "xy", "scale"],
      subroutines = {},
   ),
   Shader(
      name = "font",
      vs_path = "font.vert",
      fs_path = "font.frag",
      attributes = ["vertex", "tex_coord"],
      uniforms = ["projection", "color", "outline_color"],
      subroutines = {},
   ),
   Shader(
      name = "beam",
      vs_path = "project_pos.vert",
      fs_path = "beam.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "dt", "r", "dimensions" ],
      subroutines = {
        "beam_func" : [
            "beam_default",
            "beam_wave",
            "beam_arc",
            "beam_helix",
            "beam_organic",
            "beam_unstable",
            "beam_fuzzy",
        ]
      }
   ),
   Shader(
      name = "tk",
      vs_path = "tk.vert",
      fs_path = "tk.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "c", "dc", "lc", "oc", "wh", "corner_radius"],
      subroutines = {},
   ),
   Shader(
      name = "jump",
      vs_path = "project_pos.vert",
      fs_path = "jump.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "progress", "direction", "dimensions"],
      subroutines = {
        "jump_func" : [
            "jump_default",
            "jump_nebula",
            "jump_organic",
            "jump_circular",
            "jump_wind",
        ]
      }
   ),
   Shader(
      name = "colorblind",
      vs_path = "postprocess.vert",
      fs_path = "colorblind.frag",
      attributes = ["VertexPosition"],
      uniforms = ["ClipSpaceFromLocal", "MainTex"],
      subroutines = {},
   ),
   Shader(
      name = "shake",
      vs_path = "postprocess.vert",
      fs_path = "shake.frag",
      attributes = ["VertexPosition"],
      uniforms = ["ClipSpaceFromLocal", "MainTex", "shake_pos", "shake_vel", "shake_force"],
      subroutines = {},
   ),
   Shader(
      name = "damage",
      vs_path = "postprocess.vert",
      fs_path = "damage.frag",
      attributes = ["VertexPosition"],
      uniforms = ["ClipSpaceFromLocal", "MainTex", "damage_strength", "love_ScreenSize"],
      subroutines = {},
   ),
   Shader(
      name = "gamma_correction",
      vs_path = "postprocess.vert",
      fs_path = "gamma_correction.frag",
      attributes = ["VertexPosition"],
      uniforms = ["ClipSpaceFromLocal", "MainTex", "gamma"],
      subroutines = {},
   ),
   Shader(
      name = "status",
      vs_path = "project_pos.vert",
      fs_path = "status.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "ok"],
      subroutines = {},
   ),
   Shader(
      name = "progressbar",
      vs_path = "project_pos.vert",
      fs_path = "progressbar.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "dimensions", "progress"],
      subroutines = {},
   ),
   SimpleShader(
      name = "safelanes",
      fs_path = "safelanes.frag",
   ),
]

def write_header(f):
    f.write(f"/* FILE GENERATED BY {__file__} */")

def generate_h_file(f):
    write_header(f)

    f.write("""
#ifndef SHADER_GEN_C_H
#define SHADER_GEN_C_H
#include "opengl.h"

typedef struct SimpleShader_ {
    GLuint program;
    GLuint vertex;
    GLuint projection;
    GLuint color;
    GLuint dimensions;
} SimpleShader;

typedef struct Shaders_ {
""")

    for shader in SHADERS:
        shader.write_header( f )

    f.write("""} Shaders;

extern Shaders shaders;

void shaders_load (void);
void shaders_unload (void);

#endif /* SHADERS_GEN_C_H */""")

def generate_c_file(f):
    write_header(f)

    f.write("""
#include <string.h>
#include "shaders.gen.h"
#include "opengl_shader.h"

Shaders shaders;

void shaders_load (void) {
""")
    for i, shader in enumerate(SHADERS):
        shader.write_source( f )
        if i != len(SHADERS) - 1:
            f.write("\n")
    f.write("""}

void shaders_unload (void) {
""")
    for shader in SHADERS:
        f.write(f"   glDeleteProgram(shaders.{shader.name}.program);\n")

    f.write("""   memset(&shaders, 0, sizeof(shaders));
}""")

with open("shaders.gen.h", "w") as shaders_gen_h:
    generate_h_file(shaders_gen_h)

with open("shaders.gen.c", "w") as shaders_gen_c:
    generate_c_file(shaders_gen_c)

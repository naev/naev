#!/usr/bin/env python3

import collections

Shader = collections.namedtuple("Shader", "name vs_path fs_path attributes uniforms subroutines")

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
      name = "safelanes",
      vs_path = "project_pos.vert",
      fs_path = "safelanes.frag",
      attributes = ["vertex"],
      #uniforms = ["projection", "color", "dt", "r", "dimensions" ],
      uniforms = ["projection", "color", "dimensions" ],
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
]

def write_header(f):
    f.write("/* FILE GENERATED BY %s */\n\n" % __file__)

def generate_h_file(f):
    write_header(f)

    f.write("#ifndef SHADER_GEN_C_H\n")
    f.write("#define SHADER_GEN_C_H\n")

    f.write("#include \"opengl.h\"\n\n")

    f.write("typedef struct Shaders_ {\n")

    for shader in SHADERS:
        f.write("   struct {\n")

        f.write("      GLuint program;\n")

        for attribute in shader.attributes:
            f.write("      GLuint {};\n".format(attribute))

        for uniform in shader.uniforms:
            f.write("      GLuint {};\n".format(uniform))

        for subroutine, routines in shader.subroutines.items():
            f.write("      struct {\n")
            f.write("         GLuint uniform;\n")
            for r in routines:
                f.write(f"         GLuint {r};\n")
            f.write(f"      }} {subroutine};\n")

        f.write("   }} {};\n".format(shader.name))
    f.write("} Shaders;\n\n")

    f.write("extern Shaders shaders;\n\n")

    f.write("void shaders_load (void);\n")
    f.write("void shaders_unload (void);\n")

    f.write("#endif\n")

def generate_c_file(f):
    write_header(f)

    f.write("#include <string.h>\n")
    f.write("#include \"shaders.gen.h\"\n")
    f.write("#include \"opengl_shader.h\"\n\n")

    f.write("Shaders shaders;\n\n")

    f.write("void shaders_load (void) {\n")
    for i, shader in enumerate(SHADERS):
        f.write("   shaders.{}.program = gl_program_vert_frag(\"{}\", \"{}\");\n".format(
                 shader.name,
                 shader.vs_path,
                 shader.fs_path))
        for attribute in shader.attributes:
            f.write("   shaders.{}.{} = glGetAttribLocation(shaders.{}.program, \"{}\");\n".format(
                    shader.name,
                    attribute,
                    shader.name,
                    attribute))

        for uniform in shader.uniforms:
            f.write("   shaders.{}.{} = glGetUniformLocation(shaders.{}.program, \"{}\");\n".format(
                    shader.name,
                    uniform,
                    shader.name,
                    uniform))

        f.write("   if (gl_has( OPENGL_SUBROUTINES )) {\n")
        for subroutine, routines in shader.subroutines.items():
            f.write(f"      shaders.{shader.name}.{subroutine}.uniform = glGetSubroutineUniformLocation( shaders.{shader.name}.program, GL_FRAGMENT_SHADER, \"{subroutine}\" );\n")
            for r in routines:
                f.write(f"      shaders.{shader.name}.{subroutine}.{r} = glGetSubroutineIndex( shaders.{shader.name}.program, GL_FRAGMENT_SHADER, \"{r}\" );\n")
        f.write("   }\n");

        if i != len(SHADERS) - 1:
            f.write("\n")
    f.write("}\n\n")

    f.write("void shaders_unload (void) {\n")
    for shader in SHADERS:
        f.write("   glDeleteProgram(shaders.{}.program);\n".format(shader.name))

    f.write("   memset(&shaders, 0, sizeof(shaders));\n")
    f.write("}\n")

with open("shaders.gen.h", "w") as shaders_gen_h:
    generate_h_file(shaders_gen_h)

with open("shaders.gen.c", "w") as shaders_gen_c:
    generate_c_file(shaders_gen_c)

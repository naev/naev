#!/usr/bin/env python3

num_shaders = 0

class Shader:
    def __init__(self, name, vs_path, fs_path, attributes, uniforms, subroutines, geom_path=None):
        global num_shaders
        num_shaders += 1
        self.name       = name
        self.vs_path    = vs_path
        self.fs_path    = fs_path
        self.geom_path  = geom_path
        self.attributes = attributes
        self.uniforms   = uniforms
        self.subroutines= subroutines

    def header_chunks(self):
        yield "   struct {\n"
        yield "      GLuint program;\n"
        for attribute in self.attributes:
            yield f"      GLuint {attribute};\n"
        for uniform in self.uniforms:
            yield f"      GLuint {uniform};\n"
        for subroutine, routines in self.subroutines.items():
            yield "      struct {\n"
            yield "         GLuint uniform;\n"
            for r in routines:
                yield f"         GLuint {r};\n"
            yield f"      }} {subroutine};\n"
        yield f"   }} {self.name};\n"

    def source_chunks(self):
        gshader = f"\"{self.geom_path}\"" if self.geom_path!=None else "NULL"
        yield f"   shaders.{self.name}.program = gl_program_vert_frag(\"{self.vs_path}\", \"{self.fs_path}\", {gshader});\n"
        for attribute in self.attributes:
            yield f"   shaders.{self.name}.{attribute} = glGetAttribLocation(shaders.{self.name}.program, \"{attribute}\");\n"
        for uniform in self.uniforms:
            yield f"   shaders.{self.name}.{uniform} = glGetUniformLocation(shaders.{self.name}.program, \"{uniform}\");\n"
        if len(self.subroutines) > 0:
            yield "   if (gl_has( OPENGL_SUBROUTINES )) {\n"
            for subroutine, routines in self.subroutines.items():
                yield f"      shaders.{self.name}.{subroutine}.uniform = glGetSubroutineUniformLocation( shaders.{self.name}.program, GL_FRAGMENT_SHADER, \"{subroutine}\" );\n"
                for r in routines:
                    yield f"      shaders.{self.name}.{subroutine}.{r} = glGetSubroutineIndex( shaders.{self.name}.program, GL_FRAGMENT_SHADER, \"{r}\" );\n"
            yield "   }\n"

    def __lt__( self, other ):
        return self.name < other.name

num_simpleshaders = 0
class SimpleShader(Shader):
    def __init__(self, name, fs_path):
        super().__init__( name=name, vs_path="project_pos.vert", fs_path=fs_path, attributes=["vertex"], uniforms=["projection","color","dimensions","dt","paramf","parami","paramv"], subroutines={} )
        global num_simpleshaders
        num_simpleshaders += 1
    def header_chunks(self):
        yield f"   SimpleShader {self.name};\n"
    def source_chunks(self):
        yield f"   shaders_loadSimple( \"{self.name}\", &shaders.{self.name}, \"{self.fs_path}\" );"

SHADERS = [
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
      uniforms = ["projection", "color", "tex_mat", "sampler"],
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
      name = "stealthoverlay",
      vs_path = "texture.vert",
      fs_path = "stealthoverlay.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "color", "tex_mat"],
      subroutines = {},
   ),
   Shader(
      name = "nebula",
      vs_path = "nebula.vert",
      fs_path = "nebula_overlay.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "nonuniformity", "horizon", "eddy_scale", "time"],
      subroutines = {},
   ),
   Shader(
      name = "nebula_background",
      vs_path = "nebula.vert",
      fs_path = "nebula_background.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "nonuniformity", "eddy_scale", "time", "volatility"],
      subroutines = {},
   ),
   Shader(
      name = "nebula_puff",
      vs_path = "project_pos.vert",
      fs_path = "nebula_puff.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "nebu_col", "time", "r" ],
      subroutines = {},
   ),
   Shader(
      name = "nebula_map",
      vs_path = "system_map.vert",
      fs_path = "nebula_map.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "hue", "time", "globalpos", "alpha", "volatility"],
      subroutines = {},
   ),
   Shader(
      name = "points",
      vs_path = "smooth.vert",
      fs_path = "points.frag",
      attributes = ["vertex", "vertex_color"],
      uniforms = ["projection"],
      subroutines = {},
   ),
   Shader(
      name = "dust",
      vs_path = "dust.vert",
      fs_path = "dust.frag",
      attributes = ["vertex", "brightness"],
      uniforms = ["projection", "offset_xy", "dims", "screen", "use_lines"],
      subroutines = {},
      geom_path = "dust.geom",
   ),
   Shader(
      name = "lines",
      vs_path = "lines.vert",
      fs_path = "lines.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "colour"],
      subroutines = {},
   ),
   Shader(
      name = "font",
      vs_path = "font.vert",
      fs_path = "font.frag",
      attributes = ["vertex", "tex_coord"],
      uniforms = ["projection", "m", "color", "outline_color"],
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
      name = "jump",
      vs_path = "project_pos.vert",
      fs_path = "jump.frag",
      attributes = ["vertex"],
      uniforms = ["projection", "progress", "direction", "dimensions", "brightness"],
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
      name = "material",
      vs_path = "material.vert",
      fs_path = "material.frag",
      attributes = ["vertex", "vertex_normal", "vertex_tex"],
      uniforms = ["projection", "model", "map_Kd", "map_Ks", "map_Ke", "map_Bump", "Ns", "Ka", "Kd", "Ks", "Ke", "Ni", "d", "bm"],
      subroutines = {},
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
   SimpleShader(
      name = "status",
      fs_path = "status.frag",
   ),
   SimpleShader(
      name = "factiondisk",
      fs_path = "factiondisk.frag",
   ),
   SimpleShader(
      name = "stealthaura",
      fs_path = "stealthaura.frag",
   ),
   SimpleShader(
      name = "spobmarker_empty",
      fs_path = "spobmarker_empty.frag",
   ),
   SimpleShader(
      name = "spobmarker_earth",
      fs_path = "spobmarker_earth.frag",
   ),
   SimpleShader(
      name = "spobmarker_rhombus",
      fs_path = "spobmarker_rhombus.frag",
   ),
   SimpleShader(
      name = "spobmarker_triangle",
      fs_path = "spobmarker_triangle.frag",
   ),
   SimpleShader(
      name = "spobmarker_wormhole",
      fs_path = "spobmarker_wormhole.frag",
   ),
   SimpleShader(
      name = "spobmarker_obelisk",
      fs_path = "spobmarker_obelisk.frag",
   ),
   SimpleShader(
      name = "jumpmarker",
      fs_path = "jumpmarker.frag",
   ),
   SimpleShader(
      name = "pilotmarker",
      fs_path = "pilotmarker.frag",
   ),
   SimpleShader(
      name = "playermarker",
      fs_path = "playermarker.frag",
   ),
   SimpleShader(
      name = "blinkmarker",
      fs_path = "blinkmarker.frag",
   ),
   SimpleShader(
      name = "sysmarker",
      fs_path = "sysmarker.frag",
   ),
   SimpleShader(
      name = "notemarker",
      fs_path = "notemarker.frag",
   ),
   SimpleShader(
      name = "asteroidmarker",
      fs_path = "asteroidmarker.frag",
   ),
   SimpleShader(
      name = "targetship",
      fs_path = "targetship.frag",
   ),
   SimpleShader(
      name = "targetspob",
      fs_path = "targetspob.frag",
   ),
   SimpleShader(
      name = "jumplane",
      fs_path = "jumplane.frag",
   ),
   SimpleShader(
      name = "jumplanegoto",
      fs_path = "jumplanegoto.frag",
   ),
   SimpleShader(
      name = "safelane",
      fs_path = "safelane.frag",
   ),
   SimpleShader(
      name = "iflockon",
      fs_path = "iflockon.frag",
   ),
   SimpleShader(
      name = "gear",
      fs_path = "gear.frag",
   ),
   SimpleShader(
      name = "selectspob",
      fs_path = "selectspob.frag",
   ),
   SimpleShader(
      name = "selectposition",
      fs_path = "selectposition.frag",
   ),
   SimpleShader(
      name = "sdfsolid",
      fs_path = "sdfsolid.frag",
   ),
   SimpleShader(
      name = "circle",
      fs_path = "circle.frag",
   ),
   SimpleShader(
      name = "crosshairs",
      fs_path = "crosshairs.frag",
   ),
   SimpleShader(
      name = "astaura",
      fs_path = "astaura.frag",
   ),
   SimpleShader(
      name = "hilight",
      fs_path = "hilight.frag",
   ),
   SimpleShader(
      name = "hilight_circle",
      fs_path = "hilight_circle.frag",
   ),
   SimpleShader(
      name = "stealthmarker",
      fs_path = "stealthmarker.frag",
   ),
   SimpleShader(
      name = "healthbar",
      fs_path = "healthbar.frag",
   ),
]
SHADERS.sort()

def header_chunks():
    yield f"/* FILE GENERATED BY {__file__} */"

def generate_h_file():
    yield from header_chunks()

    yield f"""
#pragma once

#include <time.h>

#include "glad.h"
#include "conf.h"
#include "log.h"

#define NUM_SHADERS         {num_shaders}
#define NUM_SIMPLE_SHADERS  {num_simpleshaders}

typedef struct SimpleShader_ {{
   const char *name;
   GLuint program;
   GLuint vertex;
   GLuint projection;
   GLuint color;
   GLuint dimensions;
   GLuint dt;
   GLuint parami;
   GLuint paramf;
   GLuint paramv;
}} SimpleShader;

typedef struct Shaders_ {{
"""

    for shader in SHADERS:
        yield from shader.header_chunks()

    yield f"""   SimpleShader *simple_shaders[ NUM_SIMPLE_SHADERS ];
}} Shaders;

extern Shaders shaders;

void shaders_load (void);
void shaders_unload (void);
const SimpleShader *shaders_getSimple( const char *name );
"""

def generate_c_file():
    yield from header_chunks()

    yield """
#include <string.h>
#include "shaders.gen.h"
#include "opengl_shader.h"

Shaders shaders;

static int nsimpleshaders = 0;

static int shaders_cmp( const void *p1, const void *p2 )
{
   const SimpleShader **s1 = (const SimpleShader**) p1;
   const SimpleShader **s2 = (const SimpleShader**) p2;
   return strcmp( (*s1)->name, (*s2)->name );
}

static int shaders_loadSimple( const char *name, SimpleShader *shd, const char *fs_path )
{
   shd->name   = name;
   shd->program = gl_program_vert_frag( "project_pos.vert", fs_path, NULL );
   shd->vertex = glGetAttribLocation( shd->program, "vertex" );
   shd->projection = glGetUniformLocation( shd->program, "projection" );
   shd->color  = glGetUniformLocation( shd->program, "color" );
   shd->dimensions = glGetUniformLocation( shd->program, "dimensions" );
   shd->dt     = glGetUniformLocation( shd->program, "dt" );
   shd->paramf = glGetUniformLocation( shd->program, "paramf" );
   shd->parami = glGetUniformLocation( shd->program, "parami" );
   shd->paramv = glGetUniformLocation( shd->program, "paramv" );

   /* Add to list. */
   shaders.simple_shaders[ nsimpleshaders++ ] = shd;

   return 0;
}

const SimpleShader *shaders_getSimple( const char *name )
{
   const SimpleShader shd = { .name=name };
   const SimpleShader *shdptr = &shd;
   const SimpleShader **found = bsearch( &shdptr, shaders.simple_shaders, nsimpleshaders, sizeof(SimpleShader*), shaders_cmp );
   if (found!=NULL)
      return *found;
   return NULL;
}

void shaders_load (void) {
   Uint32 time = SDL_GetTicks();
"""

    for i, shader in enumerate(SHADERS):
        yield from shader.source_chunks()
        if i != len(SHADERS) - 1:
            yield "\n"
    yield """
   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_("Loaded %d Shader in %.3f s", "Loaded %d Shaders in %.3f s", NUM_SHADERS ), NUM_SHADERS, time/1000. );
   }
   else
      DEBUG( n_("Loaded %d Shader", "Loaded %d Shaders", NUM_SHADERS ), NUM_SHADERS );
}

void shaders_unload (void) {
"""
    for shader in SHADERS:
        yield f"   glDeleteProgram(shaders.{shader.name}.program);\n"

    yield """   memset(&shaders, 0, sizeof(shaders));
   nsimpleshaders = 0;
}"""

with open("shaders.gen.h", "w") as shaders_gen_h:
    shaders_gen_h.writelines(generate_h_file())

with open("shaders.gen.c", "w") as shaders_gen_c:
    shaders_gen_c.writelines(generate_c_file())

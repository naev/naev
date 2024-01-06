#!/usr/bin/python
import re
import sys
import subprocess
from multiprocessing import Pool

def preprocess( shader, shadertype ):
    lines = shader.splitlines()

    islove = False
    i = 0
    while i < len(lines):
        if not islove:
            # Pretty bad detection, but no false positives... for now
            if shadertype=="frag" and re.match("vec4 effect\(\s*vec4", lines[i].strip()):
                islove = True
            elif shadertype=="vert" and re.match("vec4 position\(\s*mat4", lines[i].strip()):
                islove = True
        m = re.match('#include\s+"([^"]+)"', lines[i].strip())
        if m:
            with open("dat/glsl/" + m.group(1)) as f:
                contents = f.read()
            lines = lines[:i] + contents.splitlines() + lines[i+1:]
        i += 1

    # Love shaders need some additional voodoo
    prepend = "#version 150 core\n"
    if islove:
        prepend += """
#define _LOVE
// Syntax sugar
#define Image           sampler2D
#define ArrayImage      sampler2DArray
#define VolumeImage     sampler3D
#define Texel           texture
#define love_PixelColor colour_out
#define love_PixelColour colour_out
#define love_Position   gl_Position
#define love_PixelCoord love_getPixelCoord()

// Uniforms shared by pixel and vertex shaders
uniform mat4 ViewSpaceFromLocal;
uniform mat4 ClipSpaceFromView;
uniform mat4 ClipSpaceFromLocal;
uniform mat3 ViewNormalFromLocal;
uniform vec4 love_ScreenSize;
uniform vec4 ConstantColour = vec4(1.0);

// Compatibility
#define TransformMatrix             ViewSpaceFromLocal
#define ProjectionMatrix            ClipSpaceFromView
#define TransformProjectionMatrix   ClipSpaceFromLocal
#define NormalMatrix                ViewNormalFromLocal
"""
        if shadertype=="frag":
            prepend += """
#define PIXEL
uniform sampler2D MainTex;

in vec4 VaryingTexCoord;
in vec4 VaryingColour;
in vec2 VaryingPosition;
out vec4 colour_out;

vec2 love_getPixelCoord() {
   vec2 uv = love_ScreenSize.xy * (0.5*VaryingPosition+0.5);
   uv.y = uv.y * love_ScreenSize.z + love_ScreenSize.w;
   return uv;
}

vec4 effect( vec4 vcolour, Image tex, vec2 texcoord, vec2 pixcoord );

void main(void) {
   love_PixelColour = effect( VaryingColour, MainTex, VaryingTexCoord.st, love_PixelCoord );
}
"""
        else:
            prepend += """
#define VERTEX
in vec4 VertexPosition;
in vec4 VertexTexCoord;
in vec4 VertexColour;

out vec4 VaryingTexCoord;
out vec4 VaryingColour;
out vec2 VaryingPosition;

vec4 position( mat4 clipSpaceFromLocal, vec4 localPosition );

void main(void) {
    VaryingTexCoord  = VertexTexCoord;
    VaryingTexCoord.y= 1.0 - VaryingTexCoord.y;
    VaryingTexCoord  = ViewSpaceFromLocal * VaryingTexCoord;
    VaryingColour     = ConstantColour;
    love_Position    = position( ClipSpaceFromLocal, VertexPosition );
    VaryingPosition  = love_Position.xy;
}
"""
    return prepend+'\n'.join(lines)

def glslvalidate( filename ):
    if filename.endswith(".vert"):
        s = "vert"
    elif filename.endswith(".frag"):
        s = "frag"
    else:
        return

    with open(filename) as f:
        data = preprocess( f.read(), s )

    #print(f"{i}:")
    #p = subprocess.Popen(["glslangValidator", "--stdin", "-S", s, "--enhanced-msgs"], stdin=subprocess.PIPE)
    #p.communicate(contents.encode())
    #p.wait()
    args = ["glslangValidator", "--stdin", "-S", s, "--enhanced-msgs"]
    ret = subprocess.run( args, capture_output=True, input=bytes(data,'utf-8') )
    return ret.returncode, ret.stdout

if __name__ == "__main__":
    filelist = sys.argv[1:]
    filelist.sort()
    with Pool() as pool:
        retlist = pool.map( glslvalidate, filelist )
    err = 0
    for r in retlist:
        if r[0]!=0:
            err += r[0]
            # only write to stdeout in class of error for less spam
            sys.stdout.buffer.write( r[1] )
    sys.exit( err )

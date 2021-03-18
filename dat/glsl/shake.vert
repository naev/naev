uniform mat4 ClipSpaceFromLocal;
uniform vec2 shake_pos = vec2( 0.0, 0.0 );

in vec4 VertexPosition;
out vec4 VaryingTexCoord;

void main(void) {
   VaryingTexCoord   = VertexPosition;
   VaryingTexCoord.y = 1.0 - VaryingTexCoord.y;
   VaryingTexCoord.xy += shake_pos;
   gl_Position       = ClipSpaceFromLocal * VertexPosition;
}


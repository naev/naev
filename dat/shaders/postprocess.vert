uniform mat4 ClipSpaceFromLocal;

in vec4 VertexPosition;
out vec4 VaryingTexCoord;

void main(void) {
   VaryingTexCoord   = VertexPosition;
   VaryingTexCoord.y = 1.0 - VaryingTexCoord.y;
   gl_Position       = ClipSpaceFromLocal * VertexPosition;
}

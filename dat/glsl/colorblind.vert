uniform mat4 ClipSpaceFromLocal;

in vec4 VertexPosition;
out vec4 VaryingTexCoord;

void main(void) {
   VaryingTexCoord   = VertexPosition;
   gl_Position       = ClipSpaceFromLocal * VertexPosition;
}


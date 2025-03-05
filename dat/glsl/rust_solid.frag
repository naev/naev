layout(std140) uniform SolidData {
   mat3 transform;
   vec4 colour;
};
layout(location = 0) out vec4 colour_out;

void main(void) {
   colour_out = colour;
}

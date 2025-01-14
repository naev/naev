uniform mat4 projection;

layout(std140) uniform PuffData {
   vec2 screen;
   vec3 offset;
   vec3 colour;
   float elapsed;
};

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 data;
layout(location = 2) in vec2 randin;
out vec2 fragpos;
out vec2 rand;

void main(void) {
   rand = randin;  // Pass as is to fragment shader
   fragpos = vertex.xy;

   float height = data.z;
   float size = data.w * offset.z;
   vec4 center = vertex * size;
   center.xy += data.xy * height;
   center.xy = mod( center.xy + screen.xy/2.0, screen.xy ) - screen.xy/2.0;
   gl_Position = center;
}

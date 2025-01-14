uniform mat4 projection;

const float PUFF_BUFFER = 300.0; // Must match rust-side

layout(std140) uniform PuffData {
   vec2 screen; // Actually half-screen
   vec3 offset;
   vec3 colour;
   float elapsed;
};

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec4 data;
layout(location = 2) in vec2 randin;
out vec2 fragpos;
out vec2 rand;

void main(void) {
   rand = randin;  // Pass as is to fragment shader
   fragpos = vertex.xy;

   vec2 pos = data.xy;
   float height = data.z;
   float size = data.w * offset.z; // Zoomed
   vec2 center = vertex * size; // 0-centered pixel-size
   center += pos - offset.xy * height; // Translate to correct position
   center = mod( center, 2.0*(screen + PUFF_BUFFER) ) - screen - PUFF_BUFFER;  // Still centered
   center *= offset.z / screen; // Rescale to [-1,1] range
   gl_Position = vec4( center, 0.0, 1.0 );
}

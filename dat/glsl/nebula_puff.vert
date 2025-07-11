uniform mat4 projection;

#ifndef PUFF_BUFFER
#  define PUFF_BUFFER 300
#endif

layout(std140) uniform PuffData {
   vec2 screen; // Actually half-screen
   vec3 offset;
   vec3 colour;
   float elapsed;
   float scale;
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
   // Translate to correct position
   vec2 center = pos - offset.xy * vec2(height);
   // Still centered in [-screen-PUFF_BUFFER, screen+PUFF_BUFFER] range
   center = mod( center, 2.0*(screen + PUFF_BUFFER) ) - screen - PUFF_BUFFER;
   // Add the 0-centered pixel-size _AFTER_ the mod operation
   center += vertex * vec2(size);
   // Rescale to [-1,1] range, with PUFF_BUFFER / screen sticking "out"
   center *= scale * offset.z / screen;
   gl_Position = vec4( center, 0.0, 1.0 );
}

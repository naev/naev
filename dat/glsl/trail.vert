/* Same as project_pos.vert */
uniform mat4 projection;
in vec4 vertex;
out vec2 pos;

uniform vec2 z; // Depth

void main(void) {
   pos = vertex.xy;
   gl_Position = projection * vertex;
   gl_Position.z = mix( z.x, z.y, vertex.x ); // Use the "trail" depth
}

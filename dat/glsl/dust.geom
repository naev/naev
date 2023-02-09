layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projection;
uniform vec3 dims;
uniform vec3 screen;
uniform bool use_lines;

in float brightness_geom[];
out float brightness_frag;
out float length_frag;
out vec2 pos_frag;

vec4 proj( mat2 M, float x, float y )
{
   return vec4( M*vec2(x, y), 0.0, 0.0 );
}

void main ()
{
   vec4 center = gl_in[0].gl_Position;
   brightness_frag = brightness_geom[0];

   /* Lines get extended and such. */
   if (use_lines) {
      vec2 r = dims.xx;
      float l = dims.z * screen.z;
      float a = dims.y;
      float c = cos(a);
      float s = sin(a);
      mat2 R = mat2( c, s, -s, c );
      mat2 S = mat2( projection[0][0], 0.0, 0.0, projection[1][1] );
      mat2 M = S * R;

      length_frag = r.x*2.0+l;
      pos_frag = vec2(-1.0, -1.0);
      gl_Position = center + proj( M, -r.x-l, -r.y );
      EmitVertex();

      length_frag = 0.0;
      pos_frag = vec2(1.0, -1.0);
      gl_Position = center + proj( M,  r.x,   -r.y );
      EmitVertex();

      length_frag = r.x*2.0+l;
      pos_frag = vec2(-1.0, 1.0);
      gl_Position = center + proj( M, -r.x-l,  r.y );
      EmitVertex();

      length_frag = 0.0;
      pos_frag = vec2(1.0, 1.0);
      gl_Position = center + proj( M,  r.x,    r.y );
      EmitVertex();
   }
   /* Points are just centered primitives. */
   else {
      vec2 r = dims.x * vec2( projection[0][0], projection[1][1] );

      pos_frag = vec2(-1.0, -1.0);
      gl_Position = center + vec4(-r.x, -r.y, 0.0, 0.0);
      EmitVertex();

      pos_frag = vec2(1.0, -1.0);
      gl_Position = center + vec4( r.x, -r.y, 0.0, 0.0);
      EmitVertex();

      pos_frag = vec2(-1.0, 1.0);
      gl_Position = center + vec4(-r.x,  r.y, 0.0, 0.0);
      EmitVertex();

      pos_frag = vec2(1.0, 1.0);
      gl_Position = center + vec4( r.x,  r.y, 0.0, 0.0);
      EmitVertex();
   }

   EndPrimitive();
}

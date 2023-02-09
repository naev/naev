layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projection;
uniform vec3 dims;
uniform vec3 screen;
uniform bool use_lines;

in float brightness_geom[];
in vec4 center_geom[];
in float length_geom[];
in vec2 angle_geom[];
out float brightness_frag;
out vec2 pos_frag;

vec4 proj( mat2 R, float x, float y )
{
   return vec4( R*vec2(x, y), 0.0, 0.0 );
}

void main ()
{
   vec4 center = gl_in[0].gl_Position;
   brightness_frag = brightness_geom[0];

   /* Lines get extended and such. */
   if (use_lines) {
      vec2 r = dims.xx;
      float l = length_geom[0] * screen.z;
      float a = dims.y;
      float c = cos(a);
      float s = sin(a);
      mat2 R = mat2( c, s, -s, c );
      mat2 S = mat2( projection[0][0], 0.0, 0.0, projection[1][1] );
      mat2 M = S * R;// * S;

      pos_frag = vec2(-1.0, -1.0);
      gl_Position = center + proj( M, -r.x-l, -r.y );
      EmitVertex();

      pos_frag = vec2(1.0, -1.0);
      gl_Position = center + proj( M,  r.x,   -r.y );
      EmitVertex();

      pos_frag = vec2(-1.0, 1.0);
      gl_Position = center + proj( M, -r.x-l,  r.y );
      EmitVertex();

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

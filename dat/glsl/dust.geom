layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform vec3 dims;
uniform vec3 screen;
uniform bool use_lines;

in float brightness_geom[];
in vec4 center_geom[];
in vec2 radius_geom[];
out float brightness_frag;

vec4 proj( mat2 R, float x, float y )
{
   return vec4( R*vec2(x, y), 0.0, 0.0 );
}

void main ()
{
   vec4 center = gl_in[0].gl_Position;
   vec2 r = radius_geom[0];
   brightness_frag = brightness_geom[0];

   /* Lines get extended and such. */
   if (use_lines) {
      float a = dims.y;
      float l = dims.z * screen.z;
      float c = cos(a);
      float s = sin(a);
      mat2 R = mat2( c, -s, s, c );

      gl_Position = center + proj( R, -r.x,   -r.y );
      EmitVertex();

      gl_Position = center + proj( R,  r.x+l, -r.y );
      EmitVertex();

      gl_Position = center + proj( R, -r.x,    r.y );
      EmitVertex();

      gl_Position = center + proj( R,  r.x+l,  r.y );
      EmitVertex();
   }
   /* Points are just centered primitives. */
   else {
      gl_Position = center + vec4(-r.x, -r.y, 0.0, 0.0);
      EmitVertex();

      gl_Position = center + vec4( r.x, -r.y, 0.0, 0.0);
      EmitVertex();

      gl_Position = center + vec4(-r.x,  r.y, 0.0, 0.0);
      EmitVertex();

      gl_Position = center + vec4( r.x,  r.y, 0.0, 0.0);
      EmitVertex();
   }

   EndPrimitive();
}

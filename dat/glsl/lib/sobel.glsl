#ifndef _SOBEL_GLSL
#define _SOBEL_GLSL

float _G( vec4 c )
{
   return c.a * (c.r+c.g+c.b) * (1.0/3.0);
}

float sobel( sampler2D tex, vec2 st, vec2 resolution )
{
   vec2 g;
   vec2 p = 1.0 / resolution;
   float tl, tr, bl, br;

   tl = _G(texture( tex, st+vec2( p.x, p.y) ));
   tr = _G(texture( tex, st+vec2(-p.x, p.y) ));
   bl = _G(texture( tex, st+vec2( p.x,-p.y) ));
   br = _G(texture( tex, st+vec2(-p.x,-p.y) ));

   g.x  = tl+bl-tr-br;
   g.x += 2.0*_G(texture( tex, st+vec2( p.x,0) ));
   g.x -= 2.0*_G(texture( tex, st+vec2(-p.x,0) ));

   g.y  = tl+tr-bl-br;
   g.y += 2.0*_G(texture( tex, st+vec2(0, p.y) ));
   g.y -= 2.0*_G(texture( tex, st+vec2(0,-p.y) ));

   return length(g);
}

#endif /* _SOBEL_GLSL */

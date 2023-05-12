#pragma language glsl3

#include "lib/math.glsl"

const float SPEED = 0.1; /**< Accretion disk rotation speed. */
const float STEPS = 6.0; /**< Iterations on accretion disk layers. */
const float SIZE  = 3.0 * %f; /**< Size of the black hole relative to texture. */
/* Set up rotation matrix at compile-time for efficiency. */
/* We also need its inverse, and some GLSL compilers in the wild lack a const inverse() function. */
const vec3 rotang = vec3( %f, %f, %f );
const float cx = cos(rotang.x);
const float sx = sin(rotang.x);
const mat3 Rx = mat3(
   1.0, 0.0, 0.0,
   0.0, cx, -sx,
   0.0, sx,  cx );
const mat3 Rinvx = mat3(
   1.0, 0.0, 0.0,
   0.0, cx,  sx,
   0.0, -sx, cx );
const float cy = cos(rotang.y);
const float sy = sin(rotang.y);
const mat3 Ry = mat3(
   cy,  0.0, sy,
   0.0, 1.0, 0.0,
   -sy, 0.0, cy );
const mat3 Rinvy = mat3(
   cy,  0.0, -sy,
   0.0, 1.0, 0.0,
    sy, 0.0, cy );
const float cz = cos(rotang.z);
const float sz = sin(rotang.z);
const mat3 Rz = mat3(
   cz, -sz,  0.0,
   sz,  cz,  0.0,
   0.0, 0.0, 1.0 );
const mat3 Rinvz = mat3(
    cz, sz,  0.0,
   -sz, cz,  0.0,
   0.0, 0.0, 1.0 );
const mat3 R = Rx * Ry * Rz; /**< Final camera rotation matrix. */
const mat3 Rinv = Rinvz * Rinvy * Rinvx;

/* Uniforms. Most is hardcoded. */
uniform float u_time = 0.0;
uniform vec3 u_camera= vec3( 0.0, 0.0, 1.0 );
uniform sampler2D u_bgtex;

/* Value Noise. */
float value( vec2 p, float f )
{
   float bl = random(floor(p*f + vec2(0.0,0.0)));
   float br = random(floor(p*f + vec2(1.0,0.0)));
   float tl = random(floor(p*f + vec2(0.0,1.0)));
   float tr = random(floor(p*f + vec2(1.0,1.0)));

   vec2 fr = fract(p*f);
   fr = (3.0 - 2.0*fr)*fr*fr;
   float b = mix(bl, br, fr.x);
   float t = mix(tl, tr, fr.x);
   return  mix(b,t, fr.y);
}

/*
 * @brief Raymarches the accretion disk.
 */
vec4 raymarch_disk( vec3 ro, vec3 rd )
{
   vec3 pos = ro;
   float poslen = length(pos.xz);
   float dist = min( 1.0, poslen*(1.0/SIZE) * 0.5) * SIZE * 0.4 * (1.0/STEPS) / abs(rd.y);

   pos += dist * STEPS * rd * 0.5;

   vec2 deltaPos = mat2( 1.0, 0.01, -0.01, 1.0 ) * ro.xz;
   deltaPos = normalize(deltaPos - ro.xz);

   float parallel = dot(rd.xz, deltaPos) * 0.5 / sqrt(poslen);
   float redShift = parallel + 0.3;
   redShift *= redShift;
   redShift = clamp( redShift, 0.0, 1.0 );

   float disMix = clamp((poslen - SIZE * 2.0) * (1.0/SIZE) * 0.24, 0.0, 1.0);
   vec3 insideCol = mix(vec3(1.0, 0.8, 0.0), vec3(0.5, 0.13, 0.02)*0.2, disMix);

   insideCol *= 1.25 * mix(vec3(0.4, 0.2, 0.1), vec3(1.6, 2.4, 4.0), redShift);
   redShift += 0.12;
   redShift *= redShift;

   vec4 o = vec4(0.0);
   for(float i=0.0; i<STEPS; i++) {
      pos -= dist * rd;

      float intensity = clamp(1.0 - abs((i-0.8) * (1.0/STEPS) * 2.0), 0.0, 1.0);
      float poslen = length(pos.xz);

      float distMult = clamp((poslen-SIZE*0.75) * (1.0/SIZE) * 1.5, 0.0, 1.0);
      distMult      *= clamp((SIZE*10.0-poslen) * (1.0/SIZE) * 0.2, 0.0, 1.0);
      distMult      *= distMult;

      float u = poslen + u_time * SPEED * SIZE*0.3 + intensity * SIZE * 0.2;

      /* Disk rotation. */
      float r = u_time * SPEED;
      float s = sin(r);
      float c = cos(r);
      vec2 xy = mat2( c, s, -s, c ) * pos.xz;

      float x = abs( xy.x / (xy.y) );
      float angle = 0.02*atan(x);

      /* Sample from noise for the disk. */
      const float f = 70.0;
      float noise = value( vec2( angle, u * (1.0/SIZE) * 0.1 ), f );
      noise = noise*0.66 + 0.33*value( vec2( angle, u * (1.0/SIZE) * 0.1), f*2.0 );

      float expand = noise * 1.0 * (1.0 - clamp(i * (1.0/STEPS)*2.0 - 1.0, 0.0, 1.0 ));
      float alpha = clamp(noise*(intensity + expand)*( (1.0/SIZE) * 10.0 + 0.01) * dist * distMult, 0.0, 1.0);

      vec3 col = 2.0*mix(vec3(0.3, 0.2, 0.15)*insideCol, insideCol, min(1.0,intensity*2.0));
      float ainv = 1.0-alpha;
      o = clamp(vec4(col*alpha + o.rgb*ainv, o.a*ainv + alpha), vec4(0.0), vec4(1.0));

      poslen *= (1.0/SIZE);

      o.rgb += redShift*(intensity+0.5)* (1.0/STEPS) * 100.0*distMult/(poslen*poslen);
   }

   o.rgb = clamp(o.rgb - 0.005, 0.0, 1.0);
   return o;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = ((2.0*texture_coords-1.0)*love_ScreenSize.xy) / love_ScreenSize.y;

   /* Camera. */
   vec3 ro  = vec3(u_camera.xy, -100.0 * u_camera.z);
   vec3 rd  = R*normalize( vec3(0.1*uv, 1.0));
   vec3 pos = R*ro;

   /* Initialize stuff. */
   vec4 col    = vec4(0.0);
   vec4 glow   = vec4(0.0);
   vec4 outCol = vec4(100.0);

   /* Outter iterations. */
   for (int iter=0; iter<16; iter++) {
      /* Reduce branching by marching as much as possible without doing checks. */
      for (int inner=0; inner<8; inner++) {
         float dotpos   = dot(pos,pos);
         float invDist  = inversesqrt(dotpos); /* 1/distance to black hole. */
         float centDist = dotpos * invDist; /* Distance to center of the black hole. */
         float stepDist = 0.92 * abs(pos.y/(rd.y)); /* Conservative distance to disk (y==0) */
         float stepFar  = centDist*0.5; /* Limit step size when far from the black hole. */
         float stepClose= centDist*0.1 + 0.05*centDist*centDist*(1.0/SIZE); /* Limit step size when close to the black hole. */
         stepDist = min( stepDist, min( stepFar, stepClose ) );

         /* Bend the ray. */
         float invDistSqr = invDist * invDist;
         float bendForce  = stepDist * invDistSqr * SIZE * 0.625; /* Bending force. */
         rd = normalize(rd - (bendForce * invDist)*pos); /* Bend ray towards black hole. */

         /* Advance along ray. */
         pos += stepDist * rd;

         /* Cheap glow. */
         glow += vec4(1.2, 1.1, 1.0, 1.0) * (0.01*stepDist * invDistSqr * invDistSqr * clamp(centDist*2.0 - 1.2, 0.0, 1.0));
      }

      /* Figure out what happened to the ray. */
      float dist2 = length(pos);
      if (dist2 < SIZE * 0.1) { /* Sucked into black hole. */
         return color * vec4( col.rgb*col.a + glow.rgb*(1.0-col.a ), 1.0) ;
      }
      else if (dist2 > SIZE * 1000.0) { /* Escaped black hole. */
         /* Have to undo the deformation effect. */
         vec3 bgrd = Rinv * rd;
         vec2 bgc = 0.5 * 10.0 * bgrd.xy / length(vec2(1.0,0.1)) / love_ScreenSize.xy * love_ScreenSize.y + 0.5;
         bgc = mix( bgc, texture_coords, 0.8 ); /* Soften effect */
         vec4 bg = texture( u_bgtex, bgc ) / color;

         return color * vec4( col.rgb*col.a + (bg.rgb + glow.rgb)*(1.0-col.a), bg.a+(1.0-bg.a)*col.a );
      }
      else if (abs(pos.y) <= SIZE * 0.002) { /* Hit accretion disk. */
         vec4 disk = raymarch_disk( pos, rd );
         float ainv = 1.0-col.a;
         pos.y = 0.0;
         pos += abs(SIZE * 0.0001 / rd.y) * rd;
         col = vec4( disk.rgb*ainv + col.rgb, col.a + disk.a*ainv);
      }
   }

   /* Ray never escaped nor got sucked in. */
   if (outCol.r == 100.0)
      outCol = vec4( col.rgb + glow.rgb * (col.a + glow.a), 1.0 );

   return color * outCol;
}

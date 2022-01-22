uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// https://www.shadertoy.com/view/4tySWK

#define time_         u_time
#define detail_steps_ 1 // 13

#define mod3_      vec3(.1031, .11369, .13787)

vec3  hash3_3(vec3 p3);
float perlin_noise3(vec3 p);
float noise_sum_abs3(vec3 p);

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   //vec2 p = screen_coords / eddy_scale;
   // re2.*uv.xy-res_.xy) / res_.y*s;
   //vec2 p = screen_coords / eddy_scale * 3.;
   //p += -10;
   //vec2 p = (texture_coords-0.5) * 5.;
   vec2 p = screen_coords / 150.;

   float electric_density = 1.1; // 0.9
   float electric_radius  = length(texture_coords*2.-1.) - 0.1;
   float velocity = 0.5;

   vec3  electric_local_domain = vec3(p, u_time * velocity);
   float electric_field = electric_density * noise_sum_abs3(electric_local_domain);

   vec3 col = vec3(107, 148, 196) / 255.;
   col += (1. - (electric_field + electric_radius));
   //col += (1. - (electric_field));
   for(int i = 0; i < detail_steps_; i++) {
      if(length(col) >= 2.1 + float(i) / 2.)
         col -= .3;
   }
   col += 1. - 4.2*electric_field;

   float alpha = 1.;
   return vec4(col, alpha);
}


vec3 hash3_3(vec3 p3) {
    p3 = fract(p3 * mod3_);
    p3 += dot(p3, p3.yxz + 19.19);
    return -1. + 2. * fract(vec3((p3.x + p3.y) * p3.z, (p3.x+p3.z) * p3.y, (p3.y+p3.z) * p3.x));
}

float perlin_noise3(vec3 p) {
    vec3 pi = floor(p);
    vec3 pf = p - pi;

    vec3 w = pf * pf * (3. - 2. * pf);

    return mix(
        mix(
            mix(
                dot(pf - vec3(0, 0, 0), hash3_3(pi + vec3(0, 0, 0))),
                dot(pf - vec3(1, 0, 0), hash3_3(pi + vec3(1, 0, 0))),
                w.x),
            mix(
                dot(pf - vec3(0, 0, 1), hash3_3(pi + vec3(0, 0, 1))),
                dot(pf - vec3(1, 0, 1), hash3_3(pi + vec3(1, 0, 1))),
                w.x),
        w.z),
        mix(
            mix(
                dot(pf - vec3(0, 1, 0), hash3_3(pi + vec3(0, 1, 0))),
                dot(pf - vec3(1, 1, 0), hash3_3(pi + vec3(1, 1, 0))),
                w.x),
            mix(
                dot(pf - vec3(0, 1, 1), hash3_3(pi + vec3(0, 1, 1))),
                dot(pf - vec3(1, 1, 1), hash3_3(pi + vec3(1, 1, 1))),
                w.x),
        w.z),
    w.y);
}


float noise_sum_abs3(vec3 p) {
    float f = 0.;
    p = p * 3.;
    f += 1.0000 * abs(perlin_noise3(p)); p = 2. * p;
    f += 0.5000 * abs(perlin_noise3(p)); p = 3. * p;
    f += 0.2500 * abs(perlin_noise3(p)); p = 4. * p;
    f += 0.1250 * abs(perlin_noise3(p)); p = 5. * p;
    f += 0.0625 * abs(perlin_noise3(p)); p = 6. * p;

    return f;
}

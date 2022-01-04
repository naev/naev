uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;

const int noiseSwirlSteps = 2; // 2
const float noiseSwirlValue = 1.; // 1.
const float noiseSwirlStepValue = noiseSwirlValue / float(noiseSwirlSteps);

const float noiseScale = 0.5; // 0.5
const float noiseTimeScale = 0.1; // 0.1

float fbm3(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.) / 2.;
   result += snoise(v * 4.) / 4.;
   result /= (1. + 1./2. + 1./4.);
   return result;
}

float fbm5(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.) / 2.;
   result += snoise(v * 4.) / 4.;
   result += snoise(v * 8.) / 8.;
   result += snoise(v * 16.) / 16.;
   result /= (1. + 1./2. + 1./4. + 1./8. + 1./16.);
   return result;
}

float getNoise(vec3 v) {
   //  make it curl
   for (int i=0; i<noiseSwirlSteps; i++) {
      v.xy += vec2(fbm3(v), fbm3(vec3(v.xy, v.z + 1000.))) * noiseSwirlStepValue;
   }
   //  normalize
   return fbm5(v) / 2. + 0.5;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time;

   uv *= vec3( noiseScale, noiseScale, noiseTimeScale );

   float noise = getNoise( uv );
   noise = pow( noise, 4.0 ) * 2.0;  //more contrast
   return vec4( 0.2, 0.9, 1.0, noise );
   //return vec4(noise, noise, noise, 1.0);
}

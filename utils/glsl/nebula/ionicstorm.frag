uniform float u_time;

/*
https://www.shadertoy.com/view/lscyD7
5D wave noise
afl_ext 2018
public domain
*/

float hash(float p){
    return fract(4768.1232345456 * sin(p));
}

#define EULER 2.7182818284590452353602874
float wave(vec4 uv, vec4 emitter, float speed, float phase, float timeshift){
    float dst = distance(uv, emitter);
    return pow(EULER, sin(dst * phase - timeshift * speed)) / EULER;
}
vec4 wavedrag(vec4 uv, vec4 emitter){
    return normalize(uv - emitter);
}
float wave(vec2 uv, vec2 emitter, float speed, float phase, float timeshift){
    float dst = distance(uv, emitter);
    return pow(EULER, sin(dst * phase - timeshift * speed)) / EULER;
}
vec2 wavedrag(vec2 uv, vec2 emitter){
    return normalize(uv - emitter);
}
float seedWaves = 0.0;
vec4 randWaves4() {
    float x = hash(seedWaves);
    seedWaves += 1.0;
    float y = hash(seedWaves);
    seedWaves += 1.0;
    float z = hash(seedWaves);
    seedWaves += 1.0;
    float w = hash(seedWaves);
    seedWaves += 1.0;
    return vec4(x,y,z,w) * 2.0 - 1.0;
}
vec2 randWaves2() {
    float x = hash(seedWaves);
    seedWaves += 1.0;
    float y = hash(seedWaves);
    seedWaves += 1.0;
    return vec2(x,y) * 2.0 - 1.0;
}

float getwaves5d( vec4 position, float dragmult, float timeshift) {
    float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<20;i++){
        vec4 p = randWaves4() * 30.0;
        float res = wave(position, p, speed, phase, 0.0 + timeshift);
        float res2 = wave(position, p, speed, phase, 0.006 + timeshift);
        position -= wavedrag(position, p) * (res - res2) * weight * dragmult;
        w += res * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.2;
        speed *= 1.02;
    }
    return w / ws;
}
float getwaves3d( vec2 position, float dragmult, float timeshift) {
    float iter = 0.0;
    float phase = 6.0;
    float speed = 2.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<20;i++){
        vec2 p = randWaves2() * 30.0;
        float res = wave(position, p, speed, phase, 0.0 + timeshift);
        float res2 = wave(position, p, speed, phase, 0.006 + timeshift);
        position -= wavedrag(position, p) * (res - res2) * weight * dragmult;
        w += res * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.2;
        speed *= 1.02;
    }
    return w / ws;
}

const vec2 DIR    = normalize(vec2(1.0,1.0));

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv = vec3( texture_coords * 2.0, u_time );
   float s = getwaves3d( uv.xy, 20.0, u_time*0.3);
   s = s*s;

   vec4 colour = vec4( vec3(1.0,0.5,1.0), s );

   float flash_0 = sin(u_time);
   float flash_1 = sin(15.0 * u_time);
   float flash_2 = sin(2.85 * u_time);
   float flash_3 = sin(5.18 * u_time);
   float flash = max( 0.0,  snoise( uv*0.5 ) * flash_0 * flash_1 * flash_2 * flash_3 );
   colour += vec4( 1.5*(s+0.5*flash)*flash*(0.5+0.5*flash_2) );

   return colour;
}

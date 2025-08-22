struct TextureData {
   tex_mat: mat3x3f,
   transform: mat3x3f,
   colour: vec4f,
}
struct SdfData {
   m: f32,
   outline: f32,
}
@group(0) @binding(0) var<uniform> texturedata: TextureData;
@group(0) @binding(1) var<uniform> sdfdata: SdfData;

@group(0) @binding(2) var texsampler: sampler;
@group(0) @binding(3) var texture: texture_2d<f32>;

struct VertexInput {
   @location(0) vertex: vec2f,
}
struct VertexOutput {
   @builtin(position) position: vec4f,
   @location(0) uv: vec2f,
}
struct FragmentInput {
   @location(0) uv: vec2f,
}

@vertex
fn main_vs( vs: VertexInput ) -> VertexOutput {
   let pos = vec3f( vs.vertex, 1.0 );
   var output: VertexOutput;
   output.position = vec4( ( texturedata.transform * pos ).xy, 0.0, 1.0 );
   output.uv = (texturedata.tex_mat * pos).xy;
   return output;
}

const OUTLINE_COLOUR: vec4f = vec4f( vec3f(0.0), 1.0 );
const M_SQRT2: f32 = 1.41421356237309504880;

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   let d = (textureSample( texture, texsampler, fs.uv ).r-0.5)*sdfdata.m;
   let alpha = smoothstep(-0.5    ,  0.5, d);
   let beta  = smoothstep(-M_SQRT2, -1.0, d);
   let fg_c  = mix( OUTLINE_COLOUR, texturedata.colour, alpha );
   return vec4f( fg_c.rgb, beta*fg_c.a );
}

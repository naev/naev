struct TextureData {
   tex_mat: mat3x3f,
   transform: mat3x3f,
   colour: vec4f,
}
@group(0) @binding(0) var<uniform> texturedata: TextureData;

@group(0) @binding(1) var texsampler: sampler;
@group(0) @binding(2) var texture: texture_2d<f32>;

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

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   return texturedata.colour * textureSample( texture, texsampler, fs.uv );
}

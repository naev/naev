struct TextureData {
   tex_mat: mat3x3f,
   transform: mat3x3f,
   colour: vec4f,
   scale: f32,
   radius: f32,
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

// mks2021
fn resizeFilter( v: f32 ) -> f32 {
   let x = abs(v);
   if (x >= 4.5) {
      return 0.0;
   } else if (x >= 3.5) {
      return -1.0/288.0 * (x - 4.5) * (x - 4.5);
   } else if (x >= 2.5) {
      return 1.0/36.0 * (x - 3.0) * (x - 3.75);
   } else if (x >= 1.5) {
      return 1.0/6.0 * (x - 2.0) * (65.0 / 24.0 - x);
   } else if (x >= 0.5) {
      return 35.0/36.0 * (x - 1.0) * (x - 239.0/140.0);
   }
   return 577.0/576.0 - 239.0/144.0 * x * x;
}

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   return texturedata.colour * textureSample( texture, texsampler, fs.uv );
   let dims = textureDimensions( texture, 0 );
   let pixsize = 1.0 / vec2f(dims);

   let src = fs.uv * dims;
   let startf = src - texturedata.radius;
   let endf = src + texturedata.radius;
   let start = vec2<i32>( floor(startf) );
   let end = vec2<i32>( ceil(endf) );

   let sum = 0.0;
   let colour = vec4f(0.0);
   for(var i = start.x; i <= end.x; i++) {
      let fi = f32(i) + 0.5;
      for (var j = start.y; j <= end.y; j++) {
         let fj = f32(j) + 0.5;
         let fij  = vec2f(fi,fj);

         let weight = resizeFilter( length( (fij-src) * texturedata.scale ) );
         let texpos = fij * pixsize;

         let value = textureSample( texture, texsampler, texpos );
         //value *= value.a;
         //value.rgb *= vec3f(value.a);
         colour += value * weight;
         sum += weight;
      }
   }
   colour /= sum;
   colour.rgb /= colour.a;
   return colour * texturedata.colour;
}

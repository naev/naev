struct SolidData {
   transform: mat3x3f,
   colour: vec4f,
}
@group(0) @binding(0) var<uniform> soliddata: SolidData;

struct VertexInput {
   @location(0) vertex: vec2f,
}
struct VertexOutput {
   @builtin(position) position: vec4f,
}

@vertex
fn main_vs( vs: VertexInput ) -> VertexOutput {
   let pos = vec3f( vs.vertex, 1.0 );
   var output: VertexOutput;
   output.position = vec4( ( soliddata.transform * pos ).xy, 0.0, 1.0 );
   return output;
}

@fragment
fn main_fs() -> @location(0) vec4f {
   return soliddata.colour;
}

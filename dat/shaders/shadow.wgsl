struct ShadowData {
   view: mat4x4f,
}
@group(0) @binding(0) var<uniform> shadow: ShadowData;

struct VertexInput {
   @location(0) vertex: vec3f,
}
struct VertexOutput {
   @builtin(position) position: vec4f,
}

@vertex
fn main_vs( vs: VertexInput ) -> VertexOutput {
   var output: VertexOutput;
   output.position = shadow.view * vec4f( vs.vertex, 1.0 );
   return output;
}

@fragment
fn main_fs() {
   // Does nothing but will write to depth buffer
   return;
}

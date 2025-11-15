struct CircleData {
   transform: mat3x3f,
   colour: vec4f,
   radius: f32,
}
@binding(0) @group(0) var<uniform> circledata: CircleData;

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
   var output: VertexOutput;
   output.position = vec4( ( circledata.transform * vec3f( vs.vertex, 1.0 ) ).xy, 0.0, 1.0 );
   output.uv = vs.vertex;
   return output;
}

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   let pos = fs.uv;
   let radius = circledata.radius;
   let m   = 1.0 / (2.0 * radius);
   let rad = 1.0 - radius * m;
   let d = length( pos*radius ) - rad;
   let alpha = smoothstep( -m, 0.0, -d );
   //let beta = smoothstep( -radius * m, -m, -d );
   //return circledata.colour * vec4f( vec3f( alpha ), beta );
   return vec4f(1.0)+circledata.colour * vec4f( vec3f( 1.0 ), alpha );
}

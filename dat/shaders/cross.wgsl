struct CrossData {
   transform: mat3x3f,
   colour: vec4f,
   radius: f32,
   border: f32,
}
@binding(0) @group(0) var<uniform> crossdata: CrossData;

struct VertexInput {
   @location(0) vertex: vec2f,
}
struct VertexOutput {
   @builtin(position) position: vec4f,
   @location(0) uv: vec2f,
   @location(1) r: f32,
   @location(2) b: f32,
}
struct FragmentInput {
   @location(0) uv: vec2f,
   @location(1) r: f32,
   @location(2) b: f32,
}

@vertex
fn main_vs( vs: VertexInput ) -> VertexOutput {
   var output: VertexOutput;
   output.position = vec4( ( crossdata.transform * vec3f( vs.vertex, 1.0 ) ).xy, 0.0, 1.0 );
   output.uv   = vs.vertex * crossdata.radius;
   output.r    = crossdata.radius;
   output.b    = crossdata.border * 0.5;
   return output;
}

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   let pos  = fs.uv;
   let m    = 1.0;
   let r    = fs.r;
   let b    = fs.b;
   let rad  = r - m;
   let d    = min( length( pos - vec2f( clamp( pos.x, -rad, rad ), 0.0 ) ),
                   length( pos - vec2f( 0.0, clamp( pos.y, -rad, rad ) ) ) ) - b;
   let alpha = smoothstep( -m, 0.0, -d );
   return crossdata.colour * vec4f( vec3f( 1.0 ), alpha );
}

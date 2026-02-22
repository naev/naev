struct TriangleHollowData {
   transform: mat3x3f,
   colour: vec4f,
   dims: vec2f,
   border: f32,
}
@binding(0) @group(0) var<uniform> triangledata: TriangleHollowData;

struct VertexInput {
   @location(0) vertex: vec2f,
}
struct VertexOutput {
   @builtin(position) position: vec4f,
   @location(0) uv: vec2f,
   @location(1) d: vec2f,
   @location(2) b: f32,
}
struct FragmentInput {
   @location(0) uv: vec2f,
   @location(1) d: vec2f,
   @location(2) b: f32,
}

@vertex
fn main_vs( vs: VertexInput ) -> VertexOutput {
   var output: VertexOutput;
   let H = triangledata.transform;
   output.position = vec4( ( H * vec3f( vs.vertex, 1.0 ) ).xy, 0.0, 1.0 );
   let scale   = vec2f( length(H[0].xy), length(H[1].xy) );
   output.uv   = (vs.vertex * triangledata.dims).yx - vec2f(0.0, triangledata.dims.y*0.5);
   output.d    = triangledata.dims;
   output.b    = triangledata.border * 0.5;
   return output;
}

fn sdTriangleIsosceles( pi: vec2f, q: vec2f ) -> f32 {
   let p = vec2f( abs(pi.x), pi.y );
   let a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
   let b = p - q*vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
   let k = sign( q.y );
   let d = min(dot( a, a ),dot(b, b));
   let s = max( k*(p.x*q.y-p.y*q.x),k*(p.y-q.y)  );
   return sqrt(d)*sign(s);
}

@fragment
fn main_fs( fs: FragmentInput ) -> @location(0) vec4f {
   let pos  = fs.uv;
   let m    = 1.0;
   let b    = fs.b;
   let d    = abs(sdTriangleIsosceles( pos, fs.d )) - b;
   let alpha = smoothstep( -m, 0.0, -d );
   return triangledata.colour * vec4f( vec3f( 1.0 ), alpha );
}

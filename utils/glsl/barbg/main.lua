local pixelcode_sdf = love.filesystem.read( "frag.glsl" )
local lg = love.graphics

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]
local pixelblur = [[
#pragma language glsl3

const float M_SQRT2     = 1.41421356237309504880;  /* sqrt(2) */

vec4 blur( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.2941176470588234;
   vec2 off1 = vec2(1.3333333333333333,0.0) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   color += texture(image, uv + off1xy) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1xy) * 0.35294117647058826 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   color += texture(image, uv + off1yx) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1yx) * 0.35294117647058826 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d1) * 0.35294117647058826 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   color += texture(image, uv + off1d2) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d2) * 0.35294117647058826 * 0.25;
   return color;
}

uniform float u_strength;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   return blur( tex, uv, love_ScreenSize.xy, u_strength );
}
]]

local rnd = {
   rnd = love.math.random,
}

local blurshader
local function blur( img, strength )
   if not blurshader then
      blurshader = lg.newShader( pixelblur, vertexcode )
   end
   blurshader:send( "u_strength", strength )

   lg.setShader(blurshader)
   lg.draw( img )
   lg.setShader()
end

local img
local pixellight = [[
#pragma language glsl3

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   float d = length(p)-1.0;
   colour.a *= smoothstep( 0.0, 0.5, -d );
   return colour;
}
]]
local lightshader
local function light( x, y, r )
   if not lightshader then
      lightshader = lg.newShader( pixellight, vertexcode )
   end

   lg.setShader(lightshader)
   lg.draw( img, x, y, 0, r, r )
   lg.setShader()
end

local pixelrectlight = [[
#pragma language glsl3

/* Box at position b with border b. */
float sdBox( vec2 p, vec2 b )
{
   vec2 d = abs(p)-b;
   return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   float d = sdBox( p, vec2(0.5) )-0.5;
   colour.a *= smoothstep( 0.0, 0.5, -d );
   return colour;
}
]]
local rectlightshader
local function rectlight( x, y, w, h, r )
   if not rectlightshader then
      rectlightshader = lg.newShader( pixelrectlight, vertexcode )
   end

   lg.setShader(rectlightshader)
   lg.draw( img, x, y, r, w, h )
   lg.setShader()
end

local pixelgrad = [[
#pragma language glsl3

uniform vec4 u_col;
uniform vec2 u_vec;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   return mix( colour, u_col, smoothstep(-1.0, 1.0, dot(p,u_vec)) );
}
]]
local gradshader
local function gradient( x, y, r, g, b, a )
   if not gradshader then
      gradshader = lg.newShader( pixelgrad, vertexcode )
   end

   local w, h = lg.getDimensions()
   gradshader:send( "u_col", {r,g,b,a})
   gradshader:send( "u_vec", {x,y} )
   lg.setShader(gradshader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()
end

local bg
local function generate_bg ()
   local b = lg.newCanvas()
   local c = lg.newCanvas()
   local w, h = c:getDimensions()

   local colbg    = { 0.3, 0.3, 0.8, 1 }
   local colfeat  = { 0.2, 0.2, 0.8, 1 }
   local collight = { 0.9, 0.9, 1.0, 1 }

   local function calpha( c, a )
      return {c[1], c[2], c[3], a}
   end

   -- Do some background
   lg.setCanvas( c )
   lg.setColor( colbg )
   gradient( -0.1+0.2*rnd.rnd(), 0.5+0.2*rnd.rnd(), 0, 0, 0, 1 );
   for i=1,20 do
      local c = calpha(colfeat, 0.3)
      c[1] = c[1] + rnd.rnd()*0.8
      c[2] = c[2] + rnd.rnd()*0.8
      c[3] = c[3] + rnd.rnd()*0.8
      lg.setColor( c )
      if rnd.rnd() < 0.6 then
         local r = rnd.rnd()*(w+h)*0.5
         light( rnd.rnd()*w-r*0.5, rnd.rnd()*h-r*0.5+0.3*h, r )
      else
         local bw = (0.2+0.6*rnd.rnd())*w
         local bh = (0.1+0.3*rnd.rnd())*h
         rectlight( rnd.rnd()*w-bw*0.5, rnd.rnd()*h-bh*0.5+0.3*h, bw, bh, (rnd.rnd()*2.0-1.0)*math.rad(15) )
      end
   end

   -- Do some lights
   lg.setCanvas( b )
   for i=1,7 do
      lg.setColor( calpha( collight, 0.5+0.4*rnd.rnd() ) )
      local r = (0.1+0.15*rnd.rnd())*(w+h)*0.5
      light( rnd.rnd()*w-r*0.5, (0.1+0.5*rnd.rnd())*h-r*0.5, r )
   end
   lg.setBlendMode("alpha", "premultiplied")
   lg.setCanvas( c )
   lg.draw( b )
   lg.setCanvas()
   lg.setBlendMode("alpha")

   return c
end

function love.load()
   local ww, wh = 800, 600
   love.window.setTitle( "Naev Bar BG Demo" )
   love.window.setMode( ww, wh )

   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 0.5, 0.5, 0.5, 1 )
   img      = love.graphics.newImage( idata )

   bg = generate_bg()
end

function love.keypressed(key)
   if key=="q" or key=="escape" then
      love.event.quit()
   elseif key=="r" then
      bg = generate_bg()
   end
end

function love.draw ()
   lg.draw( bg )
end

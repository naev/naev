local lg = love.graphics

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

local rnd = {
   rnd = love.math.random,
}

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
local function bg_generator( params )
   params = params or {}
   local cvs = lg.newCanvas()
   local w, h = cvs:getDimensions()

   local colbg    = params.colbg    or {0.3, 0.2, 0.1, 1}
   local colfeat  = params.colfeat  or {0.5, 0.3, 0.1, 1}
   local collight = params.collight or {1.0, 0.95, 0.95, 1}
   local featrnd  = params.featrnd  or {0.2, 0.2, 0.2}
   local featalpha = params.featalpha or 0.2
   local featrandonmess = params.featrandonmess or 0.1
   local featscale = params.featscale or 1
   local nfeats   = 20
   local nlights  = params.nlights  or 7
   local lightbrightness = params.lightbrightness or 0.4
   local lightrandomness = params.lightrandomness or 0.3

   local function calpha( c, a )
      return {c[1], c[2], c[3], a}
   end

   -- Do some background
   lg.setCanvas( cvs )
   lg.setColor( colbg )
   gradient( -0.1+0.2*rnd.rnd(), 0.5+0.2*rnd.rnd(), 0, 0, 0, 1 );
   for i=1,nfeats do
      local c = calpha( colfeat, featalpha+featrandonmess*rnd.rnd() )
      c[1] = c[1] + rnd.rnd()*featrnd[1]
      c[2] = c[2] + rnd.rnd()*featrnd[2]
      c[3] = c[3] + rnd.rnd()*featrnd[3]
      lg.setColor( c )
      if rnd.rnd() < 0.6 then
         local r = rnd.rnd()*(w+h)*0.4 * featscale
         light( rnd.rnd()*w-r*0.5, rnd.rnd()*h-r*0.5+0.3*h, r )
      else
         local bw = (0.2+0.4*rnd.rnd())*w * featscale
         local bh = (0.1+0.3*rnd.rnd())*h * featscale
         rectlight( rnd.rnd()*w-bw*0.5, rnd.rnd()*h-bh*0.5+0.3*h, bw, bh, (rnd.rnd()*2.0-1.0)*math.rad(15) )
      end
   end

   -- Do some lights
   for i=1,nlights do
      lg.setColor( calpha( collight, lightbrightness+lightrandomness*rnd.rnd() ) )
      local r = (0.1+0.15*rnd.rnd())*(w+h)*0.5
      light( rnd.rnd()*w-r*0.5, (0.1+0.5*rnd.rnd())*h-r*0.5, r )
   end

   lg.setCanvas()
   return cvs
end

local function bg_inert ()
   return bg_generator{
      colbg    = { 0.5, 0.5, 0.5, 1 },
      colfeat  = { 0.1, 0.1, 0.1, 1 },
      collight = { 0.9, 0.9, 0.9, 1 },
      featrnd  = { 0.1, 0.1, 0.1 },
      nlights = rnd.rnd(6,7),
   }
end

local function bg_desert ()
   return bg_generator{
      colbg    = { 0.5, 0.4, 0.1, 1 },
      colfeat  = { 0.6, 0.5, 0.2, 1 },
      collight = { 1.0, 1.0, 0.9, 1 },
      featrnd  = { 0.2, 0.2, 0.1 },
      nlights  = rnd.rnd(4,6),
   }
end

local function bg_lava ()
   return bg_generator{
      colbg    = { 0.7, 0.4, 0.4, 1 },
      colfeat  = { 0.6, 0.2, 0.2, 1 },
      collight = { 1.0, 0.9, 0.9, 1 },
      featrnd  = { 0.4, 0.2, 0.1 },
      featalpha = 0.4,
      featrandonmess = 0.2,
      nlights  = rnd.rnd(6,8),
   }
end

local function bg_tundra ()
   return bg_generator{
      colbg    = { 0.6, 0.9, 0.9, 1 },
      colfeat  = { 0.4, 0.8, 0.8, 1 },
      collight = { 1.0, 1.0, 1.0, 1 },
      featrnd  = { 0.2, 0.3, 0.3 },
      featscale = 1.5,
      nlights  = rnd.rnd(6,8),
   }
end

local function bg_underwater ()
   return bg_generator{
      colbg    = { 0.3, 0.3, 0.8, 1 },
      colfeat  = { 0.2, 0.2, 0.8, 1 },
      collight = { 0.9, 0.9, 1.0, 1 },
      featrnd  = { 0.8, 0.8, 0.8 },
      nlights  = rnd.rnd(7,9),
   }
end

local function bg_mclass ()
   return bg_generator{
      colbg    = { 0.2, 0.6, 0.2, 1 },
      colfeat  = { 0.2, 0.8, 0.2, 1 },
      collight = { 0.95, 1.0, 0.95, 1 },
      featrnd  = { 0.6, 0.4, 0.6 },
      nlights  = rnd.rnd(3,5),
      featalpha = 0.1,
   }
end

local function bg_station ()
   return bg_generator{
      colbg    = { 0.4, 0.4, 0.4, 1 },
      colfeat  = { 0.1, 0.1, 0.1, 1 },
      collight = { 0.9, 0.9, 0.9, 1 },
      featrnd  = { 0.2, 0.2, 0.2 },
      featalpha = 0.4,
      featrandonmess = 0.2,
      featscale = 0.8,
      nfeats = 50,
      nlights = rnd.rnd(10,15),
      lightbrightness = 0.5,
   }
end

local function bg_generic ()
   return bg_generator{
      nlights = rnd.rnd(6,8)
   }
end

local bgfunc = bg_generic
function love.load()
   local ww, wh = 800, 600
   love.window.setTitle( "Naev Bar BG Demo" )
   love.window.setMode( ww, wh )

   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 0.5, 0.5, 0.5, 1 )
   img      = love.graphics.newImage( idata )

   bg = bgfunc()
end

function love.keypressed(key)
   if key=="q" or key=="escape" then
      love.event.quit()
   elseif key=="1" then
      bgfunc = bg_generic
   elseif key=="2" then
      bgfunc = bg_station
   elseif key=="3" then
      bgfunc = bg_mclass
   elseif key=="4" then
      bgfunc = bg_underwater
   elseif key=="5" then
      bgfunc = bg_tundra
   elseif key=="6" then
      bgfunc = bg_lava
   elseif key=="7" then
      bgfunc = bg_desert
   elseif key=="8" then
      bgfunc = bg_inert
   elseif key~="r" then
      return
   end

   bg = bgfunc()
end

function love.draw ()
   lg.draw( bg )
end

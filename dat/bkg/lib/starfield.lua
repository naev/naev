--[[
   Some sort of stellar wind type background.
--]]
local bgshaders = require "bkg.lib.bgshaders"
local love = require 'love'
local love_shaders = require 'love_shaders'
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local prng = require("prng").new()

local starfield = {}

starfield.stars = {
   "blue01.webp",
   "blue02.webp",
   "blue04.webp",
   "green01.webp",
   "green02.webp",
   "orange01.webp",
   "orange02.webp",
   "orange05.webp",
   "redgiant01.webp",
   "white01.webp",
   "white02.webp",
   "yellow01.webp",
   "yellow02.webp"
}

local starfield_frag = lf.read('bkg/shaders/starfield.frag')

local cvs, texw, texh -- For static shader
local shader, sstarfield, sf, sz, sb -- For dynamic shader

local function star_add( added, num_added )
   -- Set up parameters
   local path  = "gfx/bkg/star/"
   -- Avoid repeating stars
   local stars = starfield.stars
   local cur_sys = system.cur()
   local num   = prng:random(1,#stars)
   local i     = 0
   while added[num] and i < 10 do
      num = prng:random(1,#stars)
      i   = i + 1
   end
   local star  = stars[ num ]
   -- Load and set stuff
   local img   = tex.open( path .. star )
   -- Position should depend on whether there's more than a star in the system
   local r     = prng:random() * cur_sys:radius()/3
   if num_added > 0 then
      r        = r + cur_sys:radius()*2/3
   end
   local a     = 2*math.pi*prng:random()
   local x     = r*math.cos(a)
   local y     = r*math.sin(a)
   local nmove = math.max( 0.05, prng:random()*0.1 )
   local move  = 0.02 + nmove
   local scale = 1.0 - (1 - nmove/0.2)/5
   scale = scale * 0.75
   -- Now, none of this makes sense, the "star dust" should be rendered on top
   -- of the star because it moves faster the stars and should be closer,
   -- however, it seems like this is actually a bit jarring, even though it's
   -- more correct, so we just mess things up and make the star render in front
   -- of the space dust. Has to move faster than the nebula to not be really really weird.
   bkg.image( img, x, y, move, scale, nil, nil, true )
   return num
end

local function add_local_stars ()
   -- Chose number to generate
   local n
   local r = prng:random()
   if r > 0.97 then
      n = 3
   elseif r > 0.94 then
      n = 2
   elseif r > 0.1 then
      n = 1
   end

   -- If there is an inhabited planet we'll need at least one star
   if not n then
      for _k,v in ipairs( system.cur():spobs() ) do
         if v:services().land then
            n = 1
            break
         end
      end
   end

   -- Generate the stars
   local i = 0
   local added = {}
   while n and i < n do
      local num = star_add( added, i )
      added[ num ] = true
      i = i + 1
   end
end

local static = true
function starfield.init( params )
   params = params or {}
   local nconf = naev.conf()
   static = params.static or not nconf.background_fancy
   local seed = params.seed or system.cur():nameRaw()

   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = math.max( 1.0, nconf.nebu_scale * 0.5 )

   -- Per system parameters
   prng:setSeed( seed )
   local theta = prng:random() * math.pi/10.0
   local phi = prng:random() * math.pi/10.0
   local psi = prng:random() * math.pi/10.0
   local rx, ry = vec2.newP( 3+1*prng:random(), 7+1*prng:random() ):get()
   local rz = 5+1*prng:random()
   --rx, ry, rz = 5, 7, 11
   sz = 1+1*prng:random()
   sb = nconf.bg_brightness

   local motionblur = 1
   if static then
      motionblur = 0
   end

   -- Ensure we're caught up with the current window/screen dimensions.
   love.origin()
   -- Initialize shader
   shader = lg.newShader( string.format(starfield_frag, motionblur, rx, ry, rz, theta, phi, psi), love_shaders.vertexcode )

   if static then
      if params.size then
         texw = params.size
         texh = params.size
      else
         local nw, nh = gfx.dim()
         texw = nw
         texh = nh
         local texs = 4096 / math.max( texw, texh )
         if texs < 1 then
            texw = texw / texs
            texh = texh / texs
         end
      end
      cvs = lg.newCanvas( texw, texh, {dpiscale=1} )
      shader:send( "u_camera", 0, 0, sz, 0.0008*sf )

      local oldcanvas = lg.getCanvas()
      lg.setCanvas( cvs )
      lg.clear( 0, 0, 0, 0 )
      lg.setShader( shader )
      lg.setColour( {1,1,1,1} )
      love_shaders.img:draw( 0, 0, 0, texw, texh )
      lg.setShader()
      lg.setCanvas( oldcanvas )
   else
      sstarfield = bgshaders.init( shader, sf, {usetex=true} )
   end

   if not params.nolocalstars then
      add_local_stars()
      -- TODO 3D lighting
   end

   -- Default to some weak ambient light
   gfx.lightAmbient( 0.1 )
end

function starfield.canvas ()
   return cvs
end

function starfield.render( dt )
   if static then
      lg.setColour( {sb,sb,sb,1} )
      cvs:draw( 0, 0 )
      return
   end
   -- Get camera properties
   local x, y, z = camera.get()
   x = x / 1e6
   y = y / 1e6
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, sz, z*0.0008*sf )

   sstarfield:render( dt )
end

return starfield

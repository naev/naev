--[[
   Some sort of stellar wind type background.
--]]
local love = require 'love'
local love_shaders = require 'love_shaders'
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local prng = require("prng").new()

local starfield = {}

-- Radiosity has been computed by blurring the base images and getting a representative colour
-- Alpha can be used to control the intensity of the radiosity and multiplies the RGB values
starfield.stars = {
   { i="blue01.webp",     r=colour.new(0.80, 0.87, 0.96, 6) },
   { i="blue02.webp",     r=colour.new(0.76, 0.88, 1.00, 6) },
   { i="blue04.webp",     r=colour.new(0.83, 0.93, 1.00, 6) },
   { i="green01.webp",    r=colour.new(0.80, 0.97, 0.78, 6) },
   { i="green02.webp",    r=colour.new(0.89, 0.98, 0.86, 6) },
   { i="orange01.webp",   r=colour.new(0.94, 0.50, 0.20, 6) }, -- r=colour.new(0.94, 0.30, 0.00, 8) }, Too red otherwise
   { i="orange02.webp",   r=colour.new(1.00, 0.90, 0.67, 6) },
   { i="orange05.webp",   r=colour.new(0.98, 0.86, 0.45, 6) },
   { i="redgiant01.webp", r=colour.new(0.78, 0.50, 0.50, 6) }, -- r=colour.new(0.57, 0.00, 0.00, 8) }, Too red otherwise
   --{ i="redgiant02.webp", r=colour.new(0.82, 0.53, 0.26, 6) }, -- Unused
   { i="white01.webp",    r=colour.new(0.68, 0.91, 0.96, 6) },
   { i="white02.webp",    r=colour.new(0.89, 0.92, 0.96, 6) },
   { i="yellow01.webp",   r=colour.new(1.00, 0.97, 0.82, 6) },
   { i="yellow02.webp",   r=colour.new(1.00, 0.95, 0.57, 6) },
}

local starfield_frag = lf.read('bkg/shaders/starfield.frag')

local cvs, texw, texh, sb

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
   local data  = stars[ num ]
   local star  = data.i
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
   local rad = data.r
   bkg.image( img, x, y, move, scale, nil, nil, true, rad )
   return num
end

local function add_local_stars ()
   -- Chose number to generate
   local n
   local r = prng:random()
   if r > 0.97 then
      n = 3
      gfx.lightIntensity( 0.10 ) -- sun gives 3*8*0.10 = 1.8
   elseif r > 0.94 then
      n = 2
      gfx.lightIntensity( 0.15 ) -- sun gives 2*8*0.15 = 1.8
   elseif r > 0.1 then
      n = 1
      gfx.lightIntensity( 0.25 ) -- sun gives 8*0.25 = 1.5
      gfx.lightAmbient( 0.05 )
   else
      gfx.lightAmbient( 0.1 ) -- Default to some weak ambient light
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

function starfield.init( params )
   params = params or {}
   local nconf = naev.conf()
   local seed = params.seed or system.cur():nameRaw()

   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   local sf = math.max( 1.0, nconf.nebu_scale * 0.5 )

   -- Per system parameters
   prng:setSeed( seed )
   local theta = prng:random() * math.pi/10.0
   local phi = prng:random() * math.pi/10.0
   local psi = prng:random() * math.pi/10.0
   local rx, ry = vec2.newP( 3+1*prng:random(), 7+1*prng:random() ):get()
   local rz = 5+1*prng:random()
   --rx, ry, rz = 5, 7, 11
   local sz = 1+1*prng:random()
   sb = nconf.bg_brightness

   -- Ensure we're caught up with the current window/screen dimensions.
   love.origin()
   -- Initialize shader
   local shader = lg.newShader( string.format(starfield_frag, rx, ry, rz, theta, phi, psi), love_shaders.vertexcode )

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

   if not params.nolocalstars then
      add_local_stars()
   end
end

function starfield.canvas ()
   return cvs
end

function starfield.render( _dt )
   lg.setColour( {sb,sb,sb,1} )
   cvs:draw( 0, 0 )
end

return starfield

--[[
   Nebula framework.
--]]
local starfield = require "bkg.lib.starfield"
local prng = require("prng").new()
-- We use the default background too!
require "bkg.default"

local nebula_image = {}

nebula_image.nebula_list = {
   "butterfly.webp",
   "cats_eye.webp",
   "cats_paw.webp",
   "crab_nebula.webp",
   "dobayashi.webp",
   "ic.webp",
   "messier.webp",
   "ncg-2392.webp",
   "ncg-2440-2.webp",
   "ncg-2440.webp",
   "ncg-5189.webp",
}

function nebula_image.init( filename )
   --local background_default = background
   function background ()
      local csys = system.cur()
      prng:setSeed( csys:nameRaw() )

      local path  = "gfx/bkg/nebula/"..filename
      local img   = tex.open( path )
      local nw,nh = gfx.dim()
      local w,h   = img:dim()
      local r     = prng:random() * csys:radius()
      local a     = 2*math.pi*prng:random()
      local move  = 0.002 + prng:random()*0.001
      local x     = 10*r*math.cos(a)
      local y     = 10*r*math.sin(a)
      local scale = 1 + (prng:random()*0.5 + 0.5)*((2048+2048)/(w+h))
      local angle = prng:random()*math.pi*2
      local col   = colour.new( 0.6, 0.6, 0.6, 1.0 )
      local md    = (w+h)/2
      if 1280 < scale * md then
         scale = 1280 / md
      end
      scale = scale * (nw*nh)/(1280*720)
      bkg.image( img, x, y, move, scale, angle, col )

      -- Default nebula background
      -- A bit too crowded so disabled for now
      --background_default()

      -- Just use starfield
      starfield.init()
      renderbg = starfield.render
   end
end

return nebula_image

--[[
   Nebula framework.
--]]
local lf = require 'love.filesystem'
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
   local background_default = background
   function background ()
      local csys = system.cur()
      prng:setSeed( csys:nameRaw() )

      local path  = "gfx/bkg/nebula/"..filename
      local img   = tex.open( path )
      local nw,nh = gfx.dim()
      local w,h   = img:dim()
      local r     = prng:random() * csys:radius()/2
      local a     = 2*math.pi*prng:random()
      local x     = r*math.cos(a)
      local y     = r*math.sin(a)
      local move  = 0.01 + prng:random()*0.01
      local scale = 1 + (prng:random()*0.5 + 0.5)*((2048+2048)/(w+h))
      local angle = prng:random()*math.pi*2
      if scale > 1.9 then
         scale = 1.9
      end
      scale = scale * (nw*nh)/(1280*720)
      bkg.image( img, x, y, move, scale, angle )

      -- Default nebula background
      background_default()
   end
end

return nebula_image

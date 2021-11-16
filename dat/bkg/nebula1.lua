--[[
   Nearby Nebula background
--]]
local starfield = require "bkg.starfield"
local nebula = require "bkg.nebula"

function background ()
   nebula.init()
   starfield.init()
end

renderbg = starfield.render

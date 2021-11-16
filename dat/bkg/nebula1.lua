--[[
   Nearby Nebula background
--]]
local starfield = require "bkg.lib.starfield"
local nebula = require "bkg.lib.nebula"

function background ()
   nebula.init()
   starfield.init()
end

renderbg = starfield.render

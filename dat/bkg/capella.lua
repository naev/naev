--[[
   4 + 1 extra star for every cycle since the Incident
--]]
local starfield = require "bkg.lib.starfield"
function background ()
   local c = time.sub(time.cur(), time.new( 593, 3726, 4663 )):split()
   starfield.init( { num_stars = math.max(4, 4+c/10) } )
end
renderbg = starfield.render

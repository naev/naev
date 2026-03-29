--[[
   4 + 1 extra star for every cycle since the Incident
--]]
local starfield = require "bkg.lib.starfield"
function background ()
   local c = time.sub(time.cur(), time.new( 593, 3726, 4663 )):split() -- ""Fake"" stars
   local n = 4 -- Base stars
   starfield.init( { num_stars = n } )
   
   local added = {}
   for i = 1, c do
      local num = starfield.star_add( added, n+i, true )
      added[ num ] = true
   end
   
end
renderbg = starfield.render

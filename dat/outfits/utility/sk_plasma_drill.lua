local sokoban = require "minigames.sokoban"

-- Global stats
local dist_threshold = math.pow( 50, 2 )
local vel_threshold  = math.pow( 15, 2 )

function init( _p, po )
   -- Since this outfit is usually off, we use shipstats to forcibly set the
   -- base stats
   po:set( "misc_asteroid_scan", 1 ) -- Doesn't support booleans
end

function ontoggle( p, _po, on )
   if not on then
      return true
   end

   -- See if there's an asteroid targetted
   local a = p:targetAsteroid()
   if not a then
      -- Get nearest if not found
      a = asteroid.get( p )
      if not a then
         return false
      end
      p:setTargetAsteroid( a )
   end

   -- Check if in range
   if a:pos():dist2( p:pos() ) > dist_threshold then
      return false
   end

   -- Check relative velocity
   if a:vel():dist2( p:vel() ) > vel_threshold then
      return false
   end

   sokoban.love()

   return true
end

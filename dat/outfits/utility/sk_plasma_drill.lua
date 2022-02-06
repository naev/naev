local mining = require "minigames.mining"

-- Global stats
local dist_threshold = math.pow( 50, 2 )
local vel_threshold  = math.pow( 15, 2 )

function init( p, po )
   -- Since this outfit is usually off, we use shipstats to forcibly set the
   -- base stats
   po:set( "misc_asteroid_scan", 1 ) -- Doesn't support booleans
   mem.isp = (p == player.pilot())
end

function ontoggle( p, _po, on )
   if not on then
      return false
   end

   -- See if there's an asteroid targetted
   local a = p:targetAsteroid()
   if not a or not p:inrangeAsteroid( a ) then
      -- Get nearest if not found
      a = asteroid.get( p )
      if not a or not p:inrangeAsteroid( a ) then
         if mem.isp then
            player.msg("#r".._("No asteroids available to mine"))
         end
         return false
      end
      p:setTargetAsteroid( a )
   end

   -- Check if in range
   if a:pos():dist2( p:pos() ) > dist_threshold then
      if mem.isp then
         player.msg("#r".._("You are too far from the asteroid to mine"))
      end
      return false
   end

   -- Check relative velocity
   if a:vel():dist2( p:vel() ) > vel_threshold then
      if mem.isp then
         player.msg("#r".._("You are moving too fast to mine the asteroid"))
      end
      return false
   end

   -- Only player gets minigame
   if mem.isp then
      mining.love()
      mining.reward() -- TODO do something with the reward
   end
   a:setTimer( -1 ) -- Get rid of the asteroid

   return false
end

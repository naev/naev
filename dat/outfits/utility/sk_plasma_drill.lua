local mining = require "minigames.mining"
local fmt = require "format"

-- Global stats
local dist_threshold = math.pow( 50, 2 )
local vel_threshold  = math.pow( 15, 2 )

function init( p, po )
   -- Since this outfit is usually off, we use shipstats to forcibly set the
   -- base stats
   po:set( "asteroid_scan", 200 )
   po:set( "mining_bonus", 1.5 )
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
      -- Try to figure out difficulty
      local max_rarity = -1
      local mat = a:materials()
      for k,m in ipairs(mat) do
         max_rarity = math.max( max_rarity, m.rarity )
      end

      mining.love{ difficulty=max_rarity }
      --local bonus = mining.reward()

      local r = max_rarity -- TODO reward-based
      local rwd = {}
      for k,m in ipairs(mat) do
         if m.rarity==r then
            table.insert( rwd, m )
         end
      end
      if #rwd > 0 then
         local rget = rwd[ rnd.rnd(1,#rwd) ]
         local bonus = p:shipstat("mining_bonus",true)
         local c = rget.commodity
         local q = math.floor(rget.quantity * bonus + 0.5)
         p:cargoAdd( c, q )
         player.msg("#g"..fmt.f(_("You obtained {amount} of {cargo}"),{amount=fmt.tonnes(q),cargo=c}))
      end
   end
   a:setTimer( -1 ) -- Get rid of the asteroid

   return false
end

--[[
Library for handling mining drills with various properties.
--]]
local drill = {}

local mining = require "minigames.mining"
local fmt = require "format"
local helper = require "outfits.lib.helper"

-- Global stats
local dist_threshold = math.pow( 50, 2 )
local vel_threshold  = math.pow( 15, 2 )

function drill.setup( p, _po, params )
   params = params or {}

   mem.isp = (p == player.pilot())

   drill.speed = params.speed or math.pi
   drill.shots_max = params.shots_max or 3
end

function drill.ontoggle( p, _po, on )
   if not on then
      mem.lastmsg = nil -- clear helper.msgnospam timer
      return false
   end

   -- See if there's an asteroid targetted
   local a = p:targetAsteroid()
   if not a or not p:inrange( a ) then
      -- Get nearest if not found
      a = asteroid.get( p )
      if not a or not p:inrange( a ) then
         if mem.isp then
            helper.msgnospam("#r".._("No asteroids available to mine"))
         end
         return false
      end
      p:setTargetAsteroid( a )
   end

   -- Make sure it exists (should do a foreground check)
   if not a:exists() then
      return
   end

   -- Check if in range
   if a:pos():dist2( p:pos() ) > dist_threshold then
      if mem.isp then
         helper.msgnospam("#r".._("You are too far from the asteroid to mine"))
      end
      return false
   end

   -- Check relative velocity
   if a:vel():dist2( p:vel() ) > vel_threshold then
      if mem.isp then
         helper.msgnospam("#r".._("You are moving too fast to mine the asteroid"))
      end
      return false
   end

   -- Try to figure out difficulty
   local max_rarity = -1
   local mat = a:materials()
   for k,m in ipairs(mat) do
      max_rarity = math.max( max_rarity, m.rarity )
   end

   -- Handles giving the function
   local function reward( bonus )
      local r = max_rarity
      if bonus < rnd.rnd() then
         r = math.max(r-1,0)
      end
      local rwd = {}
      for k,m in ipairs(mat) do
         if m.rarity==r then
            table.insert( rwd, m )
         end
      end
      if #rwd > 0 then
         local rget = rwd[ rnd.rnd(1,#rwd) ]
         local rwd_bonus = p:shipstat("mining_bonus",true)
         local c = rget.commodity
         local q = math.floor(rget.quantity * (rwd_bonus*rnd.rnd()+0.5) + 0.5)
         p:cargoAdd( c, q )
         if mem.isp then
            player.msg("#g"..fmt.f(_("You obtained {amount} of {cargo}"),{amount=fmt.tonnes(q),cargo=c}))
         end
         return c, q
      end
   end

   -- Only player gets minigame
   if mem.isp then
      local time_mod = p:shipstat("time_mod",true)
      mining.love{
         speed = drill.speed * time_mod,
         shots_max = drill.shots_max,
         difficulty = max_rarity,
         reward_func = reward,
      }
   else
      -- AI doesn't actually have to mine
      reward( 0 )
   end

   for k,s in ipairs( pilot.getInrange( a:pos(), a:alertRange() ) ) do
      pilot.msg( nil, s, "asteroid", a )
   end

   a:setTimer( -1 ) -- Get rid of the asteroid

   if mem.isp then
      naev.trigger( "mine_drill" ) -- So we can catch it
   end
   return false
end

return drill

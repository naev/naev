--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Pirate System Raid Manager">
 <location>load</location>
 <chance>100</chance>
 <unique />
 <priority>99</priority>
</event>
--]]

--[[
   Manages pirate raids in the background.
--]]
--local fmt = require "format"
local lmisn = require "lmisn"

local PIRATE            = faction.get("Pirate")
local CHANCE_PER_PERIOD = 0.03

function create ()
   -- Must clear cache on load / create new game
   naev.cache()._pirate_raid_active = {}
   mem.time    = time.cur()
   hook.date( time.new( 0, 1, 0 ), "date" )
end

local function should_be_active ()
   return player.chapter() ~= "0"
end

local function has_inhabited_generic_spob( sys )
   for k,v in ipairs(sys:spobs()) do
      local f = v:faction()
      if f and f:tags().generic then
         return true
      end
   end
   return false
end

function date ()
   if not should_be_active() then return end

   if rnd.rnd() > CHANCE_PER_PERIOD then return end

   local nc    = naev.cache()
   local pra   = nc._pirate_raid_active or {}

   -- Centred around the player somewhat
   local candidates = lmisn.getSysAtDistance( nil, 0, 9, function ( sys )
      -- No active raid
      if pra[ sys:nameRaw() ] then return false end
      -- Has some pirate presence
      if sys:presence( PIRATE ) <= 0 then return false end
      -- Claimable
      if not naev.claimTest( sys, true ) then return false end
      -- Has an inhabited "generic" spob
      if not has_inhabited_generic_spob( sys ) then return false end
      return true
   end )

   -- Set up and run the sub-event
   nc._pirate_raid = {
      sys = candidates[ rnd.rnd(#candidates) ],
   }
   naev.eventStart("Pirate System Raid")
   nc._pirate_raid = nil
end

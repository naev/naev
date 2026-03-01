--[[
<?xml version='1.0' encoding='utf8'?>
<event name="P-1587 Dragon Awake">
 <location>load</location>
 <chance>100</chance>
 <cond>player.evtDone("P-1587 Dragon Intro")</cond>
 <unique />
</event>
--]]
local SPB, SYS = spob.getS("P-1587")
local ferals = require "common.ferals"
local ai_setup = require "ai.core.setup"
local pilotai = require "pilotai"

function create ()
   if not evt.claim( SYS, true ) then
      return evt.finish(false)
   end

   hook.enter("enter")
end

local dragon
function spawn_dragon()
   local fct = ferals.faction()
   dragon = pilot.add( "Kauweke", fct, SPB )
   dragon:outfitAddSlot( outfit.get("Plasma Burst"), "plasma_burst", true)
   ai_setup.setup( dragon ) -- So plasma burst gets used
   dragon:intrinsicSet( "armour_regen", 100 )
   dragon:intrinsicSet( "armour", 50 )
   dragon:setHostile(true)
   dragon:setNoDisable(true)
   hook.pilot( dragon, "death", "dragon_died" )

   pilotai.guard( dragon, vec2.newP( 200, rnd.angle() ) )
end

function enter ()
   if system.cur()~=SYS then return end

   SPB:landDeny(true)
   hook.timer( rnd.rnd(3,5), "spawn_dragon" )
end

function dragon_died ()
   SPB:landDeny(false)
   evt.finish(true)
end

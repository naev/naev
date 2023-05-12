--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Intelligence">
 <priority>3</priority>
 <chance>150</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <cond>player.numOutfit("Mercenary License") &gt; 0</cond>
 <notes>
  <tier>1</tier>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Intelligence
-- This is the mercenary mission corresponding to "Dvaered Census"
-- The player has to get in range of a given amount of Pirate Ships in a given system.

   Stages :
   0) Way to system and in-system traffic
   1) Way to any Dvaered Spob to get paid
--]]
local cens   = require "common.census"
local fmt    = require "format"
local dv     = require "common.dvaered"
local pir    = require "common.pirate"
local vntk   = require 'vntk'


local detected

-- Estimate travelling risk (similar to what is done in cargo)
-- /!\ This is relative to the faction, and ideally not to the current system
-- I'm not sure about merging with cargo code as coefficients are different
-- Because we assume the player can hide and defend themselves
local function travellingRisk( system )

   -- If there is a Spob of the faction is system, risk is 0 for the faction
   local spobs = system:spobs()
   for i, p in ipairs(spobs) do
      if p:faction() == faction.get("Dvaered") then
         return 0
      end
   end

   local guard_factions = {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Soromid"),
      faction.get("Sirius"),
      faction.get("Za'lek"),
      faction.get("Frontier"),
   }
   local jumps = system.cur():jumpPath( system, 0 )
   local risk = 0
   for k, v in ipairs(jumps) do
      local p = v:system():presences()
      local pirates = 0
      local guards  = 0
      for i, f in ipairs( pir.factions ) do
         pirates = pirates + (p[f:nameRaw()] or 0)
      end
      for i, f in ipairs( guard_factions ) do
         guards = guards + (p[f:nameRaw()] or 0)
      end
      risk = risk + math.max( 0, pirates - guards )
   end
   return risk
end

function create ()
   mem.sys = cens.findTarget( 0, 5, "Pirate", 10 ) -- Use standard pirates to see if system is suitable
   if not misn.claim(mem.sys) then misn.finish(false) end

   local credits1
   mem.nbships, credits1 = cens.calculateNb( mem.sys, pir.factions )
   local risk = travellingRisk( mem.sys )
   mem.credits = credits1*2 + risk*500

   -- Mission details
   misn.setTitle(fmt.f(dv.prefix.._("Monitor Pirate Activity in {sys}"), {sys=mem.sys}))
   misn.setReward( mem.credits )
   misn.setDesc( fmt.f(_("Dvaered High Command requires a pilot to go to {sys} and detect {nb} Pirate ships"), {sys=mem.sys, nb=mem.nbships}))
   mem.misn_marker = misn.markerAdd( mem.sys )
end

function accept()
   misn.accept()
   mem.misn_state = 0
   cens.osd( _("Dvaered Intelligence"), mem.sys, mem.nbships, 0, _("Dvaered"), _("Pirate") )
   hook.enter("enter")
   hook.land("land")
end

function enter()
   -- Enters in the mission's system
   if mem.misn_state == 0 then
      if system.cur() == mem.sys then
         detected = {}
         testInRange()
      end
   end
   -- Note: there is no test on the player leaving the system before having scanned enough ships.
   -- This behaviour is totally allowed.
end

function land()
   --Pay the player
   if spob.cur():faction() == faction.get( "Dvaered" ) and mem.misn_state == 1 then
      vntk.msg( _("Mission Complete"), fmt.f(_([[You land and transmit a datapad to the local Dvaered liaison officer.
{reward}]]),{reward=fmt.reward(mem.credits)}) )
      player.pay( mem.credits )
      faction.modPlayerSingle("Dvaered", rnd.rnd(1, 2))
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.finish(true)
   end
end

-- Test pilots in range.
function testInRange()
   if system.cur() ~= mem.sys then
      return -- Wrong system: abort
   end

   detected = cens.testInRange( detected, pir.factions )
   -- Test if mission is complete
   if mem.nbships <= #detected then
      misn.osdActive(2)
      mem.misn_state = 1
      misn.markerRm( mem.misn_marker )
      player.msg( _("You have acquired data on enough Pirate ships") )
      return
   end
   cens.osd( _("Dvaered Census"), mem.sys, mem.nbships, #detected, _("Dvaered"), _("Pirate") )
   hook.timer(1, "testInRange") -- Recursivity 1 s
end

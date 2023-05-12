--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Census">
 <priority>3</priority>
 <chance>150</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <cond>
   if faction.playerStanding("Dvaered") &lt; 0 then
      return false
   end
   return require("misn_test").computer()
 </cond>
 <done>Dvaered Census 0</done>
 <notes>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Census
-- This is the default mission introduced by "Dvaered Census 0"
-- The player has to get in range of a given amount of Dvaered Ships in a given system.

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

function create ()
   mem.sys = cens.findTarget( 0, 3, "Dvaered", 50 )
   if not misn.claim(mem.sys) then misn.finish(false) end

   mem.nbships, mem.credits = cens.calculateNb( mem.sys, {faction.get("Dvaered")} )

   -- Mission details
   misn.setTitle(fmt.f(dv.prefix.._("Monitoring of Warlords activity in {sys}"), {sys=mem.sys}))
   misn.setReward( mem.credits )
   misn.setDesc( fmt.f(_("Dvaered High Command requires a pilot to go to {sys} and detect {nb} Dvaered ships"), {sys=mem.sys, nb=mem.nbships}))
   mem.misn_marker = misn.markerAdd( mem.sys )
end

function accept()
   misn.accept()
   mem.misn_state = 0
   cens.osd( _("Dvaered Census"), mem.sys, mem.nbships, 0, _("Dvaered"), _("Dvaered") )
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

      -- Manage counting of missions
      local n = var.peek("dc_misn")
      if n ~= nil then
         var.push("dc_misn", n+1)
      else
         var.push("dc_misn", 1)
      end

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

   detected = cens.testInRange( detected, {faction.get("Dvaered")} )
   -- Test if mission is complete
   if mem.nbships <= #detected then
      misn.osdActive(2)
      mem.misn_state = 1
      misn.markerRm( mem.misn_marker )
      player.msg( _("You have acquired data on enough Dvaered ships") )
      return
   end
   cens.osd( _("Dvaered Census"), mem.sys, mem.nbships, #detected, _("Dvaered"), _("Dvaered") )
   hook.timer(1, "testInRange") -- Recursivity 1 s
end

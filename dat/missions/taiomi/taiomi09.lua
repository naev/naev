--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 9">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 8</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 09

   Player has to make a deal with a smuggler to bring goods to bastion.
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
--local der = require 'common.derelict'
--local pilotai = require "pilotai"

local reward = taiomi.rewards.taiomi09
local title = _("Smuggler's Deal")
local base, _basesys = spob.getS("One-Wing Goddard")
local smugden, smugsys = spob.getS("Darkshed")
local startspob, startsys = spob.getS("Arrakis")
local fightsys = system.get("Gamel")
local handoffsys = system.get("Bastion")

--[[
   0: mission started
   1: met smuggler
   2: made deal with smuggler
   3: escort from xxx to bastion
   4: fight done
   5: cutscene done
--]]
mem.state = 0

function create ()
   if not misn.claim{ startsys, fightsys, handoffsys } then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )

   misn.setDesc(_(""))
   misn.setReward( fmt.credits(reward) )

   misn.markerAdd( smugden )

   misn.osdCreate( title, {
      fmt.f(_("Find the smuggler in {spob} ({sys})"),{spob=smugden, sys=smugsys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

local land_smuggler, land_escorts, land_final
function land ()
   local scur = spob.cur()
   if mem.state == 0 and scur==smugden then
      land_smuggler()
   elseif mem.state==3 and scur==startspob then
      land_escorts()
   elseif mem.state==5 and scur==base then
      land_final()
   end
end

function land_smuggler ()
end

function land_escorts ()
end

function land_final ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and and find eagerly Scavenger waiting for you.]]))
   s(_([[""]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_(""))
   misn.finish(true)
end

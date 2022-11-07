--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 10">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 9</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 10

   Defend the hypergate and final cutscene!
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local lmisn = require "lmisn"

local title = _("Final Breath of Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")

--[[
   0: mission started
   1: landed on goddard
   2: defend
   3: cutscene done
--]]
mem.state = 0

function create ()
   if not misn.claim{ basesys } then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )

   misn.setDesc(_("You have been tasked to help the citizens of Taiomi reach their freedom!"))
   misn.setReward(_("Freedom for the Taiomi Citizens!"))

   mem.marker = misn.markerAdd( base )
   misn.osdCreate( title, {
      fmt.f(_("Land on {spob} ({sys})"),{spob=base, sys=basesys})
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

function land ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   s(_([[""]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()

   misn.osdCreate( title, {
      fmt.f(_("Defend the {base}"), {base=base})
   } )
   misn.markerMove( mem.marker, basesys )
end

local hypergate
function enter ()
   if mem.state ~= 0 then
      lmisn.fail(_([[You were supposed to protect the hypergate!"]]))
   end
   mem.state = 1 -- start

   --local pos = base:pos()
   diff.apply("onewing_goddard_gone")
   hypergate = pilot.add( "One-Wing Goddard", "Independent", player.pos(), nil, {naked=true, ai="dummy"} )
   hypergate:disable()
   hypergate:setHilight(true)
   hook.pilot( hypergate, "death", "hypergate_dead" )
end

function hypergate_dead ()
   -- TODO big explosion and kill player
end

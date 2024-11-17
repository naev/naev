--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Sigma-13 Zach">
 <location>land</location>
 <chance>100</chance>
 <cond>player.misnDone("Za'lek Black Hole 11")</cond>
 <spob>Research Post Sigma-13</spob>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</event>
--]]
--[[
   Sort of follow-up for Zach.
--]]
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"

local sys1 = system.get("NGC-13674")
local sys2 = system.get("Copernicus")
local j1, j2 = jump.get( sys1, sys2 )
local reward = outfit.get("Antimatter Lance")

function create ()
   evt.npcAdd( "approach_zach", _("Zach"), zbh.zach.portrait, zbh.zach.description )
   hook.takeoff( "leave" )
end

function approach_zach ()
   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )

   if not j1:known() or not j2:known() then
      z(fmt.f(_([["Heya. I've been putting to good use the sensor upgrades you brought me and found an astounding discovery! There seems to be a jump lane between {sys1} and {sys2}! It was very weak and I missed it for a while, but I sent a drone to confirm and it seems to be fully working!"]]),
         {sys1=sys1,sys2=sys2}))
      z(_([["I'll update your system navigation with the new jump so you'll be able to come visit me more easily. Although the drones do keep me company, it's not the same, you know?"]]))

      j1:setKnown(true)
      j2:setKnown(true)

   else
      local msglist = {
         _([["Heya, back already?"]]),
         _([["I wonder what Icarus is up to…"]]),
         fmt.f(_([["How are you liking the {outfit}?"]]),{outfit=reward}),
      }

      local msg = msglist[ rnd.rnd(1,#msglist) ]
      z(msg)

   end

   vn.done( zbh.zach.transition )

   vn.run()
end

function leave ()
   evt.finish()
end

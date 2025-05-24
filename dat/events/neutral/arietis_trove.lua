--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Arietis Trove">
 <unique/>
 <location>land</location>
 <chance>100</chance>
 <spob>Arietis C</spob>
</event>
--]]
local vn = require "vn"
local fmt = require "format"
local tut = require "common.tutorial"
local vne = require "vnextras"

local reward = outfit.get("Prototype Systems T-IVa 'Quickshell'")

function create ()
   local done = false
   local thurion_known = faction.get("Thurion"):known()

   vn.clear()
   vn.scene()

   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )

   vn.na(fmt.f(_([[You touch down on {spb}, and {shipai} materializes before you.]]),
      {spb=spob.cur(), shipai=tut.ainame()}))
   if thurion_known then
      sai(_([["That is quite weird to be attacked by a Thurion ship, maybe related to it seemingly having all its communication protocols garbled. It also seemed to be protecting something or interested in this planet. Strange..."]]))
   else
      sai(_([["That is quite weird to be attacked by an unknown ship. I was not able to find anything in my database related to it. Curious. It also seemed to be protecting something or interested in this planet. Strange..."]]))
   end
   sai(_([["I've run a preliminary analysis of the planet and there seems to be some structure uncovered nearby."]]))
   vn.menu{
      {_("Have a look."), "01_look"},
      {_("Maybe later."), "01_later"},
   }

   vn.label("01_later")
   vn.done( tut.shipai.transition )

   vn.label("01_look")
   vn.scene()
   vn.transition( tut.shipai.transition )
   vn.na(_([[You go to the coordinates provided to you and find that in the upheaval of the planet surface, there seems to be an exposed underground structure. With great care, you enter.]]))
   vn.na(_([[It seems to be an ancient abandoned research facility, although most is clean, you manage to find a room that seems to have a large system core, with a terminal connected to it. Lucky that anything was left at all.]]))
   vn.na(_([[You fire up the Terminal, and after several tries, manage to get it to start up with an auxiliary power hot-rigged to it. It seems like most is corrupted, but you manage to find a file.]]))

   vn.scene()
   local log = vne.flashbackTextStart()
   -- Same as the outfit description
   log(_([[Research Log UST 102:2718
   The last prototype has exceeded expectations in shield regeneration capabilities, this may be the breakthrough we were hoping for. With such incredible results, the Empire shall surely see the power of the clustered sentience interface. Although the shield capacity parameters are suboptimal, that is just a matter of time. It is only a matter of time before Project Thurion because the technological backbone of civilization!]]))
   vne.flashbackTextEnd()

   if thurion_known then
      vn.na(_([[Seems like this place is older than you thought, and also somehow related to the Thurion you met in the Nebula.]]))
   else
      vn.na(_([[Seems like this place is older than you thought, but it's not clear what this Project Thurion is.]]))
   end
   vn.na(_([[Either way, it seems like there is not much to do here other than take the core system back with you. It may be useful on some of your ships.]]))

   vn.func( function ()
      player.outfitAdd( reward )
      done = true
   end )
   vn.sfxBingo()
   vn.na(fmt.reward(reward))

   vn.done()
   vn.run()

   evt.finish( done )
end

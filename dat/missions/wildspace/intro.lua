--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Old Friends at Protera Husk">
 <unique />
 <chance>100</chance>
 <location>Bar</location>
 <spob>Hypergate Protera</spob>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
  <done_evt name="Welcome to Wild Space" />
 </notes>
</mission>
--]]
--[[
   Wild Space Introduction

   Small mission to recover something from Protera Husk. Mainly for Lore / Flavour.
]]--
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"
local tut = require "common.tutorial"

local title = _("Old Friends at Protera Husk")
local target, targetsys = spob.getS( "Protera Husk" )
local main, mainsys = spob.cur()

--[[
   Mission states:
0: started
1: landed on target
--]]
mem.state = 0

local npc
function create ()
   npc = misn.npcAdd( "approach", _("C"), "unknown.webp", _("Get in touch with the voice over the speakers.") )

   -- Skip intro when starting from event directly
   hook.custom( "wildspace_start_misn")
end

function approach ()
   local accept
   vn.clear()
   vn.scene()
   local c = vn.newCharacter( vni.soundonly( _("character","C") ) )
   vn.transition()

   c(fmt.f(_([["Will you do me a favour and pick up the important items that my roommate Ben left behind on {spb}?"]]),{spb=target}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline for now"), "decline"},
   }

   vn.label("decline")
   vn.na(_([[You decline for now to get more time to prepare.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accept = true end )
   c(_([[They cough.
"Here, take the coordinates. Be careful."]]))
   vn.run()

   if not accept then return end

   wildspace_start_misn()
end

function wildspace_start_misn ()
   misn.accept()
   misn.npcRm( npc )

   misn.markerAdd( target )
   misn.osdCreate( title, {
      fmt.f(_([[Land on {spb} ({sys} system).]]), {spb=target, sys=targetsys}),
      fmt.f(_([[Return to {spb} ({sys} system).]]), {spb=main, sys=mainsys}),
   })
   mem.state = 0

   var.push("protera_husk_canland", true)
   hook.land( "land" )
   hook.land( "enter" )
end

function land ()
   if mem.state==0 and spob.cur() == target then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[You try to minimize your ship visibility during landing, but it's likely that it won't be long until you get unwanted local attention. You will have to be swift. Just in case, you double-check your hand weapons.]]))
      vn.na(_([[Without the niceties of a proper landing area, you try to find a clearing, but at the end, have to make do with precariously setting your ship on the rubble of a once majestic building. A bit more visible than what you were hoping for. Checking with the coordinates you were given, it doesn't seem like you are too far. Now to make haste.]]))
      vn.na(_([[Staying low you make your way across the rubble. No sign of life for now other some fungus that grows on the cracks and crevices throughout the decrepit structures. You notice some sort of tattered flag nearby, it seems like that is your destination.]]))
      vn.na(_([[]]))

      vn.run()

      -- Advance mission
      mem.state = 1
   elseif mem.state==1 and spob.cur() == main then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[After your ordeal, you once again make your way through the airlock. It is oddly silent. You arrive the cantina, but hear no welcome. You try to make some noise to catch the attention of the inhabitant but to no avail. Not really knowing what you do, you decide to take a look and delve deeper into the structure.]]))
      vn.na(_([[You find a door that seems to have been remotely unlocked and go through a winding hallway that twists and turns following the damage to the hull. You reach another door, this time it looks more ad-hoc and less reinforced. Out of courtesy, you knock with your weapon to see if anyone responds.]]))
      vn.na(_([[You seemingly have no choice but to kick it down to see what is on the other side. You brace for impact, but the door easily gives under the force of your boot. However, the seen you uncover almost makes you wish you hadn't.]]))
      vn.na(_([[You run back to your ship without looking back and lock yourself in the captain's room as you try to recover.]]))

      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition()
      sai(_([[]]))

      vn.run()

      misn.finish(true)
   end
end

function enter ()
   -- Took damage
   player.pilot():setHealth( 60, 30 )

   --pilot.add( ..., "Lost", target )
end

function abort ()
   var.pop("protera_husk_canland")
end

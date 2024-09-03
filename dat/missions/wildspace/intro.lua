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
end

function land ()
   if mem.state==0 and spob.cur() == target then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[]]))

      vn.run()

      -- Advance mission
      mem.state = 1
   elseif mem.state==1 and spob.cur() == main then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[]]))

      vn.run()

      misn.finish(true)
   end
end

function abort ()
   var.pop("protera_husk_canland")
end

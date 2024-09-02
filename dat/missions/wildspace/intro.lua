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


local npc
function create ()
   npc = misn.npcAdd( "approach", _("name"), "none.webp", _("desc") )

   -- Skip intro when starting from event directly
   hook.custom( "wildspace_start_misn")
end

function approach ()

   wildspace_start_misn()
end

function wildspace_start_misn ()
   misn.accept()

   misn.npcRm( npc )

   var.push("protera_husk_canland", true)
end

function abort ()
   var.pop("protera_husk_canland")
end

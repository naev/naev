--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Maikki's Father 1">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Minerva Station</planet>
   <cond>player.evtDone("Minerva Station Altercation 1")</cond>
  </avail>
 </mission>
--]]

--[[
-- Maikki (Maisie McPherson) asks you to find her father, the famous pilot Kex
-- McPherson. She heard rumours he was still alive and at Minerva station.
--
-- 1. Player is sent to Doeston to try to find the whereabouts.
-- 2. Guys in the bar talk about how he never came back from the Nebula,
-- 3. Player sent to explore Arandon, finds some scavengers, they run away.
-- 4. Player goes back to bar, comment how some have been trying to sell some ship scrap, apparently from Zerantix. Player told to follow scavengers.
-- 5. Scavengers show up and talk about sensors not working well, talk through broadcast. Player has to follow without getting too close (maybe needs outfits to improve sensor range in nebula?)
-- 6. They lead the player to some debris and start looting. Player gets to talk to them and convince to leave, or kill them. Learns that Za'lek have been buying the goods.
-- 7. Player finds a picture among the debris.
-- 8. Return to Maikki
--]]
local vn = require 'vn'

npc_name = _("Distraught Young Woman")
npc_portrait = "none"
npc_description = _("yeah")

misn_title = _("Finding Father")
misn_reward = _("???")
misn_desc = _("Maikki wants you to help her find her father.")

function create ()
   misn.setNPC( npc_name, npc_portrait )
   misn.setDesc( npc_description )
end


function accept ()
   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {} )
   vn.fadein()
   m(_("foo"))
   vn.fadeout()
   vn.run()

   if true then
      misn.finish(false)
   end

   misn.accept()
   hook.land( "land" )
end


function land ()
   if planet.cur() == planet.get("Cerberus") then
   end
end

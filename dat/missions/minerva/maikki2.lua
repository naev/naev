--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Maikki's Father 2">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Minerva Station</planet>
   <done>Maikki's Father 1</done>
  </avail>
 </mission>
--]]

--[[
-- Maikki (Maisie McPherson) asks you to find her father, the famous pilot Kex
-- McPherson. She heard rumours he was still alive and at Minerva station.
-- Player found out that Za'lek doing stuff with the ship cargo.
--
-- 1. Told to try to find who could have been involved and given three places to look at.
-- Hint 1: Jorlas in Regas (University)
-- Hint 2: Cantina Station in Qulam (Trade Hub)
-- Hint 3: Jurai in Hideyoshi's Star (University)
-- 2. After talking to all three, the player is told to go to Naga in Damien (Underwater University)
-- 3. Mentions eccentric colleague at Westhaven
-- 4. Eccentric colleague makes the player mine some stupid stuff for him.
-- 5. Tells you he saved him and sent him off to "Minerva Station".
-- 6. Go back and report to Maikki confirming her info.
--
-- Eccentric Scientist in "Westhaven" (slightly senile with dementia).
--]]
local vn = require 'vn'

npc_name = _("Maikki")
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
   m( "Yeah" )
   vn.fadeout()
   vn.run()

   misn.accept()
end




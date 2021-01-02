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
local portrait = require 'portrait'

maikki_name = _("Distraught Young Woman")
maikki_portrait = "maikki"
maikki_description = _("yeah")
maikki_image = "maikki.png"
maikki_colour = { 1, 0.73, 0.97 }

oldman_name = _("Old Man")
oldman_portrait = "old_man"
oldman_description = _("Old man.")
oldman_image = "old_man.png"

misn_title = _("Finding Father")
misn_reward = _("???")
misn_desc = _("Maikki wants you to help her find her father.")

searchsys = "Doeston"
-- Mission states:
--    0: going to doeston
--    1: talked to old man, going to arandon
--    2: saw scavengers, go back to old man
--    3: talked to old man again, going to zerantix
--    4: looted ship
misn_state = nil


function create ()
   misn.setNPC( maikki_name, maikki_portrait )
   misn.setDesc( maikki_description )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   if not misn.claim( {system.get("Arandon"), system.get("Zerantix")} ) then
      misn.finish( false )
   end

   approach_maikki()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   -- Re-add Maikki if accepted
   land()
end


function land ()
   if planet.cur() == planet.get("Cerberus") then
      npc_oldman = misn.npcAdd( "approach_oldman", oldman_name, oldman_portrait, oldman_desc )
   elseif planet.cur() == planet.get("Minerva Station") then
      npc_maikki = misn.npcAdd( "approach_maikki", maikki_name, maikki_portrait, maikki_description )
   end
end


function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( maikki_name,
         { image=maikki_image, color=maikki_colour } )
   vn.fadein()

   if misn_state==nil then
      -- Start mission
      maikki( _([["Blah"]]) )

      vn.menu( {
         { _("Help Maikki find her father"), "accept" },
         { _("Decline to help"), "decline" },
      } )

      vn.label( "decline" )
      maikki(_([["decline msg"]]))
      vn.done()

      vn.label( "accept" )
      maikki(_([["accept msg"]]))
      vn.func( function ()
         -- Accepted
         misn_state=0
         misn.accept()
         misn_osd = misn.osdCreate( misn_title,
            { string.format(_("Look around the %s system"), searchsys) } )
         hook.land( "land" )
         hook.enter( "enter" )
      end )
   elseif misn_state==4 then
      -- Finish mission
      maikki(_([["finish text"]]))
      vn.done()
   end

   -- Normal chitchat
   local opts = {
      {_("Ask about her father"), "father"},
      {_("Leave"), "leave"},
   }
   vn.label( "menu" )
   vn.menu( opts )

   vn.label( "father" )
   maikki( _("Blah") )
   vn.jump( "menu_msg" )

   vn.label( "menu_msg" )
   maikki( _([["Is there anything you would like to know about?"]]) )
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na( "You take your leave." )
   vn.fadeout()
   vn.run()
end


function approach_oldman ()
   vn.clear()
   vn.scene()
   local om = vn.newCharacter( oldman_name,
         { image=oldman_image } )
   vn.fadein()
   vn.na( _("You see an old man casually drinking at the bar. He has a sort of self-complacent bored look on his face.") )

   vn.label( "menu" )
   local opts = {
      {_("Ask about Doeston"), "doeston"},
      {_("Ask about the Nebula"), "nebula"},
      {_("Leave"), "leave"},
   }
   if misn_state==2 then
      table.insert( opts, 1, {_("Ask about scavengers you saw"), "scavengers"} )
   end
   vn.menu( opts )

   vn.label( "doeston" )
   om(_([[""]]))
   vn.jump( "menu_msg" )

   vn.label( "nebula" )
   om(_([[""]]))
   vn.jump( "menu_msg" )

   vn.label( "menu_msg" )
   om( _([[He gives you a bored look.\n"Is there anything else you would like to know about?"]]) )
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.fadeout()
   vn.run()
end


function enter ()
   if system.cur() == system.get("Arandon") and misn_state==1 then
      -- Cutscene with scavengers
   elseif system.cur() == system.get("Zerantix") and misn_state==3 then
      -- Have to follow scavengers
   end
end

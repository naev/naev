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
local minerva = require "minerva"
local vn = require 'vn'

maikki_name = _("Distraught Young Woman")
maikki_description = _("yeah")
maikki_portrait = minerva.maikki.portrait
maikki_image = minerva.maikki.image
maikki_colour = minerva.maikki.colour

oldman_name = _("Old Man")
oldman_portrait = "old_man"
oldman_description = _("Old man.")
oldman_image = "old_man.png"

scav_name = _("Scavengers")
scav_portrait = "scavenger1"
scav_desc = _("You see a pair of dirty looking fellows talking loudly among themselves.")
scavengera_image = "scavenger1.png"
scavengerb_image = scavengera_image
scavengera_colour = nil
scavengerb_colour = nil

misn_title = _("Finding Father")
misn_reward = _("???")
misn_desc = _("Maikki wants you to help her find her father.")

mainsys = "Limbo"
searchsys = "Doeston"
cutscenesys = "Arandon"
stealthsys = "Zerantix"
-- Mission states:
--  nil: mission not accepted yet
--    0: going to doeston
--    1: talked to old man, going to arandon
--    2: saw scavengers, go back to doeston
--    3: talked to old man again
--    4: talk to scavengers, going to zerantix
--    5: looted ship
misn_state = nil


function create ()
   misn.setNPC( maikki_name, maikki_portrait )
   misn.setDesc( maikki_description )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   if not misn.claim( {system.get(cutscenesys), system.get(stealthsys)} ) then
      misn.finish( false )
   end

   approach_maikki()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   -- Formally accept the mission and set up stuff
   misn.accept()
   misn_osd = misn.osdCreate( misn_title,
      { string.format(_("Look around the %s system"), searchsys) } )
   misn_marker = misn.markerAdd( system.get(searchsys), "low" )
   hook.land( "land" )
   hook.enter( "enter" )

   -- Re-add Maikki if accepted
   land()
end


function land ()
   if planet.cur() == planet.get("Cerberus") then
      npc_oldman = misn.npcAdd( "approach_oldman", oldman_name, oldman_portrait, oldman_desc )
      if misn_state==3 or misn_state==4 then
         npc_scavenger = misn.npcAdd( "approach_scavengers", scav_name, scav_portrait, scav_desc )
      end
   elseif planet.cur() == planet.get("Minerva Station") then
      npc_maikki = misn.npcAdd( "approach_maikki", minerva.maikki.name, minerva.maikki.portrait, minerva.maikki.description )
   end
end


function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
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
      vn.func( function () misn_state=0 end )
   elseif misn_state==5 then
      -- Finish mission
      maikki(_([["finish text"]]))
      vn.done()
   end

   -- Normal chitchat
   local opts = {
      {_("Ask about her father"), "father"},
      {_("Leave"), "leave"},
   }
   if misn_state >=4 then
      table.insert( opts, 1, {_("Show her the XXX"), "showloot"} )
   end
   vn.label( "menu" )
   vn.menu( opts )

   vn.label( "father" )
   maikki(_([["Blah"]]))
   vn.jump( "menu_msg" )

   vn.label( "showloot" )
   maikki(_([["Blah"]]))
   vn.func( function ()
      -- TODO give reward
      misn.finish(true)
   end )
   vn.done()

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
      {_("Ask about Kex McPherson"), "kex" },
      {_("Ask about Doeston"), "doeston"},
      {_("Ask about the Nebula"), "nebula"},
      {_("Leave"), "leave"},
   }
   if misn_state>=2 then
      table.insert( opts, 1, {_("Ask about scavengers you saw"), "scavengers"} )
   end
   if misn_state >=4 then
      table.insert( opts, 1, {string.format(_("Ask about %s"),stealthsys), "stealthmisn"} )
   end
   if misn_state >=5 then
      table.insert( opts, 1, {_("Show him the XXX"), "showloot"} )
   end
   vn.menu( opts )

   vn.label( "kex" )
   om(_([["Kex? Now that is not a name I've heard in a while."\nHe nods reflectively.]]))
   om(_([["Kex was a great guy. He used to hang out here before venturing into the nebula, spending his time with the useless lot of us. Shame that he went missing."]]))
   om(_([["Since they never found his ship, I like to think that he made it to the other side of the nebula, if there is one.\nHe takes a long swig from his drink."]]))
   om(_([["Still that doesn't stop the odd folk here and there from trying to find it, they usually don't end up much past Arandon."]]))
   vn.func( function ()
      if misn_state==0 then
         misn.markerMove( misn_marker, system.get(cutscenesys) )
         misn_state=1
      end
   end )
   vn.jump( "menu_msg" )

   vn.label( "doeston" )
   om(_([["Not much to do in Doeston. Mainly a stopping place for all them crazy folk heading into the nebula. Maybe if I were any younger I would be with them exploring, but can't with this bad knee."\nHe pats his left knee.]]))
   om(_([["It used to be a more popular place, but with most of the easy pickings getting scavenged out of the nebula, not many people come here after all. Especially not after the disappearance of famous scavengers like Kex and Xioni."\nHe muses thoughtfully.]]))
   om(_([["To better times."\nHe downs his drink and orders another.]]))
   vn.jump( "menu_msg" )

   vn.label( "nebula" )
   om(_([[He whistles casually.\n"The nebula's a real piece of work. It's almost mesmerizing to fly through it, however, it do got quite a character. If you try to go too deep into 'er you can't easily get back. Many a soul has been lost in there."]]))
   -- TODO play eerie sound
   om(_([[He goes a bit quieter and gets closer to you.\n"Rumour has it that there are ghosts lurking in the depths. I've seen people come back, pale as snow, "claiming they seen them."]]))
   om(_([["I believe it be the boredom getting to their heads. Likely naught but a scavenger or some debris."]]))
   vn.jump( "menu_msg" )

   vn.label( "scavengers" )
   om(_([[""]]))
   vn.func( function ()
      if misn_state==2 then
         misn_state=3
      end
   end )
   vn.jump( "menu_msg" )

   vn.label( "stealthmisn" )
   om(_([[""]]))
   vn.jump( "menu_msg" )

   vn.label( "showloot" )
   om(_([[""]]))
   vn.jump( "menu_msg" )

   vn.label( "menu_msg" )
   om( _([[He gives you a bored look as he takes a sip from his drink.\n"Is there anything else you would like to know about?"]]) )
   vn.jump( "menu" )

   vn.label( "leave" )
   vn.na(_("You take your leave."))
   vn.fadeout()
   vn.run()
end


function approach_scavengers ()
   vn.clear()
   vn.scene()
   local scavA = vn.newCharacter( _("Scavenger A"),
         { image=scavengera_image, color=scavengera_colour } )
   local scavB = vn.newCharacter( _("Scavenger B"),
         { image=scavengerb_image, color=scavengerb_colour } )
   vn.fadein()
   vn.na(_("You see some scavengers at the bar. They are clearly plastered. They don't really seem to be aware of your presence."))

   if misn_state==4 then
      -- Already got mission, just give player a refresher
      scavB(_([["Aren't you drinking too much. Don't forget to fix my sensors before we leave to Zerantix tomorrow."]]))
      scavA(_([["Meee? Drinkinging tooo mich? Thatss sshtoopid."\nHe takes another long swig of his drink and burps.]]))
   else
      -- Blabber target to player
      scavA(_([["...and then I said to him 'while that may look like a hamster, it's got a bite like a moose!'"]]))
      va.na(_("The scavengers hoot with laughter."))
      scavB(_([["That's a great story. Them space hamsters be wicked."]]))
      scavA(_([["About tomorrow, you sure the info is correct? Going that deep into the nebula always gives me the chills."\nHe shivers exaggeratedly.]]))
      scavB(_([["Yeah! I saw it with my own eyes. That shit is legit. Seems to be the wreck of a scavenger."]]))
      scavA(_([["Why didn't you haul it back then? We might not be able to find it again!"]]))
      -- TODO play bingo sound
      scavB(_([["How many times do I have to tell you? My sensors were acting up, didn't want to spend too long in Zerantix with all them ghosts around."]]))
      scavA(_([["I'll take a look at fixing your sensors. Ain't nothing these brutes can't fix!"\nHe pats his biceps in a fairly uninspiring way.]]))
      scavB(_([["Yeah, let's do that after one more round of drinks. Got to get there before other scroungers get there."]]))
      scavA(_([["To our future success in Zerantix!"]]))
      vn.na(_("They cheer, down their drinks, and order another round. Perhaps the wreck in Zerantix is related to Kex somehow."))
      vn.func( function ()
         if misn_state==3 then
            misn_osd = misn.osdCreate( misn_title,
               { string.format(_("Follow the scavengers in the %s system"), stealthsys) } )
            misn.markerMove( misn_marker, system.get(stealthsys) )
            misn_state=4
         end
      end )
   end

   vn.na(_("You take your leave without them noticing you."))
   vn.fadeout()
   vn.run()
end


function enter ()
   if system.cur() == system.get(cutscenesys) and misn_state==1 then
      -- Cutscene with scavengers
      --misn.markerMove( misn_marker, system.get(searchsys) )
      --misn_state=2
   elseif system.cur() == system.get(stealthsys) and misn_state==3 then
      -- Have to follow scavengers
      --misn_osd = misn.osdCreate( misn_title,
      --    { string.format(_("Return to %s in the %s system"), minerva.maikki.name, mainsys) } )
      --misn.markerMove( misn_marker, system.get(mainsys) )
      --misn_state=4
   end
end

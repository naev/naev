--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 4">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Minerva Station</planet>
  <done>Minerva Pirates 3</done>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
-- Torture the Dvaered Spy
--]]
local minerva = require "campaigns.minerva"
local portrait = require 'portrait'
local vn = require 'vn'
local love_shaders = require "love_shaders"
require 'numstring'

logidstr = minerva.log.pirate.idstr

misn_title = _("The Dvaered Spy")
misn_reward = _("Cold hard credits")
misn_desc = _("Someone wants you to deal with a Dvaered spy that appears to be located at Minerva Station.")
reward_amount = 200e3 -- 200k

-- Should be the same as the original chuckaluck guy
mole_image = "minervaceo.png" -- TODO replace

mainsys = "Provectus Nova"
-- Mission states:
--  nil: mission not accepted yet
--    0. have to find spy
--    1. kidnap spy and take to torture ship
--    2. defend torture ship
misn_state = nil

function create ()
   if not var.peek("testing") then misn.finish(false) end
   if not misn.claim( system.get(mainsys) ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   misn.setDesc( misn_desc )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end

function accept ()
   approach_pir()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end

   misn.accept()
   osd = misn.osdCreate( _("Minerva Mole"), {
      _("Find out who the mole is"),
      _("Take the mole to the interrogation facility")
   } )

   shiplog.appendLog( logidstr, _("You accepted another job from the shady individual deal with a mole at Minerva Station.") )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.custom("minerva_secretcode", "found_mole")
   var.push("minerva_caninputcode",true)
   generate_npc()
end

function generate_npc ()
   npc_pir = nil
   if planet.cur() == planet.get("Minerva Station") and misn_state < 1 then
      npc_pir = misn.npcAdd( "approach_pir", minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   end
end

function approach_pir ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if misn_state==nil then
      -- Not accepted
      vn.na(_("You approach the sketch individual who seems to be somewhat excited."))
      pir(_([["It seems like we have finally started to get the fruits of our labour. It seems like we have found the mole, and we would like you to help us deal with them. Are you up to the task? Things might get a little… messy though."
They beam you a smile.]]))
      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () misn_state=0 end )
      pir(_([["Glad to have you onboard again! So we have tracked down the mole and know that they are infiltrated in the station from the intercepted messages. It appears that they are most likely working at the station. Now there are not many places to work at the station so it is likely that they are involved in the gambling facility."]]))
      pir(_([["The bad news is we don't exactly know who the moles is. However, the good news is we were able to intercept a messenger. It was after a delivery so we weren't able to capture anything very interesting. But there was a small memo we found that could be a hint."
They should you a crumpled up dirty piece of paper that has '10K 5-6-3-1' on it and hands it to you.]]))
      vn.func( function ()
         local c = misn.cargoNew( _("Crumpled Up Note"), _("This is a crumpled up note that says '10K 5-6-3-1' on it. How could this be related to the Dvaered spy on Minerva Station?") )
         misn.cargoAdd( c, 0 )
      end )
      pir(_([["We're still trying to figure exactly who they are, but that note is our best hint. Maybe it can be of use to you when looking for them. Once we get them we'll kindly escort them to an interrogation ship we have and we can try to get them to spill the beans."]]))
   else
      -- Accepted.
      vn.na(_("You approach the shady character you have become familiarized with."))
   end

   vn.label("menu_msg")
   pir(_([["Is there anything you would like to know?"]]))
   vn.menu{
      {_("Ask about the job"), "job"},
      -- TODO add some other more info text
      {_("Leave"), "leave"},
   }

   vn.label("job")
   pir(_([["How is the search going? We haven't been able to find any new leads on the mole. Our best bet is still the note I gave you that says '10K 5-6-3-1' on it. Maybe it's some sort of code for something?"]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end


function found_mole ()
   vn.clear()
   vn.scene()
   local mole = vn.newCharacter( _("Mole"), {image=mole_image} )
   vn.transition()
   vn.na(_("After the chuck-a-luck dealers shift you follow him to a back alley in the station."))
   mole(_([["I don't recognize you, are you the new messenger? Last guy got sliced up."
They make a cutting gesture from their belly up to their neck.
"Poor kid, not the best way to leave this world."]]))
   mole(_([["Hey, wait a moment. Haven't I seen you around?"]]))
   vn.na(_("While they wrinkle their eyebrows, you suddenly hear a soft thud and while you hear the soud of strong electric current, you see their eyes glaze over and muscles stiffen. They quickly drop to the ground and a familiar face appears into view."))

   vn.scene()
   vn.transition()
   local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
   pir(string.format(_([["Great job! It seems like you found our mole. However, our job is not done here. We have to get all the information we can out of him. This is not something we can do here at Minerva Station. Here, take him on your ship and bring him to the %s system. There should be a ship waiting for him in the middle of the asteroid field near Pund, it might be a bit hard to spot at first."]]), mainsys))
   pir(_([["I'll be waiting on the ship with my crew. Make sure to bring him overe, however, watch out for any Dvaered patrols. We don't want them to know we have taken him."]]))
   vn.run()

   -- Add illegal cargo
   local c = misn.cargoNew( _("Dvaered Mole"), _("An unconcious and restrained Dvaered mole. You better not let Dvaered ships find out you are carrying this individual.") )
   c:illegalto{"Dvaered"}
   misn.cargoAdd( c, 0 )

   -- Signal they were caught
   var.pop("minerva_caninputcode")
   hook.trigger( "minerva_molecaught" )

   -- On to next state
   misn_state = 1
   osd = misn.osdCreate( _("Minerva Mole"), {
      string.format(_("Take the mole to the interrogation facility at %s"), mainsys),
   } )
   misn.markerAdd( system.get(mainsys) )
   misn.npcRm( npc_pir )
end


function enter ()
   if misn_state==1 and system.cur()==system.get(mainsys) then
      mainship = pilot.add( "Pirate Rhino", f, vec2.new( 5000, 9000 ) )
      mainship:rename(_("Interrogation Ship"))
      mainship:setFriendly(true)
      mainship:setVisplayer(true)
      mainship:setHilight(true)
      mainship:control(true)
      mainship:stealth()
   end
end

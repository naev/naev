--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Colonel Escort">
 <unique />
 <priority>4</priority>
 <chance>86</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
</mission>
--]]
--[[

   Mission: Escort a Dvaered colonel
   Description: Small mission where you escort a Dvaered Arsenal.
                This is a one-off that's not part of any major
                storyline; the in-game purpose will remain a mystery.

--]]

local escort = require "escort"
local fmt = require "format"
local pir = require "pirate"
local vn = require "vn"
local vni = require "vnimage"

local dest_planet = spob.getS("Adham")


local npc_name = _("Dvaered Colonel")
local npc_portrait, npc_image
function create()
   mem.npc_image, mem.npc_portrait = vni.dvaeredMilitary()
   misn.setNPC(npc_name, npc_portrait, _("A Dvaered, very professional-looking, is sitting with an excellant posture at the bar.") )
end

function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name {image=npc_image} )
   vn.transition()
   m(fmt.f([[As you approach, the Dvaered soldier stands. "Hello, captain {playername}!" they say. "Nice to see you around here. How's it going?"]]))
     {playername=player.name()}
   vn.menu{
      {_([["Quite well, thank you!"]]), "well"},
      {_([["I guess it's going alright."]]), "fine"},
   }

   vn.label("well")
   m(_([["Wonderful! Glad you're having a good time."]]))
   vn.jump("mission description")

   vn.label("fine")
   m(_([["As long as there's no trouble..."]]))
   vn.jump("mission description")

   vn.label("mission description")
   m(fmt.f([["Well, to the point. I need somebody to escort me and my Arsenal to {pnt}. Would you be willing to do that? I can't tell you why."]]))
     {pnt=mem.dest_planet}
   vn.menu{
      {_([["Remind me what system that's in?"]]), "what system"},
      {_([["I'd be happy to do that!"]]), "sure"},
      {_([["What is your name?"]]), "what is your name"},
   }

   vn.label("what system")
   m(fmt.f([["Umm..." the Dvaered says. They consult a watch. "Alright, {pnt} is in {sys}."]]))
     {pnt=mem.dest_planet, sys=mem.dest_sys}
   vn.jump("choice")

   vn.label("what is your name")
   m(_([[The Dvaered seems slightly taken aback. "Well... that may be classified information... call me Radver."]]))
   vn.jump("choice")

   vn.label("sure")
   m(_([["Wonderful! I'll be on your ship when you leave."]]))
   vn.func( function ()
      accepted = true
   end )

   vn.run()

   vn.label("choice")
   m(_([["Well? Will you do this?"]]))
   vn.menu{
      {_([["Yep, I'd be glad to!"]]), "sure"},
      {_([["Not going to happen, sorry."]]), "never"},
   }

   vn.label("never")
   m(_([["That's sad. Oh well, I'll ask someone else." The Dvaered leaves.]]))
   vn.done()

   if not accepted then return end

   misn.accept()
   
   misn.setReward(750000)
   misn.setDesc("Escort a Dvaered colonel, who is flying an Arsenal, to {pnt} in the {sys} system. You haven't been told why, but there may be a large payment."), {pnt=mem.dest_planet, sys=mem.dest_sys},
   misn.osdCreate(_("Dvaered colonel escort"), {
      fmt.f(_("Escort a Dvaered colonel to {pnt} in the {sys} system.")), {pnt=mem.dest_planet, sys=mem.dest_sys}),
   }
   misn.markerAdd( mem.destspob )
   hook.land( "land" )
   hook.enter( "pirate_ambush" )
   local colonel_ship = ship.get("Dvaered Arsenal")
   escort.init ( colonel_ship, {
   })
end

function pirate_ambush ()
   local ambush
   local ambushes = {
      {"Pirate Ancestor", "Pirate Phalanx"}
      {"Pirate Rhino", "Pirate Vendetta"}
   }

   ambush = fleet.add ( 1, ambushes[rnd.rnd(1,2)], "Marauder", {ai="baddie_norun"} )
end
   
function land ()
   if spob.cur() == misplanet then
      vntk.msg( fmt.f(_([[As you land on {pnt} with the Arsenal close behind, you receive an intercom message. "Thank you for bringing me here!" says the colonel. "Here is {reward}, as we agreed. Have safe travels!"]]), {pnt=misplanet, reward=reward_text}) )
      player.pay( credits )
      neu.addMiscLog( fmt.f(_([[You escorted a Dvaered colonel who was flying an Arsenal to {pnt}. For some reason, they didn't tell you why.]]), {pnt=misplanet} ) )
      misn.finish( true )
   end
end

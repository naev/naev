--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Colonel Escort">
 <unique />
 <priority>4</priority>
 <chance>43</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <cond>require("misn_test").reweight_active()</cond>
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
local neu = require "neutral"
local vn = require "vn"
local vni = require "vnimage"

local reward = 0

local source_system = nil

lmisn.getSpobAtDistance(2, 5, fct=faction.get("Dvaered")[, samefct=true[, filter=nil[, data=nil[, hidden=false]]]])

local ffriendly, fhostile -- codespell:ignore ffriendly

local npc_name = _("Dvaered Colonel")
local npc_portrait = nil
local npc_image = nil
function create()
   mem.npc_image, mem.npc_portrait = vni.dvaeredMilitary()
   misn.setNPC(npc_name, npc_portrait, _("A Dvaered, very professional-looking, is sitting with an excellant posture at the bar.") )
end

function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   m(fmt.f([[As you approach, the Dvaered soldier stands. "Oh, is it Captain {playername}?" they say. "Nice to see you around here. How's it going?"]]))
     {playername=player.name()}
   vn.menu{
      {_([["Quite well, thank you!"]]), "well"},
      {_([["I guess it's going alright."]]), "fine"},
      {_([["Wait a minute. How did you know my name?"]]), "my name"},
   }

   vn.label("well")
   m(_([["Alright."]]))
   vn.jump("mission description")

   vn.label("fine")
   m(_([["As long as there's no trouble..."]]))
   vn.jump("mission description")

   vn.label("my name")
   m(_([["Uhh... tough luck, captain, I'm not allowed to tell you. Too bad.]]))
   m(_([["Let's just say that my espionage services are of the best, and leave it at that... shall we?"]])) -- When/if possible, the word "shall" should be italicized.
   vn.jump("mission description")

   vn.label("mission description")
   m(fmt.f([["Well, to the point. I need somebody who's not crazy to escort me and my Arsenal to {pnt} in the {sys} system. Would you do that? I can't tell you why, that would be classified information leaked to an outsider. Very dangerous, especially when we can't trust you.]]))
     {pnt=mem.dest_planet, sys=mem.dest_sys}
   m(_([["One more thing: another warlord would be only to happy to blow any of the colonel rank such as myself up, so you may need to expect attacks. The ship will probably be called 'Asheron Anomaly', but it may be something else."]]))
   vn.menu{
      {_([["I'd be happy to do that!"]]), "sure"},
      {_([["What is your name?"]]), "what is your name"},
      {_([["Wait, why is there someone trying to kill you?"]]), "why would you die"},
   }

   vn.label("what is your name")
   m(_([[The Dvaered seems slightly taken aback. "Well... that may also be classified information... call me Radver."]]))
   vn.jump("choice")

   vn.label("sure")
   m(_([["Surprising but good! I'll be on your ship in a while, so don't leave too soon."]]))
   vn.func( function ()
      accepted = true
   end )

   vn.label("why would you die")
   m(_([[The Dvaered colonel looks uncomfortable. "That too is classified information. Let's just say that I've got a vendetta (not the ship, the relation) with this other warlord, and we're on blowing-each-other-up terms, and you can't ask me to reveal more information."]]))

   vn.label("choice")
   m(_([["Well? Will you do this for the sake of all true Dvaered, or else..?"]]))
   vn.menu{
      {_([["Yep, I'd be glad to!"]]), "sure"},
      {_([["Not going to happen, sorry."]]), "never"},
   }

   vn.label("never")
   m(_([["Quite an impolite answer," says the Dvaered. "Well, I suppose no choice is offered to me, so I'll stay here, but don't take too long in coming back."]]))
   vn.done()

   vn.run()

   if not accepted then return end

   misn.accept()

   misn.setReward(750000)
   misn.setDesc(fmt.f(_("Escort a Dvaered colonel, who is flying an Arsenal, to {pnt} in the {sys} system. You haven't been told why, but there may be a large payment."), {pnt=mem.dest_planet, sys=mem.dest_sys}))
   misn.osdCreate(_("Dvaered colonel escort"), {
      fmt.f(_("Escort a Dvaered colonel to {pnt} in the {sys} system.")), {pnt=mem.dest_planet, sys=mem.dest_sys},
   })
   misn.markerAdd( mem.dest_planet )
   hook.land( "land" )
   local colonel_ship = ship.get("Dvaered Arsenal")
   escort.init( colonel_ship, {
      func_pilot_create = function ( p )
        local fct = ffriendly()
        p:rename("Radver")
        local ffriendly = factions()
        p:setFaction( ffriendly() )
   end } )

   hook.enter( "ambush" )
   hook.enter( "factions" )
end

function ambush ()
   if not naev.claimTest( system.cur(), true ) then return end

   rm.hook( "ambush" )
   rm.hook( "factions" )
   
   local dvaered_factions = faction.get("Dvaered")

   pilot.add( "Dvaered Phalanx", "fhostile", source_system, _("Asheron Anomaly") )
end

local function factions ()
   local ffriendly = faction.dynAdd( dvaered_factions, "radver", _("Dvaered") ) -- codespell:ignore ffriendly
   local fhostile = faction.dynAdd( dvaered_factions, "radver_baddie", _("Dvaered Warlord") )
   faction.dynEnemy( ffriendly, fhostile ) -- codespell:ignore ffriendly
   return ffriendly, fhostile
end

function land ()
   if spob.cur() == dest_planet then
      vn.clear()
      vn.scene()
      vn.transition()
      m(fmt.f(_([[As you land on {pnt} with the Arsenal close behind, you receive an intercom message. "Good job bringing me here!" says the colonel. "Here is {reward}, as we agreed."]]), {pnt=dest_planet, reward=reward}) )
      vn.func( function () player.pay(reward) end )
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      neu.addMiscLog( fmt.f(_([[You escorted a Dvaered colonel who was flying an Arsenal to {pnt}. This colonel, who said to call them Radver, was very polite to you, though they didn't tell you why they needed the escort.]]), {pnt=dest_planet} ) )
      misn.finish( true )
   end
end

--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Colonel Escort">
 <unique />
 <chance>18</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <cond>
   if spob.cur():reputation("Dvaered") &lt; 0 then
      return false
   end
   if player.wealth() &lt; 1e6 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
</mission>
--]]
--[[

   Mission: Escort a Dvaered colonel
   Description: Small mission where you escort a Dvaered Ancestor.
   This is a one-off that's not part of any major
   storyline; the in-game purpose will remain a mystery.

--]]

local escort = require "escort"
local fmt = require "format"
local lmisn = require "lmisn"
local neu = require "common.neutral"
local vn = require "vn"
local vni = require "vnimage"

local reward = 750e3

local npc_name = _("Dvaered Colonel")

function create()
   local spbs = lmisn.getSpobAtDistance( nil, 2, 6, "Dvaered" )
   if #spbs <= 0 then misn.finish(false) end
   mem.destspb = spbs[ rnd.rnd(1,#spbs) ]
   mem.dest_sys = mem.destspb:system()
   mem.npc_image, mem.npc_portrait = vni.dvaeredMilitary()
   misn.setNPC(npc_name, mem.npc_portrait, _("A Dvaered soldier, very professional-looking, is sitting with an excellent posture at the bar.") )
end

local function factions()
   local dvaered_factions = faction.get("Dvaered")

   local ffriendly = faction.dynAdd( dvaered_factions, "radver", _("Dvaered") ) -- codespell:ignore ffriendly
   local fhostile = faction.dynAdd( dvaered_factions, "radver_baddie", _("Dvaered Warlord") )
   faction.dynEnemy( ffriendly, fhostile ) -- codespell:ignore ffriendly
   return ffriendly, fhostile -- codespell:ignore ffriendly
end

function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {image=mem.npc_image} )
   vn.transition()
   m(fmt.f(_([[As you approach, the Dvaered soldier stands. "Oh, is it Captain {playername}?" they say. "Nice to see you around here. How's it going?"]]),
      {playername=player.name()}))
   vn.menu{
      {_([["Quite well, thank you!"]]), "well"},
      {_([["I guess it's going alright."]]), "fine"},
      {_([["Wait a minute. How did you know my name?"]]), "my name"},
   }

   vn.label("well")
   m(_([["Alright."]]))
   vn.jump("mission description")

   vn.label("fine")
   m(_([["As long as there's no trouble…"]]))
   vn.jump("mission description")

   vn.label("my name")
   m(_([["Uhh… tough luck, captain, I'm not allowed to tell you. Too bad.]]))
   m(_([["Let's just say that my espionage services are of the best, and leave it at that… shall we?"]])) -- When possible, the word "shall" should be italicized.
   vn.jump("mission description")

   vn.label("mission description")
   m(fmt.f(_([["Well, to the point. I need somebody who's not crazy to escort me and my Ancestor to {pnt} in the {sys} system. Would you do that? I can't tell you why, that would be classified information leaked to an outsider. Very dangerous, especially when we can't trust you.]]),
      {pnt=mem.destspb, sys=mem.dest_sys}))
   m(_([["One more thing: another warlord would be only too happy to blow any of the colonel rank such as myself up, so you may need to expect attacks. The ship will probably be called 'Asheron Anomaly', but it may be something else."]]))
   vn.menu{
      {_([["I'd be happy to do that!"]]), "sure"},
      {_([["What is your name?"]]), "what is your name"},
      {_([["Wait, why is there someone trying to kill you?"]]), "why would you die"},
      {_([["'Asheron Anomaly'?"]]), "asheron anomaly"},
   }

   vn.label("what is your name")
   m(_([[The Dvaered seems slightly taken aback. "Well… that may also be classified information… call me Radver."]]))
   vn.jump("choice")

   vn.label("sure")
   m(_([["Surprising but good! I'll be on your ship in a while, so don't leave too soon."]]))
   vn.func( function ()
      accepted = true
   end )
   vn.done()

   vn.label("why would you die")
   m(_([[The Dvaered colonel looks uncomfortable. "That too is classified information. Let's just say that I've got a vendetta (not the ship, the relation) with this other warlord, and we're on blowing-each-other-up terms, and you can't ask me to reveal more information."]]))
   vn.jump("choice")

   vn.label("asheron anomaly")
   m(_([["What about it? If you're wondering why it's named that; well, don't ask me. You'd have to ask this warlord, and I doubt they'd tell you.]]))
   vn.menu{
      {_([["Why are they trying to kill you?]]), "why would you die"},
      {_([["I get that."]]), "choice"},
   }

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

   misn.setReward(reward)
   misn.setDesc(fmt.f(_("Escort a Dvaered colonel, who is flying an Ancestor, to {pnt} in the {sys} system. You haven't been told why, but there may be a large payment."), {pnt=mem.destspb, sys=mem.dest_sys}))
   misn.osdCreate(_("Dvaered colonel escort"), {
      fmt.f(_("Escort a Dvaered colonel to {pnt} in the {sys} system."), {pnt=mem.destspb, sys=mem.dest_sys}),
   })
   misn.markerAdd( mem.destspb )

   local colonel_ship = "Dvaered Ancestor"

   escort.init( { colonel_ship }, {
      func_pilot_create = "create_radver"
   } )
   escort.setDest( mem.destspb, "success" )

   hook.jumpin ( "jumpin" )
end

function create_radver ( p ) -- luacheck: globals create_radver
   p:rename(_("Radver"))
   local ffriendly = factions() -- codespell:ignore ffriendly
   p:setFaction( ffriendly ) -- codespell:ignore ffriendly
end

local source_system = system.cur()
function jumpin ()
   if not naev.claimTest( system.cur(), true ) then
      source_system = system.cur()
      return
   end

   hook.timer( 20, "ambush" )
end

function ambush ()
   if not naev.claimTest( system.cur(), true ) then
      source_system = system.cur()
      return
   end

   if mem.ambushed then return end
   mem.ambushed = true

   local _ffriendly, fhostile = factions()
   local p = pilot.add( "Dvaered Vigilance", fhostile, source_system, _("Asheron Anomaly") )
   p:setHilight(true)
   p:setHostile(true)
end

function success () -- luacheck: globals success
   if spob.cur() == mem.destspb then
      vn.clear()
      vn.scene()
      local m = vn.newCharacter( npc_name, {image=mem.npc_image} )
      vn.transition()
      m(fmt.f(_([[As you land on {pnt} with the Ancestor close behind, you receive an intercom message. The Dvaered seems to have changed appearance. "Good job bringing me here!" says the colonel. "Here is {reward}, as we agreed."]]),
            {pnt=mem.destspb, reward=fmt.credits(reward)}) )
      vn.func( function () player.pay(reward) end )
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()
      neu.addMiscLog( fmt.f(_([[You escorted a Dvaered colonel who was flying an Ancestor to {pnt}. This colonel, who said their name could be Radver, was very polite to you, though they didn't tell you why they needed the escort. Perhaps because of a hostile warlord's Vigilance.]]),
            {pnt=mem.destspb} ) )
      misn.finish( true )
   end
end

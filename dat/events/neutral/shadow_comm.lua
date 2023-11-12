--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shadowcomm">
 <location>enter</location>
 <chance>3</chance>
 <cond>system.cur():presence("hostile") &lt; 300 and player.misnDone("Shadowrun") and not (player.misnDone("Shadow Vigil") or player.misnActive("Shadow Vigil")) and not (system.cur() == system.get("Pas")) and system.cur():presence("Independent") &gt; 100</cond>
 <notes>
  <done_misn name="Shadowrun"/>
  <campaign>Shadow</campaign>
  <tier>3</tier>
  <priority>9</priority><!-- Since it tests a claim we have to run later. -->
 </notes>
</event>
--]]
--[[
   Comm Event for the Shadow missions
--]]
require "proximity"
local fmt = require "format"
local shadow = require "common.shadow"
local vn = require "vn"
local ccomm = require "common.comm"

local vendetta, hailhook -- Non-persistent state.

function create ()
   -- Make sure system isn't claimed, but we don't claim it
   if not naev.claimTest( system.cur() ) then evt.finish() end

    -- Claim: test the claims in the mission.
   local misssys = {system.get("Qex"), system.get("Shakar"), system.get("Borla"), system.get("Doranthex")}
   if not naev.claimTest( misssys ) then
      evt.finish()
   end

   -- Create a Vendetta who hails the player after a bit
   vendetta = pilot.add( "Vendetta", shadow.fct_fourwinds(), nil, _("Four Winds Vendetta"), {ai="trader"} )
   vendetta:control()
   vendetta:follow(player.pilot())
   hook.timer(0.5, "proximityScan", {focus = vendetta, funcname = "hailme"})

   -- Clean up on events that remove the Vendetta from the game
   hook.pilot(vendetta, "jump", "finish")
   hook.pilot(vendetta, "death", "finish")
   hook.land("finish")
   hook.jumpout("finish")
end

-- Make the ship hail the player
function hailme()
   vendetta:hailPlayer()
   hailhook = hook.pilot(vendetta, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail( p )
   local accepted = false
   hook.rm(hailhook)

   local sys = system.get("Pas")

   vn.clear()
   vn.scene()
   local plt = ccomm.newCharacter( vn, p )
   vn.transition()
   plt(fmt.f(_([["Greetings, {player}," the pilot of the Vendetta says to you as soon as you answer his hail. "I have been looking for you on behalf of an acquaintance of yours. She wishes to meet with you at a place of her choosing, and a time of yours. It involves a proposition that you might find interesting - if you don't mind sticking your neck out."]]),
      {player=player.name()}))
   vn.na(_([[You frown at that, but you ask the pilot where this acquaintance wishes you to go anyway.]]))
   plt(fmt.f(_([["Fly to the {sys} system," he replies. "She will meet you there. There's no rush, but I suggest you go see her at the earliest opportunity."]]),
      {sys=sys}))
   vn.na(fmt.f(_([[The screen blinks out and the Vendetta goes about its business, paying you no more attention. It seems there's someone out there who wants to see you, and there's only one way to find out what about. Perhaps you should make a note of the place you're supposed to meet her: the {sys} system.]]),
      {sys=sys}))
   vn.na(_([[Do you intend to respond to the invitation?]]))
   vn.menu{
      {_([[Yes.]]), "yes"},
      {_([[No.]]), "no"},
   }

   vn.label("yes")
   vn.func( function () accepted = true end )
   vn.na(_([[You decide to follow the offer.]]))
   vn.done()

   vn.label("no")
   vn.na(_([[You decide not to pursue the offer for now.]]))
   vn.run()

   player.commClose()
   vendetta:control()
   vendetta:hyperspace()

   if accepted then
      shadow.addLog( fmt.f(_([[Someone has invited you to meet with her in the {sys} system, supposedly an acquaintance of yours. The pilot who told you this said that there's no rush, "but I suggest you go see her at the earliest opportunity".]]), {sys=sys}) )
      naev.missionStart("Shadow Vigil")
   end
   evt.finish()
end

-- Clean up
function finish()
   if hailhook then
      hook.rm(hailhook)
   end
   evt.finish()
end

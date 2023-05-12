--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 2">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Katar I</spob>
 <location>Bar</location>
 <done>Za'lek Particle Physics 1</done>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 02

   Simple mission where the player has to bring back other supplies and gets to try the sokoban stuff for the first time.
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local lmisn = require "lmisn"
local sokoban = require "minigames.sokoban"


local reward = zpp.rewards.zpp02
local cargo_name = _("drone interface controllers")
local cargo_amount = 50 -- Amount of cargo to take

local retpnt, retsys = spob.getS( "Katar I" )

function create ()
   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 3, 8, "Za'lek", false, function( p )
      return p:tags().research and p:services()["bar"]
   end )
   if not mem.destpnt then
      misn.finish()
      return
   end

   misn.setNPC( _("Noona"), zpp.noona.portrait, zpp.noona.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You approach Noona, who seems to have a slight frown on her face.]]))
   n(fmt.f(_([["It seems I overestimated the facilities here. I was hoping they would have the XR-578 drone controller interfaces, but it seems like they are still on XR-321. I found a colleague who can provide me with them, but I need someone to go pick them up."
"Would you be so kind to go to {pnt} in the {sys} system and bring {amount} of drone interface controllers? I should be able to make it worth your time with {credits}."]]),
      {pnt=mem.destpnt, sys=mem.destsys, amount=fmt.tonnes(cargo_amount), credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([["I see. I'll have to figure out some way to get it over here…"
She furrows her brow.]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([["Thanks! Once I get the interface up and working, I should be able to start my particle physics experiments. I'm so excited. While you're gone, I guess I'll double check all my code just to calm down. It's going to be great!"]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Pick up some {cargo} from {pnt} in the {sys} system and deliver them to {retpnt}."),
      {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys, retpnt=retpnt} ))

   mem.mrk = misn.markerAdd( mem.destpnt )

   misn.osdCreate( _("Particle Physics"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   mem.state = 1

   hook.land( "land" )
   hook.load( "land" )
end

local npcguy
function land ()
   local pcur = spob.cur()
   if (mem.state==1 or mem.state==2) and pcur==mem.destpnt then
      npcguy = misn.npcAdd( "approach_guy", _("Noona's Colleague"), "zalek2.png", _("You see a Za'lek scientist who seems to fit the description of Noona's colleague.") )

   elseif mem.state==3 and pcur==retpnt then
      vn.clear()
      vn.scene()
      local n = vn.newCharacter( zpp.vn_noona() )
      vn.transition( zpp.noona.transition )
      vn.na(_([[You land and the lone loading drone starts to slowly remove the cargo from your ship. While waiting, you decide to look for Noona, who you quickly find staring at the testing site at the observation deck of the base.]]))
      n(_([["Look at it! Isn't it breathtaking?"
She motions towards the testing site.
"To think of all of the brains that went into designing and preparing for this. All the famous experiments run on that platform. The advancement of humanity as a whole! What new technological wonders are awaiting for us on the other side of the particle physics conundrums?"]]))
      n(_([[She breathes deeply and turns to you.
"Did you get the interface controllers? That's great! Let me try to get them hooked up and I'll finally be able to start my tests. I'm so excited!"
She starts to prance off, when she suddenly turns to you and tosses you a credstick.
"Almost forgot. See you around!"]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zpp.noona.transition )
      vn.run()

      faction.modPlayer("Za'lek", zpp.fctmod.zpp02)
      player.pay( reward )
      zpp.log(_("You brought some drone interface controllers to Noona so that she can begin her particle physics experiments."))
      misn.finish(true)
   end
end

local talked_once = false
function approach_guy ()
   local cargo_space = false
   vn.clear()
   vn.scene()
   local c = vn.newCharacter( _("Noona's Colleague"), {image="zalek2.png"} )
   vn.transition()

   if mem.state==1 then
      if talked_once then
         c(_([["Ready to try again? Let me activate the memory interface for you."]]))
      else
         vn.na(_([[You approach an individual that matches the contact information that Noona gave you.]]))
         c(_([[They raise an eyebrow as you approach.
"You must be Dr. Sanderaite's acquaintance. I have the drone interface controllers ready, but there was kind of a mishap and they're stuck now."
They scratch their head.]]))
         c(_([["I tried to take a look at it, but wasn't able to figure out a damn thing, and since my post-doc eloped to Rulk'ar, I don't have anyone to deal with these sort of inconveniences. If you could take a look at it and get it apart, you should be able to take the controllers with you. All you have to do is align the memory by pushing the data boxes into data sockets. Please take a shot at it."]]))
         talked_once = true
      end

      sokoban.vn{ levels={1,2,3}, header=_("Drone Memory Banks") }
      vn.func( function ()
         if sokoban.completed() then
            mem.state = 2
            vn.jump("sokoban_done")
            return
         end
         vn.jump("sokoban_fail")
      end )

      vn.label("sokoban_fail")
      c(_([["It seems like the interface hasn't been unlocked. Talk to me if you want to try again."]]))
      vn.done()

      vn.label("sokoban_done")
      c(_([[After you unlock the memory, they are able to quickly release the interface controller which falls to the ground with a resounding clunk.
"OK, that's perfect! Let me get the loading drones to get this on your ship and you can be on your way."]]))

   elseif mem.state==2 then
      c(_([["Are you ready to take the cargo now?"]]))

   end

   local fs = player.pilot():cargoFree()
   if fs < cargo_amount then
      vn.na(fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
         {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
      vn.done()
   else
      cargo_space = true
   end

   vn.na(fmt.f(_("The automated dock drones load the {amount} of {cargo} onto your ship."),{amount=fmt.tonnes(cargo_amount), cargo=cargo_name}))
   vn.run()

   -- Failed to do the Sokoban
   if mem.state~=2 or not cargo_space then
      return
   end

   local crg = commodity.new( N_("Drone Interface Controllers"), N_("Special adapters that are able to connect modern drone controllers with older drone connectors.") )
   misn.cargoAdd( crg, cargo_amount )

   misn.osdActive(2)
   mem.state = 3
   misn.markerMove( mem.mrk, retpnt )
   misn.npcRm( npcguy )
end

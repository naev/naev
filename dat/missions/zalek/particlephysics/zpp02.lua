--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Katar I</planet>
  <location>Bar</location>
  <done>Za'lek Particle Physics 1</done>
 </avail>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
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

-- luacheck: globals land approach_guy (Hook functions passed by name)

local reward = zpp.rewards.zpp02
local cargo_name = _("drone interface controllers")
local cargo_amount = 50 -- Amount of cargo to take

local retpnt, retsys = planet.getS( "Katar I" )

function create ()
   mem.destpnt, mem.destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 3, 8, "Za'lek", false, function( p )
      return p.tags().research
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
   vn.na(_([[You approach Noona who seems to have a slight frown on her face.]]))
   n(fmt.f(_([["It seems I underestimated the facilities here. I was hoping they would have the XR-578 drone controller interfaces, but it seems like they are still on XR-321. I found a colleague who would be able to provide me with them, but I need someone to go pick them up."
"Would you be so kind to go to {pnt} in the {sys} system and bring {amount} of drone interface controllers? I should be able to make it worth your time with {credits}."]]),
      {pnt=mem.destpnt, sys=mem.destsys, amount=fmt.tonnes(cargo_amount), credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(fmt.f(_([["I see. I'll have to figure out some way to get it over here…"
She furrows her brow.]]),{pnt=mem.destpnt}))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([["Thanks! Once I get the interface up and working I should be able to start my particle physics experiments. I'm so excited. I guess I'll double check all my code while you are gone to calm down. It's going to be great!"]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc( fmt.f(_("Pick up some {cargo} from {pnt} in the {sys} system and deliver them to {retpnt}."),
      {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys, retpnt=retpnt} ))

   mem.mrk = misn.markerAdd( mem.destpnt )

   misn.osdCreate( _("Particle Physics"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   mem.state = 1

   hook.land( "land" )
end

local npcguy
function land ()
   local pcur = planet.cur()
   if mem.state==1 or mem.state==2 and pcur==mem.destpnt then
      npcguy = misn.npcAdd( "approach_guy", _("Noona's Colleague"), "none.webp", _("TODO") )

   elseif mem.state==3 and pcur==retpnt then
      vn.clear()
      vn.scene()
      local n = vn.newCharacter( zpp.vn_noona() )
      vn.transition( zpp.noona.transition )
      vn.na(_([[]]))
      n(_([[""]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zpp.noona.transition )
      vn.run()

      player.pay( reward )
      zpp.log(_("You brought some drone interface controllers to Noona so that she can begin her particle physics experiments."))
      misn.finish(true)
   end
end

function approach_guy ()
   vn.clear()
   vn.scene()
   local c = vn.newCharacter( "" )
   vn.transition()

   if mem.state==1 then
      vn.na(_([[You approach an individual that matches the contact information that Noona gave you.]]))
      c(_([[They raise an eyebrow as you approach.
"You must be Noona's acquaintance. I have the drone interface controllers ready, but "]]))

      sokoban.vn{ levels={1,2,3}, header="Drone Memory Banks"}
      vn.na( function ()
         if sokoban.completed() then
            mem.state = 2
            vn.jump("sokoban_done")
            return
         end
         vn.jump("sokoban_fail")
      end )

      vn.label("sokoban_fail")
      c(_([[""]]))
      vn.done()

      vn.label("sokoban_done")
      c(_([[""]]))

   elseif mem.state==2 then
      c(_([["Are you ready to take the cargo now?"]]))

   end

   local fs = player.pilot():cargoFree()
   if fs < cargo_amount then
      vn.na(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
         {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
      vn.done()
   end
   vn.na(fmt.f(_("The automated dock drones load the {amount} of {cargo} onto your ship."),{amount=fmt.tonnes(cargo_amount), cargo=cargo_name}))
   vn.run()

   -- Failed to do the Sokoban
   if not mem.state==2 then
      return
   end

   local crg = misn.cargoNew( N_("Drone Interface Controllers"), N_("Special adapters that are able to connect modern drone controllers with older drone connectors.") )
   misn.cargoAdd( crg, cargo_amount )

   misn.osdActive(2)
   mem.state = 3
   misn.markerMove( mem.mrk, retpnt )
   misn.npcRm( npcguy )
end

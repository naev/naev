--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 1">
 <unique />
 <priority>4</priority>
 <chance>30</chance>
 <faction>Za'lek</faction>
 <location>Bar</location>
 <cond>
   if spob.cur() == spob.get("Katar I") then
      return false
   end
   if faction.playerStanding("Za'lek") &lt; 0 then
      return false
   end
   --return require("misn_test").reweight_active() -- Don't reweight since license are important
   return true
 </cond>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 01

   Introductory mission that just has you bring a Noona to Katar I.
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"

local reward = zpp.rewards.zpp01
local destpnt, destsys = spob.getS("Katar I")
local cargo_amount = 30 -- Amount of cargo to take

function create ()
   misn.setNPC( _("Za'lek Scientist"), zpp.noona.portrait, _("You see a Za'lek scientist who seems to be looking for someone to do something for them.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You approach the Za'lek scientist who seems to be a bit relieved with your presence.]]))
   n(fmt.f(_([["You wouldn't happen to be a pilot? I was supposed to get on a shuttle to {pnt}, but they refused to take my equipment with me. Can you believe the audacity?!"
"Would you be able to take me and {amount} of equipment to {pnt} in the {sys} system? I will pay you {credits}."]]),
      {pnt=destpnt, sys=destsys, amount=fmt.tonnes(cargo_amount), credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(fmt.f(_([["You too? How am I going to make it to {pnt}…"
She furrows her brow.]]),{pnt=destpnt}))
   vn.done( zpp.noona.transition )

   vn.label("nospace")
   n(fmt.f(_([["You only have {freespace} of free cargo space. My equipment is {neededspace}!"]]),
         {freespace=fmt.tonnes(player.pilot():cargoFree()), neededspace=fmt.tonnes(cargo_amount) }))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_amount then
         vn.jump("nospace")
         return
      end
   end )
   n(fmt.f(_([["Great! I'm supposed to start my residency at {pnt} soon for some particle physics testing. They have a state-of-the-art testing facility there that can perform all kinds of mind-boggling experiments. It's wild."
She pauses for a moment.
"I forgot to introduce myself didn't I. I am Dr. Sanderaite, but everyone calls me Noona."]]),
      {pnt=destpnt}))
   n(fmt.f(_([["Oh my equipment? Don't worry about it. It's nothing dangerous, and it is properly stored in radiation shielding. We just have to make sure it doesn't overheat and we should be fine."
That doesn't sound very reassuring.
"Anyway, onwards to {pnt}!"]]),
      {pnt=destpnt}))
   vn.na(_([[Automated drones at the space docks load up the particle physics equipment and Noona gets on your ship.]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   local c = commodity.new( N_("Noona and Equipment"), N_("Za'lek scientist Noona and some particle physics-related equipment.") )
   misn.cargoAdd(c, cargo_amount)

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Take Noona and some equipment to {pnt} in the {sys} system."), {pnt=destpnt, sys=destsys} ))

   misn.markerAdd( destpnt )

   misn.osdCreate( _("Particle Physics"), {
      fmt.f(_("Drop off Noona and the equipment at {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
   } )

   hook.land( "land" )
end

function land ()
   if spob.cur() ~= destpnt then
      return
   end

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_("Your ship touches down at the smaller-than-expected research centre and a single loading drone begins to slowly unload your ship. Talk about understaffed."))
   n(_([[Her eyes seem to be sparkling.
"This… is… awesome! Did you see the testing site from the ship? I thought I saw a flash! Maybe it was a Dirac-Bosemann supersymmetry feedback flare. I've never seen one of those in person yet!"
As she keeps on babbling she sort of wanders off into the base, and you make no attempt to follow her.]]))
   vn.na(_([[You head back to your ship and wonder how you should get in touch with her for your payment, when you notice a small letter with a credstick that she seems to have left in your ship. It reads "Please see your attached payment. I may have more need of your services, please find me later. -n". She must have anticipated her overexcitement. You wonder if this happens a lot.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zpp.noona.transition )
   vn.run()

   faction.modPlayer("Za'lek", zpp.fctmod.zpp01)
   player.pay( reward )
   zpp.log(fmt.f(_("You helped deliver Noona and her equipment to {pnt}. It seems like she may still have more work for you."),{pnt=destpnt}))
   misn.finish(true)
end

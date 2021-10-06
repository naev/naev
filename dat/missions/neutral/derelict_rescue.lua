--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Derelict Rescue">
 <avail>
  <chance>0</chance>
  <location>None</location>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Derelict Rescue Mission

   Triggered by boarding a derelict ship. Just have to take the passengers to a nearby system to save them.
--]]
local fmt = require "format"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

function create ()
   destpnt, destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 0, 5, "Indpendent" )

   -- See if we got something
   if not destpnt then
      -- Can't find target so just make everyone be dead
      vntk.msg(_("Empty derelict"), _([[This derelict is not deserted. The crew are still onboard. Unfortunately for them, they didn't survive whatever wrecked their ship. You decide to give them a decent space burial before moving on.]]))
      misn.finish(false)
   end

   reward_amount = 20e3 + 20e3 * destsys:jumpDist()

   local doaccept = false
   vn.clear()
   vn.scene()
   vn.na(fmt.f(_("You enter and begin to scour the ship for anything of value. As you make through the hallways you hear a noise. After investigating, you end up finding the entire crew of the ship locked up in a dormitory room. They offer you {credits} to take them to safety to {planetname} in the {sysname} system."), {planetname=destpnt:name(), sysname=destsys:name(), credits=fmt.credits(reward_amount)}))
   vn.menu{
      { _("Help them out"), "help" },
      { _("Refuse to help"), "refuse" },
   }

   vn.label("refuse")
   vn.na(_("You refuse to help them and leave them to the mercy of the stars."))
   vn.done()

   vn.label("help")
   vn.na(_("You accept and they thank you profusely as they quickly board your ship with the few belongings they had with them."))
   vn.func( function ()
      doaccept = true
   end )

   vn.run()

   -- Player didn't accept
   if not doaccept then
      misn.finish(false)
   end

   misn.accept()

   misn.setTitle(_("Derelict Rescue"))
   misn.setDesc(fmt.f(_("You have agreed to take some crew you rescued from a derelict ship to {planetname} in the {sysname} system."), {planetname=destpnt:name(), sysname=destsys:name()}))
   misn.setReward(fmt.credits(reward_amount))

   local c = misn.cargoNew( N_("Rescued Crew"), N_("Some crew you rescued from a derelict ship.") )
   civs = misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Derelict Rescue"), {
      fmt.f(_("Take the rescued crew to {planetname} in {sysname}"), {planetname=destpnt:name(), sysname=destsys:name()})
   } )
   misn.markerAdd( destsys, "low" )

   hook.land("land")
end


function land ()
   if planet.cur() ~= destpnt then return end

   vn.clear()
   vn.scene()
   vn.na(fmt.f(_([[Soon after you land the crew you rescued from the derelict burst out of the ship in joy. After a while the captain comes over you and gives you the credits you were promised.

You have received #g{credits}#0.
]]), {credits=fmt.credits(reward_amount)}))
   vn.func( function ()
      player.pay( reward_amount )
   end )
   vn.sfxVictory()
   vn.run()

   misn.finish( true )
end


function abort ()
   if player.isLanded() then
      vntk.msg(nil, _("You inform the crew you rescued from the derelict that you won't be taking them any further. They thank you and get off your ship."))
   else
      vntk.msg(nil, _("You jet the crew you rescued from the derelict out of the airlock."))
      misn.cargoJet( civs )
   end
   misn.finish(false)
end

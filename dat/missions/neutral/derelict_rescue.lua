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
local der = require "common.derelict"

-- luacheck: globals land (Hook functions passed by name)

function create ()
   mem.destpnt, mem.destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 0, 5, "Independent" )

   -- See if we got something
   if not mem.destpnt then
      -- Can't find target so just make everyone be dead
      vntk.msg(_("Empty derelict"), _([[This derelict is not deserted. The crew are still onboard. Unfortunately for them, they didn't survive whatever wrecked their ship. You decide to give them a decent space burial before moving on.]]))
      misn.finish(false)
   end

   mem.reward_amount = 20e3 + 20e3 * mem.destsys:jumpDist()

   local doaccept = false
   vn.clear()
   vn.scene()
   vn.music( der.sfx.ambient )
   vn.sfx( der.sfx.board )
   vn.transition()
   vn.na(fmt.f(_("You enter and begin to scour the ship for anything of value. As you make through the hallways you hear a noise. After investigating, you end up finding the entire crew of the ship locked up in a dormitory room. They offer you {credits} to take them to safety to {pnt} in the {sys} system."), {pnt=mem.destpnt, sys=mem.destsys, credits=fmt.credits(mem.reward_amount)}))
   vn.menu{
      { _("Help them out"), "help" },
      { _("Refuse to help"), "refuse" },
   }

   vn.label("refuse")
   vn.na(_("You refuse to help them and leave them to the mercy of the stars."))
   vn.sfx( der.sfx.unboard )
   vn.done()

   vn.label("help")
   vn.na(_("You accept and they thank you profusely as they quickly board your ship with the few belongings they had with them."))
   vn.func( function ()
      doaccept = true
   end )

   vn.sfx( der.sfx.unboard )
   vn.run()


   -- Player didn't accept
   if not doaccept then
      misn.finish(false)
   end

   misn.accept()

   misn.setTitle(_("Derelict Rescue"))
   misn.setDesc(fmt.f(_("You have agreed to take some crew you rescued from a derelict ship to {pnt} in the {sys} system."), {pnt=mem.destpnt, sys=mem.destsys}))
   misn.setReward(fmt.credits(mem.reward_amount))

   local c = commodity.new( N_("Rescued Crew"), N_("Some crew you rescued from a derelict ship.") )
   mem.civs = misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Derelict Rescue"), {
      fmt.f(_("Take the rescued crew to {pnt} in {sys}"), {pnt=mem.destpnt, sys=mem.destsys})
   } )
   misn.markerAdd( mem.destpnt, "low" )

   hook.land("land")
end


function land ()
   if planet.cur() ~= mem.destpnt then return end

   vn.clear()
   vn.scene()
   vn.na(_([[Soon after you land the crew you rescued from the derelict burst out of the ship in joy. After a while the captain comes over you and gives you the credits you were promised.]])
      .. "\n\n"
      .. fmt.reward(mem.reward_amount))
   vn.func( function ()
      player.pay( mem.reward_amount )
   end )
   vn.sfxVictory()
   vn.run()

   der.addMiscLog(_("You rescued the crew of a derelict ship."))

   misn.finish( true )
end


function abort ()
   if player.isLanded() then
      vntk.msg(nil, _("You inform the crew you rescued from the derelict that you won't be taking them any further. They thank you and get off your ship."))
   else
      vntk.msg(nil, _("You jet the crew you rescued from the derelict out of the airlock."))
      misn.cargoJet( mem.civs )
   end
   misn.finish(false)
end

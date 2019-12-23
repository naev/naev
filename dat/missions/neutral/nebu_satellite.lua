--[[

   Nebula Satellite

   One-shot mission

   Help some independent scientists put a satellite in the nebula.

]]--

include "dat/scripts/numstring.lua"

-- localization stuff, translators would work here
bar_desc = _("A bunch of scientists seem to be chattering nervously amongst themselves.")
mtitle = {}
mtitle[1] = _("Nebula Satellite")
misn_reward = _("%s credits")
mdesc = {}
mdesc[1] = _("Go to the %s system to launch the probe.")
mdesc[2] = _("Drop off the scientists at %s in the %s system.")
title = {}
title[1] = _("Bar")
title[2] = _("Scientific Exploration")
title[3] = _("Mission Success")
text = {}
text[1] = _([[You approach the scientists. They seem a bit nervous and one mutters something about whether it's a good idea or not. Eventually one of them comes up to you.
    "Hello Captain, we're looking for a ship to take us into the Sol Nebula. Would you be willing to take us there?"]])
text[2] = _([["We had a trip scheduled with a space trader ship, but they backed out at the last minute. So we were stuck here until you came. We've got a probe satellite that we have to release in the %s system to monitor the nebula's growth rate. The probe launch procedure is pretty straightforward and shouldn't have any complications."
    He takes a deep breath, "We hope to be able to find out more secrets of the Sol Nebula so mankind can once again regain its lost patrimony. So far the radiation and volatility of the deeper areas haven't been very kind to our instruments. That's why we designed this satellite we're going to launch."]])
text[3] = _([["The plan is for you to take us to %s so we can launch the probe, and then return us to our home at %s in the %s system. The probe will automatically send us the data we need if all goes well. You'll be paid %d credits when we arrive."]])
text[4] = _([[The scientists thank you for your help before going back to their home to continue their nebula research.]])
text[9] = _([["You do not have enough free cargo space to accept this mission!"]])
launch = {}
launch[1] = _("Preparing to launch satellite probe...")
launch[2] = _("Launch in 5...")
launch[3] = _("Satellite launch successful!")

articles={}
articles=
{
{
   "Generic",
   _("Scientists launch science probe into the nebula"),
   _("A group of scientists have succesfully launched a science probe in the nebula. The probe was specifically designed to be resistant to the corrosive environment of the nebula, and is supposed to bring new clues about the nature of the gas and where it's from"), 
}
}


function create ()
   -- Note: this mission does not make any system claims.
   -- Set up mission variables
   misn_stage = 0
   homeworld, homeworld_sys = planet.getLandable( misn.factions() )
   if homeworld == nil then
      misn.finish(false)
   end
   satellite_sys = system.get("Arandon") -- Not too unstable
   credits = 750000

   -- Set stuff up for the spaceport bar
   misn.setNPC( _("Scientists"), "neutral/scientist" )
   misn.setDesc( bar_desc )

end


function accept ()
   -- See if rejects mission
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   -- Check for cargo space
   if player.pilot():cargoFree() <  3 then
      tk.msg( title[1], text[9] )
      misn.finish()
   end

   -- Add cargo
   cargo = misn.cargoAdd( "Satellite", 3 )

   -- Set up mission information
   misn.setTitle( mtitle[1] )
   misn.setReward( string.format( misn_reward, numstring(credits) ) )
   misn.setDesc( string.format( mdesc[1], satellite_sys:name() ) )
   misn_marker = misn.markerAdd( satellite_sys, "low" )

   -- Add mission
   misn.accept()

   -- More flavour text
   tk.msg( title[2], string.format(text[2], satellite_sys:name()) )
   tk.msg( title[2], string.format(text[3], satellite_sys:name(),
         homeworld:name(), homeworld_sys:name(), credits ) )

   misn.osdCreate(mtitle[1], {mdesc[1]:format(satellite_sys:name())})
   -- Set up hooks
   hook.land("land")
   hook.enter("jumpin")
end


function land ()
   landed = planet.cur()
   -- Mission success
   if misn_stage == 1 and landed == homeworld then
      tk.msg( title[3], text[4] )
      player.pay( credits )
      misn.finish(true)
   end
end


function jumpin ()
   sys = system.cur()
   -- Launch satellite
   if misn_stage == 0 and sys == satellite_sys then
      hook.timer( 3000, "beginLaunch" )
   end
end

--[[
   Launch process
--]]
function beginLaunch ()
   player.msg( launch[1] )
   misn.osdDestroy()
   hook.timer( 3000, "beginCountdown" )
end
function beginCountdown ()
   countdown = 5
   player.msg( launch[2] )
   hook.timer( 1000, "countLaunch" )
end
function countLaunch ()
   countdown = countdown - 1
   if countdown <= 0 then
      launchSatellite()
   else
      player.msg( string.format(_("%d..."), countdown) )
      hook.timer( 1000, "countLaunch" )
   end
end
function launchSatellite ()

   articles[1][4]=time.get()+time.create(0,3,0)
   news.add(articles)


   misn_stage = 1
   player.msg( launch[3] )
   misn.cargoJet( cargo )
   misn.setDesc( string.format( mdesc[2], homeworld:name(), homeworld_sys:name() ) )
   misn.osdCreate(mtitle[1], {mdesc[2]:format(homeworld:name(), homeworld_sys:name())})
   misn.markerMove( misn_marker, homeworld_sys )
end

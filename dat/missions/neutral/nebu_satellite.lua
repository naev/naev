--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Nebula Satellite">
 <unique />
 <priority>4</priority>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Goddard</faction>
 <cond>
   return require("test_misn").reweight_active()
 </cond>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
 --]]
--[[

   Nebula Satellite

   One-shot mission

   Help some independent scientists put a satellite in the nebula.

]]--
local pir = require "common.pirate"
local fmt = require "format"
local neu = require "common.neutral"
local portrait = require "portrait"
local vn = require "vn"

local credits = 750e3
local reward = outfit.get("Satellite Mock-up")
local cargo_space = 3
local launchSatellite -- Forward-declared functions

local articles = {
   {
      faction = "Generic",
      head = _("Scientists Launch Research Probe Into Nebula"),
      body = _("A group of scientists successfully launched a science probe into the Nebula. The probe was specifically designed to be resistant to the corrosive environment of the Nebula and is supposed to find new clues about the nature of the gas and where it's from."),
   }
}

local npc_name = _("Scientists")
local npc_portrait = "neutral/unique/neil.webp"
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   -- Note: this mission does not make any system claims.
   -- Set up mission variables
   mem.misn_stage = 0
   mem.homeworld, mem.homeworld_sys = spob.getLandable( misn.factions() )
   if mem.homeworld == nil then
      misn.finish(false)
   end
   mem.satellite_sys = system.get("Arandon") -- Not too unstable

   -- Set stuff up for the spaceport bar
   misn.setNPC( npc_name, npc_portrait, _("A bunch of scientists seem to be chattering nervously among themselves.") )
end


function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local sci = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   sci(_([[You approach the scientists. They seem a bit nervous and one mutters something about whether it's a good idea or not. Eventually one of them comes up to you.
"Hello Captain, we're looking for a ship to take us into the Sol Nebula. Would you be willing to take us there?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   vn.done()

   vn.label("nospace")
   vn.na(fmt.f(_([["You need an additional {space} of free cargo space to accept this mission!"]]),
      {space=cargo_space - player.pilot():cargoFree()}))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_space then
         vn.jump("nospace")
         return
      end
      accepted = true
   end )
   vn.na(fmt.f(_([["We had a trip scheduled with a space trader, but they backed out at the last minute. So we were stuck here until you came. We've got a research probe that we want to release into the {sys} system to monitor the Nebula's growth rate. The probe launch procedure is pretty straightforward and shouldn't have any complications."
He takes a deep breath, "We hope to be able to find out more secrets of the Sol Nebula so mankind can once again regain its lost heritage. So far, the radiation and volatility of the deeper areas haven't been very kind to our instruments. That's why we designed this probe we're going to launch."]]),
      {sys=mem.satellite_sys}))
   vn.na(fmt.f(_([["The plan is for you to take us to {sys} so we can launch the probe, and then return us to our home at {home_pnt} in the {home_sys} system. If all goes well, the probe will automatically send us the data we need. You'll be paid {credits} when we arrive."]]),
      {sys=mem.satellite_sys, home_pnt=mem.homeworld, home_sys=mem.homeworld_sys, credits=fmt.credits(credits)}))
   vn.run()

   if not accepted then return end

   -- Add cargo
   local c = commodity.new( N_("Satellite"), N_("A small space probe loaded with sensors for exploring the depths of the nebula.") )
   mem.cargo = misn.cargoAdd( c, cargo_space )

   -- Set up mission information
   misn.setTitle( _("Nebula Satellite") )
   misn.setReward(credits)
   misn.setDesc( fmt.f( _("Go to the {sys} system to launch the probe."), {sys=mem.satellite_sys} ) )
   mem.misn_marker = misn.markerAdd( mem.satellite_sys, "low" )

   -- Add mission
   misn.accept()

   misn.osdCreate(_("Nebula Satellite"), {fmt.f(_("Go to the {sys} system to launch the probe."), {sys=mem.satellite_sys})})
   -- Set up hooks
   hook.land("land")
   hook.enter("jumpin")
end


function land ()
   mem.landed = spob.cur()
   -- Mission success
   if mem.misn_stage == 1 and mem.landed == mem.homeworld then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[The scientists thank you for your help before going back to their home to continue their nebula research. As a keepsake, one of them gives you a mock-up of the probe you helped them launch.]]) )
      vn.sfxVictory()
      vn.func( function ()
         player.outfitAdd( reward )
         player.pay( credits )
      end )
      vn.na(fmt.reward(credits).."\n"..fmt.reward(reward))
      vn.run()
      pir.reputationNormalMission(rnd.rnd(2,3))
      neu.addMiscLog( _([[You helped a group of scientists launch a research probe into the Nebula.]]) )
      misn.finish(true)
   end
end


function jumpin ()
   mem.sys = system.cur()
   -- Launch satellite
   if mem.misn_stage == 0 and mem.sys == mem.satellite_sys then
      hook.timer( 3.0, "beginLaunch" )
   end
end

--[[
   Launch process
--]]
function beginLaunch ()
   player.msg( _("Preparing to launch space probe…") )
   misn.osdDestroy()
   hook.timer( 3.0, "beginCountdown" )
end
function beginCountdown ()
   mem.countdown = 5
   player.msg( _("Launch in 5…") )
   hook.timer( 1.0, "countLaunch" )
end
function countLaunch ()
   mem.countdown = mem.countdown - 1
   if mem.countdown <= 0 then
      launchSatellite()
   else
      player.msg( string.format(_("%d…"), mem.countdown) )
      hook.timer( 1.0, "countLaunch" )
   end
end
function launchSatellite ()
   articles[1].date_to_rm = time.get()+time.new(0,20,0)
   news.add( articles )

   mem.misn_stage = 1
   player.msg( _("Space probe launch successful!") )
   misn.cargoJet( mem.cargo )
   misn.setDesc( fmt.f( _("Drop off the scientists at {pnt} in the {sys} system."), {pnt=mem.homeworld, sys=mem.homeworld_sys} ) )
   misn.osdCreate(_("Nebula Satellite"), {fmt.f(_("Drop off the scientists at {pnt} in the {sys} system."), {pnt=mem.homeworld, sys=mem.homeworld_sys})})
   misn.markerMove( mem.misn_marker, mem.homeworld )
end

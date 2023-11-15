--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Eliminate a Dvaered Patrol">
 <priority>3</priority>
 <chance>550</chance>
 <done>Disrupt a Dvaered Patrol</done>
 <location>Computer</location>
 <faction>FLF</faction>
 <faction>Frontier</faction>
 <cond>not diff.isApplied( "flf_dead" )</cond>
</mission>
 --]]
--[[

   FLF patrol elimination mission.

--]]
local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"
local vn = require "vn"

-- luacheck: globals enter fleetDV fleetFLF land_flf leave misn_title patrol_getSystem patrol_spawnDV patrol_spawnFLF pilot_death_dv setDescription timer_lateFLF (shared with derived missions flf_empatrol, flf_pre02)

misn_title = {
   _("FLF: Single Dvaered patrol in {sys}"),
   _("FLF: Small Dvaered patrol in {sys}"),
   _("FLF: Medium Dvaered patrol in {sys}"),
   _("FLF: Large Dvaered patrol in {sys}"),
   _("FLF: Dangerous Dvaered patrol in {sys}"),
   _("FLF: Highly Dangerous Dvaered patrol in {sys}"),
}

local text = {
   _("After you are handed your pay, an FLF soldier congratulates you on your victory and buys you a drink. You chat for a while before getting back to work."),
   _("As you get your pay from the officer, FLF soldiers congratulate you on your victory."),
   _("You collect your pay from the officer, who then congratulates you on your victory."),
}

mem.osd_desc    = {
   _("Fly to the {sys} system"),
   _("Eliminate the Dvaered patrol"),
   _("Return to FLF base"),
}


function setDescription ()
   local desc
   desc = fmt.f( n_(
         "There is {n} Dvaered ship patrolling the {sys} system. Eliminate this ship.",
         "There is a Dvaered patrol with {n} ships in the {sys} system. Eliminate this patrol.",
         mem.ships ), {n=mem.ships, sys=mem.missys} )

   if mem.has_vigilance then
      desc = desc .. _(" There is a Vigilance among them, so you must proceed with caution.")
   end
   if mem.has_goddard then
      desc = desc .. _(" There is a Goddard among them, so you must be very careful.")
   end
   if mem.flfships > 0 then
      desc = desc .. fmt.f( n_(
            " You will be accompanied by {n} other FLF pilot for this mission.",
            " You will be accompanied by {n} other FLF pilots for this mission.",
            mem.flfships ), {n=mem.flfships} )
   end
   return desc
end


function patrol_getSystem ()
   return flf.getTargetSystem()
end


function create ()
   mem.missys = patrol_getSystem()
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.level = rnd.rnd( 1, #misn_title )
   mem.ships = 0
   mem.has_vigilance = false
   mem.has_goddard = false
   mem.flfships = 0
   mem.reputation = 0
   if mem.level == 1 then
      mem.ships = 1
   elseif mem.level == 2 then
      mem.ships = rnd.rnd( 2, 3 )
      mem.reputation = 1
   elseif mem.level == 3 then
      mem.ships = 4
      if rnd.rnd() < 0.5 then
         mem.flfships = 2
      end
      mem.reputation = 2
   elseif mem.level == 4 then
      mem.ships = 5
      mem.flfships = rnd.rnd( 2, 4 )
      mem.reputation = 5
   elseif mem.level == 5 then
      mem.ships = 5
      mem.has_vigilance = true
      mem.flfships = rnd.rnd( 4, 6 )
      mem.reputation = 10
   elseif mem.level == 6 then
      mem.ships = rnd.rnd( 5, 6 )
      mem.has_goddard = true
      mem.flfships = rnd.rnd( 8, 10 )
      mem.reputation = 20
   end

   mem.credits = mem.ships * 30e3 - mem.flfships * 1e3
   if mem.has_vigilence then mem.credits = mem.credits + 120e3 end
   if mem.has_goddard then mem.credits = mem.credits + 270e3 end
   mem.credits = mem.credits * (system.cur():jumpDist( mem.missys, true )+1) / 3
   mem.credits = mem.credits * (1 + 0.2*rnd.sigma())

   local desc = setDescription()

   mem.late_arrival = rnd.rnd() < 0.05
   mem.late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   -- Set mission details
   misn.setTitle( fmt.f( misn_title[mem.level], {sys=mem.missys} ) )
   misn.setDesc( desc )
   misn.setReward( mem.credits )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   misn.accept()

   mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
   misn.osdCreate( _("FLF Patrol"), mem.osd_desc )

   mem.dv_ships_left = 0
   mem.job_done = false
   mem.last_system = spob.cur()

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not mem.job_done then
      if system.cur() == mem.missys then
         misn.osdActive( 2 )
         local boss
         if mem.has_goddard then
            boss = "Dvaered Goddard"
         elseif mem.has_vigilance then
            boss = "Dvaered Vigilance"
         end
         patrol_spawnDV( mem.ships, boss )

         if mem.flfships > 0 then
            if not mem.late_arrival then
               patrol_spawnFLF( mem.flfships, mem.last_system, _("Alright, let's have at them!") )
            else
               hook.timer( mem.late_arrival_delay, "timer_lateFLF" )
            end
         end
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   hook.rm( mem.spawner )
   mem.dv_ships_left = 0
   mem.last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   patrol_spawnFLF( mem.flfships, source, _("Sorry we're late! Did we miss anything?") )
end


function pilot_death_dv ()
   mem.dv_ships_left = mem.dv_ships_left - 1
   if mem.dv_ships_left <= 0 then
      mem.job_done = true
      misn.osdActive( 3 )
      misn.markerRm( mem.marker )
      hook.land( "land_flf" )
      pilot.toggleSpawn( true )
      if fleetFLF ~= nil then
         for i, j in ipairs( fleetFLF ) do
            if j:exists() then
               j:changeAI( "flf" )
            end
         end
      end
   end
end


function land_flf ()
   leave()
   mem.last_system = spob.cur()
   if spob.cur():faction() == faction.get("FLF") then

      vn.clear()
      vn.scene()
      vn.transition()
      vn.na( text[ rnd.rnd(1,#text) ] )
      vn.sfxMoney()
      vn.func( function ()
         player.pay( mem.credits )
      end )
      vn.na(fmt.reward(mem.credits))
      vn.run()

      faction.get("FLF"):modPlayer( mem.reputation )
      misn.finish( true )
   end
end


-- Spawn a Dvaered patrol with n ships.
function patrol_spawnDV( n, boss )
   pilot.clear()
   pilot.toggleSpawn( false )
   if rnd.rnd() < 0.05 then n = n + 1 end
   local r = system.cur():radius()
   fleetDV = {}
   for i = 1, n do
      local x = rnd.rnd( -r, r )
      local y = rnd.rnd( -r, r )
      local shipname
      if i == 1 and boss ~= nil then
         shipname = boss
      else
         local shipnames = { "Dvaered Vendetta", "Dvaered Ancestor" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local p = pilot.add( shipname, "Dvaered", vec2.new( x, y ), nil, {ai="dvaered_norun"} )
      hook.pilot( p, "death", "pilot_death_dv" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetDV[i] = p
      mem.dv_ships_left = mem.dv_ships_left + 1
   end
end


-- Spawn n FLF ships at/from the location param.
function patrol_spawnFLF( n, param, comm )
   if rnd.rnd() < 0.05 then n = n - 1 end
   local lancelots = rnd.rnd( n )
   fleetFLF = fleet.add( lancelots, "Lancelot", "FLF", param, nil, {ai="escort_player"} )
   local vendetta_fleet = fleet.add( n - lancelots, "Vendetta", "FLF", param, nil, {ai="escort_player"} )
   for i, j in ipairs( vendetta_fleet ) do
      fleetFLF[ #fleetFLF + 1 ] = j
   end
   for i, j in ipairs( fleetFLF ) do
      j:setFriendly()
      j:setVisible( true )
   end

   fleetFLF[1]:comm( player.pilot(), comm )
end

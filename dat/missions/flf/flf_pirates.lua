--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="FLF Pirate Disturbance">
 <priority>4</priority>
 <chance>330</chance>
 <done>Alliance of Inconvenience</done>
 <location>Computer</location>
 <faction>FLF</faction>
 <faction>Frontier</faction>
 <cond>not diff.isApplied( "flf_dead" )</cond>
</mission>
--]]
--[[
   FLF pirate elimination mission.
--]]
local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"
local vn = require "vn"

-- luacheck: globals enter land_flf leave misn_title pay_text setDescription (shared with derived mission flf_dvk05)
-- luacheck: globals pilot_death_pirate timer_lateFLF (Hook functions passed by name)

misn_title = {
   _("FLF: Lone Pirate Disturbance in {sys}"),
   _("FLF: Minor Pirate Disturbance in {sys}"),
   _("FLF: Moderate Pirate Disturbance in {sys}"),
   _("FLF: Substantial Pirate Disturbance in {sys}"),
   _("FLF: Dangerous Pirate Disturbance in {sys}"),
   _("FLF: Highly Dangerous Pirate Disturbance in {sys}"),
}

pay_text = {
   _("The official mumbles something about the pirates being irritating as a credit chip is pressed into your hand."),
   _("While polite, something seems off about the smile plastered on the official who hands you your pay."),
   _("The official thanks you dryly for your service and hands you a credit chip."),
   _("The official takes an inordinate amount of time to do so, but eventually hands you your pay as promised."),
}

mem.osd_desc = {
   _("Fly to the {sys} system"),
   _("Eliminate the pirates"),
   _("Return to FLF base"),
}

local fleetFLF -- Non-persistent state (not reused by flf_dvk05, which "require"s this script)
local patrol_spawnFLF, patrol_spawnPirates -- Forward-declared functions


function setDescription ()
   local desc
   desc = fmt.f( n_(
         "There is {n} pirate ship disturbing FLF operations in the {sys} system. Eliminate this ship.",
         "There is a pirate group with {n} ships disturbing FLF operations in the {sys} system. Eliminate this group.",
         mem.ships ), {n=mem.ships, sys=mem.missys} )

   if mem.has_phalanx then
      desc = desc .. _(" There is a Phalanx among them, so you must proceed with caution.")
   end
   if mem.has_kestrel then
      desc = desc .. _(" There is a Kestrel among them, so you must be very careful.")
   end
   if mem.flfships > 0 then
      desc = desc .. fmt.f( n_(
            " You will be accompanied by {n} other FLF pilot for this mission.",
            " You will be accompanied by {n} other FLF pilots for this mission.",
            mem.flfships ), {n=mem.flfships} )
   end
   return desc
end


function create ()
   mem.missys = flf.getPirateSystem()
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.level = rnd.rnd( 1, #misn_title )
   mem.ships = 0
   mem.has_boss = false
   mem.has_phalanx = false
   mem.has_kestrel = false
   mem.flfships = 0
   mem.reputation = 0
   if mem.level == 1 then
      mem.ships = 1
   elseif mem.level == 2 then
      mem.ships = rnd.rnd( 2, 3 )
      mem.reputation = 1
   elseif mem.level == 3 then
      mem.ships = 5
      mem.flfships = rnd.rnd( 0, 2 )
      mem.reputation = 2
   elseif mem.level == 4 then
      mem.ships = 6
      mem.has_boss = true
      mem.flfships = rnd.rnd( 4, 6 )
      mem.reputation = 5
   elseif mem.level == 5 then
      mem.ships = 6
      mem.has_phalanx = true
      mem.flfships = rnd.rnd( 4, 6 )
      mem.reputation = 10
   elseif mem.level == 6 then
      mem.ships = rnd.rnd( 6, 9 )
      mem.has_kestrel = true
      mem.flfships = rnd.rnd( 8, 10 )
      mem.reputation = 20
   end

   mem.credits = mem.ships * 190e3 - mem.flfships * 1e3
   if mem.has_phalanx then mem.credits = mem.credits + 310e3 end
   if mem.has_kestrel then mem.credits = mem.credits + 810e3 end
   mem.credits = mem.credits + rnd.sigma() * 8e3

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
   misn.osdCreate( _("Pirate Disturbance"), mem.osd_desc )

   mem.pirate_ships_left = 0
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
         if mem.has_kestrel then
            boss = "Pirate Kestrel"
         elseif mem.has_phalanx then
            boss = "Pirate Phalanx"
         elseif mem.has_boss then
            local choices = { "Pirate Admonisher", "Pirate Rhino" }
            boss = choices[ rnd.rnd( 1, #choices ) ]
         end
         patrol_spawnPirates( mem.ships, boss )

         if mem.flfships > 0 then
            if not mem.late_arrival then
               patrol_spawnFLF( mem.flfships, mem.last_system, _("Alright, let's have at them!") )
            else
               mem.spawner = hook.timer( mem.late_arrival_delay, "timer_lateFLF" )
            end
         end
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   hook.rm( mem.spawner )
   mem.pirate_ships_left = 0
   mem.last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   patrol_spawnFLF( mem.flfships, source, _("Sorry we're late! Did we miss anything?") )
end


function pilot_death_pirate ()
   mem.pirate_ships_left = mem.pirate_ships_left - 1
   if mem.pirate_ships_left <= 0 then
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
      vn.na( pay_text[ rnd.rnd(1,#pay_text) ] )
      vn.sfxMoney()
      vn.func( function ()
         player.pay( mem.credits )
         faction.get("FLF"):modPlayerSingle( mem.reputation )
      end )

      misn.finish( true )
   end
end


-- Spawn a pirate patrol with n ships.
function patrol_spawnPirates( n, boss )
   pilot.clear()
   pilot.toggleSpawn( false )

   if rnd.rnd() < 0.05 then n = n + 1 end

   local frogue = faction.dynAdd( "Pirate", "Rogue Pirate", _("Rogue Pirate"), {clear_allies=true, clear_enemies=true})
   frogue:dynEnemy("FLF")

   --fleetPirate = {}
   for i = 1, n do
      local pos = vec2.newP( 0.8*system.cur():radius()*rnd.rnd(), rnd.angle() )
      local shipname
      if i == 1 and boss ~= nil then
         shipname = boss
      else
         local shipnames = { "Pirate Hyena", "Pirate Shark", "Pirate Vendetta", "Pirate Ancestor" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local p = pilot.add( shipname, frogue, pos, nil, {ai="pirate_norun"} )
      hook.pilot( p, "death", "pilot_death_pirate" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      --fleetPirate[i] = p
      mem.pirate_ships_left = mem.pirate_ships_left + 1
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

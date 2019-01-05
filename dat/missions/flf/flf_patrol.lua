--[[

   FLF patrol elimination mission.
   Copyright (C) 2014, 2015 Julie Marchant <onpon4@riseup.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

include "numstring.lua"
include "fleethelper.lua"
include "dat/missions/flf/flf_common.lua"

misn_title  = _("FLF: %s Dvaered patrol in %s")
misn_reward = _("%s credits")

text = {}
text[1] = _("After you are handed your pay, an FLF soldier congratulates you for your victory and buys you a drink. You chat for a while before getting back to work.")
text[2] = _("As you get your pay from the officer, FLF soldiers congratulate you for your victory.")
text[3] = _("You collect your pay from the officer, who then congratulates you for your victory.")

flfcomm = {}
flfcomm[1] = _("Alright, let's have at them!")
flfcomm[2] = _("Sorry we're late! Did we miss anything?")

misn_desc = {}
misn_desc[1] = _("There is a Dvaered patrol with %d ships in the %s system. Eliminate this patrol.")
misn_desc[2] = _("There is a Dvaered ship patrolling the %s system. Eliminate this ship.")
misn_desc[3] = _(" There is a Vigilance among them, so you must proceed with caution.")
misn_desc[4] = _(" There is a Goddard among them, so you must be very careful.")
misn_desc[5] = _(" You will be accompanied by %d other FLF pilots for this mission.")

misn_level = {}
misn_level[1] = _("Single")
misn_level[2] = _("Small")
misn_level[3] = _("Medium")
misn_level[4] = _("Large")
misn_level[5] = _("Dangerous")
misn_level[6] = _("Highly Dangerous")

osd_title   = _("Dvaered Patrol")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the Dvaered patrol")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function patrol_getSystem ()
   return flf_getTargetSystem()
end


function create ()
   missys = patrol_getSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_level )
   ships = 0
   has_vigilance = false
   has_goddard = false
   flfships = 0
   reputation = 0
   if level == 1 then
      ships = 1
   elseif level == 2 then
      ships = rnd.rnd( 2, 3 )
      reputation = 1
   elseif level == 3 then
      ships = 4
      if rnd.rnd() < 0.5 then
         flfships = 2
      end
      reputation = 2
   elseif level == 4 then
      ships = 5
      flfships = rnd.rnd( 2, 4 )
      reputation = 5
   elseif level == 5 then
      ships = 5
      has_vigilance = true
      flfships = rnd.rnd( 4, 6 )
      reputation = 10
   elseif level == 6 then
      ships = rnd.rnd( 5, 6 )
      has_goddard = true
      flfships = rnd.rnd( 8, 10 )
      reputation = 20
   end

   credits = ships * 30000 - flfships * 1000
   if has_vigilence then credits = credits + 120000 end
   if has_goddard then credits = credits + 270000 end
   credits = credits * system.cur():jumpDist( missys ) / 3
   credits = credits + rnd.sigma() * 8000

   local desc
   if ships == 1 then
      desc = misn_desc[2]:format( missys:name() )
   else
      desc = misn_desc[1]:format( ships, missys:name() )
   end
   if has_vigilance then desc = desc .. misn_desc[3] end
   if has_goddard then desc = desc .. misn_desc[4] end
   if flfships > 0 then
      desc = desc .. misn_desc[5]:format( flfships )
   end

   late_arrival = rnd.rnd() < 0.05
   late_arrival_delay = rnd.rnd( 10000, 120000 )

   -- Set mission details
   misn.setTitle( misn_title:format( misn_level[level], missys:name() ) )
   misn.setDesc( desc )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = osd_desc[1]:format( missys:name() )
   misn.osdCreate( osd_title, osd_desc )

   dv_ships_left = 0
   job_done = false
   last_system = planet.cur()

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         local boss
         if has_goddard then
            boss = "Dvaered Goddard"
         elseif has_vigilance then
            boss = "Dvaered Vigilance"
         end
         patrol_spawnDV( ships, boss )

         if flfships > 0 then
            if not late_arrival then
               patrol_spawnFLF( flfships, last_system, flfcomm[1] )
            else
               hook.timer( late_arrival_delay, "timer_lateFLF" )
            end
         end
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   if spawner ~= nil then hook.rm( spawner ) end
   dv_ships_left = 0
   last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   patrol_spawnFLF( flfships, source, flfcomm[2] )
end


function pilot_death_dv ()
   dv_ships_left = dv_ships_left - 1
   if dv_ships_left <= 0 then
      job_done = true
      misn.osdActive( 3 )
      misn.markerRm( marker )
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
   last_system = planet.cur()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", text[ rnd.rnd( 1, #text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end


-- Spawn a Dvaered patrol with n ships.
function patrol_spawnDV( n, boss )
   pilot.clear()
   pilot.toggleSpawn( false )
   player.pilot():setVisible( true )
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
      local pstk = pilot.add( shipname, "dvaered_norun", vec2.new( x, y ) )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_dv" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetDV[i] = p
      dv_ships_left = dv_ships_left + 1
   end
end


-- Spawn n FLF ships at/from the location param.
function patrol_spawnFLF( n, param, comm )
   if rnd.rnd() < 0.05 then n = n - 1 end
   local lancelots = rnd.rnd( n )
   fleetFLF = addShips( "FLF Lancelot", "flf_norun", param, lancelots )
   local vendetta_fleet = addShips( "FLF Vendetta", "flf_norun", param, n - lancelots )
   for i, j in ipairs( vendetta_fleet ) do
      fleetFLF[ #fleetFLF + 1 ] = j
   end
   for i, j in ipairs( fleetFLF ) do
      j:setFriendly()
      j:setVisible( true )
   end

   fleetFLF[1]:comm( player.pilot(), comm )
end


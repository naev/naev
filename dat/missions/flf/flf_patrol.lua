--[[

   FLF patrol elimination mission.
   Copyright (C) 2014  Julian Marchant <onpon4@riseup.net>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

--]]

include "numstring.lua"
include "fleethelper.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   misn_title  = "FLF: %s Dvaered patrol in %s"
   misn_reward = "%s credits"

   text = {}
   text[1] = "After you are handed your pay, an FLF soldier congratulates you for your victory and buys you a drink. You chat for a while before getting back to work."
   text[2] = "As you get your pay from the officer, FLF soldiers congratulate you for your victory."
   text[3] = "You collect your pay from the officer, who then congratulates you for your victory."

   flfcomm = {}
   flfcomm[1] = "Alright, let's have at them!"
   flfcomm[2] = "Sorry we're late! Did we miss anything?"

   misn_desc = {}
   misn_desc[1] = "There is a Dvaered patrol with %d ships in the %s system. Eliminate this patrol."
   misn_desc[2] = "There is a Dvaered ship patrolling the %s system. Eliminate this ship."
   misn_desc[3] = " There is a Vigilance among them, so you must proceed with caution."
   misn_desc[4] = " There is a Goddard among them, so you must be very careful."
   misn_desc[5] = " You will be accompanied by %d other FLF pilots for this mission."

   misn_level = {}
   misn_level[1] = "Single"
   misn_level[2] = "Small"
   misn_level[3] = "Medium"
   misn_level[4] = "Large"
   misn_level[5] = "Dangerous"
   misn_level[6] = "Highly Dangerous"

   osd_title   = "Dvaered Patrol"
   osd_desc    = {}
   osd_desc[1] = "Fly to the %s system"
   osd_desc[2] = "Eliminate the Dvaered patrol"
   osd_desc[3] = "Return to the FLF base"
   osd_desc["__save"] = true
end


function create ()
   flfbase = system.cur()
   flfplanet = planet.cur()
   missys = patrol_getTargetSystem()
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
      marker = misn.markerAdd( flfbase, "computer" )
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
   if planet.cur() == flfplanet then
      tk.msg( "", text[ rnd.rnd( 1, #text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end


-- Get a system for the Dvaered patrol.
-- These are systems which have both FLF and Dvaered presence.
function patrol_getTargetSystem ()
   local choices = { "Surano", "Zylex", "Arcanis", "Sonas", "Raelid", "Toaxis", "Tau Prime", "Zacron", "Tuoladis", "Doranthex", "Torg", "Tarsus", "Klantar", "Verex", "Dakron", "Theras", "Gilligan's Light", "Haleb", "Slaccid", "Norpin", "Triap", "Brimstone" }
   return system.get( choices[ rnd.rnd( 1, #choices ) ] )
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


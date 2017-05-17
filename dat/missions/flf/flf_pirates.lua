--[[

   FLF pirate elimination mission.

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

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   misn_title  = "FLF: %s pirate terrorism in %s"
   misn_reward = "%s credits"

   text = {}
   text[1] = "After you are handed your pay, an FLF soldier thanks you for your service protecting the Frontier and buys you a drink. You chat for a while before getting back to work."
   text[2] = "As you get your pay from the officer, FLF soldiers thank you for your service."
   text[3] = "You collect your pay from the officer, who then thanks you for protecting the Frontier."
   text[4] = "As you return to the FLF base and collect your pay, you are met with cheers for your success in defending the Frontier from pirate scum."

   flfcomm = {}
   flfcomm[1] = "Alright, let's have at them!"
   flfcomm[2] = "Sorry we're late! Did we miss anything?"

   misn_desc = {}
   misn_desc[1] = "There is a pirate group with %d ships terrorizing the %s system. Eliminate this group."
   misn_desc[2] = "There is a pirate ship terrorizing the %s system. Eliminate this ship."
   misn_desc[3] = " There is a Phalanx among them, so you must proceed with caution."
   misn_desc[4] = " There is a Kestrel among them, so you must be very careful."
   misn_desc[5] = " You will be accompanied by %d other FLF pilots for this mission."

   misn_level = {}
   misn_level[1] = "Lone"
   misn_level[2] = "Minor"
   misn_level[3] = "Moderate"
   misn_level[4] = "Substantial"
   misn_level[5] = "Dangerous"
   misn_level[6] = "Highly Dangerous"

   osd_title   = "Pirate Terrorism"
   osd_desc    = {}
   osd_desc[1] = "Fly to the %s system"
   osd_desc[2] = "Eliminate the pirates"
   osd_desc[3] = "Return to FLF base"
   osd_desc["__save"] = true
end


function create ()
   missys = flf_getPirateSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_level )
   ships = 0
   has_ancestor = false
   has_kestrel = false
   flfships = 0
   reputation = 0
   other_reputation = 0
   if level == 1 then
      ships = 1
   elseif level == 2 then
      ships = rnd.rnd( 3, 4 )
      reputation = 1
   elseif level == 3 then
      ships = 5
      if rnd.rnd() < 0.5 then
         flfships = 2
      end
      reputation = 2
   elseif level == 4 then
      ships = 6
      flfships = rnd.rnd( 2, 4 )
      reputation = 5
      other_reputation = 1
   elseif level == 5 then
      ships = 6
      has_ancestor = true
      flfships = rnd.rnd( 4, 6 )
      reputation = 10
      other_reputation = 2
   elseif level == 6 then
      ships = rnd.rnd( 6, 9 )
      has_kestrel = true
      flfships = rnd.rnd( 8, 10 )
      reputation = 20
      other_reputation = 4
   end

   credits = ships * 190000 - flfships * 1000
   if has_vigilence then credits = credits + 310000 end
   if has_kestrel then credits = credits + 810000 end
   credits = credits + rnd.sigma() * 8000

   local desc
   if ships == 1 then
      desc = misn_desc[2]:format( missys:name() )
   else
      desc = misn_desc[1]:format( ships, missys:name() )
   end
   if has_ancestor then desc = desc .. misn_desc[3] end
   if has_kestrel then desc = desc .. misn_desc[4] end
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

   pirate_ships_left = 0
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
         if has_kestrel then
            boss = "Pirate Kestrel"
         elseif has_ancestor then
            boss = "Pirate Ancestor"
         end
         patrol_spawnPirates( ships, boss )

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
   pirate_ships_left = 0
   last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   patrol_spawnFLF( flfships, source, flfcomm[2] )
end


function pilot_death_pirate ()
   pirate_ships_left = pirate_ships_left - 1
   if pirate_ships_left <= 0 then
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
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", text[ rnd.rnd( 1, #text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayerSingle( reputation )
      faction.get("Frontier"):modPlayerSingle( other_reputation )
      if missys:presences()[ "Dvaered" ] then
         faction.get("Dvaered"):modPlayerSingle( other_reputation )
      end
      if missys:presences()[ "Sirius" ] then
         faction.get("Sirius"):modPlayerSingle( other_reputation )
      end
      if missys:presences()[ "Empire" ] then
         faction.get("Empire"):modPlayerSingle( other_reputation )
      end
      misn.finish( true )
   end
end


-- Spawn a pirate patrol with n ships.
function patrol_spawnPirates( n, boss )
   pilot.clear()
   pilot.toggleSpawn( false )
   player.pilot():setVisible( true )
   if rnd.rnd() < 0.05 then n = n + 1 end
   local r = system.cur():radius()
   fleetPirate = {}
   for i = 1, n do
      local x = rnd.rnd( -r, r )
      local y = rnd.rnd( -r, r )
      local shipname
      if i == 1 and boss ~= nil then
         shipname = boss
      else
         local shipnames = { "Pirate Hyena", "Pirate Shark", "Pirate Vendetta", "Pirate Ancestor", "Pirate Rhino" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local pstk = pilot.add( shipname, "baddie_norun", vec2.new( x, y ) )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_pirate" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetPirate[i] = p
      pirate_ships_left = pirate_ships_left + 1
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


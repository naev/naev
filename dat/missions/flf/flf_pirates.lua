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

misn_title  = _("FLF: %s Pirate Disturbance in %s")
misn_reward = _("%s credits")

pay_text = {}
pay_text[1] = _("The official mumbles something about the pirates being irritating as a credit chip is pressed into your hand.")
pay_text[2] = _("While polite,  something seems off about the smile plastered on the official who hands you your pay.")
pay_text[3] = _("The official thanks you dryly for your service and hands you a credit chip.")
pay_text[4] = _("The offial takes an inordinate amount of time to do so, but eventually hands you your pay as promised.")

flfcomm = {}
flfcomm[1] = _("Alright, let's have at them!")
flfcomm[2] = _("Sorry we're late! Did we miss anything?")

misn_desc = {}
misn_desc[1] = _("There is a pirate group with %d ships disturbing FLF operations in the %s system. Eliminate this group.")
misn_desc[2] = _("There is a pirate ship disturbing FLF operations in the %s system. Eliminate this ship.")
misn_desc[3] = _(" There is a Phalanx among them, so you must proceed with caution.")
misn_desc[4] = _(" There is a Kestrel among them, so you must be very careful.")
misn_desc[5] = _(" You will be accompanied by %d other FLF pilots for this mission.")

misn_level = {}
misn_level[1] = _("Lone")
misn_level[2] = _("Minor")
misn_level[3] = _("Moderate")
misn_level[4] = _("Substantial")
misn_level[5] = _("Dangerous")
misn_level[6] = _("Highly Dangerous")

osd_title   = _("Pirate Disturbance")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the pirates")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function create ()
   missys = flf_getPirateSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_level )
   ships = 0
   has_boss = false
   has_phalanx = false
   has_kestrel = false
   flfships = 0
   reputation = 0
   if level == 1 then
      ships = 1
   elseif level == 2 then
      ships = rnd.rnd( 2, 3 )
      reputation = 1
   elseif level == 3 then
      ships = 5
      flfships = rnd.rnd( 0, 2 )
      reputation = 2
   elseif level == 4 then
      ships = 6
      has_boss = true
      flfships = rnd.rnd( 4, 6 )
      reputation = 5
   elseif level == 5 then
      ships = 6
      has_phalanx = true
      flfships = rnd.rnd( 4, 6 )
      reputation = 10
   elseif level == 6 then
      ships = rnd.rnd( 6, 9 )
      has_kestrel = true
      flfships = rnd.rnd( 8, 10 )
      reputation = 20
   end

   credits = ships * 190000 - flfships * 1000
   if has_phalanx then credits = credits + 310000 end
   if has_kestrel then credits = credits + 810000 end
   credits = credits + rnd.sigma() * 8000

   local desc
   if ships == 1 then
      desc = misn_desc[2]:format( missys:name() )
   else
      desc = misn_desc[1]:format( ships, missys:name() )
   end
   if has_phalanx then desc = desc .. misn_desc[3] end
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
         if has_kestrel then
            boss = "Pirate Kestrel"
         elseif has_phalanx then
            boss = "Pirate Phalanx"
         elseif has_boss then
            local choices = { "Pirate Admonisher", "Pirate Rhino" }
            boss = choices[ rnd.rnd( 1, #choices ) ]
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
   last_system = planet.cur()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayerSingle( reputation )
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
         local shipnames = { "Pirate Hyena", "Pirate Shark", "Pirate Vendetta", "Pirate Ancestor" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local pstk = pilot.add( shipname, "baddie_norun", vec2.new( x, y ) )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_pirate" )
      p:setFaction( "Rogue Pirate" )
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


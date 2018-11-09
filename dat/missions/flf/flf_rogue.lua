--[[

   Rogue FLF Elimination Mission

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

misn_title  = _("FLF: Rogue %s in %s")
misn_reward = _("%s credits")

text = {}
text[1] = _("You are thanked for eliminating the traitorous scum and handed a credit chip with the agreed-upon payment.")
text[2] = _("The official who hands you your pay mumbles something about traitors and then summarily dismisses you.")
text[3] = _("While it takes an inordinate amount of time, you are eventually handed the agreed-upon payment for dispatching the traitor.")

misn_desc = {}
misn_desc[1] = _("There is a squadron of rogue FLF ships with %d ships in the %s system. Eliminate this squadron.")
misn_desc[2] = _("There is a rogue FLF ship in the %s system. Eliminate this ship.")
misn_desc[3] = _(" You will be accompanied by %d other FLF pilots for this mission.")

misn_level = {}
misn_level[1] = _("Pilot")
misn_level[2] = _("Squadron")
misn_level[3] = _("Squadron")
misn_level[4] = _("Fleet")

osd_title   = _("Rogue FLF")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the rogue FLF %s")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function create ()
   missys = flf_getSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_level )
   ships = 0
   flfships = 0
   if level == 1 then
      ships = 1
   elseif level == 2 then
      ships = rnd.rnd( 2, 3 )
   elseif level == 3 then
      ships = 4
      if rnd.rnd() < 0.5 then
         flfships = 2
      end
   elseif level == 4 then
      ships = 7
      flfships = rnd.rnd( 2, 4 )
   end

   credits = ships * 30000 - flfships * 1000
   credits = credits * system.cur():jumpDist( missys ) / 3
   credits = credits + rnd.sigma() * 8000

   local desc
   if ships == 1 then
      desc = misn_desc[2]:format( missys:name() )
   else
      desc = misn_desc[1]:format( ships, missys:name() )
   end
   if flfships > 0 then
      desc = desc .. misn_desc[3]:format( flfships )
   end

   late_arrival = rnd.rnd() < 0.75
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
   osd_desc[2] = osd_desc[2]:format( misn_level[level] )
   misn.osdCreate( osd_title, osd_desc )

   rogue_ships_left = 0
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
         rogue_spawnRogue( ships )
         if flfships > 0 then
            if not late_arrival then
               rogue_spawnFLF( flfships, last_system )
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
   rogue_ships_left = 0
   last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   rogue_spawnFLF( flfships, source )
end


function pilot_death_rogue ()
   rogue_ships_left = rogue_ships_left - 1
   if rogue_ships_left <= 0 then
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
      misn.finish( true )
   end
end


-- Spawn a rogue FLF squad with n ships.
function rogue_spawnRogue( n )
   pilot.clear()
   pilot.toggleSpawn( false )
   player.pilot():setVisible( true )
   if rnd.rnd() < 0.05 then n = n + 1 end
   local r = system.cur():radius()
   fleetRogue = {}
   for i = 1, n do
      local x = rnd.rnd( -r, r )
      local y = rnd.rnd( -r, r )
      local shipname
      local shipnames = { "Rogue FLF Vendetta", "Rogue FLF Lancelot" }
      shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      local pstk = pilot.add( shipname, "flf_rogue_norun", vec2.new( x, y ) )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_rogue" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetRogue[i] = p
      rogue_ships_left = rogue_ships_left + 1
   end
end


-- Spawn n FLF ships at/from the location param.
function rogue_spawnFLF( n, param )
   if rnd.rnd() < 0.25 then n = n - 1 end
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
end


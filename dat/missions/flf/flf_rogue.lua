--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Rogue FLF">
  <avail>
   <priority>4</priority>
   <chance>550</chance>
   <done>The FLF Split</done>
   <location>Computer</location>
   <faction>FLF</faction>
   <faction>Frontier</faction>
   <cond>not diff.isApplied( "flf_dead" )</cond>
  </avail>
 </mission>
 --]]
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

require "numstring"
require "fleethelper"
require "missions/flf/flf_common"

misn_title  = {}
misn_title[1] = _("FLF: Rogue Pilot in %s")
misn_title[2] = _("FLF: Rogue Squadron in %s")
misn_title[3] = _("FLF: Rogue Squadron in %s")
misn_title[4] = _("FLF: Rogue Fleet in %s")

text = {}
text[1] = _("You are thanked for eliminating the traitorous scum and handed a credit chip with the agreed-upon payment.")
text[2] = _("The official who hands you your pay mumbles something about traitors and then summarily dismisses you.")
text[3] = _("While it takes an inordinate amount of time, you are eventually handed the agreed-upon payment for dispatching the traitor.")

osd_title   = _("Rogue FLF")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the rogue FLF patrol")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function setDescription ()
   local desc
   desc = gettext.ngettext(
         "There is %d rogue FLF ship in the %s system. Eliminate this ship.",
         "There is a squadron of rogue FLF ships with %d ships in the %s system. Eliminate this squadron.",
         ships ):format( ships, missys:name() )
   if flfships > 0 then
      desc = desc .. gettext.ngettext(
            " You will be accompanied by %d other FLF pilot for this mission.",
            " You will be accompanied by %d other FLF pilots for this mission.",
            flfships ):format( flfships )
   end
   return desc
end


function create ()
   missys = flf_getSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_title )
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
   credits = credits * system.cur():jumpDist( missys, true ) / 3
   credits = credits + rnd.sigma() * 8000

   local desc = setDescription()

   late_arrival = rnd.rnd() < 0.75
   late_arrival_delay = rnd.rnd( 10000, 120000 )

   -- Set mission details
   misn.setTitle( misn_title[level]:format( missys:name() ) )
   misn.setDesc( desc )
   misn.setReward( creditstring( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = osd_desc[1]:format( missys:name() )
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
   if planet.cur():faction() == faction.get("FLF") then
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
      local pstk = pilot.addFleet( shipname, vec2.new( x, y ), "flf_rogue_norun" )
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
   fleetFLF = addShips( lancelots, "Lancelot", "FLF", param, _("FLF Lancelot"), "flf_norun" )
   local vendetta_fleet = addShips( n - lancelots, "Vendetta", "FLF", param, _("FLF Vendetta"), "flf_norun" )
   for i, j in ipairs( vendetta_fleet ) do
      fleetFLF[ #fleetFLF + 1 ] = j
   end
   for i, j in ipairs( fleetFLF ) do
      j:setFriendly()
      j:setVisible( true )
   end
end


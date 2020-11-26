--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Eliminate an Empire Patrol">
  <avail>
   <priority>3</priority>
   <chance>550</chance>
   <location>Computer</location>
   <faction>FLF</faction>
   <faction>Frontier</faction>
   <cond>diff.isApplied("flf_vs_empire") and not diff.isApplied( "flf_dead" )</cond>
  </avail>
  <notes>
   <requires name="The Empire and the FLF are enemies"/>
  </notes>
 </mission>
 --]]
--[[

   FLF Empire patrol elimination mission

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

require "missions/flf/flf_patrol.lua"

misn_title = {}
misn_title[1] = _("FLF: Single Empire patrol in %s")
misn_title[2] = _("FLF: Small Empire patrol in %s")
misn_title[3] = _("FLF: Medium Empire patrol in %s")
misn_title[4] = _("FLF: Large Empire patrol in %s")
misn_title[5] = _("FLF: Dangerous Empire patrol in %s")
misn_title[6] = _("FLF: Highly Dangerous Empire patrol in %s")

osd_title   = _("FLF Patrol")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the Empire patrol")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function setDescription ()
   local desc
   desc = gettext.ngettext(
         "There is %d Empire ship patrolling the %s system. Eliminate this ship.",
         "There is an Empire patrol with %d ships in the %s system. Eliminate this patrol.",
         ships ):format( ships, missys:name() )

   if has_vigilance then
      desc = desc .. _(" There is a Pacifier among them, so you must proceed with caution.")
   end
   if has_goddard then
      desc = desc .. _(" There is a Hawking among them, so you must be very careful.")
   end
   if flfships > 0 then
      desc = desc .. gettext.ngettext(
            " You will be accompanied by %d other FLF pilot for this mission.",
            " You will be accompanied by %d other FLF pilots for this mission.",
            flfships ):format( flfships )
   end
   return desc
end


function patrol_getSystem ()
   return flf_getEmpireSystem()
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         local boss
         if has_goddard then
            boss = "Empire Hawking"
         elseif has_vigilance then
            boss = "Empire Pacifier"
         end
         patrol_spawnEmpire( ships, boss )

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


-- Spawn an Empire patrol with n ships.
function patrol_spawnEmpire( n, boss )
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
         local shipnames = { "Empire Shark", "Empire Lancelot" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local pstk = pilot.add( shipname, "empire_norun", vec2.new( x, y ) )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_dv" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetDV[i] = p
      dv_ships_left = dv_ships_left + 1
   end
end

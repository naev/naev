--[[

   FLF Empire patrol elimination mission
   Copyright (C) 2018 Julie Marchant <onpon4@riseup.net>

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

include "dat/missions/flf/flf_patrol.lua"

misn_title  = _("FLF: %s Empire patrol in %s")

misn_desc = {}
misn_desc[1] = _("There is an Empire patrol with %d ships in the %s system. Eliminate this patrol.")
misn_desc[2] = _("There is an Empire ship patrolling the %s system. Eliminate this ship.")
misn_desc[3] = _(" There is a Pacifier among them, so you must proceed with caution.")
misn_desc[4] = _(" There is a Hawking among them, so you must be very careful.")
misn_desc[5] = _(" You will be accompanied by %d other FLF pilots for this mission.")

osd_title   = _("Dvaered Patrol")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Eliminate the Empire patrol")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


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

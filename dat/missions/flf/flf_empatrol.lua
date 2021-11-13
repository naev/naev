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

--]]
require "missions.flf.flf_patrol"
local flf = require "missions.flf.flf_common"
local fmt = require "format"

local patrol_spawnEmpire -- Forward-declared functions

misn_title = {}
misn_title[1] = _("FLF: Single Empire patrol in {sys}")
misn_title[2] = _("FLF: Small Empire patrol in {sys}")
misn_title[3] = _("FLF: Medium Empire patrol in {sys}")
misn_title[4] = _("FLF: Large Empire patrol in {sys}")
misn_title[5] = _("FLF: Dangerous Empire patrol in {sys}")
misn_title[6] = _("FLF: Highly Dangerous Empire patrol in {sys}")

mem.osd_desc    = {}
mem.osd_desc[1] = _("Fly to the {sys} system")
mem.osd_desc[2] = _("Eliminate the Empire patrol")
mem.osd_desc[3] = _("Return to FLF base")


function setDescription ()
   local desc
   desc = fmt.f( n_(
         "There is {n} Empire ship patrolling the {sys} system. Eliminate this ship.",
         "There is an Empire patrol with {n} ships in the {sys} system. Eliminate this patrol.",
         mem.ships ), {n=mem.ships, sys=mem.missys} )

   if mem.has_vigilance then
      desc = desc .. _(" There is a Pacifier among them, so you must proceed with caution.")
   end
   if mem.has_goddard then
      desc = desc .. _(" There is a Hawking among them, so you must be very careful.")
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
   return flf.getEmpireSystem()
end


function enter ()
   if not mem.job_done then
      if system.cur() == mem.missys then
         misn.osdActive( 2 )
         local boss
         if mem.has_goddard then
            boss = "Empire Hawking"
         elseif mem.has_vigilance then
            boss = "Empire Pacifier"
         end
         patrol_spawnEmpire( mem.ships, boss )

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


-- Spawn an Empire patrol with n ships.
function patrol_spawnEmpire( n, boss )
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
         local shipnames = { "Empire Shark", "Empire Lancelot" }
         shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      end
      local p = pilot.add( shipname, "Empire", vec2.new( x, y ), nil, {ai="empire_norun"} )
      hook.pilot( p, "death", "pilot_death_dv" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      fleetDV[i] = p
      mem.dv_ships_left = mem.dv_ships_left + 1
   end
end

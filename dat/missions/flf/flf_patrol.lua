--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Eliminate a Dvaered Patrol">
  <avail>
   <priority>3</priority>
   <chance>550</chance>
   <done>Disrupt a Dvaered Patrol</done>
   <location>Computer</location>
   <faction>FLF</faction>
   <faction>Frontier</faction>
   <cond>not diff.isApplied( "flf_dead" )</cond>
  </avail>
 </mission>
 --]]
--[[

   FLF patrol elimination mission.

--]]

local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"

misn_title = {}
misn_title[1] = _("FLF: Single Dvaered patrol in %s")
misn_title[2] = _("FLF: Small Dvaered patrol in %s")
misn_title[3] = _("FLF: Medium Dvaered patrol in %s")
misn_title[4] = _("FLF: Large Dvaered patrol in %s")
misn_title[5] = _("FLF: Dangerous Dvaered patrol in %s")
misn_title[6] = _("FLF: Highly Dangerous Dvaered patrol in %s")

text = {}
text[1] = _("After you are handed your pay, an FLF soldier congratulates you for your victory and buys you a drink. You chat for a while before getting back to work.")
text[2] = _("As you get your pay from the officer, FLF soldiers congratulate you for your victory.")
text[3] = _("You collect your pay from the officer, who then congratulates you for your victory.")

osd_desc    = {}
osd_desc[1] = _("Fly to the {sys} system")
osd_desc[2] = _("Eliminate the Dvaered patrol")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function setDescription ()
   local desc
   desc = gettext.ngettext(
         "There is %d Dvaered ship patrolling the %s system. Eliminate this ship.",
         "There is a Dvaered patrol with %d ships in the %s system. Eliminate this patrol.",
         ships ):format( ships, missys:name() )

   if has_vigilance then
      desc = desc .. _(" There is a Vigilance among them, so you must proceed with caution.")
   end
   if has_goddard then
      desc = desc .. _(" There is a Goddard among them, so you must be very careful.")
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
   return flf.getTargetSystem()
end


function create ()
   missys = patrol_getSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   level = rnd.rnd( 1, #misn_title )
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

   credits = ships * 30e3 - flfships * 1e3
   if has_vigilence then credits = credits + 120e3 end
   if has_goddard then credits = credits + 270e3 end
   credits = credits * system.cur():jumpDist( missys, true ) / 3
   credits = credits + rnd.sigma() * 80e3

   local desc = setDescription()

   late_arrival = rnd.rnd() < 0.05
   late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   -- Set mission details
   misn.setTitle( misn_title[level]:format( missys:name() ) )
   misn.setDesc( desc )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = fmt.f( osd_desc[1], {sys=missys} )
   misn.osdCreate( _("FLF Patrol"), osd_desc )

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
               patrol_spawnFLF( flfships, last_system, _("Alright, let's have at them!") )
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
   patrol_spawnFLF( flfships, source, _("Sorry we're late! Did we miss anything?") )
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
   if planet.cur():faction() == faction.get("FLF") then
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
      local p = pilot.add( shipname, "Dvaered", vec2.new( x, y ), nil, {ai="dvaered_norun"} )
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


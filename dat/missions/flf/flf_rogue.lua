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

--]]

local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"

local fleetFLF -- Non-persistent state (not reused by flf_dvk07, which "require"s this script)

local misn_title  = {}
misn_title[1] = _("FLF: Rogue Pilot in {sys}")
misn_title[2] = _("FLF: Rogue Squadron in {sys}")
misn_title[3] = _("FLF: Rogue Squadron in {sys}")
misn_title[4] = _("FLF: Rogue Fleet in {sys}")

local text = {}
text[1] = _("You are thanked for eliminating the traitorous scum and handed a credit chip with the agreed-upon payment.")
text[2] = _("The official who hands you your pay mumbles something about traitors and then summarily dismisses you.")
text[3] = _("While it takes an inordinate amount of time, you are eventually handed the agreed-upon payment for dispatching the traitor.")

osd_desc    = {__save=true}
osd_desc[2] = _("Eliminate the rogue FLF patrol")
osd_desc[3] = _("Return to FLF base")


local function setDescription ()
   local desc
   desc = fmt.f( n_(
         "There is {n} rogue FLF ship in the {sys} system. Eliminate this ship.",
         "There is a squadron of rogue FLF ships with {n} ships in the {sys} system. Eliminate this squadron.",
         ships ), {n=ships, sys=missys} )
   if flfships > 0 then
      desc = desc .. fmt.f( n_(
            " You will be accompanied by {n} other FLF pilot for this mission.",
            " You will be accompanied by {n} other FLF pilots for this mission.",
            flfships ), {n=flfships} )
   end
   return desc
end


function create ()
   missys = flf.getSystem()
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

   credits = ships * 30e3 - flfships * 1e3
   credits = credits * system.cur():jumpDist( missys, true ) / 3
   credits = credits + rnd.sigma() * 8e3

   local desc = setDescription()

   late_arrival = rnd.rnd() < 0.75
   late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   -- Set mission details
   misn.setTitle( fmt.f( misn_title[level], {sys=missys} ) )
   misn.setDesc( desc )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = fmt.f( _("Fly to the {sys} system"), {sys=missys} )
   misn.osdCreate( _("Rogue FLF"), osd_desc )

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
   hook.rm( spawner )
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

   if rnd.rnd() < 0.05 then n = n + 1 end
   local shipnames = { "Vendetta", "Lancelot" }
   local pilotnames = { _("Rogue FLF Vendetta"), _("Rogue FLF Lancelot") }
   local frogue = faction.dynAdd( "FLF", "Rogue FLF", _("Rogue FLF"), {clear_allies=true, clear_enemies=true})
   frogue:dynEnemy("FLF")

   --fleetRogue = {}
   for i = 1, n do
      local pos = vec2.newP( 0.8*system.cur():radius()*rnd.rnd(), 360*rnd.rnd )
      local pstk = fleet.add( 1, shipnames, frogue, pos, pilotnames, {ai="flf_rogue_norun"} )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_rogue" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      --fleetRogue[i] = p
      rogue_ships_left = rogue_ships_left + 1
   end
end


-- Spawn n FLF ships at/from the location param.
function rogue_spawnFLF( n, param )
   if rnd.rnd() < 0.25 then n = n - 1 end
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
end


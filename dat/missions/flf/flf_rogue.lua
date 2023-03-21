--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Rogue FLF">
 <priority>4</priority>
 <chance>550</chance>
 <done>The FLF Split</done>
 <location>Computer</location>
 <faction>FLF</faction>
 <faction>Frontier</faction>
 <cond>not diff.isApplied( "flf_dead" )</cond>
</mission>
 --]]
--[[

   Rogue FLF Elimination Mission

--]]

local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"


local fleetFLF -- Non-persistent state (not reused by flf_dvk07, which "require"s this script)
local rogue_spawnFLF, rogue_spawnRogue -- Forward-declared functions

local misn_title  = {}
misn_title[1] = _("FLF: Rogue Pilot in {sys}")
misn_title[2] = _("FLF: Rogue Squadron in {sys}")
misn_title[3] = _("FLF: Rogue Squadron in {sys}")
misn_title[4] = _("FLF: Rogue Fleet in {sys}")

local text = {}
text[1] = _("You are thanked for eliminating the traitorous scum and handed a credit chip with the agreed-upon payment.")
text[2] = _("The official who hands you your pay mumbles something about traitors and then summarily dismisses you.")
text[3] = _("While it takes an inordinate amount of time, you are eventually handed the agreed-upon payment for dispatching the traitor.")

mem.osd_desc    = {}
mem.osd_desc[2] = _("Eliminate the rogue FLF patrol")
mem.osd_desc[3] = _("Return to FLF base")


local function setDescription ()
   local desc
   desc = fmt.f( n_(
         "There is {n} rogue FLF ship in the {sys} system. Eliminate this ship.",
         "There is a squadron of rogue FLF ships with {n} ships in the {sys} system. Eliminate this squadron.",
         mem.ships ), {n=mem.ships, sys=mem.missys} )
   if mem.flfships > 0 then
      desc = desc .. fmt.f( n_(
            " You will be accompanied by {n} other FLF pilot for this mission.",
            " You will be accompanied by {n} other FLF pilots for this mission.",
            mem.flfships ), {n=mem.flfships} )
   end
   return desc
end


function create ()
   mem.missys = flf.getSystem()
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.level = rnd.rnd( 1, #misn_title )
   mem.ships = 0
   mem.flfships = 0
   if mem.level == 1 then
      mem.ships = 1
   elseif mem.level == 2 then
      mem.ships = rnd.rnd( 2, 3 )
   elseif mem.level == 3 then
      mem.ships = 4
      if rnd.rnd() < 0.5 then
         mem.flfships = 2
      end
   elseif mem.level == 4 then
      mem.ships = 7
      mem.flfships = rnd.rnd( 2, 4 )
   end

   mem.credits = mem.ships * 30e3 - mem.flfships * 1e3
   mem.credits = mem.credits * system.cur():jumpDist( mem.missys, true ) / 3
   mem.credits = mem.credits + rnd.sigma() * 8e3

   local desc = setDescription()

   mem.late_arrival = rnd.rnd() < 0.75
   mem.late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   -- Set mission details
   misn.setTitle( fmt.f( misn_title[mem.level], {sys=mem.missys} ) )
   misn.setDesc( desc )
   misn.setReward( mem.credits )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   misn.accept()

   mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} )
   misn.osdCreate( _("Rogue FLF"), mem.osd_desc )

   mem.rogue_ships_left = 0
   mem.job_done = false
   mem.last_system = spob.cur()

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not mem.job_done then
      if system.cur() == mem.missys then
         misn.osdActive( 2 )
         rogue_spawnRogue( mem.ships )
         if mem.flfships > 0 then
            if not mem.late_arrival then
               rogue_spawnFLF( mem.flfships, mem.last_system )
            else
               mem.spawner = hook.timer( mem.late_arrival_delay, "timer_lateFLF" )
            end
         end
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   hook.rm( mem.spawner )
   mem.rogue_ships_left = 0
   mem.last_system = system.cur()
end


function timer_lateFLF ()
   local systems = system.cur():adjacentSystems()
   local source = systems[ rnd.rnd( 1, #systems ) ]
   rogue_spawnFLF( mem.flfships, source )
end


function pilot_death_rogue ()
   mem.rogue_ships_left = mem.rogue_ships_left - 1
   if mem.rogue_ships_left <= 0 then
      mem.job_done = true
      misn.osdActive( 3 )
      misn.markerRm( mem.marker )
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
   mem.last_system = spob.cur()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( "", text[ rnd.rnd( 1, #text ) ] )
      player.pay( mem.credits )
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
      local pos = vec2.newP( 0.8*system.cur():radius()*rnd.rnd(), rnd.angle() )
      local pstk = fleet.add( 1, shipnames, frogue, pos, pilotnames, {ai="flf_rogue_norun"} )
      local p = pstk[1]
      hook.pilot( p, "death", "pilot_death_rogue" )
      p:setHostile()
      p:setVisible( true )
      p:setHilight( true )
      --fleetRogue[i] = p
      mem.rogue_ships_left = mem.rogue_ships_left + 1
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
